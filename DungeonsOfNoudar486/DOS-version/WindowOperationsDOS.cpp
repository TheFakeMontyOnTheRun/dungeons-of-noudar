#include <stdio.h>
#include <stdlib.h>

#include <osmesa.h>
#include <conio.h>        // For kbhit, getch, textmode (console access)
#include <dpmi.h>         // For __dpmi_int (mouse access)
#include <go32.h>         // For _dos_ds (VRAM access)
#include <sys/movedata.h> // For movedata (VRAM access)
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <GL/glu.h>     // GLU = OpenGL utility library


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <go32.h>
#include <sys/farptr.h>
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

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <pc.h>
#include "NativeBitmap.h"

#include "SoundClip.h"
#include "SoundUtils.h"
#include "SoundListener.h"
#include "SoundEmitter.h"

#include "IFileLoaderDelegate.h"

#include "Vec2i.h"
#include "NativeBitmap.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"
#include "CPlainFileLoader.h"
#include "NoudarDungeonSnapshot.h"

#include "GameNativeAPI.h"
#include "WindowOperations.h"
#include "Common.h"
#include "LoadPNG.h"
#include "DOSHacks.h"

std::function< std::string(std::string)> kDosLongFileNameTransformer = [](const std::string& filename ) {
      std::cout << "bla: " << filename << std::endl;

      auto dotPosition = std::find( std::begin(filename), std::end( filename), '.');
      auto indexDot =  std::distance( std::begin( filename ), dotPosition );
      auto extension = filename.substr( indexDot + 1, 3 );

      if ( indexDot >  8 ) {
	std::cout << indexDot  << std::endl;
	auto toReturn = filename.substr( 0, 6 ) + "~1." + extension;

	std::cout << toReturn << std::endl;
	return toReturn;
      }


      
      if ( filename.length() - indexDot > 4 ) {
	std::cout <<  filename.length() << " - " <<  indexDot  << std::endl;
	auto toReturn = filename.substr( 0, indexDot ) + "~1." + extension;

	std::cout << toReturn << std::endl;
	return toReturn;
      }

      return filename;
  };

const int winWidth = 128, winHeight = 64;
bool done = false;
bool isActive = false;
long ms = 0;
bool automatic = false;

namespace PC {
	const unsigned W = 320, H = 200;

	unsigned ImageBuffer[winWidth * winHeight];
	int selector;

	void Init() {
		__dpmi_regs reg;

		reg.x.ax = 0x13;
		__dpmi_int(0x10, &reg);


		outp(0x03c8, 0);

		for ( int r = 0; r < 4; ++r ) {
		  for ( int g = 0; g < 4; ++g ) {
		    for ( int b = 0; b < 4; ++b ) {
		      outp(0x03c9, (r * (85) ) );
		      outp(0x03c9, (g * (85) ) );
		      outp(0x03c9, (b * (85) ) );
		    }
		  }
		}
	}

  int getPaletteEntry( int origin ) {
    int shade = 0;
    shade += (((((origin & 0x0000FF)      ) * 4  ) / 255 ) ) << 4;
    shade += (((((origin & 0x00FF00) >> 8 ) * 4  ) / 255 ) ) << 2;
    shade += (((((origin & 0xFF0000) >> 16) * 4  ) / 255 ) ) << 0;
    return shade;
  }
  
	void renderPalette() {
		_farsetsel(_dos_ds);
		int offset = 0;
		int fullSize = 320 * 200;

		for (int r = 0; r < 255; ++r) {
		  for (int x = 0; x < 50; ++x) {
		    
		    int origin = r << 16;
		    int shade = getPaletteEntry( origin );
		    
		    _farnspokeb(0xA0000 + (320 * x) + r, shade);
		  }
		}

		for (int g = 0; g < 255; ++g) {
			for (int x = 50; x < 100; ++x) {
				int origin = g << 8;
				int shade = getPaletteEntry( origin );
				_farnspokeb(0xA0000 + (320 * x) + g, shade);
			}
		}

		for (int b = 0; b < 255; ++b) {
			for (int x = 100; x < 150; ++x) {
				int origin = b;
				int shade = getPaletteEntry( origin );
				_farnspokeb(0xA0000 + (320 * x) + b, shade);
			}
		}

		for (int b = 0; b < 255; ++b) {
			for (int x = 150; x < 200; ++x) {
				_farnspokeb(0xA0000 + (320 * x) + b, b);
			}
		}

		std::fill(std::end(ImageBuffer) - (320 * 200), std::end(ImageBuffer), 0x0);
	}



	void Render() {
	  _farsetsel(_dos_ds);
	  int offset = 0;
	  int fullSize = 320 * 200;
	  
	 
	  
	  for (int y = 0; y < 128; ++y) {
	    for (int x = 0; x < 256; ++x) {
	      
	      offset = ((y/2) * (winWidth)) + (x/2);
	      auto origin = ImageBuffer[offset];
	      
	      int shade = getPaletteEntry( origin );
	      
	      _farnspokeb( 0xA0000 + (320 * y ) + (x+32), shade);
	    }
	  }


	  for (int y = 0; y < 129; ++y) {
	    for (int x = 0; x < 32; ++x) {
	      _farnspokeb( 0xA0000 + (320 * y ) + x, 0);
	    }
	  }

	  for (int y = 0; y < 129; ++y) {
	    for (int x = 288; x < 320; ++x) {
	      _farnspokeb( 0xA0000 + (320 * y ) + x, 0 );
	    }
	  }

	  for (int y = 128; y < 200; ++y) {
	    for (int x = 0; x < 320; ++x) {
	      _farnspokeb( 0xA0000 + (320 * y ) + x, 0);
	    }
	  }
	  gotoxy(0, 21 );
	  std::cout << std::endl;
	  std::cout << getCurrentObjectName() << std::endl;
	  std::cout << getHP() << std::endl;
	}

	void Close() {
	  textmode(C80); // Set textmode again.
	}
}

void initWindow() {
	OSMesaContext om = OSMesaCreateContext(OSMESA_RGBA, NULL);
	OSMesaMakeCurrent(om, PC::ImageBuffer, GL_UNSIGNED_BYTE, winWidth, winHeight );
	OSMesaPixelStore( OSMESA_ROW_LENGTH, winWidth );
	OSMesaPixelStore( OSMESA_Y_UP, 0 );
	PC::Init();


	auto gVertexShader = "";
	auto gFragmentShader = "";

	auto fileLoader = std::make_shared<Knights::CPlainFileLoader>("res/");

	fileLoader->setFilenameTransformation(  kDosLongFileNameTransformer );

	setupGraphics(winWidth, winHeight, gVertexShader, gFragmentShader, fileLoader );

	auto soundListener = std::make_shared<odb::SoundListener>();

	std::vector<std::shared_ptr<odb::SoundEmitter>> sounds;

	std::string filenames[]{
			"res/grasssteps.wav",
			"res/stepsstones.wav",
			"res/bgnoise.wav",
			"res/monsterdamage.wav",
			"res/monsterdead.wav",
			"res/playerdamage.wav",
			"res/playerdead.wav",
			"res/swing.wav"
	};

	for (auto filename : filenames) {
		FILE *file = fopen(filename.c_str(), "r");
		auto soundClip = odb::makeSoundClipFrom(file);

		sounds.push_back(std::make_shared<odb::SoundEmitter>(soundClip));
	}

	setSoundEmitters(sounds, soundListener);
}

void tick() {
  gameLoopTick(250);
  renderFrame(250);
  PC::Render();
}


void setMainLoop() {

	while (!done) {
		while (kbhit())
			switch (getch()) {
				case 27:
					done = true;
					break;
				case 'w':
					moveUp();
					break;
				case 's':
					moveDown();
					break;
				case 'd':
					moveRight();
					break;
				case 'a':
					moveLeft();
					break;
				case 'c':
					interact();
					break;
				case 'e':
					rotateCameraRight();
					break;
				case 'q':
					rotateCameraLeft();
					break;

				case 'z':
				  cycleNextItem();
					break;
				case 'x':
				  cyclePrevItem();
					break;

				case 'r':
				  pickupItem();
					break;
				case 'f':
				  dropItem();
					break;										
			}
		tick();
	}
}

void destroyWindow() {
	shutdown();	
	PC::Close();
	clrscr();
	std::cout << "Thank you for playing!" << std::endl;
}
