/////////////////////////////////////////
//
//   OpenLieroX
//
//   Touchscreen processing
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////


#include <assert.h>


#include "LieroX.h"

#include "Touchscreen.h"
#include "AuxLib.h"
#include "Error.h"
#include "ConfigHandler.h"
#include "InputEvents.h"
#include "StringUtils.h"
#include "CInput.h"
#include "Debug.h"
#include "LieroX.h"
#include "CWorm.h"
#include "Mutex.h"
#include "Condition.h"
#include "ThreadPool.h"

#ifdef __ANDROID__

#include <SDL/SDL_android.h>
#include <SDL/SDL_screenkeyboard.h>


enum {
	LEFT_JOYSTICK_X = 0,
	LEFT_JOYSTICK_Y = 1,
	TOUCHJOY_DEAD_ZONE = 65536 / 20,
};

static const float AIM_SPEED = 0.03f;
static const float CARVE_TAP_DELAY = 0.3f;

static SDL_Joystick* touchJoy = NULL;
static AbsTime tapTime;
static int aimTime;
static int oldPosX, oldPosY;
static bool oldPressed = false, oldCarve = false;
static bool oldLeft = false, oldRight = false, oldUp = false, oldDown = false;
static bool weaponSelectionHoriz = false, weaponSelectionVert = false;

static bool controlsInitialized = false;
static char sScreenKeyboardBuf[256] = "";
static Mutex screenKeyboardMutex;
static Condition screenKeyboardCond;


void ProcessTouchscreenEvents()
{
	if (!touchJoy) {
		SDL_InitSubSystem(SDL_INIT_JOYSTICK);
		touchJoy = SDL_JoystickOpen(0);
	}

	int posX = SDL_JoystickGetAxis(touchJoy, LEFT_JOYSTICK_X);
	int posY = SDL_JoystickGetAxis(touchJoy, LEFT_JOYSTICK_Y);
	bool pressed = (posX != 0 || posY != 0);

	if (oldCarve) {
		oldCarve = false;
		KeyboardEvent k;
		k.down = false;
		k.ch = 0;
		k.sym = SDLK_a;
		HandleCInputs_KeyEvent(k);
	}

	if (!pressed) {
		if (pressed != oldPressed) {
			oldPressed = pressed;
			tapTime = tLX->currentTime;
			oldUp = false;
			oldDown = false;
			oldLeft = false;
			oldRight = false;
			weaponSelectionHoriz = false;
			weaponSelectionVert = false;
			KeyboardEvent k;
			k.down = false;
			k.ch = 0;
			k.sym = SDLK_UP;
			HandleCInputs_KeyEvent(k);
			k.sym = SDLK_DOWN;
			HandleCInputs_KeyEvent(k);
			k.sym = SDLK_LEFT;
			HandleCInputs_KeyEvent(k);
			k.sym = SDLK_RIGHT;
			HandleCInputs_KeyEvent(k);
		}
		return;
	}

	CWorm* worm = cClient->getWorm(0);

	if (pressed) {
		if (pressed != oldPressed) {
			oldPressed = pressed;
			oldPosX = posX;
			oldPosY = posY;
			aimTime = 0;
			if (tapTime + CARVE_TAP_DELAY > tLX->currentTime) {
				oldCarve = true;
				KeyboardEvent k;
				k.down = true;
				k.ch = 0;
				k.sym = SDLK_a;
				HandleCInputs_KeyEvent(k);
			}
		}
	}

	if (posX - oldPosX < -TOUCHJOY_DEAD_ZONE) {
		if (!oldLeft) {
			oldLeft = true;
			weaponSelectionHoriz = true;
			KeyboardEvent k;
			k.down = true;
			k.ch = 0;
			k.sym = SDLK_LEFT;
			HandleCInputs_KeyEvent(k);
		}
		if (posX - oldPosX < -TOUCHJOY_DEAD_ZONE * 3) {
			oldPosX = posX + TOUCHJOY_DEAD_ZONE * 3;
			if (worm && !worm->getWeaponsReady()) {
				// Weapon selection menu, press up/down slowly and in steps
				oldPosX = posX;
			}
		}
	} else {
		if (oldLeft) {
			oldLeft = false;
			KeyboardEvent k;
			k.down = false;
			k.ch = 0;
			k.sym = SDLK_LEFT;
			HandleCInputs_KeyEvent(k);
		}
	}

	if (posX - oldPosX > TOUCHJOY_DEAD_ZONE) {
		if (!oldRight) {
			oldRight = true;
			weaponSelectionHoriz = true;
			KeyboardEvent k;
			k.down = true;
			k.ch = 0;
			k.sym = SDLK_RIGHT;
			HandleCInputs_KeyEvent(k);
		}
		if (posX - oldPosX > TOUCHJOY_DEAD_ZONE * 3) {
			oldPosX = posX - TOUCHJOY_DEAD_ZONE * 3;
			if (worm && !worm->getWeaponsReady()) {
				// Weapon selection menu, press up/down slowly and in steps
				oldPosX = posX;
			}
		}
	} else {
		if (oldRight) {
			oldRight = false;
			KeyboardEvent k;
			k.down = false;
			k.ch = 0;
			k.sym = SDLK_RIGHT;
			HandleCInputs_KeyEvent(k);
		}
	}

	if (worm && !worm->getWeaponsReady()) {
		// Weapon selection menu, press up/down slowly and in steps
		if (weaponSelectionHoriz) {
			oldPosY = posY; // Prevent moving cursor left/right
		}
		if (weaponSelectionVert) {
			oldPosX = posX; // Prevent moving cursor up/down
		}
		if (abs(posY - oldPosY) > TOUCHJOY_DEAD_ZONE * 2.5) {
			aimTime += (posY - oldPosY) > 0 ? 1 : -1;
			oldPosY = posY;
			weaponSelectionVert = true;
		}
	} else {
		aimTime += (posY - oldPosY) * AIM_SPEED;
		oldPosY = posY;
	}

	if (aimTime > 0) {
		if (!oldDown) {
			oldDown = true;
			KeyboardEvent k;
			k.down = true;
			k.ch = 0;
			k.sym = SDLK_DOWN;
			HandleCInputs_KeyEvent(k);
		}
		aimTime -= tLX->fDeltaTime.milliseconds();
		if (aimTime < 0) {
			aimTime = 0;
		}
	}

	if (aimTime <= 0) {
		if (oldDown) {
			oldDown = false;
			KeyboardEvent k;
			k.down = false;
			k.ch = 0;
			k.sym = SDLK_DOWN;
			HandleCInputs_KeyEvent(k);
		}
	}

	if (aimTime < 0) {
		if (!oldUp) {
			oldUp = true;
			KeyboardEvent k;
			k.down = true;
			k.ch = 0;
			k.sym = SDLK_UP;
			HandleCInputs_KeyEvent(k);
		}
		aimTime += tLX->fDeltaTime.milliseconds();
		if (aimTime > 0) {
			aimTime = 0;
		}
	}

	if (aimTime >= 0) {
		if (oldUp) {
			oldUp = false;
			KeyboardEvent k;
			k.down = false;
			k.ch = 0;
			k.sym = SDLK_UP;
			HandleCInputs_KeyEvent(k);
		}
	}

	//errors << "posX " << posX << " " << oldPosX << " posY " << posY << " " << oldPosY << " aimTime " << aimTime << " down " << oldDown << " up " << oldUp << endl;
}

bool GetTouchscreenControlsShown()
{
	return SDL_ANDROID_GetScreenKeyboardShown();
}

void SetTouchscreenControlsShown(bool shown)
{
	SDL_ANDROID_SetScreenKeyboardShown(shown);
	if (!shown) {
		// De-press all touchscreen keys
		oldPressed = false;
		oldUp = false;
		oldDown = false;
		oldLeft = false;
		oldRight = false;
		weaponSelectionHoriz = false;
		weaponSelectionVert = false;
		KeyboardEvent k;
		k.down = false;
		k.ch = 0;
		k.sym = SDLK_UP;
		HandleCInputs_KeyEvent(k);
		k.sym = SDLK_DOWN;
		HandleCInputs_KeyEvent(k);
		k.sym = SDLK_LEFT;
		HandleCInputs_KeyEvent(k);
		k.sym = SDLK_RIGHT;
		HandleCInputs_KeyEvent(k);
		k.sym = SDLK_LALT;
		HandleCInputs_KeyEvent(k);
		k.sym = SDLK_x;
		HandleCInputs_KeyEvent(k);
		k.sym = SDLK_a;
		HandleCInputs_KeyEvent(k);
		k.sym = SDLK_LCTRL;
		HandleCInputs_KeyEvent(k);
		k.sym = SDLK_LSHIFT;
		HandleCInputs_KeyEvent(k);
		k.sym = SDLK_i;
		HandleCInputs_KeyEvent(k);
	}
}

void SetupTouchscreenControls()
{
	SDL_Rect pos, pos2;

	if (controlsInitialized) {
		return;
	}
	controlsInitialized = true;

	pos.w = SDL_ListModes(NULL, 0)[0]->w / 2;
	pos.h = SDL_ListModes(NULL, 0)[0]->h / 3 * 2;
	pos.x = 0;
	pos.y = SDL_ListModes(NULL, 0)[0]->h - pos.h;
	SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, &pos);
	pos.w = 0;
	pos.h = 0;
	pos.w = pos.h = SDL_ListModes(NULL, 0)[0]->h / 8;
	pos.x = SDL_ListModes(NULL, 0)[0]->w - pos.w;
	pos.y = 0;
	SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_4, &pos);
	// Rearrange fire and rope buttons
	SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &pos);
	SDL_ANDROID_GetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, &pos2);
	SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &pos2);
	SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, &pos);
}

bool GetTouchscreenTextInputShown()
{
	return SDL_IsScreenKeyboardShown(NULL);
}

void ShowTouchscreenTextInput(const std::string & initialText)
{
	if (GetTouchscreenTextInputShown())
		return;

	Mutex::ScopedLock lock( screenKeyboardMutex );

	strncpy(sScreenKeyboardBuf, initialText.c_str(), sizeof(sScreenKeyboardBuf));
	sScreenKeyboardBuf[sizeof(sScreenKeyboardBuf) - 1] = 0;

	struct MainThreadAction: public Action
	{
		int handle()
		{
			Mutex::ScopedLock lock( screenKeyboardMutex );
			SDL_ANDROID_GetScreenKeyboardTextInputAsync(sScreenKeyboardBuf, sizeof(sScreenKeyboardBuf));
			screenKeyboardCond.signal();
			return 0;
		} 
	};
	doActionInMainThread( new MainThreadAction );

	screenKeyboardCond.wait( screenKeyboardMutex );
}

bool ProcessTouchscreenTextInput(std::string * output)
{
	if (!GetTouchscreenTextInputShown())
		return false;

	Mutex::ScopedLock lock( screenKeyboardMutex );

	bool result = false;

	struct MainThreadAction: public Action
	{
		std::string * output;
		bool * result;
		MainThreadAction(std::string * output, bool * result): output(output), result(result) {}
		int handle()
		{
			Mutex::ScopedLock lock( screenKeyboardMutex );
			if (SDL_ANDROID_GetScreenKeyboardTextInputAsync(sScreenKeyboardBuf, sizeof(sScreenKeyboardBuf)) == SDL_ANDROID_TEXTINPUT_ASYNC_FINISHED) {
				output->assign(sScreenKeyboardBuf);
				*result = true;
			}
			screenKeyboardCond.signal();
			return 0;
		}
	};
	doActionInMainThread( new MainThreadAction(output, &result) );

	screenKeyboardCond.wait( screenKeyboardMutex );

	return result;
}

void SetTouchscreenTextInputHintMessage(const std::string & message)
{
	Mutex::ScopedLock lock( screenKeyboardMutex );

	struct MainThreadAction: public Action
	{
		const std::string & message;
		MainThreadAction(const std::string & message): message(message) {}
		int handle()
		{
			Mutex::ScopedLock lock( screenKeyboardMutex );
			SDL_ANDROID_SetScreenKeyboardHintMesage(message.c_str());
			screenKeyboardCond.signal();
			return 0;
		} 
	};
	doActionInMainThread( new MainThreadAction(message) );

	screenKeyboardCond.wait( screenKeyboardMutex );
}

#else // __ANDROID__

void ProcessTouchscreenEvents()
{
}

bool GetTouchscreenControlsShown()
{
	return false;
}

void SetTouchscreenControlsShown(bool shown)
{
}

void SetupTouchscreenControls()
{
}

bool GetTouchscreenTextInputShown()
{
	return false;
}

void ShowTouchscreenTextInput(const std::string & initialText)
{
}

bool ProcessTouchscreenTextInput(std::string * output)
{
	return false;
}

void SetTouchscreenTextInputHintMessage(const std::string & message)
{
}

#endif // __ANDROID__
