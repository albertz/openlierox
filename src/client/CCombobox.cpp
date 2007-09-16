/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Combo box
// Created 3/9/02
// Jason Boettcher

#include <assert.h>

#include "LieroX.h"
#include "Graphics.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"


///////////////////
// Draw the combo box
void CCombobox::Draw(SDL_Surface *bmpDest)
{
	mouse_t *tMouse = GetMouse();

	// Strip text buffer
	static std::string buf;

	int mainbitheight = MAX(tLX->cFont.GetHeight()+1, 16);  // 16 - arrow height

	// Count the item height
	int ItemHeight = tLX->cFont.GetHeight()+1;
	if (tItems)
		if (tItems->tImage)
			if ((tItems->tImage->h+1) > ItemHeight)
				ItemHeight = tItems->tImage->h+1;

	if (bRedrawMenu)
		Menu_redrawBufferRect( iX,iY, iWidth+15,tLX->cFont.GetHeight()+4);
    if( !iDropped && iLastDropped ) {
		if (bRedrawMenu)
			Menu_redrawBufferRect( iX,iY+tLX->cFont.GetHeight(), iWidth+15,iHeight);
        iLastDropped = false;
    }

	// Draw the background bit
	Menu_DrawBoxInset(bmpDest, iX, iY, iX+iWidth, iY+mainbitheight+1);

	if(iDropped) {
		// Dropped down
		if(tSelected)  {
			buf = tSelected->sName;
			if (tSelected->tImage)  {
				DrawImage(bmpDest,tSelected->tImage,iX+3,iY+1);
				stripdot(buf,iWidth-(6+tSelected->tImage->w+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+6+tSelected->tImage->w, iY+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2), tLX->clDisabled, buf);
			}
			else  {
				stripdot(buf,iWidth-(3+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+3, iY+2, tLX->clDisabled,buf);
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
		cScrollbar.Setup(0, iX+iWidth-16, iY+ItemHeight+4, 14, iHeight-tLX->cFont.GetHeight()-6);


		Menu_DrawBox(bmpDest, iX, iY+ItemHeight+2, iX+iWidth, iY+iHeight);
		DrawRectFill(bmpDest, iX+2,iY+ItemHeight+4,iX+iWidth-1, iY+iHeight-1,tLX->clBlack);

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
                DrawRectFill(bmpDest, iX+2, y, w, y+ItemHeight-1, tLX->clComboboxSelected);

			buf = item->sName;

			bool stripped = false;

			if (item->tImage)  {
				// Draw the image
				DrawImage(bmpDest,item->tImage,iX+3,y);
				stripped = stripdot(buf,iWidth-(6+tSelected->tImage->w+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+6+item->tImage->w, y, tLX->clDropDownText,buf);
				if (stripped && selected)  {
					int x1 = iX+4+tSelected->tImage->w;
					int y1 = y+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2);
					int x2 = iX+4+tSelected->tImage->w+tLX->cFont.GetWidth(item->sName)+4;
					if (x2 > bmpDest->w)  {
						x1 = bmpDest->w-x2-5;
						x2 = bmpDest->w-5;
					}
					int y2 = y1+tLX->cFont.GetHeight();

					DrawRect(bmpDest,x1-1,y1-1,x2,y2, tLX->clComboboxShowAllBorder);
					DrawRectFill(bmpDest,x1,y1,x2,y2, tLX->clComboboxShowAllMain);
					tLX->cFont.Draw(bmpDest, x1+2, y1-1, tLX->clDropDownText,item->sName);
				}
			}
			else  {
				stripped = stripdot(buf,iWidth-(3+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+3, y+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2), tLX->clDropDownText, buf);
				if (stripped && selected)  {
					int x1 = iX+4;
					int y1 = y+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2);
					int x2 = iX+4+tLX->cFont.GetWidth(item->sName)+1;
					if (x2 > bmpDest->w)  {
						x1 = bmpDest->w-x2-5;
						x2 = bmpDest->w-5;
					}
					int y2 = y1+tLX->cFont.GetHeight();

					DrawRect(bmpDest,x1-1,y1-1,x2,y2, tLX->clComboboxShowAllBorder);
					DrawRectFill(bmpDest,x1,y1,x2,y2, tLX->clComboboxShowAllMain);
					tLX->cFont.Draw(bmpDest, x1-1, y1, tLX->clDropDownText, item->sName);
				}
			}

			y+=ItemHeight;
			if(y+ItemHeight >= iY+iHeight)
				break;
		}

	} else {
		// Normal
		if (tSelected)  {
			buf = tSelected->sName;
			if (tSelected->tImage)  {
				DrawImage(bmpDest,tSelected->tImage,iX+3,iY+1);
				stripdot(buf,iWidth-(6+tSelected->tImage->w+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+6+tSelected->tImage->w, iY+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2), tLX->clDropDownText, buf);
			}
			else  {
				stripdot(buf,iWidth-(3+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+3, iY+2, tLX->clDropDownText, buf);
			}
		}

		iHeight = ItemHeight+3;
	}

	// Button
	int x=0;
	if(iArrowDown)
		x=15;
	DrawImageAdv(bmpDest, gfxGUI.bmpScrollbar, x,14, iX+iWidth-16,iY+2, 15,14);

	if(!bFocused)  {
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
			bool failed1,failed2;
			int nat_cmp1 = from_string<int>(item->sName,failed1);
			int nat_cmp2 = from_string<int>(next_item->sName,failed2);
			// First try, if we compare numbers
			if (!failed1 && !failed2)  {
				if (ascending)
					swap = nat_cmp1 > nat_cmp2;
				else
					swap = nat_cmp2 > nat_cmp1;
			// String comparison
			} else {
				int tmp = stringcasecmp(item->sName,next_item->sName);
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
	tLastItem = NULL;
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
	cScrollbar.setItemsperbox(6);
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
	tLastItem = NULL;
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


            // Drop or close it
			iNow = (int)(GetMilliSeconds() * 1000);
			if (tMouse->FirstDown)  {
				if (!iDropped)  {  // Not dropped, drop it
					iArrowDown = true;
					iDropped = true;
					iKeySelectedItem = -1;
					iDropTime = iNow;
				} else {
					// If clicked the arrow or body again, close the combobox
					int mainbitheight = MAX(tLX->cFont.GetHeight()+1, 16);
					if (tMouse->Y < iY + mainbitheight)
						iDropped = false;
				}
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

	// Count the item height
	int ItemHeight = tLX->cFont.GetHeight()+1;
	if (tItems)
		if (tItems->tImage)
			if ((tItems->tImage->h+1) > ItemHeight)
				ItemHeight = tItems->tImage->h+1;

	// Go through the items checking for a mouse click
	int count=0;
	int y = iY+tLX->cFont.GetHeight()+4;
	int w = iX+iWidth-1;
	if(iGotScrollbar)
		w-=16;

	cb_item_t *item = tItems;
	for(;item;item=item->tNext,count++) {
		if(count < cScrollbar.getValue())
			continue;

		if(tMouse->X > iX && tMouse->X < w)
			if(tMouse->Y >= y && tMouse->Y < y + ItemHeight)
				if(tMouse->Up & SDL_BUTTON(1)) {
                    if(tSelected)
                        tSelected->iSelected = false;

					tSelected = item;
                    tSelected->iSelected = true;
					iDropped = false;
					return CMB_CHANGED;
				}


		y += ItemHeight;
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
int CCombobox::KeyDown(UnicodeChar c, int keysym)
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
DWORD CCombobox::SendMessage(int iMsg, DWORD Param1, DWORD Param2)
{

	switch(iMsg) {

		// Get the current item's index
		case CBM_GETCURINDEX:
			if(tSelected)
				return tSelected->iIndex;
			break;

		// Get the current item
		case CBM_GETCURITEM:
			return (DWORD)tSelected;

		// Set the current item
		case CBM_SETCURSEL:
			setCurItem(Param1);
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

		// Set the sorted property
		case CBM_SETSORTED:
			bSorted = Param1 != 0;
			break;

		// Set the unique property
		case CBM_SETUNIQUE:
			bUnique = Param1 != 0;
			break;
	}


	return 0;
}

DWORD CCombobox::SendMessage(int iMsg, const std::string& sStr, DWORD Param)
{
	switch (iMsg)  {
	// Add item message
	case CBS_ADDITEM:
		addItem(Param,"",sStr);
		break;
	// Add item message (string index)
	case CBS_ADDSITEM:
		addItem(0, sStr, *((std::string *) Param));
		break;
	// Set the current item based on the string index
	case CBS_SETCURSINDEX:
		setCurSIndexItem(sStr);
		break;
	}

	return 0;
}

DWORD CCombobox::SendMessage(int iMsg, std::string *sStr, DWORD Param)
{
	switch (iMsg)  {
	// Get the current item's string index
	case CBS_GETCURSINDEX:
		if (tSelected)  {
			*sStr = tSelected->sIndex;
			return 1;
		}
		else  {
			*sStr = "";
			return 0;
		}
		break;

	// Get the current item's name
	case CBS_GETCURNAME:
		if (tSelected)  {
			*sStr = tSelected->sName;
			return 1;
		} else {
			*sStr = "";
			return 0;
		}
		break;
	}

	return 0;
}


///////////////////
// Add an item to the combo box
bool CCombobox::addItem(int index, const std::string& sindex, const std::string& name)
{
	cb_item_t *item;

	item = new cb_item_t;
	if(item == NULL)
		return false;

	// Fill in the info
	item->iIndex = index;
	item->sIndex = sindex;
	item->sName = name;
	item->tNext = NULL;
	item->tPrev = NULL;
	item->iSelected = false;
	item->tImage = NULL;

	//
	// Add it to the list
	//

	// First item
	if (!tItems || !tLastItem)  {
		tItems = item;
		tLastItem = item;

		// Select the first item
		item->iSelected = true;
		tSelected = item;

		iItemCount++;

		return true;
	}

	// List should be automatically sorted when adding
	if (bSorted)  {
		cb_item_t *it = tLastItem;
		int res = -1;
		while (it)  {
			if ((res = stringcasecmp(name, it->sName)) >= 0)
				break;
			it = it->tPrev;
		}
		
		// Another item wih this name already exists
		// If every item should be unique, we don't add it
		if (res == 0 && bUnique)  {
			delete item;
			return false;
		}

		// Link it in
		if (it)  {
			item->tPrev = it;
			if (it->tNext)  // Somewhere in the middle
				it->tNext->tPrev = item;
			else  // Last item
				tLastItem = item;
			item->tNext = it->tNext;
			it->tNext = item;
		} else {  // First item
			tItems->tPrev = item;
			item->tNext = tItems;
			tItems = item;
		}

	// Not sorted, just put it at the end
	} else {
		// Check for duplicates
		if (bUnique)  {
			for (cb_item_t *it = tLastItem; it; it = it->tPrev)
				if (stringcasecmp(it->sName, name) == 0)  {
					delete item;
					return false;
				}
		}

		tLastItem->tNext = item;
		item->tPrev = tLastItem;
		tLastItem = item;
	}

	// If no item is selected, select this one
	if(!tSelected) {
		tSelected = item;
		item->iSelected = true;
	}


	iItemCount++;
    cScrollbar.setMax( iItemCount );

	iGotScrollbar = iItemCount > 6;

	return true;
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
// Set the current item based on item pointer
void CCombobox::setCurItem(cb_item_t *it)
{
	if (it == NULL)
		return;

	if(tSelected)
		tSelected->iSelected = false;

	tSelected = it;
	tSelected->iSelected = true;
}


///////////////////
// Set the current item based on string index
void CCombobox::setCurSIndexItem(const std::string& szString)
{
    cb_item_t *i = tItems;
	for(; i; i=i->tNext) {

        if( stringcasecmp(i->sIndex,szString) == 0 ) {
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


///////////////
// Get the item based on its index property
cb_item_t* CCombobox::getItem(int index) 
{
	for(cb_item_t* i = tItems; i; i = i->tNext) {
		if(i->iIndex == index)
			return i;
	}
	return NULL;
}

/////////////
// Get the number of items
int	CCombobox::getItemsCount() {
	return iItemCount;
}

/////////////
// Get the item based on its displayed name
cb_item_t* CCombobox::getItem(const std::string& name) {
	for(cb_item_t* i = tItems; i; i = i->tNext) {
		if(stringcasecmp(i->sName,name) == 0)
			return i;
	}
	return NULL;
}

////////////
// Get index if the selected item
int CCombobox::getSelectedIndex()  {
	int result = 0;
	for (cb_item_t *i = tItems; i && i != tSelected; i=i->tNext,result++) {};
	return result;
}
