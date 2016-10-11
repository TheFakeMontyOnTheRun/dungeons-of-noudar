//
// Created by monty on 06/10/16.
//
#include <memory>
#include <string>
#include <png.h>
#include "NativeBitmap.h"
#include "LoadPNG.h"


//https://en.wikibooks.org/wiki/OpenGL_Programming/Intermediate/Textures#A_simple_libpng_example
std::shared_ptr<odb::NativeBitmap> loadPNG(const std::string filename ) {
    //header for testing if it is a png
    png_byte header[8];

    //open file as binary
    FILE *fp = fopen(filename.c_str(), "rb");

    //read the header
    fread(header, 1, 8, fp);


    //create png struct
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
                                                 NULL, NULL);

    //create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);

    //create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);

    //init png reading
    png_init_io(png_ptr, fp);

    //let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);

    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    //variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 twidth, theight;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type,
                 NULL, NULL, NULL);

    //update width and height based on png info
    int width = twidth;
    int height = theight;

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // Allocate the image_data as a big block, to be given to opengl
    png_byte *image_data = new png_byte[rowbytes * height];

    //row_pointers is for pointing to image_data for reading the png with libpng
    png_bytep *row_pointers = new png_bytep[height];
    // set the individual row_pointers to point at the correct offsets of image_data
    for (int i = 0; i < height; ++i)
        row_pointers[ i] = image_data + i * rowbytes;

    //read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);

    //clean up memory and close stuff
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    delete[] row_pointers;
    fclose(fp);

    std::shared_ptr<odb::NativeBitmap> toReturn = std::make_shared<odb::NativeBitmap>( width, height, (int*)image_data );

    return toReturn;
}