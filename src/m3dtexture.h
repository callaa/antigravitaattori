#ifndef _M3DTEXTURE_H_
#define _M3DTEXTURE_H_

#include <GL/gl.h>
#include <string>
#include <png.h>

struct TextureUnit
{
	std::string filename;
	GLuint handle;
	png_uint_32 width, height;
};

/// A texture
/**
	@todo All the actual texture stuff
*/
class m3dTexture
{
public:
	m3dTexture();
	~m3dTexture();
	
#ifdef TINYXML_INCLUDED
	int loadFromXML(const TiXmlElement *root);
#endif

	int load(const char *filename);
	int load(int num, const char *filenames[]);

	void bind() const;
	int getNumTexUnits() const;
	
	m3dTexture &operator=(const m3dTexture &t);

	static int loadPNG(const char *filename, unsigned char **data, png_uint_32 *width, png_uint_32 *height);
	static int savePNG(const char *filename, const unsigned char *data, png_uint_32 width, png_uint_32 height);
	static int screenshot(const char *filename);
	
	static GLuint loadTexture(const char *filename);

private:
	struct TextureUnit *texUnits;
	int numTexUnits;

	static void pngReadCallbackSTDIO(png_structp pngPtr, png_bytep data, png_size_t length);
	static void pngWriteCallbackSTDIO(png_structp pngPtr, png_bytep data, png_size_t length);
	static void pngFlushCallbackSTDIO(png_structp pngPtr);
	
	static int loadPNG(unsigned char **data, png_uint_32 *width, png_uint_32 *height, void *handle, void (*pngReadCallback)(png_structp ctx, png_bytep area, png_size_t size));
	static int savePNG(const unsigned char *data, png_uint_32 width, png_uint_32 height, void *handle, void (*pngWriteCallback)(png_structp pngPtr, png_bytep data, png_size_t length), void (*pngFlushCallback)(png_structp pngPtr));
};

#endif
