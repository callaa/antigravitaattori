#ifndef _RING_H_
#define _RING_H_

class Ring {
	public:
		static int init();

		Ring();
		Ring(float x, float y, float ang, const Vector2& vel, const float col[3]);
		void update(float t);
		void draw();
		bool isAlive() const;

		static void resetAll();
		static void updateAll(float t);
		static void drawAll();
		static void addRing(const Ring& ring);

	private:
		static m3dMesh mesh;
		float posx,posy,angle;
		float velx,vely;
		float life;
		float color[4];

		static const float MAXLIFE;
		static const int MAXRINGS = 100;

		static Ring *rings;
		static int numRings;
};

#endif

