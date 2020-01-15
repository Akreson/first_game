
inline u32
FindLeastSignificantSetBit(u32 Source)
{
	u32 Result = 0;
	b32 Succes = _BitScanForward((unsigned long *)&Result, Source);

	// TODO: Handle not found case?
	Assert(Succes);

	return Result;
}

inline u32
FindMostSignificantSetBit(u32 Source)
{
	u32 Result = 0;
	b32 Succes = _BitScanReverse((unsigned long *)&Result, Source);

	// TODO: Handle not found case?
	Assert(Succes);

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