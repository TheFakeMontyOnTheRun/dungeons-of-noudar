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
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"

#include "NoudarDungeonSnapshot.h"

#include "GameNativeAPI.h"
#include "WindowOperations.h"
#include "Common.h"
#include "LoadPNG.h"

bool inGraphics = true;

namespace PC {
    const unsigned W = 320, H = 200, mode = 0x10F; // resolution & BIOS mode#

    unsigned ImageBuffer[W*H];
    unsigned reverseBuffer[W*H];
    int selector;
    void Init() // Initialize graphics
    {
        __dpmi_regs regs = { };
        regs.x.ax = 0x4F02;
        regs.x.bx = 0x4000 | mode;
        __dpmi_int(0x10, &regs);
        // Map the video memory into memory
        __dpmi_meminfo meminfo;
        meminfo.address = 0xC0000000ul; // DOSBox's S3 video memory
        meminfo.size    = PC::W * PC::H * 4;
        __dpmi_physical_address_mapping(&meminfo);
        __dpmi_lock_linear_region(&meminfo);
        selector = __dpmi_allocate_ldt_descriptors(1);
        __dpmi_set_segment_base_address(selector, meminfo.address);
        __dpmi_set_segment_limit(selector, ((meminfo.size+4095)&~4095)-1);
    }
    void Render() // Update the displayed screen
    {
      int fullSize = 320 * 200;
      for ( int y = 100; y < 200; ++y ) {
	for ( int x = 80; x < 240; ++x ) {
	  auto origin = ImageBuffer[ (320 * y ) + x];
	  reverseBuffer[ 160 + ( (200 - (2 * ( (y - 100)) )) * 320 ) + (( 2 * x) ) + 20 ] = origin;
	  reverseBuffer[ 160 + ( (199 - (2 * ( (y - 100)) )) * 320 ) + (( 2 * x) ) + 19 ] = origin;
	  reverseBuffer[ 160 + ( (200 - (2 * ( (y - 100)) )) * 320 ) + (( 2 * x) ) + 19 ] = origin;
	  reverseBuffer[ 160 + ( (199 - (2 * ( (y - 100)) )) * 320 ) + (( 2 * x) ) + 20 ] = origin;
	}
      }

      movedata( _my_ds(), (long)(&reverseBuffer) + (320/4), selector, 0, -(320/4 ) + sizeof(reverseBuffer)  );
    }
    void Close() // End graphics
    {
        textmode(C80); // Set textmode again.
    }
}

void setGraphics() {
  inGraphics = true;
  PC::Init();
}

void setTextMode() {
  inGraphics = false;

  __dpmi_regs r;

  r.x.ax = 3;
  __dpmi_int(0x10, &r);
}

const int winWidth = 320, winHeight = 200;
bool done = false;
bool isActive = false;

std::vector <std::shared_ptr<odb::NativeBitmap>> loadTextures() {
    std::vector<std::shared_ptr<odb::NativeBitmap>> toReturn;

    toReturn.push_back( loadPNG( "res/grass.ppm") );
    toReturn.push_back( loadPNG( "res/stonef1.ppm") );
    toReturn.push_back( loadPNG( "res/bricks.ppm") );
    toReturn.push_back( loadPNG( "res/arch.ppm") );
    toReturn.push_back( loadPNG( "res/bars.ppm") );
    toReturn.push_back( loadPNG( "res/begin.ppm") );
    toReturn.push_back( loadPNG( "res/exit.ppm") );
    toReturn.push_back( loadPNG( "res/bricks2.ppm") );
    toReturn.push_back( loadPNG( "res/bricks3.ppm") );
    toReturn.push_back( loadPNG( "res/foe0.ppm") );
    toReturn.push_back( loadPNG( "res/foe1.ppm") );
    toReturn.push_back( loadPNG( "res/foe2.ppm") );
    toReturn.push_back( loadPNG( "res/foe3.ppm") );
    toReturn.push_back( loadPNG( "res/foe4.ppm") );
    toReturn.push_back( loadPNG( "res/foe5.ppm") );
    toReturn.push_back( loadPNG( "res/crusad0.ppm") );
    toReturn.push_back( loadPNG( "res/crusad1.ppm") );
    toReturn.push_back( loadPNG( "res/crusad2.ppm") );
    toReturn.push_back( loadPNG( "res/shadow.ppm") );
    toReturn.push_back( loadPNG( "res/ceilin.ppm") );
    toReturn.push_back( loadPNG( "res/ceigdr.ppm") );
    toReturn.push_back( loadPNG( "res/ceigbgn.ppm") );
    toReturn.push_back( loadPNG( "res/ceilend.ppm") );
    toReturn.push_back( loadPNG( "res/splat0.ppm") );
    toReturn.push_back( loadPNG( "res/splat1.ppm") );
    toReturn.push_back( loadPNG( "res/splat2.ppm") );
    toReturn.push_back( loadPNG( "res/ceilbar.ppm") );
    toReturn.push_back( loadPNG( "res/clouds.ppm"));
    toReturn.push_back( loadPNG( "res/stngrsf.ppm"));
    toReturn.push_back( loadPNG( "res/grsstnf.ppm"));
    toReturn.push_back( loadPNG( "res/stngrsn.ppm"));
    toReturn.push_back( loadPNG( "res/grsstnn.ppm"));
    toReturn.push_back( loadPNG( "res/cross.ppm"));


    return toReturn;
}

void initWindow() {

  auto textures = loadTextures();
  OSMesaContext om  = OSMesaCreateContext(OSMESA_RGBA, NULL);
  OSMesaMakeCurrent(om, PC::ImageBuffer, GL_UNSIGNED_BYTE, PC::W, PC::H);
  
  PC::Init();

   
  auto gVertexShader = "";
  auto gFragmentShader = "";

  setupGraphics(winWidth, winHeight, gVertexShader, gFragmentShader, textures);
  
  auto soundListener = std::make_shared<odb::SoundListener>();
  
  std::vector<std::shared_ptr<odb::SoundEmitter>> sounds;
  
  std::string filenames[] {
    "res/grasssteps.wav",
      "res/stepsstones.wav",
      "res/bgnoise.wav",
      "res/monsterdamage.wav",
      "res/monsterdead.wav",
      "res/playerdamage.wav",
      "res/playerdead.wav",
      "res/swing.wav"
      };
  
  for ( auto filename : filenames ) {
    FILE *file = fopen( filename.c_str(), "r");
    auto soundClip = odb::makeSoundClipFrom( file );
    
    sounds.push_back( std::make_shared<odb::SoundEmitter>(soundClip) );
  }
  
  setSoundEmitters( sounds, soundListener );
}

void tick() {
  //if I want at least 10fps, I need my rendering and updates to take no more than 100ms, combined
  if ( inGraphics ) {
    gameLoopTick( 250 );
    renderFrame( 250 );
    PC::Render();
  }
}



void setMainLoop() {
  while ( !done ) {
    while(kbhit())
      switch(getch()) {
      case 27: done = true; break;
      case '1':
	setGraphics();
	break;
      case '2':
	setTextMode();
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
      case 'e':
	rotateCameraRight();
	break; 
      case 'q':
	rotateCameraLeft();
	break;
      }
    tick();
  }
}

void destroyWindow() {
   shutdown();
}
