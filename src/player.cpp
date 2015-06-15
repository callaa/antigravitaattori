#include "SDL_opengl.h"

#include <AL/al.h>

#include <cstring>
#include <cmath>
#include "antigrav.h"

GLuint Player::gauges;
GLuint Player::needle;
GLuint Player::fuel;

ALuint Player::buffer;

float Player::engineVolume = 1.0;

Player::Player()
	: finished(false)
{
}

int Player::init()
{
	if(!gauges) {
		gauges = m3dTexture::loadTexture("gauges.png");
			if(gauges==0)
				return -1;
	}
	if(!needle) {
		needle = m3dTexture::loadTexture("needle.png");
			if(needle==0)
				return -1;
	}
	if(!fuel) {
		fuel = m3dTexture::loadTexture("fuel.png");
			if(fuel==0)
				return -1;
	}

	// <temporary>
// 	buffer = alutCreateBufferWaveform(ALUT_WAVEFORM_SAWTOOTH, 100.0, 0.0, 1.0);
// 	buffer = alutCreateBufferFromFile("hover.wav");
	buffer = loadWavBuffer("hover.wav");
	if(buffer == AL_NONE) return -1;
	// </temporary>
	
	return 0;
}

void Player::reset(float craftx, float crafty)
{
    craft.setPos(Vector2(craftx,crafty));
    craft.setVel(Vector2(0,0));
    craft.setAngle(0);
    craft.setOmega(0);
    finished = false;
}

void Player::setName(const char *n)
{
	strncpy(name,n,sizeof name);
}

void Player::setTexture(m3dTexture *tex)
{
	texture = tex;
}

void Player::setActive(bool a)
{
	active = a;
}

void Player::setLocal(bool l)
{
	local = l;
}

Craft &Player::getCraft() { return craft; }
bool Player::isActive() const { return active; }
bool Player::isLocal() const { return local; }
const char *Player::getName() const { return name; }

bool Player::isFinished() const { return finished; }

bool Player::update(float dt)
{
    bool rval = false;
	craft.update(dt);
	
	if(craft.getX() >= Terrain::FINISH_LINE * Terrain::VERTEX_DIST && finished==false)
	{
		finished = true;
        rval = true;
	}
	
	// update force meter
	const float maxForceRise = 1.0;
	const float maxForceFade = 0.5;
	
	float force = craft.getHoverForce() / 5.0;
	if(force>1.0) force=1.0;
	
	if(force > forceMeter)
	{
		if(force - forceMeter < maxForceRise * dt) forceMeter = force;
		else forceMeter += maxForceRise * dt;
	} else
	{
		if(forceMeter - force < maxForceFade * dt) forceMeter = force;
		else forceMeter -= maxForceFade * dt;
	}
	

	// update sound source
	ALint state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	if(state != AL_PLAYING) alSourcePlay(source);
	
	float vec[3];

	vec[0] = craft.getX();
	vec[1] = craft.getY();
	vec[2] = 0.0;
	alSourcefv(source, AL_POSITION, vec);
	
	vec[0] = craft.getVX();
	vec[1] = craft.getVY();
	vec[2] = 0.0;
	alSourcefv(source, AL_VELOCITY, vec);
	
	const float minGain = 0.5;
	const float maxGain = 3.0;
	
	alSourcef(source, AL_GAIN, (minGain + forceMeter * (maxGain - minGain)) * engineVolume);

	const float pitchChange = 1.5;
	float speed = craft.getSpeed() / 7.0;
	if(speed > 1.0) speed = 1.0;
	
	alSourcef(source, AL_PITCH, 1.0 + (speed - 0.5) * pitchChange);

    return rval;
}

static void drawRect(float width=1.0,float height=1.0)
{
	float w2 = width/2.0;
	float h2 = height/2.0;
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(-w2, -h2);

	glTexCoord2f(1.0, 0.0);
	glVertex2f(w2, -h2);

	glTexCoord2f(0.0, 1.0);
	glVertex2f(-w2, h2);

	glTexCoord2f(1.0, 1.0);
	glVertex2f(w2, h2);
	glEnd();
}

void Player::drawHud(const GLint *viewport, int activePlayers, int num)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	float width, height;
	width = viewport[2] / 8.0;
	height = width * viewport[3] / viewport[2];
	glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	
    glColor3f(1,1,1);
	// Draw gauges
	if(activePlayers < 3 || (num % 2) != 0)
	{
		glTranslatef(width - 20.0, 0.0, 0.0);
	}


	glTranslatef(16,8,0);
	glScalef(15.0, 15.0, 1.0);
	glBindTexture(GL_TEXTURE_2D,  gauges);
	drawRect(2.0);

	// Draw speed gauge needle
	glBindTexture(GL_TEXTURE_2D,needle);
	float speed = craft.getSpeed() / 5.0;
	if(speed>1.0) speed = 1.0;
	glPushMatrix();
	glTranslatef(-0.5,0,0);
	glScalef(0.8,0.8,1.0);
	glRotatef((speed - 0.5) * (150.0*2), 0, 0, 1);
	drawRect();
	glPopMatrix();

	// Draw force gauge needle
	glPushMatrix();

	glTranslatef(-0.10,-0.25,0);
	glScalef(0.4,0.4,1);
	glRotatef(50 + (forceMeter - 0.5) * (90.0*2), 0, 0, 1);
	drawRect();
	glPopMatrix();

	// Draw fuel gauge bars (-55 <-> 100, 22 degree increments)
	glTranslatef(-0.5,0,0);
	float bars = -55 + craft.getBoostFuel()*(100+55);
	if(bars > -55+22) {
		glBindTexture(GL_TEXTURE_2D,fuel);
		glBegin(GL_TRIANGLE_FAN);
		glTexCoord2f(0.25,0.5);
		glVertex3f(0,0,0);
		for(float b=-55;b<bars;b+=22) {
			float c = cos(b / 180.0 * M_PI);
			float s = sin(b / 180.0 * M_PI);
			glTexCoord2f(0.25-c/4.0, 0.5-s/2.0);
			glVertex3f(c/-2.0,s/-2.0,0);
		}
		glEnd();
	}

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

const m3dTexture &Player::getTexture() const
{
	return *texture;
}

void Player::setColor(float r, float g, float b)
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
    craft.setColor(r,g,b);
}

void Player::setColor(const float *col)
{
	for(int i = 0; i < 3; i++)
	{
		color[i] = col[i];
	}
    craft.setColor(col[0],col[1],col[2]);
}

void Player::setSource(ALuint src)
{
	source = src;
	
	alSourcei(source, AL_LOOPING, AL_TRUE);
	alSourcei(source, AL_BUFFER, buffer);
}

const float *Player::getColor() const { return color; }
void Player::bindColor() const { glColor3fv(color); }

void Player::bindColor(float alpha) const
{
	glColor4f(color[0], color[1], color[2], alpha);
}

ALuint Player::getSource() const { return source; }

void Player::setEngineVolume(float vol) { engineVolume = vol; }
