/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////



#ifndef __CWORMHUMAN_H__
#define __CWORMHUMAN_H__

#include "CWorm.h"


class CWormHumanInputHandler : public CWormInputHandler {
public:
	CWormHumanInputHandler(CWorm* w);
	virtual ~CWormHumanInputHandler();
	virtual std::string name() { return "Human input handler"; }

	virtual void initWeaponSelection();
	virtual	void doWeaponSelectionFrame(SDL_Surface * bmpDest, CViewport *v);
	
	// simulation
	virtual void startGame();
	virtual void getInput(); 
    virtual void clearInput();
	
	
protected:
	
	// Input
	CInput		cUp, cDown, cLeft, cRight,
	cShoot, cJump, cSelWeapon, cInpRope,
	cStrafe, cDig, cWeapons[5];
	//bool		bUsesMouse;
	
public:

	
	CInput &	getInputUp()					{ return cUp; };
	CInput &	getInputDown()					{ return cDown; };
	CInput &	getInputLeft()					{ return cLeft; };
	CInput &	getInputRight()					{ return cRight; };
	CInput &	getInputShoot()					{ return cShoot; };
	CInput &	getInputJump()					{ return cJump; };
	CInput &	getInputWeapon()				{ return cSelWeapon; };
	CInput &	getInputRope()					{ return cInpRope; };
	CInput &	getInputStrafe()				{ return cStrafe; };
	CInput &	getInputDig()					{ return cDig; };
	
	bool	canType();
	
	
	// Input
	void		setupInputs(const PlyControls& Inputs);
	void		initInputSystem();
	void		stopInputSystem();
	
};

#endif  //  __CWORMHUMAN_H__
