//
//  GL3NoudarWindowController.m
//  Dungeons of Noudar
//
//  Created by Daniel Monteiro on 06/11/16.
//  Copyright © 2016 Ostrich Design Bureau. All rights reserved.
//

#import "GL3NoudarWindowController.h"
#import "GL3NoudarFullscreenWindow.h"



@interface GL3NoudarWindowController () {
	
	GL3NoudarFullscreenWindow *_fullscreenWindow;
	NSWindow* _standardWindow;
}
@end

@implementation GL3NoudarWindowController

- (instancetype)initWithWindow:(NSWindow *)window {
	self = [super initWithWindow:window];
	
	if (self) {
		_fullscreenWindow = nil;
	}
	
	return self;
}

- (void) goFullscreen {

	if(_fullscreenWindow) {
		return;
	}
	

	_fullscreenWindow = [[GL3NoudarFullscreenWindow alloc] init];

	NSRect viewRect = [_fullscreenWindow frame];
	
	[self.window.contentView setFrameSize: viewRect.size];
	
	[_fullscreenWindow setContentView:self.window.contentView];
	
	_standardWindow = [self window];
	
	[_standardWindow orderOut:self];
	
	[self setWindow:_fullscreenWindow];
	
	[_fullscreenWindow makeKeyAndOrderFront:self];
}

- (void) goWindow {

	if(_fullscreenWindow == nil) 	{
		return;
	}
	
	NSRect viewRect = [_standardWindow frame];
	
	[self.window.contentView setFrame:viewRect];
	
	[self setWindow:_standardWindow];
	
	[[self window] setContentView:_fullscreenWindow.contentView];
	
	[[self window] makeKeyAndOrderFront:self];
	
	_fullscreenWindow = nil;
}


- (void) keyDown:(NSEvent *)event {
	
	unichar c = [[event charactersIgnoringModifiers] characterAtIndex:0];
	
	switch (c) 	{
			
		case 27:
			if(_fullscreenWindow != nil) {
				[self goWindow];
			}
			
			return;
		case 'a':
			if(_fullscreenWindow == nil) {
				[self goFullscreen];
			} else {
				[self goWindow];
			}
			return;
	}
	
	[super keyDown:event];
}		
@end
