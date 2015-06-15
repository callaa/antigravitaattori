#include "SDL.h"
#include "SDL_opengl.h"
#include <AL/al.h>

#include "antigrav.h"

const float Menu::ANIMLEN = 0.25;
GLuint Menu::keys[4];

Menu::Menu()
{
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	width = vp[2];
	height = vp[3];
}

int Menu::init()
{
	char name[10];
	for(int p=0;p<4;p++) {
		sprintf(name,"keys%d.png",p+1);
		keys[p] = m3dTexture::loadTexture(name);
		if(!keys[p])
			return 1;
	}

	return 0;
}

int Menu::show()
{
	SDL_Event event;
	int rval=-1;

	Game &game = Game::getInstance();
	for(int p=0;p<4;p++) {
		char name[20];
		sprintf(name,"Pelaaja %d",p+1);
		game.getPlayer(p).setLocal(true);
		game.getPlayer(p).setActive(false);
		game.getPlayer(p).setName(name);
		game.getPlayer(p+4).setLocal(false);
		game.getPlayer(p+4).setActive(false);
		anim[p]=0;
	}
	startanim = 0;
	canstart = false;

	Uint32 startTime = SDL_GetTicks();
	while(rval==-1) {
		// Update timer and frame rate
		Uint32 frameTime = SDL_GetTicks() - startTime;
		if(frameTime < Game::MIN_FRAME_TIME && Game::FRAME_LIMITER) {
			SDL_Delay(Game::MIN_FRAME_TIME - frameTime);
		}
		frameTime = (SDL_GetTicks() - startTime);
		float t = (frameTime) / 1000.0f;
		startTime = SDL_GetTicks();

		for(int p=0;p<4;++p) {
			bool active = game.getPlayer(p).isActive();
			if(active && anim[p]<ANIMLEN) {
				anim[p] += t;
                if(anim[p]>ANIMLEN) anim[p]=ANIMLEN;
            } else if(!active && anim[p]>0) {
				anim[p] -= t;
                if(anim[p]<0) anim[p]=0;
            }
		}
		if(canstart && startanim < ANIMLEN) {
			startanim += t;
            if(startanim>ANIMLEN)
                startanim=ANIMLEN;
        } else if(!canstart && startanim > 0) {
			startanim -= t;
            if(startanim<0)
                startanim=0;
        }

		update();
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) rval=1;
			else if(event.type == SDL_KEYDOWN) {
				if(event.key.keysym.sym == SDLK_ESCAPE) rval=1;
				else if(event.key.keysym.sym == SDLK_RETURN) {
					// Start game if at least one player is active
					if(canstart)
						rval=0;
				} else {
					// Toggle player activity
					for(int p=0;p<Game::MAX_LOCAL_PLAYERS;++p) {
						for(int c=0;c<Game::NUM_CONTROLS;++c) {
							if(event.key.keysym.sym == game.CONTROLS[p][c]) {
								togglePlayer(p);
								break;
							}
						}
					}
				}
			}
		}
	}

	return rval;
}

void Menu::togglePlayer(int p)
{
	Game &game = Game::getInstance();
	Player &plr = game.getPlayer(p);
	plr.setActive(!plr.isActive());
	int act=0;
	for(int plr=0;plr<4;plr++) {
		if(game.getPlayer(plr).isActive())
			++act;
	}
	canstart = act>0;
}

void Menu::drawPlayer(int p)
{
	Player &plr = Game::getInstance().getPlayer(p);

	glViewport((width/2)*(p%2), (height/2)*!(p/2), width/2, height/2);

	// Draw the ship
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLdouble)width/height, 0.1, 100.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	gluLookAt(1,2,3, 0,0,0, 0,1,0);
	float scale = anim[p]/ANIMLEN+1;
	float rot = anim[p]/ANIMLEN*360;
	glScalef(scale,scale,scale);
	glRotatef(rot,1,1,0);

	plr.getCraft().getMesh().setTexture(0,plr.getTexture());
	plr.getCraft().getMesh().draw();

	// Draw keys
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, width/16.0, height/16.0, 0.0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_2D, keys[p]);
	glPushMatrix();
	glTranslatef(((p%2)?((width/16.0)):24)-12.0,
			((p/2)?(height/16.0):22)-11, 0);
	glScalef(16,16,1);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0,0);
	glVertex2f(-0.5,-0.5);
	glTexCoord2f(1,0);
	glVertex2f(0.5,-0.5);
	glTexCoord2f(0,1);
	glVertex2f(-0.5,0.5);
	glTexCoord2f(1,1);
	glVertex2f(0.5,0.5);
	glEnd();
	glPopMatrix();

	// Draw text
	Font &font = Font::getInstance();
	glScalef(2.0,2.0,1);
	glPushMatrix();
	glTranslatef(((p%2)?(width/32.0)-2:strlen(plr.getName())+2) -
				strlen(plr.getName()),1,0);
	font.drawString(plr.getName());

	glPopMatrix();
	if(anim[p]<ANIMLEN) {
		glTranslatef((width/32.0)/2.0 - 12/2 , height/32.0-4.0, 0.0);
		glColor4f(1,1,1,1-anim[p]/ANIMLEN);
		font.drawString("Paina nappia");
	}
	glColor4f(1,1,1,1);
}

void Menu::update()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	// Draw players
	for(int p=0;p<4;p++) {
		drawPlayer(p);
	}

	// Draw startup text
	if(startanim>0) {
		glViewport(0, 0, width, height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, width/16.0, height/16.0, 0.0, -1.0, 1.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		Font &font = Font::getInstance();
		glTranslatef((width/16.0)/2.0 - 13/2.0,(height/16.0)/2.0-1,0);
		glColor4f(1,1,1,startanim/ANIMLEN);
		font.drawString("Enter aloittaa");
		glColor4f(1,1,1,1);
	}

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);


	// Draw other stuff
	SDL_GL_SwapBuffers();
}

