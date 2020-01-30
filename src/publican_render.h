#pragma once
#ifndef PUB_RENDER_H
#define PUB_RENDER_H

/************************************************************************************		
__________     ___.   .__  .__                      	Render header
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 			
*************************************************************************************/

enum sprite_elements {
	sprite_body,
	sprite_dress,
	sprite_head,
	sprite_hair,
	sprite_face	
};

struct camera_transform {
	bool ortho;
	
	float metresToPixels;
	union vec2 screenCen;
	
	float focalLength;
	
	union vec3 pos;
};

struct camera_params {		
	float focalLength;
};

static inline struct camera_params GetCamParams(uint32_t w, float focalLength)
{
	struct camera_params result;
	
	result.focalLength = focalLength;
	
	return(result);
}

struct object_transform {
	bool upright;
	union vec3 offset;
	float scale;
	
	struct rect_int screenSpace;
};

	
struct bmp_dim {
	union vec3 basis;
	union vec2 size;
	union vec3 pos;
};	 

static inline struct object_transform render_UprightTrans(void)
{
	struct object_transform result = {};
	
	result.upright = true; 
	result.scale = 1.0f;
	
	return(result);
}

static inline struct object_transform render_FlatTrans(void)
{
	struct object_transform result = {};		
	
	result.scale = 1.0f;
	
	return(result);
}

struct render_setup {
	struct rect_int clipRect;
	uint32_t renderTargetIndex;
	struct mat4 proj;	
	union vec3 camP;
};


struct render_group {
	struct game_assets *assets;

	union vec2 screenDim;		
			
	struct render_commands *commands;	
	struct render_setup lastSetup;
	struct render_entry_mesh_batch *meshBatch;
	
	union vec3 camX;
	union vec3 camY;
	union vec3 camZ;
	union vec3 camP;
	
	struct mat4 camForward;
	struct mat4 camInverse;
	struct mat4 projForward;		
	struct mat4 projInverse;		
};

struct push_buffer_result {		
	struct render_entry_header *header;
};

enum entry_type {
	entry_bmp,
	entry_mesh,
	entry_mesh_batch,
	entry_composite_surface, 
	entry_composite_piece, 
	entry_composite_bmp, 
	entry_cliprect,
	entry_rect,
	entry_blend_target,
	entry_cube,
	entry_mouse,
};

struct render_entry_header {
	struct render_setup setup;	
	uint16_t type;	
};

struct render_entry_mesh_batch {
	uint32_t meshCount;
};

struct render_entry_mesh {
	struct loaded_mesh *mesh;
	struct loaded_bmp *bmp;	
	struct mat4 transform;
};

struct render_entry_bmp {
	struct loaded_bmp *bmp;
	union vec3 pos;
	float zBias;
	
	union vec3 xAxis;
	union vec3 yAxis;
	
	bool usesAtlas;		
	struct rect2 atlasOffset;
};

struct render_entry_bmp_piece {
	struct loaded_bmp *bmp;
	union vec3 pos;
	float zBias;
	
	union vec3 xAxis;
	union vec3 yAxis;
	
	uint32_t handle;
};

struct render_entry_cube {
	struct loaded_bmp *bmp;
	
	union vec3 pos;
	
	float height;
	float radius;
	
	uint32_t colour;
};

struct render_entry_mouse {
	union vec3 pos;
};

struct render_entry_rect {		
	union vec3 pos;		
	union vec2 dim;
	union vec4 colour;
};

struct render_entry_blend_target {
	uint32_t sourceTargetIndex;
	float alpha;
};

struct gl_mesh_render_command {
	uint32_t vertexCount;
	uint32_t instanceCount;
	uint32_t firstIndex;
	uint32_t baseVertex;
	uint32_t baseInstance;	
};

struct render_commands {
	int32_t w;
	int32_t h;
	
	uint32_t maxPushSize;
	uint32_t entryCount;
	uint8_t *pushBase;		
	uint8_t *pushData;	
	/*
	*	Data for multi draw meshes
	*	Prepare for this not being big enough
	*/
	struct gl_mesh_render_command *meshCommands;
	uint32_t meshCount;
	uint32_t *texArrayOffsets;
	struct mat4 *transforms;
	
	uint32_t maxVertexSize;
	uint32_t vertexCount;
	uint8_t *vertexBase;
	uint8_t *vertexData;
	
	uint32_t maxElementSize;
	uint32_t elementCount;	
	uint8_t *elementBase;
	uint8_t *elementData;
	/*
	*	TODO: is this currently being used properly?
	*/
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

#endif