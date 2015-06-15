#ifndef _LEVEL_H_
#define _LEVEL_H_

class Level
{
public:
	Level();
	
	int init();
	
	void generate(int seed);
	void generate();
	
	float getWidth();
	
	void draw3d(const float *eye, const float *at, float fovDiag);
	void draw2d();
	
	void drawRadar();
	
	bool intersect(const Vector2& v1, const Vector2 &v2, Vector2 &point) const;
	bool ellipseIntersect(const Vector2& center, float angle, float major, float minor, Vector2& point, Vector2& normal, Vector2 &delta);
	float getHeight(float x) const;

	static Level &getInstance();

	static bool segmentIsect(float x11, float y11, float x12, float y12, float x21, float y21, float x22, float y22, float *x, float *y);
	static bool segmentIsect(const Vector2& v11, const Vector2 &v12, const Vector2 &v21, const Vector2 &v22, Vector2 &point);
	
	static bool ellipseSegmentIsect(const Vector2& center, float angle, float major, float minor, const Vector2 &start, const Vector2 &end, Vector2& point, Vector2& normal, Vector2 &delta);
	
private:
	
	static const int ZERO_DEPTH = 24;
	static const int MAX_VERTICES = 512;
	static const float VERTEX_DIST;
	
	Vector2 vertices[MAX_VERTICES];
	Terrain terrain;
};

#endif

