/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Floating options window file
// Created 24/9/08
// Karel Petranek

#include "Options.h"
#include "CClient.h"

void UpdateGameInputs()
{
	// Setup the controls
	cClient->getWorm(0)->SetupInputs( tLXOptions->sPlayerControls[0] );
	// TODO: setup also more viewports
	if(cClient->getNumWorms() >= 2)
		cClient->getWorm(1)->SetupInputs( tLXOptions->sPlayerControls[1] );
	
    cClient->getViewports()[0].setupInputs( tLXOptions->sPlayerControls[0] );
    cClient->getViewports()[1].setupInputs( tLXOptions->sPlayerControls[1] );

}
