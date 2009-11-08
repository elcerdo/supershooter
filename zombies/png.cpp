#include "utils.h"
#include <png.h>
#include <stdlib.h>

int write_png(const char *file, unsigned char *image_data, unsigned int width, unsigned int height) {
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 i, rowbytes;
	png_bytepp row_pointers;
	
	// Open PNG file for writting
	FILE *fp = fopen(file, "wb");
	if (!fp) return 4;
	
	// Allocate basic libpng structures
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr) { fclose(fp); return 1; }
	
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) { png_destroy_write_struct(&png_ptr, 0); fclose(fp); return 1; }
	
	// setjmp() must be called in every function
	// that calls a PNG-reading libpng function
	if (setjmp(png_ptr->jmpbuf)) { png_destroy_write_struct(&png_ptr, &info_ptr); fclose(fp); return 3; }
	
	png_init_io(png_ptr,fp);
	
	// set the zlib compression level
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
	
	// write PNG info to structure
	png_set_IHDR(png_ptr, info_ptr, width, height, 8,
	 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
	 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	png_write_info(png_ptr, info_ptr);
	
	// setjmp() must be called in every function
	// that calls a PNG-writing libpng function
	if (setjmp(png_ptr->jmpbuf)) { png_destroy_write_struct(&png_ptr, &info_ptr); fclose(fp); return 3; }
	
	// If color is BGR change to RGB
	// png_set_bgr(png_ptr);
	
	// Allocate pointers for each line
	row_pointers = new png_bytep[height];
	if (!row_pointers) { png_destroy_write_struct(&png_ptr, &info_ptr); fclose(fp); return 1; }
	
	// how many bytes in each row
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	
	// set the individual row_pointers to point at the correct offsets
	for (i = 0; i < height; ++i) row_pointers[i] = image_data + i * rowbytes;
	
	// now we can go ahead and just write the whole image
	png_write_image(png_ptr, row_pointers);
	
	png_write_end(png_ptr, 0);
	
	png_destroy_write_struct(&png_ptr, &info_ptr);
	delete [] row_pointers; fclose(fp); return 0;
}

int write_frame_float_scale(const char *file, float *frame_data, unsigned int width, unsigned int height,float min,float max) {
	int k,l;
	int size = width * height;

	unsigned char *buffer = new unsigned char[3*size];
	if (!buffer) return 1;

	for (k=0; k<height; k++) {
		for (l=0; l<width; l++) {
			unsigned char foo = 255 * (frame_data[k + l*height] - min) / (max - min);
			buffer[3*(l + k*width)] = foo;
			buffer[3*(l + k*width) + 1] = foo;
			buffer[3*(l + k*width) + 2] = foo;
		}
	}

	int rc = write_png(file,buffer,width,height);
    delete [] buffer;
	return rc;
}

int write_frame_float(const char *file, float *frame_data, unsigned int width, unsigned int height) {
	int k,l;
	int size = width * height;
	float min = frame_data[0];
	float max = frame_data[0];

	for (k=0; k<size; k++) {
		if (min>frame_data[k]) min = frame_data[k];
		if (max<frame_data[k]) max = frame_data[k];
	}

	if (min == max) return 1;

    return write_frame_float_scale(file,frame_data,width,height,min,max);
}

