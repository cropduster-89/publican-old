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

//all directional logic should be clockwise, starting with north,
//with north being the upper left corner of the screen in game. 

enum direction {
	dir_null, 
	
	dir_north, 
	dir_northeast, 
	dir_east, 
	dir_southeast, 
	dir_south, 
	dir_southwest, 
	dir_west, 
	dir_northwest, 
};

struct grid_point {
	struct point3 p;
		
	enum direction dir;
		
	int8_t isDiag;
};

struct path_node {
	int32_t gValue;
	int32_t hValue;
	int32_t fValue;
	
	struct grid_point pos;		
	struct grid_point parentPos;
	uint32_t parentG;
		
	int8_t onOpenList;
	int8_t onClosedList;
		
	struct path_node *next;
	struct path_node *prev;	
};

struct node_list {
	struct grid_point node;		
		
	float pathProgressC;
	float pathProgressD;		
	struct point3 parentP;
	
	uint32_t length;
};
#endif