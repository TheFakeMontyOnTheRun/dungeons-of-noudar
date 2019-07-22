//
//  GL3NoudarView.m
//  Dungeons of Noudar
//
//  Created by Daniel Monteiro on 06/11/16.
//  Copyright © 2016 Ostrich Design Bureau. All rights reserved.
//



#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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



#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#if TARGET_IOS
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#else
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
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
#include "NativeBitmap.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"

#include "NoudarDungeonSnapshot.h"

#include "GameNativeAPI.h"
#include "Common.h"


#include <vector>
#include <memory>



#include <cstdio>

#include "Logger.h"
#include "GameNativeAPI.h"

#import "GL3NoudarView.h"

bool ready = false;

long timeSinceStart = 0;

@interface GL3NoudarView ()
{
//	OpenGLRenderer* _renderer;
}
@end

@implementation GL3NoudarView


- (CVReturn) getFrameForTime:(const CVTimeStamp*)outputTime
{
	// There is no autorelease pool when this method is called
	// because it will be called from a background thread.
	// It's important to create one or app can leak objects.
	@autoreleasepool {
		[self drawView];
	}
	return kCVReturnSuccess;
}

// This is the renderer output callback function
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,
									  const CVTimeStamp* now,
									  const CVTimeStamp* outputTime,
									  CVOptionFlags flagsIn,
									  CVOptionFlags* flagsOut,
									  void* displayLinkContext)
{
	CVReturn result = [(__bridge GL3NoudarView*)displayLinkContext getFrameForTime:outputTime];
	return result;
}

- (void) awakeFromNib
{
	NSOpenGLPixelFormatAttribute attrs[] =
	{
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 32,
		NSOpenGLPFADepthSize, 32,
		NSOpenGLPFAOpenGLProfile,
		NSOpenGLProfileVersionLegacy,
		0
	};


	
	
	NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	
	if (!pf)
	{
		NSLog(@"No OpenGL pixel format");
	}
	
	NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
	
	// When we're using a CoreProfile context, crash if we call a legacy OpenGL function
	// This will make it much more obvious where and when such a function call is made so
	// that we can remove such calls.
	// Without this we'd simply get GL_INVALID_OPERATION error for calling legacy functions
	// but it would be more difficult to see where that function was called.
//	CGLEnable([context CGLContextObj], kCGLCECrashOnRemovedFunctions);
	
	[self setPixelFormat:pf];
	
	[self setOpenGLContext:context];
}

- (void) prepareOpenGL
{
	[super prepareOpenGL];
	
	// Make all the OpenGL calls to setup rendering
	//  and build the necessary rendering objects
	[self initGL];
	
	// Create a display link capable of being used with all active displays
	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
	
	// Set the renderer output callback function
	CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, (__bridge void*)self);
	
	// Set the display link for the current renderer
	CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
	CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
	
	// Activate the display link
	CVDisplayLinkStart(displayLink);
	
	// Register to be notified when the window closes so we can stop the displaylink
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(windowWillClose:)
												 name:NSWindowWillCloseNotification
											   object:[self window]];
}

- (void) windowWillClose:(NSNotification*)notification
{
	// Stop the display link when the window is closing because default
	// OpenGL render buffers will be destroyed.  If display link continues to
	// fire without renderbuffers, OpenGL draw calls will set errors.
	
	CVDisplayLinkStop(displayLink);
}

- (void) initGL
{
	// The reshape function may have changed the thread to which our OpenGL
	// context is attached before prepareOpenGL and initGL are called.  So call
	// makeCurrentContext to ensure that our OpenGL context current to this
	// thread (i.e. makeCurrentContext directs all OpenGL calls on this thread
	// to [self openGLContext])
	[[self openGLContext] makeCurrentContext];
	
	// Synchronize buffer swaps with vertical refresh rate
	GLint swapInt = 1;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	
	// Init our renderer.  Use 0 for the defaultFBO which is appropriate for
	// OSX (but not iOS since iOS apps must create their own FBO)
//	_renderer = [[OpenGLRenderer alloc] initWithDefaultFBO:0];
	
	float  glLanguageVersion;
	sscanf((char *)glGetString(GL_SHADING_LANGUAGE_VERSION), "%f", &glLanguageVersion);
	odb::Logger::log( "GLSL: %s", glLanguageVersion );
}

- (void)reshape
{
	[super reshape];
	
	// We draw on a secondary thread through the display link. However, when
	// resizing the view, -drawRect is called on the main thread.
	// Add a mutex around to avoid the threads accessing the context
	// simultaneously when resizing.
	CGLLockContext([[self openGLContext] CGLContextObj]);
	
	// Get the view size in Points
	NSRect viewRectPoints = [self bounds];
	
	NSRect viewRectPixels = viewRectPoints;
	
	auto path = std::string( [ [ [ NSBundle mainBundle] resourcePath ] UTF8String ] ) + "/";
	
	auto fileLoader = std::make_shared<Knights::CPlainFileLoader>( path );

	readMap( std::make_shared<Knights::CPlainFileLoader>( path ) );
	
	auto vertex = fileLoader->loadFileFromPath("vertex.glsl");
	auto fragment = fileLoader->loadFileFromPath("fragment.glsl");
	
	setupGraphics(viewRectPixels.size.width, viewRectPixels.size.height, vertex, fragment, fileLoader);
	
	auto soundListener = std::make_shared<odb::SoundListener>();
	
	std::vector<std::shared_ptr<odb::SoundEmitter>> sounds;
	
	std::string filenames[] {
		[[[NSBundle mainBundle] pathForResource:@"grasssteps" ofType:@"wav"] UTF8String ],
		[[[NSBundle mainBundle] pathForResource:@"stepsstones" ofType:@"wav"] UTF8String ],
		[[[NSBundle mainBundle] pathForResource:@"bgnoise" ofType:@"wav"] UTF8String ],
		[[[NSBundle mainBundle] pathForResource:@"monsterdamage" ofType:@"wav"] UTF8String ],
		[[[NSBundle mainBundle] pathForResource:@"monsterdead" ofType:@"wav"] UTF8String ],
		[[[NSBundle mainBundle] pathForResource:@"playerdamage" ofType:@"wav"] UTF8String ],
		[[[NSBundle mainBundle] pathForResource:@"playerdead" ofType:@"wav"] UTF8String ],
		[[[NSBundle mainBundle] pathForResource:@"swing" ofType:@"wav"] UTF8String ]
	};
	
	for ( auto filename : filenames ) {
		FILE *file = fopen( filename.c_str(), "r");
		auto soundClip = odb::makeSoundClipFrom( file );
		
		sounds.push_back( std::make_shared<odb::SoundEmitter>(soundClip) );
	}
	
	setSoundEmitters( sounds, soundListener );
	
	
	
	


	
	CGLUnlockContext([[self openGLContext] CGLContextObj]);
	
	ready = true;
}


- (void)renewGState
{
	// Called whenever graphics state updated (such as window resize)
	
	// OpenGL rendering is not synchronous with other rendering on the OSX.
	// Therefore, call disableScreenUpdatesUntilFlush so the window server
	// doesn't render non-OpenGL content in the window asynchronously from
	// OpenGL content, which could cause flickering.  (non-OpenGL content
	// includes the title bar and drawing done by the app with other APIs)
	[[self window] disableScreenUpdatesUntilFlush];
	
	[super renewGState];
}

- (void) drawRect: (NSRect) theRect
{
	// Called during resize operations
	
	// Avoid flickering during resize by drawiing
	[self drawView];
}

- (void) drawView
{
	if (!ready) {
		return;
	}
	
	@synchronized (self) {
		[[self openGLContext] makeCurrentContext];
		{
			gameLoopTick( 20 );
			renderFrame( 20 );
			
		}
		[[self openGLContext] flushBuffer];
	}
}

- (void) dealloc
{
	// Stop the display link BEFORE releasing anything in the view
	// otherwise the display link thread may call into the view and crash
	// when it encounters something that has been release
	CVDisplayLinkStop(displayLink);
	
	CVDisplayLinkRelease(displayLink);
}
@end
