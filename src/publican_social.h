#pragma once

enum interaction_types {
	interactiondir_northull,
	
	interactiondir_northeuttalk,
	
	interaction_goodjoke,	
	interaction_badjoke,
	interaction_goodchat,	
	interaction_badchat,	
	interaction_goodflirt,	
	interaction_badflirt,	
};

enum mod_types {
	social_friend,
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

