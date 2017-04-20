// ****************************************************************************************
//
// Horde3D Scene Editor 
// --------------------------------------
// Copyright (C) 2007 Volker Wiendl
// 
// This file is part of the Horde3D Scene Editor.
//
// The Horde3D Scene Editor is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 3 of the License 
//
// The Horde3D Scene Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ****************************************************************************************

#ifndef CUSTOMTYPES_H_
#define CUSTOMTYPES_H_

#include <QtCore/QVariant>
#include <math.h>

class Property;
class QObject;

struct QVec3f
{
	QVec3f() : X(0.0f), Y(0.0f), Z(0.0f) {} 
	QVec3f(float x, float y, float z) : X(x), Y(y), Z(z) {}
	float X, Y, Z;

	bool operator == (const QVec3f& other) const {return X == other.X && Y == other.Y && Z == other.Z;} 
	bool operator != (const QVec3f& other) const {return X != other.X || Y != other.Y || Z != other.Z;} 
	QVec3f operator - (const QVec3f& other) const {return QVec3f(X - other.X, Y - other.Y, Z - other.Z);}
	QVec3f operator + (const QVec3f& other) const {return QVec3f(X + other.X, Y + other.Y, Z + other.Z);}
	QVec3f operator / (const QVec3f& other) const {return QVec3f( X / other.X, Y / other.Y, Z / other.Z);}
	QVec3f operator * (const float& scale) const {return QVec3f(X * scale, Y * scale, Z * scale);}
	QVec3f& operator *= (const float& scale)  {X *= scale; Y *= scale; Z *= scale; return *this;}

	float length() const {return sqrt(X * X + Y * Y + Z * Z);}
	QVec3f normalized() const {float l = this->length(); return QVec3f(X/l, Y/l, Z/l);} 
	QVec3f toRad() const {return QVec3f(X * 3.14159265358979f / 180.0f, Y * 3.14159265358979f / 180.0f, Z * 3.14159265358979f / 180.0f);}
	QVec3f toDeg() const {return QVec3f(X * 180.0f / 3.14159265358979f , Y * 180.0f / 3.14159265358979f, Z * 180.0f / 3.14159265358979f);}

};
Q_DECLARE_METATYPE(QVec3f)

struct Material
{
	Material() {}
	Material(const QString& fileName) : FileName(fileName) {}
	QString FileName;

	bool operator == (const Material& other) const {return FileName == other.FileName;} 
	bool operator != (const Material& other) const {return FileName != other.FileName;} 
};
Q_DECLARE_METATYPE(Material)

struct Effect
{
	Effect() {}
	Effect(const QString& fileName) : FileName(fileName) {}
	QString FileName;

	bool operator == (const Effect& other) const {return FileName == other.FileName;} 
	bool operator != (const Effect& other) const {return FileName != other.FileName;} 
};
Q_DECLARE_METATYPE(Effect)

struct Shader
{
	Shader() {}
	Shader(const QString& fileName) : FileName(fileName) {}
	QString FileName;

	bool operator == (const Shader& other) const {return FileName == other.FileName;} 
	bool operator != (const Shader& other) const {return FileName != other.FileName;} 
};
Q_DECLARE_METATYPE(Shader)


struct Texture
{
	Texture() {}
	Texture( const QString& fileName ) : FileName(fileName) {}
	QString FileName;

	bool operator == (const Texture& other) const {return FileName == other.FileName; } 
	bool operator != (const Texture& other) const {return FileName != other.FileName; } 
};
Q_DECLARE_METATYPE(Texture)

struct Frustum
{
	Frustum() {}
	Frustum(const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane) : Left(left), 
		Right(right), Bottom(bottom), Top(top), Near(nearPlane), Far(farPlane) {}

	float Left, Right, Bottom, Top, Near, Far;

	float Fov()
	{			
		return atanf( Top / Near ) * 360.0f / 3.1415926;			
	}
	float Aspect()
	{
		return Right / Top;
	}

	bool isSymetric() {return Left == - Right && Bottom == -Top;}

	bool operator == (const Frustum& other) const { return Left == other.Left && Right == other.Right && Bottom == other.Bottom && Top == other.Top && Near == other.Near && Far == other.Far;}
	bool operator != (const Frustum& other) const { return Left != other.Left || Right != other.Right || Bottom != other.Bottom || Top != other.Top || Near != other.Near || Far != other.Far;}

};
Q_DECLARE_METATYPE(Frustum)


struct Pipeline
{
	Pipeline() : FileName(QString()) {}
	Pipeline(const QString& file) : FileName(file) {}

	QString		FileName;
	int			ResourceID;

	bool operator != (const Pipeline& other) const {return ResourceID != other.ResourceID || FileName != other.FileName;}	
};
Q_DECLARE_METATYPE(Pipeline)

// -------------------------------------------------------------------------------------------------
// QQuatF
// -------------------------------------------------------------------------------------------------

class QQuatF
{
public:	
	float x, y, z;
	float w;

	// ------------
	// Constructors
	// ------------
	QQuatF() : x(0.0f), y(0.0f), z(0.0f), w( 0.0f ) 
	{ 
	}
	
	explicit QQuatF( const float& X, const float& Y, const float& Z, const float& W ) : x( X ), y( Y ), z( Z ), w( W )
	{
	}
	
	QQuatF( const float eulerX, const float eulerY, const float eulerZ )
	{
		QQuatF  roll( sinf( eulerX / 2 ),                  0,                  0, cosf( eulerX / 2 ) );
		QQuatF pitch(                  0, sinf( eulerY / 2 ),                  0, cosf( eulerY / 2 ) );
		QQuatF   yaw(                  0,                  0, sinf( eulerZ / 2 ), cosf( eulerZ / 2 ) );
	
		// Order: y * x * z
		*this = pitch * roll * yaw;
	}

	// ---------------------
	// Artitmetic operations
	// ---------------------
	QQuatF operator*( const QQuatF &q ) const
	{
		return QQuatF( 
			(y * q.z - z * q.y) + q.x * w + x * q.w, 
			(z * q.x - x * q.z) + q.y * w + y * q.w, 
			(x * q.y - y * q.x) + q.z * w + z * q.w,
			 w * q.w - (x * q.x + y * q.y + z * q.z));
		//return QQuatF( v.crossProduct( q.v ) + q.v * w + v * q.w, w * q.w - v * q.v );
	}

	QQuatF operator*=( const QQuatF &q )
	{
		*this = *this * q;
		return *this;
	}

	// ----------------
	// Other operations
	// ----------------

	float magnitude() const
	{
		return sqrtf(w * w + x * x + y * y + z * z);
	}

};


// -------------------------------------------------------------------------------------------------
// Matrix
// -------------------------------------------------------------------------------------------------

class QMatrix4f
{
private:

	QMatrix4f( bool /*noInit*/ )
	{
		// Don't initialize the matrix
	}

public:
	
	union
	{
		float c[4][4];	// Column major order for OpenGL: c[column][row]
		float x[16];
	};
	
	// --------------
	// Static methods
	// --------------
	static QMatrix4f TransMat( float x, float y, float z )
	{
		QMatrix4f m;

		m.c[3][0] = x;
		m.c[3][1] = y;
		m.c[3][2] = z;

		return m;
	}

	static QMatrix4f TransMat( const QVec3f& trans )
	{
		QMatrix4f m;

		m.c[3][0] = trans.X;
		m.c[3][1] = trans.Y;
		m.c[3][2] = trans.Z;

		return m;
	}

	static QMatrix4f ScaleMat( float x, float y, float z )
	{
		QMatrix4f m;
		
		m.c[0][0] = x;
		m.c[1][1] = y;
		m.c[2][2] = z;

		return m;
	}

	static QMatrix4f ScaleMat( const QVec3f& scale )
	{
		QMatrix4f m;
		
		m.c[0][0] = scale.X;
		m.c[1][1] = scale.Y;
		m.c[2][2] = scale.Z;

		return m;
	}

	static QMatrix4f RotMat( float x, float y, float z )
	{
		// Rotation order: YXZ [* Vector]
		return QMatrix4f( QQuatF( x, y, z ) );
	}

	static QMatrix4f RotMat( QVec3f axis, float angle )
	{
		axis = axis * sinf( angle / 2 );
		return QMatrix4f( QQuatF( axis.X, axis.Y, axis.Z, cosf( angle / 2 ) ) );
	}

	static QMatrix4f RotMat( QVec3f angles )
	{		
		return QMatrix4f( QQuatF( angles.X, angles.Y, angles.Z ) );
	}

	// ------------
	// Constructors
	// ------------
    QMatrix4f( const float& x1 = 1.0f, const float& x2 = 0.0f, const float& x3 = 0.0f, const float& x4 = 0.0f, 
              const float& x5 = 0.0f, const float& x6 = 1.0f, const float& x7 = 0.0f, const float& x8 = 0.0f, 
              const float& x9 = 0.0f, const float& x10 = 0.0f, const float& x11 = 1.0f, const float& x12 = 0.0f, 
              const float& x13 = 0.0f, const float& x14 = 0.0f, const float& x15 = 0.0f, const float& x16 = 1.0f ) 	
	{
		x[0]  = x1;  x[1]  = x2;  x[2]  = x3;  x[3] = x4;
		x[4]  = x5;  x[5]  = x6;  x[6]  = x7;  x[7] = x8;
		x[8]  = x9;  x[9]  = x10; x[10] = x11; x[11] = x12;
		x[12] = x13; x[13] = x14; x[14] = x15; x[15] = x16;
	}

	QMatrix4f( const float *floatArray16 )
	{
		for( unsigned int i = 0; i < 4; ++i )
		{
			for( unsigned int j = 0; j < 4; ++j )
			{
				c[i][j] = floatArray16[i * 4 + j];
			}
		}
	}

	QMatrix4f( const QQuatF &q )
	{
		float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

		// Calculate coefficients
		x2 = q.x + q.x;	y2 = q.y + q.y;	z2 = q.z + q.z;
		xx = q.x * x2;	xy = q.x * y2;	xz = q.x * z2;
		yy = q.y * y2;	yz = q.y * z2;	zz = q.z * z2;
		wx = q.w * x2;		wy = q.w * y2;		wz = q.w * z2;


		c[0][0] = 1 - (yy + zz);	c[1][0] = xy - wz;	
		c[2][0] = xz + wy;			c[3][0] = 0;
		c[0][1] = xy + wz;			c[1][1] = 1 - (xx + zz);
		c[2][1] = yz - wx;			c[3][1] = 0;
		c[0][2] = xz - wy;			c[1][2] = yz + wx;
		c[2][2] = 1 - (xx + yy);	c[3][2] = 0;
		c[0][3] = 0;				c[1][3] = 0;
		c[2][3] = 0;				c[3][3] = 1;
	}

	// ---------------------
	// Matrix multiplication
	// ---------------------
	QMatrix4f operator*( const QMatrix4f &m ) const 
	{
		QMatrix4f mf( false );
		
		mf.x[0] = x[0] * m.x[0] + x[4] * m.x[1] + x[8] * m.x[2] + x[12] * m.x[3];
		mf.x[1] = x[1] * m.x[0] + x[5] * m.x[1] + x[9] * m.x[2] + x[13] * m.x[3];
		mf.x[2] = x[2] * m.x[0] + x[6] * m.x[1] + x[10] * m.x[2] + x[14] * m.x[3];
		mf.x[3] = x[3] * m.x[0] + x[7] * m.x[1] + x[11] * m.x[2] + x[15] * m.x[3];

		mf.x[4] = x[0] * m.x[4] + x[4] * m.x[5] + x[8] * m.x[6] + x[12] * m.x[7];
		mf.x[5] = x[1] * m.x[4] + x[5] * m.x[5] + x[9] * m.x[6] + x[13] * m.x[7];
		mf.x[6] = x[2] * m.x[4] + x[6] * m.x[5] + x[10] * m.x[6] + x[14] * m.x[7];
		mf.x[7] = x[3] * m.x[4] + x[7] * m.x[5] + x[11] * m.x[6] + x[15] * m.x[7];

		mf.x[8] = x[0] * m.x[8] + x[4] * m.x[9] + x[8] * m.x[10] + x[12] * m.x[11];
		mf.x[9] = x[1] * m.x[8] + x[5] * m.x[9] + x[9] * m.x[10] + x[13] * m.x[11];
		mf.x[10] = x[2] * m.x[8] + x[6] * m.x[9] + x[10] * m.x[10] + x[14] * m.x[11];
		mf.x[11] = x[3] * m.x[8] + x[7] * m.x[9] + x[11] * m.x[10] + x[15] * m.x[11];

		mf.x[12] = x[0] * m.x[12] + x[4] * m.x[13] + x[8] * m.x[14] + x[12] * m.x[15];
		mf.x[13] = x[1] * m.x[12] + x[5] * m.x[13] + x[9] * m.x[14] + x[13] * m.x[15];
		mf.x[14] = x[2] * m.x[12] + x[6] * m.x[13] + x[10] * m.x[14] + x[14] * m.x[15];
		mf.x[15] = x[3] * m.x[12] + x[7] * m.x[13] + x[11] * m.x[14] + x[15] * m.x[15];

		return mf;
	}

	QMatrix4f operator*( const float f ) const
	{
		QMatrix4f m( *this );
		
		for( unsigned int y = 0; y < 4; ++y )
		{
			for( unsigned int x = 0; x < 4; ++x ) 
			{
				m.c[x][y] *= f;
			}
		}

		return m;
	}

	// ----------------------------
	// Vector-Matrix multiplication
	// ----------------------------
	QVec3f operator*( const QVec3f &v ) const
	{
		return QVec3f( v.X * c[0][0] + v.Y * c[1][0] + v.Z * c[2][0] + c[3][0],
					  v.X * c[0][1] + v.Y * c[1][1] + v.Z * c[2][1] + c[3][1],
					  v.X * c[0][2] + v.Y * c[1][2] + v.Z * c[2][2] + c[3][2] );
	}

	//Vec4f operator*( const Vec4f &v ) const
	//{
	//	return Vec4f( v.x * c[0][0] + v.y * c[1][0] + v.z * c[2][0] + c[3][0],
	//				  v.x * c[0][1] + v.y * c[1][1] + v.z * c[2][1] + c[3][1],
	//				  v.x * c[0][2] + v.y * c[1][2] + v.z * c[2][2] + c[3][2],
	//				  v.x * c[0][3] + v.y * c[1][3] + v.z * c[2][3] + c[3][3] );
	//}
	
	// ---------------
	// Transformations
	// ---------------
	void translate( const float x, const float y, const float z )
	{
		*this = TransMat( x, y, z ) * *this;
	}

	void translate( const QVec3f& trans )
	{
		*this = TransMat( trans.X, trans.Y, trans.Z ) * *this;
	}

	void scale( const float x, const float y, const float z )
	{
		*this = ScaleMat( x, y, z ) * *this;
	}

	void scale( const QVec3f& scale )
	{
		*this = ScaleMat( scale.X, scale.Y, scale.Z ) * *this;
	}

	void rotate( const float x, const float y, const float z )
	{
		*this = RotMat( x, y, z ) * *this;
	}

	void rotate( const QVec3f& rot )
	{
		*this = RotMat( rot.X, rot.Y, rot.Z) * *this;
	}

	// ---------------
	// Other
	// ---------------

	QMatrix4f transposed() const
	{
		QMatrix4f m( *this );
		
		for( unsigned int y = 0; y < 4; ++y )
		{
			for( unsigned int x = y + 1; x < 4; ++x ) 
			{
				float tmp = m.c[x][y];
				m.c[x][y] = m.c[y][x];
				m.c[y][x] = tmp;
			}
		}

		return m;
	}

	float determinant() const
	{
		return 
			c[0][3]*c[1][2]*c[2][1]*c[3][0] - c[0][2]*c[1][3]*c[2][1]*c[3][0] - c[0][3]*c[1][1]*c[2][2]*c[3][0] + c[0][1]*c[1][3]*c[2][2]*c[3][0] +
			c[0][2]*c[1][1]*c[2][3]*c[3][0] - c[0][1]*c[1][2]*c[2][3]*c[3][0] - c[0][3]*c[1][2]*c[2][0]*c[3][1] + c[0][2]*c[1][3]*c[2][0]*c[3][1] +
			c[0][3]*c[1][0]*c[2][2]*c[3][1] - c[0][0]*c[1][3]*c[2][2]*c[3][1] - c[0][2]*c[1][0]*c[2][3]*c[3][1] + c[0][0]*c[1][2]*c[2][3]*c[3][1] +
			c[0][3]*c[1][1]*c[2][0]*c[3][2] - c[0][1]*c[1][3]*c[2][0]*c[3][2] - c[0][3]*c[1][0]*c[2][1]*c[3][2] + c[0][0]*c[1][3]*c[2][1]*c[3][2] +
			c[0][1]*c[1][0]*c[2][3]*c[3][2] - c[0][0]*c[1][1]*c[2][3]*c[3][2] - c[0][2]*c[1][1]*c[2][0]*c[3][3] + c[0][1]*c[1][2]*c[2][0]*c[3][3] +
			c[0][2]*c[1][0]*c[2][1]*c[3][3] - c[0][0]*c[1][2]*c[2][1]*c[3][3] - c[0][1]*c[1][0]*c[2][2]*c[3][3] + c[0][0]*c[1][1]*c[2][2]*c[3][3];
	}

	QMatrix4f inverted() const
	{
		QMatrix4f m( false );

		float d = determinant();
		if( d == 0 ) return m;
		d = 1 / d;
		
		m.c[0][0] = d * (c[1][2]*c[2][3]*c[3][1] - c[1][3]*c[2][2]*c[3][1] + c[1][3]*c[2][1]*c[3][2] - c[1][1]*c[2][3]*c[3][2] - c[1][2]*c[2][1]*c[3][3] + c[1][1]*c[2][2]*c[3][3]);
		m.c[0][1] = d * (c[0][3]*c[2][2]*c[3][1] - c[0][2]*c[2][3]*c[3][1] - c[0][3]*c[2][1]*c[3][2] + c[0][1]*c[2][3]*c[3][2] + c[0][2]*c[2][1]*c[3][3] - c[0][1]*c[2][2]*c[3][3]);
		m.c[0][2] = d * (c[0][2]*c[1][3]*c[3][1] - c[0][3]*c[1][2]*c[3][1] + c[0][3]*c[1][1]*c[3][2] - c[0][1]*c[1][3]*c[3][2] - c[0][2]*c[1][1]*c[3][3] + c[0][1]*c[1][2]*c[3][3]);
		m.c[0][3] = d * (c[0][3]*c[1][2]*c[2][1] - c[0][2]*c[1][3]*c[2][1] - c[0][3]*c[1][1]*c[2][2] + c[0][1]*c[1][3]*c[2][2] + c[0][2]*c[1][1]*c[2][3] - c[0][1]*c[1][2]*c[2][3]);
		m.c[1][0] = d * (c[1][3]*c[2][2]*c[3][0] - c[1][2]*c[2][3]*c[3][0] - c[1][3]*c[2][0]*c[3][2] + c[1][0]*c[2][3]*c[3][2] + c[1][2]*c[2][0]*c[3][3] - c[1][0]*c[2][2]*c[3][3]);
		m.c[1][1] = d * (c[0][2]*c[2][3]*c[3][0] - c[0][3]*c[2][2]*c[3][0] + c[0][3]*c[2][0]*c[3][2] - c[0][0]*c[2][3]*c[3][2] - c[0][2]*c[2][0]*c[3][3] + c[0][0]*c[2][2]*c[3][3]);
		m.c[1][2] = d * (c[0][3]*c[1][2]*c[3][0] - c[0][2]*c[1][3]*c[3][0] - c[0][3]*c[1][0]*c[3][2] + c[0][0]*c[1][3]*c[3][2] + c[0][2]*c[1][0]*c[3][3] - c[0][0]*c[1][2]*c[3][3]);
		m.c[1][3] = d * (c[0][2]*c[1][3]*c[2][0] - c[0][3]*c[1][2]*c[2][0] + c[0][3]*c[1][0]*c[2][2] - c[0][0]*c[1][3]*c[2][2] - c[0][2]*c[1][0]*c[2][3] + c[0][0]*c[1][2]*c[2][3]);
		m.c[2][0] = d * (c[1][1]*c[2][3]*c[3][0] - c[1][3]*c[2][1]*c[3][0] + c[1][3]*c[2][0]*c[3][1] - c[1][0]*c[2][3]*c[3][1] - c[1][1]*c[2][0]*c[3][3] + c[1][0]*c[2][1]*c[3][3]);
		m.c[2][1] = d * (c[0][3]*c[2][1]*c[3][0] - c[0][1]*c[2][3]*c[3][0] - c[0][3]*c[2][0]*c[3][1] + c[0][0]*c[2][3]*c[3][1] + c[0][1]*c[2][0]*c[3][3] - c[0][0]*c[2][1]*c[3][3]);
		m.c[2][2] = d * (c[0][1]*c[1][3]*c[3][0] - c[0][3]*c[1][1]*c[3][0] + c[0][3]*c[1][0]*c[3][1] - c[0][0]*c[1][3]*c[3][1] - c[0][1]*c[1][0]*c[3][3] + c[0][0]*c[1][1]*c[3][3]);
		m.c[2][3] = d * (c[0][3]*c[1][1]*c[2][0] - c[0][1]*c[1][3]*c[2][0] - c[0][3]*c[1][0]*c[2][1] + c[0][0]*c[1][3]*c[2][1] + c[0][1]*c[1][0]*c[2][3] - c[0][0]*c[1][1]*c[2][3]);
		m.c[3][0] = d * (c[1][2]*c[2][1]*c[3][0] - c[1][1]*c[2][2]*c[3][0] - c[1][2]*c[2][0]*c[3][1] + c[1][0]*c[2][2]*c[3][1] + c[1][1]*c[2][0]*c[3][2] - c[1][0]*c[2][1]*c[3][2]);
		m.c[3][1] = d * (c[0][1]*c[2][2]*c[3][0] - c[0][2]*c[2][1]*c[3][0] + c[0][2]*c[2][0]*c[3][1] - c[0][0]*c[2][2]*c[3][1] - c[0][1]*c[2][0]*c[3][2] + c[0][0]*c[2][1]*c[3][2]);
		m.c[3][2] = d * (c[0][2]*c[1][1]*c[3][0] - c[0][1]*c[1][2]*c[3][0] - c[0][2]*c[1][0]*c[3][1] + c[0][0]*c[1][2]*c[3][1] + c[0][1]*c[1][0]*c[3][2] - c[0][0]*c[1][1]*c[3][2]);
		m.c[3][3] = d * (c[0][1]*c[1][2]*c[2][0] - c[0][2]*c[1][1]*c[2][0] + c[0][2]*c[1][0]*c[2][1] - c[0][0]*c[1][2]*c[2][1] - c[0][1]*c[1][0]*c[2][2] + c[0][0]*c[1][1]*c[2][2]);
		
		return m;
	}

	void decompose( QVec3f &trans, QVec3f &rot, QVec3f &scale ) const
	{
		// Getting translation is trivial
		trans = QVec3f( c[3][0], c[3][1], c[3][2] );

		// Scale is length of columns
		scale.X = sqrt( c[0][0] * c[0][0] + c[0][1] * c[0][1] + c[0][2] * c[0][2] );
		scale.Y = sqrt( c[1][0] * c[1][0] + c[1][1] * c[1][1] + c[1][2] * c[1][2] );
		scale.Z = sqrt( c[2][0] * c[2][0] + c[2][1] * c[2][1] + c[2][2] * c[2][2] );

		if( scale.X == 0 || scale.Y == 0 || scale.Z == 0 ) return;

		// Detect negative scale with determinant and flip one arbitrary axis
		if( determinant() < 0 ) scale.X = -scale.X;

		// Combined rotation matrix YXZ
		//
		// Cos[y]*Cos[z]+Sin[x]*Sin[y]*Sin[z]	Cos[z]*Sin[x]*Sin[y]-Cos[y]*Sin[z]	Cos[x]*Sin[y]	
		// Cos[x]*Sin[z]						Cos[x]*Cos[z]						-Sin[x]
		// -Cos[z]*Sin[y]+Cos[y]*Sin[x]*Sin[z]	Cos[y]*Cos[z]*Sin[x]+Sin[y]*Sin[z]	Cos[x]*Cos[y]

		rot.X = asinf( -c[2][1] / scale.Z );
		
		// Special case: Cos[x] == 0 (when Sin[x] is +/-1)
		float f = fabsf( c[2][1] / scale.Z );
		if( f > 0.999f && f < 1.001f )
		{
			// Pin arbitrarily one of y or z to zero
			// Mathematical equivalent of gimbal lock
			rot.Y = 0;
			
			// Now: Cos[x] = 0, Sin[x] = +/-1, Cos[y] = 1, Sin[y] = 0
			// => m[0][0] = Cos[z] and m[1][0] = Sin[z]
			rot.Z = atan2f( -c[1][0] / scale.Y, c[0][0] / scale.X );
		}
		// Standard case
		else
		{
			rot.Y = atan2f( c[2][0] / scale.Z, c[2][2] / scale.Z );
			rot.Z = atan2f( c[0][1] / scale.X, c[1][1] / scale.Y );
		}
	}

	QVec3f getTranslation() const
	{
		return QVec3f( x[12], x[13], x[14] );
	}

	QVec3f getRotation() const
	{
		QVec3f scale;
		// Scale is length of columns
		scale.X = sqrt( c[0][0] * c[0][0] + c[0][1] * c[0][1] + c[0][2] * c[0][2] );
		scale.Y = sqrt( c[1][0] * c[1][0] + c[1][1] * c[1][1] + c[1][2] * c[1][2] );
		scale.Z = sqrt( c[2][0] * c[2][0] + c[2][1] * c[2][1] + c[2][2] * c[2][2] );

		if( scale.X == 0 || scale.Y == 0 || scale.Z == 0 ) return QVec3f();

		// Detect negative scale with determinant and flip one arbitrary axis
		if( determinant() < 0 ) scale.X = -scale.X;

		QVec3f rot;
		rot.X = asinf( -c[2][1] / scale.Z );
		
		// Special case: Cos[x] == 0 (when Sin[x] is +/-1)
		float f = fabsf( c[2][1] / scale.Z );
		if( f > 0.999f && f < 1.001f )
		{
			// Pin arbitrarily one of y or z to zero
			// Mathematical equivalent of gimbal lock
			rot.Y = 0;
			
			// Now: Cos[x] = 0, Sin[x] = +/-1, Cos[y] = 1, Sin[y] = 0
			// => m[0][0] = Cos[z] and m[1][0] = Sin[z]
			rot.Z = atan2f( -c[1][0] / scale.Y, c[0][0] / scale.X );
		}
		// Standard case
		else
		{
			rot.Y = atan2f( c[2][0] / scale.Z, c[2][2] / scale.Z );
			rot.Z = atan2f( c[0][1] / scale.X, c[1][1] / scale.Y );
		}
		return rot;
	}

	QVec3f getScale() const
	{

		// Scale is length of columns
		return QVec3f(
			sqrt( c[0][0] * c[0][0] + c[0][1] * c[0][1] + c[0][2] * c[0][2] ),
			sqrt( c[1][0] * c[1][0] + c[1][1] * c[1][1] + c[1][2] * c[1][2] ),
			sqrt( c[2][0] * c[2][0] + c[2][1] * c[2][1] + c[2][2] * c[2][2] ));		
				
	}

	QMatrix4f getRotationMatrix() const
	{
		QVec3f scale = getScale();
		QMatrix4f rot;
		rot.c[0][0] = c[0][0] / scale.X; rot.c[0][1] = c[0][1] / scale.X; rot.c[0][2] = c[0][2] / scale.X;
		rot.c[1][0] = c[1][0] / scale.X; rot.c[1][1] = c[1][1] / scale.X; rot.c[1][2] = c[1][2] / scale.Y;
		rot.c[2][0] = c[2][0] / scale.X; rot.c[2][1] = c[2][1] / scale.X; rot.c[2][2] = c[2][2] / scale.Z;
		return rot;
	}

	//Vec4f getCol( unsigned int col )
	//{
	//	return Vec4f( x[col * 4 + 0], x[col * 4 + 1], x[col * 4 + 2], x[col * 4 + 3] );
	//}

	//Vec4f getRow( unsigned int row )
	//{
	//	return Vec4f( x[row + 0], x[row + 4], x[row + 8], x[row + 12] );
	//}
};
Q_DECLARE_METATYPE(QMatrix4f);

namespace CustomTypes
{
	void registerTypes();
	Property* createCustomProperty(const QString& name, QObject* propertyObject, Property* parent);

}

//Write definition for rounding function
extern "C" double roundIt(double x, double n);

#endif




