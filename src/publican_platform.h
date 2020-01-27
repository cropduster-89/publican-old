#pragma once

#define AlignPow2(Value, Alignment) ((Value + ((Alignment) - 1)) & ~((Value - Value) + (Alignment) - 1))
#define Align4(Value) ((Value + 3) & ~3)
#define Align8(Value) ((Value + 7) & ~7)
#define Align16(Value) ((Value + 15) & ~15)

#define OffsetOf(type, Member) (uintptr_t)&(((type *)0)->Member)

#define U32FromPointer(Pointer) ((uint32_t)(size_t)(Pointer))
#define PointerFromU32(type, Value) (type *)((size_t)Value)

static inline uint32_t SafeTruncateUInt64(uint64_t a)
{
	assert(a <= 0xFFFFFFFF);
	uint32_t result = (uint32_t)a;
	return(result);
}

static inline uint16_t SafeTruncateUInt16(uint32_t a)
{
	assert(a <= 0xFFFF);
	uint16_t result = (uint16_t)a;
	return(result);
}

struct button_state {
	bool endedDown;
	bool isDown;
};

struct button_control {
	struct button_state lClick;
	struct button_state rClick;
	
	struct button_state up;
	struct button_state shift;
	struct button_state esc;
	struct button_state alt;
	struct button_state ctrl;
	struct button_state _A;
	struct button_state _B;
	struct button_state _C;
	struct button_state _D;
	struct button_state _F;
	struct button_state DIR_NORTH;
	struct button_state _O;
	struct button_state _P;
	struct button_state _R;
	struct button_state DIR_SOUTH;
	struct button_state _T;
	struct button_state _U;
	struct button_state DIR_WEST;
	struct button_state f1;
};

struct render_commands {
	int32_t w;
	int32_t h;
	
	uint32_t maxPushSize;
	uint32_t entryCount;
	uint8_t *pushBase;		
	uint8_t *pushData;		
	
	uint32_t lastUsedSortKey;
	uint32_t clipRectCount;
	uint32_t maxRenderTargetIndex;
	
	struct render_entry_cliprect *firstRect;
	struct render_entry_cliprect *lastRect;
};	

struct render_prep {
	struct render_entry_cliprect *clipRects;
	uint32_t sortedIndexCount;
	uint32_t sortedIndices;
};

struct pub_input {
	int32_t mouseX, mouseY, mouseZ;
	struct button_control buttons;
}; 

enum platform_memory_block_flags {
	PLATFORM_NOTRESTORED = 0x1,
	PLATFORM_OVERFLOW = 0x2,
	PLATFORM_UNDERFLOW = 0x4
};

struct platform_file_handle {
	bool noErrors;
	void *platform;
};

struct platform_file_group {
	uint32_t fileCount; 
	void *platform;
};

struct file_read_output {
	uint32_t contentsSize;
	void *contents;
};

enum platform_file_type {
	filetype_asset,
	filetype_count,
};

struct platform_memory_block {
	uint64_t flags;
	uint64_t size;
	uint8_t *base;
	uintptr_t used;
	struct platform_memory_block *arenaPrev;	
};

struct ticket_mutex {
	uint64_t volatile ticket;
	uint64_t volatile serving;
};

enum {
	debugCounter_mainLoop,
	debugCounter_drawRect,
	debugCounter_count
};

struct debug_counter {
	uint64_t cycleCount;
	uint32_t hitCount;
} debug_counter;

struct platform_work_queue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(struct platform_work_queue *queue, void *data)
typedef PLATFORM_WORK_QUEUE_CALLBACK(platform_work_queue_callback);

typedef void platform_add_entry(struct platform_work_queue *queue, platform_work_queue_callback *callback, void *data);
typedef void platform_complete_all_work(struct platform_work_queue *queue);

#define PLATFORM_GET_ALL_FILES_OF_TYPE_START(name) struct platform_file_group name(enum platform_file_type type)
typedef PLATFORM_GET_ALL_FILES_OF_TYPE_START(platform_get_all_files_of_type_start);

#define PLATFORM_GET_ALL_FILES_OF_TYPE_END(name) void name(struct platform_file_group *fileGroup)
typedef PLATFORM_GET_ALL_FILES_OF_TYPE_END(platform_get_all_files_of_type_end);

#define PLATFORM_OPEN_FILE(name) struct platform_file_handle name(struct platform_file_group *fileGroup)
typedef PLATFORM_OPEN_FILE(platform_opendir_northext_file);

#define PLATFORM_READ_DATA_FROM_FILE(name) void name(struct platform_file_handle *source, uint64_t offset, uint64_t size, void *dest)
typedef PLATFORM_READ_DATA_FROM_FILE(platform_read_data_from_file);

#define PLATFORM_FILE_ERROR(name) void name(struct platform_file_handle *handle, char *message)
typedef PLATFORM_FILE_ERROR(platform_file_error);

#define PlatformNoFileErrors(handle) ((handle)->noErrors)

#define PLATFORM_ALLOCATE_MEMORY(name) struct platform_memory_block *name(size_t size, uint64_t flags)
typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);

#define PLATFORM_DEALLOCATE_MEMORY(name) void name(struct platform_memory_block *block)
typedef PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_memory);

struct platform_texture_op_queue {
	struct ticket_mutex mutex;
	
	struct texture_op *first;
	struct texture_op *last;
	struct texture_op *firstFree;
};

typedef struct platform_api {
	platform_allocate_memory *Alloc;
	platform_deallocate_memory *DeAlloc;
	
	platform_add_entry *AddEntry;
	platform_complete_all_work *CompleteAllWork;
	
	platform_get_all_files_of_type_start *GetFilesTypeStart;
	platform_get_all_files_of_type_end *GetFilesTypeEnd;
	platform_opendir_northext_file *OpenNextFile;
	platform_read_data_from_file *ReadDataFromFile;
	platform_file_error *FileError;
} platform_api;
extern struct platform_api platform;

struct pub_memory {
	struct game_state *state;
	struct temp_state *tState;
	struct render_state *rState;
	
	struct platform_work_queue *highPriorityQueue;
	struct platform_work_queue *lowPriorityQueue;	
	struct platform_texture_op_queue textureOpQueue;

	uint32_t fboCount;

	struct platform_api platformAPI;	
#ifdef DEBUG		
	struct cycle_counter counters[debug_count];
#endif		
};

#ifdef DEBUG

struct pub_memory *debugMemory;

#define TIMED_START(id) uint64_t startCycleCount##id = __rdtsc();
#define TIMED_END(id) debugMemory->counters[debug_##id].cycleCount += __rdtsc() - startCycleCount##id; ++debugMemory->counters[debug_##id].hitCount;
#else
#define TIMED_START(id)
#define TIMED_END(id)
#endif

static inline uint32_t GetThreadID(void)
{
    uint8_t *ThreadLocalStorage = (uint8_t *)__readgsqword(0x30);
    uint32_t ThreadID = *(uint32_t *)(ThreadLocalStorage + 0x48);

    return(ThreadID);
}

static inline uint32_t AtomicCompareExchangeUInt32(uint32_t volatile *val,
						   uint32_t input,
						   uint32_t expected)
{
	uint32_t result = _InterlockedCompareExchange((long volatile *)val,
		input, expected);
	return(result);													
}
	
static inline uint64_t AtomicAddU64(uint64_t volatile *a,
		        	    uint64_t b)
{
	uint64_t result = __sync_fetch_and_add(a, b);
	return(result);
}

static inline void BeginTicketMutex(struct ticket_mutex *mutex)
{
	uint64_t ticket = AtomicAddU64(&mutex->ticket, 1);
	while(ticket != mutex->serving) {_mm_pause();}
}

static inline void EndTicketMutex(struct ticket_mutex *mutex)
{
	AtomicAddU64(&mutex->serving, 1);
}