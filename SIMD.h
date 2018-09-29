#ifndef RR_SIMD
#define RR_SIMD


typedef __m128 f32Wide;
typedef __m128i u32Wide;


static f32Wide Zerof32Wide()
{
	return _mm_setzero_ps();
}
static u32Wide ZeroU32Wide()
{
	return _mm_setzero_si128();
}
static f32Wide Load(f32 f)
{
	return _mm_set1_ps(f);
}
static f32Wide Load(f32 f1, f32 f2, f32 f3, f32 f4)
{
	return _mm_setr_ps(f1, f2, f3, f4);
}
static u32Wide Load(u32 u)
{
	return _mm_set1_epi32(u);
}

static u32Wide Load(u32 u1, u32 u2, u32 u3, u32 u4)
{
	return _mm_setr_epi32(u1, u2, u3, u4);
}
static u32Wide LoadFromArray(u32 *arr)
{
	return Load(arr[0], arr[1], arr[2], arr[3]);
}
static f32Wide operator*(f32Wide f1, f32Wide f2)
{
	return _mm_mul_ps(f1, f2);
}

static f32Wide operator*(f32Wide f1, f32 f2)
{
	return _mm_mul_ps(f1, _mm_set1_ps(f2));
}
static f32Wide operator*(f32 f2, f32Wide f1)
{
	return _mm_mul_ps(f1, _mm_set1_ps(f2));
}
static f32Wide CastToF32(u32Wide f)
{
	return _mm_cvtepi32_ps(f);
}
static f32Wide LoadU32(u32 i)
{
	return CastToF32(Load(i));
}

static f32Wide operator/(f32Wide f1, f32Wide f2)
{
	return _mm_div_ps(f1, f2);
}
static f32Wide operator/(f32Wide f1, f32 f2)
{
	return _mm_div_ps(f1, Load(f2));
}
static f32Wide operator/(f32 f1, f32Wide f2)
{
	return _mm_div_ps(Load(f1), f2);
}


static f32Wide operator+(f32Wide f1, f32Wide f2)
{
	return _mm_add_ps(f1, f2);
}
static u32Wide operator+(u32Wide f1, u32Wide f2)
{
	return _mm_add_epi32(f1, f2);
}
static f32Wide operator-(f32Wide f1, f32Wide f2)
{
	return _mm_sub_ps(f1, f2);
}
static f32Wide operator-(f32Wide f)
{
	return (_mm_setzero_ps() - f);
}
static f32Wide Min(f32Wide f1, f32Wide f2)
{
	return _mm_min_ps(f1, f2);
}
static f32Wide Max(f32Wide f1, f32Wide f2)
{
	return _mm_max_ps(f1, f2);
}
static f32Wide Min(f32Wide f1, f32 f2)
{
	return _mm_min_ps(f1, Load(f2));
}
static f32Wide Max(f32Wide f1, f32 f2)
{
	return _mm_max_ps(f1, Load(f2));
}
static f32Wide Min(f32 f1, f32Wide f2)
{
	return _mm_min_ps(Load(f1), f2);
}
static f32Wide Max(f32 f1, f32Wide f2)
{
	return _mm_max_ps(Load(f1), f2);
}
static f32Wide Clamp(f32Wide val, f32Wide min, f32Wide max)
{
	return Min(Max(val, min), max);
}
static f32Wide Clamp(f32Wide val, f32 min, f32 max)
{
	return Min(Max(val, min), max);
}
static f32Wide Sqrt(f32Wide f1)
{
	return _mm_sqrt_ps(f1);
}

static u32Wide operator<(f32Wide f1, f32Wide f2)
{
	return _mm_castps_si128(_mm_cmplt_ps(f1, f2));
}
static u32Wide operator>(f32Wide f1, f32Wide f2)
{
	return _mm_castps_si128(_mm_cmpgt_ps(f1, f2));
}
static u32Wide operator>=(f32Wide f1, f32Wide f2)
{
	return _mm_castps_si128(_mm_cmpge_ps(f1, f2));
}
static u32Wide operator<=(f32Wide f1, f32Wide f2)
{
	return _mm_castps_si128(_mm_cmple_ps(f1, f2));
}
static u32Wide operator==(f32Wide f1, f32Wide f2)
{
	return _mm_castps_si128(_mm_cmpeq_ps(f1, f2));
}
static u32Wide operator!=(f32Wide f1, f32Wide f2)
{
	return _mm_castps_si128(_mm_cmpneq_ps(f1, f2));
}

static u32Wide operator<(u32Wide u1, u32Wide u2)
{
	return _mm_cmplt_epi32(u1, u2);
}
static u32Wide operator>(u32Wide u1, u32Wide u2)
{
	return _mm_cmpgt_epi32(u1, u2);
}
static u32Wide operator==(u32Wide u1, u32Wide u2)
{
	return _mm_cmpeq_epi32(u1, u2);
}
static u32Wide operator^(u32Wide a, u32Wide b)
{
	return _mm_xor_si128(a, b);
}

static u32Wide operator&(u32Wide a, u32Wide b)
{
	return _mm_and_si128(a, b);
}

//(not b) and a
static u32Wide AndNot(u32Wide a, u32Wide b)
{
	return _mm_andnot_si128(b, a); 
}

static f32Wide operator&(u32Wide a, f32Wide b)
{
	return _mm_and_ps(_mm_castsi128_ps(a), b);
}
static u32Wide operator|(u32Wide a, u32Wide b)
{
	return _mm_or_si128(a, b);
}
static u32Wide operator<<(u32Wide a, u32 shift)
{
	return _mm_slli_epi32(a, shift);
}
static u32Wide operator>>(u32Wide a, u32 shift)
{
	return _mm_srli_epi32(a, shift);
}

struct v3Wide
{
	f32Wide x;
	f32Wide y;
	f32Wide z;
};

static v3Wide ZeroV3Wide()
{
	return {Zerof32Wide(), Zerof32Wide(), Zerof32Wide() };
}

static v3Wide Zerov3Wide()
{
	return { Zerof32Wide(), Zerof32Wide(), Zerof32Wide() };
}
static v3Wide Load(v3 a)
{
	return {Load(a.x), Load(a.y), Load(a.z)};
}

static v3Wide Load(v3 a, v3 b, v3 c, v3 d)
{
	v3Wide ret;
	ret.x = Load(a.x, b.x, c.x, d.x);
	ret.y = Load(a.y, b.y, c.y, d.y);
	ret.z = Load(a.z, b.z, c.z, d.z);
	return ret;
}
#define LoadFromStruct(str, thing) Load(str[0]->thing, str[1]->thing, str[2]->thing, str[3]->thing)
static v3Wide LoadFromArray(v3 *arr)
{
	return Load(arr[0], arr[1], arr[2], arr[3]);
}

static v3Wide V3(f32Wide x, f32Wide y, f32Wide z)
{
	v3Wide ret;
	ret.x = x;
	ret.y = y;
	ret.z = z;
	return ret;
}

static v3Wide operator*(v3Wide a, f32Wide f)
{
	return {_mm_mul_ps(a.x, f), _mm_mul_ps(a.y, f) ,_mm_mul_ps(a.z, f) };
}
static v3Wide operator*(f32Wide f, v3Wide a)
{
	/*v3Wide ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	ret.z = a.z * f;
	return ret;*/
	return { _mm_mul_ps(a.x, f), _mm_mul_ps(a.y, f) ,_mm_mul_ps(a.z, f) };
}
static v3Wide operator*(v3Wide a, f32 f)
{
	v3Wide ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	ret.z = a.z * f;
	return ret;
}

static v3Wide operator*(f32 f, v3Wide a)
{
	v3Wide ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	ret.z = a.z * f;
	return ret;
}

static v3Wide operator/(v3Wide a, f32Wide f)
{
	v3Wide ret;
	ret.x = a.x / f;
	ret.y = a.y / f;
	ret.z = a.z / f;
	return ret;
}
static v3Wide operator*(v3Wide a, v3Wide b)
{
	v3Wide ret;
	ret.x = a.x * b.x;
	ret.y = a.y * b.y;
	ret.z = a.z * b.z;
	return ret;
}

static v3Wide operator+(v3Wide a, v3Wide b)
{
	v3Wide ret;
	ret.x = a.x + b.x;
	ret.y = a.y + b.y;
	ret.z = a.z + b.z;
	return ret;
}
static v3Wide operator-(v3Wide a, v3Wide b)
{
	v3Wide ret;
	ret.x = a.x - b.x;
	ret.y = a.y - b.y;
	ret.z = a.z - b.z;
	return ret;
}
static v3Wide& operator+=(v3Wide& a, v3Wide b)
{
	a = a + b;
	return a;
}
static v3Wide& operator-=(v3Wide& a, v3Wide b)
{
	a = a - b;
	return a;
}
static v3Wide& operator*=(v3Wide& a, f32Wide b)
{
	a = a * b;
	return a;
}
static v3Wide operator-(v3Wide a)
{
	v3Wide ret;
	ret.x = -a.x;
	ret.y = -a.y;
	ret.z = -a.z;
	return ret;
}
static v3Wide LerpV3Wide(v3Wide a, v3Wide b, f32Wide percent)
{
	return (Load(1.0f) - percent) * a + percent * b;
}

static f32Wide Dot(v3Wide a, v3Wide b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

static f32Wide Norm(v3Wide a)
{
	return Sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}
static void ConditionalAssign(v3Wide *dest, u32Wide mask, v3Wide source)
{
	__m128 MaskPS = _mm_castsi128_ps(mask);

	dest->x = _mm_or_ps(_mm_andnot_ps(MaskPS, dest->x), _mm_and_ps(MaskPS, source.x));
	dest->y = _mm_or_ps(_mm_andnot_ps(MaskPS, dest->y), _mm_and_ps(MaskPS, source.y));
	dest->z = _mm_or_ps(_mm_andnot_ps(MaskPS, dest->z), _mm_and_ps(MaskPS, source.z));
}
static void ConditionalAssign(f32Wide *dest, u32Wide mask, f32Wide source)
{
	__m128 MaskPS = _mm_castsi128_ps(mask);

	*dest = _mm_or_ps(_mm_andnot_ps(MaskPS, *dest), _mm_and_ps(MaskPS, source));
}
static void ConditionalAssign(u32Wide *dest, u32Wide mask, u32Wide source)
{	
	*dest = _mm_or_si128(_mm_andnot_si128(mask, *dest), _mm_and_si128(mask, source));
}

static v3Wide NOZ(v3Wide a)
{
	v3Wide ret = Load(V3(0, 0, 0));

	f32Wide norm = Norm(a);
	u32Wide mask = (norm > Load(0.0001f));
	f32Wide oneDivNorm = (1.0f / norm);
	v3Wide normalizedVector = oneDivNorm * a;
	ConditionalAssign(&ret, mask, normalizedVector);

	return ret;
}
static f32Wide QuadNorm(v3Wide a)
{
	return ((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
}
static f32Wide NormSquared(v3Wide a)
{
	return ((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
}
static f32Wide QuadDist(v3Wide a, v3Wide b)
{
	return QuadNorm(a - b);
}
static v3Wide FastNormalize(v3Wide a)
{
	f32Wide quadNorm = QuadNorm(a);
	f32Wide inverseNorm = _mm_rsqrt_ps(quadNorm);

	return  (a * inverseNorm);
}

static v3Wide Unpack3x8(u32Wide *pack)
{
	u32Wide OxFF = Load(0xFFu);

	f32Wide ignored = CastToF32((*pack >> 24) & OxFF) / 255.0f;
	f32Wide r = CastToF32((*pack >> 16) & OxFF) / 255.0f;
	f32Wide g = CastToF32((*pack >> 8) & OxFF) / 255.0f;
	f32Wide b = CastToF32((*pack >> 0) & OxFF) / 255.0f;
	return V3(r, g, b);
}

static f32 Lane(f32Wide f, u32 lane)
{
	return ((f32 *)(&f))[lane];
}
static u32 Lane(u32Wide f, u32 lane)
{
	return ((u32 *)(&f))[lane];
}
static v3 Lane(v3Wide v, u32 lane)
{
	return V3(Lane(v.x, lane), Lane(v.y, lane), Lane(v.z, lane));
}

static f32 SumLanes(f32Wide f)
{
	f32 *val = (f32 *)&f;

	return (val[0] + val[1] + val[2] + val[3]);
}
static v3 SumLanes(v3Wide f)
{	
	return { SumLanes(f.x), SumLanes(f.y), SumLanes(f.z) };
}
static bool MaskIsZero(u32Wide a)
{
	i32 Mask = _mm_movemask_epi8(a);
	return(Mask == 0);
}

static bool AnyTrue(u32Wide a)
{
	i32 Mask = _mm_movemask_epi8(a);
	return !(Mask == 0);
}
static bool AnyFalse(u32Wide a)
{
	i32 Mask = _mm_movemask_epi8(a);
	return !(Mask == 0xffff);
}
static bool AllFalse(u32Wide a)
{
	i32 Mask = _mm_movemask_epi8(a);
	return (Mask == 0);
}
static bool AllTrue(u32Wide a)
{
	i32 Mask = _mm_movemask_epi8(a);
	return (Mask == 0xffff);
}

static u32 FirstZeroLane(u32Wide a)
{
	i32 mask = _mm_movemask_epi8(a);

	if (!(mask & 0xf000)) return 0;
	if (!(mask & 0xf00)) return 1;
	if (!(mask & 0xf0)) return 2;
	if (!(mask & 0xf)) return 3;


	return -1;
}

#endif