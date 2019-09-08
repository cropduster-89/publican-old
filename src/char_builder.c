#include <windows.h>
#include <Windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_ONLY_TGA
#include "stb_image.h"


struct memory_arena {
	struct platform_memory_block *currentBlock;
	uintptr_t minBlockSize;
	
	uint64_t allocationFlags;		
	int32_t tempCount;
};

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

#include "publican_business.h"
#include "publican_ai.h"
#include "publican_assets.h"
#include "char_builder.h"

static const int32_t winX = 925;
static const int32_t winY = 512;

	
static enum event_type event;	
	

static HINSTANCE globalHInstance;
static bool running;

#define HEAD_OFFSET 128
#define BODY_OFFSET 256

#define BODY_MAX 3
#define FACE_MAX 3
#define HAIR_MAX 9
#define DRESS_MAX 3

#define DOC_X 1024
#define DOC_Y 4736

struct display_buffer {
	BITMAPINFO info;
	void *data;
	int32_t x;
	int32_t y;
	int32_t pitch;
	int32_t bpp;
} bmpBuffer;

struct char_node *charList = NULL; 
struct char_node *charListCurrent = NULL; 

static int32_t CountNodes(void)
{
	struct char_node *current = charList;
	int32_t result = 0;
	
	if(!charList) {
		printf("nothing to export\n");
		return(0);
	}
	
	while(current->next != NULL) {
		++result;
		current = current->next;		
	}
	return(result + 1);
}

static void AddNode(struct loaded_char chara)
{
	struct char_node *new = malloc(sizeof(struct char_node));	
	new->chara = chara;
	new->next = NULL;
	new->prev = NULL;
	
	if(!charList) {
		charList = new;	
		charListCurrent = charList;
	} else {		
		charListCurrent->next = new;
		new->prev = charListCurrent;
		charListCurrent = charListCurrent->next;
	}
}

static void AddToListbox(struct win32_gui_control *control,
			 struct loaded_char chara)
{
	char buffer[64];
	sprintf(buffer, "%s %s", chara.firstName, chara.lastName);
	SendMessage(control->charList, LB_ADDSTRING, 0, (LPARAM)buffer);
}

static void ClearBuffer()
{
	
	uint32_t *data = (uint32_t *)bmpBuffer.data;
	for(int32_t i = 0; i < 128 * 256; ++i) {
		*data++ = 0xFFDDBB55;
	}
}

static void WriteToBuffer(void *input,
			  uint32_t startX,
			  uint32_t startY,
			  enum render_type type)
{
	uint32_t offset = 0;
	if(type == render_head) offset = render_head;
	uint32_t *dest = (uint32_t *)bmpBuffer.data + (offset * 128);
	uint32_t *src = (uint32_t *)input + ((startY * DOC_X) + startX);
	
	uint32_t max = (type == render_body) ? (render_body * render_head) : (render_head * render_head);
	
	for(int32_t i = 1; i <= max; ++i) {
		if(i % render_head == 0) src -= (DOC_X + render_head);
		uint8_t bytes[4];
		bytes[0] = (*src >> 24) & 0xFF;	
		bytes[1] = (*src >> 16) & 0xFF;	
		bytes[2] = (*src >> 8) & 0xFF;	
		bytes[3] = (*src >> 0) & 0xFF;	
				
		uint32_t final = 
			bytes[1] | 
			(bytes[2] << 8) |
			(bytes[3] << 16) |
			(bytes[0] << 24);		
		
		if(bytes[0] != 0) {
			*dest = final;			
		} 
		++dest;
		++src;
	}		
}

static void Redraw(struct win32_gui_control *control)
{
	ClearBuffer();
	void *data = NULL;
	if(!control->currentChar.gender) {
		data = control->male.data;
	} else {
		data = control->female.data;
	}
	uint32_t startX = 0;
	uint32_t startY = 0;	
	
	uint32_t dressStart = render_body * BODY_MAX;
	uint32_t headStart = dressStart + render_body * (DRESS_MAX * 3);
	uint32_t hairStart = headStart + render_head;
	uint32_t faceStart = hairStart + (HAIR_MAX * render_head);
	
	startX = render_head * 5;
	startY = DOC_Y - (render_body * control->currentChar.body);	
	WriteToBuffer(data, startX, startY, render_body);	
	
	startX = render_head * 5;
	startY = DOC_Y - (dressStart + ((render_body * control->currentChar.dress) + 
		(render_body * 3 * control->currentChar.body)));	
	WriteToBuffer(data, startX, startY, render_body);
	
	startX = render_head * 5;
	startY = DOC_Y - headStart;	
	WriteToBuffer(data, startX, startY, render_head);

	startX = render_head * 5;
	startY = DOC_Y - (hairStart + control->currentChar.hair * render_head);	
	WriteToBuffer(data, startX, startY, render_head);
	
	startX = render_head * 5;
	startY = DOC_Y - (faceStart + control->currentChar.face * render_head);	
	WriteToBuffer(data, startX, startY, render_head);	
}

static void ChangeSelection(int32_t *current,
			    int32_t max,
			    bool isRight)
{		
	if(!isRight) {
		*current -= 1;		
		if(*current < 0) {*current = max - 1;} 
	} else {
		*current += 1;
		if(*current > max - 1) {*current = 0;} 
	}		
}

static bool SaveChar(struct win32_gui_control *control)
{
	bool result = true;
	struct loaded_char newChar = {};
	
	Edit_GetText(control->fNameEdit, newChar.firstName, 32);
	assert(newChar.firstName);
	
	Edit_GetText(control->lNameEdit, newChar.lastName, 32);
	assert(newChar.lastName);
	
	char buffer[10], *endPtr;
	Edit_GetText(control->ageEdit, buffer, 4);
	newChar.age = strtoimax(buffer, &endPtr, 10);
	assert(newChar.age);	
	
	Edit_GetText(control->strEdit, buffer, 4);
	newChar.attribs.strength = strtoumax(buffer, &endPtr, 10);
	assert(newChar.attribs.strength);
	
	Edit_GetText(control->dexEdit, buffer, 4);
	newChar.attribs.dexterity = strtoumax(buffer, &endPtr, 10);
	assert(newChar.attribs.dexterity);
	
	Edit_GetText(control->intEdit, buffer, 4);
	newChar.attribs.intelligence = strtoumax(buffer, &endPtr, 10);
	assert(newChar.attribs.intelligence);
	
	Edit_GetText(control->chrEdit, buffer, 4);
	newChar.attribs.charisma = strtoumax(buffer, &endPtr, 10);
	assert(newChar.attribs.charisma);
	
	Edit_GetText(control->fortEdit, buffer, 4);
	newChar.attribs.fortitude = strtoumax(buffer, &endPtr, 10);
	assert(newChar.attribs.fortitude);
	
	Edit_GetText(control->perEdit, buffer, 4);
	newChar.attribs.perception = strtoumax(buffer, &endPtr, 10);
	assert(newChar.attribs.perception);
	
	newChar.race = SendMessage(control->raceCombo, CB_GETCURSEL, 0, 0);
	assert(newChar.race != CB_ERR);
	++newChar.race;
	
	newChar.job = SendMessage(control->jobCombo, CB_GETCURSEL, 0, 0);
	assert(newChar.job != CB_ERR);
	++newChar.job;
	
	newChar.body = control->currentChar.body;
	newChar.dress = control->currentChar.dress;
	newChar.face = control->currentChar.face;
	newChar.hair = control->currentChar.hair;
	newChar.gender = control->currentChar.gender;
	
	AddNode(newChar);
	AddToListbox(control, newChar);	
	
	struct current_char newCurrent = {};
	control->currentChar = newCurrent;
	
	SendMessage(control->fNameEdit, WM_SETTEXT, 0 , (LPARAM)"\0");
	SendMessage(control->lNameEdit, WM_SETTEXT, 0 , (LPARAM)"\0");
	SendMessage(control->ageEdit, WM_SETTEXT, 0 , (LPARAM)"\0");
	SendMessage(control->strEdit, WM_SETTEXT, 0 , (LPARAM)"\0");
	SendMessage(control->chrEdit, WM_SETTEXT, 0 , (LPARAM)"\0");
	SendMessage(control->intEdit, WM_SETTEXT, 0 , (LPARAM)"\0");
	SendMessage(control->dexEdit, WM_SETTEXT, 0 , (LPARAM)"\0");
	
	return(result);
}

static void InitBMPBuffer(int32_t x, int32_t y)
{
	bmpBuffer.x = x;
	bmpBuffer.y = y;
	bmpBuffer.bpp = 4;
	
	bmpBuffer.info.bmiHeader.biSize = sizeof(bmpBuffer.info.bmiHeader);
	bmpBuffer.info.bmiHeader.biWidth = x;
	bmpBuffer.info.bmiHeader.biHeight = y;
	bmpBuffer.info.bmiHeader.biPlanes = 1;
	bmpBuffer.info.bmiHeader.biBitCount= 32;
	bmpBuffer.info.bmiHeader.biCompression = BI_RGB;
	size_t bufferSize = (x * y) * bmpBuffer.bpp;
	bmpBuffer.data = VirtualAlloc(0, bufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	bmpBuffer.pitch = x * bmpBuffer.bpp;
	
	ClearBuffer();	
}

static void ExportFile(struct win32_gui_control *control)
{
	char title[MAX_PATH] = "";
	OPENFILENAME ofn = {};	
	ofn.lStructSize = sizeof(ofn);	
	ofn.hwndOwner = control->mainWindow;	
	ofn.lpstrFilter = "Char files (*.chara)\0*.chara\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = title;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = "chara";
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT|OFN_EXPLORER;		
	if(!GetSaveFileName(&ofn)) {
		return;		
	} 
	HANDLE fh = CreateFileA(title, GENERIC_WRITE, 0, 0, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);	
	
	struct char_file_header header;
	header.count = CountNodes();
	printf("%d\n", header.count);
	
	WriteFile(fh, &header, sizeof(struct char_file_header),
		NULL, NULL);
	
	struct char_node *current = charList;
	for(int32_t i = 0; i < header.count; ++i) {
		WriteFile(fh, &current->chara, sizeof(struct loaded_char),
		NULL, NULL);
		current = current->next;
	}
	
	CloseHandle(fh);
}

LRESULT CALLBACK WindowProc(HWND   window,
	UINT   msg,
	WPARAM wParam,
	LPARAM lParam)
{
	LRESULT result = 0;

	switch (msg) {
	case WM_DESTROY:
	case WM_CLOSE: {
		running = false;		
		break;
	} case WM_COMMAND: {
		switch(LOWORD(wParam)) {
		case WX_SAVE: {
			event = event_save;
			break;
		} case WX_EXPORT: {
			event = event_export;
			break;
		} case WX_HAIRL: {
			event = event_hairL;
			break;			
		} case WX_HAIRR: {
			event = event_hairR;	
			break;
		} case WX_FACEL: {
			event = event_faceL;
			break;
		} case WX_FACER: {
			event = event_faceR;
			break;
		} case WX_DRESSL: {
			event = event_dressL;
			break;
		} case WX_DRESSR: {
			event = event_dressR;
			break;
		} case WX_BODYL: {
			event = event_bodyL;
			break;
		} case WX_BODYR: {
			event = event_bodyR;
			break;
		} case WX_GENDER: {
			event = event_gender;
			break;
		} 
		 default: break;	
		}
		break;
	} case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(window, &ps);
		TextOutA(dc, 275, 13, "Name", 4);	
		TextOutA(dc, 275, 42, "Age", 4);	
		TextOutA(dc, 275, 70, "Race", 4);	
		TextOutA(dc, 275, 100, "Job", 4);
		TextOutA(dc, 650, 13, "Str", 4);	
		TextOutA(dc, 650, 42, "Char", 4);	
		TextOutA(dc, 650, 70, "Int", 4);	
		TextOutA(dc, 650, 100, "Dex", 4);
		TextOutA(dc, 650, 130, "Fort", 4);
		TextOutA(dc, 650, 160, "Per", 4);
		
		StretchDIBits(dc, 460, 35, 128, 256, 
			0, 0, 128, 256,
			bmpBuffer.data, &bmpBuffer.info,
			DIB_RGB_COLORS, SRCCOPY);
		
		EndPaint(window, &ps);	
		break;	
	} default: {
		result = DefWindowProcA(window, msg, wParam, lParam);
		break;
	}
	}
	return(result);
}

static bool win32_InitGui(struct win32_gui_control *control)
{
	bool result = true;
	
	WNDCLASSEX wClass = {};	
	wClass.cbSize = sizeof(WNDCLASSEX);
	wClass.style = CS_HREDRAW|CS_VREDRAW;
	wClass.lpfnWndProc = WindowProc;		
	wClass.lpszClassName = "SubWindow";
	wClass.hInstance = globalHInstance;
	wClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	if(!RegisterClassEx(&wClass)) {
		printf("RegisterClassEx Failed\n");
		PostQuitMessage(0);
	}	
	control->charList = CreateWindowExA(0, "LISTBOX", NULL, WS_CHILD|
		WS_VISIBLE|WS_BORDER, 5, 25, 250, winY - 70,								  		
		control->mainWindow, (HMENU)WX_CHARLIST,
		globalHInstance, NULL);
	
	control->fNameEdit = CreateWindowExA(0, WC_EDIT, NULL, WS_CHILD|
		WS_VISIBLE|WS_BORDER, 320, 5, 100, 25,								  		
		control->mainWindow, (HMENU)WX_FIRSTNAME,
		globalHInstance, NULL);	
	
	control->lNameEdit = CreateWindowExA(0, WC_EDIT, NULL, WS_CHILD|
		WS_VISIBLE|WS_BORDER, 425, 5, 100, 25,								  		
		control->mainWindow, (HMENU)WX_LASTNAME,
		globalHInstance, NULL);	
	
	control->strEdit = CreateWindowExA(0, WC_EDIT, NULL, WS_CHILD|
		WS_VISIBLE|WS_BORDER, 675, 5, 30, 25,							  		
		control->mainWindow, (HMENU)WX_STR,
		globalHInstance, NULL);	
	
	control->chrEdit = CreateWindowExA(0, WC_EDIT, NULL, WS_CHILD|
		WS_VISIBLE|WS_BORDER, 675, 35, 30, 25,							  		
		control->mainWindow, (HMENU)WX_CHR,
		globalHInstance, NULL);	

	control->intEdit = CreateWindowExA(0, WC_EDIT, NULL, WS_CHILD|
		WS_VISIBLE|WS_BORDER, 675, 65, 30, 25,							  		
		control->mainWindow, (HMENU)WX_INT,
		globalHInstance, NULL);	

	control->dexEdit = CreateWindowExA(0, WC_EDIT, NULL, WS_CHILD|
		WS_VISIBLE|WS_BORDER, 675, 95, 30, 25,							  		
		control->mainWindow, (HMENU)WX_DEX,
		globalHInstance, NULL);		
		
	control->fortEdit = CreateWindowExA(0, WC_EDIT, NULL, WS_CHILD|
		WS_VISIBLE|WS_BORDER, 675, 125, 30, 25,							  		
		control->mainWindow, (HMENU)WX_FOR,
		globalHInstance, NULL);	

	control->perEdit = CreateWindowExA(0, WC_EDIT, NULL, WS_CHILD|
		WS_VISIBLE|WS_BORDER, 675, 155, 30, 25,							  		
		control->mainWindow, (HMENU)WX_PER,
		globalHInstance, NULL);			
	
	control->ageEdit = CreateWindowExA(0, WC_EDIT, NULL, WS_CHILD|
		WS_VISIBLE|WS_BORDER, 320, 35, 100, 25,							  		
		control->mainWindow, (HMENU)WX_AGE,
		globalHInstance, NULL);		
	
	control->raceCombo = CreateWindowExA(0, WC_COMBOBOX, "", WS_CHILD|
		CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_VISIBLE, 320, 65, 100, 25,							  		
		control->mainWindow, (HMENU)WX_RACE,
		globalHInstance, NULL);		
	assert(control->raceCombo);
	char *races[] = {
		"Human", "Elf",
	};
	for(int32_t i = 0; i < ARRAY_COUNT(races); ++i) {
		SendMessage(control->raceCombo, CB_ADDSTRING, 0, (LPARAM)races[i]);			
	}	
	
	control->jobCombo = CreateWindowExA(0, WC_COMBOBOX, "", WS_CHILD|
		CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_VISIBLE, 320, 95, 100, 25,							  		
		control->mainWindow, (HMENU)WX_RACE,
		globalHInstance, NULL);		
	assert(control->jobCombo);
	char *jobs[] = {
		"Owner", "Bar Staff", "Blacksmith", "Builder", "Shopkeeper", 
		"Labourer", "Housewife", "Unemployed", "Militia", "Policemen",
		"Vagrant",
	};
	for(int32_t i = 0; i < ARRAY_COUNT(jobs); ++i) {
		SendMessage(control->jobCombo, CB_ADDSTRING,	0, (LPARAM)jobs[i]);			
	}
	
	control->hairL = CreateWindowExA(0, WC_BUTTON, "<", WS_CHILD|
		WS_VISIBLE|WS_BORDER, 435, 40, 20, 20,								  		
		control->mainWindow, (HMENU)WX_HAIRL,
		globalHInstance, NULL);
		
	control->hairR = CreateWindowExA(0, WC_BUTTON, ">", WS_CHILD|
		WS_VISIBLE|WS_BORDER, 592, 40, 20, 20,								  		
		control->mainWindow, (HMENU)WX_HAIRR,
		globalHInstance, NULL);	
		
	control->faceL = CreateWindowExA(0, WC_BUTTON, "<", WS_CHILD|
		WS_VISIBLE|WS_BORDER, 435, 70, 20, 20,								  		
		control->mainWindow, (HMENU)WX_FACEL,
		globalHInstance, NULL);
		
	control->faceR = CreateWindowExA(0, WC_BUTTON, ">", WS_CHILD|
		WS_VISIBLE|WS_BORDER, 592, 70, 20, 20,								  		
		control->mainWindow, (HMENU)WX_FACER,
		globalHInstance, NULL);	

	control->dressL = CreateWindowExA(0, WC_BUTTON, "<", WS_CHILD|
		WS_VISIBLE|WS_BORDER, 435, 120, 20, 20,								  		
		control->mainWindow, (HMENU)WX_DRESSL,
		globalHInstance, NULL);
		
	control->dressR = CreateWindowExA(0, WC_BUTTON, ">", WS_CHILD|
		WS_VISIBLE|WS_BORDER, 592, 120, 20, 20,								  		
		control->mainWindow, (HMENU)WX_DRESSR,
		globalHInstance, NULL);	

	control->bodyL = CreateWindowExA(0, WC_BUTTON, "<", WS_CHILD|
		WS_VISIBLE|WS_BORDER, 435, 150, 20, 20,								  		
		control->mainWindow, (HMENU)WX_BODYL,
		globalHInstance, NULL);
		
	control->bodyR = CreateWindowExA(0, WC_BUTTON, ">", WS_CHILD|
		WS_VISIBLE|WS_BORDER, 592, 150, 20, 20,								  		
		control->mainWindow, (HMENU)WX_BODYR,
		globalHInstance, NULL);		
	
	control->saveChar = CreateWindowExA(0, WC_BUTTON, "Save", WS_CHILD|
		WS_VISIBLE|WS_BORDER, 275, 425, 50, 35,								  		
		control->mainWindow, (HMENU)WX_SAVE,
		globalHInstance, NULL);
	
	control->exportTown = CreateWindowExA(0, WC_BUTTON, "Export", WS_CHILD|
		WS_VISIBLE|WS_BORDER, 330, 425, 50, 35,								  		
		control->mainWindow, (HMENU)WX_EXPORT,
		globalHInstance, NULL);	
		
	control->genderCheck = CreateWindowExA(0, WC_BUTTON, "Gender", WS_CHILD|
		WS_VISIBLE|WS_BORDER, 320, 120, 100, 25,								  		
		control->mainWindow, (HMENU)WX_GENDER,
		globalHInstance, NULL);		
	
	int c;	
	control->male.data = stbi_load("data/male.tga", &control->male.w, &control->male.h, &c, 0); 	
	assert(control->male.data);
	
	control->female.data = stbi_load("data/female.tga", &control->female.w, &control->female.h, &c, 0); 	
	assert(control->female.data);
		
	Redraw(control);
	
	return result;
}

int CALLBACK WinMain(HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     LPSTR     lpCmdLine,
		     int       nCmdShow)
{
	globalHInstance = hInstance;
	
	INITCOMMONCONTROLSEX commCtrl;
	commCtrl.dwSize = sizeof(INITCOMMONCONTROLSEX);
	commCtrl.dwICC = ICC_TAB_CLASSES|ICC_STANDARD_CLASSES|ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&commCtrl);	
	
	InitBMPBuffer(128, 256);
	
	WNDCLASSEX wClass = {};	
	wClass.cbSize = sizeof(WNDCLASSEX);
	wClass.style = CS_HREDRAW|CS_VREDRAW;
	wClass.lpfnWndProc = WindowProc;		
	wClass.lpszClassName = "MainWindow";
	wClass.hInstance = globalHInstance;
	wClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	
	if(!RegisterClassEx(&wClass)) {
		printf("RegisterClassEx Failed\n");
		PostQuitMessage(0);
	}	
	struct win32_gui_control control = {};
	control.mainWindow = CreateWindowExA(0, wClass.lpszClassName, "photo", 
		(WS_OVERLAPPEDWINDOW^WS_THICKFRAME)|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 
		winX, winY, 0, 0, hInstance, 0);	
	
	if(!win32_InitGui(&control)) {
		printf("GUI creation failed\n");
		PostQuitMessage(0);
	} else running = true;	
	
	while(running) {
		MSG msg;
		if(GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);	
		}	
		switch(event) {
		case event_bodyL: {
			ChangeSelection(&control.currentChar.body, BODY_MAX, false);
			Redraw(&control);
			break;	
		} case event_bodyR: {			
			ChangeSelection(&control.currentChar.body, BODY_MAX, true);
			Redraw(&control);
			break;
		} case event_hairL: {
			ChangeSelection(&control.currentChar.hair, HAIR_MAX, false);
			Redraw(&control);
			break;	
		} case event_hairR: {			
			ChangeSelection(&control.currentChar.hair, HAIR_MAX, true);
			Redraw(&control);
			break;
		} case event_faceL: {
			ChangeSelection(&control.currentChar.face, FACE_MAX, false);
			Redraw(&control);
			break;
		} case event_faceR: {			
			ChangeSelection(&control.currentChar.face, FACE_MAX, true);
			Redraw(&control);
			break;
		} case event_dressL: {
			ChangeSelection(&control.currentChar.dress, DRESS_MAX, false);
			Redraw(&control);
			break;	
		} case event_dressR: {			
			ChangeSelection(&control.currentChar.dress, DRESS_MAX, true);
			Redraw(&control);
			break;
		} case event_gender: {
			if(!control.currentChar.gender) {
				control.currentChar.gender = true;
			} else {
				control.currentChar.gender = false;
			}			
			Redraw(&control);
			break;
		} case event_save: {
			SaveChar(&control);
			break;
		} case event_export: {
			ExportFile(&control);
			break;
		}
		default: break;	
		}	
		event = event_null;
		HDC dc = GetDC(control.mainWindow);
		StretchDIBits(dc, 460, 35, 128, 256, 
			0, 0, 128, 256,
			bmpBuffer.data, &bmpBuffer.info,
			DIB_RGB_COLORS, SRCCOPY);	
		ReleaseDC(control.mainWindow, dc);	
	}	
	return 0;
}