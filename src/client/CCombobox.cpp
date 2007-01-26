/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Combo box
// Created 3/9/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


///////////////////
// Draw the combo box
void CCombobox::Draw(SDL_Surface *bmpDest)
{
	mouse_t *tMouse = GetMouse();

	// Strip text buffer
	static char buf[256];

	// Count the item height
	int ItemHeight = tLX->cFont.GetHeight()+1;
	if (iItemCount)
		if (tItems->tImage)
			if ((tItems->tImage->h+1) > ItemHeight)
				ItemHeight = tItems->tImage->h+1;

    Menu_redrawBufferRect( iX,iY, iWidth+15,17);
    if( !iDropped && iLastDropped ) {
        Menu_redrawBufferRect( iX,iY+17, iWidth+15,117);
        iLastDropped = false;
    }

	// Draw the background bit
	Menu_DrawBoxInset(bmpDest, iX, iY, iX+iWidth, iY+ItemHeight+1);

	if(iDropped) {
		// Dropped down
		if(tSelected)  {
			fix_strncpy(buf,tSelected->sName);
			if (tSelected->tImage)  {
				DrawImage(bmpDest,tSelected->tImage,iX+3,iY+1);
				stripdot(buf,iWidth-(6+tSelected->tImage->w+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+6+tSelected->tImage->w, iY+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2), MakeColour(128,128,128),"%s", buf);
			}
			else  {
				stripdot(buf,iWidth-(3+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+3, iY+2, tLX->clDisabled,"%s", buf);
			}
		}

        iLastDropped = true;

		// Change the widget's height
		iHeight = 0;
		int display_count = 6;
		if (iItemCount < display_count)
			display_count = iItemCount;
		iHeight = ItemHeight*(display_count+1)+5;
		// Screen clipping
		while (iHeight+iY > bmpDest->h)  {
			display_count--;
			iGotScrollbar = true;
			iHeight = ItemHeight*(display_count+1)+5;
		}
		cScrollbar.Setup(0, iX+iWidth-16, iY+ItemHeight+4, 14, iHeight-22);


		Menu_DrawBox(bmpDest, iX, iY+ItemHeight+2, iX+iWidth, iY+iHeight);
		DrawRectFill(bmpDest, iX+2,iY+ItemHeight+4,iX+iWidth-1, iY+iHeight-1,0);

		// Draw the items
		int count=0;
		int y = iY+ItemHeight+4;
		int w = iX+iWidth-1;
		if(iGotScrollbar)  {
			w-=16;
			cScrollbar.Draw(bmpDest);
		}

		cb_item_t *item = tItems;
		for(;item;item=item->tNext,count++) {
			if(count < cScrollbar.getValue())
				continue;

            bool selected = false;

			if(tMouse->X > iX && tMouse->X < w)
				if(tMouse->Y >= y && tMouse->Y < y+ItemHeight)  {
                    selected = true;
					iKeySelectedItem = -1;
				}

			if(iKeySelectedItem == item->iIndex)
				selected = true;

            if(selected)
                DrawRectFill(bmpDest, iX+2, y, w, y+ItemHeight-1, MakeColour(0,66,102));

			fix_strncpy(buf,item->sName);

			bool stripped = false;

			if (item->tImage)  {
				// Draw the image
				DrawImage(bmpDest,item->tImage,iX+3,y);
				stripped = stripdot(buf,iWidth-(6+tSelected->tImage->w+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+6+item->tImage->w, y, tLX->clDropDownText,"%s", buf);
				if (stripped && selected)  {
					int x1 = iX+4+tSelected->tImage->w;
					int y1 = y+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2);
					int x2 = iX+4+tSelected->tImage->w+tLX->cFont.GetWidth(item->sName)+4;
					if (x2 > bmpDest->w)  {
						x1 = bmpDest->w-x2-5;
						x2 = bmpDest->w-5;
					}
					int y2 = y1+tLX->cFont.GetHeight();

					DrawRect(bmpDest,x1-1,y1-1,x2,y2,MakeColour(220,220,220));
					DrawRectFill(bmpDest,x1,y1,x2,y2,MakeColour(40,84,122));
					tLX->cFont.Draw(bmpDest, x1+2, y1-1, tLX->clDropDownText,"%s", item->sName);
				}
			}
			else  {
				stripped = stripdot(buf,iWidth-(3+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+3, y+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2), tLX->clDropDownText,"%s", buf);
				if (stripped && selected)  {
					int x1 = iX+4;
					int y1 = y+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2);
					int x2 = iX+4+tLX->cFont.GetWidth(item->sName)+1;
					if (x2 > bmpDest->w)  {
						x1 = bmpDest->w-x2-5;
						x2 = bmpDest->w-5;
					}
					int y2 = y1+tLX->cFont.GetHeight();

					DrawRect(bmpDest,x1-1,y1-1,x2,y2,MakeColour(220,220,220));
					DrawRectFill(bmpDest,x1,y1,x2,y2,MakeColour(40,84,122));
					tLX->cFont.Draw(bmpDest, x1-1, y1, tLX->clDropDownText,"%s", item->sName);
				}
			}

			y+=ItemHeight;
			if(y+ItemHeight >= iY+iHeight)
				break;
		}

	} else {
		// Normal
		if (tSelected)  {
			fix_strncpy(buf,tSelected->sName);
			if (tSelected->tImage)  {
				DrawImage(bmpDest,tSelected->tImage,iX+3,iY+1);
				stripdot(buf,iWidth-(6+tSelected->tImage->w+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+6+tSelected->tImage->w, iY+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2), tLX->clDropDownText,"%s", buf);
			}
			else  {
				stripdot(buf,iWidth-(3+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+3, iY+2, tLX->clDropDownText,"%s", buf);
			}
		}

		iHeight = ItemHeight+3;
	}

	// Button
	int x=0;
	if(iArrowDown)
		x=15;
	DrawImageAdv(bmpDest, gfxGUI.bmpScrollbar, x,14, iX+iWidth-16,iY+2, 15,14);

	if(!iFocused)  {
		iDropped = false;
		iKeySelectedItem = -1;
	}

	iArrowDown = false;
}

//////////////////////
// Sorts te items in the combobox
void CCombobox::Sort(bool ascending)
{
	// Get the item
	cb_item_t *item = tItems;
	if (!item)
		return;

	cb_item_t *prev_item = NULL;
	cb_item_t *next_item = NULL;

	bool bSwapped = true;

	// Bubble sort the list
	while (bSwapped)  {
		bSwapped = false;
		prev_item = NULL;
		for(item=tItems;item && item->tNext;item=item->tNext) {

			// Get next item
			next_item = item->tNext;

			bool swap = false;

			// Swap the two items?
			int nat_cmp1 = atoi(item->sName);
			int nat_cmp2 = atoi(next_item->sName);
			// First try, if we compare numbers
			if (nat_cmp1 && nat_cmp2)  {
				if (ascending)
					swap = nat_cmp1 > nat_cmp2;
				else
					swap = nat_cmp2 > nat_cmp1;
			// String comparison
			} else {
				int tmp = strncasecmp(item->sName,next_item->sName,sizeof(item->sName));
				if (ascending)
					swap = tmp > 0;
				else
					swap = tmp < 0;
			}

			// Swap if they're not in the right order
			if (swap)  {
				cb_item_t *it4 = item->tNext->tNext;
				if (prev_item)
					prev_item->tNext = next_item;
				else
					tItems = next_item;
				next_item->tNext = item;
				item->tNext = it4;
				bSwapped = true;
				prev_item = item;
				continue;
			}

			prev_item = item;
		}
	}

	// Update the indexes
	int i=0;
	for (item=tItems;item;item=item->tNext,i++)
		item->iIndex = i;
}


///////////////////
// Create the combo box
void CCombobox::Create(void)
{
	tItems = NULL;
	tSelected = NULL;
	iItemCount = 0;
	iGotScrollbar = false;
	iDropped = false;
	iArrowDown = false;
	iKeySelectedItem = -1;

	cScrollbar.Create();
	cScrollbar.Setup(0, iX+iWidth-16, iY+20, 14, 95);
	cScrollbar.setMin(0);
	cScrollbar.setMax(1);
	cScrollbar.setValue(0);
	cScrollbar.setItemsperbox(7);
}


///////////////////
// Destroy the combo box
void CCombobox::Destroy(void)
{
	// Free the items
	cb_item_t *i,*item;
	for(i=tItems;i;i=item) {
		item = i->tNext;

		// Free the item
		assert(i);
		delete i;
	}

	tItems = NULL;
	tSelected = NULL;

	// Destroy the scrollbar
	cScrollbar.Destroy();
}


///////////////////
// Mouse over event
int CCombobox::MouseOver(mouse_t *tMouse)
{
	if(tMouse->X >= iX+iWidth-16 && iGotScrollbar && iDropped)
		cScrollbar.MouseOver(tMouse);

	return CMB_NONE;
}


///////////////////
// Mouse down event
int CCombobox::MouseDown(mouse_t *tMouse, int nDown)
{
//	int x = iX+iWidth-16;  // TODO: not used
	iArrowDown = false;

	if((tMouse->X >= iX+iWidth-16 || cScrollbar.getGrabbed()) && iGotScrollbar && iDropped) {
		cScrollbar.MouseDown(tMouse, nDown);
		return CMB_NONE;
	}

	if(tMouse->X >= iX && tMouse->X <= iX+iWidth)
		if(tMouse->Y >= iY && tMouse->Y < iY+iHeight) {

            //
            // If we aren't dropped, shift the scroll bar
            //
            if(!iDropped) {
                cb_item_t *i = tItems;
                int count = 0;
                for(; i; i=i->tNext, count++) {
                    if(i->iSelected) {
                        // Setup the scroll bar so it shows this item in the middle
                        cScrollbar.setValue( count - cScrollbar.getItemsperbox() / 2 );
                        break;
                    }
                }
            }


            // Drop it
			iNow = (int)GetMilliSeconds();
			if (!iDropped)  {
				iArrowDown = true;
				iDropped = true;
				iKeySelectedItem = -1;
				iDropTime = iNow;
			}

			if (iNow-iDropTime <= 20 && iDropTime != 0 && (tMouse->Y < iY+20))  // let the arrow pushed a bit longer
				iArrowDown = true;


		}

	return CMB_NONE;
}


///////////////////
// Mouse up event
int CCombobox::MouseUp(mouse_t *tMouse, int nDown)
{
	iArrowDown = false;

	if(tMouse->X >= iX+iWidth-16 && iGotScrollbar && iDropped) {
		cScrollbar.MouseUp(tMouse, nDown);
		return CMB_NONE;
	}

	// Go through the items checking for a mouse click
	int count=0;
	int y = iY+20;
	int w = iX+iWidth-1;
	if(iGotScrollbar)
		w-=16;

	cb_item_t *item = tItems;
	for(;item;item=item->tNext,count++) {
		if(count < cScrollbar.getValue())
			continue;

		if(tMouse->X > iX && tMouse->X < w)
			if(tMouse->Y >= y && tMouse->Y < y+16)
				if(tMouse->Up & SDL_BUTTON(1)) {
                    if(tSelected)
                        tSelected->iSelected = false;

					tSelected = item;
                    tSelected->iSelected = true;
					iDropped = false;
					return CMB_CHANGED;
				}


		y+=16;
		if(y > iY+iHeight)
			break;
	}


	return CMB_NONE;
}

///////////////////
// Mouse wheel down event
int CCombobox::MouseWheelDown(mouse_t *tMouse)
{
	if(iGotScrollbar && iDropped)
		cScrollbar.MouseWheelDown(tMouse);

	if(!iDropped)  {
		if (tSelected->tNext) {
			setCurItem(tSelected->tNext->iIndex);
			return CMB_CHANGED;
		}
	}

	return CMB_NONE;
}

///////////////////
// Mouse wheel up event
int CCombobox::MouseWheelUp(mouse_t *tMouse)
{
	if(iGotScrollbar && iDropped)
		cScrollbar.MouseWheelUp(tMouse);

	if(!iDropped)  {
		cb_item_t *item = tItems;
		for(;item;item=item->tNext)  {
			if (!item->tNext)
				break;
			if (item->tNext->iIndex == tSelected->iIndex)  {
				setCurItem(item->iIndex);
				return CMB_CHANGED;
			}  // if
		}  // for
	}

	return CMB_NONE;
}

//////////////////
// Key down event
int CCombobox::KeyDown(int c)
{
	// Search for items by pressed key
	if (!iCanSearch)
		return CMB_NONE;

	iCanSearch = false;

	// Go from current item to the end of the list
	int count = 0;
	cb_item_t *item = tSelected->tNext;
	for(;item;item=item->tNext)  {
		if (chrcasecmp(item->sName[0],(char)c))  {
			tSelected = item;
			cScrollbar.setValue( item->iIndex - cScrollbar.getItemsperbox() / 2 );
			iKeySelectedItem = item->iIndex;
			return CMB_CHANGED;
		}  // if
	}  // for

	// If not found, go from the beginning to the selected item
	item = tItems;
	count = 0;
	for (;item && item->iIndex != tSelected->iIndex;item=item->tNext)  {
		if (chrcasecmp(item->sName[0],(char)c))  {
			tSelected = item;
			cScrollbar.setValue( count - cScrollbar.getItemsperbox() / 2 );
			iKeySelectedItem = item->iIndex;
			return CMB_CHANGED;
		}
	}


	// Not found
	return CMB_NONE;
}


///////////////////
// Process a message sent to this widget
int CCombobox::SendMessage(int iMsg, DWORD Param1, DWORD Param2)
{

	switch(iMsg) {

		// Add item message
		case CBM_ADDITEM:
			addItem(Param1, "", (char *)Param2);
			return iItemCount;

		// Add item message (string index)
		case CBM_ADDSITEM:
			addItem(0, (char *)Param1, (char *)Param2);
			return 0;

		// Get the current item's index
		case CBM_GETCURINDEX:
			if(tSelected)
				return tSelected->iIndex;
			break;

		// Get the current item's string index
		case CBM_GETCURSINDEX:
			if(tSelected) {
				strncpy((char *)Param1, tSelected->sIndex, Param2);
				char *p = (char *)Param1;
				p[Param2-1] = '\0';
			}
			break;

        // Get the current item's name
        case CBM_GETCURNAME:
            if(tSelected) {
				strncpy((char *)Param1, tSelected->sName, Param2);
				char *p = (char *)Param1;
				p[Param2-1] = '\0';
			}
			break;


		// Get the current item
		case CBM_GETCURITEM:
			return (int)tSelected;

		// Set the current item
		case CBM_SETCURSEL:
			setCurItem(Param1);
			break;

        // Set the current item based on the string index
        case CBM_SETCURSINDEX:
            setCurSIndexItem((char *)Param1);
            break;

        // Set the current item based on the int index
        case CBM_SETCURINDEX:
            setCurIndexItem(Param1);
            break;

		// Set the image for the specified item
		case CBM_SETIMAGE:
			setImage((SDL_Surface *) Param2, Param1);
			break;

		// Return true, if the combobox is dropped
		case CBM_ISDROPPED:
			return iDropped;
			break;
	}


	return 0;
}


///////////////////
// Add an item to the combo box
void CCombobox::addItem(int index, char *sindex, char *name)
{
	cb_item_t *item;

	item = new cb_item_t;
	if(item == NULL)
		return;

	// Fill in the info
	item->iIndex = index;
	fix_strncpy(item->sIndex, sindex);
	fix_strncpy(item->sName,name);
	item->tNext = NULL;
	item->iSelected = false;
	item->tImage = NULL;

	if(!tSelected) {
		// Set the first item to selected
		tSelected = item;
		item->iSelected = true;
	}


	// Add it to the list
	if(tItems) {
		cb_item_t *i = tItems;
		for(;i;i = i->tNext) {
			if(i->tNext == NULL) {
				i->tNext = item;
				break;
			}
		}
	}
	else
		tItems = item;


	iItemCount++;
    cScrollbar.setMax( iItemCount );

	iGotScrollbar = false;
	if(iItemCount*16 >= 100)
		iGotScrollbar = true;
}


///////////////////
// Set the current item based on count
void CCombobox::setCurItem(int index)
{
	cb_item_t *i = tItems;
	int count=0;

	for(; i; i=i->tNext, count++) {
		if(count == index) {
			if(tSelected)
				tSelected->iSelected = false;

			tSelected = i;
			tSelected->iSelected = true;
			return;
		}
	}
}


///////////////////
// Set the current item based on string index
void CCombobox::setCurSIndexItem(char *szString)
{
    cb_item_t *i = tItems;
	for(; i; i=i->tNext) {

        if( stricmp(i->sIndex,szString) == 0 ) {
            if(tSelected)
                tSelected->iSelected = false;

            tSelected = i;
            tSelected->iSelected = true;
            return;
        }
    }
}


///////////////////
// Set the current item based on numerical index
void CCombobox::setCurIndexItem(int nIndex)
{
    cb_item_t *i = tItems;
	for(; i; i=i->tNext) {
        if( i->iIndex == nIndex ) {
            if(tSelected)
                tSelected->iSelected = false;

            tSelected = i;
            tSelected->iSelected = true;
            return;
        }
    }
}

///////////////////
// Set the image for the specified item
void CCombobox::setImage(SDL_Surface *img, int ItemIndex)
{
	cb_item_t *i = tItems;
	for(; i; i=i->tNext)
		if (i->iIndex == ItemIndex) {
			i->tImage = img;
			break;
		}
}


///////////////////
// Clear the data
void CCombobox::clear(void)
{
    Destroy();
    Create();
}


cb_item_t* CCombobox::getItem(int index) {
	for(cb_item_t* i = tItems; i; i = i->tNext) {
		if(i->iIndex == index)
			return i;
	}
	return NULL;
}

int	CCombobox::getItemsCount() {
	int c = 0;
	for(cb_item_t* i = tItems; i; i = i->tNext)
		c++;
	return c;
}

cb_item_t* CCombobox::getItem(char* name) {
	for(cb_item_t* i = tItems; i; i = i->tNext) {
		if(strncmp(i->sName,name,sizeof(i->sName)) == 0)
			return i;
	}
	return NULL;
}
