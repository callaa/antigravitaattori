#ifndef _TERRAIN_H_
#define _TERRAIN_H_

class Terrain
{
public:
	static const float VERTEX_DIST;
	static const float HEIGHT_SCALE;
	
	static const int FINISH_LINE = 492;
	
	Terrain();
	~Terrain();
	
	void draw();
	void drawLists(const float *eye, const float *at, float fovDiag, float dz);
	void drawRoad(int n);

	float getHeight(int x, int y) const;
	void setHeight(int x, int y, float h);
	
	void generate();
	void normalize();
	void computeNormals();
	void descent(int start);
	void createLists();
	
	int init(int w, int h);
	
	static float random();
	static void srandom(int s);
	
	static bool isPow2(int x);
	static int log2(int x);
	
private:
	static int seed;
	
	void xproduct(const float *v1, const float *v2, float *result) const;
	float dotproduct(const float *v1 , const float *v2) const;
	void vectorsub(const float *v1, const float *v2, float *result) const;
	void vectorsub(float *v1, const float *v2) const;
	void vectoradd(const float *v1, const float *v2, float *result) const;
	void vectoradd(float *v1, const float *v2) const;
	void vectormul(float a, const float *v, float *result) const;
	void vectormul(float a, float *v) const;
	void vectornorm(float *v) const;
	float vectorlen(const float *v) const;
	
	void vertex(int x, int y);
	
	void createList(GLuint list, int x0, int y0, int w, int h);
	
	float *data;
	float *normals;
	int width;
	int height;
	
	GLuint roadTex, goalTex, roadTex2;
	GLuint texture;
	GLuint listBase;
};

#endif

