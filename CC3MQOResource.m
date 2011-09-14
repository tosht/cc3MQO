//
//  CC3MQOResource.m
//  cc3MQO
//
//  Created by T.Takabayashi on 11/08/07.
//  Copyright 2011 T.Takabayashi. All rights reserved.
//

#import "CC3MQOResource.h"


#import "MQOModel.h"

#import "CC3MQOMaterial.h"
#import "CC3VertexArrayMesh.h"
#import "CC3MeshNode.h"
#import "CC3Node.h"

@interface CC3MQOResource(Privates)
- (void)buildMaterials;
- (void)buildObjects;
- (void)buildCamera;
@end

@implementation CC3MQOResource

@synthesize boundingBox=boundingBox_,camera=camera_,light=light_;

#pragma mark Allocation and initialization

- (id)init {
	if ( (self = [super init]) ) {
		wasLoaded = NO;
        materials_ = [[NSMutableArray array] retain];
	}
	return self;
}


- (void)dealloc {
    if (model != NULL) {
        MQOModelRefRelease(model);
        model = NULL;
    }
    
    [camera_ release];
    camera_ = nil;
    [light_ release];
    light_ = nil;
    
    [materials_ release];
    
    [super dealloc];
}


- (BOOL)loadFromFile: (NSString*) aFilepath {
	if (wasLoaded) {
		LogError(@"%@ has already been loaded from MQO file '%@'", self, aFilepath);
		return wasLoaded;
	}
	LogTrace(@"Loading MQO file '%@'", aFilepath);
	self.name = aFilepath;
    
    model = MQOModelRefCreate([aFilepath UTF8String], 1.0);
    if (model == NULL) {
        wasLoaded = NO;
        LogError(@"Could not load MQO file '%@'", aFilepath);
        return NO;
    }
    wasLoaded = YES;
    [self build];
    MQOModelRefRelease(model);
    model = NULL;
    return wasLoaded;
}

- (void)build {
    [self buildMaterials];
    [self buildObjects];
    [self buildCamera];
}

#pragma mark -
#pragma mark Private methods
- (void)buildMaterials 
{
    CC3MQOMaterial *material;
    MQOMaterialRef mqoMat;
    NSString *pathDir = [self.name stringByDeletingLastPathComponent];
    for (int i = 0; i < model->material_num; i ++) {
        mqoMat = &model->material_arr[i];
        material = [CC3MQOMaterial materialWithName:[NSString stringWithFormat:@"MAT-%d", i] 
                                    withMQOMaterial:mqoMat];
        // bump map
        if (strlen(mqoMat->bmpFile) > 0) {
            NSString *bumpFile = [[NSString stringWithCString:mqoMat->bmpFile
                                                    encoding:NSShiftJISStringEncoding]
                                    stringByReplacingOccurrencesOfString:@"\\" 
                                                              withString:@"/"];
            [material addBumpTextureFromFile:
             [pathDir stringByAppendingPathComponent:
              [bumpFile lastPathComponent]]];
            
        }
        
        // texture map
        if (strlen(mqoMat->texFile) > 0) {
            NSString *texFile = [[NSString stringWithCString:mqoMat->texFile
                                                   encoding:NSShiftJISStringEncoding]
                                 stringByReplacingOccurrencesOfString:@"\\" 
                                                           withString:@"/"];
            [material addTextureFromFile:
             [pathDir stringByAppendingPathComponent:
              [texFile lastPathComponent]]];
        }
        
        // alpha map
        if (strlen(mqoMat->alpFile) > 0) {
            NSString *alpFile = [[NSString stringWithCString:mqoMat->alpFile 
                                                   encoding:NSShiftJISStringEncoding]
                                 stringByReplacingOccurrencesOfString:@"\\" 
                                                           withString:@"/"];
            [material addAlphaTextureFromFile:
             [pathDir stringByAppendingPathComponent:
              [alpFile lastPathComponent]]];
        }
        [materials_ addObject:material];
    }
    // 未設定材質を追加
    [materials_ addObject:[CC3Material shinyWhite]];

    
}

- (void)buildObjects
{
    CC3Node *modelNode = [CC3Node nodeWithName:self.name];
    for (int i = 0; i < model->object_num; i ++) {
        MQOObject *obj = &model->obj[i];
        CC3Node *objectNode = [CC3Node nodeWithName:[NSString stringWithCString:obj->objname 
                                                                             encoding:NSShiftJISStringEncoding]];
        objectNode.visible = (obj->isVisible == 0)? NO: YES;
        for (int j = 0; j < obj->mesh_num; j ++) {
            MQOMesh *mesh = &obj->mesh_arr[j];
            
            CC3VertexLocations *locations = [CC3VertexLocations vertexArrayWithName:@"locations"];
            locations.elementCount = mesh->vertex_num;
            locations.elements = mesh->locations;
            locations.drawingMode = GL_TRIANGLES;
            
            
            CC3VertexNormals *normals = [CC3VertexNormals vertexArrayWithName:@"normals"];
            normals.elementCount = mesh->vertex_num;
            normals.elements = mesh->normals;
            
            CC3VertexTextureCoordinates *textureCoordinates = [CC3VertexTextureCoordinates vertexArrayWithName:@"texture"];
            textureCoordinates.elementCount = mesh->vertex_num;
            textureCoordinates.elements = mesh->texture_cordinates;
            
            
            CC3VertexArrayMesh *mesh_ = [CC3VertexArrayMesh meshWithName:[NSString stringWithFormat:@"%@-MESH-%d",
                                                                          objectNode.name, 
                                                                          i]];
            mesh_.vertexLocations = locations;
            mesh_.vertexNormals = normals;
            mesh_.vertexTextureCoordinates = textureCoordinates;
            
            CC3BoundingBox meshBoundingBox = mesh_.boundingBox;
            boundingBox_.minimum = CC3VectorMinimize(boundingBox_.minimum, meshBoundingBox.minimum);
            boundingBox_.maximum = CC3VectorMaximize(boundingBox_.maximum, meshBoundingBox.maximum);
            
            CC3MeshNode *meshNode = [CC3MeshNode nodeWithName:mesh_.name];
            @try {
                meshNode.mesh = mesh_;
                meshNode.material = [materials_ objectAtIndex:mesh->material_index];
                
                [objectNode addChild:meshNode];
                
            }
            @catch (NSException *exception) {
            }
            @finally {
            }
        }
        
        
        [modelNode addChild:objectNode];
    }
	[modelNode createGLBuffers];
	[modelNode releaseRedundantData];
    
    [nodes addObject:modelNode];
    
}


- (void)buildCamera {
	// Create the camera, place it back a bit, and add it to the world
	CC3Camera* cam = [CC3Camera nodeWithName: @"Camera"];
    
    
    CC3Vector pos = cc3v(model->camera_pos.x, 
                         0, 
                         model->camera_pos.z);
    CC3Vector lookat = cc3v(model->camera_lookat.x, 
                            model->camera_lookat.y - model->camera_pos.y, 
                            model->camera_lookat.z);
    
    CC3GLMatrix *matrix = [CC3GLMatrix identity];
    //ピッチ、ヘッド回転設定
    [matrix rotateByX: - (model->camera_pich * 180 / M_PI)];
    [matrix rotateByY: - (model->camera_head * 180 / M_PI)];

    CCLOG(@"camera location(%f, %f, %f)",
          pos.x,
          pos.y,
          pos.z);
    CCLOG(@"camera target(%f, %f, %f)",
          lookat.x,
          lookat.y,
          lookat.z);
    
    pos = [matrix transformLocation:CC3VectorAdd(pos, CC3VectorNegate(lookat))];
    
    cam.location = CC3VectorAdd(pos, lookat);
    cam.targetLocation = lookat;
    
    
    //距離
    float dist = model->camera_pos.z;
//    float dist = CC3VectorDistance(cam.location, cam.targetLocation);
    
    float nearz = dist * 0.15f;         //ニア面
    float farz = dist * (4.0f * M_PI);  //ファー面
    //FOV計算
    const float fov_scl = 0.001f; //FOV倍率
    float w = fov_scl * model->camera_zoom2 * dist;
    float fov = atanf( 1.0f / w) * 2.0f * 180 / M_PI;
    CCLOG(@"FOV:%f", fov);
    cam.fieldOfView = fov;
//    if (model->ortho == 0) {
//        // Perspective
//        cam.isUsingParallelProjection = NO;
//        cam.nearClippingPlane = nearz;
//        cam.farClippingPlane = farz;
//    } else {
//        // Ortho
//        cam.isUsingParallelProjection = YES;
//        cam.nearClippingPlane = nearz;
//        cam.farClippingPlane = farz;
//    }
      cam.isUsingParallelProjection = NO;
      cam.nearClippingPlane = nearz;
      cam.farClippingPlane = farz;
    
    camera_ = [cam retain];
}


@end
