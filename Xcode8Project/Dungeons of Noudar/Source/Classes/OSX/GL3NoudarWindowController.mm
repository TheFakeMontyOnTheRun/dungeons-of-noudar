//
//  GL3NoudarWindowController.m
//  Dungeons of Noudar
//
//  Created by Daniel Monteiro on 06/11/16.
//  Copyright © 2016 Ostrich Design Bureau. All rights reserved.
//


#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#if TARGET_IOS
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#else
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>
#endif
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#endif

#include <cstdio>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <memory>
#include <iostream>
#include <map>
#include <array>
#include <iostream>

#include "NativeBitmap.h"

#include "SoundClip.h"
#include "SoundUtils.h"
#include "SoundListener.h"
#include "SoundEmitter.h"


#include "IFileLoaderDelegate.h"
#include "CPlainFileLoader.h"

#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"


#include "NoudarDungeonSnapshot.h"
#include "GameNativeAPI.h"


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
		case '1':
			if(_fullscreenWindow == nil) {
				[self goFullscreen];
			} else {
				[self goWindow];
			}
			return;
			
		case 'z':
			moveLeft();
			break;
			
		case 'x':
			moveRight();
			break;
			
		case 'q':
			exit(0);
			break;
			
			
			
		case NSRightArrowFunctionKey:
			rotateCameraRight();
			break;
			
		case NSLeftArrowFunctionKey:
			rotateCameraLeft();
			break;
			
		case NSUpArrowFunctionKey:
			moveUp();
			break;
			
		case NSDownArrowFunctionKey:
			moveDown();
			break;
			
			
	}
	
	[super keyDown:event];
}
@end
