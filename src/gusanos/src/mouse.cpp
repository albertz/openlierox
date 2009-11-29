#ifndef DEDSERV
#include "mouse.h"
#include "gfx.h"

MouseHandler mouseHandler;

namespace
{
	bool buttonStates[3] = {false, false, false};
	int  posX = 0;
	int  posY = 0;
	int  posZ = 0;
	volatile int toggles = 0;
	
	void mouseCallback(int flags)
	{
		toggles |= flags;
	}
	END_OF_FUNCTION(mouseCallback);
}

void MouseHandler::init()
{
	install_mouse();
	LOCK_VARIABLE(toggles);
	LOCK_FUNCTION(mouseCallback);
	mouse_callback = mouseCallback;
	posX = mouse_x;
	posY = mouse_y;
	posZ = mouse_z;
}

void MouseHandler::poll()
{
	poll_mouse();
	
	/*
	for(int i = 0; i < 3; ++i)
	{
		bool state = (mouse_b & (1 << i));
		if(state != buttonStates[i])
		{
			buttonStates[i] = state;
			if(state)
				buttonDown(i);
			else
				buttonUp(i);
		}
	}
	*/
	static int const stateFlags[3] =
	{
		MOUSE_FLAG_LEFT_DOWN | MOUSE_FLAG_LEFT_UP,
		MOUSE_FLAG_RIGHT_DOWN | MOUSE_FLAG_RIGHT_UP,
		MOUSE_FLAG_MIDDLE_DOWN | MOUSE_FLAG_MIDDLE_UP,
	};
	int localToggles = toggles; toggles = 0; // TODO: Mutex?
	for(int i = 0; i < 3; ++i)
	{
		bool state = (mouse_b & (1 << i));
		if(state != buttonStates[i])
		{
			buttonStates[i] = state;
			if(state)
				buttonDown(i);
			else
				buttonUp(i);
		}
		else if((localToggles & stateFlags[i]) == stateFlags[i])
		{
			if(state)
			{
				buttonUp(i);
				buttonDown(i);
			}
			else
			{
				buttonDown(i);
				buttonUp(i);
			}
		}
	}
	
	int newPosX = mouse_x;
	int newPosY = mouse_y;
	int newPosZ = mouse_z;
	
	newPosX /= gfx.getScalingFactor();
	newPosY /= gfx.getScalingFactor();
	
	if(newPosX != posX
	|| newPosY != posY)
	{
		posX = newPosX;
		posY = newPosY;
		move(posX, posY);
	}
	
	if(newPosZ != posZ)
	{
		scroll(posZ - newPosZ);
		posZ = newPosZ;
	}
}

void MouseHandler::shutDown()
{
	remove_mouse();
}

int MouseHandler::getX()
{
	return posX;
}

int MouseHandler::getY()
{
	return posY;
}

#endif
