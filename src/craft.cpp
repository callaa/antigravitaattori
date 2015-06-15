#include "SDL_opengl.h"
#include <cmath>
#include <cstdio>

#include <AL/al.h>

#include "antigrav.h"

CraftState::CraftState() : pos(0.0, 0.0), angle(0.0) { }

CraftState::CraftState(const Vector2& p, float a)
{
	pos = p;
	angle = a;
}

const Vector2& CraftState::getPos() const { return pos; }
float CraftState::getAngle() const { return angle; }

CraftState operator+(const CraftState& s1, const CraftState& s2)
{
	return CraftState(s1.getPos() + s2.getPos(), s1.getAngle() + s2.getAngle());
}

CraftState operator*(float f, const CraftState& s2)
{
	return CraftState(f * s2.getPos(), f * s2.getAngle());
}

void CraftState::setPos(const Vector2 &p) { pos = p; }
void CraftState::setAngle(float a) { angle = a; }

float CraftState::getX() const { return pos.getX(); }
float CraftState::getY() const { return pos.getY(); }


const float Craft::TURN_RATE = 3.14;
const float Craft::TURN_DAMP = 1.0;
const float Craft::TURN_BACK = 2.0;

const float Craft::GRAVITY = -2.0;
const float Craft::HOVER_FORCE = 2.0;
const float Craft::BOOST = 2.0;
const float Craft::DAMP = 0.25;

const float Craft::HEIGHT = 0.15;
const float Craft::WIDTH = 0.5;
const float Craft::MASS = 1.0;
const float Craft::INERTIA = 1.0 / 12.0 * MASS * (HEIGHT * HEIGHT + WIDTH * WIDTH);

const float Craft::INVERTED_FORCE_MOD = 0.2;

const float Craft::BOOST_REFUEL = 0.3;
const float Craft::BOOST_FUEL_USE = 1.0;

const float Craft::MAJOR_AXIS = 0.25;
const float Craft::MINOR_AXIS = 0.075;

const float Craft::BOUNCYNESS = 0.5;
const float Craft::LEVEL_BOUNCYNESS = 0.5;

m3dMesh Craft::mesh;

Craft::Craft()
{
	// rectangle shaped bounds
	vertices[0] = Vector2(-.5 * WIDTH, -.5 * HEIGHT);
	vertices[1] = Vector2(.5 * WIDTH, -.5 * HEIGHT);
	vertices[2] = Vector2(.5 * WIDTH, .5 * HEIGHT);
	vertices[3] = Vector2(-.5 * WIDTH, .5 * HEIGHT);
	
	// diamond shaped bounds
/*	vertices[0] = Vector2(0.0, -.5 * HEIGHT);
	vertices[1] = Vector2(.5 * WIDTH, 0.0);
	vertices[2] = Vector2(0.0, .5 * HEIGHT);
	vertices[3] = Vector2(-.5 * WIDTH, 0.0);*/
	
	majorAxis = MAJOR_AXIS;
	minorAxis = MINOR_AXIS;
	
	boostFuel = 1.0;
    ringTimer = 0;
}

template <class T> void Craft::integrateRKN(float x0, T y0, T dy0, float dx, T *y, T *dy)
{
	T k1, k2, k3, k4;
	T l;
	
	k1 = 0.5 * dx * derive(x0, y0, dy0);
	l = 0.5 * dx * (dy0 + 0.5 * k1);
	k2 = 0.5 * dx * derive(x0 + 0.5 * dx, y0 + l, dy0 + k1);
	k3 = 0.5 * dx * derive(x0 + 0.5 * dx, y0 + l, dy0 + k2);
	l = dx * (dy0 + k3);
	k4 = 0.5 * dx * derive(x0 + dx, y0 + l, dy0 + 2.0 * k3);
	
	*y = dx * (dy0 + (1.0/3.0) * (k1 + k2 + k3));
	*dy = dy0 + (1.0/3.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
}

CraftState Craft::derive(float t, const CraftState &s, const CraftState &ds)
{
	CraftState result;
	
	(void)t;
	
	beam[0] = Vector2(getPos());
	beam[1] = Vector2(s.getPos().getY() * tan(s.getAngle()), -s.getPos().getY());
	beam[0] = beam[0] + beam[1].unitVector() * 0.55 * HEIGHT;
	beam[1] = beam[1] + beam[0] -  beam[1].unitVector() * 0.55 * HEIGHT;
	
	Vector2 temp;
	if(Game::getInstance().getLevel().intersect(beam[0], beam[1], temp))
	{
		beam[1] = temp;
	}
	
	bool inverted = false;
	if(s.getAngle() < -M_PI/2.0 || s.getAngle() > M_PI/2.0) inverted = true;
	
	Vector2 l = beam[1] - beam[0];
	float d = l.length();
	
	int numCraft = -1;
	for(int i = 0; i < Game::MAX_PLAYERS; i++)
	{
		Craft &craft = Game::getInstance().getPlayer(i).getCraft();
		Vector2 point;
		
		if(craft.beamIntersect(beam[0], beam[1], point))
		{
			float clen = (point - beam[0]).length();
			if(clen < d)
			{
				beam[1] = point;
				d = clen;
				numCraft = i;
			}
		}
	}
	
	l = beam[1] - beam[0];
	d = exp(-d + 1) * HOVER_FORCE;
	
	if(inverted) d *= INVERTED_FORCE_MOD;
	
	// Boost
	if(ctrl[CTRL_BOOST] && boostFuel > 0.0 && !inverted) d *= BOOST;
	
	hoverForce = d;
	
	Vector2 fce = l.unitVector() * d;
	if(numCraft != -1) Game::getInstance().getPlayer(numCraft).getCraft().addForce(fce);

	// set velocity vector
	result.setPos(Vector2(0.0, GRAVITY) - l.unitVector() * d - DAMP * ds.getPos() + force);
	force = Vector2(0.0,0.0);
	
	// Turning
	if(ctrl[CTRL_CCW]) result.setAngle(TURN_RATE);
	else if(ctrl[CTRL_CW]) result.setAngle(-TURN_RATE);
	else result.setAngle(-TURN_BACK * s.getAngle() - TURN_DAMP * ds.getAngle());
	
	return result;
}

void Craft::update(float dt)
{
    bool boost = false;
	// boost
	if(ctrl[CTRL_BOOST])
	{
		boostFuel -= BOOST_FUEL_USE * dt;
		if(boostFuel < 0.0)
            boostFuel = 0.0;
        else
            boost = true;
	} else
	{
		boostFuel += BOOST_REFUEL * dt;
		if(boostFuel > 1.0) boostFuel = 1.0;
	}

    ringTimer += dt;
    if(ringTimer > ((boost)?0.20:0.4)) {
        if(getAngle() > -M_PI/4.0 && getAngle() < M_PI/4.0 && (beam[1]-beam[0]).length()<3.0) {
            Vector2 vel((beam[1]-beam[0]).unitVector()*2);
            Ring::addRing(Ring(getX(),getY(),getAngle(),vel+getVel(),color));
            ringTimer = 0;
        }
    }

	integrateRKN<CraftState>(0.0f, state, dState, dt, &delta, &dState);
}

void Craft::move()
{
	state = state + delta;
	
	while(state.getAngle() > M_PI) state.setAngle(state.getAngle() - 2.0 * M_PI);
	while(state.getAngle() < -M_PI) state.setAngle(state.getAngle() + 2.0 * M_PI);
}

void Craft::setControl(int control, bool value)
{
	ctrl[control] = value;
}

void Craft::setColor(float r, float g, float b)
{
    color[0] = r;
    color[1] = g;
    color[2] = b;
}

void Craft::draw3d()
{
	glPushMatrix();
	
	glTranslatef(state.getPos().getX(), state.getPos().getY(), 0.0);
	glRotatef(DEG(state.getAngle()), 0.0, 0.0, 1.0);
	
	// draw 3d mesh
	glEnable(GL_TEXTURE_2D);
	mesh.draw();
	glDisable(GL_TEXTURE_2D);
	
	glPopMatrix();
	
#if 0
	// draw beam
	if(state.getAngle() > -M_PI / 2.0 && state.getAngle() < M_PI / 2.0)
	{
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glVertex2fv(beam[0].getData());
		glVertex2fv(beam[1].getData());
		glEnd();
		glEnable(GL_LIGHTING);
	}
#endif
}

void Craft::draw2d()
{
	glPushMatrix();
	
	glTranslatef(state.getPos().getX(), state.getPos().getY(), 0.0);
	glRotatef(DEG(state.getAngle()), 0.0, 0.0, 1.0);
	
	// draw bounding ellipse
	glBegin(GL_LINE_LOOP);
	for(int i = 0; i < 16; i++)
	{
		glVertex2f(cos((float) i / 16.0 * 2.0 * M_PI) * majorAxis, sin((float) i / 16.0 * 2.0 * M_PI) * minorAxis);
	}
	glEnd();
	
	// draw bounding box
	glBegin(GL_LINE_LOOP);
	for(int i = 0; i < 4; i++)
	{
		glVertex2fv(vertices[i].getData());
	}
	glEnd();
	
	glPopMatrix();

	// draw beam
	glBegin(GL_LINES);
	glVertex2fv(beam[0].getData());
	glVertex2fv(beam[1].getData());
	glEnd();
}

bool Craft::collide(Craft &other)
{
	Vector2 normal, point;
	
	if(!checkCollision(other, point, normal)) return false;
	handleCollision(other, point, normal);
	
	return true;
}

bool Craft::checkCollision(const Craft &other, Vector2 &point, Vector2 &normal)
{
	Vector2 myV[4];	// my vertices
	Vector2 otV[4]; // other's vertices
	int i;
	
	for(i = 0; i < 4; i++)
	{
		myV[i] = 0.99 * getVertex(i);
		myV[i].rotate(getAngle() + getDAngle());
		myV[i] = myV[i] + getPos() + getDPos();
		
		otV[i] = other.getVertex(i);
		otV[i].rotate(other.getAngle() + other.getDAngle());
		otV[i] = otV[i] + other.getPos() + other.getDPos();
	}
	
	Vector2 points[2];
	int num = 0;
	for(i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			// check for contact
			if(!Level::segmentIsect(myV[i], myV[(i+1)%4], otV[j], otV[(j+1)%4], points[num])) continue;
			
			num++;
			if(num < 2) continue;	// we need 2 contact points

			// collision has occured
			
			// contact point
			point = 0.5 * (points[0] + points[1]);
			
			// normal vector
			normal = (points[1] - points[0]).normalVector();
			normal.normalize();
			
			// check if normal is facing away from this object
			if((point-getPos()).unitVector() * normal > 0.0)
			{
				// it is, flip it!
				normal = -normal;
			}

			return true;
		}
	}
	
	return false;
}
void Craft::addImpulse(const Vector2 &impulse, const Vector2 &point)
{
	// Limit impulses to 5.0 units
	Vector2 imp = impulse;
	if(imp.length() > 5.0) imp = imp.unitVector() * 5.0;
	
	// Add impulse	
	dState.setPos(dState.getPos() + imp / MASS);
	dState.setAngle(dState.getAngle() + ((point - getPos()) ^ imp) / INERTIA);
	
	// reset delta
	delta.setPos(Vector2(0.0, 0.0));
	delta.setAngle(0.0);
}

void Craft::handleCollision(Craft &other, const Vector2& point, const Vector2& normal)
{
	Vector2 v1, v2;
	
	v1 = getVel() + (point - (getPos() + getDPos())).rotate(getOmega());
	v2 = other.getVel() + (point - (other.getPos() + other.getDPos())).rotate(other.getOmega());
	
	Vector2 impulse = getImpulse(other, BOUNCYNESS, v1 - v2, point, normal);
	
	if((v1 - v2) * impulse < 0.0)
	{
		addImpulse(impulse, point);
		other.addImpulse(-impulse, point);
	} else
	{
	}
	
	// move the crafts away from each other
	v1 = getVel();
	v1.normalize();
	if((other.getPos() - getPos()).unitVector() * v1 > 0.0) v1 = -v1;
	setPos(getPos() + v1 * 0.03);
	
	v1 = other.getVel();
	v1.normalize();
	if((getPos() - other.getPos()).unitVector() * v1 > 0.0) v1 = -v1;
	other.setPos(other.getPos() + v1 * 0.03);
	
	// rotate the crafts away from each other
	if(((point - getPos()).rotate(getOmega())) * ((point - other.getPos()).rotate(other.getOmega())) > 0.0)
	{
		if(getOmega() > 0.0) setAngle(getAngle() - 0.05);
		else  setAngle(getAngle() + 0.05);
		
		if(other.getOmega() > 0.0) other.setAngle(other.getAngle() - 0.05);
		else  other.setAngle(other.getAngle() + 0.05);
	} else
	{
		if(getOmega() > 0.0) setAngle(getAngle() + 0.05);
		else  setAngle(getAngle() - 0.05);
		
		if(other.getOmega() > 0.0) other.setAngle(other.getAngle() + 0.05);
		else  other.setAngle(other.getAngle() - 0.05);
	}
}

Vector2 Craft::getImpulse(Craft &other, float e, const Vector2 &v, const Vector2 &point, const Vector2 &normal)
{
	float c1, c2;
	float a;
	
	a = -(1.0 + e) * (v * normal);
	
	c1 = (point - (getPos() + getDPos())) ^ normal;
	c1 *= c1;
	c2 = (point - (other.getPos() + other.getDPos()) ) ^ normal;
	c2 *= c2;
	
	a /= (1.0 / MASS) + (1.0 / other.MASS) + (c1 / INERTIA) + (c2 / other.INERTIA);
	
	return a * normal;
}

bool Craft::levelCollide()
{
	Vector2 point, normal, delta;
	if(!checkLevelCollision(point, normal, delta)) return false;
	
	setPos(getPos() - 1.1 * delta);
	
	
	glBegin(GL_POINTS);
	glVertex2fv(point.getData());
	glEnd();
	
	glColor3f(1,0,0);
	glBegin(GL_LINES);
	glVertex2fv(point.getData());
	glVertex2fv((point + normal).getData());
	glEnd();
	glColor3f(1,1,1);
	
	
	Vector2 px = point - (getPos() + getDPos());
	Vector2 v = getVel() + px.rotate(getOmega());
	
	float c1;
	float a;
	
	a = -(1.0 + LEVEL_BOUNCYNESS) * (v * normal);
	c1 = px ^ normal;
	c1 *= c1;
	a /= (1.0 / MASS) + (c1 / INERTIA);
	
	addImpulse(a * normal, point);
	
	return true;
}

bool Craft::checkLevelCollision(Vector2 &point, Vector2 &normal, Vector2 &delta)
{
	Level &level = Game::getInstance().getLevel();
	
	return level.ellipseIntersect((getPos() + getDPos()), (getAngle() + getDAngle()), majorAxis, minorAxis, point, normal, delta);
}

int Craft::init()
{
	return mesh.loadFromXML("racer.xml");
}

void Craft::setPos(const Vector2 &p) { state.setPos(p); }
void Craft::setVel(const Vector2 &v) { dState.setPos(v); }
void Craft::setAngle(float a) { state.setAngle(a); }
void Craft::setOmega(float a) { dState.setAngle(a); }

float Craft::getX() const { return state.getX(); }
float Craft::getY() const { return state.getY(); }
const Vector2 &Craft::getPos() const { return state.getPos(); }

float Craft::getVX() const { return dState.getX(); }
float Craft::getVY() const { return dState.getY(); }
const Vector2 &Craft::getVel() const { return dState.getPos(); }

float Craft::getDX() const { return delta.getX(); }
float Craft::getDY() const { return delta.getY(); }
const Vector2 &Craft::getDPos() const { return delta.getPos(); }

float Craft::getAngle() const { return state.getAngle(); }
float Craft::getOmega() const { return dState.getAngle(); }
float Craft::getDAngle() const { return delta.getAngle(); }

const Vector2 &Craft::getVertex(int n) const { return vertices[n]; }

float Craft::getBoostFuel() const { return boostFuel; }

float Craft::getHoverForce() const { return hoverForce; }
float Craft::getSpeed() const { return dState.getPos().length(); }

m3dMesh &Craft::getMesh() const { return mesh; }

bool Craft::beamIntersect(const Vector2& v1, const Vector2& v2, Vector2 &point) const
{
	Vector2 dv(0.5 * WIDTH, 0.0);
	dv.rotate(getAngle());
	
	Vector2 p;
	if(Level::segmentIsect(getPos() + dv, getPos() - dv, v1, v2, p))
	{
		point = p;
		return true;
	}
	
	return false;
}

void Craft::addForce(Vector2 &f)
{
	force = force + f;
}



