#include <stdio.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES
# include <osmesa.h>     // For everything OpenGL, but done all in software.

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

namespace PC
{
    //const unsigned W = 848, H = 480, mode = 0x225; // resolution & BIOS mode#
    const unsigned W = 320, H = 200, mode = 0x10F; // resolution & BIOS mode#
   // const unsigned W=424, H=W*9/16, mode=0xFFF;
}
namespace PC
{
    const unsigned DitheringBits = 6;
    const unsigned R = 7, G = 9, B = 4; // 7*9*4 regular palette (252 colors)
    const double PaletteGamma = 1.5;            // Apply this gamma to palette
    const double DitherGamma = 2.0/PaletteGamma;// Apply this gamma to dithering
    const bool TemporalDithering = true;
    unsigned char ColorConvert[3][256][256], Dither8x8[8][8];
    unsigned Pal[R*G*B];

    unsigned ImageBuffer[W*H];
    unsigned reverseBuffer[W*H];
    int selector;
    void Init() // Initialize graphics
    {
        // Set DOSBox mode 848x480, 32 bpp
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

        // Initialize mouse
        regs.x.ax = 0;
        __dpmi_int(0x33, &regs);


        // Create bayer 8x8 dithering matrix.
        for(unsigned y=0; y<8; ++y)
            for(unsigned x=0; x<8; ++x)
                Dither8x8[y][x] =
                    ((x  ) & 4)/4u + ((x  ) & 2)*2u + ((x  ) & 1)*16u
                  + ((x^y) & 4)/2u + ((x^y) & 2)*4u + ((x^y) & 1)*32u;
        // Create gamma-corrected look-up tables for dithering.
        double dtab[256], ptab[256];
        for(unsigned n=0; n<256; ++n)
        {
            dtab[n] = (255.0/256.0) - std::pow(n/256.0, 1/DitherGamma);
            ptab[n] = std::pow( n/255.0, 1.0 / PaletteGamma);
        }
        for(unsigned n=0; n<256; ++n)
            for(unsigned d=0; d<256; ++d)
            {
                ColorConvert[0][n][d] =     std::min(B-1, (unsigned)(ptab[n]*(B-1) + dtab[d]));
                ColorConvert[1][n][d] =   B*std::min(G-1, (unsigned)(ptab[n]*(G-1) + dtab[d]));
                ColorConvert[2][n][d] = G*B*std::min(R-1, (unsigned)(ptab[n]*(R-1) + dtab[d]));
            }
        for(unsigned color=0; color< R*G*B; ++color)
        {
            Pal[color] =
                0x40000 * (int)(std::pow(((color/(B*G))%R)*1./(R-1), PaletteGamma) *63)
              + 0x00400 * (int)(std::pow(((color/    B)%G)*1./(G-1), PaletteGamma) *63)
              + 0x00004 * (int)(std::pow(((color      )%B)*1./(B-1), PaletteGamma) *63);
        }
    }
    void Render() // Update the displayed screen
    {
        //for(auto& e: ImageBuffer) e &= 0xC0C0C0;
		std::reverse_copy(ImageBuffer, ImageBuffer + (320*200), reverseBuffer );
        movedata( _my_ds(), (long)&reverseBuffer, selector, 0, sizeof(reverseBuffer));
    }
    void Close() // End graphics
    {
        textmode(C80); // Set textmode again.
    }
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
    gameLoopTick( 20 );
    renderFrame( 20 );
    PC::Render();
}



void setMainLoop() {
	while ( !done ) {

        while(kbhit())

            switch(getch())
            {
                case 'q': case 27: case 'Q': done = true; break;
                case 'w':                         moveUp();   break; // forward
                case 's':                         moveDown(); break; // backward
                case 'a':                         rotateCameraLeft(); break; // strafe left
                case 'd':                         rotateCameraRight();  break; // strafe right
            }


	      tick();
	}
}


void destroyWindow() {
   shutdown();
}
