/*
 *  Menu_SpeedTest.cpp
 *  OpenLieroX
 *
 *  Created by Karel Petranek on 29.1.09
 *	code under LGPL
 *
 */

#include "DeprecatedGUI/Menu.h"
#include "AuxLib.h"
#include "UploadSpeedTest.h"
#include "DeprecatedGUI/CGuiLayout.h"
#include "DeprecatedGUI/CButton.h"
#include "DeprecatedGUI/CProgressbar.h"
#include "DeprecatedGUI/CLabel.h"
#include "DeprecatedGUI/Graphics.h"
#include "Debug.h"
#include "LieroX.h"

namespace DeprecatedGUI  {

UploadSpeedTest *tSpeedTest = NULL;
CGuiLayout cSpeedTest;
static const int width = 400;
static const int height = 110;
bool bCancelled = false;

enum  {
	ms_Cancel,
	ms_Progress,
	ms_Label,
	ms_Ok,
	ms_Reset
};

void Menu_SpeedTest_OnFinished(UploadSpeedTest::TestData);

#define TESTING_STRING "Testing your connection speed. Please wait..."

void Menu_SpeedTest_Initialize()
{
	tSpeedTest = new UploadSpeedTest();
	tSpeedTest->onFinished.handler() = getEventHandler(Menu_SpeedTest_OnFinished);
	cSpeedTest.Initialize();
	bCancelled = false;

	// Window position
	int x = (VideoPostProcessor::get()->screenWidth() - width) / 2;
	int y = (VideoPostProcessor::get()->screenHeight() - height) / 2;

	cSpeedTest.Add(new CButton(BUT_CANCEL, tMenu->bmpButtons), ms_Cancel, x + (width - 80)/2, y + height - 22, 80, 20);
	cSpeedTest.Add(new CButton(BUT_OK, tMenu->bmpButtons), ms_Ok, x + width/2 + 15, y + height - 22, 40, 20);
	cSpeedTest.Add(new CButton(BUT_RESET, tMenu->bmpButtons), ms_Reset, x + width/2 - 60, y + height - 22, 60, 20);
	cSpeedTest.Add(new CProgressBar(gfxGUI.bmpSpeedTestProgress, 0, 0, false, 2), ms_Progress, x + 20, y + 20 + (height - 20) / 2, width - 40, 20);
	cSpeedTest.Add(new CLabel(TESTING_STRING, tLX->clNormalLabel, true), ms_Label, x + width /2, y + 30, width, 20);
	cSpeedTest.Add(new CLabel("Connection Test", tLX->clNormalLabel, true), -1, x + width /2, y + 4, width, 20);

	// Hide the OK and Reset buttons
	cSpeedTest.getWidget(ms_Ok)->setEnabled(false);
	cSpeedTest.getWidget(ms_Reset)->setEnabled(false);

	cSpeedTest.SetGlobalProperty(PRP_REDRAWMENU, 0);

	// Run the test
	tSpeedTest->startTest();
}

////////////////////
// Called when the test finishes
void Menu_SpeedTest_OnFinished(UploadSpeedTest::TestData d)
{
	if (d.succeeded)
		notes << "Speed test successfully finished, the upload rate is " << (int)d.rate << " B/s" << endl;
	else
		errors << "Speed test failed: " << d.test->getError().sErrorMsg << endl;
}

bool Menu_SpeedTest_Frame()
{
	if (!tSpeedTest)
		return true;

	SDL_Surface *bmpDest = VideoPostProcessor::videoSurface();

	// Error?
	if (tSpeedTest->hasFinished())  {
		CLabel *lbl = (CLabel *)cSpeedTest.getWidget(ms_Label);
		if (tSpeedTest->hasErrorOccured())  {
			lbl->setText("Connection test failed, please try again later.");
			lbl->ChangeColour(tLX->clError);
		} else {
			int speed = (int)tSpeedTest->getUploadRate();
			std::string human_speed = itoa(speed) + " B/s";
			if (speed >= 1024 * 1024 * 1024)
				human_speed = itoa(speed / (1024 * 1024 * 1024)) + " GB/s";
			else if (speed >= 1024 * 1024)
				human_speed = itoa(speed / (1024 * 1024)) + " MB/s";
			else if (speed >= 1024)
				human_speed = itoa(speed / 1024) + " kB/s";

			lbl->setText("Test completed! Your upload speed is " + human_speed);
			lbl->ChangeColour(tLX->clNormalLabel);
		}

		// Hide the cancel button and show the OK button
		cSpeedTest.getWidget(ms_Cancel)->setEnabled(false);
		cSpeedTest.getWidget(ms_Ok)->setEnabled(true);
		cSpeedTest.getWidget(ms_Reset)->setEnabled(true);
	}

	// Update the progress bar
	CProgressBar *bar = (CProgressBar *)cSpeedTest.getWidget(ms_Progress);
	bar->SetPosition(tSpeedTest->getProgress());

	// Events
	gui_event_t *ev = cSpeedTest.Process();
	if (ev)  {
		switch (ev->iControlID)  {
		case ms_Cancel:
			if (ev->iEventMsg == BTN_CLICKED)  {
				tSpeedTest->cancelTest();
				bCancelled = true;
				return true;
			}
		break;

		case ms_Ok:
			if (ev->iEventMsg == BTN_CLICKED)  {
				return true;
			}
		break;

		case ms_Reset:
			if (ev->iEventMsg == BTN_CLICKED)  {
				tSpeedTest->startTest();

				// Hide the OK and Reset buttons and show the Cancel button
				cSpeedTest.getWidget(ms_Cancel)->setEnabled(true);
				cSpeedTest.getWidget(ms_Ok)->setEnabled(false);
				cSpeedTest.getWidget(ms_Reset)->setEnabled(false);

				CLabel *lbl = (CLabel *)cSpeedTest.getWidget(ms_Label);
				lbl->setText(TESTING_STRING);
				lbl->ChangeColour(tLX->clNormalLabel);

				return false;
			}
		}
	}

	// Draw
	int x = (VideoPostProcessor::get()->screenWidth() - width) / 2;
	int y = (VideoPostProcessor::get()->screenHeight() - height) / 2;

	DrawRectFill(bmpDest, x + 2, y + 2, x + width - 2, y + height - 2, tLX->clDialogBackground);
	DrawRectFill(bmpDest, x + 2, y + 2, x + width - 2, y + 22, tLX->clDialogCaption);
	Menu_DrawBox(bmpDest, x, y, x + width - 1, y + height - 1);
	cSpeedTest.Draw(bmpDest);

	return false;
}

//////////////////////
// Gets the measured speed, returns tLXOptions->iMaxUploadBandwidth on error
// Must be called before Menu_SpeedTest_Shutdown!
float Menu_SpeedTest_GetSpeed()
{
	if (!tSpeedTest)
		return (float)tLXOptions->iMaxUploadBandwidth;

	if (tSpeedTest->hasErrorOccured())
		return (float)tLXOptions->iMaxUploadBandwidth;

	if (!tSpeedTest->hasFinished())
		return (float)tLXOptions->iMaxUploadBandwidth;

	if (bCancelled)
		return (float)tLXOptions->iMaxUploadBandwidth;

	return tSpeedTest->getUploadRate();
}


void Menu_SpeedTest_Shutdown()
{
	if (tSpeedTest)
		delete tSpeedTest;
	tSpeedTest = NULL;

	bCancelled = false;

	cSpeedTest.Shutdown();
}

} // namespace DeprecatedGUI
