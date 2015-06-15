#include "SDL.h"
#include "SDL_opengl.h"

#include <AL/al.h>
#include <cmath>

#include "antigrav.h"

Game Game::instance;

const int Game::CONTROLS[MAX_LOCAL_PLAYERS][NUM_CONTROLS] = {
	{SDLK_RIGHT, SDLK_LEFT, SDLK_UP},
	{SDLK_d, SDLK_a, SDLK_w},
	{SDLK_l, SDLK_j, SDLK_i},
	{SDLK_KP6, SDLK_KP4, SDLK_KP8}};

const char *Game::PLAYER_TEXTURES[MAX_PLAYERS] = {"", "racer1.png", "racer2.png", "racer3.png", "racer4.png", "racer5.png", "racer6.png", "racer7.png"};
const float Game::PLAYER_COLORS[MAX_PLAYERS][3] = {{1,0,0},{0,0,1},{0,1,0},{1,1,0}, {0.65, 0, 1}, {0.20, 0.64, 0.69}, {0.89, 0.63, 0.18}, {0.59, 0.56, 0.88}};

GLuint Game::signal;
GLuint Game::signalred;
GLuint Game::signalgreen;
ALuint Game::signalredbuffer;
ALuint Game::signalgreenbuffer;

Game::Game()
{
	enable3d = true;
	enable2d = false;
	
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		players[i].setActive(false);
		players[i].setLocal(false);
	}
}

Game::~Game()
{
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(alIsSource(playerSources[i]))
		{
			alDeleteSources(1, &playerSources[i]);
		}
	}
	
	for(int i = 0; i < GLOBAL_SOURCES; i++)
	{
		if(alIsSource(globalSources[i]))
		{
			alDeleteSources(1, &globalSources[i]);
		}
	}
}

Game &Game::getInstance()
{
	return instance;
}

Level &Game::getLevel()
{
	return level;
}

int Game::init()
{
	// Get viewport
	glGetIntegerv(GL_VIEWPORT, masterViewport);
	screenWidth = masterViewport[2];
	screenHeight = masterViewport[3];

	// Load resources
	if(Craft::init() != 0) return 1;
	if(level.init() != 0) return 1;
	if(Font::getInstance().init() != 0) return 1;
	if(Player::init() != 0) return 1;
	if(Ring::init() != 0) return 1;
	if(Background::init() != 0) return 1;

	signal = m3dTexture::loadTexture("signal.png");
	if(signal==0)
		return -1;

	signalred = m3dTexture::loadTexture("signalred.png");
	if(signalred==0)
		return -1;

	signalgreen = m3dTexture::loadTexture("signalgreen.png");
	if(signalgreen==0)
		return -1;

/*	signalredbuffer = alutCreateBufferWaveform(ALUT_WAVEFORM_SINE, 200.0, 0.0, 0.4);
	signalgreenbuffer = alutCreateBufferWaveform(ALUT_WAVEFORM_SINE, 300.0, 0.0, 0.7);*/
	signalredbuffer = loadWavBuffer("signalred.wav");
	if(signalredbuffer == AL_NONE) return -1;
	signalgreenbuffer = loadWavBuffer("signalgreen.wav");
	if(signalgreenbuffer == AL_NONE) return -1;
	
	playerTex[0] = players[0].getCraft().getMesh().getTexture(0);
	for(int i = 1; i < MAX_PLAYERS; i++)
	{
		if(playerTex[i].load(PLAYER_TEXTURES[i]) != 0)
		{
			return -1;
		}
	}
	
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		players[i].setTexture(&playerTex[i]);
		players[i].setColor(PLAYER_COLORS[i]);
	}

	// set up player sources
	alGenSources(MAX_PLAYERS, playerSources);
	if(alGetError() != AL_NO_ERROR)
	{
		fprintf(stderr, "Can't initialize OpenAL sources\n");
		return -1;
	}

	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		players[i].setSource(playerSources[i]);
	}
	
	// set up global sources
	alGenSources(GLOBAL_SOURCES, globalSources);
	if(alGetError() != AL_NO_ERROR)
	{
		fprintf(stderr, "Can't initialize OpenAL sources\n");
		return -1;
	}
	
	// set up permanent OpenGL state
	// lighting
	const GLfloat lightPos[] = {0,10,0,1};
	const GLfloat lightAmb[] = {.5,.5,.5,1};
	const GLfloat lightDif[] = {1,1,1,1};

	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDif);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glEnable(GL_LIGHT0);
	
	// Depth func
	glDepthFunc(GL_LEQUAL);
	
	// Blend func
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	return 0;
}

static int sortStats(const void *s1, const void *s2) {
	float t1 = ((PlayerStat*)s1)->time;
	float t2 = ((PlayerStat*)s2)->time;
	if(t1<t2) return -1;
	if(t1>t2) return 1;
	return 0;
}

int Game::gameLoop()
{
	SDL_Event event;
	Uint32 startTime;
	float t;
	bool loop;
	
	// <temporary>
	// generate level
	level.generate(time(NULL));
	// </temporary>

	backg = Background();

	// set up starting grid
	int grid = 0;
	activeplayers = 0;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		playerFinishTime[i].plr = -1;
		if(!players[i].isActive()) continue;
		++activeplayers;
		
		float xPos = 15.0 - grid * 0.75;
		float yPos = 0.75 + (grid & 1) * 0.5;
		grid++;
		
		players[i].reset(xPos, level.getHeight(xPos) + yPos);

		playerFinishTime[i].plr = i;
		playerFinishTime[i].time = -1;
	}

	// Clear rings
	Ring::resetAll();

	// set up viewports
	int temp = 0;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(players[i].isActive() && players[i].isLocal()) temp++;
	}
	
	initViewports(temp);
	
	// set up audio distance model
	if(activeplayers > 1)
	{
		alDistanceModel(AL_NONE);
		Player::setEngineVolume(0.2);
		alDopplerFactor(0.0);
	} else
	{
		alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
		Player::setEngineVolume(1.0);
		alDopplerFactor(0.25);
	}
	
	// set up timer
	fps = 0.0;
	int numFrames = 0, skippedFrames = 0;
	Uint32 frameTime = 0, fpsTimer = 0;
	showFps = false;
	
	// start main loop
	state = WAITFORSTART;
	stateTimer = 0;
	loop = true;
	startTime = SDL_GetTicks();
	while(loop)
	{

		// Update timer and frame rate
		frameTime = SDL_GetTicks() - startTime;
		if(frameTime < MIN_FRAME_TIME && FRAME_LIMITER) { SDL_Delay(MIN_FRAME_TIME - frameTime); }
		frameTime = (SDL_GetTicks() - startTime);
		t = (frameTime) / 1000.0f;
		
		fpsTimer += frameTime;
		if(numFrames++ >= 5)
		{
			if(fpsTimer == 0) fpsTimer = 1;			// avoid division by zero on smoking fast machines
			fps = (float)(1000 * (numFrames - skippedFrames)) / fpsTimer;
			updateRate = (float)(1000 * numFrames) / fpsTimer;
			numFrames = 0;
			fpsTimer = 0;
			skippedFrames = 0;
		}
		
		startTime = SDL_GetTicks();

		// Handle events
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT) loop = false;
			else if(event.type == SDL_KEYDOWN)
			{
				if(event.key.keysym.sym == SDLK_ESCAPE) loop = false;
				else if(event.key.keysym.sym == SDLK_F9) showFps = !showFps;

				// update player controls
				updateControls(event.key.keysym.sym, true);
				if(state==WAITFORSTART) {
					state = START;
					stateTimer = 0;
					stateVal = 0;
				}

				// debug-mode "secret" keys
#ifdef DEBUG
				if(event.key.keysym.sym == SDLK_F10) m3dTexture::screenshot("antigrav-screenshot.png");
				else if(event.key.keysym.sym == SDLK_F11) enable3d = !enable3d;
				else if(event.key.keysym.sym == SDLK_F12) enable2d = !enable2d;
#endif
			} else if(event.type == SDL_KEYUP)
			{
				// handle player controls
				updateControls(event.key.keysym.sym, false);
			}
		}

		stateTimer += t;

		switch(state) {
			case WAITFORSTART:
				break;
			case START:
				if(stateTimer > 4.0) {
					state = GAME;
					stateTimer = 0;
				}
				break;
			case GAME: {
				// Check if all players have finished
				int finp=0;
				for(int p=0;p<8;p++)
					if(players[p].isActive() && players[p].isFinished())
						finp++;
				if(finp==activeplayers) {
					state = FINISHED;
					stateTimer = 0;
					qsort(playerFinishTime,MAX_PLAYERS,sizeof(PlayerStat),
							sortStats);
				}
				// Update
				updateWorld(t);
				} break;

			case FINISHED:
				// Update
				updateWorld(t);
				if(stateTimer>=5.0)
					loop = false;
		}

		// skip frames to maintain solid frame rate
		if(frameTime > MAX_FRAME_TIME && FRAMESKIP)
		{
			skippedFrames++;
			continue;
		}

		// Draw (all states)
		resetListener();
		drawFrame();
		updateListener();
	}
	
	// Kill all audio
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		ALint state;
		alGetSourcei(playerSources[i], AL_SOURCE_STATE, &state);
		if(state == AL_PLAYING) alSourceStop(playerSources[i]);
	}
	
	for(int i = 0; i < GLOBAL_SOURCES; i++)
	{
		ALint state;
		alGetSourcei(globalSources[i], AL_SOURCE_STATE, &state);
		if(state == AL_PLAYING) alSourceStop(globalSources[i]);
	}

	return state!=FINISHED;
}

// returns true if key handled, false if not
bool Game::updateControls(int keysym, bool down)
{
	int pl = -1;
	int ctrl = -1;
	if(state!=GAME) return false;

	for(int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		for(int j = 0; j < NUM_CONTROLS; j++)
		{
			if(keysym == CONTROLS[i][j])
			{
				pl = i;
				ctrl = j;
				break;
			}
						
			if(pl != -1) break;
		}
	}

	if(pl == -1 || ctrl == -1) return false;
	
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(!players[i].isLocal()) continue;
		if(pl == 0)
		{
			if(!players[i].isFinished())
				players[i].getCraft().setControl(ctrl, down);
			else
				players[i].getCraft().setControl(ctrl,false);
			return true;
		}
		
		pl--;
	}
	
	return false;
}

void Game::updateWorld(float t)
{
	// update crafts
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(!players[i].isActive()) continue;
		if(players[i].update(t)) {
			// Player finished
			playerFinishTime[i].time = stateTimer;
		}
	}

	// Update rings
	Ring::updateAll(t);

	// handle craft to craft collisions
	for(int i = 0; i < MAX_PLAYERS - 1; i++)
	{
		if(!players[i].isActive()) continue;
		Craft &craft1 = players[i].getCraft();

		for(int j = i + 1; j < MAX_PLAYERS; j++)
		{
			if(!players[j].isActive()) continue;
			Craft &craft2 = players[j].getCraft();

			if(!craft1.collide(craft2)) craft2.collide(craft1);
		}
	}

	// handle craft to level collisions and move crafts
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(!players[i].isActive()) continue;

		players[i].getCraft().levelCollide();
		players[i].getCraft().move();
	}

}

void Game::initViewports(int num)
{
	numViewports = num;
	if(numViewports > MAX_VIEWPORTS) numViewports = MAX_VIEWPORTS;
	
	if(numViewports == 1)
	{
		viewports[0][2] = screenWidth;
		viewports[0][3] = screenWidth / 16 * 9;
		viewports[0][0] = 0;
		viewports[0][1] = (screenHeight - viewports[0][3]) / 2;
		
		for(int i = 0; i < 4; i++) masterViewport[i] = viewports[0][i];
		return;
	} else if(numViewports == 2)
	{
		for(int i = 0; i < 2; i++)
		{
			viewports[i][0] = 0;
			viewports[i][1] = (1 - i) * screenHeight / 2;
			viewports[i][2] = screenWidth;
			viewports[i][3] = screenHeight / 2;
		}
	} else
	{
		for(int i = 0; i < 4; i++)
		{
			viewports[i][0] = (i & 1) * screenWidth / 2;
			viewports[i][1] = (1 - i/2) * screenHeight / 2;
			viewports[i][2] = screenWidth / 2;
			viewports[i][3] = screenHeight / 2;
		}
	}
	
	masterViewport[0] = 0;
	masterViewport[1] = 0;
	masterViewport[2] = screenWidth;
	masterViewport[3] = screenHeight;
}

void Game::drawFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	int vp = numViewports - 1;
	for(int i = MAX_PLAYERS-1; i >= 0; i--)
	{
		if(!players[i].isActive() || !players[i].isLocal()) continue;
			
		drawViewport(i, viewports[vp--]);
		if(vp < 0) break;
	}
		
	drawHud();
		
	SDL_GL_SwapBuffers();
}

void Game::draw3d(const float *eye, const float *at, float fovDiag)
{

	glDisable(GL_LIGHTING);
	backg.draw();

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	
	level.draw3d(eye, at, fovDiag);
	
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(!players[i].isActive()) continue;

		players[i].getCraft().getMesh().setTexture(0, players[i].getTexture());
		players[i].getCraft().draw3d();
	}

	Ring::drawAll();
}

void Game::draw2d()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	
	level.draw2d();
	
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(!players[i].isActive()) continue;

		players[i].getCraft().draw2d();
	}
}

void Game::drawViewport(int current, const GLint *viewport)
{
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// set up projection and calculate diagoonal fov
	const double fovY = RAD(45.0);
	const double aspect = (GLdouble)viewport[2]/viewport[3];
	gluPerspective(DEG(fovY), aspect, 0.1, 200.0);
	
	double tfovY2 = tan(fovY / 2.0);
	double tfovX2 = tfovY2 * aspect;
	
	float fovDiag = (float)atan(sqrt( tfovX2 * tfovX2 + tfovY2 * tfovY2 ));
	
	// Set up transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	Craft &craft = players[current].getCraft();
	
	// Set camera position
	float eyeX, eyeY, eyeZ;
	float centerX, centerY, centerZ;
	eyeX = craft.getX();
	if(eyeX < 10.0) eyeX = 10.0;
	if(eyeX > level.getWidth() - 10.0) eyeX = level.getWidth() - 10.0;
	
	eyeY = 6.0;
	eyeZ = 7.0;
	
	centerX = eyeX;
	centerY = 4.0;
	centerZ = 0.0;
	
	gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, centerX - eyeX, eyeZ - centerZ, centerY - eyeY);
	
	// Update listener values
	listenerPos[0] += eyeX;
	listenerPos[1] += eyeY;
	listenerPos[2] += eyeZ;
	
	listenerVel[0] += craft.getVX();
	
	listenerOri[0] += centerX - eyeX;
	listenerOri[1] += centerY - eyeY;
	listenerOri[2] += centerZ - eyeZ;
	
	listenerOri[3] += centerX - eyeX;
	listenerOri[4] += eyeZ - centerZ;
	listenerOri[5] += centerY - eyeY;
	
	listenerWeight += 1.0;

	// Draw
	const float at[3] = {centerX - eyeX, centerY - eyeY, centerZ - eyeZ};
	const float eye[3] = {eyeX, eyeY, eyeZ};
	
	if(enable3d) draw3d(eye, at, fovDiag);
	if(enable2d) draw2d();
	
	players[current].drawHud(viewport, activeplayers, current);
}

void Game::drawHud()
{
	glViewport(masterViewport[0], masterViewport[1], masterViewport[2], masterViewport[3]);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	float width, height;
	width = masterViewport[2] / 8.0;
	height = width * masterViewport[3] / masterViewport[2];
	glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	
	// Draw signal lights
	if(state == START || (state==GAME&&stateTimer<1.0)) {
		float y=0.0, w=0.0;

		if(state==START) {
			glBindTexture(GL_TEXTURE_2D, signal);
			if(stateTimer < 1.0) {
				y = -10+stateTimer*10;
			} else {
				w = int(stateTimer)/3.0;
				if(stateVal != int(stateTimer)) {
					stateVal = int(stateTimer);
					ALuint source = getSource();
					if(alIsSource(source)) {
						float vec[3] = {0,0,0};
						alSourcefv(source, AL_VELOCITY, vec);
						float pos[3] = {listenerPos[0],listenerPos[1],listenerPos[2]-1};
						alSourcefv(source, AL_POSITION, pos);
						alSourcei(source, AL_LOOPING, AL_FALSE);
						alSourcei(source, AL_BUFFER, signalredbuffer);
						alSourcePlay(source);
					}
				}
			}
		} else {
			glBindTexture(GL_TEXTURE_2D, signalgreen);
			y = stateTimer*-10.0;
			if(stateVal!=-1) {
				stateVal=-1;
				ALuint source = getSource();
				if(alIsSource(source)) {
					float vec[3] = {0,0,0};
					alSourcefv(source, AL_VELOCITY, vec);
					float pos[3] = {listenerPos[0],listenerPos[1],listenerPos[2]-1};
					alSourcefv(source, AL_POSITION, pos);
					alSourcei(source, AL_LOOPING, AL_FALSE);
					alSourcei(source, AL_BUFFER, signalgreenbuffer);
					alSourcePlay(source);
				}
			}
		}

		// Unlit or green signals
		glPushMatrix();
		glTranslatef(width/2.0 - 10.0,y,0);
		glScalef(10,10,1);
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(w, 0.0);
		glVertex2f(2*w, 0);

		glTexCoord2f(1, 0.0);
		glVertex2f(2, 0);

		glTexCoord2f(w, 1.0);
		glVertex2f(2*w, 1);

		glTexCoord2f(1, 1.0);
		glVertex2f(2, 1);
		glEnd();

		if(w>0.0) {
			// Red signals
			glBindTexture(GL_TEXTURE_2D, signalred);
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0, 0.0);
			glVertex2f(0, 0);

			glTexCoord2f(w, 0.0);
			glVertex2f(2*w, 0);

			glTexCoord2f(0, 1.0);
			glVertex2f(0, 1);

			glTexCoord2f(w, 1.0);
			glVertex2f(2*w, 1);
			glEnd();
		}
		glPopMatrix();
	}

	// draw radar
	if(state == GAME && stateTimer > 1.0)
	{
		float alpha = (stateTimer - 1.0);
		if(alpha > 1.0) alpha = 1.0;
		drawRadar(width, height, alpha);
	}
	
	// draw fps counter
	if(showFps)
	{
		glPushMatrix();
		glTranslatef(width - 10.0, 0.0, 0.0);
		Font::getInstance().printf(" fps: %d\nrate: %d",
				(int)fps, (int)updateRate);
		glPopMatrix();
	}
	
	// Draw end game statistics
	if(state == FINISHED)
		drawStatistics(width,height);

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

void Game::drawRadar(float width, float height, float alpha)
{
	(void)height;
	
	// draw radar
	glPushMatrix();
	glDisable(GL_TEXTURE_2D);
	
	const float maxHeight = 10.0;
	glTranslatef(width/6.0, 4.5, 0.0);
	glScalef((2.0/3.0 * width) / level.getWidth(), -4.0 / maxHeight, 1.0);
	
	glColor4f(0.3, 0.3, 0.3, alpha);
	glBegin(GL_TRIANGLE_STRIP);	// gray background
	glVertex2f(0.0, 0.0);
	glVertex2f(level.getWidth(), 0.0);
	glVertex2f(0.0, maxHeight);
	glVertex2f(level.getWidth(), maxHeight);
	glEnd();
	
	glColor4f(1.0, 1.0, 1.0, alpha); // white frame
	glBegin(GL_LINE_LOOP);
	glVertex2f(0.0, 0.0);
	glVertex2f(level.getWidth(), 0.0);
	glVertex2f(level.getWidth(), maxHeight);
	glVertex2f(0.0, maxHeight);
	glEnd();
	
	level.drawRadar();
	
	glPointSize(5.0);
	glBegin(GL_POINTS);
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(!players[i].isActive()) continue;
		
		float x = players[i].getCraft().getX();
		float y = players[i].getCraft().getY();
		if(x>0 && x<level.getWidth() && y>0 && y<maxHeight) {
			players[i].bindColor(alpha);
			glVertex2fv(players[i].getCraft().getPos().getData());
		}
	}
	glEnd();
	
	glEnable(GL_TEXTURE_2D);
	glPopMatrix();
	
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

void Game::drawStatistics(float width, float height)
{
	glPushMatrix();
	glDisable(GL_TEXTURE_2D);

	float boxw = width * (2.0/3.0);
	float boxh = height * (2.0/3.0);
	
	glTranslatef(width/2.0-boxw/2.0, height/2.0-boxh/2.0, 0.0);
	
	glColor4f(0.3, 0.3, 0.3, 0.5);
	glBegin(GL_TRIANGLE_STRIP);	// gray background
	glVertex2f(0.0, 0.0);
	glVertex2f(boxw, 0.0);
	glVertex2f(0.0, boxh);
	glVertex2f(boxw, boxh);
	glEnd();
	
	glColor4f(1.0, 1.0, 1.0, 0.5); // white frame
	glBegin(GL_LINE_LOOP);
	glVertex2f(0.0, 0.0);
	glVertex2f(boxw, 0.0);
	glVertex2f(boxw, boxh);
	glVertex2f(0.0, boxh);
	glEnd();
	
	glEnable(GL_TEXTURE_2D);

	Font &font = Font::getInstance();
	glColor4f(1,1,1,1);

	glPushMatrix();
	glTranslatef(boxw/2-9,1,0);
	
	glScalef(2,2,1);
	font.drawString("Kilpa ohi");
	glPopMatrix();

	glTranslatef(10,8,0);

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	glVertex2f(0,1.5);
	glVertex2f(boxw-12,1.5);
	glVertex2f(38,0);
	glVertex2f(38,boxh-10);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	font.drawString("Pelaaja");
	glTranslatef(40,0,0);
	font.drawString("Aika");
	glTranslatef(-40,4,0);

	for(int p=0;p<MAX_PLAYERS;p++) {
		if(playerFinishTime[p].plr!=-1) {
			font.drawString(getPlayer(playerFinishTime[p].plr).getName());
			glTranslatef(40,0,0);
			font.printf("%.3f sek.",playerFinishTime[p].time);
			glTranslatef(-40,2,0);
		}
	}

	glPopMatrix();
	
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

Player &Game::getPlayer(int n)
{
	return players[n];
}

void Game::resetListener()
{
	listenerWeight = 0.0;
	
	for(int i = 0; i < 3; i++)
	{
		listenerPos[i] = 0.0;
		listenerVel[i] = 0.0;
		listenerOri[i] = 0.0;
		listenerOri[i + 3] = 0.0;
	}
}

void Game::updateListener()
{
	// Division by zero sanity check
	if(listenerWeight == 0.0) return;
	
	for(int i = 0; i < 3; i++)
	{
		listenerPos[i] /= listenerWeight;
		listenerVel[i] /= listenerWeight;
		listenerOri[i] /= listenerWeight;
		listenerOri[i + 3] /= listenerWeight;
	}
	
	// Update OpenAL listener
	alListenerfv(AL_POSITION, listenerPos);
	alListenerfv(AL_VELOCITY, listenerVel);
	alListenerfv(AL_ORIENTATION, listenerOri);
}

ALuint Game::getSource()
{
	for(int i = 0; i < GLOBAL_SOURCES; i++)
	{
		ALint state;
		alGetSourcei(globalSources[i], AL_SOURCE_STATE, &state);
		if(state != AL_PLAYING) return globalSources[i];
	}
	
	return 0;
}

