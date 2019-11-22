#pragma once

enum entity_type {
	ENTTYPE_NULL,
	ENTTYPE_CHAR,
	ENTTYPE_OBJ,
	ENTTYPE_FURN		
};

enum entity_alias {
	ENTALIAS_NULL,
	ENTALIAS_FLOOR,
	ENTALIAS_SUPPORT,	
	ENTALIAS_STOOL,
	ENTALIAS_TABLE,
	ENTALIAS_WALL,
	alias_door,
	alias_pillar,
	alias_bar,
	alias_tiolet,
	alias_steps,
	alias_bigsteps,
	alias_char
};

struct entity_target {
	struct entity *ent;
	uint32_t targetId;
	uint8_t slot;
};

enum entity_state {
	entstate_dormant,
	
	entstate_onscreen,
	entstate_offscreen,		
};

struct entity_store {
	union vec3 dim;	
	enum entity_alias alias;
};

struct entity_struc {
	uint32_t entIndex;

	uint8_t bmpType;	
	uint8_t meshType;
	
	struct point3 coords;
};

struct entity_char {	
	uint32_t entIndex;		
		
	struct chara_stats stats;
	
	struct node_list path[128];
	struct node_list *currentNode;
	uint32_t pathLength;
	int32_t activePath;
	int32_t endPoint;
	
	struct entity_target target;	
	struct char_intent intent;
	
	struct action_time actionTime;	
	
	enum interaction_types interaction;	
	struct entity_target socialTarget;
	bool bubble;
	struct action_time socialTime;
};

struct entity_furn {		
	uint32_t entIndex;
	
	uint8_t maxUsers;
	struct entity_target users[4];	
};

enum entity_rotation {
	rotation_hor,
	rotation_ver,
};

struct entity {
	uint32_t index;
	void *sub;
	
	union vec4 pos;
	union vec3 offset;
	
	enum entity_state state;		
	enum entity_alias alias;
	enum entity_type type;
	
	union vec3 dim;
	
	struct rect3 bounds;
	struct cube3 bound;
	
	float opinions[1024];	
	
	uint8_t rotation;
	
	struct entity *next;
	struct entity *prev;
};

struct entity_store entStore[] = {		

	{{.x = 1.0f, .y = 1.0f, .z = 0.1f}, ENTALIAS_FLOOR},
		 
	{{.x = 1.0f, .y = 1.0f, .z = 1.0f}, alias_steps},	 
	
	{{.x = 1.0f, .y = 1.0f, .z = 1.0f}, alias_bigsteps},
	
	{{.x = 1.0f, .y = 1.0f, .z = 1.0f}, ENTALIAS_SUPPORT},
	
	{{.x = 1.0f, .y = 1.0f, .z = 3.0f}, alias_char},
		
	{{.x = 1.0f, .y = 4.0f, .z = 0.1f}, ENTALIAS_WALL},
	
	{{.x = 3.0f, .y = 4.0f, .z = 0.1f}, alias_door},
	
	{{.x = 0.2f, .y = 0.2f, .z = 4.0f}, alias_pillar},
		
	{{.x = 0.8f, .y = 0.8f, .z = 1.0f}, ENTALIAS_STOOL},
	
	{{.x = 2.0f, .y = 1.2f, .z = 1.0f}, ENTALIAS_TABLE},
		
	{{.x = 1.0f, .y = 1.0f, .z = 1.0f}, alias_tiolet},	
		
	{{.x = 1.0f, .y = 1.0f, .z = 1.0f}, alias_bar},
};
