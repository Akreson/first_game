#pragma once

struct memory_arena
{
	memory_index Size;
	memory_index Used;
	u8 *Base;
	u16 CountOfTempMem;
};

struct temp_memory
{
	memory_arena *Arena;
	u32 Used;
};

//TODO: Combine AllocStatus and PoolAllocInfo??
struct page_memory_arena
{
	void *Base;
	u32 *AllocStatus; // NOTE: One bit status: 0 - used, 1 - unused (for now)
	s16 *UsedStatus; // per page, must be signed
	s16 *PoolAllocInfo;//
	u32 PageSize; // NOTE: Must be power of 2
	u32 AllocStatusBlocksCount;
	u16 PageCount;
	u16 UnusedSpace;
};

enum PageStatus
{
	PageStatus_Used,
	PageStatus_Unused
};

#define PAGES_PER_ALLOC_STATUS_BLOCK (sizeof(*((page_memory_arena *)0)->AllocStatus) * 8)

#define PushSize(Arena, Size, ...) PushSize_(Arena, Size, ##__VA_ARGS__)
#define PushStruct(Arena, type, ...) (type *)PushSize_(Arena, sizeof(type), ##__VA_ARGS__)
#define PushArray(Arena, type, Count, ...) (type *)PushSize_(Arena, (Count)*sizeof(type), ##__VA_ARGS__)

#define PagePushSize(Arena, Size, Dest, Source) PushSize_(Arena, Size, (void **)&Dest, (void *)Source)
#define PagePushStruct(Arena, type, Dest, Source) PushSize_(Arena, sizeof(type), (void **)&Dest, (void *)Source)
#define PagePushArray(Arena, type, Count, Dest, Source) PushSize_(Arena, sizeof(type)*Count, (void **)&Dest, (void *)Source)

// NOTE: Just allocate space
void *
PushSize_(memory_arena *Arena, u32 Size, u32 Alignment = 4)
{
	memory_index CurrentArenaPtr = (memory_index)Arena->Base + Arena->Used;

	u32 AlignOffset = GetAlignmentOffsetForwad(CurrentArenaPtr, Alignment);

	u32 TotalAddedSize = Size + AlignOffset;

	if (!(Arena->Size >= (Arena->Used + TotalAddedSize))) { Assert(0); }

	void *Result = (void *)(Arena->Base + Arena->Used + AlignOffset);
	Arena->Used += TotalAddedSize;

	return Result;
}

inline void
InitArena(memory_arena *Arena, memory_index Size, u8 *Base)
{
	Arena->Base = Base;
	Arena->Size = Size;
	Arena->Used = 0;
}

memory_arena
SubArena(memory_arena *Arena, umm Size)
{
	memory_arena Result = {};

	Result.Size = Size;
	Result.Base = (u8 *)PushSize(Arena, Size, 64);

	return Result;
}

inline temp_memory
BeginTempMemory(memory_arena *Arena)
{
	temp_memory Result;

	Result.Arena = Arena;
	Result.Used = Arena->Used;

	++Arena->CountOfTempMem;

	return Result;
}

inline void
EndTempMemory(temp_memory TempMem)
{
	memory_arena *Arena = TempMem.Arena;

	Assert(Arena->Used >= TempMem.Used);
	Arena->Used = TempMem.Used;

	Assert(Arena->CountOfTempMem > 0);
	--Arena->CountOfTempMem;
}

internal inline u32
GetPageIndex(void *Ptr, void *PageBase, u32 PageSize)
{
	Assert(IsPowerOf2(PageSize));
	Assert(IsAligned(PageBase, PageSize));

	u32 AlignOffset = GetAlignmentOffsetBack((memory_index)Ptr, PageSize);
	u8 *AlignPtr = (u8 *)Ptr - AlignOffset;

	u32 ShiftValue = FindLeastSignificantSetBit(PageSize);

	u32 Result = AlignPtr - (u8 *)PageBase;
	Result >>= ShiftValue;

	return Result;
}

internal inline u8 *
GetPageBaseFromPageIndex(page_memory_arena *Arena, u32 PageIndex)
{
	u8 *Result = (u8 *)Arena->Base + (PageIndex * Arena->PageSize);
	return Result;
}

// TODO: Find way to distribute block search for prevent segmentation?
internal b32
FindFreePages(page_memory_arena *Arena, u32 *StartPageIndex, u32 NeededAmount = 1)
{
	u32 StartOfSequence = 0;
	u32 PageOffsetFactor = 0;
	u32 PrevFreePageIndex = 0;
	u32 Sequence = 1;
	b32 Success = false;

	for (u32 BlockIndex = 0;
		BlockIndex <= Arena->AllocStatusBlocksCount, !Success;
		++BlockIndex)
	{
		u32 PageBlock = Arena->AllocStatus[BlockIndex];

		if (PageBlock)
		{
			PageOffsetFactor = BlockIndex * PAGES_PER_ALLOC_STATUS_BLOCK;

			u32 PrevFreePageIndexInBlock = FindLeastSignificantSetBit(PageBlock);
			PrevFreePageIndex = PageOffsetFactor + PrevFreePageIndexInBlock;

			PageBlock = ResetBit(PageBlock, PrevFreePageIndexInBlock);

			StartOfSequence = PrevFreePageIndex;

			while (PageBlock)
			{
				if (Sequence == NeededAmount)
				{
					*StartPageIndex = StartOfSequence;
					Success = true;
					break;
				}

				u32 FreePageIndexInBlock = FindLeastSignificantSetBit(PageBlock);
				u32 FreePageIndex = PageOffsetFactor + FreePageIndexInBlock;

				PageBlock = ResetBit(PageBlock, FreePageIndexInBlock);

				if (FreePageIndex == (PrevFreePageIndex + 1))
				{
					Sequence++;
				}
				else
				{
					Sequence = 1;
					StartOfSequence = FreePageIndex;
				}

				PrevFreePageIndex = FreePageIndex;
			}
		}
	}

	return Success;
}

internal inline void
CalcBlockInfoFromPageIndex(u32 PageIndex, u32 *BlockIndex, u32 *InBlockIndex)
{
	*BlockIndex = PageIndex / PAGES_PER_ALLOC_STATUS_BLOCK;
	*InBlockIndex = PageIndex - (*BlockIndex * PAGES_PER_ALLOC_STATUS_BLOCK);
}

internal inline u32
CountOfOverlapedPageBlock(u32 InBlockIndex, u32 CountOfPages)
{
	u32 Result = (InBlockIndex + CountOfPages) / PAGES_PER_ALLOC_STATUS_BLOCK;
	Result = Result ? Result : 1;
	return Result;
}

internal inline void
SetPagesStatus(page_memory_arena *Arena, u32 StartPageIndex, u32 PageStatus, u32 CountOfPages = 1)
{
	u32 StartBlockIndex;
	u32 StartInBlockIndex;

	CalcBlockInfoFromPageIndex(StartPageIndex, &StartBlockIndex, &StartInBlockIndex);

	u32 PagesSet = 0;
	u32 BlockCount = CountOfOverlapedPageBlock(StartInBlockIndex, CountOfPages);
	for (u32 BlockIndex = StartBlockIndex;
		BlockCount;
		--BlockCount)
	{
		u32 PageBlock = Arena->AllocStatus[BlockIndex];

		for (u32 InBlockIndex = StartInBlockIndex;
			(PagesSet < CountOfPages) && (InBlockIndex < PAGES_PER_ALLOC_STATUS_BLOCK);
			PagesSet++, InBlockIndex++)
		{
			if (PageStatus) PageBlock = SetBit(PageBlock, InBlockIndex);
			else PageBlock = ResetBit(PageBlock, InBlockIndex);
		}

		StartInBlockIndex = 0;
		Arena->AllocStatus[BlockIndex] = PageBlock;
	}
}

internal inline b32
AllocateNextPagesIfItFree(page_memory_arena *Arena, u32 StartPageIndexInPool, u32 CountOfPages = 1)
{
	b32 Success = true;

	u32 Block;
	u32 StartInBlockIndex;

	u32 LastPageInPool = StartPageIndexInPool + (Arena->PoolAllocInfo[StartPageIndexInPool] - 1);
	CalcBlockInfoFromPageIndex(LastPageInPool, &Block, &StartInBlockIndex);
	StartInBlockIndex++;

	if (StartInBlockIndex >= PAGES_PER_ALLOC_STATUS_BLOCK)
	{
		StartInBlockIndex -= PAGES_PER_ALLOC_STATUS_BLOCK;
		Block++;
	}

	u32 PagesTest = 0;
	u32 InitInBlockIndex = StartInBlockIndex;
	u32 BlockCount = CountOfOverlapedPageBlock(StartInBlockIndex, CountOfPages);
	for (u32 BlockIndex = Block;
		BlockCount, Success;
		--BlockCount)
	{
		u32 PageBlock = Arena->AllocStatus[BlockIndex];

		for (u32 InBlockIndex = InitInBlockIndex;
			(PagesTest < CountOfPages) && (InBlockIndex == PAGES_PER_ALLOC_STATUS_BLOCK);
			PagesTest++, InBlockIndex++)
		{
			if (!IsBitSet(PageBlock, InBlockIndex))
			{
				Success = false;
				break;
			}
		}

		InitInBlockIndex = 0;
	}

	if (PagesTest == CountOfPages)
	{
		SetPagesStatus(Arena, LastPageInPool + 1, PageStatus_Used, CountOfPages);
	}

	return Success;
}

internal inline u32
AllocatePagePool(page_memory_arena *Arena, u32 PageCount)
{
	u32 StartPagePoolIndex;

	if (FindFreePages(Arena, &StartPagePoolIndex, PageCount))
	{
		SetPagesStatus(Arena, StartPagePoolIndex, PageStatus_Used, PageCount);
		Arena->PoolAllocInfo[StartPagePoolIndex] = PageCount;
	}
	else
	{
		Assert(0);
	}

	return StartPagePoolIndex;
}

internal inline void
DeallocatePagePool(page_memory_arena *Arena, u32 PageIndex)
{
	u32 PagesInPool = Arena->PoolAllocInfo[PageIndex];

	Arena->UsedStatus[PageIndex] = 0;
	Arena->PoolAllocInfo[PageIndex] = 0;
	
	SetPagesStatus(Arena, PageIndex, PageStatus_Unused, PagesInPool);
}

// NOTE: Allocate and copy memory
// TODO: Should copy memory??
void *
PushSize_(page_memory_arena *Arena, u32 Size, void **Dest, void *Source)
{
	u32 PageIndex;

	u8 *PageBase = (u8 *)*Dest;

	u32 UsedPagesBySize = Size / Arena->PageSize;
	memory_index Remainder = (Size - (UsedPagesBySize * Arena->PageSize));
	if (Remainder) UsedPagesBySize++;

	if (PageBase)
	{
		Assert((umm)PageBase >= (umm)Arena->Base);
		Assert((umm)PageBase < ((umm)Arena->Base + (umm)(Arena->PageSize * Arena->PageCount)));

		PageIndex = GetPageIndex(PageBase, Arena->Base, Arena->PageSize);
	}
	else
	{
		PageIndex = AllocatePagePool(Arena, UsedPagesBySize);
		*Dest = GetPageBaseFromPageIndex(Arena, PageIndex);
	}

	u32 PagesInPool = Arena->PoolAllocInfo[PageIndex];
	Assert(PagesInPool);

	PageBase = GetPageBaseFromPageIndex(Arena, PageIndex);
	u32 TotalPoolSize = PagesInPool * Arena->PageSize;
	s16 UsedPoolSize = Arena->UsedStatus[PageIndex];

	if (TotalPoolSize < ((memory_index)UsedPoolSize + Size))
	{
		if (!AllocateNextPagesIfItFree(Arena, PageIndex, UsedPagesBySize))
		{
			u32 NewPoolSizeInPages = PagesInPool + UsedPagesBySize;
			
			u32 StartOfNewPool = AllocatePagePool(Arena, NewPoolSizeInPages);
			Arena->UsedStatus[StartOfNewPool] = UsedPoolSize;

			u8 *NewPageBase = GetPageBaseFromPageIndex(Arena, StartOfNewPool);
			Copy128(TotalPoolSize, (void *)NewPageBase, (void *)PageBase);

			DeallocatePagePool(Arena, PageIndex);

			*Dest = NewPageBase;
			PageBase = NewPageBase;
			PageIndex = StartOfNewPool;
		}
		else
		{
			Arena->PoolAllocInfo[PageIndex] += UsedPagesBySize;
		}
	}

	u8 *InPoolDest = PageBase + UsedPoolSize;

	Copy(Size, InPoolDest, Source);
	Arena->UsedStatus[PageIndex] += (s16)Size;

	return nullptr;
}

// TODO: Use struct for holding page alloc or use pointer and calc needed metadata
// on allocation side?
inline void
InitPageArena(memory_arena *Arena, page_memory_arena *PageArena, u32 PageArenaSize, u16 PageSize = KiB(4))
{
	Assert((PageSize >= KiB(1)) && (PageSize < SHRT_MAX) && IsPowerOf2(PageSize));
	Assert(PageArenaSize >= PageSize);
	Assert(!(PageArenaSize % PageSize));

	u32 PageCount = PageArenaSize / PageSize;
	u32 AllocStatusBlocksCount = PageCount / PAGES_PER_ALLOC_STATUS_BLOCK;

	u32 Remainder = (PageCount - (AllocStatusBlocksCount * PAGES_PER_ALLOC_STATUS_BLOCK));
	if (Remainder) AllocStatusBlocksCount++;

	PageArena->AllocStatus = PushArray(Arena, u32, AllocStatusBlocksCount, sizeof(*PageArena->AllocStatus));
	PageArena->UsedStatus = PushArray(Arena, s16, PageCount, sizeof(*PageArena->UsedStatus));
	PageArena->PoolAllocInfo = PushArray(Arena, s16, PageCount, sizeof(*PageArena->PoolAllocInfo));
	PageArena->Base = PushSize(Arena, PageArenaSize, PageSize);
	PageArena->PageSize = PageSize;
	PageArena->PageCount = PageCount;
	PageArena->AllocStatusBlocksCount = AllocStatusBlocksCount;
	PageArena->UnusedSpace =
		(u16)((umm)PageArena->Base - (umm)((u8 *)PageArena->PoolAllocInfo + (sizeof(s16)*PageCount)));

	MemSet(PageArena->AllocStatus, AllocStatusBlocksCount, ULONG_MAX);
	ZeroSize(PageArena->PoolAllocInfo, PageCount * sizeof(*PageArena->PoolAllocInfo));
}