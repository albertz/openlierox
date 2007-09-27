/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Cursor source file
// Created 18/6/07
// Dark Charlie

#include "FindFile.h"
#include "InputEvents.h"
#include "GfxPrimitives.h"
#include "ConfigHandler.h"
#include "Cursor.h"
#include "types.h"
#include "LieroX.h"


//
// Game cursor handling
//


CCursor *tCurrentCursor = NULL;
CCursor *tCursors[CURSOR_COUNT] = {NULL, NULL, NULL, NULL, NULL}; // TODO: any cursor_count independent way?
float fCursorFrameTime = 0.2f;

int iMaxCursorWidth = 0;
int iMaxCursorHeight = 0;

//////////////////
// Initialize cursors
bool InitializeCursors()
{
	tCurrentCursor = NULL;
	
	// Load the cursors
	tCursors[CURSOR_ARROW] = new CCursor("data/frontend/mouse.png",CUR_ARROW);
	tCursors[CURSOR_HAND] = new CCursor("data/frontend/mouse_hand.png",CUR_ARROW);
	tCursors[CURSOR_TEXT] = new CCursor("data/frontend/mouse_text.png",CUR_TEXT);
	tCursors[CURSOR_RESIZE] = new CCursor("data/frontend/mouse_resize.png",CUR_SPLITTER);
	tCursors[CURSOR_AIM] = new CCursor("data/frontend/mouse_aim.png",CUR_AIM); // TODO: draw new gfx
	if( tCursors[CURSOR_AIM]->GetHeight() == 0 )
		tCursors[CURSOR_AIM] = new CCursor("data/frontend/mouse.png",CUR_ARROW);

	// Load the frame time from external config
	ReadFloat("data/frontend/frontend.cfg","Cursors","FrameTime",&fCursorFrameTime,0.2f);

	// Check that all were loaded correctly and calculate maximal width/height
	bool result = true;
	for (byte i=0;i<CURSOR_COUNT;i++)
		if (tCursors[i])  {
			// Max width/height
			if (tCursors[i]->GetWidth() > iMaxCursorWidth)
				iMaxCursorWidth = tCursors[i]->GetWidth();

			if (tCursors[i]->GetHeight() > iMaxCursorHeight)
				iMaxCursorHeight = tCursors[i]->GetHeight();
		} else {
			result = false;
		}


	return result;
}

//////////////////
// Shutdown the cursors
void ShutdownCursors()
{
	// Free all the cursor structures
	for (byte i=0; i<CURSOR_COUNT; i++)
		if (tCursors[i])  {
			delete tCursors[i];
			tCursors[i] = NULL;
		}
}

////////////////
// Set the current cursor
void SetGameCursor(int c)  {
	if (c < 0 || c >= CURSOR_COUNT)
		tCurrentCursor = NULL;
	else
		tCurrentCursor = tCursors[c];
}

/////////////////
// Draw game cursor
void DrawCursor(SDL_Surface *dst) {
	if (tCurrentCursor)
		tCurrentCursor->Draw(dst);
}

///////////////////
// Get height of a cursor
int GetCursorHeight(int c)  {
	if (c < 0 || c >= CURSOR_COUNT)
		return 0;
	else
		return tCursors[c]->GetHeight();
}


///////////////////
// Get width of a cursor
int GetCursorWidth(int c)  {
	if (c < 0 || c >= CURSOR_COUNT)
		return 0;
	else
		return tCursors[c]->GetWidth();
}



//
// Cursor class
//

/////////////////
// Constructor
CCursor::CCursor(const std::string& filename, int type)
{
	// Defaults
	iType = type;
	fAnimationSwapTime = tLX->fCurTime;
	bAnimated = false;
	iFrameWidth = 0;
	iNumFrames = 0;
	iFrame = 0;
	cUp = NULL;
	cDown = NULL;


	// Load the cursor
	bmpCursor = LoadImage(filename,true);
	if (bmpCursor)  {
		if (bmpCursor->w >= 2*bmpCursor->h && (bmpCursor->w % bmpCursor->h) == 0)  {  // The file contains more frames
			bAnimated = true;
			iFrameWidth = bmpCursor->h;
			iNumFrames = bmpCursor->w / iFrameWidth;
			iFrame = 0;
			
		} else {  // Only one frame
			bAnimated = false;
			iFrameWidth = bmpCursor->w;
			iNumFrames = 1;
			iFrame = 0;
		}

		// Set the color key
		SetColorKey(bmpCursor);
	}

	// Get the filename for the "down" cursor
	std::string down_filename = filename.substr(0,filename.rfind('.')) + "_down.png";

	// Get the filename for the "up" cursor
	std::string up_filename = filename.substr(0,filename.rfind('.')) + "_up.png";

	// Load up and down states if they're present
	if (IsFileAvailable(down_filename))
		cDown = new CCursor(down_filename,iType);
	if (IsFileAvailable(up_filename))
		cUp = new CCursor(up_filename,iType);
}

////////////////
// Destructor
CCursor::~CCursor()
{
	// Freed by the cache
	bmpCursor = NULL;

	// Free the special cases
	if (cDown)
		delete cDown;
	if (cUp)
		delete cUp;
}

//////////////////
// Draw the cursor
void CCursor::Draw(SDL_Surface *dst)
{
	// Check
	if (!dst || !bmpCursor)
		return;

	mouse_t *Mouse = GetMouse();

	// Special states first
	if (Mouse->FirstDown)  // Mouse down
		if (cDown)  {
			cDown->Draw(dst);
			return;
		}
	else if (Mouse->Up) // Mouse up
		if (cUp)  {
			cUp->Draw(dst);
			return;
		}

	// Image position
	int X = Mouse->X;
	int Y = Mouse->Y;

	// Change the position depending on the type
	switch (iType)  {
	case CUR_ARROW:
	case CUR_TEXT:
		// No position change needed
		break;
	case CUR_SPLITTER:
		X -= iFrameWidth / 2;
		break;
	case CUR_AIM:
		X -= iFrameWidth / 2;
		Y -= bmpCursor->h / 2;
		break;
	default:
		printf("Warning: CCursor::Draw - unknown type");
		break;
	};


	// Process animating
	if ((tLX->fCurTime - fAnimationSwapTime) >= fCursorFrameTime)  {
		iFrame++;
		if (iFrame >= iNumFrames)
			iFrame = 0;
	}

	// Draw the cursor
	DrawImageAdv(dst,bmpCursor,iFrame*iFrameWidth,0,X,Y,iFrameWidth,bmpCursor->h);
}
