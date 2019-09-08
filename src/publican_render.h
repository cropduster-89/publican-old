#pragma once

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

extern inline struct camera_params GetCamParams(uint32_t w, float focalLength)
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

extern inline struct object_transform render_UprightTrans(void)
{
	struct object_transform result = {};
	
	result.upright = true; 
	result.scale = 1.0f;
	
	return(result);
}

extern inline struct object_transform render_FlatTrans(void)
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

struct render_entry_mesh {
	struct loaded_mesh *mesh;
	struct loaded_bmp *bmp;	
	struct mat4 transform;
	union vec3 pos;
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
