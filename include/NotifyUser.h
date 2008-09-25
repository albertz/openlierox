/*
 *  NotifyUser.h
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 25.09.08.
 *  code under LGPL
 *
 */

#ifndef __NOTIFYUSER_H__
#define __NOTIFYUSER_H__

void NotifyUserOnEvent();	// Blink the window titlebar and produce the sound when game starts or going to lobby
void ClearUserNotify();		// stops the blinking (for example on X11, this is not done automatically)

#endif
