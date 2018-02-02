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

static bool controlsInitialized = false;
static char sScreenKeyboardBuf[256] = "";


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
		k.sym = SDLK_z;
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
				k.sym = SDLK_z;
				HandleCInputs_KeyEvent(k);
			}
		}
	}

	if (posX - oldPosX < -TOUCHJOY_DEAD_ZONE) {
		if (!oldLeft) {
			oldLeft = true;
			KeyboardEvent k;
			k.down = true;
			k.ch = 0;
			k.sym = SDLK_LEFT;
			HandleCInputs_KeyEvent(k);
		}
		if (posX - oldPosX < -TOUCHJOY_DEAD_ZONE * 3) {
			oldPosX = posX + TOUCHJOY_DEAD_ZONE * 3;
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
			KeyboardEvent k;
			k.down = true;
			k.ch = 0;
			k.sym = SDLK_RIGHT;
			HandleCInputs_KeyEvent(k);
		}
		if (posX - oldPosX > TOUCHJOY_DEAD_ZONE * 3) {
			oldPosX = posX - TOUCHJOY_DEAD_ZONE * 3;
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

	CWorm* worm = cClient->getWorm(0);
	if (worm && !worm->getWeaponsReady()) {
		// Weapon selection menu, press up/down slowly and in steps
		if (abs(posY - oldPosY) > TOUCHJOY_DEAD_ZONE * 2.5) {
			aimTime += (posY - oldPosY) > 0 ? 1 : -1;
			oldPosY = posY;
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
	strncpy(sScreenKeyboardBuf, initialText.c_str(), sizeof(sScreenKeyboardBuf));
	sScreenKeyboardBuf[sizeof(sScreenKeyboardBuf) - 1] = 0;
	SDL_ANDROID_GetScreenKeyboardTextInputAsync(sScreenKeyboardBuf, sizeof(sScreenKeyboardBuf));
}

bool ProcessTouchscreenTextInput(std::string * output)
{
	if (SDL_ANDROID_GetScreenKeyboardTextInputAsync(sScreenKeyboardBuf, sizeof(sScreenKeyboardBuf)) == SDL_ANDROID_TEXTINPUT_ASYNC_FINISHED) {
		output->assign(sScreenKeyboardBuf);
		return true;
	}
	return false;
}

void SetTouchscreenTextInputHintMessage(const std::string & message)
{
	SDL_ANDROID_SetScreenKeyboardHintMesage(message.c_str());
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
