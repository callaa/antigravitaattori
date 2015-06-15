#include <math.h>

#include "vector2.h"

/* Constructors */
/// Create a zero vector
/**
	This is the default constructor for the Vector class. Equivalent to Vector(0, 0)
	@see Vector(float,float)
*/
Vector2::Vector2()
{
	data[0] = 0;
	data[1] = 0;
}

/// Create a new vector
/**
	Create a new Vector (xx, yy).
	@param xx	The x-component of the vector
	@param yy	The y-component of the vector
*/
Vector2::Vector2(float xx, float yy)
{
	data[0] = xx;
	data[1] = yy;
}

/* Methods */
/// Set the x-component of this vector
/**
	@param xx	the new x-component of this vector.
*/
void Vector2::setX(float xx)
{
	data[0] = xx;
}

/// Set the y-component of this vector
/**
	@param yy	the new x-component of this vector.
*/
void Vector2::setY(float yy)
{
	data[1] = yy;
}

/// Get the x-component of this vector
/**
	@return		The x-component of this vector
*/
float Vector2::getX() const
{
	return data[0];
}

/// Get the y-component of this vector
/**
	@return		The y-component of this vector
*/
float Vector2::getY() const
{
	return data[1];
}

/// Calculate length of this vector
/**
	Calculates the length of this vector. The length is defined as
	sqrt(x<sup>2</sup>+y<sup>2</sup>).
	@return		The lenght of this vector
*/
float Vector2::length() const
{
	return sqrt(data[0] * data[0] + data[1] * data[1]);
}

/// Calculate the angle of this vector
/**
	Calculate the angle of this vector. The angle is defined as atan2(y, x);
	@return 	The angle of this vector in radians.
*/
float Vector2::angle() const
{
	return atan2(data[1], data[0]);
}

/// Rotate this vector
/**
	Rotate this vector theta radians counter clockwise. The rotation operator is defined
	as x' = Ax, where A is the orthogonal rotation matrix.
	@param	theta	The angle to rotate in radians. Positive means counterclockwise.
	@return a reference to this vector
*/
Vector2& Vector2::rotate(float theta)
{
	float c = cos(theta);
	float s = sin(theta);
	float tx = c * data[0] - s* data[1];
	data[1] = s * data[0] + c * data[1];
	data[0] = tx;
	
	return *this;
}

/// Normalize this vector
/**
	This function normalizes this vector, when a vector is normalized, it preserves
	it direction, but is scaled to have a length of 1.0.
*/
Vector2& Vector2::normalize()
{
	float len = sqrt(data[0] * data[0] + data[1] * data[1]);
	data[0] /= len;
	data[1] /= len;
	
	return *this;
}

Vector2 Vector2::operator+(const Vector2 &vec) const
{
	return Vector2(getX() + vec.getX(), getY() + vec.getY());
}

Vector2 Vector2::operator-(const Vector2 &vec) const
{
	return Vector2(getX() - vec.getX(), getY() - vec.getY());
}

/// Get the unit vector of this vector
/**
	The unit vector is a vector with the same direction as this one, but a length of 1.0.
	@return		A new Vector2 which is the unit vector of this vector.
*/
Vector2 Vector2::unitVector() const
{
	Vector2 temp(data[0], data[1]);
	temp.normalize();
	return temp;
}

/// Get the normal vector of this vector
/**
	The normal vector is a vector with equal length to this vector, but is normal to
	this vector (ie. the two vectors form a 90 degree angle).
	@return		A vector that is normal to this vector and equal in length.
*/
Vector2 Vector2::normalVector() const
{
	return Vector2(-data[1], data[0]);
}

Vector2 &Vector2::operator=(const Vector2 &other)
{
	data[0] = other.getX();
	data[1] = other.getY();
	return *this;
}

/* Operators */

const Vector2 Vector2::operator-() const
{
	return Vector2(-data[0], -data[1]);
}


Vector2 operator*(float a, const Vector2 &vec)
{
	Vector2 temp(vec.getX() * a, vec.getY() * a);
	return temp;
}

Vector2 operator*(const Vector2 &vec, float a)
{
	Vector2 temp(vec.getX() * a, vec.getY() * a);
	return temp;
}

Vector2 operator/(const Vector2 &vec, float a)
{
	Vector2 temp(vec.getX() / a, vec.getY() / a);
	return temp;
}

/* Dot product */
float operator*(const Vector2 &vec1, const Vector2 &vec2)
{
	return vec1.getX() * vec2.getX() + vec1.getY() * vec2.getY();
}

/* Cross product */
float operator^(const Vector2 &vec1, const Vector2 &vec2)
{
	return vec1.getX() * vec2.getY() - vec2.getX() * vec1.getY();
}

const float *Vector2::getData() const { return data; }

