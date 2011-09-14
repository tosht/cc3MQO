//
//  CC3MQOResourceNode.h
//  cc3MQO
//
//  Created by T.Takabayashi on 11/08/07.
//  Copyright 2011 T.Takabayashi. All rights reserved.
//


#import "CC3ResourceNode.h"
#import "CC3World.h"
#import "CC3Light.h"

@interface CC3MQOResourceNode : CC3ResourceNode {
}
@property (nonatomic, readonly) CC3BoundingBox resouceBoundingBox;
@property (nonatomic, readonly) CC3Camera *resouceCamera;
@property (nonatomic, readonly) CC3Light *resouceLight;
@end



#pragma mark -
#pragma mark CC3World extensions to support PVR POD content

/**
 * This category extends CC3World to add convenience methods for loading
 * MQO content directly into the CC3World instance, adding the extracted
 * and configured nodes as child nodes to the CC3World.
 */
@interface CC3World (CC3MQOAdditional)

-(void) addContentFromMQOFile: (NSString*) aFilepath;

-(void) addContentFromMQOFile: (NSString*) aFilepath withName: (NSString*) aName;

-(void) addContentFromMQOResourceFile: (NSString*) aRezPath;

-(void) addContentFromMQOResourceFile: (NSString*) aRezPath withName: (NSString*) aName;

@end
