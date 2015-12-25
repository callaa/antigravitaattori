#define GL_GLEXT_PROTOTYPES
#include "SDL_opengl.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <png.h>

using namespace std;

#include "tinyxml.h"
#include "m3dmaterial.h"
#include "m3dtexture.h"
#include "m3dmesh.h"

#include "extensions.h"

/// Create a new null object
/**
	Create a new object and reset rotation, scaling and transformation
*/
m3dTexture::m3dTexture()
{
	texUnits = NULL;
	numTexUnits = 0;
}

m3dTexture::~m3dTexture()
{
// 	for(int i = 0; i < numTexUnits; i++)
// 	{
// 		glDeleteTextures(1, &texUnits[i].handle);
// 	}

	delete[] texUnits;
}

int m3dTexture::loadFromXML(const TiXmlElement *root)
{
// 	for(int i = 0; i < numTexUnits; i++)
// 	{
// 		glDeleteTextures(1, &texUnits[i].handle);
// 	}

	delete[] texUnits;

	if(string(root->Value()) != "Texture")
	{
		fprintf(stderr, "Unknown node type: %s  (required: %s)\n", root->Value(), "Texture");
		return -1;
	}

	if(root->QueryIntAttribute("units", &numTexUnits) != TIXML_SUCCESS) return -1;

	int maxTexUnits;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTexUnits);
	if(numTexUnits > maxTexUnits) numTexUnits = maxTexUnits;

	texUnits = new struct TextureUnit[numTexUnits];

	int n = 0;
	const TiXmlElement *element = root->FirstChildElement();
	const char *attr;
	string value;
	while(element)
	{
		value = element->Value();

		if(value == "Image")
		{
			if(n >= numTexUnits)
			{
				fprintf(stderr, "Invalid: too many texture units!\n");
				return -1;
			}

			attr = element->Attribute("filename");
			if(attr == NULL)
			{
				fprintf(stderr, "Invalid: texture unit without filename!\n");
				return -1;
			}

			texUnits[n].filename = string(attr);

			n++;
		}

		element = element->NextSiblingElement();
	}

	if(n != numTexUnits)
	{
		fprintf(stderr, "Invalid texture: incorrect number of texture units (wanted %d, got %d)!\n", numTexUnits, n);
		return -1;
	}

	for(n = 0; n < numTexUnits; n++)
	{
		unsigned char *data = NULL;

		glGenTextures(1, &(texUnits[n].handle));
		// ERROR CHECK!

		if(loadPNG(texUnits[n].filename.c_str(), &data, &(texUnits[n].width), &(texUnits[n].height)) != 0)
		{
			fprintf(stderr, "Invalid: can't load texture %s\n", texUnits[n].filename.c_str());
			return -1;
		}

		glBindTexture(GL_TEXTURE_2D, texUnits[n].handle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texUnits[n].width, texUnits[n].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Advanced texture parameters here (anisotropy, mipmapping, different filters, etc)
		delete[] data;
	}

	return 0;
}

int m3dTexture::load(const char *filename)
{
	return load(1, &filename);
}

int m3dTexture::load(int num, const char *filenames[])
{
/*	for(int i = 0; i < numTexUnits; i++)
	{
		glDeleteTextures(1, &texUnits[i].handle);
	}*/

	delete[] texUnits;

	numTexUnits = num;
	texUnits = new struct TextureUnit[numTexUnits];

	for(int n = 0; n < numTexUnits; n++)
	{
		texUnits[n].filename = std::string(filenames[n]);

		unsigned char *data = NULL;

		glGenTextures(1, &(texUnits[n].handle));
		// ERROR CHECK!

		if(loadPNG(texUnits[n].filename.c_str(), &data, &(texUnits[n].width), &(texUnits[n].height)) != 0)
		{
			fprintf(stderr, "Invalid: can't load texture %s\n", texUnits[n].filename.c_str());
			return -1;
		}

		glBindTexture(GL_TEXTURE_2D, texUnits[n].handle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texUnits[n].width, texUnits[n].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Advanced texture parameters here (anisotropy, mipmapping, different filters, etc)
		delete[] data;
	}

	return 0;
}

void m3dTexture::bind() const
{
	if(numTexUnits < 1) return;

#ifdef HAVE_MULTITEX
	for(int i = 0; i < numTexUnits; i++)
	{
		mglActiveTextureARB(GL_TEXTURE0_ARB + i);
// 		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texUnits[i].handle);
	}
#else
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texUnits[0].handle);
#endif
}

int m3dTexture::getNumTexUnits() const
{
	return numTexUnits;
}

void m3dTexture::pngReadCallbackSTDIO(png_structp pngPtr, png_bytep data, png_size_t length)
{
	FILE *f;

	f = (FILE*) png_get_io_ptr(pngPtr);
	fread(data, length, 1, f);
}

/// Load a PNG image from a file
/**
	Only 32-bit RGBA images supported

	@param filename the filename to load from
	@param data a pointer to an uninitialized data area where the image data will be stored
	@param width a pointer where to store the image width
	@param height a pointer where to store the image width
	@return 0 on success, -1 on failure
*/
int m3dTexture::loadPNG(const char *filename, unsigned char **data, png_uint_32 *width, png_uint_32 *height)
{
	FILE *f;
	int result;

	f = fopen(filename, "rb");
	if(f == NULL)
	{
		fprintf(stderr, "Can't open file %s\n", filename);
		return -1;
	}

	result = loadPNG(data, width, height, f, m3dTexture::pngReadCallbackSTDIO);
	fclose(f);
	return result;
}

int m3dTexture::loadPNG(unsigned char **data, png_uint_32 *width, png_uint_32 *height, void *handle, void (*pngReadCallback)(png_structp ctx, png_bytep area, png_size_t size))
{
	png_structp pngPtr;
	png_infop pngInfoPtr;
	int bitDepth, colorType, interlaceType;
// 	unsigned char header[4];
	volatile int ckey = -1;
	png_color_16 *transv;
	png_bytep *rowPointers;
	unsigned int row;

	pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!pngPtr)
	{
		*data = NULL;
		return -1;
	}

	pngInfoPtr = png_create_info_struct(pngPtr);
	if(!pngInfoPtr)
	{
		png_destroy_read_struct(&pngPtr, NULL, NULL);
		*data = NULL;
		return -1;
	}

	if (setjmp(png_jmpbuf(pngPtr)))
	{
		perror("setjmp");
		png_destroy_read_struct(&pngPtr, &pngInfoPtr, NULL);
		*data = NULL;
		return -1;
	}

	png_set_read_fn(pngPtr, handle, pngReadCallback);

	png_read_info(pngPtr, pngInfoPtr);
	png_get_IHDR(pngPtr, pngInfoPtr, width, height, &bitDepth, &colorType, &interlaceType, NULL, NULL);

	png_set_strip_16(pngPtr);

	png_set_packing(pngPtr);

	if(png_get_valid(pngPtr, pngInfoPtr, PNG_INFO_tRNS))
	{
		int num_trans;
		unsigned char *trans;
		png_get_tRNS(pngPtr, pngInfoPtr, &trans, &num_trans, &transv);
		ckey = 0;
	}

	if(colorType != PNG_COLOR_TYPE_RGB_ALPHA || bitDepth != 8 || png_get_channels(pngPtr, pngInfoPtr) != 4)
	{
		fprintf(stderr, "Only 32-bit RGBA png images are supported\n");
		return -1;
	}

	png_read_update_info(pngPtr, pngInfoPtr);
	png_get_IHDR(pngPtr, pngInfoPtr, width, height, &bitDepth, &colorType, &interlaceType, NULL, NULL);

	(*data) = new unsigned char[(*width) * (*height) * png_get_channels(pngPtr, pngInfoPtr)];
	if((*data) == NULL)
	{
		fprintf(stderr, "loadPng(): Out of memory !\n");
		png_destroy_read_struct(&pngPtr, &pngInfoPtr, NULL);
		*data = NULL;
		return -1;
	}

	rowPointers = new png_bytep[*height];
	if(!rowPointers)
	{
		perror("malloc");
		png_destroy_read_struct(&pngPtr, &pngInfoPtr, NULL);
		delete[] (*data);
		*data = NULL;
		return -1;
	}

	for(row = 0; (unsigned int) row < (*height); row++)
	{
		rowPointers[row] = (png_bytep)*data + (row * (*width) * png_get_channels(pngPtr, pngInfoPtr));
	}
	png_read_image(pngPtr, rowPointers);
	png_read_end(pngPtr, pngInfoPtr);

	png_destroy_read_struct(&pngPtr, &pngInfoPtr, NULL);
	delete[] rowPointers;
	return 0;
}

int m3dTexture::savePNG(const char *filename, const unsigned char *data, png_uint_32 width, png_uint_32 height)
{
	FILE *f;
	int result;

	f = fopen(filename, "wb");
	if(f == NULL)
	{
		fprintf(stderr, "Can't open %s for writing!\n", filename);
		return -1;
	}

	result = savePNG(data, width, height, (void*)f, m3dTexture::pngWriteCallbackSTDIO, m3dTexture::pngFlushCallbackSTDIO);
	fclose(f);
	return result;
}

void m3dTexture::pngWriteCallbackSTDIO(png_structp pngPtr, png_bytep data, png_size_t length)
{
	FILE *f;

	f = (FILE*) png_get_io_ptr(pngPtr);
	fwrite(data, length, 1, f);
}

void m3dTexture::pngFlushCallbackSTDIO(png_structp pngPtr)
{
	FILE *f;

	f = (FILE*) png_get_io_ptr(pngPtr);
	fflush(f);
}

int m3dTexture::savePNG(const unsigned char *data, png_uint_32 width, png_uint_32 height, void *handle, void (*pngWriteCallback)(png_structp pngPtr, png_bytep data, png_size_t length), void (*pngFlushCallback)(png_structp pngPtr))
{
	png_structp pngPtr;
	png_infop pngInfoPtr;
	png_bytep *rowPointers;
	int i;
	png_color_8 sig_bit;

	pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	pngInfoPtr = png_create_info_struct(pngPtr);
	png_set_IHDR(pngPtr, pngInfoPtr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, 0, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_DEFAULT);
	png_set_write_fn(pngPtr, handle, pngWriteCallback, pngFlushCallback);

	// pngInfoPtr->width = width;
	// pngInfoPtr->height = height;
	// pngInfoPtr->rowbytes = width * 4;
	// pngInfoPtr->bit_depth = 8;
	// pngInfoPtr->interlace_type = 0;
	// pngInfoPtr->num_palette = 0;
	// pngInfoPtr->valid = 0;

	sig_bit.red = 8;
	sig_bit.green = 8;
	sig_bit.blue = 8;
	sig_bit.alpha = 8;

	png_set_sBIT(pngPtr, pngInfoPtr, &sig_bit);

	// pngInfoPtr->color_type = PNG_COLOR_TYPE_RGB_ALPHA;

	png_write_info(pngPtr, pngInfoPtr);

	rowPointers = new png_bytep[png_get_image_height(pngPtr,pngInfoPtr)];

	for(i = 0; (unsigned int) i < png_get_image_height(pngPtr,pngInfoPtr); i++)
	{
		rowPointers[i] = (unsigned char*)data + i * width * 4;
	}

	png_write_image(pngPtr, rowPointers);
	png_write_end(pngPtr, pngInfoPtr);
	delete rowPointers;
	png_destroy_write_struct(&pngPtr, &pngInfoPtr);
	return 0;
}

int m3dTexture::screenshot(const char *filename)
{
	unsigned char *data;
	unsigned int width, height;
	unsigned char *ptr1, *ptr2;
	unsigned int i, j;

	unsigned char temp;
	GLint viewport[4];

	glGetIntegerv(GL_VIEWPORT, viewport);
	width = viewport[2];
	height = viewport[3];
	data = new unsigned char[width * height * 4];
	if(data == NULL)
	{
		return -1;
	}

	glReadBuffer(GL_COLOR_BUFFER_BIT);
	glReadPixels(viewport[0], viewport[1], viewport[2], viewport[3], GL_RGBA, GL_UNSIGNED_BYTE, data);

	ptr1 = data;
	for(i = 0; i < height/2; i++)
	{
		ptr2 = data + (height - i - 1) * width * 4;
		for(j = 0 ; j < width * 4; j++)
		{
			temp = *ptr1;
			*ptr1++ = *ptr2;
			*ptr2++ = temp;
		}
	}

	if(savePNG(filename, data, width, height) != 0)
	{
		delete data;
		return -1;
	}

	delete data;
	return 0;
}


GLuint m3dTexture::loadTexture(const char *filename)
{
	unsigned char *data;
	png_uint_32 width, height;
	GLuint tex;

	glGenTextures(1, &tex);

	if(m3dTexture::loadPNG(filename, &data, &width, &height) != 0)
	{
		fprintf(stderr, "Can't load texture %s\n", filename);
		fprintf(stderr, "Width %d, Height %d\n",&width, &height);
		return 0;
	}

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	delete[] data;

	if(glGetError() != GL_NO_ERROR)
	{
		return 0;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}

m3dTexture &m3dTexture::operator=(const m3dTexture &t)
{
	if(&t == this) return *this;

// 	for(int i = 0; i < numTexUnits; i++)
// 	{
// 		glDeleteTextures(1, &texUnits[i].handle);
// 	}

	if(t.getNumTexUnits() != numTexUnits || texUnits == NULL)
	{
		delete[] texUnits;
		texUnits = new struct TextureUnit[t.getNumTexUnits()];
	}

	numTexUnits = t.getNumTexUnits();

	for(int i = 0; i < numTexUnits; i++)
	{
// 		texUnits[i].filename = t.texUnits[i].filename;
		texUnits[i].handle = t.texUnits[i].handle;
		texUnits[i].width = t.texUnits[i].width;
		texUnits[i].height = t.texUnits[i].height;
	}

	return *this;
}
