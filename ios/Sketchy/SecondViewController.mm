//
//  SecondViewController.m
//  Sketchy
//
//  Created by Joshua Gargus on 11/12/11.
//  Copyright (c) 2011 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//

#import "SecondViewController.h"
#import "SchwaGLViewController.h"

@implementation SecondViewController

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
	[super viewDidDisappear:animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
	return YES;
}

- (IBAction)destroyRenderView:(id)sender {
	fprintf(stderr, "destroyRenderView: button was clicked\n");
	UITabBarController *parent = (UITabBarController*)self.parentViewController;
	SchwaGLViewController *renderer = parent.viewControllers.lastObject;
	renderer.view = nil;
}
@end
