#ifndef _VECTOR_H_
#define _VECTOR_H_

/// A mathematical 2D Vector
/**
	The Vector class is the basis of all mathematics used in the physical
	simulation. The Vector class is a mathematical representation of a 2D-vector.
	The vector has two components, x and y. The vector class is overloaded with the
	following operators: + (vector addition), - (vector substraction/negation), ^
	(vector <b>cross</b> product) and * (vector <b>dot</b> product). Operators
	for scalar multiplication and division are also defined.
*/
class Vector2
{
public:
	Vector2();
	Vector2(float xx, float yy);
	
	void setX(float xx);
	void setY(float yy);

	float getX() const;
	float getY() const;
	float length() const;
	float angle() const;
	Vector2 &rotate(float theta);
	
	Vector2 &normalize();
	
	Vector2 unitVector() const;
	Vector2 normalVector() const;
	
	Vector2 operator+(const Vector2 &vec) const;
	Vector2 operator-(const Vector2 &vec) const;
	
	Vector2 &operator=(const Vector2 &other);
	
	const float *getData() const;
	
	/* Vector negation */
	const Vector2 operator-() const;
	
private:
	float data[2];
};

/* Operators */
extern float operator*(const Vector2 &vec1, const Vector2 &vec2);
extern float operator^(const Vector2 &vec1, const Vector2 &vec2);
extern Vector2 operator/(const Vector2 &vec, float a);
extern Vector2 operator*(const Vector2 &vec, float a);
extern Vector2 operator*(float a, const Vector2 &vec);

#endif
