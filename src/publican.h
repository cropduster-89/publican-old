#pragma once
#ifndef PUB_H
#define PUB_H

/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	Main header
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 
		    @still a bit random
		    @keep headers in order or risk sadness
*************************************************************************************/

static inline uint32_t Float2Uint(float a)
{
		uint32_t result = (uint32_t)roundf(a);
		return(result);
}

static inline int32_t Float2Int(float a)
{
		int32_t result = (int32_t)roundf(a);
		return(result);
}

#include "vec.h"

#define uiOffset (RealToVec3(0.0f, 0.0f, 50.0f))

#define KILOBYTES(value) ((value)*1024LL)
#define MEGABYTES(value) (KILOBYTES(value)*1024LL)
#define GIGABYTES(value) (MEGABYTES(value)*1024LL)
#define TERABYTES(value) (GIGABYTES(value)*1024LL)

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

#define _MIN(a,b) (((a)<(b))?(a):(b))
#define _MAX(a,b) (((a)>(b))?(a):(b))

#define _270D 4.712389
#define _180D 3.141593
#define _90D 1.570796
#define _60D 1.047198
#define _45D 0.7853982
#define _30D 0.5235988

#define DEG(val) ((val) * (PI / 180))

#define _0OFFSET (RealToVec3(0.0f, 0.0f, 0.0f))
#define _1OFFSET (RealToVec3(1.0f, 0.0f, 0.0f))
#define _2OFFSET (RealToVec3(1.0f, 1.0f, 0.0f))
#define _3OFFSET (RealToVec3(0.0f, 1.0f, 0.0f))

#define _32BIT sizeof(uint32_t)

#define BYTES_PER_PIXEL 4

#ifdef DEBUG 
#include "publican_debug.h"
#endif
#include "publican_business.h"
#include "publican_time.h"
#include "publican_social.h"
#include "publican_ai.h"
#include "publican_map.h"
#include "publican_platform.h"  
#include "publican_render.h"
#include "publican_memory.h"
#include "publican_assets.h"
#include "publican_win32.h" 
#include "publican_pathing.h"
#include "publican_entity.h"
#include "publican_edit.h"
#include "publican_ui.h"
#include "publican_world.h"

#include "publican_utils.h"

struct memory_arena tempArena;

//static platform_add_entry *platformAddEntry;
//static platform_complete_all_work *platformCompleteAllWork;

#define MAX_VERTEX 0xFFFFF
#define MAX_ELEMENT 0xFFFFF
#define MAX_MESHCOMMAND 0xFFFF

struct render_state {
	struct memory_arena arena;
	
	struct gl_mesh_render_command meshCommands[MAX_MESHCOMMAND];
	struct mat4 transforms[MAX_MESHCOMMAND];
	uint32_t texArrayOffsets[MAX_MESHCOMMAND];
	uint32_t meshCount;
		
	float vertexData[MAX_VERTEX];	
	uint32_t vertexOffset;
	uint32_t vertexCount;
	
	uint32_t elementData[MAX_ELEMENT];	
	uint32_t elementOffset;	
	uint32_t elementCount;	
};

struct game_state {
	struct memory_arena mainArena;
	struct memory_arena modeArena;
	
	struct world_mode *worldMode;	
};

struct task_with_memory {
	bool inUse;
	bool modeDepandant;
	struct memory_arena arena;
	
	struct temp_memory memFlush;
};

struct temp_state {
	int32_t isInitialized;
	struct memory_arena tempArena;
	
	struct game_assets *assets;
	
	struct task_with_memory tasks[4];
	
	struct platform_work_queue *highPriorityQueue;
	struct platform_work_queue *lowPriorityQueue;
	
	uint32_t opLock;
};

static uint32_t text_StringCount(char *string)
{
	uint32_t result = 0;
	char *current = string;
	for( ; *current != '\0'; ++result, ++current) {
			;		
	}
	return(result);
}

void pub_MainLoop(struct pub_memory *, struct pub_input *, struct render_commands *);

#include "publican_map.c"
#include "publican_time.c"
#include "publican_entity.c"
#include "publican_pathing.c"
#include "publican_gl.c"
#include "publican_social.c"
#include "publican_ai.c"
#include "publican_assets.c"
#include "publican_render.c"
#include "publican_debug.c"
#include "publican_edit.c"
#include "publican_ui.c"
#include "publican_world.c"

#endif