/*
 Copyright (C) 2015 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information
 
 Abstract:
 The Application Delegate.
 */

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


#include "GameNativeAPI.h"
#include "Common.h"


#include <vector>
#include <memory>

#import "AppDelegate.h"
#include "AppleImageLoader.h"

#include <cstdio>

std::vector <std::shared_ptr<odb::NativeBitmap>> loadTextures() {
	std::vector<std::shared_ptr<odb::NativeBitmap>> toReturn;
	
	toReturn.push_back( loadPNG( [[[NSBundle mainBundle] pathForResource:@"grass" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"stonefloor" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"bricks" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"arch" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"bars" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"begin" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"exit" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"bricks_blood" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"bricks_candles" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"boss0" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"boss1" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"boss2" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"cuco0" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"cuco1" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"cuco2" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"demon0" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"demon1" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"demon2" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"lady0" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"lady1" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"lady2" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"bull0" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"shadow" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"ceiling" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"ceilingdoor" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"ceilingbegin" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"ceilingend" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"splat0" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"splat1" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"splat2" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"ceilingbars" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"bricks" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"bricks" ofType:@"png"] UTF8String ]));	;
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"clouds" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"monty" ofType:@"png"] UTF8String ]));
	
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"stonegrassfar" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"grassstonefar" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"stonegrassnear" ofType:@"png"] UTF8String ]));
	toReturn.push_back( loadPNG([[[NSBundle mainBundle] pathForResource:@"grassstonenear" ofType:@"png"] UTF8String ]));
	
	return toReturn;
}

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	
	FILE *fd;
	fd = fopen( [[[NSBundle mainBundle] pathForResource:@"vertex" ofType:@"glsl"] UTF8String ], "r");
	std::string gVertexShader = readToString(fd);
	fclose(fd);
	
	fd = fopen( [[[NSBundle mainBundle] pathForResource:@"fragment" ofType:@"glsl"] UTF8String ], "r");
	std::string gFragmentShader = readToString(fd);
	fclose(fd);
	
//	setupGraphics(640, 480, gVertexShader, gFragmentShader, loadTextures());
	
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
	
//	setSoundEmitters( sounds, soundListener );

	
	
//	fd = std::fopen( [[[NSBundle mainBundle] pathForResource:@"map_tiles0" ofType:@"txt"] UTF8String ], "r" );
//	readMap( readToString(fd ) );
//	fclose( fd );
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

@end
