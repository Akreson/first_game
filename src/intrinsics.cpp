
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
	Result.Succes = _BitScanForward((unsigned long *)&Result.Index, Source);

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