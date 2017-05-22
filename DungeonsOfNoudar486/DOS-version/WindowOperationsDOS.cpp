
#include <osmesa.h>
#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/movedata.h>
#include <pc.h>
#include <sys/farptr.h>
#include <bios.h>
#include <sys/nearptr.h>

#include <cstdlib>
#include <cstdio>
#include <functional>
#include <cmath>
#include <algorithm>
#include <cmath>
#include <vector>
#include <cstring>
#include <memory>
#include <iostream>
#include <map>
#include <array>
#include <iostream>

#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


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


const int bufferWidth = 64;
const int bufferHeight = 64;
const int screenWidth = 320;
const int screenHeight = 200;
bool done = false;
bool isActive = false;
long ms = 0;
bool automatic = false;
int imageBuffer[bufferWidth * bufferHeight];


std::function< std::string(std::string)> kDosLongFileNameTransformer = [](const std::string& filename ) {
  char c = 219;
  c = 176;
  c = 177;
  c = 178;
  c = '.';
  std::cout << c;
  std::cout.flush();

  auto dotPosition = std::find( std::begin(filename), std::end( filename), '.');
  auto indexDot =  std::distance( std::begin( filename ), dotPosition );
  auto extension = filename.substr( indexDot + 1, 3 );
  
  if ( indexDot >  8 ) {
    return filename.substr( 0, 6 ) + "~1." + extension;
  }
  
  if ( filename.length() - indexDot > 4 ) {
    return filename.substr( 0, indexDot ) + "~1." + extension;
  }
  
  return filename;
};

char readKbd() {
  unsigned char controlKbd;
  
  char lastKey = inportb(0x60);

  controlKbd = inportb(0x61);
  outportb( 0x61, controlKbd | 0x80 );
  outportb( 0x61, controlKbd );
  outportb( 0x20, inportb(0x20) );

  return lastKey;
}

void initMode13h() {
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

unsigned char getPaletteEntry( int origin ) {
  unsigned char shade = 0;
  shade += (((((origin & 0x0000FF)      ) * 4  ) / 255 ) ) << 4;
  shade += (((((origin & 0x00FF00) >> 8 ) * 4  ) / 255 ) ) << 2;
  shade += (((((origin & 0xFF0000) >> 16) * 4  ) / 255 ) ) << 0;
  return shade;
}

void renderPalette() {
  _farsetsel(_dos_ds);
    
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
  
  std::fill(std::end(imageBuffer) - (320 * 200), std::end(imageBuffer), 0x0);
}

void copyImageBufferToVideoMemory() {
  
  int offset = 0;

  unsigned char buffer[ 320 * 200 ];
  unsigned char bg =  getPaletteEntry( 0xFF0000 );
  memset( buffer, bg, 320*200*sizeof(unsigned char));

  int origin = 0;
  int lastOrigin = -1;
  unsigned char shade = 0;
  
  for (int y = 0; y < 128; ++y) {
    for (int x = 0; x < 128; ++x) {
      
      offset = ((y/2) * (bufferWidth)) + (x/2);
      origin = imageBuffer[offset];
      
      if ( origin != lastOrigin ) {
	shade = getPaletteEntry( origin );
      }

      lastOrigin = origin;

      buffer[ ( y * 320 ) + (x + 96) ] = shade;
    }
  }

  dosmemput(buffer, 64000, 0xa0000);
 
  gotoxy(1, 18 );
  printf( "%s\n%d\n%ld", getCurrentObjectName().c_str(), getHP(), ms );
  
}

void exitToTextMode() {
  textmode(C80);
}

void initWindow() {
  
  OSMesaContext om = OSMesaCreateContext(OSMESA_RGBA, nullptr );
  OSMesaMakeCurrent(om, imageBuffer, GL_UNSIGNED_BYTE, bufferWidth, bufferHeight );
  OSMesaPixelStore( OSMESA_ROW_LENGTH, bufferWidth );
  OSMesaPixelStore( OSMESA_Y_UP, 0 );

  
  auto gVertexShader = "";
  auto gFragmentShader = "";
  auto fileLoader = std::make_shared<Knights::CPlainFileLoader>("res/");
  
  fileLoader->setFilenameTransformation(  kDosLongFileNameTransformer ); 
  setupGraphics(bufferWidth, bufferHeight, gVertexShader, gFragmentShader, fileLoader );
  
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
  initMode13h();
}

void tick() {
  ++ms;
  gameLoopTick(250);
  renderFrame(250);
  copyImageBufferToVideoMemory();
}

void setMainLoop() {
  
  while (!done) {
    while (kbhit())
      switch (getch()) {
      case 27:
	done = true;
	break;
      case 'H':
      case 'w':
	moveUp();
	break;
      case 'P':
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
      case 'M':
      case 'e':
	rotateCameraRight();
	break;
      case 'K':
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
  exitToTextMode();
  clrscr();
  std::cout << "Thank you for playing!" << std::endl;
}
