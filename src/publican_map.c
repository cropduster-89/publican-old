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


/*
*		Utility functions for getting cardinal dirs
*		in coordinates
*/
extern inline struct point3 GET_N_COORD(struct point3 oldP,
				        int32_t floorFlag)
{
	struct point3 newP = {
		oldP.x, 
		oldP.y + 1, 
		oldP.z + floorFlag,
	};
	return(newP);
}

extern inline struct point3 GET_E_COORD(struct point3 oldP,
				        int32_t floorFlag)
{
	struct point3 newP = {
		oldP.x + 1, 
		oldP.y, 
		oldP.z + floorFlag,
	};
	return(newP);
}

extern inline struct point3 GET_S_COORD(struct point3 oldP,
				        int32_t floorFlag)
{
	struct point3 newP = {
		oldP.x, 
		oldP.y - 1, 
		oldP.z + floorFlag,
	};
	return(newP);
}

extern inline struct point3 GET_W_COORD(struct point3 oldP,
				        int32_t floorFlag)
{
	struct point3 newP = {
		oldP.x - 1, 
		oldP.y, 
		oldP.z + floorFlag,
	};
	return(newP);
}

extern inline struct point3 GET_NE_COORD(struct point3 oldP)
{
	struct point3 newP = {
		oldP.x + 1, 
		oldP.y + 1, 
		oldP.z,
	};
	return(newP);
}

extern inline struct point3 GET_SE_COORD(struct point3 oldP)
{
	struct point3 newP = {
		oldP.x + 1, 
		oldP.y - 1, 
		oldP.z,
	};
	return(newP);
}

extern inline struct point3 GET_SW_COORD(struct point3 oldP)
{
	struct point3 newP = {
		oldP.x - 1, 
		oldP.y - 1, 
		oldP.z,
	};
	return(newP);
}

extern inline struct point3 GET_NW_COORD(struct point3 oldP)
{
	struct point3 newP = {
		oldP.x - 1, 
		oldP.y + 1, 
		oldP.z,
	};
	return(newP);
}

extern inline bool tile_GetStepped(struct game_world *map,
				   struct point3 p)
{
	return(map->tiles[(p.z * (map->sizeX * map->sizeY))
		+ p.y * map->sizeX + p.x].stepped);
}

extern inline void tile_SetStepped(struct game_world *map,
				  struct point3 p,
				  bool val)
{
	map->tiles[(p.z * (map->sizeX * map->sizeY))
		+ p.y * map->sizeX + p.x].stepped = val;	
}

extern inline struct step_data tile_GetStepData(struct game_world *map,
						struct point3 p)
{
	return(map->tiles[(p.z * (map->sizeX * map->sizeY))
		+ p.y * map->sizeX + p.x].stepData);
}

extern inline void tile_SetStepData(struct game_world *map,
				    struct point3 p,
				    struct step_data newData)
{
	map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].stepData = newData;	
}

extern inline int32_t tile_GetWalled(struct game_world *map,
				     struct point3 p)
{
	return(map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].walled);		
}

extern inline void tile_SetWalled(struct game_world *map,
			    	  struct point3 p,	
			    	  int32_t val)
{
	map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].walled = val;	
}

extern inline struct tile_data tile_GetTile(struct game_world *map,
					   struct point3 p)
{
	
	return(map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x]);		
}

extern inline struct tile_data *tile_GetTilePtr(struct game_world *map,
						struct point3 p)
{
	return(&map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x]);		
}

extern inline int32_t tile_GetElevation(struct game_world *map,
					struct point3 p)
{
	if(p.x < 0) p.x = 0;
	if(p.y < 0) p.y = 0;
	
	return(map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].elevation);		
}

extern inline void tile_SetElevation(struct game_world *map,
	         		     struct point3 p,	
				     int32_t val)
{
	if(p.x < 0) p.x = 0;
	if(p.y < 0) p.y = 0;
	
	map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].elevation = val;	
}

extern inline float tile_GetElevationOffset(struct game_world *map,
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

extern inline int32_t tile_GetPass(struct game_world *map,
				   struct point3 p)
{
	return(map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].impassable);		
}

extern inline void tile_SetPass(struct game_world *map,
			        struct point3 p,
			        int32_t val)
{
	map->tiles[(p.z * (map->sizeX * map->sizeY)) + 
		p.y * map->sizeX + p.x].impassable = val;	
}

