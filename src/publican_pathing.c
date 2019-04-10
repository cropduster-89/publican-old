/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	Pathfinding algorithm
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 	
			@????mostly done????? keep an eye out for small bugs
			
			@add support for paths over valid length
			@ie, allow path, but make a new one when completed
			@and remove the assertion
*************************************************************************************/

/*
*		Get the lowest f for a* calculation
*/
static struct path_node *path_GetLowestF(struct path_node *node)
{
	struct path_node *result = node;
	
	while(node != NULL) {
		
		if(node->fValue < result->fValue &&
		   node->fValue != 0 && node->onOpenList) {
			   
			result = node;						
		}				
		node = node->next;
	}	
	return(result);
}
/*
*		Calculates a cardinal direction for use  		
*		when storing the final path
*
*		Remember: Calculated backwards
*/
static enum direction path_GetCard(struct node_list *destNode, 
				   struct node_list *currNode)
{
	enum direction result = dir_null;
	struct point3 c = currNode->node.p;
	struct point3 d = destNode->node.p;
	
	if(c.y < d.y && c.x == d.x) {
		
		result = dir_north;
		
	} else if(c.y == d.y && c.x < d.x) {
		
		result = dir_east;
		
	} else if(c.y > d.y && c.x == d.x) {
		
		result = dir_south;
		
	} else if(c.y == d.y && c.x > d.x) {
		
		result = dir_west;	
		
	} else if(c.y < d.y && c.x < d.x) {
		
		result = dir_northeast;	
		
	} else if(c.y > d.y && c.x < d.x) {
		
		result = dir_southeast;
		
	} else if(c.y > d.y && c.x > d.x) {
		
		result = dir_southwest;
		
	} else if(c.y < d.y && c.x > d.x) {
		
		  result = dir_northwest;	
		  
	}	
	return(result);	
}
/*
*		Gets the cardinal direction coords
*		for a given direction
*/
static struct grid_point path_GetDir(int32_t i, 
				     struct grid_point current,
				     int32_t floorFlag)
{
	struct grid_point result;
	
	switch(i) {
	case 1: {
		result.p = GET_N_COORD(current.p, floorFlag);
		result.isDiag = 0;
		break;
	} case 2: {
		result.p = GET_NE_COORD(current.p);
		result.isDiag = 1;
		break;
	} case 3: {
		result.p = GET_E_COORD(current.p, floorFlag);
		result.isDiag = 0;
		break;
	} case 4: {
		result.p = GET_SE_COORD(current.p);
		result.isDiag = 1;
		break;
	} case 5: {
		result.p = GET_S_COORD(current.p, floorFlag);
		result.isDiag = 0;
		break;
	} case 6: {
		result.p = GET_SW_COORD(current.p);
		result.isDiag = 1;
		break;
	} case 7: {
		result.p = GET_W_COORD(current.p, floorFlag);
		result.isDiag = 0;
		break;
	} case 8: {
		result.p = GET_NW_COORD(current.p);
		result.isDiag = 1;
		break;
	}	
	}	
	return(result);
}

/*
*		Calculates whether a stepped tile is rotation the		
*		correct dirction for the direction specified
*/
static bool path_StepCheck(struct step_data step,
			   enum direction dir)
{
	bool result = false;
	
	if(dir % 2 == 0) {
		return(result);
	}
	if(step.rotation == 0 && dir == dir_north) {result = true;}
	else if(step.rotation == 1 && dir == dir_west) {result = true;}
	else if(step.rotation == 2 && dir == dir_south) {result = true;}
	else if(step.rotation == 3 && dir == dir_east) {result = true;}
	else if(step.rotation == 0 && dir == dir_south) {result = true;}
	else if(step.rotation == 1 && dir == dir_east) {result = true;}
	else if(step.rotation == 2 && dir == dir_north) {result = true;}
	else if(step.rotation == 3 && dir == dir_west) {result = true;}	
	
	return(result);
} 

/*
*		Calculates whether or path may LEAVE a stepped tile		
*		
*/
static inline bool path_StepLeave(struct tile_data new,
				  struct tile_data current,
				  enum direction dir)
{
	bool result = false;
	if(new.elevation > current.elevation) {
		if(!current.stepped) {
			result = false;
		} else {
			if(path_StepCheck(current.stepData, dir)) {
				result = true;
			}
		}		
	} else if(new.elevation < current.elevation) {
		if(!new.stepped) {
			result = false;
		} else {
			if(path_StepCheck(new.stepData, dir)) {
				result = true;
			}
		}	
	} else if(new.stepped && current.stepped) {
		if(path_StepCheck(new.stepData, dir)) {
			result = true;
		}
	} else if(!new.stepped && current.stepped) {
		if(path_StepCheck(current.stepData, dir)) {
			result = true;
		}
	}	
	return(result);
}
/*
*		Calculates whether or path may ENTER a stepped tile		
*		
*/
static inline bool path_StepEmbark(struct tile_data new,
				   struct tile_data current,	
				   enum direction dir,
				   int floorFlag)
{
	bool result = false;
	
	if(new.stepped && !current.stepped) {	
	
		bool stepSide = false;
		
		if(floorFlag == -1) {			
		
			stepSide = new.stepData.minElev == 3;	
			
		} else if(floorFlag == 1) {		
		
			stepSide = new.stepData.minElev == 3;		
			
		} else if (floorFlag == 0) {
			
			stepSide = new.stepData.minElev == 0;	
			
		} else INVALID_PATH;
		
		result = (path_StepCheck(current.stepData, dir) && stepSide);	
		
	} else if(current.stepped && new.stepped) {
		
		result = (path_StepCheck(new.stepData, dir) && 
			  path_StepCheck(current.stepData, dir));
			  
	} else if(current.stepped && !new.stepped) {
		
		result = path_StepCheck(current.stepData, dir);	
		
	} 	
	return(result);
}
/*
*		Checks whether or not the target square is blocked by a 
*		wall or not, and if stepped tiles can be entered or left.
*
*		wall values:
*	
*		****** wall 0  /***** wall 1 ////// wall 2 ////// wall 3 	N
*		******         /*****        ******        /*****           W	   E
*		******         /*****        ******        /*****       	S
*		******         /*****        ******        /*****       
*		
*		No steps or elevations allowed on diagonals
*
*/
static bool path_GetWalls(enum direction dir, 
			  struct game_world *map,
			  struct grid_point node,
			  int32_t floorFlag)
{
	bool result = false;	
	struct tile_data c = tile_GetTile(map, node.p);	
	switch(dir) {
	case dir_north: {		
		
		struct tile_data n = tile_GetTile(map, GET_N_COORD(node.p, floorFlag));		
		
		result = ((c.walled == 0 || c.walled == 1) && !n.impassable &&
			 (((!c.stepped && !n.stepped) && c.elevation == n.elevation) ||
			 (path_StepEmbark(n, c, dir, floorFlag) && c.elevation == n.elevation) || 
			 (path_StepEmbark(n, c, dir,floorFlag) && path_StepLeave(n, c, dir)) ||
			 (path_StepLeave(n, c, dir))));		
		break;
		
	} case dir_northeast: {	
	
		if(c.stepped) {break;}
		struct tile_data n = tile_GetTile(map, GET_N_COORD(node.p, floorFlag));
		struct tile_data e = tile_GetTile(map, GET_E_COORD(node.p, floorFlag));			
		struct tile_data ne = tile_GetTile(map, GET_NE_COORD(node.p));			
						
		result = ((ne.walled == 0 || ne.walled == 2) && 
			 (e.walled == 0) && c.walled != 2 && 
			 (!n.stepped && !ne.stepped && !e.stepped) &&
			 c.elevation == n.elevation && c.elevation == ne.elevation && 
			 c.elevation == e.elevation &&
			 !ne.impassable && !n.impassable && !e.impassable);
		break;
	} case dir_east: {	
	
		struct tile_data e = tile_GetTile(map, GET_E_COORD(node.p, floorFlag));	
		
		result = ((e.walled == 0 || e.walled == 2) && !e.impassable &&
			 (((!c.stepped && !e.stepped) && c.elevation == e.elevation) ||
			  (path_StepEmbark(e, c, dir, floorFlag) && c.elevation == e.elevation) || 
			  (path_StepLeave(e, c, dir) && path_StepLeave(e, c, dir)) ||
			  (path_StepLeave(e, c, dir))));		
		break;
	} case dir_southeast: {	
	
		if(c.stepped) {break;}
		struct tile_data e = tile_GetTile(map, GET_E_COORD(node.p, floorFlag));			
		struct tile_data s = tile_GetTile(map, GET_S_COORD(node.p, floorFlag));			
		struct tile_data se = tile_GetTile(map, GET_SE_COORD(node.p));	
		
		result = ((e.walled == 0 || e.walled == 2) &&
			  (s.walled == 0 || s.walled == 1) &&
			  (se.walled == 0) &&			 
			  (!e.stepped && !se.stepped && !s.stepped) &&
			  c.elevation == s.elevation && c.elevation == se.elevation && 
			  c.elevation == e.elevation &&
			  !se.impassable && !s.impassable && !e.impassable);		
		break;
	} case dir_south: {		
	
		struct tile_data s = tile_GetTile(map, GET_S_COORD(node.p, floorFlag));
		
		result = ((s.walled == 0 || s.walled == 1) && !s.impassable &&
			 (((!c.stepped && !s.stepped) && c.elevation == s.elevation) ||
			 (path_StepEmbark(s, c, dir, floorFlag) && c.elevation == s.elevation) ||	
			 (path_StepEmbark(s, c, dir, floorFlag) && path_StepLeave(s, c, dir)) || 
			 (path_StepLeave(s, c, dir))));			
		break;
	} case dir_southwest: {		
	
		if(c.stepped) {break;}
		struct tile_data w = tile_GetTile(map, GET_W_COORD(node.p, floorFlag));			
		struct tile_data s = tile_GetTile(map, GET_S_COORD(node.p, floorFlag));			
		struct tile_data sw = tile_GetTile(map, GET_SW_COORD(node.p));	
		
		result = ((s.walled == 0) && (sw.walled == 0 || sw.walled == 2) &&
			  (s.walled == 0 || s.walled == 1) &&			   
			  (!s.stepped && !sw.stepped && !w.stepped) &&
			   c.elevation == s.elevation && c.elevation == sw.elevation && 
			   c.elevation == w.elevation &&
			  !sw.impassable && !s.impassable && !w.impassable);	
		break;
	} case dir_west: {		
	
		struct tile_data w = tile_GetTile(map, GET_W_COORD(node.p, floorFlag));
		
		result = ((c.walled == 0 || c.walled == 2) && !w.impassable &&
			 (((!c.stepped && !w.stepped) && c.elevation == w.elevation)||
			 (path_StepEmbark(w, c, dir, floorFlag) && c.elevation == w.elevation) ||
			 (path_StepEmbark(w, c, dir, floorFlag) && path_StepLeave(w, c, dir)) || 	
			 (path_StepLeave(w, c, dir))));	
		break;
	} case dir_northwest: {	
	
		if(c.stepped) {break;}
		struct tile_data w = tile_GetTile(map, GET_W_COORD(node.p, floorFlag));			
		struct tile_data n = tile_GetTile(map, GET_N_COORD(node.p, floorFlag));			
		struct tile_data nw = tile_GetTile(map, GET_NW_COORD(node.p));	
		
		result = ((nw.walled == 0) &&
			  (w.walled == 0 || w.walled == 1) &&
			  (c.walled == 0 || c.walled == 2) &&		   
			  (!n.stepped && !nw.stepped && !w.stepped) &&
			   c.elevation == n.elevation && c.elevation == nw.elevation && 
			   c.elevation == w.elevation &&
			  !nw.impassable && !n.impassable && !w.impassable);
		break;
	}	
	}	
	
	return(result);
}
/*
*		Calculates whether or not floor above or		
*		below can be tested
*/
static int32_t path_GetFloorFlag(struct world_mode *world,
				 struct path_node *current,
				 enum direction dir)
{
	int32_t result = 0;
	
	if(dir % 2 == 0) return(result);
	
	uint8_t upRot = 0;
	uint8_t downRot = 0;
	struct point3 upTestCoord = {};
	struct point3 downTestCoord = {};
	
	switch(dir) {
	case dir_north: {
		upTestCoord = GET_N_COORD(current->pos.p, 1);
		downTestCoord = GET_N_COORD(current->pos.p, -1);
		upRot = 0;
		downRot = 2;
		break;
	} case dir_east: {
		upTestCoord = GET_E_COORD(current->pos.p, 1);
		downTestCoord = GET_E_COORD(current->pos.p, -1);
		upRot = 3;
		downRot = 1;
		break;
	} case dir_south: {
		upTestCoord = GET_S_COORD(current->pos.p, 1);
		downTestCoord = GET_S_COORD(current->pos.p, -1);
		upRot = 2;
		downRot = 0;	
		
		break;
	} case dir_west: {
		upTestCoord = GET_W_COORD(current->pos.p, 1);
		downTestCoord = GET_W_COORD(current->pos.p, -1);
		upRot = 1;
		downRot = 3;
		break;
	} default: INVALID_PATH;	
	}
	
	if(current->pos.p.z < world->map.sizeZ &&
	   tile_GetStepped(&world->map, current->pos.p) &&
	   tile_GetStepData(&world->map, current->pos.p).rotation == upRot &&
	   !tile_GetStepped(&world->map, upTestCoord)) {
		   
		struct step_data step = tile_GetStepData(&world->map, current->pos.p);
		
		if(step.maxElev == 4.0f) {result = 1;}
		else {result = 0;}
		
	} else if(current->pos.p.z > 0 && 
		 !tile_GetStepped(&world->map, current->pos.p) &&
		  tile_GetStepData(&world->map, downTestCoord).rotation == downRot &&
		  tile_GetStepped(&world->map, downTestCoord)) {
			  
		struct step_data step = 
			tile_GetStepData(&world->map, downTestCoord);
			
		if(step.maxElev == 4.0f) {result = -1;} 
		else {result = 0;}		
	}
	return(result);	
}
/*
*		Main pathfinding algorithm
*
*		TODO, if path exceeds maximum, keep final result 
*		and make new path in ai when completed, so assetrion 
*		can be removed
*
*/
extern struct node_list *path_GetPath(struct point3 origin,
				      struct point3 dest,
				      struct entity_char *ent,
				      struct world_mode *world,
				      struct node_list *path)
{		
	struct game_world *map = &world->map;
	
	struct path_node pathGrid[64][64][2] = {};
	bool dupGrid[64][64][2] = {};
	origin.z /= 4;
	dest.z /= 4;
	struct path_node *begin = &pathGrid[origin.x][origin.y][origin.z];
	struct path_node *end = &pathGrid[dest.x][dest.y][dest.z];
	struct path_node *current = begin;
	
	current->onOpenList = 1;
	current->gValue = 0;
	current->fValue = 0x0FFFFFFF;
	current->parentPos.p.x = 0x0FFFFFFF;
	current->parentPos.p.y = 0x0FFFFFFF;
	current->parentPos.p.z = 0x0FFFFFFF;
	current->prev = NULL;
	current->next = NULL;
	current->pos.p = origin;			
	
	struct path_node *openList = begin;	
	int32_t floorFlag = 0;
	uint32_t count = 0;
	
	while(current != end) {		
		for(int32_t i = 1; i <= 8; ++i) {	
			floorFlag = path_GetFloorFlag(world, current, i);			
			struct grid_point testDir = path_GetDir(i, current->pos, floorFlag);
			
			if(testDir.p.x <  0 || testDir.p.x >= map->sizeX ||
			   testDir.p.y <  0 || testDir.p.y >= map->sizeY) {continue;}					
						
			struct path_node *testNode = &pathGrid[testDir.p.x][testDir.p.y][testDir.p.z];						
			
			if(!path_GetWalls(i, map, current->pos, floorFlag)) {continue;}	
			if(!testNode->onClosedList && dupGrid[testDir.p.x][testDir.p.y][testDir.p.z] == false) {
				openList->next = testNode;
				testNode->onOpenList = 1;
				testNode->prev = openList;
				testNode->next = NULL;
				openList = openList->next;
				dupGrid[testDir.p.x][testDir.p.y][testDir.p.z] = true;
				testNode->parentG = current->gValue;
				if(!testDir.isDiag) {
					testNode->gValue = testNode->parentG + 10;
				} else {
					testNode->gValue = testNode->parentG + 14;
				}
				testNode->hValue = abs(dest.x - testDir.p.x) + 
						   abs(dest.y - testDir.p.y) +
						   (abs(dest.z - testDir.p.z) * 10);
				testNode->fValue = testNode->gValue + testNode->hValue;
				testNode->pos = testDir;	
				
				testNode->parentPos = current->pos;
			} else if (!testNode->onClosedList && 
				   dupGrid[testDir.p.x][testDir.p.y][testDir.p.z] == true) {
					   
				if((testNode->gValue > current->gValue + 10 && !testDir.isDiag) ||
				   (testNode->gValue > current->gValue + 14 && testDir.isDiag)) {
					   
					testNode->parentPos = current->pos;
					testNode->parentG = current->gValue;
					testNode->hValue = abs(dest.x - testDir.p.x) + 
							   abs(dest.y - testDir.p.y) +
						           (abs(dest.z - testDir.p.z) * 10);
					testNode->fValue = testNode->gValue + testNode->hValue;	
					testNode->pos = testDir;					
				}								
			}
				
		}	
		count++;		
		if(current->next == NULL) {
			printf("Could not find after %u tiles\n", count);
			printf("%u %u %u\n", dest.x, dest.y, dest.z);
			return(NULL);						
		}
		current->onOpenList = 0;
		current->onClosedList = 1;		
		current = path_GetLowestF(begin);	
					
	}
	/*
	*	Route found, computing final path
	*
	*/	
	for(int32_t i = 0; i < 128; ++i) {
		path[i].node = end->pos;
		if(i != 0) {
			path[i].node.dir = path_GetCard(&path[i - 1], &path[i]);
		} else {
			path[i].node.dir = dir_null;
		}
		
		path[i].pathProgressC = 0.0f;
		path[i].pathProgressD = 0.0f;
		end = &pathGrid[end->parentPos.p.x][end->parentPos.p.y][end->parentPos.p.z];				
		if(path[i].node.p.x == origin.x &&
		   path[i].node.p.y == origin.y &&
		   path[i].node.p.z == origin.z) {
			ent->activePath = 1;   
			ent->pathLength = i;
			
			return(&path[i]);
		}
	}	
	printf("DEBUG! path exceeds maximum\n");
	INVALID_PATH;
	return(NULL); 		
}