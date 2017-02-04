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

std::vector<std::shared_ptr<odb::NativeBitmap>> gunStates;
std::vector<std::shared_ptr<odb::NativeBitmap>>::iterator gunState;

std::array<int, 4> soundSource = {
		100,
};

std::array<int, 10>::iterator soundPos = std::end(soundSource);

namespace PC {
	const unsigned W = 320, H = 200;

	unsigned ImageBuffer[W * H];
	int selector;

	void Init() {
		__dpmi_regs r;

		r.x.ax = 0x13;
		__dpmi_int(0x10, &r);


		outp(0x03c8, 0);


		for ( int r = 0; r < 4; ++r ) {
			for ( int g = 0; g < 4; ++g ) {
				for ( int b = 0; b < 4; ++b ) {
					outp(0x03c9, (r * 85));
					outp(0x03c9, (g * 85));
					outp(0x03c9, (b * 85));
				}
			}
		}
	}

	void renderPalette() {
		_farsetsel(_dos_ds);
		int offset = 0;
		int fullSize = 320 * 200;

		for (int r = 0; r < 255; ++r) {
			for (int x = 0; x < 50; ++x) {
				int shade = 0;

				int origin = r << 16;
				shade += (((((origin & 0x0000FF))) / 85));
				shade += (((((origin & 0x00FF00) >> 8)) / 85)) << 2;
				shade += (((((origin & 0xFF0000) >> 16)) / 85)) << 4;

				_farnspokeb(0xA0000 + (320 * x) + r, shade);
			}
		}

		for (int g = 0; g < 255; ++g) {
			for (int x = 50; x < 100; ++x) {
				int shade = 0;

				int origin = g << 8;
				shade += (((((origin & 0x0000FF))) / 85));
				shade += (((((origin & 0x00FF00) >> 8)) / 85)) << 2;
				shade += (((((origin & 0xFF0000) >> 16)) / 85)) << 4;

				_farnspokeb(0xA0000 + (320 * x) + g, shade);
			}
		}

		for (int b = 0; b < 255; ++b) {
			for (int x = 100; x < 150; ++x) {
				int shade = 0;

				int origin = b;
				shade += (((((origin & 0x0000FF))) / 85));
				shade += (((((origin & 0x00FF00) >> 8)) / 85)) << 2;
				shade += (((((origin & 0xFF0000) >> 16)) / 85)) << 4;

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

  int getPaletteEntry( int origin ) {
    int shade = 0;
    shade += (((((origin & 0x0000FF)      )  ) / 85 ) );
    shade += (((((origin & 0x00FF00) >> 8 )  ) / 85 ) ) << 2;
    shade += (((((origin & 0xFF0000) >> 16)  ) / 85 ) ) << 4;
    return shade;
  }

	void Render() {
		_farsetsel(_dos_ds);
		auto pixelData = (*gunState)->getPixelData();
		int offset = 0;
		int fullSize = 320 * 200;

		for (int y = 100; y < 200; ++y) {
			for (int x = 80; x < 240; ++x) {

				offset = (320 * y) + x;
				auto origin = ImageBuffer[offset];
				offset = (320 * (200 - (2 * (y - 100)))) + (((x - 80) * 320) / 160);

				if (pixelData[offset] & 0xFF000000) {
					origin = pixelData[offset];
				}

				int shade = getPaletteEntry( origin );

				_farnspokeb( 0xA0000 + 160 + ((200 - (2 * ((y - 100)))) * 320) + ((2 * x)) + 1, shade);
				_farnspokeb( 0xA0000 + 160 + ((199 - (2 * ((y - 100)))) * 320) + ((2 * x)), shade);
				_farnspokeb( 0xA0000 + 160 + ((200 - (2 * ((y - 100)))) * 320) + ((2 * x)), shade);
				_farnspokeb( 0xA0000 + 160 + ((199 - (2 * ((y - 100)))) * 320) + ((2 * x)) + 1, shade);
			}
		}

<<<<<<< HEAD
		std::fill(std::end(ImageBuffer) - (320 * 100), std::end(ImageBuffer), getPaletteEntry(0xAAAAAA));
=======
		std::fill(std::end(ImageBuffer) - (320 * 100), std::end(ImageBuffer), 0xFFAAAA);
>>>>>>> paint the sky pinkish
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

std::vector<std::shared_ptr<odb::NativeBitmap>> loadTextures() {
	std::vector<std::shared_ptr<odb::NativeBitmap>> toReturn;

	toReturn.push_back(loadPNG("res/grass.ppm"));
	toReturn.push_back(loadPNG("res/stonef1.ppm"));
	toReturn.push_back(loadPNG("res/bricks.ppm"));
	toReturn.push_back(loadPNG("res/arch.ppm"));
	toReturn.push_back(loadPNG("res/bars.ppm"));
	toReturn.push_back(loadPNG("res/begin.ppm"));
	toReturn.push_back(loadPNG("res/exit.ppm"));
	toReturn.push_back(loadPNG("res/bricks2.ppm"));
	toReturn.push_back(loadPNG("res/bricks3.ppm"));
	toReturn.push_back(loadPNG("res/turtle0.ppm"));
	toReturn.push_back(loadPNG("res/turtle0.ppm"));
	toReturn.push_back(loadPNG("res/turtle1.ppm"));
	toReturn.push_back(loadPNG("res/turtle1.ppm"));
	toReturn.push_back(loadPNG("res/turtle1.ppm"));
	toReturn.push_back(loadPNG("res/turtle1.ppm"));
	toReturn.push_back(loadPNG("res/crusad0.ppm"));
	toReturn.push_back(loadPNG("res/crusad1.ppm"));
	toReturn.push_back(loadPNG("res/crusad2.ppm"));
	toReturn.push_back(loadPNG("res/shadow.ppm"));
	toReturn.push_back(loadPNG("res/ceilin.ppm"));
	toReturn.push_back(loadPNG("res/ceigdr.ppm"));
	toReturn.push_back(loadPNG("res/ceigbgn.ppm"));
	toReturn.push_back(loadPNG("res/ceilend.ppm"));
	toReturn.push_back(loadPNG("res/splat0.ppm"));
	toReturn.push_back(loadPNG("res/splat1.ppm"));
	toReturn.push_back(loadPNG("res/splat2.ppm"));
	toReturn.push_back(loadPNG("res/ceilbar.ppm"));
	toReturn.push_back(loadPNG("res/clouds.ppm"));
	toReturn.push_back(loadPNG("res/stngrsf.ppm"));
	toReturn.push_back(loadPNG("res/grsstnf.ppm"));
	toReturn.push_back(loadPNG("res/stngrsn.ppm"));
	toReturn.push_back(loadPNG("res/grsstnn.ppm"));
	toReturn.push_back(loadPNG("res/cross.ppm"));


	return toReturn;
}

void initWindow() {

	auto textures = loadTextures();
	gunStates.push_back(loadPNG("res/shotgun0.ppm", 320, 200));
	gunStates.push_back(loadPNG("res/shotgun1.ppm", 320, 200));
	gunState = std::begin(gunStates);

	OSMesaContext om = OSMesaCreateContext(OSMESA_RGBA, NULL);
	OSMesaMakeCurrent(om, PC::ImageBuffer, GL_UNSIGNED_BYTE, PC::W, PC::H);

	PC::Init();


	auto gVertexShader = "";
	auto gFragmentShader = "";

	setupGraphics(winWidth, winHeight, gVertexShader, gFragmentShader, textures);

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
	//if I want at least 10fps, I need my rendering and updates to take no more than 100ms, combined
	if (inGraphics) {
		gameLoopTick(250);
		renderFrame(250);
		PC::Render();

		if (gunState != std::begin(gunStates)) {
			gunState = std::prev(gunState);
		}

		if (soundPos != std::end(soundSource)) {
			sound(*soundPos);
			soundPos = std::next(soundPos);
		} else {
			nosound();
		}

	}
}


void setMainLoop() {

	while (!done) {
		while (kbhit())
			switch (getch()) {
				case 27:
					done = true;
					break;
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
				case 'h':
					interact();
					gunState = std::prev(std::end(gunStates));
					soundPos = std::begin(soundSource);
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
	setTextMode();
	clrscr();
	std::cout << "Thank you for playing!" << std::endl;
}
