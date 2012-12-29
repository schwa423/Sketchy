//
//  SchwaGLViewController.mm
//  Sketchy
//
//  Created by Joshua Gargus on 12/30/11.
//  Copyright (c) 2011 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//

// TODO: build framework to test view loading/unloading...
// verify that we're not leaking memory all the way down through
// the SchwaGLView's Renderer, etc.

#import "SchwaGLViewController.h"

#import "TouchHandler.h"
using schwa::input::Touch;

#include <iostream>
using std::cerr;
using std::endl;

#include <vector>

#import "SchwaGLView.h"

#include "renderer_ios.h"
#include "presenter_ios.h"

// TODO: Un-hardwire this
#include "pageview.h"
using namespace schwa;
using namespace schwa::app;


@implementation SchwaGLViewController

- (id)initWithNibName:(NSString *)nibName bundle:(NSBundle *)bundle
{
    self = [super initWithNibName:@"SchwaGLView" bundle:bundle];
    if (self != nil) {
        // Further initialization if needed
    }
    return self;
}

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

    // Plug view into presenter.
    // TODO: don't hardcode view creation.
    auto presenter = [(SchwaGLView*)self.view presenter];
    auto renderer = [(SchwaGLView*)self.view renderer];
    auto page = schwa::grfx::View::New<sketchy::PageView>(renderer);
    presenter->setView(page);
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
    cerr << "[SchwaGLViewController viewDidDisappear:]" << endl;
    [(SchwaGLView*)self.view stopRendering];

    [super viewDidDisappear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
    cerr << "[SchwaGLViewController viewDidAppear:]" << endl;

    // Whenever view appears, we potentially need to update
    // the orientation, since we views do not receive rotation
    // events when not visible.
    [self updateOrientation];

    [(SchwaGLView*)self.view startRendering];

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
    if (_touchHandler) {
        _touchHandler->touchesBegan([self createTouches: touches]);
    } else {
        static long BEGAN = 0;
        cerr << "TOUCHES BEGAN " << BEGAN++ << "     COUNT: " << [[event allTouches] count] << endl;
        [self printTouchInfo: touches];
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (_touchHandler) {
        _touchHandler->touchesMoved([self createTouches: touches]);
    } else {
        static long MOVED = 0;
        cerr << "TOUCHES MOVED " << MOVED++ << "     COUNT: " << [[event allTouches] count] << endl;
        [self printTouchInfo: touches];
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (_touchHandler) {
        _touchHandler->touchesEnded([self createTouches: touches]);
    } else {
        static long ENDED = 0;
        cerr << "TOUCHES ENDED " << ENDED++ << "     COUNT: " << [[event allTouches] count] << endl;
        [self printTouchInfo: touches];
    }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (_touchHandler) {
        _touchHandler->touchesCancelled([self createTouches: touches]);
    } else {
        static long CANCELLED = 0;
        cerr << "TOUCHES CANCELLED " << CANCELLED++ << endl;
        [self printTouchInfo: touches];
    }
}

- (void)setTouchHandler:(shared_ptr<schwa::input::TouchHandler>)handler
{
    _touchHandler = handler;
}

@end
