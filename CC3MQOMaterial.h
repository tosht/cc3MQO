//
//  CC3MQOMaterial.h
//  cc3MQO
//
//  Created by T.Takabayashi on 11/08/07.
//  Copyright 2011 T.Takabayashi. All rights reserved.
//

#import "CC3Material.h"
#import "MQOModel.h"

@interface CC3MQOMaterial : CC3Material {
    
}

+ (id)materialWithName:(NSString *)aName withMQOMaterial:(MQOMaterialRef)mqoMaterial;

-(void) addTextureFromFile:(NSString *)aFilepath;

-(void) addAlphaTextureFromFile:(NSString *)aFilepath;

-(void) addBumpTextureFromFile:(NSString *)aFilepath;

@end
