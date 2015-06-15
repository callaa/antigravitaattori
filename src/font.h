#ifndef _FONT_H_
#define _FONT_H_

#include <GL/gl.h>

class Font
{
public:
	int init();
	void deinit();
	void drawChar(char c);
	void drawString(const char *str);
	void printf(const char *fmt, ...);
	
	
	static Font &getInstance();
private:
	static const int NUM_CHARS = 40;
	
	Font();
	static Font instance;

	static const unsigned char fontData[];
	static const char *chars;
	
	GLuint textures[NUM_CHARS];
};

#endif
