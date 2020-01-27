#pragma once


enum item_type {
	itemdir_northull,
	
	item_beer,
	item_whiskey,
};

struct item {
	enum item_type type;
	float quantity;	
	uint32_t price;
};

struct item itemStore[] = {
	{item_beer, 10.0f, 5},
};

struct pub_stats {
	float quality;
	float safety;
	float sheek;
	float guildRep; 
		
	int32_t money;
};