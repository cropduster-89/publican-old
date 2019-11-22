#ifndef MATHLIB_H
#define MATHLIB_H

#define TAU (_TAU())
extern inline float _TAU(void)
{
	return(6.28318530717958647692f);
}

#define PI (_PI())
extern inline float _PI(void)
{
	return(3.14159265358979323846f);
}

union vec2 {
	struct {
		float x;
		float y;
	};
	float all[2];
};

union vec3 {
	struct {
		float x, y, z;
	};
	struct {
		float u, v, w;
	};
	struct {
		float r, g ,b;
	};
	struct {
		union vec2 xy;
		float ignored0;
	};
	struct {
		float ignored1;
		union vec2 yz;				
	};
	struct {
		union vec2 uv;
		float ignored2;
	};
	struct {
		float ignored3;
		union vec2 vw;				
	};
	float all[3];
};

union vec4 {
	struct {
		union {
			union vec3 xyz;
			struct {
				float x, y, z;
			};
		};
		float w;
	};
	struct {
		union {
			union vec3 rgb;
			struct {
				float r, g, b;
			};
		};
		float a;
	};
	struct {
		union vec2 xy;
		float ignored0;
		float ignored1;
	};
	struct {
		float ignored2;
		union vec2 yz;				
		float ignored3;
	};
	struct {				
		float ignored4;
		float ignored5;
		union vec2 zw;
	};
	float all[4];
};

extern inline union vec2 RealToVec2(float a, 
				    float b)
{
	union vec2 result = {
		.x = a, 
		.y = b
	};
	return(result);	
}

extern inline union vec3 RealToVec3(float a, 
				    float b, 
				    float c)
{
	union vec3 result = {
		.x = a, 
		.y = b, 
		.z = c
	};
	return(result);	
}

extern inline union vec4 RealToVec4(float a, 
				    float b, 
				    float c, 
				    float d)
{
	union vec4 result = {
		.x = a, 
		.y = b, 
		.z = c, 
		.w = d
	};
	return(result);	
}

#define EMPTY_V2 (RealToVec2(0, 0))
#define EMPTY_V3 (RealToVec3(0, 0, 0))
#define EMPTY_V4 (RealToVec4(0, 0, 0 , 0))

#define DEF_OFFSET EMPTY_V3

extern inline union vec2 MultVec2(union vec2 a, 
				  float b)
{
	union vec2 result = {
		.x = a.x * b,
		.y = a.y * b
	};
	return(result);
}

extern inline union vec3 MultVec3(union vec3 a, 
				  float b)
{
	union vec3 result = {
		.x = a.x * b,
		.y = a.y * b,
		.z = a.z * b
	};
	return(result);
}

extern inline union vec4 MultVec4(union vec4 a, 
				  float b)
{
	union vec4 result = {
		.x = a.x * b, 
		.y = a.y * b, 
		.z = a.z * b, 
		.w = a.a * b
	};
	return(result);	
}

#define MULTVEC(a, b) _Generic((a), \
union vec2: MultVec2, \
union vec3: MultVec3, \
union vec4: MultVec4) ((a), (b)) 

extern inline union vec2 AddVec2(union vec2 a, 
				 union vec2 b)
{
	union vec2 result = {
		.x = a.x + b.x,
		.y = a.y + b.y
	};
	return(result);
}

extern inline union vec3 AddVec3(union vec3 a, 
				 union vec3 b)
{
	union vec3 result = {
		.x = a.x + b.x,
		.y = a.y + b.y,
		.z = a.z + b.z
	};
	return(result);
}

extern inline union vec4 AddVec4(union vec4 a, 
				 union vec4 b)
{
	union vec4 result = {
		.x = a.x + b.x,
		.y = a.y + b.y,
		.z = a.z + b.z,
		.w = a.w + b.w
	};
	return(result);
}

#define ADDVEC(a, b) _Generic((a), \
union vec2: AddVec2, \
union vec3: AddVec3, \
union vec4: AddVec4) ((a), (b))

extern inline union vec2 SubVec2(union vec2 a, 
				 union vec2 b)
{
	union vec2 result = {
		.x = a.x - b.x,
		.y = a.y - b.y
	};
	return(result);
}

extern inline union vec3 SubVec3(union vec3 a,
				 union vec3 b)
{
	union vec3 result = {
		.x = a.x - b.x,
		.y = a.y - b.y,
		.z = a.z - b.z
	};
	return(result);	
}

extern inline union vec4 SubVec4(union vec4 a, 
				 union vec4 b)
{
	union vec4 result = {
		.x = a.x - b.x,
		.y = a.y - b.y,
		.z = a.z - b.z,
		.w = a.w - b.w
	};
	return(result);
}

#define SUBVEC(a, b) _Generic((a), \
union vec2: SubVec2, \
union vec3: SubVec3, \
union vec4: SubVec4) ((a), (b))

extern inline union vec2 IntToVec2(int32_t a, 
				   int32_t b)
{
	union vec2 result = {.x = (float)a, .y = (float)b};
	return(result);	
}

extern inline float inner_v2(union vec2 a,
			     union vec2 b)
{
	float result = 
		a.x * b.x +
		a.y * b.y;
	return(result);
}

extern inline float lengthSq_v2(union vec2 a)
{
	float result = inner_v2(a, a);
	return(result);
}

extern inline float length_v2(union vec2 a)
{
	float result = sqrtf(lengthSq_v2(a));
	return(result);
}

extern inline union vec2 Hamadard(union vec2 a,
				  union vec2 b)
{
	union vec2 result = {
		.x = a.x * b.x,
		.y = a.y * b.y
	};
	
	return(result);
}

extern inline bool LessThanVec3(union vec3 a, union vec3 b)
{
	bool result = false;
	
	if(a.x < b.x && a.y < b.y && a.z < b.z) {result = true;}
	
	return(result);	
}

extern inline bool MoreThanVec3(union vec3 a, union vec3 b)
{
	bool result = false;
	
	if(a.x > b.x && a.y > b.y && a.z > b.z) {result = true;}
	
	return(result);	
}

extern inline union vec3 DivVec3(union vec3 a,
								 float b)
{
	union vec3 result = MultVec3(a , (1.0f / b));
	return(result);
}

extern inline union vec3 Hamadard3(union vec3 a,
								  union vec3 b)
{
	union vec3 result = {
		.x = a.x * b.x,
		.y = a.y * b.y,
		.z = a.z * b.z
	};
	
	return(result);
}

extern inline union vec3 Vec2ToVec3(union vec2 a, float b)
{
	union vec3 result = {.x = a.x, .y = a.y, .z = b};
	return(result);	
}

extern inline union vec3 NegVec3(union vec3 a)
{
	union vec3 result = {
		.x = -a.x,
		.y = -a.y,
		.z = -a.z
	};
	return(result);	
}

extern inline float inner_v3(union vec3 a,
			     union vec3 b)
{
	float result = 
		a.x * b.x +
		a.y * b.y +
		a.z * b.z;
	return(result);
}

extern inline float lengthSq_v3(union vec3 a)
{
		float result = inner_v3(a, a);
		return(result);
}

extern inline float length_v3(union vec3 a)
{
		float result = sqrtf(lengthSq_v3(a));
		return(result);
}

extern inline float Magnitude(union vec3 a)
{
	return(sqrtf(a.x * a.x + a.y * a.y + a.z * a.z));		
}

#define FORWARD_VEC (MouseForwardVector())
extern inline union vec3 MouseForwardVector(void)
{
	union vec3 result = {.x = 1, .y = 1, .z = -0.816};
	return(result);
}

#define BACKWARD_VEC (MouseBackwardVector())
extern inline union vec3 MouseBackwardVector(void)
{
	union vec3 result = {.x = -1, .y = -1, .z = 0.816 };
	return(result);
}

#define FORWARD_VEC_HALF (MouseForwardVectorHalf())
extern inline union vec3 MouseForwardVectorHalf(void)
{
	union vec3 result = {.x = 0.5f, .y = 0.5f, .z = -0.408 };
	return(result);
}

#define BACKWARD_VEC_HALF (MouseBackwardVectorHalf())
extern inline union vec3 MouseBackwardVectorHalf(void)
{
	union vec3 result = {.x = -0.5f, .y = -0.5f, .z = 0.408 };
	return(result);
}

extern inline union vec3 Normalise(union vec3 a)
{
	float mag = Magnitude(a);
	
	assert(mag);
	union vec3 result = {
		.x= a.x / mag,
		.y= a.y / mag,
		.z= a.z / mag,
	};
	return(result);
}

extern inline union vec3 NormVec3(union vec3 a)
{
	union vec3 result = {}; 
	float mag = length_v3(a);
	result.x = a.x * mag; 
	result.y = a.y * mag; 
	result.z = a.z * mag; 
	
	return(result);
}

extern inline union vec4 Vec2ToVec4(union vec2 a,
				    float b,
				    float c)
{
	union vec4 result = {
		.x = a.x,
		.y = a.y,
		.z = b,
		.w = c,
	};
	return(result);
}

extern inline union vec4 Vec3ToVec4(union vec3 a,
				    float b)
{
	union vec4 result = {
		.x = a.x,
		.y = a.y,
		.z = a.z,
		.w = a.w,
	};
	return(result);
}

extern inline void PrintVec(union vec3 a)
{
	printf("x = %f y = %f z = %f\n", a.x, a.y ,a.z);
}

struct point2 {
	int32_t x;
	int32_t y;
};

struct point3 {
	int32_t x;
	int32_t y;
	int32_t z;
};

extern inline struct point2 AddPoint2(struct point2 a,
				      struct point2 b)
{
	struct point2 result = {
		a.x + b.x, 
		a.y + b.y, 
	};
	return(result);
}

extern inline struct point3 AddPoint3(struct point3 a,
				      struct point3 b)
{
	struct point3 result = {
		a.x + b.x, 
		a.y + b.y,
		a.z + b.z
	};
	return(result);
}


extern inline struct point2 IntToPoint2(int32_t a,
									    int32_t b)
{
	struct point2 result = {
		a, 
		b
	};
	return(result);
}

extern inline struct point3 IntToPoint3(int32_t a,
				        int32_t b,
					int32_t c)
{
	struct point3 result = {
		a, 
		b,
		c
	};
	return(result);
}

extern inline struct point2 Vec2ToPoint2(union vec2 a)
{
	struct point2 result = {
		(int32_t)a.x, 
		(int32_t)a.y
	};
	return(result);
}


extern inline struct point3 Vec3ToPoint3(union vec3 a)
{
	struct point3 result = {
		(int32_t)a.x, 
		(int32_t)a.y,
		(int32_t)a.z,
	};
	return(result);
}

#define VEC_TO_POINT(a) _Generic((a), \
union vec2: Vec2ToPoint2, \
union vec3: Vec3ToPoint3) ((a))

extern inline bool CompPoint2(struct point2 a,
			      struct point2 b)
{
	return(a.x == b.x && a.y == b.y);
}

extern inline bool CompPoint3(struct point3 a,
			      struct point3 b)
{
	return(a.x == b.x && a.y == b.y && a.z == b.z);
}

#define COMP_POINT(a, b) _Generic((a), \
struct point2: CompPoint2, \
struct point3: CompPoint3) ((a), (b))

struct rect_int {
		int32_t minX, minY;
		int32_t maxX, maxY;
};

extern inline struct rect_int IntToRect(int32_t a, int32_t b, int32_t c, int32_t d)
{
		struct rect_int result = {
				a, b, c, d
		};
		return(result);
}

extern inline int32_t GetWidthI(struct rect_int a)
{
		int32_t result = a.maxX - a.minX;
		return(result);
}

extern inline int32_t GetHeightI(struct rect_int a)
{
		int32_t result = a.maxY - a.minY;
		return(result);
}

extern inline struct rect_int RectMinDimI(union vec2 min,
				  union vec2 dim)
{
		struct rect_int result; 
		
		result.minX = min.x;
		result.minY = min.y;
		result.maxX = min.x + dim.x;
		result.maxY = min.y + dim.y;
		
		return(result);
}

extern struct rect_int OffsetI(struct rect_int a,
							  int32_t x,
							  int32_t y)
{
		struct rect_int result = a;	

		result.minX += x;	
		result.maxX += x;	
		result.minY += y;	
		result.maxY += y;

		return(result);	
}
							
struct rect2 {
		union vec2 min;
		union vec2 max;
};

extern inline struct rect2 FloatToRect2(float a,
					float b,
					float c,
					float d)
{
	struct rect2 result = {
		{.x = a, .y = b},
		{.x = c, .y = d},
	};
	return(result);
}

extern inline union vec2 GetMinCorner2(struct rect2 rect)
{
		return(rect.min);	
}

extern inline union vec2 GetMaxCorner2(struct rect2 rect)
{
		return(rect.max);	
}

inline bool RectanglesIntersect2(struct rect2 a, struct rect2 b)
{
	bool result = !((b.max.x <= a.min.x) ||
			(b.min.x >= a.max.x) ||
			(b.max.y <= a.min.y) ||
			(b.min.y >= a.max.y));
	return(result);
}

extern inline struct rect2 RectCentreHalf(union vec2 centre,
										  union vec2 halfDim) 
{
		struct rect2 result;
		
		result.min = SubVec2(centre, halfDim);
		result.max = AddVec2(centre, halfDim);
		
		return(result);
}

extern inline struct rect2 Rect2MinMax(union vec2 min,
									   union vec2 max)
{
		struct rect2 result;

		result.min = min;	
		result.max = max;

		return(result);	
}

extern inline struct rect2 RectMinDim(union vec2 min,
				  union vec2 dim)
{
		struct rect2 result; 
		
		result.min = min;
		result.max = AddVec2(min, dim);
		
		return(result);
}

extern inline union vec2 GetDim2(struct rect2 rect)
{
		union vec2 result = SubVec2(rect.max, rect.min);
		return(result);
}

extern inline struct rect2 RectCentreHalfDim(union vec2 centre,
											 union vec2 halfDim)
{
		struct rect2 result;
		
		result.min = SubVec2(centre, halfDim);
		result.max = AddVec2(centre, halfDim);
		
		return(result);
}

extern inline struct rect2 Rect2CentreDim(union vec2 centre,
											 union vec2 dim)
{
		struct rect2 result = RectCentreHalfDim(centre, MultVec2(dim, 0.5f));
		
		return(result);
}

extern inline union vec2 GetCentre2(struct rect2 rect)
{
	union vec2 result = MultVec2(AddVec2(rect.min, rect.max), 0.5f);
	return(result);
}
			
struct rect3 {
	union vec3 min;
	union vec3 max;
};

struct cube3 {
	struct rect3 t;
	struct rect3 n;
	struct rect3 e;
	struct rect3 s;
	struct rect3 w;
	struct rect3 b;
};

extern inline struct rect3 Rect3CentreHalfDim(union vec3 centre,
					      union vec3 halfDim)
{
	struct rect3 result;
	
	result.min = SubVec3(centre, halfDim);
	result.max = AddVec3(centre, halfDim);
	
	return(result);
}

extern inline struct rect3 Rect3CentreDim(union vec3 centre,
					  union vec3 halfDim)
{
	struct rect3 result = Rect3CentreHalfDim(centre, MultVec3(halfDim, 0.5f));
	
	return(result);
}

extern inline struct rect3 Rect3MinMax(union vec3 min,
									  union vec3 max)
{
	struct rect3 result;
	
	result.min = min;
	result.max = max;
	
	return(result);
}

/********************************************************************************
		Matrix functions

********************************************************************************/

struct mat4 {
		float e[4][4];
};

struct mat4_inv {
		struct mat4 forward;
		struct mat4 inverse;
};

extern inline struct mat4_inv OrthoProj(float aspect, 
					float focal, 
					float nearClip,
					float farClip)
{
	float a = 1.0f;
	float b = aspect;
	float c = focal;
	
	float n = nearClip;
	float f = farClip;	

	float d = 2.0f / (n - f);
	float e = (n + f) / (n - f);
	
	struct mat4_inv r = {
		{{{(a * c), 0, 0, 0},
		   {0, (b * c), 0, 0},
		   {0, 0,  d,	e},
		   {0, 0, 0, 1}}},
		 
		 {{{1/(a * c), 0, 0, 0},
		   {0, 1/(b * c), 0, 0},
		   {0, 0,  1/d,	-e/d},
		   {0, 0, 0, 1}}}
	};

	return(r);
}

extern inline struct mat4_inv PerspProj(float aspect, 
					float focal, 
					float nearClip,
					float farClip)
{
	float a = 1.0f;
	float b = aspect;
	float c = focal;
	
	float n = nearClip;
	float f = farClip;	

	float d = (n+f) / (n-f);
	float e = (2*f*n) / (n-f);
	
	struct mat4_inv r = {
		{{{(a*c), 0, 0, 0},
		 {0, (b*c), 0, 0},
		 {0, 0,  d,	e},
		{0, 0, -1, 0}}},
		 
		 {{{1/(a*c), 0, 0, 0},
		 {0, 1/(b*c), 0, 0},
		 {0, 0,  0,	-1},
		 {0, 0, 1/e, d/e}}}
	};

	return(r);
}

extern inline struct mat4 Identity(void)
{
		struct mat4 r = {
			{{1, 0, 0, 0},
			 {0, 1, 0, 0},
			 {0, 0, 1, 0},
			 {0, 0, 0, 1}}
		};
		return(r);
}

extern inline struct mat4 RotX(float angle)
{
		float c = cos(angle);
		float s = sin(angle);
		
		struct mat4 r = {
			{{1, 0, 0, 0},
			 {0, c, -s, 0},
			 {0, s, c, 0},
			 {0, 0, 0, 1}}
		};
		return(r);
}

extern inline struct mat4 RotY(float angle)
{
		float c = cos(angle);
		float s = sin(angle);
		
		struct mat4 r = {
			{{c, 0, s, 0},
			 {0, 1, 0, 0},
			 {-s, 0, c, 0},
			 {0, 0, 0, 1}}
		};
		return(r);
}

extern inline struct mat4 RotZ(float angle)
{
		float c = cos(angle);
		float s = sin(angle);
		
		struct mat4 r = {
			{{c, -s, 0, 0},
			 {s, c, 0, 0},
			 {0, 0, 1, 0},
			 {0, 0, 0, 1}}
		};
		return(r);
}

extern inline struct mat4 Rows3x3(union vec3 x,
				  union vec3 y,
				  union vec3 z)
{
	struct mat4 r = {
		{{x.x, x.y, x.z , 0},
		 {y.x, y.y, y.z, 0},
		 {z.x, z.y, z.z, 0},
		 {0, 0, 0, 1}}
	};
	return(r);
}

extern inline struct mat4 Cols3x3(union vec3 x,
				  union vec3 y,
				  union vec3 z)
{
		struct mat4 r = {
			{{x.x, y.x, z.x, 0},
			 {x.y, y.y, z.y, 0},
			 {x.z, y.z, z.z, 0},
			 {0, 0, 0, 1}}
		};
		return(r);
}

extern union vec3 GetCol(struct mat4 a,
						 uint32_t b)
{
		union vec3 r = {
				.x = a.e[0][b], 
				.y = a.e[1][b], 
				.z = a.e[2][b]		
		};
		return(r);
}

extern union vec3 GetRow(struct mat4 a,
						 uint32_t row)
{
	union vec3 r = {
		.x = a.e[row][0], 
		.y = a.e[row][1], 
		.z = a.e[row][2]		
	};
	return(r);
}

extern inline struct mat4 Translate(struct mat4 a,  
			     union vec3 b)
{
		struct mat4 r = a;
		
		r.e[0][3] += b.x; 
		r.e[1][3] += b.y; 
		r.e[2][3] += b.z; 
		
		return(r);
}

extern union vec3 Transform(struct mat4 a,
			    union vec3 b,
			    float w)
{		
	union vec3 r;
	r.x = b.x * a.e[0][0] + b.y * a.e[0][1] + b.z * a.e[0][2] + w * a.e[0][3];
	r.y = b.x * a.e[1][0] + b.y * a.e[1][1] + b.z * a.e[1][2] + w * a.e[1][3];
	r.z = b.x * a.e[2][0] + b.y * a.e[2][1] + b.z * a.e[2][2] + w * a.e[2][3];
	return(r);
}

extern union vec4 Transform4(struct mat4 a,
							union vec4 b)
{
		
	union vec4 r;
	r.x = b.x * a.e[0][0] + b.y * a.e[0][1] + b.z * a.e[0][2] + b.w * a.e[0][3];
	r.y = b.x * a.e[1][0] + b.y * a.e[1][1] + b.z * a.e[1][2] + b.w * a.e[1][3];
	r.z = b.x * a.e[2][0] + b.y * a.e[2][1] + b.z * a.e[2][2] + b.w * a.e[2][3];
	r.w = b.x * a.e[3][0] + b.y * a.e[3][1] + b.z * a.e[3][2] + b.w * a.e[3][3];
	return(r);
}

extern inline struct mat4 Transpose(struct mat4 a)
{
	struct mat4 r;
	
	for(int j = 0; j <= 3; ++j) {
		for(int i = 0; i <= 3; ++i) {
			r.e[j][i] = a.e[i][j];
		}
	}
	return(r);
}

extern inline struct mat4 MultMat4(struct mat4 a,
				   struct mat4 b)
{
	struct mat4 r = {};
	for(int32_t i = 0; i < 4; ++i) {
		for(int32_t j = 0; j < 4; ++j) {
			for(int32_t k = 0; k < 4; ++k) {
				r.e[i][j] += a.e[i][k] * b.e[k][j];
			}
		}
	}
	return(r);
}

extern inline struct mat4_inv CamTrans(union vec3 x,
				       union vec3 y,
				       union vec3 z,
				       union vec3 p)
{
	struct mat4_inv result;
	
	struct mat4 a = Rows3x3(x, y, z);
	union vec3 ap = NegVec3(Transform(a, p, 1.0f));
	a = Translate(a, ap);
	result.forward = a;
	
	union vec3 iX = DivVec3(x, lengthSq_v3(x));
	union vec3 iY = DivVec3(y, lengthSq_v3(y));
	union vec3 iZ = DivVec3(z, lengthSq_v3(z));
	union vec3 iP = {
			.x = ap.x * iX.x + ap.y * iY.x + ap.z * iZ.x,
			.y = ap.x * iX.y + ap.y * iY.y + ap.z * iZ.y,
			.z = ap.x * iX.z + ap.y * iY.z + ap.z * iZ.z
	};
	struct mat4 b = Cols3x3(iX, iY, iZ);
	b = Translate(b, NegVec3(iP));
	result.inverse = b;
	
	return(result);
}

extern inline float DegreesToRads(float a)
{
	return(a * PI / 180);
}

extern inline void _BITTOGGLE_U8(uint8_t *a,
			     int32_t n)
{
	assert(n < sizeof(uint8_t) * 8);
	*a ^= (1 << n);
}

extern inline void _BITTOGGLE_U16(uint16_t *a,
			      int32_t n)
{
	assert(n < sizeof(uint16_t) * 8);
	*a ^= (1 << n);
}

extern inline void _BITTOGGLE_U32(uint32_t *a,
			      int32_t n)
{
	assert(n < sizeof(uint32_t) * 8);
	*a ^= (1 << n);
}

extern inline void _BITTOGGLE_U64(uint64_t *a,
			     int32_t n)
{
	assert(n < sizeof(uint64_t) * 8);
	*a ^= (1ul << n);
}

extern inline void _BITTOGGLE_S8(int8_t *a,
			     int32_t n)
{
	assert(n < sizeof(int8_t) * 8);
	*a ^= (1 << n);
}

extern inline void _BITTOGGLE_S16(int16_t *a,
			      int32_t n)
{
	assert(n < sizeof(int16_t) * 8);
	*a ^= (1 << n);
}

extern inline void _BITTOGGLE_S32(int32_t *a,
			          int32_t n)
{
	assert(n < sizeof(int32_t) * 8);
	*a ^= (1 << n);
}

extern inline void _BITTOGGLE_S64(int64_t *a,
			      int32_t n)
{
	assert(n < sizeof(int64_t) * 8);
	*a ^= (1ul << n);
}

#define BITTOGGLE(a, n) _Generic((a), \
uint8_t : _BITTOGGLE_U8, \
uint16_t : _BITTOGGLE_U16, \
uint32_t : _BITTOGGLE_U32, \
uint64_t : _BITTOGGLE_U64, \
int8_t : _BITTOGGLE_S8, \
int16_t : _BITTOGGLE_S16, \
int32_t : _BITTOGGLE_S32, \
int64_t : _BITTOGGLE_S64)  ((&a), (n))

extern inline void _BITCLEAR_U8(uint8_t *a,
			     int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a &= ~(1 << n);
}

extern inline void _BITCLEAR_U16(uint16_t *a,
			      int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a &= ~(1 << n);
}

extern inline void _BITCLEAR_U32(uint32_t *a,
			      int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a &= ~(1 << n);
}

extern inline void _BITCLEAR_U64(uint64_t *a,
			     int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a &= ~(1ul << n);
}

extern inline void _BITCLEAR_S8(int8_t *a,
			     int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a &= ~(1 << n);
}

extern inline void _BITCLEAR_S16(int16_t *a,
				 int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a &= ~(1 << n);
}

extern inline void _BITCLEAR_S32(int32_t *a,
				 int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a &= ~(1 << n);
}

extern inline void _BITCLEAR_S64(int64_t *a,
				 int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a &= ~(1ul << n);
}

#define BITCLEAR(a, n) _Generic((a), \
uint8_t 	: _BITCLEAR_U8, \
uint16_t 	: _BITCLEAR_U16, \
uint32_t 	: _BITCLEAR_U32, \
uint64_t 	: _BITCLEAR_U64, \
int8_t 		: _BITCLEAR_S8, \
int16_t 	: _BITCLEAR_S16, \
int32_t 	: _BITCLEAR_S32, \
int64_t 	: _BITCLEAR_S64)  ((&a), (n))

extern inline void _BITSET_U8(uint8_t *a,
			     int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a |= 1 << n;
}

extern inline void _BITSET_U16(uint16_t *a,
			      int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a |= 1 << n;
}

extern inline void _BITSET_U32(uint32_t *a,
			      int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a |= 1 << n;
}

extern inline void _BITSET_U64(uint64_t *a,
			     int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a |= 1ul << n;
}

extern inline void _BITSET_S8(int8_t *a,
			     int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a |= 1 << n;
}

extern inline void _BITSET_S16(int16_t *a,
			      int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a |= 1 << n;
}

extern inline void _BITSET_S32(int32_t *a,
			      int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a |= 1 << n;
}

extern inline void _BITSET_S64(int64_t *a,
			       int32_t n)
{
	assert(n < sizeof(*a) * 8);
	*a |= 1ul << n;
}

#define BITSET(a, n) _Generic((a), \
uint8_t : _BITSET_U8, \
uint16_t : _BITSET_U16, \
uint32_t : _BITSET_U32, \
uint64_t : _BITSET_U64, \
int8_t : _BITSET_S8, \
int16_t : _BITSET_S16, \
int32_t : _BITSET_S32, \
int64_t : _BITSET_S64)  ((&a), (n))

extern inline bool _BITCHECK_U8(uint8_t a,
			     int32_t n)
{
	assert(n < sizeof(a) * 8);
	return(a & (1 << n));
}

extern inline bool _BITCHECK_U16(uint16_t a,
			      int32_t n)
{
	assert(n < sizeof(a) * 8);
	return(a & (1 << n));
}

extern inline bool _BITCHECK_U32(uint32_t a,
			      int32_t n)
{
	assert(n < sizeof(a) * 8);
	return(a & (1 << n));
}

extern inline bool _BITCHECK_U64(uint64_t a,
			     int32_t n)
{
	assert(n < sizeof(a) * 8);
	return(a & (1ul << n));
}

extern inline bool _BITCHECK_S8(int8_t a,
			     int32_t n)
{
	assert(n < sizeof(a) * 8);
	return(a & (1 << n));
}

extern inline bool _BITCHECK_S16(int16_t a,
			      int32_t n)
{
	assert(n < sizeof(a) * 8);
	return(a & (1 << n));
}

extern inline bool _BITCHECK_S32(int32_t a,
			      int32_t n)
{
	assert(n < sizeof(a) * 8);
	return(a & (1 << n));
}

extern inline bool _BITCHECK_S64(int64_t a,
			      int32_t n)
{
	assert(n < sizeof(a) * 8);
	return(a & (1ul << n));
}

#define BITCHECK(a, n) _Generic((a), \
uint8_t : _BITCHECK_U8, \
uint16_t : _BITCHECK_U16, \
uint32_t : _BITCHECK_U32, \
uint64_t : _BITCHECK_U64, \
int8_t : _BITCHECK_S8, \
int16_t : _BITCHECK_S16, \
int32_t : _BITCHECK_S32, \
int64_t : _BITCHECK_S64)  ((a), (n))

#endif