//
//  CC3MQOMaterial.m
//  cc3MQO
//
//  Created by T.Takabayashi on 11/08/07.
//  Copyright 2011 T.Takabayashi. All rights reserved.
//

#import "CC3MQOMaterial.h"


@implementation CC3MQOMaterial

+ (id)materialWithName:(NSString *)aName withMQOMaterial:(MQOMaterialRef)mqoMaterial {
    CC3MQOMaterial *material = [CC3MQOMaterial materialWithName:aName];

    if (!material) {
        return nil;
    }
    
    material.color = ccc3(mqoMaterial->col.r * 255, 
                          mqoMaterial->col.g * 255, 
                          mqoMaterial->col.b * 255);
    
    material.ambientColor = CCC4FMake(mqoMaterial->amb[0], 
                                      mqoMaterial->amb[1], 
                                      mqoMaterial->amb[2], 
                                      mqoMaterial->amb[3]);
    
    material.diffuseColor = CCC4FMake(mqoMaterial->dif[0], 
                                      mqoMaterial->dif[1], 
                                      mqoMaterial->dif[2], 
                                      mqoMaterial->dif[3]);
    
    material.emissionColor = CCC4FMake(mqoMaterial->emi[0], 
                                       mqoMaterial->emi[1], 
                                       mqoMaterial->emi[2], 
                                       mqoMaterial->emi[3]);
    
    material.specularColor = CCC4FMake(mqoMaterial->spc[0], 
                                       mqoMaterial->spc[1], 
                                       mqoMaterial->spc[2], 
                                       mqoMaterial->spc[3]);
    
  
    
    return material;
}

-(void) addTextureFromFile:(NSString *)aFilepath {
    CC3Texture * aTexture = [CC3Texture textureFromFile:aFilepath];
    [self addTexture:aTexture];
    CCLOG(@"%@", aFilepath);
    if (aTexture.hasPremultipliedAlpha) {
        self.isOpaque = NO;
        self.sourceBlend = GL_ONE;
    }
}

-(void) addAlphaTextureFromFile:(NSString *)aFilepath {
//    return;
    CC3Texture * aTexture = [CC3Texture textureFromFile:aFilepath];

    // アルファマップを作成
    UIImage *img = [UIImage imageWithContentsOfFile:aFilepath];
    
    if (img == nil) {
        CCLOG(@"Could not load image:%@", aFilepath);
        return;
    }
    
    CGColorSpaceRef graySpace = CGColorSpaceCreateDeviceGray();
    CGContextRef alphaContext = CGBitmapContextCreate(nil, 
                                                      img.size.width, 
                                                      img.size.height, 
                                                      8, 
                                                      0, 
                                                      graySpace, 
                                                      kCGImageAlphaNone);
    CGColorSpaceRelease(graySpace);
    CGContextDrawImage(alphaContext, 
                       CGRectMake(0, 0, img.size.width, img.size.height), 
                       [img CGImage]);
    
    CGImageRef maskImg = CGBitmapContextCreateImage(alphaContext);
    
    CGImageRef alpImageRef = CGImageCreateWithMask([img CGImage], maskImg);
    
    if (alpImageRef == nil) {
        CCLOG(@"mask not found:%@", aFilepath);
    } else {
        CCTexture2D *alpTex = [[CCTextureCache sharedTextureCache] addCGImage:alpImageRef 
                                                                       forKey:[aFilepath stringByAppendingString:@".alp"]];
        
        aTexture.texture = alpTex;
        

        CC3ConfigurableTextureUnit *texunit = [CC3ConfigurableTextureUnit textureUnit];
//        texunit.textureEnvironmentMode = GL_COMBINE;
//        
//        texunit.combineRGBFunction = GL_REPLACE;
//        texunit.rgbSource0 = GL_PREVIOUS;
        texture.textureUnit = texunit;
        
        [self addTexture:aTexture];
        
        self.isOpaque = NO;
        self.sourceBlend = GL_ONE;
    }
    CGImageRelease(alpImageRef);
    CGImageRelease(maskImg);
    CGContextRelease(alphaContext);
    
}

-(void) addBumpTextureFromFile:(NSString *)aFilepath {
    CC3Texture * aTexture = [CC3Texture textureFromFile:aFilepath];
    aTexture.textureUnit = [CC3BumpMapTextureUnit textureUnit];
    [self addTexture:aTexture];
}


@end
