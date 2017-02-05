//
// Created by monty on 06/10/16.
//
#include <vector>
#include <array>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "NativeBitmap.h"
#include "LoadPNG.h"
#include "Logger.h"

std::shared_ptr<odb::NativeBitmap> loadPNG(const std::string filename, int width, int height ) {
   std::cout << "opening " << filename << std::endl;
   std::ifstream file(filename);

	if (!file.is_open()) {
		std::cout << "failed to open " << filename << std::endl;
		exit(0);
		return nullptr;
	}
	
	std::string line;
	int* data = new int[  width * height ];
	int* pos = data;   

	for (int i=0;i<3;i++) { 
		std::getline(file,line); 
	}

        for (int i=0;i< width * height;i++) {
		int r;
		int g;
		int b;
		
            file >> r;
            file >> g;
            file >> b;

		int val = 0;
		val += ( b & 0xFF );
		val += ( g & 0xFF ) << 8;
		val += ( r & 0xFF ) << 16;

		if ( (r == b) && (b == 255) ) {
		  val = ( 0x00FFFFFF & val );
		} else {
		  val += ( 0xFF ) << 24;
		}

		*pos = val;
		++pos;
        }




    	std::shared_ptr<odb::NativeBitmap> toReturn = std::make_shared<odb::NativeBitmap>( width, height, data );
	return toReturn;
}

std::shared_ptr<odb::NativeBitmap> loadPNG(const std::string filename ) {
  return loadPNG( filename, 64, 64 );
}

VGApalette extractPalette( std::shared_ptr<odb::NativeBitmap> bitmap ) {
  VGApalette toReturn;

  int used = 0;

  int height = bitmap->getHeight();
  int width = bitmap->getWidth();
  int* data = bitmap->getPixelData();

  for ( int y = 0; y < height; ++y ) {
    for ( int x = 0; x < width; ++x ) {
      int pixel = *( data + ( y * width ) + x );
      if ( std::find( std::begin(toReturn), std::begin( toReturn) + used, pixel ) == 0 ) {
	toReturn[ ++used ] = pixel;
      }
    }
  }

  return toReturn;
}

VGApalette mergePalettes( std::vector< VGApalette > palettes ) {
  VGApalette toReturn;
  return toReturn;
}

