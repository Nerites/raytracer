// Transform.cpp: implementation of the Transform class.

// Note: when you construct a matrix using mat4() or mat3(), it will be COLUMN-MAJOR
// Keep this in mind in readfile.cpp and display.cpp
// See FAQ for more details or if you're having problems.

#include "Transform.h"

// Helper rotation function.  Please implement this.  
mat3 Transform::rotate(const float degrees, const vec3& axis)
{
	float rads = degrees*pi / 180.0f;

	mat3 AAT;
	mat3 dualA;

	mat3 I = mat3(1.0f); // Identity matrix

						 // Transposed due to column-major ordering
	AAT[0] = vec3(pow(axis[0], 2), axis[0] * axis[1], axis[0] * axis[2]);
	AAT[1] = vec3(axis[0] * axis[1], pow(axis[1], 2), axis[1] * axis[2]);
	AAT[2] = vec3(axis[0] * axis[2], axis[1] * axis[2], pow(axis[2], 2));
	AAT = glm::transpose(AAT);

	dualA[0] = vec3(0, -axis[2], axis[1]);
	dualA[1] = vec3(axis[2], 0, -axis[0]);
	dualA[2] = vec3(-axis[1], axis[0], 0);
	dualA = glm::transpose(dualA);

	// Rodrigues' formula
	mat3 R = cos(rads)*I + (1 - cos(rads))*AAT + sin(rads)*dualA;
	return R;
}

void Transform::left(float degrees, vec3& eye, vec3& up)
{
	vec3 axis = glm::normalize(up);
	mat3 R = rotate(degrees, axis);
	eye = R*eye;
	up = R*up;
}

void Transform::up(float degrees, vec3& eye, vec3& up)
{
	vec3 axis = glm::normalize(glm::cross(eye, up));
	mat3 R = rotate(degrees, axis);
	eye = R*eye;
	up = R*up;
}

mat4 Transform::lookAt(const vec3 &eye, const vec3 &center, const vec3 &up)
{
	vec3 a = eye - center;
	vec3 b = up;

	vec3 w = glm::normalize(a);
	vec3 u = glm::cross(b, w);
	u = glm::normalize(u);
	vec3 v = glm::cross(w, u);
	v = glm::normalize(v);

	mat4 R;
	// Transposed since GLM has column-major ordering
	R[0] = vec4(u, 0);
	R[1] = vec4(v, 0);
	R[2] = vec4(w, 0);
	R[3] = vec4(0, 0, 0, 1);
	R = glm::transpose(R);

	mat4 T;
	// Transposed since GLM has column-major ordering
	T[0] = vec4(1, 0, 0, -a[0]);
	T[1] = vec4(0, 1, 0, -a[1]);
	T[2] = vec4(0, 0, 1, -a[2]);
	T[3] = vec4(0, 0, 0, 1);
	T = glm::transpose(T);

	mat4 result;

	result = R*T;


	// You will change this return call
	return result;
}

mat4 Transform::perspective(float fovy, float aspect, float zNear, float zFar)
{
	// Perspective matrix:
	// d/aspect	0	0	0
	// 0		d	0	0
	// 0		0	A	B
	// 0		0	-1	0
	//
	// A = -(f + n)/(f - n)
	// B = -(2 * f * n)/(f - n)
	// d = cot(theta) = 1/tan(theta)
	// theta = (fovy/2) * (pi/180)

	mat4 P;
	float A;
	float B;
	float d;
	float theta;

	theta = (fovy / 2)*(pi / 180); // Convert to radians
	d = 1 / tan(theta);

	A = -((zFar + zNear) / (zFar - zNear));
	B = -((2 * zFar*zNear) / (zFar - zNear));

	P[0] = vec4(d / aspect, 0, 0, 0);
	P[1] = vec4(0, d, 0, 0);
	P[2] = vec4(0, 0, A, B);
	P[3] = vec4(0, 0, -1, 0);
	// Transpose since GLM is column-major
	P = glm::transpose(P);

	return P;
}

mat4 Transform::scale(const float &sx, const float &sy, const float &sz)
{
	mat4 S;

	S[0] = vec4(sx, 0, 0, 0);
	S[1] = vec4(0, sy, 0, 0);
	S[2] = vec4(0, 0, sz, 0);
	S[3] = vec4(0, 0, 0, 1);
	// Transpose since GLM is column-major
	S = glm::transpose(S);

	return S;
}

mat4 Transform::translate(const float &tx, const float &ty, const float &tz)
{
	mat4 T;

	T[0] = vec4(1, 0, 0, tx);
	T[1] = vec4(0, 1, 0, ty);
	T[2] = vec4(0, 0, 1, tz);
	T[3] = vec4(0, 0, 0, 1);
	// Transpose since GLM is column-major
	T = glm::transpose(T);

	return T;
}

// To normalize the up direction and construct a coordinate frame.  
// As discussed in the lecture.  May be relevant to create a properly 
// orthogonal and normalized up. 
// This function is provided as a helper, in case you want to use it. 
// Using this function (in readfile.cpp or display.cpp) is optional.  

vec3 Transform::upvector(const vec3 &up, const vec3 & zvec)
{
	vec3 x = glm::cross(up, zvec);
	vec3 y = glm::cross(zvec, x);
	vec3 ret = glm::normalize(y);
	return ret;
}


Transform::Transform()
{

}

Transform::~Transform()
{

}
