#include "SDL.h"
#include "SDL_opengl.h"
#include <cmath>

#include <AL/al.h>

#include "antigrav.h"

const float Terrain::VERTEX_DIST = 0.5;
const float Terrain::HEIGHT_SCALE = 5.0;
int Terrain::seed = 1;

Terrain::Terrain()
{
	width = 0;
	height = 0;
	data = NULL;
	normals = NULL;
	roadTex = 0;
	goalTex = 0;
	roadTex2 = 0;
	listBase = 0;
}

Terrain::~Terrain()
{
	delete[] normals;
	delete[] data;
}

int Terrain::init(int w, int h)
{
	if(roadTex == 0) roadTex = m3dTexture::loadTexture("road.png");
	if(roadTex == 0) return -1;
	if(goalTex == 0) goalTex = m3dTexture::loadTexture("goal.png");
	if(goalTex == 0) return -1;
	if(roadTex2 == 0) roadTex2 = m3dTexture::loadTexture("road2.png");
	if(roadTex2 == 0) return -1;
	if(texture == 0) texture = m3dTexture::loadTexture("stone.png");
	if(texture == 0) return -1;
	
	if(listBase == 0) listBase = glGenLists(64);
	if(listBase == 0) return -1;
	
	if(normals) delete[] normals;
	if(data) delete[] data;
	
	if(w <= h)
	{
		if(!isPow2(w)) return -1;
		if(h % w != 0) return -1;
	} else
	{
		if(!isPow2(h)) return -1;
		if(w % h != 0) return -1;
	}
	
	
	data = new float[(w+1) * (h+1)];
	if(data == NULL) return -1;
	
	for(int i = 0; i < (w+1)*(h+1); i++) data[i] = 0;
	
	width = w;
	height = h;
	
	normals = new float[(w+1) * (h+1) * 3];
	if(normals == NULL)
	{
		delete[] data;
		return -1;
	}
	
	return 0;
}

float Terrain::getHeight(int x, int y) const
{
	if(data == NULL) return 0.0;
	if(x < 0 || x > width || y < 0 || y > height) return 0.0;
	
	return data[(y * (width + 1)) + x];
}

void Terrain::setHeight(int x, int y, float h)
{
	if(x < 0 || x > width || y < 0 || y > height) return;
	data[y * (width + 1) + x] = h;
}

void Terrain::generate()
{
	float scale, ratio;
	float avg;
	int s, i, j;
	int x, y, dim;
	int xBlocks, yBlocks;
	int blockSize;
	int n;
	
	if(width <= height)
	{
		n = Terrain::log2(width); // log2(min(width,height))
		xBlocks = 1;
		yBlocks = (height / width);
		blockSize = width;
	} else
	{
		n = Terrain::log2(height);
		yBlocks = 1;
		xBlocks = (width / height);
		blockSize = height;
	}
	
	// seed the array
	for(i = 0; i <= yBlocks; i++)
	{
		for(j = 0; j <= xBlocks; j++)
		{
			setHeight(j * blockSize, i * blockSize, 0.5f);
		}
	}
	
	// roughness parameters
	const float h = 0.7;
	const float heightScale = 3.0;
	ratio = powf(2.0,-h);
	scale = heightScale * ratio;
	
	
	dim = 1 << n;
	for(s = 0; s < n; s++)
	{
		scale *= ratio;
// 		if(s == n-1) scale = 0.0;	// interpolate last step

		// diamond step
		for(i = 0; i < (1 << s) * yBlocks; i++)
		{
			for(j = 0; j < (1 << s) * xBlocks; j++)
			{
				x = j * dim;
				y = i * dim;
				avg = 0;
				avg += getHeight(x, y);
				avg += getHeight(x + dim, y);
				avg += getHeight(x + dim, y + dim);
				avg += getHeight(x, y + dim);
				avg /= 4;
				setHeight(x + (dim >> 1), y + (dim >> 1), avg + scale * (random() - 0.5f));
			}
		}

		// square step
		for(i = 0; i <= (1 << s) * yBlocks; i++)
		{
			for(j = 0; j <= (1 << s) * xBlocks; j++)
			{
				x = j * dim;
				y = i * dim;
				
				if(j < (1 << s) * xBlocks)
				{
					avg = 0;
					avg += getHeight(x, y);
					avg += getHeight(x + dim, y);
					avg /= 2.0;
					setHeight(x + (dim >> 1), y, avg + scale * (random() -0.5f));
				}
				
				if(i < (1 << s) * yBlocks)
				{
					avg = 0;
					avg += getHeight(x, y);
					avg += getHeight(x, y + dim);
					avg /= 2.0;
					setHeight(x, y + (dim >> 1), avg + scale * (random() -0.5f));
				}
			}
		}

		dim >>= 1;
	}
}

void Terrain::normalize()
{
	float min = 1e10, max = -1e10;
	int i, j;
	
	for(i = 0; i <= height; i++)
	{
		for(j = 0; j <= width; j++)
		{
			float h = getHeight(j ,i);
			if(h < min) min = h;
			if(h > max) max = h;
		}
	}
	
	for(i = 0; i <= height; i++)
	{
		for(j = 0; j <= width; j++)
		{
			setHeight(j, i, (getHeight(j ,i) - min) / (max - min));
		}
	}
}

bool Terrain::isPow2(int x)
{
	return x && !( (x-1) & x );
}

int Terrain::log2(int x)
{
	int n = 0;
	
	while(x != 0 && !(x & 1))
	{
		n++;
		x >>= 1;
	}
	
	return n;
}

void Terrain::srandom(int s)
{
	seed = s;
}

float Terrain::random()
{
	const int a = 48271;
	const int m  = 2147483647;
	const int q  = (m / a);
	const int r  = (m % a);
	
	int hi = seed / q;
	int lo = seed % q;
	int test = a * lo - r * hi;
	
	if(test > 0) seed = test;
	else seed = test + m;
	
	return (float)seed / m;
}

void Terrain::vertex(int x, int y)
{
	glNormal3fv(&normals[(y * (width + 1) + x) * 3]);
	glVertex3f(x * VERTEX_DIST, getHeight(x, y) * HEIGHT_SCALE, y * VERTEX_DIST);
}


void Terrain::draw()
{
	int i, j;
	
	for(i = 0; i < height - 1; i++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		
		for(j = 0; j < width; j++)
		{
			vertex(j, i);
			vertex(j, i+1);
		}
		
		glEnd();
	}
}

void Terrain::xproduct(const float *v1, const float *v2, float *result) const
{
	result[0] = v1[1] * v2[2] - v2[1] * v1[2];
	result[1] = v1[0] * v2[2] - v2[0] * v1[2];
	result[2] = v1[0] * v2[1] - v2[0] * v1[1];
}

float Terrain::dotproduct(const float *v1 , const float *v2) const
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void Terrain::vectorsub(const float *v1, const float *v2, float *result) const
{
	result[0] = v1[0] - v2[0];
	result[1] = v1[1] - v2[1];
	result[2] = v1[2] - v2[2];

}
void Terrain::vectorsub(float *v1, const float *v2) const
{
	v1[0] -= v2[0];
	v1[1] -= v2[1];
	v1[2] -= v2[2];
}

void Terrain::vectoradd(const float *v1, const float *v2, float *result) const
{
	result[0] = v1[0] - v2[0];
	result[1] = v1[1] - v2[1];
	result[2] = v1[2] - v2[2];
}

void Terrain::vectoradd(float *v1, const float *v2) const
{
	v1[0] += v2[0];
	v1[1] += v2[1];
	v1[2] += v2[2];
}

void Terrain::vectormul(float a, const float *v, float *result) const
{
	result[0] = a * v[0];
	result[1] = a * v[1];
	result[2] = a * v[2];
}

void Terrain::vectormul(float a, float *v) const
{
	v[0] *= a;
	v[1] *= a;
	v[2] *= a;
}

void Terrain::vectornorm(float *v) const
{
	float l = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= l;
	v[1] /= l;
	v[2] /= l;	
}

float Terrain::vectorlen(const float *v) const
{
	return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void Terrain::computeNormals()
{
	const int delta[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
	
	float *normal;
	float temp[3];
	float v0[3], v1[3], v2[3];
	float weight;
	int i, j, k;
	int x1, y1, x2, y2;
	
	for(i = 0; i <= height; i++)
	{
		for(j = 0; j <= width; j++)
		{
			normal = &normals[(i * (width + 1) + j) * 3];
			normal[0] = 0.0;
			normal[1] = 0.0;
			normal[2] = 0.0;
			
			
			v0[0] = j * VERTEX_DIST;
			v0[1] = getHeight(j, i) * HEIGHT_SCALE;
			v0[2] = i * VERTEX_DIST;
			
			weight = 0.0;
			for(k = 0; k < 4; k++)
			{
				x1 = j + delta[k][0];
				y1 = i + delta[k][1];
				x2 = j + delta[(k+1)%4][0];
				y2 = i + delta[(k+1)%4][1];
				
				if(x1 < 0 || x1 > width || y1 < 0 || y1 > height) continue;
				if(x2 < 0 || x2 > width || y2 < 0 || y2 > height) continue;
			
				v1[0] = x1 * VERTEX_DIST;
				v1[1] = getHeight(x1, y1) * HEIGHT_SCALE;
				v1[2] = y1 * VERTEX_DIST;
				vectorsub(v1, v0);
				
				v2[0] = x2 * VERTEX_DIST;
				v2[1] = getHeight(x2, y2) * HEIGHT_SCALE;
				v2[2] = y2 * VERTEX_DIST;
				vectorsub(v2, v0);
				
				xproduct(v2, v1, temp);
				vectornorm(temp);
				
				vectoradd(normal, temp);
			
				weight += 1.0;
			}
			
			vectormul(1.0 / weight, normal);
		}
	}
}

void Terrain::descent(int start)
{
	int i, j;
	
	for(j = 0; j <= width; j++)
	{
		float maximum = getHeight(j, start);
// 		float minimum = 0.5 * maximum;
		float minimum = 0.0;
		
		for(i = start + 1; i <= height; i++)
		{
			float x = (float)(i - start - 1) / (height - start - 1);
			x = 1 - x * x * x;
			float h = minimum + getHeight(j, i) * x * (maximum - minimum);
			setHeight(j, i, h);
		}
	}
}

void Terrain::drawRoad(int n)
{
	glBindTexture(GL_TEXTURE_2D, roadTex);
	for(int i = 0; i < width - 1; i++)
	{
		if(i == FINISH_LINE) glBindTexture(GL_TEXTURE_2D, goalTex);
		if(i == FINISH_LINE+1) glBindTexture(GL_TEXTURE_2D, roadTex2);
		
		glBegin(GL_TRIANGLE_STRIP);
		for(int j = 0; j <= 2; j++)
		{
			glTexCoord2f(0.0, j * 0.5);
			vertex(i, j + n - 1);
			glTexCoord2f(1.0, j * 0.5);
			vertex(i + 1, j + n - 1);
		}
		glEnd();
	}
}

void Terrain::createList(GLuint list, int x0, int y0, int w, int h)
{
	glNewList(list, GL_COMPILE);
	
	for(int i = 0; i < w; i++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for(int j = 0; j <= h; j++)
		{
			glTexCoord2f((float)i / w, (float)j / h);
			vertex(x0 + i, y0 + j);
			glTexCoord2f((float)(i + 1) / w, (float)j / h);
			vertex(x0 + i + 1, y0 + j);
		}
		glEnd();
	}
	
	glEndList();
}

void Terrain::createLists()
{
	for(int i = 0; i < 32; i++)
	{
		for(int j =0 ; j < 2; j++)
		{
			createList(listBase + j * 32 + i, i * 16, j * 16, 16, 16);
		}
	}
}

void Terrain::drawLists(const float *eye, const float *at, float fovDiag, float dz)
{
	float vec[3];
	
	vec[0] = at[0];
	vec[1] = at[1];
	vec[2] = at[2];
	vectornorm(vec);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	int drawn = 0;
	
	for(int i = 0; i < 32; i++)
	{
		for(int j = 0; j < 2; j++)
		{
			float pos[3] = {i * 16 * VERTEX_DIST - eye[0], 5.0 - eye[1], j * 16 * VERTEX_DIST + dz - eye[2]};
			float len;
			bool inside = false;
			
			len = vectorlen(pos);
			if(acosf(dotproduct(pos, vec) / len) < fovDiag) inside = true;
			
			if(!inside)
			{
				pos[0] = (i + 1) * 16 * VERTEX_DIST - eye[0];
				len = vectorlen(pos);
				if(acosf(dotproduct(pos, vec) / len) < fovDiag) inside = true;
			}
			
			if(!inside)
			{
				pos[2] = (j + 1) * 16 * VERTEX_DIST + dz - eye[2];
				len = vectorlen(pos);
				if(acosf(dotproduct(pos, vec) / len) < fovDiag) inside = true;
			}
			
			if(!inside)
			{
				pos[0] = i * 16 * VERTEX_DIST - eye[0];
				len = vectorlen(pos);
				if(acosf(dotproduct(pos, vec) / len) < fovDiag) inside = true;
			}
			
			if(inside)
			{
				glCallList(listBase + j * 32 + i);
				drawn++;
			}
		}
	}
	
	glDisable(GL_TEXTURE_2D);
}

