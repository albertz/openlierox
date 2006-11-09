/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Checkbox
// Created 30/7/02
// Jason Boettcher


#ifndef __CCHECKBOX_H__
#define __CCHECKBOX_H__


// Checkbox events
enum {
	CHK_NONE=-1,
	CHK_CHANGED
};

// Checkbox messages
enum {
	CKM_SETCHECK=0,
	CKM_GETCHECK
};


class CCheckbox : public CWidget {
public:
	// Constructor
	CCheckbox(int val) {
		iValue = val;
        bmpImage = NULL;
		iType = wid_Checkbox;
	}


private:
	// Attributes

	int			iValue;
	SDL_Surface	*bmpImage;


public:
	// Methods

	void	Create(void);
	void	Destroy(void) { }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)			{ return CHK_NONE; }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ iValue = !iValue;		return CHK_CHANGED; }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return CHK_NONE; }
	int		MouseWheelDown(mouse_t *tMouse)		{ return CHK_NONE; }
	int		MouseWheelUp(mouse_t *tMouse)		{ return CHK_NONE; }
	int		KeyDown(int c)						{ return CHK_NONE; }
	int		KeyUp(int c)						{ return CHK_NONE; }
	

	// Process a message sent
	int		SendMessage(int iMsg, DWORD Param1, DWORD Param2) {

				switch(iMsg) {
					case CKM_SETCHECK:
						iValue = Param1;
						return 0;
					case CKM_GETCHECK:
						return iValue;
				}

				return 0;
			}
		

	// Draw the title button
	void	Draw(SDL_Surface *bmpDest);

	void	LoadStyle(void) {}


	int		getValue(void)						{ return iValue; }

};



#endif  //  __CCHECKBOX_H__
