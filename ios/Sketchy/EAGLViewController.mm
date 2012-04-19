//
//  EAGLViewController.mm
//  Sketchy
//
//  Created by Joshua Gargus on 12/30/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

// TODO: build framework to test view loading/unloading...
// verify that we're not leaking memory all the way down through
// the EAGLView's Renderer, etc.

#import "EAGLViewController.h"

#include <iostream>
using std::cerr;
using std::endl;

#import "EAGLView.h"

@implementation EAGLViewController

// TODO: this isn't called... which init method is being called?
// And what do we need to do there?  I guess the "page model" will
// live in the controller...
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    fprintf(stderr, "initializing view-controller\n");

    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
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
    cerr << "view did load" << endl;
    self.view.multipleTouchEnabled = TRUE;
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
}

- (void)viewDidUnload
{
    cerr << "view did unload" << endl;
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return YES;
}

- (void)viewDidDisappear:(BOOL)animated
{
    cerr << "view did disappear" << endl;
    [(EAGLView*)self.view pauseRendering];
    [super viewDidDisappear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
    cerr << "view did appear" << endl;
    [(EAGLView*)self.view unpauseRendering];
    [super viewDidAppear:animated];
}

- (void)printTouchInfo:(NSSet*)touches
{
    for (UITouch* touch in touches) {
        CGPoint pt = [touch locationInView: self.view];
        cerr << "    point: " << pt.x << "," << pt.y << endl;
    }
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    static long BEGAN = 0;
    cerr << "TOUCHES BEGAN " << BEGAN++ << endl;
    [self printTouchInfo: touches];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    static long MOVED = 0;
    cerr << "TOUCHES MOVED " << MOVED++ << endl;
    [self printTouchInfo: touches];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    static long ENDED = 0;
    cerr << "TOUCHES ENDED " << ENDED++ << endl;
    [self printTouchInfo: touches];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    static long CANCELLED = 0;
    cerr << "TOUCHES CANCELLED " << CANCELLED++ << endl;
    [self printTouchInfo: touches];
}

/*
// Send a resized event when the orientation changes.
- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
    const UIInterfaceOrientation toInterfaceOrientation = [self interfaceOrientation];
    SDL_WindowData *data = self->window->driverdata;
    UIWindow *uiwindow = data->uiwindow;
    SDL_VideoDisplay *display = SDL_GetDisplayForWindow(self->window);
    SDL_DisplayData *displaydata = (SDL_DisplayData *) display->driverdata;
    SDL_DisplayModeData *displaymodedata = (SDL_DisplayModeData *) display->current_mode.driverdata;
    UIScreen *uiscreen = displaydata->uiscreen;
    const int noborder = (self->window->flags & (SDL_WINDOW_FULLSCREEN|SDL_WINDOW_BORDERLESS));
    CGRect frame = noborder ? [uiscreen bounds] : [uiscreen applicationFrame];
    const CGSize size = frame.size;
    int w, h;

    switch (toInterfaceOrientation) {
        case UIInterfaceOrientationPortrait:
        case UIInterfaceOrientationPortraitUpsideDown:
            w = (size.width < size.height) ? size.width : size.height;
            h = (size.width > size.height) ? size.width : size.height;
            break;

        case UIInterfaceOrientationLandscapeLeft:
        case UIInterfaceOrientationLandscapeRight:
            w = (size.width > size.height) ? size.width : size.height;
            h = (size.width < size.height) ? size.width : size.height;
            break;

        default:
            SDL_assert(0 && "Unexpected interface orientation!");
            return;
    }

    w = (int)(w * displaymodedata->scale);
    h = (int)(h * displaymodedata->scale);

    [uiwindow setFrame:frame];
    [data->view setFrame:frame];
    [data->view updateFrame];
    SDL_SendWindowEvent(self->window, SDL_WINDOWEVENT_RESIZED, w, h);
}
*/

@end
