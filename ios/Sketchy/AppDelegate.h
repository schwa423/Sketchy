//
//  AppDelegate.h
//  Sketchy
//
//  Created by Joshua Gargus on 11/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate, GLKViewControllerDelegate>
{
	int renderedFrames;
}

@property (strong, nonatomic) UIWindow *window;

@end
