//
//  cc3MQOSampleAppDelegate.h
//  cc3MQOSample
//
//  Created by T.Takabayashi on 11/09/14.
//  Copyright T.Takabayashi 2011. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "CCNodeController.h"
#import "CC3World.h"

@interface cc3MQOSampleAppDelegate : NSObject <UIApplicationDelegate> {
	UIWindow* window;
	CCNodeController* viewController;
}

@property (nonatomic, retain) UIWindow* window;

@end
