//
//  EAGLNoudarView.m
//  Dungeons of Noudar
//
//  Created by Daniel Monteiro on 06/11/16.
//  Copyright © 2016 Ostrich Design Bureau. All rights reserved.
//

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
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
#include <vector>
#include <memory>

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






#import "EAGLNoudarView.h"

#import <OpenAl/al.h>
#import <OpenAl/alc.h>
#include <AudioToolbox/AudioToolbox.h>

@interface EAGLNoudarView () {

	EAGLContext* _context;
//	NSInteger _animationFrameInterval;
//	CADisplayLink* _displayLink;
	GLuint _colorRenderbuffer;
	GLuint _depthRenderbuffer;

}
@end

@implementation EAGLNoudarView

// Must return the CAEAGLLayer class so that CA allocates an EAGLLayer backing for this view
+ (Class) layerClass
{
	return [CAEAGLLayer class];
}

- (void)setupDisplayLink {
	CADisplayLink* displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render:)];
	[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}


-(void) onSwipeLeft:(UISwipeGestureRecognizer *)recognizer {
	NSUInteger touches = recognizer.numberOfTouches;
	switch (touches) {
		case 1:
			rotateCameraRight();
			break;
		case 2:
				cycleNextItem();
			break;
	}
	
}

-(void) onSwipeRight:(UISwipeGestureRecognizer *)recognizer {
	NSUInteger touches = recognizer.numberOfTouches;
	switch (touches) {
		case 1:
			rotateCameraLeft();
			break;
		case 2:
			cyclePrevItem();
			break;
	}
	
}

-(void) onSwipeUp {
	moveUp();
}

-(void) onSwipeDown {
	moveDown();
}

-(void) onDoubleTap {
	pickupItem();
}

-(void) onLongTap {
	interact();
}

// The GL view is stored in the storyboard file. When it's unarchived it's sent -initWithCoder:
- (instancetype) initWithCoder:(NSCoder*)coder
{
	if ((self = [super initWithCoder:coder]))
	{
		// Get the layer
		CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
		
		eaglLayer.opaque = TRUE;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
										[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		
		_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		
		if (!_context || ![EAGLContext setCurrentContext:_context])
		{
			return nil;
		}
		
		glGenRenderbuffers(1, &_depthRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, self.frame.size.width, self.frame.size.height);
		
		glGenRenderbuffers(1, &_colorRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
		
		
		// This call associates the storage for the current render buffer with the
		// EAGLDrawable (our CAEAGLLayer) allowing us to draw into a buffer that
		// will later be rendered to the screen wherever the layer is (which
		// corresponds with our view).
		[_context renderbufferStorage:GL_RENDERBUFFER fromDrawable: (id<EAGLDrawable>)self.layer ];
		
		GLuint framebuffer;
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								  GL_RENDERBUFFER, _colorRenderbuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
		
		glEnable(GL_DEPTH_TEST);


//		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//		{
//			NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
//			return nil;
//		}
		
//		_animating = FALSE;
//		_animationFrameInterval = 1;
//		_displayLink = nil;
	}
	
//	[self setupDisplayLink];
//	[self render: nil ];
	
	
	
	
	FILE *fd;
	fd = fopen( [[[NSBundle mainBundle] pathForResource:@"vertex" ofType:@"glsl"] UTF8String ], "r");
	std::string gVertexShader = Knights::readToString(fd);
	fclose(fd);
	
	fd = fopen( [[[NSBundle mainBundle] pathForResource:@"fragment" ofType:@"glsl"] UTF8String ], "r");
	std::string gFragmentShader = Knights::readToString(fd);
	fclose(fd);
	
	auto frameDimensions = [self frame];
	
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
	
	auto path = std::string( [ [ [ NSBundle mainBundle] resourcePath ] UTF8String ] ) + "/";
	
	auto fileLoader = std::make_shared<Knights::CPlainFileLoader>( path );
	
	readMap( fileLoader );
	
	setupGraphics( frameDimensions.size.width, frameDimensions.size.height, gVertexShader, gFragmentShader, fileLoader);
	
	
//	[NSTimer scheduledTimerWithTimeInterval:0.1f
//									 target:self selector:@selector(render) userInfo:nil repeats:YES];
	[self setupDisplayLink];
	
	
	mSwipeLeftRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipeLeft:)];
	mSwipeLeftRecognizer.direction = UISwipeGestureRecognizerDirectionLeft;
	[ self addGestureRecognizer: mSwipeLeftRecognizer ];

	mSwipeRightRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipeRight:)];
	mSwipeRightRecognizer.direction = UISwipeGestureRecognizerDirectionRight;
	[ self addGestureRecognizer: mSwipeRightRecognizer ];

	mSwipeUpRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipeUp)];
	mSwipeUpRecognizer.direction = UISwipeGestureRecognizerDirectionUp;
	[ self addGestureRecognizer: mSwipeUpRecognizer ];

	mSwipeDownRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipeDown)];
	
	mSwipeDownRecognizer.direction = UISwipeGestureRecognizerDirectionDown;
	[ self addGestureRecognizer: mSwipeDownRecognizer ];
	
	mDoubleSwipeLeftRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipeLeft:)];
	mDoubleSwipeLeftRecognizer.direction = UISwipeGestureRecognizerDirectionLeft;
	mDoubleSwipeLeftRecognizer.numberOfTouchesRequired = 2;
	[ self addGestureRecognizer: mDoubleSwipeLeftRecognizer ];

	mDoubleSwipeRightRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipeRight:)];
	mDoubleSwipeRightRecognizer.direction = UISwipeGestureRecognizerDirectionRight;
	mDoubleSwipeRightRecognizer.numberOfTouchesRequired = 2;
	[ self addGestureRecognizer: mDoubleSwipeRightRecognizer ];
	
	
	mDoubleTapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onDoubleTap)];
	
	mDoubleTapRecognizer.numberOfTapsRequired = 2;
	
	[self addGestureRecognizer: mDoubleTapRecognizer];
	
	mLongTapRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(onLongTap)];

	[self addGestureRecognizer:mLongTapRecognizer];
	
	return self;
}

- (void)render:(CADisplayLink*)displayLink {
	
	gameLoopTick(20);
	renderFrame(20);
	[ _context  presentRenderbuffer: GL_RENDERBUFFER ];
}

- (void) drawView:(id)sender
{

	gameLoopTick(20);
	renderFrame(20);
	[ _context  presentRenderbuffer: GL_RENDERBUFFER ];
}
//
//- (void) layoutSubviews {
//
//	
//	// The pixel dimensions of the CAEAGLLayer
//	GLint backingWidth;
//	GLint backingHeight;
//	
//	// Allocate color buffer backing based on the current layer size
//	glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
//	[_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(id<EAGLDrawable>)self.layer];
//	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
//	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
//	
//	glGenRenderbuffers(1, &_depthRenderbuffer);
//	glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
//	
//	[self drawView:nil];
//}

//- (NSInteger) animationFrameInterval
//{
//	return _animationFrameInterval;
//}
//
//- (void) setAnimationFrameInterval:(NSInteger)frameInterval
//{
//	// Frame interval defines how many display frames must pass between each time the
//	// display link fires. The display link will only fire 30 times a second when the
//	// frame internal is two on a display that refreshes 60 times a second. The default
//	// frame interval setting of one will fire 60 times a second when the display refreshes
//	// at 60 times a second. A frame interval setting of less than one results in undefined
//	// behavior.
//	if (frameInterval >= 1)
//	{
//		_animationFrameInterval = frameInterval;
//		
//		if (_animating)
//		{
//			[self stopAnimation];
//			[self startAnimation];
//		}
//	}
//}

//- (void) startAnimation
//{
//	if (!_animating)
//	{
//		// Create the display link and set the callback to our drawView method
//		_displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
//		
//		// Set it to our _animationFrameInterval
//		[_displayLink setFrameInterval:_animationFrameInterval];
//		
//		// Have the display link run on the default runn loop (and the main thread)
//		[_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
//		
//		_animating = TRUE;
//	}
//}
//
//- (void)stopAnimation
//{
//	if (_animating)
//	{
//		[_displayLink invalidate];
//		_displayLink = nil;
//		_animating = FALSE;
//	}
//}

- (void) dealloc
{
	// tear down context
	if ([EAGLContext currentContext] == _context)
	[EAGLContext setCurrentContext:nil];
}

@end

