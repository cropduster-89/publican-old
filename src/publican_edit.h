#pragma once

struct edit_placed_entity {
	struct entity_store *ent;
	enum asset_type_id id;
	struct object_transform trans;
	union vec3 rotOffset;
	int32_t rot;	
};

enum wall_dir {
	dirlock_null,
	
	dirlock_north,
	dirlock_east,
	dirlock_south,
	dirlock_west,	
};

struct edit_state {
	bool editorOpen;
	
	struct edit_placed_entity current;		
	
	bool newAnchor;	
	int32_t newEnd;
	int32_t anchorX;
	int32_t anchorY;	
	int32_t anchorZ;	
	int32_t endX;		
	int32_t endY;
	enum wall_dir dirLock;
	int8_t elevLock;
	int32_t maxLine;
	union vec3 basePos;
	bool wallsInit;
	
	struct step_data step;
	bool validStep;
	
	struct point3 wallPlan[10];
	
	struct entity *parentEntity;
};