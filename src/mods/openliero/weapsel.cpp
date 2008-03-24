#include "gfx.hpp"
#include "game.hpp"
#include "worm.hpp"
#include "text.hpp"
#include "menu.hpp"
#include "sfx.hpp"
#include <SDL/SDL.h>


void selectWeapons()
{
	int enabledWeaps = 0;
	
	for(int i = 0; i < 40; ++i)
	{
		if(game.settings.weapTable[i] == 0)
			++enabledWeaps;
	}
	
	int curSel[2];
	bool isReady[2];
	Menu menus[2];
		
	for(int i = 0; i < 2; ++i)
	{
		bool weapUsed[256] = {};
		
		Worm& worm = *game.worms[i];
		WormSettings& ws = *worm.settings;
		
		menus[i].items.push_back(MenuItem(1, 1, game.texts.randomize));
		
		for(int j = 0; j < game.settings.selectableWeapons; ++j)
		{
			if(ws.weapons[j] == 0)
			{
				ws.weapons[j] = game.rand(1, 41);
			}
			
			bool enoughWeapons = (enabledWeaps >= game.settings.selectableWeapons);
			
			while(true)
			{
				int w = game.weapOrder[ws.weapons[j]];
				
				if((!enoughWeapons || !weapUsed[w])
				&& game.settings.weapTable[w] <= 0)
					break;
					
				ws.weapons[j] = game.rand(1, 41);
			}
			
			int w = game.weapOrder[ws.weapons[j]];
			
			weapUsed[w] = true;
			
			WormWeapon& ww = worm.weapons[j];
			
			ww.ammo = 0;
			ww.id = w;
			
			menus[i].items.push_back(MenuItem(48, 48, game.weapons[w].name));
		}
		
		menus[i].items.push_back(MenuItem(10, 10, game.texts.done));
		
		worm.currentWeapon = 0;
		
		curSel[i] = 0;
		isReady[i] = (ws.controller != 0); // AIs are ready immediately
	}

	game.processViewports();
	game.drawViewports();
	
	drawRoundedBox(114, 2, 0, 7, gfx.font.getWidth(game.texts.selWeap));
	
	gfx.font.drawText(game.texts.selWeap, 116, 3, 50);
	
	for(int i = 0; i < 2; ++i)
	{
		WormSettings& ws = game.settings.wormSettings[i];
		int selWeapX = ws.selWeapX;
		int width = gfx.font.getWidth(ws.name);
		drawRoundedBox(selWeapX + 29 - width/2, 18, 0, 7, width);
		gfx.font.drawText(ws.name, selWeapX + 31 - width/2, 19, ws.colour + 1);
	}
		
	if(game.settings.levelFile.empty())
	{
		gfx.font.drawText(game.texts.levelRandom, 0, 162, 50);
	}
	else
	{
		gfx.font.drawText((game.texts.levelIs1 + game.settings.levelFile + game.texts.levelIs2), 0, 162, 50);
	}
	
	std::memcpy(&gfx.frozenScreen[0], gfx.screen->pixels, gfx.frozenScreen.size());
	
	int fadeValue = 0;
	
	do
	{
		std::memcpy(gfx.screen->pixels, &gfx.frozenScreen[0], gfx.frozenScreen.size());

		for(int i = 0; i < 2; ++i)
		{
			int weapID = curSel[i] - 1;
			
			Worm& worm = *game.worms[i];
			WormSettings& ws = *worm.settings;
			
			if(!isReady[i])
			{
				menus[i].draw(ws.selWeapX - 2, 28, false, curSel[i]);
				
			/*
				int colour = 0;
				
				for(int j = 1; j <= game.settings.selectableWeapons; ++j)
				{
					int weapID = j - 1;
					int yPos = j * 8;
					
					if(j == curSel
				}
			*/
				
				
				if(weapID >= 0 && weapID < game.settings.selectableWeapons)
				{
					if(gfx.testKey(worm.keyLeft()))
					{
						gfx.releaseKey(worm.keyLeft());
						
						sfx.play(25, -1);
						
						do
						{
							--ws.weapons[weapID];
							if(ws.weapons[weapID] < 1)
								ws.weapons[weapID] = 40; // TODO: Unhardcode
						}
						while(game.settings.weapTable[game.weapOrder[ws.weapons[weapID]]] != 0);
						
						int w = game.weapOrder[ws.weapons[weapID]];
						worm.weapons[weapID].id = w;
						menus[i].items[curSel[i]].string = game.weapons[w].name;
					}
					
					if(gfx.testKey(worm.keyRight()))
					{
						gfx.releaseKey(worm.keyRight());
						
						sfx.play(26, -1);
						
						do
						{
							++ws.weapons[weapID];
							if(ws.weapons[weapID] > 40)
								ws.weapons[weapID] = 1; // TODO: Unhardcode
						}
						while(game.settings.weapTable[game.weapOrder[ws.weapons[weapID]]] != 0);
						
						int w = game.weapOrder[ws.weapons[weapID]];
						worm.weapons[weapID].id = w;
						menus[i].items[curSel[i]].string = game.weapons[w].name;
					}
				}
				
				if(gfx.testKeyOnce(worm.keyUp()))
				{
					sfx.play(26, -1);
					int s = int(menus[i].items.size());
					curSel[i] = (curSel[i] - 1 + s) % s;
				}
				
				if(gfx.testKeyOnce(worm.keyDown()))
				{
					sfx.play(25, -1);
					int s = int(menus[i].items.size());
					curSel[i] = (curSel[i] + 1 + s) % s;
				}
				
				if(gfx.testKey(worm.keyFire()))
				{
					if(curSel[i] == 0)
					{
						bool weapUsed[256] = {};
						
						bool enoughWeapons = (enabledWeaps >= game.settings.selectableWeapons);
						
						for(int j = 0; j < game.settings.selectableWeapons; ++j)
						{
							while(true)
							{
								ws.weapons[j] = game.rand(1, 41);
								
								int w = game.weapOrder[ws.weapons[j]];
								
								if((!enoughWeapons || !weapUsed[w])
								&& game.settings.weapTable[w] <= 0)
									break;
							}
							
							int w = game.weapOrder[ws.weapons[j]];
							
							weapUsed[w] = true;
							
							WormWeapon& ww = worm.weapons[j];
							
							ww.ammo = 0;
							ww.id = w;
							
							menus[i].items[j + 1].string = game.weapons[w].name;
						}
					}
					else if(curSel[i] == 6) // TODO: Unhardcode
					{
						sfx.play(27, -1);
						isReady[i] = true;
					}
				}
			}
		}
			
			/*
    {
     long weapid = cursel[w]-1;
     if(weapid >= 0 && weapid < SELECTABLE_WEAPONS)
     {
						cLOSP::bDisKeyBuffer[cSettings::sSettings.m_iControls[w][KEY_LEFT]] = true;
      if(cLOSP::bSoundEnabled)
      {
       playsound(0, 0, cLOSP::pSoundPointers[26]);
      } // B07A
      do
      {
       cSettings::sSettings.m_iWeapons[w][weapid]--;
       if(cSettings::sSettings.m_iWeapons[w][weapid] < 0)
       {
        cSettings::sSettings.m_iWeapons[w][weapid] = 39;
       } // B0C9
      } while(cSettings::sSettings.m_iWeapTable[cSettings::sWeap.order[cSettings::sSettings.m_iWeapons[w][weapid]]] != 0);

      cGame::cWorm[w].m_sWeapons[weapid].m_iID = cSettings::sWeap.order[cSettings::sSettings.m_iWeapons[w][weapid]];
     }
    } // B137

    if(cGame::cWorm[w].m_bKeyright)
    {
     long weapid = cursel[w]-1;
     if(weapid >= 0 && weapid < SELECTABLE_WEAPONS)
     {
      cLOSP::bDisKeyBuffer[cSettings::sSettings.m_iControls[w][KEY_RIGHT]] = true;
      if(cLOSP::bSoundEnabled)
      {
       playsound(0, 0, cLOSP::pSoundPointers[27]);
      } // B176
      do
      {
       cSettings::sSettings.m_iWeapons[w][weapid]++;
       if(cSettings::sSettings.m_iWeapons[w][weapid] > 39)
       {
        cSettings::sSettings.m_iWeapons[w][weapid] = 0;
       } // B1C5
      } while(cSettings::sSettings.m_iWeapTable[cSettings::sWeap.m_iOrder[cSettings::sSettings.m_iWeapons[w][weapid]]] != 0);

      cGame::cWorm[w].m_iWeapons[weapid].id = cSettings::sWeap.m_iOrder[cSettings::sSettings.m_iWeapons[w][weapid]];
     }
    } // B233
		*/
		
			/*
		do
 {
  for(w = 0; w <= 1; w++)
  {
   if(isready[w] == 0)
   {
    BYTE color;

    for(o = 1; o <= SELECTABLE_WEAPONS; o++)
    {
     long weapid = o - 1;
     long ypos = (o<<3);
     if(o == cursel[w])
     {
      wid = GetTextWidth(cSettings::sWeapNames.m_bData[cGame::cWorm[w].m_sWeapons[weapid].m_iID]);
      DrawRoundedBox(cSettings::iSelWeapX[w]-2, ypos+28, 0, 7, wid);
     } else // AE58
     {
      DrawTextMW(cSettings::sWeapNames.m_bData[cGame::cWorm[w].m_sWeapons[weapid].id], cSettings::iSelWeapX[w]+1, ypos+30, 0);
     } // AE9D

     
     if(o == cursel[w])
     {
      color = 168;
     } else
     {
      color = 48;
     }
     DrawTextMW(cSettings::sWeapNames.m_bData[cGame::cWorm[w].m_sWeapons[weapid].m_iID], cSettings::iSelWeapX[w], ypos+29, color);
    } // AF01

    wid = GetTextWidth(txt_done);
    if(cursel[w] == 6)
    {
     DrawRoundedBox(cSettings::iSelWeapX[w]-2, 76, 0, 7, wid);
    } else // AF3E
    {
     DrawTextMW(txt_done, cSettings::iSelWeapX[w]+1, 78, 0);
    } // AF64

    if(cursel[w] == 6) //NOTE! The table is a bit strange here!
    {
     color = 168;
    } else
    {
     color = 10;
    }

    DrawTextMW(cMenus::bTxtDone, cSettings::iSelWeapX[w], 77, color);

    wid = GetTextWidth(cMenus::bTxtRandomize);
    if(cursel[w] == 0)
    {
     DrawRoundedBox(cSettings::iSelWeapX[w]-2, 28, 0, 7, wid);
    } else // AFDB
    {
     DrawTextMW(cMenus::bTxtRandomize, cSettings::iSelWeapX[w]+1, 30, 0);
    } // B001

    if(cursel[w] == 0)
    {
     color = 168;
    } else
    {
     color = 13;
    }
    DrawTextMW(cMenus::bTxtRandomize, cSettings::iSelWeapX[w], 29, color);
*/
		gfx.origpal.rotate(168, 174);
		gfx.pal = gfx.origpal;
		
		if(fadeValue <= 32)
		{
			gfx.pal.fade(fadeValue);
			fadeValue += 1;
		}
		
		gfx.flip();
		gfx.process();
	}
	while((!isReady[0] || !isReady[1]) && !gfx.testSDLKey(SDLK_ESCAPE)); // Important that escape isn't released here
	
	for(std::size_t i = 0; i < game.worms.size(); ++i)
	{
		Worm& worm = *game.worms[i];
		
		worm.currentWeapon = 0; // It was 1 in OpenLiero A1
		
		for(int j = 0; j < game.settings.selectableWeapons; ++j)
		{
			worm.weapons[j].ammo = game.weapons[worm.weapons[j].id].ammo;
		}
		
		for(int j = 0; j < 6; ++j)
		{
			gfx.releaseKey(worm.settings->controls[j]);
		}
	}
}
	
/*
void SelectWeapons()
{
 //At this point, cEnabledWeaps contains the number of weapons that are selectable from the menu




 memcpy(cGFX::pScrTemp, cGFX::pScr, cGFX::iScrSize);

	fadevalue = 0;

 do
 {
  for(w = 0; w <= 1; w++)
  {
   if(isready[w] == 0)
   {
    BYTE color;

    for(o = 1; o <= SELECTABLE_WEAPONS; o++)
    {
     long weapid = o - 1;
     long ypos = (o<<3);
     if(o == cursel[w])
     {
      wid = GetTextWidth(cSettings::sWeapNames.m_bData[cGame::cWorm[w].m_sWeapons[weapid].m_iID]);
      DrawRoundedBox(cSettings::iSelWeapX[w]-2, ypos+28, 0, 7, wid);
     } else // AE58
     {
      DrawTextMW(cSettings::sWeapNames.m_bData[cGame::cWorm[w].m_sWeapons[weapid].id], cSettings::iSelWeapX[w]+1, ypos+30, 0);
     } // AE9D

     
     if(o == cursel[w])
     {
      color = 168;
     } else
     {
      color = 48;
     }
     DrawTextMW(cSettings::sWeapNames.m_bData[cGame::cWorm[w].m_sWeapons[weapid].m_iID], cSettings::iSelWeapX[w], ypos+29, color);
    } // AF01

    wid = GetTextWidth(txt_done);
    if(cursel[w] == 6)
    {
     DrawRoundedBox(cSettings::iSelWeapX[w]-2, 76, 0, 7, wid);
    } else // AF3E
    {
     DrawTextMW(txt_done, cSettings::iSelWeapX[w]+1, 78, 0);
    } // AF64

    if(cursel[w] == 6) //NOTE! The table is a bit strange here!
    {
     color = 168;
    } else
    {
     color = 10;
    }

    DrawTextMW(cMenus::bTxtDone, cSettings::iSelWeapX[w], 77, color);

    wid = GetTextWidth(cMenus::bTxtRandomize);
    if(cursel[w] == 0)
    {
     DrawRoundedBox(cSettings::iSelWeapX[w]-2, 28, 0, 7, wid);
    } else // AFDB
    {
     DrawTextMW(cMenus::bTxtRandomize, cSettings::iSelWeapX[w]+1, 30, 0);
    } // B001

    if(cursel[w] == 0)
    {
     color = 168;
    } else
    {
     color = 13;
    }
    DrawTextMW(cMenus::bTxtRandomize, cSettings::iSelWeapX[w], 29, color);

    if(cGame::cWorm[w].m_bKeyleft)
    {
     long weapid = cursel[w]-1;
     if(weapid >= 0 && weapid < SELECTABLE_WEAPONS)
     {
						cLOSP::bDisKeyBuffer[cSettings::sSettings.m_iControls[w][KEY_LEFT]] = true;
      if(cLOSP::bSoundEnabled)
      {
       playsound(0, 0, cLOSP::pSoundPointers[26]);
      } // B07A
      do
      {
       cSettings::sSettings.m_iWeapons[w][weapid]--;
       if(cSettings::sSettings.m_iWeapons[w][weapid] < 0)
       {
        cSettings::sSettings.m_iWeapons[w][weapid] = 39;
       } // B0C9
      } while(cSettings::sSettings.m_iWeapTable[cSettings::sWeap.order[cSettings::sSettings.m_iWeapons[w][weapid]]] != 0);

      cGame::cWorm[w].m_sWeapons[weapid].m_iID = cSettings::sWeap.order[cSettings::sSettings.m_iWeapons[w][weapid]];
     }
    } // B137

    if(cGame::cWorm[w].m_bKeyright)
    {
     long weapid = cursel[w]-1;
     if(weapid >= 0 && weapid < SELECTABLE_WEAPONS)
     {
      cLOSP::bDisKeyBuffer[cSettings::sSettings.m_iControls[w][KEY_RIGHT]] = true;
      if(cLOSP::bSoundEnabled)
      {
       playsound(0, 0, cLOSP::pSoundPointers[27]);
      } // B176
      do
      {
       cSettings::sSettings.m_iWeapons[w][weapid]++;
       if(cSettings::sSettings.m_iWeapons[w][weapid] > 39)
       {
        cSettings::sSettings.m_iWeapons[w][weapid] = 0;
       } // B1C5
      } while(cSettings::sSettings.m_iWeapTable[cSettings::sWeap.m_iOrder[cSettings::sSettings.m_iWeapons[w][weapid]]] != 0);

      cGame::cWorm[w].m_iWeapons[weapid].id = cSettings::sWeap.m_iOrder[cSettings::sSettings.m_iWeapons[w][weapid]];
     }
    } // B233

    if(cGame::cWorm[w].m_bKeyup)
    {
     cLOSP::bDisKeyBuffer[cSettings::sSettings.m_iControls[w][KEY_UP]] = true;
     if(cLOSP::bSoundEnabled)
     {
      playsound(0, 0, cLOSP::pSoundPointers[27]);
     } // B26F
     cursel[w]--;
     if(cursel[w] < 0)
     {
      cursel[w] = 6;
     }
    } // B28C

    if(cGame::cWorm[w].m_bKeydown)
    {
					cLOSP::bDisKeyBuffer[cSettings::sSettings.m_iControls[w][KEY_DOWN]] = true;
     if(cLOSP::bSoundEnabled)
     {
      playsound(0, 0, cLOSP::pSoundPointers[26]);
     } // B2C8
     cursel[w]++;
     if(cursel[w] > 6)
     {
      cursel[w] = 0;
     }
    } // B2E5

    if(cGame::cWorm[w].m_bKeyfire)
    {
     if(cursel[w] == 0)
     {
      memset(bWeapUsed, 0, 256);

      for(o = 0; o < SELECTABLE_WEAPONS; o++)
      {
       cSettings::sSettings.m_iWeapons[w][o] = random(40);

       if(cEnabledWeaps < SELECTABLE_WEAPONS)
       {
        while(1)
        {
         if(cSettings::sSettings.m_iWeapTable[cSettings::sWeap.m_iOrder[cSettings::sSettings.m_iWeapons[w][o]]] == 0)
          break;

         cSettings::sSettings.m_iWeapons[w][o] = random(40);
        }
       } else // B398
       {
        while(1)
        {
         if(!bWeapUsed[cSettings::sSettings.m_iWeapons[w][o]]) //Avoid choosing the same weapon twice
         {
          if(cSettings::sSettings.weaptable[cSettings::sWeap.m_iOrder[cSettings::sSettings.m_iWeapons[w][o]]] <= 0) break;
         } // B3E4
         cSettings::sSettings.weapons[w][o] = random(40);
        }
       } // B404
       bWeapUsed[cSettings::sSettings.weapons[w][o]] = true;

       cGame::cWorm[w].m_sWeapons[o].m_iAmmo = 0;
       cGame::cWorm[w].m_sWeapons[o].m_iID = cSettings::sWeap.m_iOrder[cSettings::sSettings.m_iWeapons[w][o]];
      } // B4A7
     } else if(cursel[w] == 6)
     {
      //NOTE! Disable the key here. Do we really need to?
      if(cLOSP::bSoundEnabled)
      {
       playsound(0, 0, cLOSP::pSoundPointers[28]);
      } // B4DB
      isready[w] = 1;
     } // B4E2
    }
   }
  } // B4EB

  AnimateOrigPal(168, 174);
  GetOrigPal();

  if(fadevalue < 256)
  {
   FadePalette(fadevalue);
   fadevalue += 8;
  } // B506

  Flip();
  
  memcpy(cGFX::pScr, cGFX::pScrTemp, cGFX::pScrSize); //Restore the original screen
 } while((isready[0] == 0 || isready[1] == 0) && !cLOSP::bKeyBuffer[DIK_ESCAPE]);

//l_B53F:
 for(w = 0; w < 2; w++)
 {
  cGame::cWorm[w].m_iCurrentWeapon = 1;
  for(o = 0; o < SELECTABLE_WEAPONS; o++) //Fill up ammo!
  {
   cGame::cWorm[w].m_sWeapons[o].m_iAmmo = cSettings::sWeap.m_iAmmo[cGame::cWorm[w].m_sWeapons[o].m_iID];
  } // B5A2
 } // B5A8

 for(w = 0; w; w++)
 {
  for(wid = 0; wid <= 6; wid++)
  {
			cLOSP::bDisKeyBuffer[cSettings::sSettings.m_iControls[w][wid]] = true;
  }
 } // B5DE

}
*/
