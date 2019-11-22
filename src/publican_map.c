/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	Map tile data 
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 
		    @contains simple tile data,
		    @nothing entity related
*************************************************************************************/

BUTTON_EVENT_CALLBACK(FloorUp)
{
	world->currentFloor++;
}

BUTTON_EVENT_CALLBACK(FloorDown)
{
	world->currentFloor--;
}

/*
*		Utility functions for getting cardinal dirs
*		in coordinates
*/
static inline struct point3 GET_N_COORD(struct point3 oldP,
				        int32_t floorFlag)
{
	struct point3 newP = {
		oldP.x, 
		oldP.y + 1, 
		oldP.z + floorFlag,
	};
	return(newP);
}

static inline struct point3 GET_E_COORD(struct point3 oldP,
				        int32_t floorFlag)
{
	struct point3 newP = {
		oldP.x + 1, 
		oldP.y, 
		oldP.z + floorFlag,
	};
	return(newP);
}

static inline struct point3 GET_S_COORD(struct point3 oldP,
				        int32_t floorFlag)
{
	struct point3 newP = {
		oldP.x, 
		oldP.y - 1, 
		oldP.z + floorFlag,
	};
	return(newP);
}

static inline struct point3 GET_W_COORD(struct point3 oldP,
				        int32_t floorFlag)
{
	struct point3 newP = {
		oldP.x - 1, 
		oldP.y, 
		oldP.z + floorFlag,
	};
	return(newP);
}

static inline struct point3 GET_NE_COORD(struct point3 oldP)
{
	struct point3 newP = {
		oldP.x + 1, 
		oldP.y + 1, 
		oldP.z,
	};
	return(newP);
}

static inline struct point3 GET_SE_COORD(struct point3 oldP)
{
	struct point3 newP = {
		oldP.x + 1, 
		oldP.y - 1, 
		oldP.z,
	};
	return(newP);
}

static inline struct point3 GET_SW_COORD(struct point3 oldP)
{
	struct point3 newP = {
		oldP.x - 1, 
		oldP.y - 1, 
		oldP.z,
	};
	return(newP);
}

static inline struct point3 GET_NW_COORD(struct point3 oldP)
{
	struct point3 newP = {
		oldP.x - 1, 
		oldP.y + 1, 
		oldP.z,
	};
	return(newP);
}

static inline bool tile_GetStepped(struct map_data *map,
				   struct point3 p)
{
	return(map->tiles[(p.z * (map->sizeX * map->sizeY))
		+ p.y * map->sizeX + p.x].stepped);
}

static inline void tile_SetStepped(struct map_data *map,
				  struct point3 p,
				  bool val)
{
	map->tiles[(p.z * (map->sizeX * map->sizeY))
		+ p.y * map->sizeX + p.x].stepped = val;	
}

static inline struct step_data tile_GetStepData(struct map_data *map,
						struct point3 p)
{
	return(map->tiles[(p.z * (map->sizeX * map->sizeY))
		+ p.y * map->sizeX + p.x].stepData);
}

static inline void tile_SetStepData(struct map_data *map,
				    struct point3 p,
				    struct step_data newData)
{
	map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].stepData = newData;	
}

static inline int32_t tile_GetWalled(struct map_data *map,
				     struct point3 p)
{
	return(map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].walled);		
}

static inline void tile_SetWalled(struct map_data *map,
			    	  struct point3 p,	
			    	  int32_t val)
{
	map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].walled = val;	
}

static inline struct tile_data tile_GetTile(struct map_data *map,
					   struct point3 p)
{
	
	return(map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x]);		
}

static inline struct tile_data *tile_GetTilePtr(struct map_data *map,
						struct point3 p)
{
	return(&map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x]);		
}

static inline int32_t tile_GetElevation(struct map_data *map,
					struct point3 p)
{
	if(p.x < 0) p.x = 0;
	if(p.y < 0) p.y = 0;
	
	return(map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].elevation);		
}

static inline void tile_SetElevation(struct map_data *map,
	         		     struct point3 p,	
				     int32_t val)
{
	if(p.x < 0) p.x = 0;
	if(p.y < 0) p.y = 0;
	
	map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].elevation = val;	
}

static inline float tile_GetElevationOffset(struct map_data *map,
					    struct point3 p)
{
	float result = 0;	
	int32_t val = tile_GetElevation(map, p);
	switch(val) {
	case 0: {
		break;
	} case 1: {
		result = 0.5f;
		break;
	} case 2: {
		result = 1.0f;
		break;
	}	
	}
	return(result);
}

static inline int32_t tile_GetPass(struct map_data *map,
				   struct point3 p)
{
	return(map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].impassable);		
}

static inline void tile_SetPass(struct map_data *map,
			        struct point3 p,
			        int32_t val)
{
	map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].impassable = val;	
}

