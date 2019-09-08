/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	UI processing and rendering 
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 
		    @still very much place-holder
		    @if it works, no complaining
*************************************************************************************/
#ifndef PUB_UI_H
#define PUB_UI_H

/********************************************************
		FOR ALL UI ENUMS:
		items that are always displayed come first,
		starting from 0
********************************************************/

enum ui_element_sate {
	UISTATE_ALWAYSON,
	UISTATE_VISIBLE,
};

enum panel_type {
	panel_furn,
	panel_char,
	panel_relations,	
	
	panel_count
};

enum button_type {
	button_down,
	button_up,		
	button_collision,
	button_scrollup,
	button_scroll,
	button_scrolldown,
	
	button_count
};

struct ui_element {
	uint32_t type;
	uint8_t state;	
	union vec2 pos;	
	float height;
};

struct ui_furnpanel {
	char name[32];	
	struct point2 namePos;
};

struct ui_charpanel {
	char name[128];	
	struct point2 namePos;
	
	char drunk[32];
	struct point2 drunkTextPos;
	struct point2 drunkBarPos;
	char thirst[32];
	struct point2 thirstTextPos;
	struct point2 thirstBarPos;
	char bladder[32];
	struct point2 bladderTextPos;
	struct point2 bladderBarPos;
	
	struct chara_stats *stats;
	struct point2 relationsTextPos;
	struct point2 relationsValPos;
	
	struct point2 scrollUp;
	struct point2 scrollDown;
	struct point2 scrollButton;
	
	uint8_t *relationNames[];
	//int32_t relationNames[];	
};

struct ui_control {
	struct ui_element buttons[button_count];
	struct ui_element panels[panel_count];
	
	struct ui_charpanel charPanel;	
	struct ui_furnpanel furnPanel;
	
	union vec2 cursorPos;
		
	bool init;	
};

#endif