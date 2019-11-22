#pragma once

enum interaction_types {
	INTERACT_NULL,

	INTERACT_TALK,
	
	INTERACT_GOODJOKE,	
	INTERACT_BADJOKE,
	INTERACT_GOODCHAT,	
	INTERACT_BADCHAT,	
	INTERACT_GOODFLIRT,	
	INTERACT_BADFLIRT,	
};

enum mod_types {
	SOCIAL_FRIEND,
	social_enemy,
	social_spouse,
	social_family,
	
	social_goodjoke,
	social_badjoke,
	social_goodtalk,
	social_badtalk,
};

struct social_relation {
	uint32_t index;
	float relations;
	
	enum mod_types mods[128];
	int32_t modCount;
};

