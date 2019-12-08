#pragma once
#ifndef PUB_UI_H
#define PUB_UI_H

/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	UI processing and rendering 
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 
		    
*************************************************************************************/

#define DEF_LABEL_SIZE 32

enum ui_element_sate {
	UISTATE_ALWAYSON = 0,
	UISTATE_VISIBLE = 8,
	UISTATE_CLIPPED = 16,
};

enum element_type {
	UITYPE_PANEL,
	UITYPE_STATIC_LABEL,
	UITYPE_DYNAMIC_LABEL,
	UITYPE_TEXTBUTTON,
	UITYPE_BAR,
};

enum element_alias {
	UIALIAS_BOTTOMPANEL,
	UIALIAS_FLOORUP,
	UIALIAS_FLOORDOWN,
	UIALIAS_BUILD,
	UIALIAS_CHARPANEL,
	UIALIAS_CHARNAME,
	UIALIAS_CHARTHIRSTLBL,
	UIALIAS_CHARBLADDERLBL,
	UIALIAS_CHARDRUNKLBL,
	UIALIAS_CHARTHIRSTBAR,
	UIALIAS_CHARBLADDERBAR,
	UIALIAS_CHARDRUNKBAR,
	
	UI_COUNT,
};

struct world_mode;
#define BUTTON_EVENT_CALLBACK(event) void event(struct world_mode *world, int32_t code)
typedef BUTTON_EVENT_CALLBACK(button_event_callback);

struct ui_bar {	
	union vec4 colour;
};

struct ui_label {
	enum asset_type_id text;
	float scale;
};

struct ui_text_button {
	enum asset_type_id text;
	union vec2 textOffset;
	float scale;	
	union vec4 colour;		
};

struct ui_panel {	
	float scale;
	union vec4 colour;
};

struct ui_element {
	enum element_type type;
	enum element_alias alias;
	struct ui_element *parent;	
	button_event_callback *callback;
	uint64_t state;	
	union vec2 pos;	
	union vec2 dim;
	uint32_t handle;
	union {
		struct ui_panel panel;
		struct ui_label label;
		struct ui_text_button textButton;		
		struct ui_bar bar;		
	};	
};

struct ui_control {	
	bool init;	
	struct ui_element elements[UI_COUNT];
};

#endif