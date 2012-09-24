//
//  SchwaGLViewController.mm
//  Sketchy
//
//  Created by Joshua Gargus on 12/30/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

// TODO: build framework to test view loading/unloading...
// verify that we're not leaking memory all the way down through
// the SchwaGLView's Renderer, etc.

#import "SchwaGLViewController.h"

#import "TouchHandler.h"
using Sketchy::Input::Touch;

#include <iostream>
using std::cerr;
using std::endl;

#include <vector>

#import "SchwaGLView.h"

@implementation SchwaGLViewController

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];

    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    cerr << "[SchwaGLViewController viewDidLoad]" << endl;
    [super viewDidLoad];

    // Mais, bien sur!
    self.view.multipleTouchEnabled = TRUE;

    // Guarantee that we always update the orientation of a new view
    // (and hence initialize the default framebuffer appropriately).
    _orientationValid = false;
}

- (void)viewDidUnload
{
    cerr << "[SchwaGLViewController viewDidUnload]" << endl;
    [super viewDidUnload];
}

- (NSInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskPortrait
         & UIInterfaceOrientationMaskLandscapeLeft
         & UIInterfaceOrientationMaskLandscapeRight
         & UIInterfaceOrientationMaskPortraitUpsideDown;
}

- (void)viewDidDisappear:(BOOL)animated
{
    cerr << "view did disappear" << endl;
    [(SchwaGLView*)self.view pauseRendering];

    [super viewDidDisappear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
    cerr << "view did appear" << endl;
    [(SchwaGLView*)self.view unpauseRendering];

    // Whenever view appears, we potentially need to update
    // the orientation, since we views do not receive rotation
    // events when not visible.
    [self updateOrientation];

    // Perhaps this should go first?  Not sure.
    [super viewDidAppear:animated];
}

- (void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation duration:(NSTimeInterval)duration
{
    // Clearly, need to update orientation when rotating.
    [self updateOrientation];
}

- (void)updateOrientation
{
    cerr << "[SchwaGLViewController updateOrientation]" << endl;

    UIInterfaceOrientation newOrientation = [self interfaceOrientation];

    if (_orientationValid && _orientation == newOrientation) {
        cerr << "   ... early exit because valid orientation hasn't changed" << endl;
        return;
    }

    // Remember new orientation, and update view.
    _orientation = newOrientation;
    _orientationValid = true;
    [(SchwaGLView*)self.view updateOrientation: _orientation];
}

- (void)printTouchInfo:(NSSet*)touches
{
    for (UITouch* touch in touches) {
        CGPoint pt = [touch locationInView: self.view];
        cerr << "    point: " << pt.x << "," << pt.y << endl;
    }
}

- (std::vector<Touch>) createTouches:(NSSet*)touches
{
    std::vector<Touch> vect;
    for (UITouch* touch in touches) {
        CGPoint pt = [touch locationInView: self.view];
        vect.push_back(Touch(reinterpret_cast<uint64_t>(touch),
                             pt.x, pt.y,
                             static_cast<Touch::Phase>(touch.phase - UITouchPhaseBegan),
                             touch.timestamp, touch.tapCount));
    }
    return vect;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (_touchHandler.get()) {
        _touchHandler->touchesBegan([self createTouches: touches]);
    } else {
        static long BEGAN = 0;
        cerr << "TOUCHES BEGAN " << BEGAN++ << "     COUNT: " << [[event allTouches] count] << endl;
        [self printTouchInfo: touches];
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (_touchHandler.get()) {
        _touchHandler->touchesMoved([self createTouches: touches]);
    } else {
        static long MOVED = 0;
        cerr << "TOUCHES MOVED " << MOVED++ << "     COUNT: " << [[event allTouches] count] << endl;
        [self printTouchInfo: touches];
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (_touchHandler.get()) {
        _touchHandler->touchesEnded([self createTouches: touches]);
    } else {
        static long ENDED = 0;
        cerr << "TOUCHES ENDED " << ENDED++ << "     COUNT: " << [[event allTouches] count] << endl;
        [self printTouchInfo: touches];
    }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (_touchHandler.get()) {
        _touchHandler->touchesCancelled([self createTouches: touches]);
    } else {
        static long CANCELLED = 0;
        cerr << "TOUCHES CANCELLED " << CANCELLED++ << endl;
        [self printTouchInfo: touches];
    }
}

- (void)setTouchHandler:(shared_ptr<Sketchy::Input::TouchHandler>)handler
{
    _touchHandler = handler;
}

@end
