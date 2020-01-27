/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	Pathfinding header
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 		  
*************************************************************************************/
#ifndef PUB_PATHING_H
#define PUB_PATHING_H

enum direction {
	DIR_NULL, 
	
	DIR_NORTH, 
	DIR_NORTHEAST, 
	DIR_EAST, 
	DIR_SOUTHEAST, 
	DIR_SOUTH, 
	DIR_SOUTHWEST, 
	DIR_WEST, 
	DIR_NORTHWEST, 
};

enum list_status {
	LIST_OPEN = 0,
	LIST_CLOSED = 8,
};

enum floor_flag {
	FLOOR_DESCEND = -1,
	FLOOR_REMAIN = 0,
	FLOOR_ASCEND = 1	
};

struct path_node {
	int32_t gValue;
	int32_t hValue;
	int32_t fValue;
	
	struct point3 pos;	
	enum direction dir;		
	struct point3 parentPos;	
	uint32_t parentG;
		
	uint16_t listStatus;
		
	struct path_node *next;
	struct path_node *prev;	
};

struct node_list {
	struct point3 pos;	
	struct point3 parentPos;	
	enum direction dir;
		
	float pathProgressC;
	float pathProgressD;	
	
	uint32_t length;
};

struct entity_char;
struct world_mode;
struct node_list *path_GetPath(struct point3, struct point3, struct entity_char *, struct world_mode *, struct node_list *);

#endif