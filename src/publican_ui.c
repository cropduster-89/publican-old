/************************************************************************************		
__________     ___.   .__  .__                      	
\______   \__ _\_ |__ |  | |__| ____ _____    ____  
 |     ___/  |  \ __ \|  | |  |/ ___\\__  \  /    \ 	UI processing and rendering 
 |    |   |  |  / \_\ \  |_|  \  \___ / __ \|   |  \
 |____|   |____/|___  /____/__|\___  >____  /___|  /
                    \/             \/     \/     \/ 
		    
*************************************************************************************/

#define BUTTON_X 32.0f
#define PANEL_X 256.0f
#define PANEL_Y 512.0f

#define PANELVIS(type) (BITCHECK(world->ui.panels[type].state, UISTATE_VISIBLE))
#define BUTTONVIS(type) (BITCHECK(world->ui.buttons[type].state, UISTATE_VISIBLE))

#define PANELVIS_DEF(type) (BITCHECK(world->ui.panels[type].state, UISTATE_ALWAYSON))
#define BUTTONVIS_DEF(type) (BITCHECK(world->ui.buttons[type].state, UISTATE_ALWAYSON))

extern inline bool DEF_VIS_B(int32_t i)
{
	return(i < button_scrollup ? true : false);
}

extern inline bool DEF_VIS_P(int32_t i)
{
	return(i < panel_furn ? true : false);
}

static struct ui_element ui_InitPanel(enum panel_type type)
{
	union vec2 pos = {};
	uint8_t state = type < panel_furn ? 0x1 : 0x0;
	float x = -(WIN_X / 2);
	float y = -(WIN_Y / 2);	
	float offset = 8.0f;
	float height = 0;
	
	switch(type) {
	case panel_furn: 
	case panel_char: {
		pos = RealToVec2(x + (WIN_X - offset - PANEL_X), 0);	
		height = 512.0f;	
		break;
	} case panel_relations: {
		pos = RealToVec2(x + (WIN_X - offset - PANEL_X) + 24, 32);
		height = 256.0f;
		break;
	} default: {
		break;
	}	
	}
	struct ui_element result = {
		type, state, pos, height
	};
	return(result);
}

static struct ui_element ui_InitButton(enum button_type type)
{	
	union vec2 pos = {};
	uint8_t state = type < button_scrollup ? 0x1 : 0x0;
	float x = -(WIN_X / 2);
	float y = -(WIN_Y / 2);
	float stride = 40.0f;
	float offset = 8.0f;
	
	switch(type) {
	case button_down: {
		pos = RealToVec2(x + (WIN_X - stride), y + offset);
		break;
	} case button_up: {
		pos = RealToVec2(x + (WIN_X - stride), y + offset + stride);
		break;
	} case button_collision: {
		pos = RealToVec2(x + (WIN_X - stride), y + offset + (stride * 2));
		break;
	} case button_scrollup: {
		pos = RealToVec2(x + (WIN_X - stride * 2) + 20, y + (WIN_Y * .75) - 14);
		break;
	} case button_scrolldown: {
		pos = RealToVec2(x + (WIN_X - stride * 2) + 20, y + (WIN_Y * .75) - 238);
		break;
	} case button_scroll: {
		pos = RealToVec2(x + (WIN_X - stride * 2) + 20, y + (WIN_Y * .75) - 45);
		break;
	} default: {
		INVALID_PATH;
	}	
	}
		
	struct ui_element result = {
		type, state, pos, 32.0f,
	};
	return(result);
}

static inline struct rect2 ui_GetButtonDim(union vec2 pos)
{
	struct rect2 result = {
		pos,
		{.x = pos.x + BUTTON_X, .y = pos.y + BUTTON_X}
	};
	return(result);
}

static inline struct rect2 ui_GetPanelDim(union vec2 pos)
{
	struct rect2 result = {
		pos,
		{.x = pos.x + PANEL_X, .y = pos.y + PANEL_Y}
	};
	return(result);
}

static void ui_DrawButton(struct render_group *group,
			  struct temp_state *tState,	
			  struct ui_element button)
{
	struct object_transform trans = {};
	trans.offset.xy = button.pos;
	
	render_PushBMP(group, trans, asset_FindBMP(tState->assets, 
		asset_buttons, button.type), button.height, uiOffset, 0);	
}

static void ui_DrawPanel(struct render_group *group,
			 struct temp_state *tState,	
			 struct ui_element panel)
{
	struct object_transform trans = {};
	trans.offset.xy = panel.pos;
	
	render_PushBMP(group, trans, asset_FindBMP(tState->assets, 
		asset_panels, panel.type), panel.height, uiOffset, 0);	
}

extern void ui_TextOut(char *string,
		       struct render_group *renderGroup,
		       struct temp_state *tState,
		       struct point2 startPos,	
		       float scale)
{		
	float lineWeight = 20.0f;	
	struct object_transform trans = render_FlatTrans();
	trans.offset.x = startPos.x;
	trans.offset.y = startPos.y;
	uint32_t len = text_StringCount(string);	
	char *current = string;		
	float prevX = 0;
	float prevSizeX = 0;
	for(uint32_t i = 0; i < len; ++i, ++current) {
		if(*current == ' ') {
			trans.offset.x += 15;
			continue;
		} else if(*current == '\n') {
			trans.offset.y -= lineWeight;
		} else if(*current == '\0') {
			break;
		}		
		struct bmp_id id = asset_FindChar(tState->assets, asset_font, _CHAR(*current));	
		struct loaded_bmp *bmp = assets_GetBMP(tState->assets, id);							   
		if(!bmp) {
			assets_LoadBMP(renderGroup->assets, id, true, final_bmp);
			return;
		} else {
			union vec2 size = {
				.x = bmp->w,
				.y = bmp->h,
			};
			union vec2 align = {
				.x = bmp->alignX,
				.y = bmp->alignY,
			};
			
			align = MultVec2(align, 1.0f);				
			align = MultVec2(align, scale);		
			size = MultVec2(size, 1.0f);		
			size = MultVec2(size, scale);				
								
			trans.offset.x +=  align.x + prevX + prevSizeX;				
			trans.offset.y -=  size.y + align.y;
			
			//trans = render_UprightTrans();		
			
			prevX = align.x;
			prevSizeX = size.x;
			render_PushBMP(renderGroup, trans, id, size.y, uiOffset, 1.0f);			
			
		}				
	}		
}

static void ui_InitFurnPanel(struct ui_furnpanel *furnPanel,
			     struct rect2 dim)
{	
	furnPanel->namePos.x = dim.min.x + 20;
	furnPanel->namePos.y = dim.max.y - 30;
}

#define CPANEL_B_START button_scrollup
#define CPANEL_B_END button_scrolldown
#define CPANEL_P_START panel_char
#define CPANEL_P_END panel_relations

#define FPANEL_P_START panel_furn
#define FPANEL_P_END panel_furn

#define ui_StateClear(elements) \
	_ui_ChangeState(elements, 0, 0, ARRAY_COUNT(elements), true)
	
#define ui_ChangeState(elements, start, end) \
	_ui_ChangeState(elements, start, end, ARRAY_COUNT(elements), false)
	
static void _ui_ChangeState(struct ui_element elements[],
			    int32_t start,
			    int32_t end,		
			    int32_t count,
			    bool fullClear)
{	
	for(int32_t i = 0; i < count; ++i) {
		
		if(BITCHECK(elements[i].state, UISTATE_ALWAYSON)) {continue;}		
		else if(fullClear) {BITCLEAR(elements[i].state, UISTATE_VISIBLE);}			
		else if(i >= start && i <= end) {BITSET(elements[i].state, UISTATE_VISIBLE);}	
		else {BITCLEAR(elements[i].state, UISTATE_VISIBLE);}	
	
	}		
}

static void ui_InitCharPanel(struct ui_charpanel *charPanel,
			     struct rect2 dim)
{	
	charPanel->namePos.x = dim.min.x + 20;
	charPanel->namePos.y = dim.max.y - 30;		
	strcpy(charPanel->drunk, "Intoxication:");
	charPanel->drunkTextPos.x = dim.min.x + 20;
	charPanel->drunkTextPos.y = dim.max.y - 60;	
	charPanel->drunkBarPos.x = charPanel->drunkTextPos.x + 100;
	charPanel->drunkBarPos.y = charPanel->drunkTextPos.y;	
	strcpy(charPanel->thirst, "Thirst:"); 
	charPanel->thirstTextPos.x = charPanel->drunkTextPos.x;
	charPanel->thirstTextPos.y = charPanel->drunkTextPos.y - 15;	
	charPanel->thirstBarPos.x = charPanel->thirstTextPos.x + 100;
	charPanel->thirstBarPos.y = charPanel->thirstTextPos.y;	
	strcpy(charPanel->bladder, "Bladder:");
	charPanel->bladderTextPos.x = charPanel->thirstTextPos.x;
	charPanel->bladderTextPos.y = charPanel->thirstTextPos.y - 15;	
	charPanel->bladderBarPos.x = charPanel->bladderTextPos.x + 100;
	charPanel->bladderBarPos.y = charPanel->bladderTextPos.y;
	
	charPanel->relationsTextPos.x = charPanel->bladderTextPos.x;
	charPanel->relationsTextPos.y = charPanel->bladderTextPos.y - 50;	
	charPanel->relationsValPos.x = charPanel->relationsTextPos.x + 100;
	charPanel->relationsValPos.y = charPanel->relationsTextPos.y;
}
/*
*		
*		
*		
*/
static void ui_InitWorldUI(struct world_mode *world)
{
	for(int32_t i = 0; i < button_count; ++i) {
		world->ui.buttons[i] = ui_InitButton(i);
		
	}
	for(int32_t i = 0; i < panel_count; ++i) {		
		world->ui.panels[i] = ui_InitPanel(i);		
	}
	
	ui_InitCharPanel(&world->ui.charPanel, ui_GetPanelDim(world->ui.panels[panel_char].pos));
	ui_InitFurnPanel(&world->ui.furnPanel, ui_GetPanelDim(world->ui.panels[panel_furn].pos));
		
	world->ui.init = true;
}
/*
*		Checks is mouse click corresponds with button,
*		and processes result. 
*
*		A bit pony atm
*/
extern bool ui_Process(struct world_mode *world, bool click)
{
	bool result = false;
	
	int32_t panelIndex = 0;
	int32_t buttonIndex = 0;
	
	for(int32_t i = 0; i < button_count; ++i) {
		struct rect2 dim = ui_GetButtonDim(world->ui.buttons[i].pos);	
		if(world->ui.cursorPos.x >= dim.min.x && world->ui.cursorPos.y >= dim.min.y && 
		   world->ui.cursorPos.x <= dim.max.x && world->ui.cursorPos.y <= dim.max.y &&
		   (BUTTONVIS(i) || BUTTONVIS_DEF(i))) {
			
			buttonIndex = i;
			result = true;				
			break;
		}
	}	
	for(int32_t i = 0; i < panel_count; ++i) {
		struct rect2 dim = ui_GetPanelDim(world->ui.panels[i].pos);	
		if(world->ui.cursorPos.x >= dim.min.x && world->ui.cursorPos.y >= dim.min.y && 
		   world->ui.cursorPos.x <= dim.max.x && world->ui.cursorPos.y <= dim.max.y &&
		   BITCHECK(world->ui.panels[i].state, UISTATE_VISIBLE)) {
			   
			panelIndex = i;
			result = true;	
			break;	
		}
	}	
	//if(click) {
	//	switch(world->ui.buttons[buttonIndex].type) {
	//	case 0: {
	//		--world->currentFloor;
	//		break;
	//	} case 1: {
	//		++world->currentFloor;
	//		break;
	//	} case 2: {
	//		if(!world->collisionOverlay) world->collisionOverlay = true;
	//		else world->collisionOverlay = false;
	//		break;
	//	} default: break;;
	//	}	
	//}	
	return(result);
}

extern void ui_WorldMode(struct world_mode *world,
			 struct pub_input *input,
			 struct render_group *group,
			 struct temp_state *tState)
{
	if(!world->ui.init) {
		ui_InitWorldUI(world);
	}	
	world->ui.cursorPos = Unproject(group, RealToVec2(input->mouseX, 
		input->mouseY), 0).xy;	
	
	for(int32_t i = 0; i < panel_count; ++i) {
		if(PANELVIS(i) || DEF_VIS_P(i)) {			
			ui_DrawPanel(group, tState, world->ui.panels[i]);
		}	
	}
	for(int32_t i = 0; i < button_count; ++i) {
		if(BITCHECK(world->ui.buttons[i].state, UISTATE_VISIBLE) || DEF_VIS_B(i))
			ui_DrawButton(group, tState, world->ui.buttons[i]);
	}	
		
	if(BITCHECK(world->ui.panels[panel_char].state, UISTATE_VISIBLE)) {		
		
		struct ui_charpanel *charPanel = &world->ui.charPanel;	
		ui_TextOut(charPanel->name, group, tState, charPanel->namePos, 0.3f);	
		ui_TextOut(charPanel->drunk, group, tState, charPanel->drunkTextPos, 0.2f);			
		ui_TextOut(charPanel->thirst, group, tState, charPanel->thirstTextPos, 0.2f);			
		ui_TextOut(charPanel->bladder, group, tState, charPanel->bladderTextPos, 0.2f);
		render_PushRect(group, RealToVec3(charPanel->drunkBarPos.x, charPanel->drunkBarPos.y, 51), 
			RealToVec2(charPanel->stats->drunk * 100, 10),
			RealToVec4(0.0, 1.0f, 0.0, 1.0f));	
		render_PushRect(group, RealToVec3(charPanel->thirstBarPos.x, charPanel->thirstBarPos.y, 51), 
			RealToVec2((charPanel->stats->thirst * 100), 10),
			RealToVec4(0.0, 1.0f, 0.0, 1.0f));				
		render_PushRect(group, RealToVec3(charPanel->bladderBarPos.x, charPanel->bladderBarPos.y, 51), 
			RealToVec2(charPanel->stats->bladder * 100, 10),
			RealToVec4(0.0, 1.0f, 0.0, 1.0f));
		
		
		for(int32_t i = 0; i < charPanel->stats->relationCount; ++i) {
			struct point2 pos = {
				charPanel->relationsTextPos.x,
				charPanel->relationsTextPos.y - (i * 20),
			}; 	
			ui_TextOut(GET_CHARFROMENTINDEX(world, charPanel->stats->relations[i].index)
				->stats.firstName, 
				group, tState, pos, 0.2f);
			
		}	
			
	} else if(BITCHECK(world->ui.panels[panel_furn].state, UISTATE_VISIBLE)) {			
		struct ui_furnpanel *furnPanel = &world->ui.furnPanel;	
		ui_TextOut(furnPanel->name, group, tState, furnPanel->namePos, 0.3f);				
	}
	struct object_transform mouseTrans = {};
	union vec2 mouseP = {
		.x = input->mouseX - (WIN_X / 2),
		.y = input->mouseY - (WIN_Y / 2),
	};
	mouseTrans.offset.xy = mouseP;
	render_PushBMP(group, mouseTrans, 
			asset_FindBMP(tState->assets, asset_cursor, 0),
			32.0f, EMPTY_V3, 1);	
} 

extern void ui_OpenPanel(struct world_mode *world,
		         enum panel_type type)
{	
	switch(type) {
	case panel_char: {	
		ui_StateClear(world->ui.panels);
		ui_StateClear(world->ui.buttons);
		
		ui_ChangeState(world->ui.panels, CPANEL_P_START, CPANEL_P_END);	
		ui_ChangeState(world->ui.buttons, CPANEL_B_START, CPANEL_B_END);
		
		struct entity_char *chara = GET_CHAR(world->selectedEnt.ent);			
		sprintf(world->ui.charPanel.name, "%s %s", 
			chara->stats.firstName, chara->stats.lastName);
		world->ui.charPanel.stats = &chara->stats;				
		
		break;
	} case panel_furn: {
		BITCLEAR(world->ui.panels[panel_char].state, UISTATE_VISIBLE);
		ui_StateClear(world->ui.panels);
		ui_StateClear(world->ui.buttons);
		if(!PANELVIS(panel_furn)) {
			ui_ChangeState(world->ui.panels, FPANEL_P_START, FPANEL_P_END);	
			//ui_ChangeState(world->ui.buttons);
		}	
		
		if(world->selectedEnt.ent->alias == alias_table2x1) {				
			strcpy(world->ui.furnPanel.name, "Table");				
		} else if(world->selectedEnt.ent->alias == alias_bar) {
			strcpy(world->ui.furnPanel.name, "Bar");
		} else if(world->selectedEnt.ent->alias == alias_stool) {
			strcpy(world->ui.furnPanel.name, "Stool");
		}		
		world->ui.charPanel.stats = NULL;
		break;
	} default: {
		BITCLEAR(world->ui.panels[panel_furn].state, UISTATE_VISIBLE);
		BITCLEAR(world->ui.panels[panel_char].state, UISTATE_VISIBLE);
		ui_StateClear(world->ui.panels);
		ui_StateClear(world->ui.buttons);
		world->ui.charPanel.stats = NULL;			
		break;
	}
	}	
}