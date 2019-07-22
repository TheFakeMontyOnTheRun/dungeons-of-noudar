//
//  GL3NoudarFullscreenWindow.m
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

using std::vector;
using std::array;

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
	unichar c = [[event charactersIgnoringModifiers] characterAtIndex:0];
	
	switch (c) 	{
			/*
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
			*/
		case 'z':
			moveLeft();
			break;
			
		case 'x':
			moveRight();
			break;
			
		case 'q':
			exit(0);
			break;
			
		case 'w':
			pickupItem();
			break;
			
		case 'e':
			cycleNextItem();
			break;
		case 'd':
			cyclePrevItem();
			break;
			
		case 'h':
			interact();
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
}

@end
