#pragma once
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

struct step_data {
	float minElev;
	float maxElev;
	uint8_t rotation;
};

struct tile_data {		
	bool impassable;
	uint8_t walled;
	uint8_t elevation;
	bool stepped;
	struct step_data stepData;
	int8_t z;
};