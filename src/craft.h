#ifndef _CRAFT_H_
#define _CRAFT_H_

class CraftState
{
public:
	CraftState();
	CraftState(const Vector2& pos, float angle);
	
	const Vector2& getPos() const;
	float getAngle() const;
	
	float getX() const;
	float getY() const;
	
	void setPos(const Vector2 &p);
	void setAngle(float a);
	
private:
	Vector2 pos;
	float angle;
};

extern CraftState operator+(const CraftState& s1, const CraftState& s2);
extern CraftState operator*(float f, const CraftState& s2);

class Craft
{
public:
	static const int CTRL_CW = 0;
	static const int CTRL_CCW = 1;
	static const int CTRL_BOOST = 2;
	static const int NUM_CONTROLS = 3;

	static const float TURN_RATE;
	static const float TURN_DAMP;
	static const float TURN_BACK;

	static const float GRAVITY;
	static const float HOVER_FORCE;
	static const float BOOST;
	static const float DAMP;
	
	static const float MAJOR_AXIS;
	static const float MINOR_AXIS;
	static const float WIDTH;
	static const float HEIGHT;
	static const float INERTIA;
	static const float MASS;
	
	static const float INVERTED_FORCE_MOD;
	
	static const float BOOST_REFUEL;
	static const float BOOST_FUEL_USE;
	
	static const float BOUNCYNESS;
	static const float LEVEL_BOUNCYNESS;

	Craft();

	void update(float dt);
	void setControl(int control, bool value);
	
	void setPos(const Vector2 &p);
	void setVel(const Vector2 &v);
	void setAngle(float a);
	void setOmega(float a);
	void setColor(float r, float g, float b);

	float getX() const;
	float getY() const;
	const Vector2 &getPos() const;
	float getVX() const;
	float getVY() const;
	float getDX() const;
	float getDY() const;
	const Vector2 &getDPos() const;
	const Vector2 &getVel() const;
	float getAngle() const;
	float getOmega() const;
	float getDAngle() const;
	
	bool beamIntersect(const Vector2& v1, const Vector2& v2, Vector2 &point) const;
	void addForce(Vector2 &f);
	
	float getBoostFuel() const;
	float getHoverForce() const;
	float getSpeed() const;
	
	const Vector2 &getVertex(int n) const;
	
	bool collide(Craft &other);
	bool levelCollide();
	void move();

	void draw2d();
	void draw3d();
	
	static int init();
	
	m3dMesh &getMesh() const;

private:
	CraftState state, dState, delta;
	Vector2 vertices[4];
	float majorAxis, minorAxis;
	
	Vector2 beam[2];
	
	Vector2 force;

	bool ctrl[NUM_CONTROLS];
	
	Vector2 getImpulse(Craft &other, float e, const Vector2 &v, const Vector2 &point, const Vector2 &normal);
	bool checkCollision(const Craft &other, Vector2 &point, Vector2 &normal);
	void handleCollision(Craft &other, const Vector2& point, const Vector2& normal);
	void addImpulse(const Vector2 &impulse, const Vector2 &point);
	
	bool checkLevelCollision(Vector2 &point, Vector2 &normal, Vector2 &delta);
	
	template <class T> void integrateRKN(float x0, T y0, T dy0, float dx, T *y, T *dy);
	CraftState derive(float t, const CraftState &s, const CraftState &ds);
	
	static m3dMesh mesh;
	
	float boostFuel;
	float hoverForce;
	float ringTimer;
	float color[3];
};


#endif

