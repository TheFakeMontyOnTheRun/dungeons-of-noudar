
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
#include <cstring>
#include <memory>
#include <iostream>
#include <map>
#include <iostream>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using eastl::vector;
using eastl::array;


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

const int bufferWidth = 160;
const int bufferHeight = 100;
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
    for ( int g = 0; g < 8; ++g ) {
      for ( int b = 0; b < 8; ++b ) {
	outp(0x03c9, (r * (21) ) );
	outp(0x03c9, (g * (8) ) );
	outp(0x03c9, (b * (8) ) );
      }
    }
  }
}

unsigned char getPaletteEntry( int origin ) {
  return ((((((origin & 0x0000FF)      ) << 2 ) >> 8 ) ) << 6 )
    + ((((((origin & 0x00FF00) >> 8 ) << 3 ) >> 8 ) ) << 3 )
    + ((((((origin & 0xFF0000) >> 16) << 3 ) >> 8 ) ) << 0);
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

  unsigned char buffer[ screenWidth * screenHeight ];
  unsigned char bg =  getPaletteEntry( 0xAA );
  memset( buffer, bg, screenWidth * screenHeight *sizeof(unsigned char));

  int origin = 0;
  int lastOrigin = -1;
  unsigned char shade = 0;
  
  for (int y = 0; y < (bufferHeight * 2 ); ++y) {
    for (int x = 0; x < (bufferWidth * 2); ++x) {
      
      offset = ((y/2) * (bufferWidth)) + (x/2);
      origin = imageBuffer[offset];
      
      if ( origin != lastOrigin ) {
	shade = getPaletteEntry( origin );
      }

      lastOrigin = origin;

      buffer[ ( y * screenWidth ) + (x + ( ( screenWidth - (2 * bufferWidth) ) / 2) ) ] = shade;
    }
  }

  dosmemput(buffer, screenWidth * screenHeight, 0xa0000);
 
  gotoxy(1, 22 );
  printf( "%s\n%d\n%s\n%ld", getCurrentObjectName().c_str(), getHP(), to_string(getCurrentDirection()).c_str(), ms );
  
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
  
  vector<std::shared_ptr<odb::SoundEmitter>> sounds;
  
  setSoundEmitters(sounds, soundListener);
  initMode13h();
}

void tick() {
  ++ms;
  gameLoopTick(250);
  renderFrame(250);
  copyImageBufferToVideoMemory();
}

char read_char(void)
{
  union REGS regs;
  regs.x.ax = 0x0100; //DOS function 01h
  int86(0x21,&regs,&regs);
  
  return regs.x.ax;
}


void setMainLoop() {
  
  while (!done) {
    auto lastKey = bioskey(0x11);
    auto extendedKeys = bioskey(0x12);
    
    if (extendedKeys & (0b0000000000000100) ||
        extendedKeys & (0b0000000100000000)
        ) {
      //ctrl
      dropItem();
      
    }
    
    if (extendedKeys & (0b0000000000000001) ||
        extendedKeys & (0b0000000000000010)
        ) {
      //shift
      pickupItem();
      
    }
    
    bdos(0xC, 0, 0);
    
    switch (lastKey) {
    case 9836: //l
    case 11640: //x
      cyclePrevItem();
      break;
    case 27:
    case 283:
      //esc
      done = true;
      break;
    case 'w':
    case 4471:
    case 18656:
      //up
      moveUp();
      break;

    case 7777: //a
      moveLeft();
      break;

    case 8292: //d
      moveRight();
      break;

    case 's':
    case 8051:
    case 20704:
      //down
      moveDown();
      break;
    case 'a':
    case 19424: //right arrow
    case 4209: //q

      //left
      rotateCameraLeft();
      break;
    case 'd':

    case 4709: //e
    case 19936: //right arrow 
      //right
      rotateCameraRight();
      break;
    case ' ':
    case 3849:
    case 14624:
    case 11785: //c
    case 5236: //t
      //space
      interact();
      break;
    case 7181:
    case 11386: //z
      //enter
      cycleNextItem();
      break;
    case 0:
      break;

    case 4978: //r
      pickupItem();
      break;

    case 8550: //f
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
