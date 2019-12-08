#ifndef PUB_H
#define PUB_H
/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	Header for base gameplay.
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 
		 
*************************************************************************************/

#define BAR_STAFF 0
#define BAR_PUNTER 0

struct map_data {
	int32_t sizeX;
	int32_t sizeY;
	int32_t sizeZ;
	
	int32_t activeX;
	int32_t activeXLen;
	int32_t activeY;
	int32_t activeYLen;
	
	float typicalFloorHeight;		
	
	union vec3 mapDimInMetres;
	
	struct tile_data *tiles;
};

struct world_cam {
	union vec3 pos;
	union vec3 lastPos;
	union vec3 offset;
	
	float orbit;
	float pitch;
	float dolly;
	
	float editOffset;	
};

struct world_alerts {
	bool serveAlert;
	struct entity *bars[36];
	uint32_t barCount;
};

struct world_mode {
	struct map_data map;
	struct game_time timer;
	struct world_cam cam;
	
	union vec3 mouseCoords[8];
	union vec3 mouseCoordsAbove[8];
	union vec3 mouseCoordsBelow[8];
	union vec3 mouseRay[8];
	union vec3 mouseRayBase;
	
	int8_t elevFlag;
				
	struct ui_control ui;
	
	bool collisionOverlay;
	
	int8_t currentFloor;
	
	struct world_alerts alerts;
	
	uint32_t charCount;
	struct entity_char characters[1024];
	
	uint32_t furnCount;
	struct entity_furn furniture[1024];
	
	uint32_t strucCount;
	struct entity_struc structures[5012];
	
	uint32_t entityCount;
	struct entity entities[16384];
	
	uint32_t onscreenCount;
		
	uint32_t lastEnt;
	struct entity *lastOnscreen;
	
	struct entity_target mouseOverEnt;
	struct entity_target selectedEnt;
	
	
	struct edit_state edit;			
};

#endif