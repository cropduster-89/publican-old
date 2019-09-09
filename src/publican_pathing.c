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

static inline bool IsOnOpenList(struct path_node *node)
{
	return(BITCHECK(node->listStatus, LIST_OPEN));
}

static inline bool IsOnClosedList(struct path_node *node)
{
	return(BITCHECK(node->listStatus, LIST_CLOSED));
}

static inline void SetOpenList(struct path_node *node)
{
	BITSET(node->listStatus, LIST_OPEN);
}

static inline void SetClosedList(struct path_node *node)
{
	BITSET(node->listStatus, LIST_CLOSED);
}

static inline void ClearOpenList(struct path_node *node)
{
	BITCLEAR(node->listStatus, LIST_OPEN);
}

static inline void ClearClosedList(struct path_node *node)
{
	BITCLEAR(node->listStatus, LIST_CLOSED);
}

/*
*		Get the lowest f for a* calculation
*/
static struct path_node *path_GetLowestF(struct path_node *node)
{
	struct path_node *result = node;
	
	while(node != NULL) 
	{		
		if(node->fValue < result->fValue &&
		   node->fValue != 0 && IsOnOpenList(node)) 
		{			   
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
	enum direction result = DIR_NULL;
	struct point3 c = currNode->pos;
	struct point3 d = destNode->pos;
	
	if(c.y < d.y && c.x == d.x) 		{result = DIR_NORTH;} 
	else if(c.y == d.y && c.x < d.x) 	{result = DIR_EAST;} 
	else if(c.y > d.y && c.x == d.x) 	{result = DIR_SOUTH;} 
	else if(c.y == d.y && c.x > d.x) 	{result = DIR_WEST;} 
	else if(c.y < d.y && c.x < d.x) 	{result = DIR_NORTHEAST;} 
	else if(c.y > d.y && c.x < d.x) 	{result = DIR_SOUTHEAST;} 
	else if(c.y > d.y && c.x > d.x) 	{result = DIR_SOUTHWEST;} 
	else if(c.y < d.y && c.x > d.x) 	{result = DIR_NORTHWEST;}
	
	return(result);	
}
/*
*		Gets the coords	for a given direction
*		from the input pos
*/
static struct point3 path_GetDir(int32_t dir, 
				 struct point3 pos,
				 int32_t floorFlag)
{
	struct point3 result;
	
	switch(dir) {
	case DIR_NORTH: 	{result = GET_N_COORD(pos, floorFlag); 	break;} 
	case DIR_NORTHEAST: 	{result = GET_NE_COORD(pos); 		break;} 
	case DIR_EAST: 		{result = GET_E_COORD(pos, floorFlag);	break;} 
	case DIR_SOUTHEAST: 	{result = GET_SE_COORD(pos); 		break;} 
	case DIR_SOUTH: 	{result = GET_S_COORD(pos, floorFlag);	break;} 
	case DIR_SOUTHWEST: 	{result = GET_SW_COORD(pos); 		break;} 
	case DIR_WEST: 		{result = GET_W_COORD(pos, floorFlag);  break;} 
	case DIR_NORTHWEST: 	{result = GET_NW_COORD(pos); 		break;}	
	}	
	return(result);
}

/*
*		Calculates whether a stepped tile is rotated the		
*		correct dirction for the direction specified
*/
static bool StepCheck(struct step_data step,
			   enum direction dir)
{
	bool result = false;	
	if(dir % 2 == 0) {return(result);}
	
	if(step.rotation == 0 && dir == DIR_NORTH) 	{result = true;}
	else if(step.rotation == 1 && dir == DIR_WEST) 	{result = true;}
	else if(step.rotation == 2 && dir == DIR_SOUTH)	{result = true;}
	else if(step.rotation == 3 && dir == DIR_EAST) 	{result = true;}
	else if(step.rotation == 0 && dir == DIR_SOUTH)	{result = true;}
	else if(step.rotation == 1 && dir == DIR_EAST) 	{result = true;}
	else if(step.rotation == 2 && dir == DIR_NORTH)	{result = true;}
	else if(step.rotation == 3 && dir == DIR_WEST) 	{result = true;}	
	
	return(result);
} 

/*
*		Calculates whether or path may LEAVE a stepped tile		
*		
*/
static inline bool path_StepLeave(struct tile_data new,
				  struct tile_data currentNode,
				  enum direction dir)
{
	bool result = false;
	if(new.elevation > currentNode.elevation) 
	{
		if(!currentNode.stepped) {result = false;} 
		else 
		{
			if(StepCheck(currentNode.stepData, dir)) 
			{
				result = true;
			}
		}		
	} 
	else if(new.elevation < currentNode.elevation) 
	{
		if(!new.stepped) {result = false;} 
		else 
		{
			if(StepCheck(new.stepData, dir)) 
			{
				result = true;
			}
		}	
	} 
	else if(new.stepped && currentNode.stepped) 
	{
		if(StepCheck(new.stepData, dir)) 
		{
			result = true;
		}
	} 
	else if(!new.stepped && currentNode.stepped) 
	{
		if(StepCheck(currentNode.stepData, dir)) 
		{
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
				   struct tile_data currentNode,	
				   enum direction dir,
				   int floorFlag)
{
	bool result = false;
	
	if(new.stepped && !currentNode.stepped) 
	{		
		bool stepSide = false;
		
		if(floorFlag == -1) 
		{		
			stepSide = new.stepData.minElev == 3;				
		} 
		else if(floorFlag == 1) 
		{			
			stepSide = new.stepData.minElev == 3;			
		} 
		else if (floorFlag == 0) 
		{			
			stepSide = new.stepData.minElev == 0;				
		} 
		else INVALID_PATH;
		
		result = (StepCheck(currentNode.stepData, dir) && stepSide);		
	} 
	else if(currentNode.stepped && new.stepped) 
	{		
		result = (StepCheck(new.stepData, dir) && 
			  StepCheck(currentNode.stepData, dir));
			  
	} 
	else if(currentNode.stepped && !new.stepped) 
	{		
		result = StepCheck(currentNode.stepData, dir);		
	} 	
	return(result);
}

static inline bool IsNorthTilePassable(struct map_data *map,					
				       enum direction dir,
				       int32_t floorFlag,
				       struct point3 pos)
{
	struct tile_data c = tile_GetTile(map, pos);	
	struct tile_data n = tile_GetTile(map, GET_N_COORD(pos, floorFlag));	
	
	return((c.walled == 0 || c.walled == 1) && !n.impassable &&
	       (((!c.stepped && !n.stepped) && c.elevation == n.elevation) ||
	       (path_StepEmbark(n, c, dir, floorFlag) && c.elevation == n.elevation) || 
	       (path_StepEmbark(n, c, dir,floorFlag) && path_StepLeave(n, c, dir)) ||
	       (path_StepLeave(n, c, dir))));
}

static inline bool IsNortheastTilePassable(struct map_data *map,
					   enum direction dir,
				           int32_t floorFlag,	
				           struct point3 pos)
{
	struct tile_data c = tile_GetTile(map, pos);	
	if(c.stepped) {return(false);}
	
	struct tile_data n = tile_GetTile(map, GET_N_COORD(pos, floorFlag));
	struct tile_data e = tile_GetTile(map, GET_E_COORD(pos, floorFlag));			
	struct tile_data ne = tile_GetTile(map, GET_NE_COORD(pos));			
	
	return((ne.walled == 0 || ne.walled == 2) && 
	       (e.walled == 0) && c.walled != 2 && 
	       (!n.stepped && !ne.stepped && !e.stepped) &&
	       c.elevation == n.elevation && c.elevation == ne.elevation && 
	       c.elevation == e.elevation &&
	       !ne.impassable && !n.impassable && !e.impassable);
}

static inline bool IsEastTilePassable(struct map_data *map,					
				      enum direction dir,
				      int32_t floorFlag,
				      struct point3 pos)
{
	struct tile_data c = tile_GetTile(map, pos);	
	struct tile_data e = tile_GetTile(map, GET_E_COORD(pos, floorFlag));	
	
	return((e.walled == 0 || e.walled == 2) && !e.impassable &&
	       (((!c.stepped && !e.stepped) && c.elevation == e.elevation) ||
	       (path_StepEmbark(e, c, dir, floorFlag) && c.elevation == e.elevation) || 
	       (path_StepLeave(e, c, dir) && path_StepLeave(e, c, dir)) ||
	       (path_StepLeave(e, c, dir))));
}

static inline bool IsSoutheastTilePassable(struct map_data *map,
					   enum direction dir,
				           int32_t floorFlag,	
				           struct point3 pos)
{
	struct tile_data c = tile_GetTile(map, pos);	
	if(c.stepped) {return(false);}
	
	struct tile_data e = tile_GetTile(map, GET_E_COORD(pos, floorFlag));			
	struct tile_data s = tile_GetTile(map, GET_S_COORD(pos, floorFlag));			
	struct tile_data se = tile_GetTile(map, GET_SE_COORD(pos));			
	
	return((e.walled == 0 || e.walled == 2) &&
	       (s.walled == 0 || s.walled == 1) &&
	       (se.walled == 0) &&			 
	       (!e.stepped && !se.stepped && !s.stepped) &&
	       c.elevation == s.elevation && c.elevation == se.elevation && 
	       c.elevation == e.elevation &&
	       !se.impassable && !s.impassable && !e.impassable);	
}

static inline bool IsSouthTilePassable(struct map_data *map,					
				       enum direction dir,
				       int32_t floorFlag,
				       struct point3 pos)
{
	struct tile_data c = tile_GetTile(map, pos);	
	struct tile_data s = tile_GetTile(map, GET_S_COORD(pos, floorFlag));
	
	return((s.walled == 0 || s.walled == 1) && !s.impassable &&
		(((!c.stepped && !s.stepped) && c.elevation == s.elevation) ||
		(path_StepEmbark(s, c, dir, floorFlag) && c.elevation == s.elevation) ||	
		(path_StepEmbark(s, c, dir, floorFlag) && path_StepLeave(s, c, dir)) || 
		(path_StepLeave(s, c, dir))));
}

static inline bool IsSouthwestTilePassable(struct map_data *map,
					   enum direction dir,
				           int32_t floorFlag,	
				           struct point3 pos)
{
	struct tile_data c = tile_GetTile(map, pos);	
	if(c.stepped) {return(false);}
	
	struct tile_data w = tile_GetTile(map, GET_W_COORD(pos, floorFlag));			
	struct tile_data s = tile_GetTile(map, GET_S_COORD(pos, floorFlag));			
	struct tile_data sw = tile_GetTile(map, GET_SW_COORD(pos));	
	
	return((s.walled == 0) && (sw.walled == 0 || sw.walled == 2) &&
		(s.walled == 0 || s.walled == 1) &&			   
		(!s.stepped && !sw.stepped && !w.stepped) &&
		c.elevation == s.elevation && c.elevation == sw.elevation && 
		c.elevation == w.elevation &&
		!sw.impassable && !s.impassable && !w.impassable);
}

static inline bool IsWestTilePassable(struct map_data *map,					
				      enum direction dir,
				      int32_t floorFlag,
				      struct point3 pos)
{
	struct tile_data c = tile_GetTile(map, pos);	
	struct tile_data w = tile_GetTile(map, GET_W_COORD(pos, floorFlag));
	
	return((c.walled == 0 || c.walled == 2) && !w.impassable &&
	       (((!c.stepped && !w.stepped) && c.elevation == w.elevation)||
	       (path_StepEmbark(w, c, dir, floorFlag) && c.elevation == w.elevation) ||
	       (path_StepEmbark(w, c, dir, floorFlag) && path_StepLeave(w, c, dir)) || 	
	       (path_StepLeave(w, c, dir))));	
}

static inline bool IsNorthwestTilePassable(struct map_data *map,
					   enum direction dir,
				           int32_t floorFlag,	
				           struct point3 pos)
{
	struct tile_data c = tile_GetTile(map, pos);	
	if(c.stepped) {return(false);}
	
	struct tile_data w = tile_GetTile(map, GET_W_COORD(pos, floorFlag));			
	struct tile_data n = tile_GetTile(map, GET_N_COORD(pos, floorFlag));			
	struct tile_data nw = tile_GetTile(map, GET_NW_COORD(pos));	
	
	return((nw.walled == 0) &&
	       (w.walled == 0 || w.walled == 1) &&
	       (c.walled == 0 || c.walled == 2) &&		   
	       (!n.stepped && !nw.stepped && !w.stepped) &&
	       c.elevation == n.elevation && c.elevation == nw.elevation && 
	       c.elevation == w.elevation &&
	       !nw.impassable && !n.impassable && !w.impassable);
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
static bool TileIsPassable(enum direction dir, 
			   struct map_data *map,
			   struct point3 pos,
			   int32_t floorFlag)
{
	bool result = false;	
		
	switch(dir) {
	case DIR_NORTH: 
	{					
		result = IsNorthTilePassable(map, dir, floorFlag, pos);
		break;		
	} 
	case DIR_NORTHEAST: 
	{			
		result = IsNortheastTilePassable(map, dir, floorFlag, pos);
		break;
	} 
	case DIR_EAST: 
	{			
		result = IsEastTilePassable(map, dir, floorFlag, pos);
		break;
	} 
	case DIR_SOUTHEAST: 
	{		
		result = IsSoutheastTilePassable(map, dir, floorFlag, pos);	
		break;
	} 
	case DIR_SOUTH: 
	{		
		result = IsSouthTilePassable(map, dir, floorFlag, pos);	
		break;
	} 
	case DIR_SOUTHWEST: 
	{		
		result = IsSouthwestTilePassable(map, dir, floorFlag, pos);	 	
		break;
	} 
	case DIR_WEST: 
	{				
		result = IsWestTilePassable(map, dir, floorFlag, pos);	 	
		break;
	} case DIR_NORTHWEST: 
	{			
		result = IsNorthwestTilePassable(map, dir, floorFlag, pos);	 	
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
				 struct path_node *currentNode,
				 enum direction dir)
{
	int32_t result = 0;
	
	if(dir % 2 == 0) return(result);
	
	uint8_t upRot = 0;
	uint8_t downRot = 0;
	struct point3 upTestCoord = {};
	struct point3 downTestCoord = {};
	
	switch(dir) {
	case DIR_NORTH: 
	{
		upTestCoord = GET_N_COORD(currentNode->pos, 1);
		downTestCoord = GET_N_COORD(currentNode->pos, -1);
		upRot = 0;
		downRot = 2;
		break;
	} 
	case DIR_EAST: 
	{
		upTestCoord = GET_E_COORD(currentNode->pos, 1);
		downTestCoord = GET_E_COORD(currentNode->pos, -1);
		upRot = 3;
		downRot = 1;
		break;
	} 
	case DIR_SOUTH: 
	{
		upTestCoord = GET_S_COORD(currentNode->pos, 1);
		downTestCoord = GET_S_COORD(currentNode->pos, -1);
		upRot = 2;
		downRot = 0;
		break;
	} 
	case DIR_WEST:
	{
		upTestCoord = GET_W_COORD(currentNode->pos, 1);
		downTestCoord = GET_W_COORD(currentNode->pos, -1);
		upRot = 1;
		downRot = 3;
		break;
	} 
	default: INVALID_PATH;	
	}
	
	if(currentNode->pos.z < world->map.sizeZ &&
	   tile_GetStepped(&world->map, currentNode->pos) &&
	   tile_GetStepData(&world->map, currentNode->pos).rotation == upRot &&
	   !tile_GetStepped(&world->map, upTestCoord)) 
	{
		   
		struct step_data step = tile_GetStepData(&world->map, currentNode->pos);
		
		if(step.maxElev == 4.0f) 	{result = 1;}
		else 				{result = 0;}
		
	} 
	else if(currentNode->pos.z > 0 && 
		!tile_GetStepped(&world->map, currentNode->pos) &&
		tile_GetStepData(&world->map, downTestCoord).rotation == downRot &&
		tile_GetStepped(&world->map, downTestCoord)) 
	{
			  
		struct step_data step = 
			tile_GetStepData(&world->map, downTestCoord);
			
		if(step.maxElev == 4.0f) 	{result = -1;} 
		else 				{result = 0;}		
	}
	return(result);	
}

static inline void path_DistanceTest(struct path_node *testNode,
				     struct path_node *currentNode,
				     struct point3 dest,	
				     struct point3 testDir,
				     enum direction dir)
{
	if(dir % 2 != 0) 	{testNode->gValue = testNode->parentG + 10;} 
	else 			{testNode->gValue = testNode->parentG + 14;}
	
	testNode->hValue = abs(dest.x - testDir.x) + 
			   abs(dest.y - testDir.y) +
			   (abs(dest.z - testDir.z) * 10);
	testNode->fValue = testNode->gValue + testNode->hValue;
	testNode->pos = testDir;		
	testNode->parentPos = currentNode->pos;
}

static inline bool path_IsWithinMapExtents(struct map_data *map,
					   struct point3 testDir)
{
	return(testDir.x >= 0 && testDir.x < map->sizeX &&
	       testDir.y >= 0 && testDir.y < map->sizeY);
}

static inline bool path_IsDuplicate(struct point3 testDir,
				    bool dupGrid[][64][2])
{
	return(dupGrid[testDir.x][testDir.y][testDir.z]);
}

static inline bool path_IsNewRouteFaster(struct path_node *testNode,
					 struct path_node *currentNode,					
					 enum direction dir)
{
	return((testNode->gValue > currentNode->gValue + 10 && dir % 2 != 0) ||
	       (testNode->gValue > currentNode->gValue + 14 && dir % 2 == 0));
}

/*
*		Main pathfinding algorithm
*
*		TODO, if path exceeds maximum, keep final result 
*		and make new path in ai when completed, so assetrion 
*		can be removed
*
*/
extern struct node_list *path_GetPath(struct point3 startPoint,
				      struct point3 endPoint,
				      struct entity_char *ent,
				      struct world_mode *world,
				      struct node_list *path)
{		
	struct map_data *map = &world->map;
	
	struct path_node pathGrid[64][64][2];
	bool dupGrid[64][64][2] = {};
	
	struct path_node *startNode = &pathGrid[startPoint.x][startPoint.y][startPoint.z];
	struct path_node *endNode = &pathGrid[endPoint.x][endPoint.y][endPoint.z];
	struct path_node *currentNode = startNode;
	
	SetOpenList(currentNode);
	currentNode->gValue = 0;
	currentNode->fValue = INT_MAX;
	currentNode->parentPos.x = INT_MAX;
	currentNode->parentPos.y = INT_MAX;
	currentNode->parentPos.z = INT_MAX;
	currentNode->prev = NULL;
	currentNode->next = NULL;
	currentNode->pos = startPoint;			
	
	struct path_node *openList = startNode;	
	struct point3 testCoords = {};
	struct path_node *testNode = 0;
	int32_t floorFlag = 0;	
	
	while(currentNode != endNode) 
	{		
		for(enum direction dir = DIR_NORTH; dir <= DIR_NORTHWEST; ++dir) 
		{				
			testCoords = path_GetDir(dir, currentNode->pos, floorFlag);			
			if(!path_IsWithinMapExtents(map, testCoords)) {continue;}	
			
			floorFlag = path_GetFloorFlag(world, currentNode, dir);	
			if(!TileIsPassable(dir, map, currentNode->pos, floorFlag)) {continue;}					
						
			testNode = &pathGrid[testCoords.x][testCoords.y][testCoords.z];
			if(IsOnClosedList(testNode)) {continue;}
			
			if(!path_IsDuplicate(testCoords, dupGrid)) 
			{
				SetOpenList(testNode);
				openList->next = testNode;				
				testNode->prev = openList;
				testNode->next = NULL;
				openList = openList->next;
				dupGrid[testCoords.x][testCoords.y][testCoords.z] = true;
				testNode->parentG = currentNode->gValue;
				
				path_DistanceTest(testNode, currentNode, endPoint, testCoords, dir);				
			} 
			else 
			{					   
				if(path_IsNewRouteFaster(testNode, currentNode, dir)) 
				{   
					path_DistanceTest(testNode, currentNode, 
						endPoint, testCoords, dir);				
				}								
			}				
		}			
		if(currentNode->next == NULL) {return(NULL);}
		ClearOpenList(currentNode);
		SetClosedList(currentNode);	
		currentNode = path_GetLowestF(startNode);						
	}
	/*
	*	Route found, computing final path
	*
	*/	
	for(int32_t i = 0; i < 128; ++i) 
	{
		path[i].pos = endNode->pos;
		if(i != 0) {
			path[i].dir = path_GetCard(&path[i - 1], &path[i]);
		} else {
			path[i].dir = DIR_NULL;
		}		
		path[i].pathProgressC = 0.0f;
		path[i].pathProgressD = 0.0f;
		endNode = &pathGrid[endNode->parentPos.x]
			[endNode->parentPos.y][endNode->parentPos.z];				
		if(path[i].pos.x == startPoint.x &&
		   path[i].pos.y == startPoint.y &&
		   path[i].pos.z == startPoint.z) {
			ent->activePath = 1;   
			ent->pathLength = i;
			
			return(&path[i]);
		}
	}	
	printf("DEBUG! path exceeds maximum\n");
	INVALID_PATH;
	return(NULL); 		
}