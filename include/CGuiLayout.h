/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// GUI Layout class
// Created 30/5/02
// Jason Boettcher


#ifndef __CGUILAYOUT_H__
#define __CGUILAYOUT_H__


// layout event structure
typedef struct {
	int		iControlID;
	int		iEventMsg;

	CWidget	*cWidget;
} gui_event_t;


// Widget types
enum {
	wid_Button=0,
	wid_Label,
	wid_Listview,
	wid_Scrollbar,
	wid_Slider,
	wid_Textbox,
	wid_Titlebutton,
	wid_Checkbox,
	wid_Inputbox
};


class CGuiLayout {
public:
	// Constructor
	CGuiLayout() {
		tEvent = NULL;
		cFocused = NULL;
		cWidgets = NULL;
		iCanFocus = true;
		//Initialize();
	}

	// Destructor
	~CGuiLayout() {
		//Shutdown();
	}


private:
	// Attributes

	CWidget		*cWidgets;
	gui_event_t	*tEvent;
	CWidget		*cFocused;

	// Mouse button repeats
	int			nMouseButtons;
	float		fMouseNext[3];

	// Can we set focus to another widget?
	int			iCanFocus;


public:
	// Methods

	void		Initialize(void);

	void		Add(CWidget *widget, int id, int x, int y, int w, int h);
	CWidget		*getWidget(int id);
    void        removeWidget(int id);

	gui_event_t	*Process(void);
	void		Draw(SDL_Surface *bmpDest);

	void		Shutdown(void);


	// Messaging
	int			SendMessage(int iControl, int iMsg, DWORD Param1, DWORD Param2);

};




#endif  //  __CGUILAYOUT_H__
