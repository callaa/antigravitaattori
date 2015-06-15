#ifndef _GAME_H_
#define _GAME_H_

class Craft;
class Level;

struct PlayerStat {
	int plr;
	float time;
};

class Game {
public:
	~Game();

	static const int MAX_PLAYERS = 8;
	static const int MAX_LOCAL_PLAYERS = 4;
	static const int MAX_VIEWPORTS = MAX_LOCAL_PLAYERS;
	static const int NUM_CONTROLS = Craft::NUM_CONTROLS;

	static const int MAX_FPS = 100;
	static const int MIN_FPS = 40;
	static const unsigned int MIN_FRAME_TIME = 1000 / MAX_FPS;
	static const unsigned int MAX_FRAME_TIME = 1000 / MIN_FPS;
	static const bool FRAMESKIP = false;
	static const bool FRAME_LIMITER = false;
	
	static const char *PLAYER_TEXTURES[MAX_PLAYERS];
	static const float PLAYER_COLORS[MAX_PLAYERS][3];
	static const int CONTROLS[MAX_LOCAL_PLAYERS][NUM_CONTROLS];

	int init();

	int gameLoop();
	
	Level &getLevel();
	static Game &getInstance();
	
	void initViewports(int num);
	
	Player &getPlayer(int n);
	
	ALuint getSource();
	
private:
	static const int GLOBAL_SOURCES = 8;
	
	Game();
	
	void drawFrame();
	void drawViewport(int current, const GLint *viewport);
	void draw2d();
	void draw3d(const float *eye, const float *at, float fovDiag);
	void drawHud();
	void drawRadar(float width, float height, float alpha = 1.0);
	void drawStatistics(float width, float height);
	
	bool updateControls(int keysym, bool down);
	void updateWorld(float t);
	
	void resetListener();
	void updateListener();
	
	Player players[MAX_PLAYERS];
	Level level;
	
	m3dTexture playerTex[MAX_PLAYERS];
	
	PlayerStat playerFinishTime[MAX_PLAYERS];

	GLint viewports[MAX_VIEWPORTS][4];
	int numViewports;
	
	GLint masterViewport[4];
	int screenWidth, screenHeight;
	
	bool showFps;
	float fps, updateRate;
	
	ALuint playerSources[MAX_PLAYERS];
	ALuint globalSources[GLOBAL_SOURCES];
	
	float listenerWeight;
	float listenerPos[3];
	float listenerVel[3];
	float listenerOri[6];
	
	bool enable3d, enable2d;
	static Game instance;
	
	int activeplayers;

	enum {WAITFORSTART,START,GAME,FINISHED} state;
	float stateTimer;
	int stateVal;

    Background backg;
	
	static GLuint signal,signalred,signalgreen;
	static ALuint signalredbuffer,signalgreenbuffer;
};

#endif

