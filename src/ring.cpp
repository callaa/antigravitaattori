#include "SDL_opengl.h"

#include <AL/al.h>
#include <cmath>

#include "antigrav.h"

m3dMesh Ring::mesh;

const float Ring::MAXLIFE = 1.0;
Ring *Ring::rings;
int Ring::numRings;

int Ring::init()
{
    if(mesh.loadFromXML("ring.xml"))
        return 1;

    rings = new Ring[MAXRINGS];
    if(!rings)
        return 1;
    return 0;
}

Ring::Ring()
    : life(0)
{
}

Ring::Ring(float x, float y, float ang, const Vector2& vel, const float col[3])
{
    posx = x;
    posy = y;
    velx = vel.getX();
    vely = vel.getY();
    life = MAXLIFE;
    angle = DEG(ang);

    color[0] = col[0];
    color[1] = col[1];
    color[2] = col[2];
    color[3] = 1;
}

void Ring::update(float t)
{
    if(isAlive()) {
        posx += velx * t;
        posy += vely * t;
        life -= t;
        if(life<0)
            color[3]=0;
        else
            color[3] = life/MAXLIFE;
    }
}

void Ring::draw()
{
    glPushMatrix();
    glColor4fv(color);
    glTranslatef(posx,posy,0);
    glRotatef(angle,0,0,1);
    glScalef(7,7,7);
    mesh.draw();
    glPopMatrix();
}

bool Ring::isAlive() const
{
    return life>0;
}

void Ring::updateAll(float t)
{
    for(int i=0;i<MAXRINGS;++i) {
        if(rings[i].isAlive()) {
            rings[i].update(t);
        }
    }
}

void Ring::drawAll() {
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    for(int i=0;i<MAXRINGS;++i) {
        if(rings[i].isAlive()) {
            rings[i].draw();
        }
    }
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

void Ring::addRing(const Ring& ring)
{
    for(int i=0;i<MAXRINGS;++i) {
        if(rings[i].isAlive()==false) {
            rings[i] = ring;
            break;
        }
    }
}

void Ring::resetAll()
{
    for(int i=0;i<MAXRINGS;++i)
        rings[i] = Ring();
}

