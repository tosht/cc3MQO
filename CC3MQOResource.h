//
//  CC3MQOResource.h
//  cc3MQO
//
//  Created by T.Takabayashi on 11/08/07.
//  Copyright 2011 T.Takabayashi. All rights reserved.
//

#import "CC3Resource.h"
#import "MQOModel.h"
#import "CC3Camera.h"
#import "CC3Light.h"

@interface CC3MQOResource : CC3Resource {
    
	BOOL wasLoaded;
    MQOModelRef model;
    NSString *filename;
        
    NSMutableArray *materials_;
    
    CC3BoundingBox boundingBox_;
    CC3Camera *camera_;
    CC3Light *light_;
}

@property (nonatomic, readonly) CC3BoundingBox boundingBox;
@property (nonatomic, readonly) CC3Camera *camera;
@property (nonatomic, readonly) CC3Light *light;

-(void) build;

@end
