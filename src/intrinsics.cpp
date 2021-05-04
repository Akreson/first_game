
#define ShuffleU32(A, S0, S1, S2, S3) \
	_mm_shuffle_epi32(A, (S0 << 0) | (S1 << 2) | (S2 << 4) | (S3 << 6))

#define ShuffleU32_4x(A, S) \
	_mm_shuffle_epi32(A, (S << 0) | (S << 2) | (S << 4) | (S << 6))

#define ShuffleF32(A, B, S0, S1, S2, S3) \
	_mm_shuffle_ps(A, B, (S0 << 0) | (S1 << 2) | (S2 << 4) | (S3 << 6))

#define iToMaskU32(A) _mm_movemask_ps(_mm_castsi128_ps((A)))

struct bit_scan_result
{
	u16 Index;
	u16 Succes;
};

inline bit_scan_result
FindLeastSignificantSetBit(u32 Source)
{
	bit_scan_result Result;
	Result.Succes = _BitScanForward((unsigned long *)&Result.Index, Source);

	return Result;
}

inline bit_scan_result
FindMostSignificantSetBit(u32 Source)
{
	bit_scan_result Result;
	Result.Succes = _BitScanReverse((unsigned long *)&Result.Index, Source);

	return Result;
}

inline u32
CountOfSetBits(u32 Value)
{
	u32 Result = __popcnt(Value);
	return Result;
}

inline f32
SquareRoot(f32 Value)
{
	f32 Result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ps1(Value)));
	return Result;
}