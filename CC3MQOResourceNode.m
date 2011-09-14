//
//  CC3MQOResourceNode.m
//  cc3MQO
//
//  Created by T.Takabayashi on 11/08/07.
//  Copyright 2011 T.Takabayashi. All rights reserved.
//

#import "CC3MQOResourceNode.h"
#import "CC3MQOResource.h"

@implementation CC3MQOResourceNode




- (Class)resourceClass {
	return [CC3MQOResource class];
}

- (CC3BoundingBox)resouceBoundingBox {
    if ([self.resource isKindOfClass:[CC3MQOResource class]]) {
        return ((CC3MQOResource *)self.resource).boundingBox;
    }
    CC3BoundingBox def;
    return def;
}

- (CC3Camera *)resouceCamera {
    if ([self.resource isKindOfClass:[CC3MQOResource class]]) {
        return ((CC3MQOResource *)self.resource).camera;
    }
    return nil;
}

- (CC3Light *)resouceLight {
//    if ([self.resource isKindOfClass:[CC3MQOResource class]]) {
//        return ((CC3MQOResource *)self.resource).light;
//    }
    return nil;
}



@end


#pragma mark -
#pragma mark CC3World extensions to support PVR POD content

@implementation CC3World (CC3MQOAdditional)

- (void)addContentFromMQOFile:(NSString *)aFilepath {
	[self addChild:[CC3MQOResourceNode nodeFromFile:aFilepath]];
}

- (void)addContentFromMQOFile:(NSString *)aFilepath withName:(NSString *)aName {
	[self addChild:[CC3MQOResourceNode nodeWithName:aName fromFile:aFilepath]];
}

- (void)addContentFromMQOResourceFile:(NSString *)aRezPath {
	[self addChild:[CC3MQOResourceNode nodeFromResourceFile:aRezPath]];
}

- (void)addContentFromMQOResourceFile:(NSString *)aRezPath withName:(NSString *)aName {
	[self addChild:[CC3MQOResourceNode nodeWithName:aName fromResourceFile:aRezPath]];
}

@end
