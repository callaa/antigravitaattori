#include "SDL_opengl.h"
#include <AL/al.h>
#include <cmath>

#include "antigrav.h"

const float Level::VERTEX_DIST = Terrain::VERTEX_DIST;

Level::Level()
{
	for(int i = 0; i < MAX_VERTICES; i++) vertices[i] = Vector2(i * VERTEX_DIST, 0.0);
	
	for(int i = 5; i < MAX_VERTICES - 5; i++)
	{
		vertices[i] = Vector2(i * VERTEX_DIST, 0.0);
	}
}

void Level::draw3d(const float *eye, const float *at, float fovDiag)
{
	glPushMatrix();
	glTranslatef(0.0, 0.0, -(float)ZERO_DEPTH/2.0);
	terrain.drawLists(eye, at, fovDiag, -(float)ZERO_DEPTH/2.0);
	
	glEnable(GL_TEXTURE_2D);
	terrain.drawRoad(ZERO_DEPTH);
	glDisable(GL_TEXTURE_2D);
	
	glPopMatrix();
}

void Level::draw2d()
{
	glBegin(GL_LINE_STRIP);
	for(int i = 0; i < MAX_VERTICES; i++) glVertex2fv(vertices[i].getData());
	glEnd();
}

bool Level::intersect(const Vector2& v1, const Vector2 &v2, Vector2 &point) const
{
	float x1, x2;
	int min, max;
	
	x1 = v1.getX();
	x2 = v2.getX();
	
	min = (int)(MIN(x1,x2) / VERTEX_DIST) - 1;
	if(min < 0) min = 0;
	
	max = (int)(MAX(x1,x2) / VERTEX_DIST) + 2;
	if(max >= MAX_VERTICES) max = MAX_VERTICES - 1;
	
	bool isect = false;
	for(int i = min; i < max; i++)
	{
		Vector2 temp;
		
		if(segmentIsect(v1, v2, vertices[i], vertices[i+1], temp))
		{
			if(isect)
			{
				if((temp - v1).length() < (point-v1).length()) point = temp;
			} else
			{
				point = temp;
			}
			isect = true;
		}
	}
	
	return isect;
}

float Level::getHeight(float x) const
{
	Vector2 point;
	const Vector2 v1(x, 1000.0);
	const Vector2 v2(x, -1000.0);
	
	if(!intersect(v1,v2,point)) return 0.0;
	return point.getY();
}

bool Level::segmentIsect(float x11, float y11, float x12, float y12, float x21, float y21, float x22, float y22, float *x, float *y)
{
	float s1x, s1y, s2x, s2y, s, t;

	s1x = x12 - x11;
	s1y = y12 - y11;

	s2x = x22 - x21;
	s2y = y22 - y21;

	s = (-s1y*(x11-x21) + s1x*(y11-y21))/(-s2x*s1y + s1x*s2y);
	t =  (s2x*(y11-y21) - s2y*(x11-x21))/(-s2x*s1y + s1x*s2y);

	if(s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		*x = x11 + t * s1x;
		*y = y11 + t * s1y;
		return true;
	}

	return false;
}

bool Level::segmentIsect(const Vector2& v11, const Vector2 &v12, const Vector2 &v21, const Vector2 &v22, Vector2 &point)
{
	Vector2 s1, s2;
	float s, t;

	s1 = v12 - v11;
	s2 = v22 - v21;
	
	s = (s1 ^ (v11 - v21)) / (s1 ^ s2);
	t = (s2 ^ (v11 - v21)) / (s1 ^ s2);

	if(s < 0.0 || s > 1.0 || t < 0.0 || t > 1.0) return false;
	
	point = v11 + t * s1;
	return true;
}


int Level::init()
{
	if(terrain.init(MAX_VERTICES, 32) != 0) return -1;
	
	return 0;
}

void Level::generate(int seed)
{
	Terrain::srandom(seed);
	generate();
}

void Level::generate()
{
	terrain.generate();
	terrain.normalize();
	terrain.descent(30);
	
	for(int i = 0; i < MAX_VERTICES; i++)
	{
		float h = terrain.getHeight(i, ZERO_DEPTH-1);
		terrain.setHeight(i, ZERO_DEPTH, h);
		terrain.setHeight(i, ZERO_DEPTH+1, h);
	}
	
	terrain.computeNormals();
	
	for(int i = 0; i < MAX_VERTICES; i++)
	{
		vertices[i] = Vector2(i * VERTEX_DIST, terrain.getHeight(i, ZERO_DEPTH) * Terrain::HEIGHT_SCALE);
	}
	
	terrain.createLists();
}

bool Level::ellipseSegmentIsect(const Vector2& center, float angle, float major, float minor, const Vector2 &start, const Vector2 &end, Vector2& point, Vector2& normal, Vector2& delta)
{
	Vector2 v1, v2;
	
	v1 = start - center;
	v1.rotate(-angle);
	v1 = Vector2(v1.getX() / major, v1.getY() / minor);
	
	v2 = end - center;
	v2.rotate(-angle);
	v2 = Vector2(v2.getX() / major, v2.getY() / minor);
	
	Vector2 d = v2 - v1;
	float ld = d.length();
	
	float t = (-v1) * d / (ld * ld);
	
	if(t > 0.0 && t < 1.0)
	{
		point = v1 + t * d;
		if(point.length() > 1.0) return false;
		
		delta = point;
		point.normalize();
		delta = point - delta;
	} else
	{
		if(v1.length() > 1.0 && v2.length() > 1.0) return false;
		
		float a, b, c, discr;
		
		a = d * d;
		b = 2.0 * (v1 * d);
		c = (v1 * v1) - 1.0;
		
		discr = b * b - 4.0 * a * c;
		if(discr < 0.0) return false;
		
		t = (-b - sqrt(discr)) / (2.0 * a);
		if(t < 0.0 || t > 1.0) t = (-b + sqrt(discr)) / (2.0 * a);
		if(t < 0.0 || t > 1.0) return false;
		
		point = v1 + t * d;
		
		if(v2.length() < 1.0) delta = (t - 1.0) * d;
		else delta = t * d;
	}
	
	
	// calculate normal
	normal = -point;

	// transform back to world coordinates
	point = Vector2(point.getX() * major, point.getY() * minor);
	point.rotate(angle);
	point = point + center;
	
	normal.rotate(angle);
	
	delta = Vector2(delta.getX() * major, delta.getY() * minor);
	delta.rotate(angle);
	
	return true;
}

bool Level::ellipseIntersect(const Vector2& center, float angle, float major, float minor, Vector2& point, Vector2& normal, Vector2& delta)
{
	float x1, x2;
	int min, max;
	
	x1 = center.getX() - major;
	x2 = center.getX() + major;
	
	min = (int)(MIN(x1,x2) / VERTEX_DIST) - 1;
	if(min < 0) min = 0;
	
	max = (int)(MAX(x1,x2) / VERTEX_DIST) + 2;
	if(max >= MAX_VERTICES) max = MAX_VERTICES - 1;
	
	for(int i = min; i < max; i++)
	{
		if(ellipseSegmentIsect(center, angle, major, minor, vertices[i], vertices[i+1], point, normal, delta)) return true;
	}
	
	return false;
}

float Level::getWidth()
{
	return MAX_VERTICES * VERTEX_DIST;
}

void Level::drawRadar()
{
	glBegin(GL_TRIANGLE_STRIP);
	
	for(int i = 0; i < MAX_VERTICES; i++)
	{
		glVertex2f(vertices[i].getX(), 0.0);
		glVertex2f(vertices[i].getX(), vertices[i].getY());
	}
	
	glEnd();
}
