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
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
}

- (void)viewDidUnload
{
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


@end
