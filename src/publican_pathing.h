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

struct path_node {
	int32_t gValue;
	int32_t hValue;
	int32_t fValue;
	
	struct point3 pos;	
	enum direction dir;		
	struct point3 parentPos;	
	uint32_t parentG;
		
	int8_t onOpenList;
	int8_t onClosedList;
		
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

#endif