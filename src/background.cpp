#include "SDL_opengl.h"

#include <AL/al.h>
#include <cstdlib>

#include "antigrav.h"

GLuint Background::planet;

int Background::init()
{
	planet = m3dTexture::loadTexture("planet.png");
	if(planet==0)
		return -1;
	return 0;
}

Background::Background()
{
	float minx = -0.25 * Game::getInstance().getLevel().getWidth();
	float w = 1.5 * Game::getInstance().getLevel().getWidth();
	float miny = -5;
	float h = 65;
	float minz = 85;
	float d = 25;
	for(int s=0;s<STARS;++s) {
		starx[s] = minx + rand()/(double)RAND_MAX*w;
		stary[s] = miny + rand()/(double)RAND_MAX*h;
		starz[s] = minz + rand()/(double)RAND_MAX*d;
	}
}

void Background::draw()
{
	// Draw stars
	glPointSize(1.0);
	glBegin(GL_POINTS);
	for(int s=0;s<STARS;++s)
		glVertex3f(starx[s],stary[s],-starz[s]);
	glEnd();

	// Draw planet
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glTranslatef(100,32,-192);
	glScalef(64,-64,1);
	glBindTexture(GL_TEXTURE_2D, planet);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0,0);
	glVertex2f(0,0);

	glTexCoord2f(1,0);
	glVertex2f(1,0);

	glTexCoord2f(0,1);
	glVertex2f(0,1);

	glTexCoord2f(1,1);
	glVertex2f(1,1);
	glEnd();
	glPopMatrix();
}

