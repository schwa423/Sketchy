//
//  GLKSceneDelegate.mm
//  Sketchy
//
//  Created by Joshua Gargus on 11/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "GLKSceneController.h"
#import "OpenGLES/EAGL.h"

@interface GLKSceneController () {
    EAGLContext *context;
}
- (void)setupGL;
- (void)tearDownGL;
@end

@implementation GLKSceneController

- (id) init
{
    if (self = [super init]) {
		renderedFrames = 0;
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
	
    if (!context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = context;
    self.preferredFramesPerSecond = 60;
    
    view.contentScaleFactor = [UIScreen mainScreen].scale;
    
    [self setupGL];    
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:context];
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:context];    
}


#pragma mark - GLKView and GLKViewController delegate method

- (void)update 
{
	NSLog(@"UPDATED FRAME: %d", renderedFrames);
	renderedFrames++;
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
	glClearColor(0.5, 1.0, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}


@end
