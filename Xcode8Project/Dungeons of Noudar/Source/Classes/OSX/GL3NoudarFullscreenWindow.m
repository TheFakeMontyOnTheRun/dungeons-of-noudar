//
//  GL3NoudarFullscreenWindow.m
//  Dungeons of Noudar
//
//  Created by Daniel Monteiro on 06/11/16.
//  Copyright © 2016 Ostrich Design Bureau. All rights reserved.
//

#import "GL3NoudarFullscreenWindow.h"

@implementation GL3NoudarFullscreenWindow

-(instancetype)init {
	NSRect screenRect = [[NSScreen mainScreen] frame];
	
	self = [super initWithContentRect:screenRect
							styleMask:NSBorderlessWindowMask
							  backing:NSBackingStoreBuffered
								defer:YES];
	
	[self setLevel:NSMainMenuWindowLevel+1];
	
	[self setOpaque:YES];
	
	[self setHidesOnDeactivate:YES];
	
	return self;
}

-(BOOL)canBecomeKeyWindow {
	return YES;
}

- (void)keyDown:(NSEvent *)event {
	[[self windowController] keyDown:event];
}

@end
