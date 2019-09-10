#pragma once 

struct memory_arena {
	struct platform_memory_block *currentBlock;
	uintptr_t minBlockSize;
	
	uint64_t allocationFlags;		
	int32_t tempCount;
};

struct temp_memory {
	struct memory_arena *arena;
	struct platform_memory_block *block;
	uintptr_t used;
};

#define ZERO_STRUCT(instance) ZeroSize_(sizeof(instance), &(instance))
#define ZERO_ARRAY(count, ptr) ZeroSize_(count * sizeof((ptr)[0]), ptr)
extern inline void ZeroSize_(size_t size, void *ptr)
{
	uint8_t *byte = (uint8_t *)ptr;
	while(size--) {
		*byte++ = 0;
	}
}

struct arena_push_params {
		uint32_t flags;
		uint32_t align;		
};

static inline size_t GetAlignOffset(struct memory_arena *arena,
									size_t align)
{
	size_t alignOffset = 0;
	
	size_t resultPtr = (size_t)arena->currentBlock->base + arena->currentBlock->used;
	size_t alignMask = align - 1;
	if(resultPtr & alignMask) {
			alignOffset = align - (resultPtr & alignMask);
	}
	return(alignOffset);
}

#define DEF_PUSH (DefaultPushParams_())
extern struct arena_push_params DefaultPushParams_(void)
{
		struct arena_push_params params = {};
		params.flags = 0x1;
		params.align = 4;
		return(params);
}

struct arena_boot_params {
		uint64_t allocationFlags;
		uintptr_t minBlockSize;
};

#define DEF_BOOT (DefaultBootParams_())
extern struct arena_boot_params DefaultBootParams_(void)
{
		struct arena_boot_params params = {};
		return(params);
}

#define PUSH_STRUCT(arena, type, params) \
		(type *)PushSize_(arena, sizeof(type), params)
#define PUSH_ARRAY(arena, count, type, params) \
		(type *)PushSize_(arena, count * sizeof(type), params)
#define PUSH_SIZE(arena, size, params) \
		PushSize_(arena, size, params)
#define PUSH_COPY(arena, size, source, params) \
		Copy_(size, source, PushSize_(arena, size), params)

static inline size_t GetEffectiveSizeFor(struct memory_arena 		*arena,
					 size_t 			sizeInit,
					 struct arena_push_params	params)
{
	size_t size = sizeInit;
	
	size_t alignOffset = GetAlignOffset(arena, params.align);
	size += alignOffset;
	
	return(size);
}	
	
extern inline void *PushSize_(struct memory_arena 	*arena,
			      size_t 			sizeInit,
			      struct arena_push_params 	params)
{
	void *result = 0;
	size_t size = 0;
	if(arena->currentBlock) {
		size = GetEffectiveSizeFor(arena, sizeInit, params);
	}
	if(!arena->currentBlock || (arena->currentBlock->used + size) 
		> arena->currentBlock->size) {
		size = sizeInit;
		if(arena->allocationFlags & (PLATFORM_OVERFLOW | PLATFORM_UNDERFLOW)) {
			arena->minBlockSize = 0;
			size = AlignPow2(size, params.align);
		} else if(!arena->minBlockSize) {
			arena->minBlockSize = 1024 * 1024;
		}		
		size_t blockSize = _MAX(size, arena->minBlockSize);
		
		struct platform_memory_block *newBlock =				
			platform.Alloc(blockSize, arena->allocationFlags);						
		newBlock->arenaPrev = arena->currentBlock;
		arena->currentBlock = newBlock;				
	}
	assert((arena->currentBlock->used + size) <= arena->currentBlock->size);
	
	size_t alignOffset = GetAlignOffset(arena, params.align);
	result = arena->currentBlock->base + arena->currentBlock->used + alignOffset;
	arena->currentBlock->used += size;
	
	assert(size >= sizeInit);
	
	if(params.flags & 0x1) {
		ZeroSize_(sizeInit, result);
	}
	return(result);
}

extern inline void *Copy(size_t size,
			 void 	*sourceInit,
			 void	*destInit)
{
		uint8_t *source = (uint8_t *)sourceInit;
		uint8_t *dest = (uint8_t *)destInit;
		while(size--) {
				*dest++ = *source++;
		}
		return(destInit);
}

extern inline struct temp_memory BeginTempMem(struct memory_arena *arena)
{
		struct temp_memory result;
		
		result.arena = arena;
		result.block = arena->currentBlock;
		result.used = arena->currentBlock ? arena->currentBlock->used : 0;
		
		++arena->tempCount;

		return(result);	
}

extern inline void FreeLastBlock(struct memory_arena *arena)
{
		struct platform_memory_block *free = arena->currentBlock;
		arena->currentBlock = free->arenaPrev;
		platform.DeAlloc(free);
}

extern inline void EndTempMem(struct temp_memory temp)
{
		struct memory_arena *arena = temp.arena;
		while(arena->currentBlock != temp.block) {
			FreeLastBlock(arena);
			printf("blobk freed\n");
		}
		if(arena->currentBlock) {
			assert(arena->currentBlock->used >= temp.used);
			arena->currentBlock->used = temp.used;
			assert(arena->tempCount > 0);
		}	
		--arena->tempCount;
}

extern inline void CheckArena(struct memory_arena *arena)
{
		assert(arena->tempCount == 0);
}

extern inline struct arena_boot_params NonRestoredArena(void)
{
		struct arena_boot_params params = DEF_BOOT;
		params.allocationFlags = PLATFORM_NOTRESTORED;
		return(params);
}

extern inline struct arena_push_params NoClear(void)
{
		struct arena_push_params params = DEF_PUSH;
		params.flags &= ~0x1;
		return(params);
}

#define BootStrapPushSize(type, member, bootparams, pushparams) \
	(type *)BootStrapPushSize_(sizeof(type), OffsetOf(type, member), bootparams, pushparams)
extern inline void *BootStrapPushSize_(uintptr_t structSize,
									   uintptr_t offsetToArena,
									   struct arena_boot_params bootParams,
									   struct arena_push_params pushParams)
{
		struct memory_arena bootstrap = {};
		
		bootstrap.allocationFlags = bootParams.allocationFlags;
		bootstrap.minBlockSize = bootstrap.minBlockSize;
		void *newStruct = PUSH_SIZE(&bootstrap, structSize, pushParams);
		*(struct memory_arena *)((uint8_t *)newStruct + offsetToArena) = bootstrap;
		
		return(newStruct);
}
