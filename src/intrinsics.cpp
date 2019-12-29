#include <intrin.h>

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
CountOfSetBits(u32 Value)
{
	u32 Result = 0;// _popcnt(Value);
	return Result;
}