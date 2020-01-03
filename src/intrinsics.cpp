
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