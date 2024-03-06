// from https://github.com/jbikker/bvh_article/, precomp.h
// matrix class
#ifndef MAT4_H
#define MAT4_H

class mat4
{
public:
	mat4() = default;
	float cell[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	float& operator [] ( const int idx ) { return cell[idx]; }
	float operator()( const int i, const int j ) const { return cell[i * 4 + j]; }
	float& operator()( const int i, const int j ) { return cell[i * 4 + j]; }
	mat4& operator += ( const mat4& a )
	{
		for (int i = 0; i < 16; i++) cell[i] += a.cell[i];
		return *this;
	}
	bool operator==( const mat4& m )
	{
		for (int i = 0; i < 16; i++) if (m.cell[i] != cell[i]) return false; return true;
	}
	vec3f GetTranslation() const { return vec3f{ cell[3], cell[7], cell[11] }; }
	static mat4 FromColumnMajor( const mat4& T )
	{
		mat4 M;
		M.cell[0] = T.cell[0], M.cell[1] = T.cell[4], M.cell[2] = T.cell[8], M.cell[3] = T.cell[12];
		M.cell[4] = T.cell[1], M.cell[5] = T.cell[5], M.cell[6] = T.cell[9], M.cell[7] = T.cell[13];
		M.cell[8] = T.cell[2], M.cell[9] = T.cell[6], M.cell[10] = T.cell[10], M.cell[11] = T.cell[14];
		M.cell[12] = T.cell[3], M.cell[13] = T.cell[7], M.cell[14] = T.cell[11], M.cell[15] = T.cell[15];
		return M;
	}
	constexpr static mat4 Identity() { return mat4{}; }
	static mat4 ZeroMatrix() { mat4 r; memset( r.cell, 0, 64 ); return r; }
	static mat4 RotateX( const float a ) { mat4 r; r.cell[5] = cosf( a ); r.cell[6] = -sinf( a ); r.cell[9] = sinf( a ); r.cell[10] = cosf( a ); return r; };
	static mat4 RotateY( const float a ) { mat4 r; r.cell[0] = cosf( a ); r.cell[2] = sinf( a ); r.cell[8] = -sinf( a ); r.cell[10] = cosf( a ); return r; };
	static mat4 RotateZ( const float a ) { mat4 r; r.cell[0] = cosf( a ); r.cell[1] = -sinf( a ); r.cell[4] = sinf( a ); r.cell[5] = cosf( a ); return r; };
	static mat4 Scale( const float s ) { mat4 r; r.cell[0] = r.cell[5] = r.cell[10] = s; return r; }
	static mat4 Scale( const vec3f s ) { mat4 r; r.cell[0] = s.x(), r.cell[5] = s.y(), r.cell[10] = s.z(); return r; }
	static mat4 Scale( const vec4f s ) { mat4 r; r.cell[0] = s.x(), r.cell[5] = s.y(), r.cell[10] = s.z(), r.cell[15] = s.w(); return r; }
	static mat4 Rotate( const vec3f& u, const float a ) { return Rotate( u.x(), u.y(), u.z(), a ); }
	static mat4 Rotate( const float x, const float y, const float z, const float a )
	{
		const float c = cosf( a ), l_c = 1 - c, s = sinf( a );
		// row major
		mat4 m;
		m[0] = x * x + (1 - x * x) * c, m[1] = x * y * l_c + z * s, m[2] = x * z * l_c - y * s, m[3] = 0;
		m[4] = x * y * l_c - z * s, m[5] = y * y + (1 - y * y) * c, m[6] = y * z * l_c + x * s, m[7] = 0;
		m[8] = x * z * l_c + y * s, m[9] = y * z * l_c - x * s, m[10] = z * z + (1 - z * z) * c, m[11] = 0;
		m[12] = m[13] = m[14] = 0, m[15] = 1;
		return m;
	}
	static mat4 LookAt( const vec3f P, const vec3f T )
	{
		const vec3f z = unit_vector( T - P );
		const vec3f x = unit_vector( cross( z, vec3f{ 0, 1, 0 } ) );
		const vec3f y = cross( x, z );
		mat4 M = Translate( P );
		M[0] = x.x(), M[4] = x.y(), M[8] = x.z();
		M[1] = y.x(), M[5] = y.y(), M[9] = y.z();
		M[2] = z.x(), M[6] = z.y(), M[10] = z.z();
		return M;
	}
	static mat4 LookAt( const vec3f& pos, const vec3f& look, const vec3f& up )
	{
		// PBRT's lookat
		mat4 cameraToWorld;
		// initialize fourth column of viewing matrix
		cameraToWorld( 0, 3 ) = pos.x();
		cameraToWorld( 1, 3 ) = pos.y();
		cameraToWorld( 2, 3 ) = pos.z();
		cameraToWorld( 3, 3 ) = 1;

		// initialize first three columns of viewing matrix
		vec3f dir = unit_vector( look - pos );
		vec3f right = cross( unit_vector( up ), dir );
		if (dot( right, right ) == 0)
		{
			printf(
				"\"up\" vector (%f, %f, %f) and viewing direction (%f, %f, %f) "
				"passed to LookAt are pointing in the same direction.  Using "
				"the identity transformation.\n",
				up.x(), up.y(), up.z(), dir.x(), dir.y(), dir.z() );
			return mat4();
		}
		right = unit_vector( right );
		vec3f newUp = cross( dir, right );
		cameraToWorld( 0, 0 ) = right.x(), cameraToWorld( 1, 0 ) = right.y();
		cameraToWorld( 2, 0 ) = right.z(), cameraToWorld( 3, 0 ) = 0.;
		cameraToWorld( 0, 1 ) = newUp.x(), cameraToWorld( 1, 1 ) = newUp.y();
		cameraToWorld( 2, 1 ) = newUp.z(), cameraToWorld( 3, 1 ) = 0.;
		cameraToWorld( 0, 2 ) = dir.x(), cameraToWorld( 1, 2 ) = dir.y();
		cameraToWorld( 2, 2 ) = dir.z(), cameraToWorld( 3, 2 ) = 0.;
		return cameraToWorld.Inverted();
	}
	static mat4 Translate( const float x, const float y, const float z ) { mat4 r; r.cell[3] = x; r.cell[7] = y; r.cell[11] = z; return r; };
	static mat4 Translate( const vec3f P ) { mat4 r; r.cell[3] = P.x(); r.cell[7] = P.y(); r.cell[11] = P.z(); return r; };
	float Trace3() const { return cell[0] + cell[5] + cell[10]; }

	mat4 Transposed() const
	{
		mat4 M;
		M[0] = cell[0], M[1] = cell[4], M[2] = cell[8];
		M[4] = cell[1], M[5] = cell[5], M[6] = cell[9];
		M[8] = cell[2], M[9] = cell[6], M[10] = cell[10];
		return M;
	}

	mat4 Inverted() const
	{
		// from MESA, via http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
		const float inv[16] = {
			cell[5] * cell[10] * cell[15] - cell[5] * cell[11] * cell[14] - cell[9] * cell[6] * cell[15] +
			cell[9] * cell[7] * cell[14] + cell[13] * cell[6] * cell[11] - cell[13] * cell[7] * cell[10],
			-cell[1] * cell[10] * cell[15] + cell[1] * cell[11] * cell[14] + cell[9] * cell[2] * cell[15] -
			cell[9] * cell[3] * cell[14] - cell[13] * cell[2] * cell[11] + cell[13] * cell[3] * cell[10],
			cell[1] * cell[6] * cell[15] - cell[1] * cell[7] * cell[14] - cell[5] * cell[2] * cell[15] +
			cell[5] * cell[3] * cell[14] + cell[13] * cell[2] * cell[7] - cell[13] * cell[3] * cell[6],
			-cell[1] * cell[6] * cell[11] + cell[1] * cell[7] * cell[10] + cell[5] * cell[2] * cell[11] -
			cell[5] * cell[3] * cell[10] - cell[9] * cell[2] * cell[7] + cell[9] * cell[3] * cell[6],
			-cell[4] * cell[10] * cell[15] + cell[4] * cell[11] * cell[14] + cell[8] * cell[6] * cell[15] -
			cell[8] * cell[7] * cell[14] - cell[12] * cell[6] * cell[11] + cell[12] * cell[7] * cell[10],
			cell[0] * cell[10] * cell[15] - cell[0] * cell[11] * cell[14] - cell[8] * cell[2] * cell[15] +
			cell[8] * cell[3] * cell[14] + cell[12] * cell[2] * cell[11] - cell[12] * cell[3] * cell[10],
			-cell[0] * cell[6] * cell[15] + cell[0] * cell[7] * cell[14] + cell[4] * cell[2] * cell[15] -
			cell[4] * cell[3] * cell[14] - cell[12] * cell[2] * cell[7] + cell[12] * cell[3] * cell[6],
			cell[0] * cell[6] * cell[11] - cell[0] * cell[7] * cell[10] - cell[4] * cell[2] * cell[11] +
			cell[4] * cell[3] * cell[10] + cell[8] * cell[2] * cell[7] - cell[8] * cell[3] * cell[6],
			cell[4] * cell[9] * cell[15] - cell[4] * cell[11] * cell[13] - cell[8] * cell[5] * cell[15] +
			cell[8] * cell[7] * cell[13] + cell[12] * cell[5] * cell[11] - cell[12] * cell[7] * cell[9],
			-cell[0] * cell[9] * cell[15] + cell[0] * cell[11] * cell[13] + cell[8] * cell[1] * cell[15] -
			cell[8] * cell[3] * cell[13] - cell[12] * cell[1] * cell[11] + cell[12] * cell[3] * cell[9],
			cell[0] * cell[5] * cell[15] - cell[0] * cell[7] * cell[13] - cell[4] * cell[1] * cell[15] +
			cell[4] * cell[3] * cell[13] + cell[12] * cell[1] * cell[7] - cell[12] * cell[3] * cell[5],
			-cell[0] * cell[5] * cell[11] + cell[0] * cell[7] * cell[9] + cell[4] * cell[1] * cell[11] -
			cell[4] * cell[3] * cell[9] - cell[8] * cell[1] * cell[7] + cell[8] * cell[3] * cell[5],
			-cell[4] * cell[9] * cell[14] + cell[4] * cell[10] * cell[13] + cell[8] * cell[5] * cell[14] -
			cell[8] * cell[6] * cell[13] - cell[12] * cell[5] * cell[10] + cell[12] * cell[6] * cell[9],
			cell[0] * cell[9] * cell[14] - cell[0] * cell[10] * cell[13] - cell[8] * cell[1] * cell[14] +
			cell[8] * cell[2] * cell[13] + cell[12] * cell[1] * cell[10] - cell[12] * cell[2] * cell[9],
			-cell[0] * cell[5] * cell[14] + cell[0] * cell[6] * cell[13] + cell[4] * cell[1] * cell[14] -
			cell[4] * cell[2] * cell[13] - cell[12] * cell[1] * cell[6] + cell[12] * cell[2] * cell[5],
			cell[0] * cell[5] * cell[10] - cell[0] * cell[6] * cell[9] - cell[4] * cell[1] * cell[10] +
			cell[4] * cell[2] * cell[9] + cell[8] * cell[1] * cell[6] - cell[8] * cell[2] * cell[5]
		};
		const float det = cell[0] * inv[0] + cell[1] * inv[4] + cell[2] * inv[8] + cell[3] * inv[12];
		mat4 retVal;
		if (det != 0)
		{
			const float invdet = 1.0f / det;
			for (int i = 0; i < 16; i++) retVal.cell[i] = inv[i] * invdet;
		}
		return retVal;
	}

	mat4 Inverted3x3() const
	{
		// via https://stackoverflow.com/questions/983999/simple-3x3-matrix-inverse-code-c
		const float invdet = 1.0f / (cell[0] * (cell[5] * cell[10] - cell[6] * cell[9]) -
		 cell[4] * (cell[1] * cell[10] - cell[9] * cell[2]) +
		 cell[8] * (cell[1] * cell[6] - cell[5] * cell[2]));
		mat4 R;
		R.cell[0] = (cell[5] * cell[10] - cell[6] * cell[9]) * invdet;
		R.cell[4] = (cell[8] * cell[6] - cell[4] * cell[10]) * invdet;
		R.cell[8] = (cell[4] * cell[9] - cell[8] * cell[5]) * invdet;
		R.cell[1] = (cell[9] * cell[2] - cell[1] * cell[10]) * invdet;
		R.cell[5] = (cell[0] * cell[10] - cell[8] * cell[2]) * invdet;
		R.cell[9] = (cell[1] * cell[8] - cell[0] * cell[9]) * invdet;
		R.cell[2] = (cell[1] * cell[6] - cell[2] * cell[5]) * invdet;
		R.cell[6] = (cell[2] * cell[4] - cell[0] * cell[6]) * invdet;
		R.cell[10] = (cell[0] * cell[5] - cell[1] * cell[4]) * invdet;
		return R;
	}

	inline vec3f TransformVector( const vec3f& v ) const
	{
		return vec3f{ cell[0] * v.x() + cell[1] * v.y() + cell[2] * v.z(),
			cell[4] * v.x() + cell[5] * v.y() + cell[6] * v.z(),
			cell[8] * v.x() + cell[9] * v.y() + cell[10] * v.z() };
	}

	inline vec3f TransformPoint( const vec3f& v ) const
	{
		const vec3f res = vec3f{
			cell[0] * v.x() + cell[1] * v.y() + cell[2] * v.z() + cell[3],
			cell[4] * v.x() + cell[5] * v.y() + cell[6] * v.z() + cell[7],
			cell[8] * v.x() + cell[9] * v.y() + cell[10] * v.z() + cell[11] };
		const float w = cell[12] * v.x() + cell[13] * v.y() + cell[14] * v.z() + cell[15];
		if (w == 1) return res;
		return (1.f / w) * res;
	}
};

mat4 operator*( const mat4& a, const mat4& b )
{
	mat4 r;
	for (uint i = 0; i < 16; i += 4)
		for (uint j = 0; j < 4; ++j)
		{
			r[i + j] =
				(a.cell[i + 0] * b.cell[j + 0]) +
				(a.cell[i + 1] * b.cell[j + 4]) +
				(a.cell[i + 2] * b.cell[j + 8]) +
				(a.cell[i + 3] * b.cell[j + 12]);
		}
	return r;
}
mat4 operator+( const mat4& a, const mat4& b )
{
	mat4 r;
	for (uint i = 0; i < 16; i += 4) r.cell[i] = a.cell[i] + b.cell[i];
	return r;
}
mat4 operator*( const mat4& a, const float s )
{
	mat4 r;
	for (uint i = 0; i < 16; i += 4) r.cell[i] = a.cell[i] * s;
	return r;
}
mat4 operator*( const float s, const mat4& a )
{
	mat4 r;
	for (uint i = 0; i < 16; i++) r.cell[i] = a.cell[i] * s;
	return r;
}
bool operator==( const mat4& a, const mat4& b )
{
	for (uint i = 0; i < 16; i++)
		if (a.cell[i] != b.cell[i]) return false;
	return true;
}
bool operator!=( const mat4& a, const mat4& b ) { return !(a == b); }
vec4f operator*( const mat4& a, const vec4f& b )
{
	return vec4f( a.cell[0] * b.x() + a.cell[1] * b.y() + a.cell[2] * b.z() + a.cell[3] * b.w(),
		a.cell[4] * b.x() + a.cell[5] * b.y() + a.cell[6] * b.z() + a.cell[7] * b.w(),
		a.cell[8] * b.x() + a.cell[9] * b.y() + a.cell[10] * b.z() + a.cell[11] * b.w(),
		a.cell[12] * b.x() + a.cell[13] * b.y() + a.cell[14] * b.z() + a.cell[15] * b.w() );
}
vec4f operator*( const vec4f& b, const mat4& a )
{
	return vec4f( a.cell[0] * b.x() + a.cell[1] * b.y() + a.cell[2] * b.z() + a.cell[3] * b.w(),
		a.cell[4] * b.x() + a.cell[5] * b.y() + a.cell[6] * b.z() + a.cell[7] * b.w(),
		a.cell[8] * b.x() + a.cell[9] * b.y() + a.cell[10] * b.z() + a.cell[11] * b.w(),
		a.cell[12] * b.x() + a.cell[13] * b.y() + a.cell[14] * b.z() + a.cell[15] * b.w() );
}

vec3f TransformPosition( const vec3f& a, const mat4& M )
{
	return vec3f(vec4f( a, 1 ) * M);
}

vec3f TransformVector( const vec3f& a, const mat4& M )
{
	return vec3f( vec4f( a, 0 ) * M );
}

#endif
