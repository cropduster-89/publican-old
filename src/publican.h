#pragma once
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
extern inline uint32_t Float2Uint(float a)
{
		uint32_t result = (uint32_t)roundf(a);
		return(result);
}

extern inline int32_t Float2Int(float a)
{
		int32_t result = (int32_t)roundf(a);
		return(result);
}

#include "vec.h"

#define uiOffset (RealToVec3(0.0f, 0.0f, 50.0f))

#define INVALID_PATH (assert(0))

#define _TAU 6.28318530717958647692f
#define _PI 3.14159265358979323846f

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

#define DEG(val) ((val) * (_PI / 180))

#define _0OFFSET (RealToVec3(0.0f, 0.0f, 0.0f))
#define _1OFFSET (RealToVec3(1.0f, 0.0f, 0.0f))
#define _2OFFSET (RealToVec3(1.0f, 1.0f, 0.0f))
#define _3OFFSET (RealToVec3(0.0f, 1.0f, 0.0f))

#define _32BIT sizeof(uint32_t)

#define BYTES_PER_PIXEL 4


/*
*		Two walls stored per tile,
*		0 = no walls
*		1 = y axis wall
*		2 = x axis wall
*		3=  both walls
*/

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

struct pub_graphics_buffer {
	void *data;
	int32_t x;
	int32_t y;
	int32_t pitch;
	int32_t stride;
};

#define MAX_VERTEX 0xFFFFF
#define MAX_ELEMENT 0xFFFFF
#define MAX_MESHCOMMAND 0xFFFF


struct gl_mesh_render_command {
	GLuint vertexCount;
	GLuint instanceCount;
	GLuint firstIndex;
	GLuint baseVertex;
	GLuint baseInstance;	
};

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

#include "publican_map.c"
#include "publican_time.c"
#include "publican_entity.c"
#include "publican_pathing.c"
#include "publican_gl.c"
#include "publican_social.c"
#include "publican_ai.c"
#include "publican_assets.c"
#include "publican_render.c"

static uint32_t text_StringCount(char *string)
{
	uint32_t result = 0;
	char *current = string;
	for( ; *current != '\0'; ++result, ++current) {
			;		
	}
	return(result);
}

extern void debug_TextOut(char *string,
			  struct render_group *renderGroup,
			  struct temp_state *tState,
			  uint32_t lineStart,	
			  float scale)
{		
	union vec3 offset = {
		.x = 50.0f - (WIN_X / 2),
		.y = (WIN_Y / 2) - 50.0f,
		.z = 0.0f
	};
	float lineWeight = 20.0f;
	offset.y -= lineStart * lineWeight;	
	struct object_transform trans = render_FlatTrans();
	trans.offset = offset;
	uint32_t len = text_StringCount(string);
	//uint32_t len = 20;
	char *current = string;		
	float prevX = 0;
	float prevSizeX = 0;
	for(uint32_t i = 0; i < len; ++i, ++current) {
		if(*current == ' ') {
			trans.offset.x += 15;
			continue;
		} else if(*current == '\n') {
			offset.y -= lineWeight;
		} else if(*current == '\0') {
			break;
		}		
		struct bmp_id id = asset_FindChar(tState->assets, asset_font, _CHAR(*current));	
		struct loaded_bmp *bmp = assets_GetBMP(tState->assets, id);							   
		if(!bmp) {
			assets_LoadBMP(renderGroup->assets, id, true, final_bmp);
			return;
		} else {
			union vec2 size = {
				.x = bmp->w,
				.y = bmp->h,
			};
			union vec2 align = {
				.x = bmp->alignX,
				.y = bmp->alignY,
			};
			
			align = MultVec2(align, 1.0f);				
			align = MultVec2(align, scale);		
			size = MultVec2(size, 1.0f);		
			size = MultVec2(size, scale);				
								
			trans.offset.x +=  align.x + prevX + prevSizeX;				
			trans.offset.y -=  size.y + align.y;
			
			//trans = render_UprightTrans();		
			
			prevX = align.x;
			prevSizeX = size.x;
			render_PushBMP(renderGroup, trans, id, size.y, uiOffset, 0);
			trans.offset.y = offset.y;
			
		}				
	}		
}
#include "publican_edit.c"
#include "publican_ui.c"
#include "publican_world.c"
#include "publican_debug.c"


