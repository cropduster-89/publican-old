#pragma once

enum render_type {
	render_head = 128,
	render_body = 256
};

enum event_type {
	event_null,
	
	event_bodyL,	
	event_bodyR,	
	event_hairL,	
	event_hairR,	
	event_faceL,	
	event_faceR,	
	event_dressL,	
	event_dressR,	
	event_gender,
	
	event_save,
	event_export,
};

struct current_char {
	int32_t body;
	int32_t hair;
	int32_t face;
	int32_t dress;	
	
	bool gender;
};

#define WX_CHARLIST 701
#define WX_FIRSTNAME 702
#define WX_LASTNAME 703
#define WX_AGE 704
#define WX_RACE 705

#define WX_SAVE 706
#define WX_EXPORT 707

#define WX_JOB 708

#define WX_HAIRL 713
#define WX_HAIRR 714
#define WX_FACEL 715
#define WX_FACER 716
#define WX_DRESSL 717
#define WX_DRESSR 718
#define WX_BODYL 719
#define WX_BODYR 720

#define WX_GENDER 721

#define WX_STR 750
#define WX_CHR 751
#define WX_INT 752
#define WX_DEX 753
#define WX_FOR 753
#define WX_PER 753


struct win32_gui_control {
	HWND mainWindow;	
		
	HWND charList;	
	HWND fNameEdit;	
	HWND lNameEdit;	
	HWND ageEdit;	
	HWND raceCombo;	
	HWND strEdit;	
	HWND chrEdit;	
	HWND intEdit;	
	HWND dexEdit;	
	HWND fortEdit;	
	HWND perEdit;	
	HWND jobCombo;
	HWND genderCheck;
	
	HWND hairL;	
	HWND hairR;	
	HWND faceL;	
	HWND faceR;	
	HWND dressL;	
	HWND dressR;	
	HWND bodyL;	
	HWND bodyR;	
	
	HWND saveChar;	
	HWND exportTown;
	
	struct loaded_bmp male;
	struct loaded_bmp female;	
	struct current_char currentChar;
};

struct char_node {
	struct loaded_char chara;
	struct char_node *next;
	struct char_node *prev;
};