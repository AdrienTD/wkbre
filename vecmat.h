// wkbre - WK (Battles) recreated game engine
// Copyright (C) 2015-2016 Adrien Geets
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

//#define VECMAT_D3DX

///// D3DX9 /////

#ifdef VECMAT_D3DX
#define Vector3 D3DXVECTOR3
#define Vector4 D3DXVECTOR4
#define Matrix D3DXMATRIXA16
inline void CreateIdentityMatrix(Matrix *m)
	{D3DXMatrixIdentity(m);}
inline void CreateZeroMatrix(Matrix *m)
	{memset(m, 0, 16 * sizeof(float));}
inline void CreateTranslationMatrix(Matrix *m, float x, float y, float z)
	{D3DXMatrixTranslation(m, x, y, z);}
inline void CreateScaleMatrix(Matrix *m, float x, float y, float z)
	{D3DXMatrixScaling(m, x, y, z);}
inline void CreateRotationXMatrix(Matrix *m, float a)
	{D3DXMatrixRotationX(m, a);}
inline void CreateRotationYMatrix(Matrix *m, float a)
	{D3DXMatrixRotationY(m, a);}
inline void CreateRotationZMatrix(Matrix *m, float a)
	{D3DXMatrixRotationZ(m, a);}
inline void MultiplyMatrices(Matrix *m, Matrix *a, Matrix *b)
	{D3DXMatrixMultiply(m, a, b);}
inline void TransposeMatrix(Matrix *m, Matrix *a)
	{D3DXMatrixTranspose(m, a);}
inline void TransformNormal3(Vector3 *v, Vector3 *a, Matrix *m)
	{D3DXVec3TransformNormal(v, a, m);}
inline void CreatePerspectiveMatrix(Matrix *m, float fovy, float aspect, float zn, float zf)
	{D3DXMatrixPerspectiveFovLH(m, fovy, aspect, zn, zf);}
inline void CreateLookAtLHViewMatrix(Matrix *m, Vector3 *eye, Vector3 *at, Vector3 *up)
	{D3DXMatrixLookAtLH(m, eye, at, up);}
inline void CreateRotationYXZMatrix(Matrix *m, float y, float x, float z)
	{D3DXMatrixRotationYawPitchRoll(m, y, x, z);}
inline void TransformCoord3(Vector3 *r, Vector3 *v, Matrix *m)
	{D3DXVec3TransformCoord(r, v, m);}
//inline void InverseMatrix(Matrix *o, Matrix *i)
//	{D3DXMatrixInverse(o, NULL, i);}
inline void NormalizeVector3(Vector3 *o, Vector3 *i)
	{D3DXVec3Normalize(o, i);}
inline BOOL SphereIntersectsRay(Vector3 *sphPos, float radius, Vector3 *raystart, Vector3 *raydir)
	{return D3DXSphereBoundProbe(sphPos, radius, raystart, raydir);}

// Unfininished XNA removed, see wkbre21

///// Built-in math library /////

//#elif defined VECMAT_OWN

#else
struct Matrix;
struct Vector3;
void CreateIdentityMatrix(Matrix *m);
void CreateZeroMatrix(Matrix *m);
void CreateTranslationMatrix(Matrix *m, float x, float y, float z);
void CreateScaleMatrix(Matrix *m, float x, float y, float z);
void CreateRotationXMatrix(Matrix *m, float a);
void CreateRotationYMatrix(Matrix *m, float a);
void CreateRotationZMatrix(Matrix *m, float a);
void MultiplyMatrices(Matrix *m, Matrix *a, Matrix *b);
void TransposeMatrix(Matrix *m, Matrix *a);
void TransformVector3(Vector3 *v, Vector3 *a, Matrix *m);
void TransformNormal3(Vector3 *v, Vector3 *a, Matrix *m);
void CreatePerspectiveMatrix(Matrix *m, float fovy, float aspect, float zn, float zf);
void CreateLookAtLHViewMatrix(Matrix *m, Vector3 *eye, Vector3 *at, Vector3 *up);
void CreateRotationYXZMatrix(Matrix *m, float y, float x, float z);
void TransformCoord3(Vector3 *r, Vector3 *v, Matrix *m);
//void InverseMatrix(Matrix *o, Matrix *i);
void NormalizeVector3(Vector3 *o, Vector3 *i);
int SphereIntersectsRay(Vector3 *sphPos, float radius, Vector3 *raystart, Vector3 *raydir);

struct Matrix
{
	union {
		float v[16];
		float m[4][4];
		struct {
			float _11, _12, _13, _14, _21, _22, _23, _24, 
				_31, _32, _33, _34, _41, _42, _43, _44;
		};
	};
	Matrix operator*(Matrix &a)
	{
		Matrix m;
		MultiplyMatrices(&m, this, &a);
		return m;
	}
	void operator*=(Matrix &a)
	{
		Matrix m = *this;
		MultiplyMatrices(this, &m, &a);
	}
};

struct Vector3
{
	valuetype x, y, z, w; // w used to align size.
	Vector3() {x = y = z = 0;}
	Vector3(valuetype a, valuetype b) {x = a; y = b; z = 0;}
	Vector3(valuetype a, valuetype b, valuetype c) {x = a; y = b; z = c;}

	Vector3 operator+(Vector3 a) {return Vector3(x + a.x, y + a.y, z + a.z);}
	Vector3 operator-(Vector3 a) {return Vector3(x - a.x, y - a.y, z - a.z);}
	Vector3 operator*(Vector3 a) {return Vector3(x * a.x, y * a.y, z * a.z);}
	Vector3 operator/(Vector3 a) {return Vector3(x / a.x, y / a.y, z / a.z);}
	Vector3 operator+(valuetype a) {return Vector3(x + a, y + a, z + a);}
	Vector3 operator-(valuetype a) {return Vector3(x - a, y - a, z - a);}
	Vector3 operator*(valuetype a) {return Vector3(x * a, y * a, z * a);}
	Vector3 operator/(valuetype a) {return Vector3(x / a, y / a, z / a);}
	Vector3 operator-() {return Vector3(-x, -y, -z);}

	Vector3 operator+=(Vector3 a) {x += a.x; y += a.y; z += a.z; return *this;}
	Vector3 operator-=(Vector3 a) {x -= a.x; y -= a.y; z -= a.z; return *this;}
	Vector3 operator*=(Vector3 a) {x *= a.x; y *= a.y; z *= a.z; return *this;}
	Vector3 operator/=(Vector3 a) {x /= a.x; y /= a.y; z /= a.z; return *this;}
	Vector3 operator+=(valuetype a) {x += a; y += a; z += a; return *this;}
	Vector3 operator-=(valuetype a) {x -= a; y -= a; z -= a; return *this;}
	Vector3 operator*=(valuetype a) {x *= a; y *= a; z *= a; return *this;}
	Vector3 operator/=(valuetype a) {x /= a; y /= a; z /= a; return *this;}

	bool operator==(Vector3 a) {return (x==a.x) && (y==a.y) && (z==a.z);}
	bool operator!=(Vector3 a) {return !( (x==a.x) && (y==a.y) && (z==a.z) );}

	void print() {printf("(%f, %f, %f)\n", x, y, z);}
	valuetype len2xy() {return sqrt(x*x + y*y);}
	valuetype sqlen2xy() {return x*x + y*y;}
	valuetype len2xz() {return sqrt(x*x + z*z);}
	valuetype sqlen2xz() {return x*x + z*z;}
	valuetype len3() {return sqrt(x*x + y*y + z*z);}
	valuetype sqlen3() {return x*x + y*y + z*z;}
	Vector3 normal() {valuetype l = len3(); if(l != 0.0f) return Vector3(x/l, y/l, z/l); else return Vector3(0,0,0);}
	Vector3 normal2xz() {valuetype l = len2xz(); if(l != 0.0f) return Vector3(x/l, 0, z/l); else return Vector3(0,0,0);}
	valuetype dot(Vector3 a) {return a.x * x + a.y * y + a.z * z;}
	valuetype dot2xz(Vector3 a) {return a.x * x + a.z * z;}
};

#endif

void TransformBackFromViewMatrix(Vector3 *r, Vector3 *o, Matrix *m);
void CreateWorldMatrix(Matrix *mWorld, Vector3 is, Vector3 ir, Vector3 it);
