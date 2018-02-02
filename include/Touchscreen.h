/////////////////////////////////////////
//
//   OpenLieroX
//
//   Auxiliary Software class library
//
//   based on the work of JasonB
//   enhanced by Dark Charlie and Albert Zeyer
//
//   code under LGPL
//
/////////////////////////////////////////

#ifndef __TOUCHSCREEN_H__
#define __TOUCHSCREEN_H__

#include "InputEvents.h"

void ProcessTouchscreenEvents();
bool GetTouchscreenControlsShown();
void SetTouchscreenControlsShown(bool shown);
void SetupTouchscreenControls();
bool GetTouchscreenTextInputShown();
void ShowTouchscreenTextInput(const std::string & initialText = "");
bool ProcessTouchscreenTextInput(std::string * output);
void SetTouchscreenTextInputHintMessage(const std::string & message);

#endif
