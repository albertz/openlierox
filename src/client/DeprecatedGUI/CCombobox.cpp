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
#include <algorithm>

#include "LieroX.h"
#include "DeprecatedGUI/CCombobox.h"

#include "DeprecatedGUI/Graphics.h"
#include "DeprecatedGUI/Menu.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"
#include "Sounds.h"
#include "Timer.h"
#include "Debug.h"


namespace DeprecatedGUI {

///////////////////
// Draw the combo box
void CCombobox::Draw(SDL_Surface * bmpDest)
{
	mouse_t *tMouse = GetMouse();

	// Strip text buffer
	std::string buf;

	// Count the item height
	int ItemHeight = getItemHeight();

	int mainbitheight = MAX(ItemHeight, MAX(tLX->cFont.GetHeight()+1, 16));  // 16 - arrow height
	
	if (bRedrawMenu)
		Menu_redrawBufferRect( iX,iY, iWidth+15,tLX->cFont.GetHeight()+4);
    if( !bDropped && bLastDropped ) {
		// HINT: the background is repainted when closing the combobox
		// That is done because if we did it here, it would also erase widgets that were drawn before us
		// (see CGuiLayout::Draw)
        bLastDropped = false;
    }

	// Draw the background bit
	Menu_DrawBoxInset(bmpDest, iX, iY, iX+iWidth, iY+mainbitheight+1);

	if(bDropped) {
		// Dropped down
		if(getItemRW(iSelected))  {
			buf = getItemRW(iSelected)->sName;
			if (getItemRW(iSelected)->tImage.get())  {
				getItemRW(iSelected)->tImage->draw(bmpDest,iX+3,iY+1);
				stripdot(buf, iWidth - (8 + getItemRW(iSelected)->tImage.get()->w + (bGotScrollbar ? 15 : 0)));
				tLX->cFont.Draw(bmpDest, iX+8+getItemRW(iSelected)->tImage.get()->w, iY+2+(ItemHeight - tLX->cFont.GetHeight()) / 2, tLX->clDisabled, buf);
			}
			else  {
				stripdot(buf,iWidth-(3 + bGotScrollbar ? 15 : 0));
				tLX->cFont.Draw(bmpDest, iX+3, iY+2, tLX->clDisabled,buf);
			}
		}

        bLastDropped = true;

		// Change the widget's height
		iHeight = 0;
		size_t display_count = 6;
		if (tItems.size() < display_count)
			display_count = tItems.size();
		iHeight = (int)(ItemHeight*(display_count+1)+5);
		// Screen clipping
		while (iHeight+iY > bmpDest->h && display_count)  {
			display_count--;
			bGotScrollbar = true;
			iHeight = (int)(ItemHeight*(display_count+1)+5);
		}
		cScrollbar.Setup(0, iX+iWidth-16, iY+ItemHeight+4, 14, iHeight-mainbitheight-6);


		Menu_DrawBox(bmpDest, iX, iY+ItemHeight+2, iX+iWidth, iY+iHeight);
		DrawRectFill(bmpDest, iX+2,iY+ItemHeight+4,iX+iWidth-1, iY+iHeight-1,tLX->clBlack);

		// Draw the items
		int count=0;
		int y = iY+ItemHeight+4;
		int w = iX+iWidth-1;
		if(bGotScrollbar)  {
			w-=16;
			cScrollbar.Draw(bmpDest);
		}

		int index = 0;
		for(std::list<cb_item_t>::const_iterator item = tItems.begin(); item != tItems.end(); item++, count++, index++) {
			if(count < cScrollbar.getValue())
				continue;

            bool selected = false;

			if(tMouse->X > iX && tMouse->X < w)
				if(tMouse->Y >= y && tMouse->Y < y+ItemHeight)  {
                    selected = true;
					iKeySelectedItem = -1;
				}

			if(iKeySelectedItem == index)
				selected = true;

            if(selected)
                DrawRectFill(bmpDest, iX+2, y, w, y+ItemHeight-1, tLX->clComboboxSelected);

			buf = item->sName;

			bool stripped = false;

			if (item->tImage.get())  {
				// Draw the image
				item->tImage->draw(bmpDest,iX+3,y);
				stripped = stripdot(buf,iWidth - (8 + item->tImage.get()->w + bGotScrollbar ? 15 : 0));
				tLX->cFont.Draw(bmpDest, iX+8+item->tImage.get()->w, y + (ItemHeight-tLX->cFont.GetHeight())/2, tLX->clDropDownText,buf);
				if (stripped && selected)  {
					int x1 = iX+4+item->tImage.get()->w;
					int y1 = y+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2);
					int x2 = iX+4+item->tImage.get()->w+tLX->cFont.GetWidth(item->sName)+4;
					if (x2 > bmpDest->w)  {
						x1 = bmpDest->w-x2-5;
						x2 = bmpDest->w-5;
					}
					int y2 = y1+tLX->cFont.GetHeight();

					DrawRect(bmpDest,x1-1,y1-1,x2,y2, tLX->clComboboxShowAllBorder);
					DrawRectFill(bmpDest,x1,y1,x2,y2, tLX->clComboboxShowAllMain);
					tLX->cFont.Draw(bmpDest, x1+2, y1-1 + (ItemHeight - tLX->cFont.GetHeight())/2, tLX->clDropDownText,item->sName);
				}
			}
			else  {
				stripped = stripdot(buf,iWidth - (3 + bGotScrollbar ? 15 : 0));
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
		if (getItemRW(iSelected))  {
			buf = getItemRW(iSelected)->sName;
			if (getItemRW(iSelected)->tImage.get())  {
				getItemRW(iSelected)->tImage->draw(bmpDest,iX+3,iY+1);
				stripdot(buf,iWidth - (8 + getItemRW(iSelected)->tImage.get()->w + bGotScrollbar ? 15 : 0));
				tLX->cFont.Draw(bmpDest, iX+8+getItemRW(iSelected)->tImage.get()->w, iY+2+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2), tLX->clDropDownText, buf);
			}
			else  {
				stripdot(buf,iWidth - (3 + bGotScrollbar ? 15 : 0));
				tLX->cFont.Draw(bmpDest, iX+3, iY+2, tLX->clDropDownText, buf);
			}
		}

		iHeight = ItemHeight+3;
	}

	// Button
	int x=0;
	if(bArrowDown)
		x = 15;
	DrawImageAdv(bmpDest, gfxGUI.bmpScrollbar, x,14, iX+iWidth-16,iY+2+(ItemHeight-2-tLX->cFont.GetHeight())/2, 15,14);

	if(!bFocused)  {
		bDropped = false;
		iKeySelectedItem = -1;
	}

	bArrowDown = false;
}










static inline int compare_items(const cb_item_t& item1, const cb_item_t& item2) {
	// Swap the two items?
	bool failed1,failed2;
	int nat_cmp1 = from_string<int>(item1.sName, failed1);
	int nat_cmp2 = from_string<int>(item2.sName, failed2);
	
	// First try, if we compare numbers
	if (!failed1 && !failed2)  {
		if(nat_cmp1 == nat_cmp2)
			return item1.sName.size() - item2.sName.size(); // because from_string("123456") == from_string("123456abcd")
		else
			return nat_cmp1 - nat_cmp2;
	// String comparison
	} else {
		return stringcasecmp(item1.sName, item2.sName);
	}
}


static inline bool less_items(const cb_item_t& item1, const cb_item_t& item2)  {
	return compare_items(item1, item2) < 0;
}


static inline bool greater_items(const cb_item_t& item1, const cb_item_t& item2)  {
	return compare_items(item1, item2) > 0;
}

static inline bool equal_items(const cb_item_t& item1, const cb_item_t& item2) {
	return compare_items(item1, item2) == 0;
}

bool operator<(const cb_item_t& item1, const cb_item_t& item2) { return less_items(item1, item2); }
bool operator>(const cb_item_t& item1, const cb_item_t& item2) { return greater_items(item1, item2); };

//////////////////////
// Sorts te items in the combobox
// HINT: private
void CCombobox::Sort(bool ascending)
{
#if !defined(_MSC_VER) || _MSC_VER > 1200
	if (ascending)
		tItems.sort(less_items);
	else
		tItems.sort(greater_items);
#else
	// A very dirty workaround for msvc 6
	tItems.sort();
	if (!ascending)
		tItems.reverse();

#endif
}

///////////////////////
// Removes duplicate entries (based on sName)
// HINT: private
void CCombobox::Unique() {
	if (tItems.size() < 2)
		return;

	std::list<cb_item_t>::iterator new_end;
	if (iSortDirection == SORT_NONE)  {
		new_end = std::unique(tItems.begin(), tItems.end(), equal_items);
	} else {
		std::list<cb_item_t>::iterator cur, next;
		next = new_end = cur = tItems.begin();
		next++;
		while (next != tItems.end())  {
			if (!equal_items(*cur, *next))  {
				*new_end = *cur;
				new_end++;
			}

			cur = next;
			next++;
		}
	}
	tItems.erase(new_end, tItems.end());

    cScrollbar.setMax( (int)tItems.size() );	
	bGotScrollbar = tItems.size() > 6;
}

////////////////////////
// Enables/disables duplicate removing
// Affects both existing and future items
void CCombobox::setUnique(bool _u)
{
	if (!bUnique && _u)
		Unique();
	bUnique = _u;
}

////////////////////////
// Enables/disables combobox sorting
// Affects both existing and future items
void CCombobox::setSorted(int sort_direction)
{
	if (iSortDirection != sort_direction && sort_direction != SORT_NONE)
		Sort(sort_direction == SORT_ASC);
	iSortDirection = sort_direction;
}

///////////////////
// Create the combo box
void CCombobox::Create()
{
	iSelected = 0;
	bGotScrollbar = false;
	bDropped = false;
	bArrowDown = false;
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
void CCombobox::Destroy()
{
	tItems.clear();
	iSelected = 0;

	// Destroy the scrollbar
	cScrollbar.Destroy();
}


///////////////////
// Mouse over event
int CCombobox::MouseOver(mouse_t *tMouse)
{
	if(tMouse->X >= iX+iWidth-16 && bGotScrollbar && bDropped)
		cScrollbar.MouseOver(tMouse);

	return CMB_NONE;
}


///////////////////
// Mouse down event
int CCombobox::MouseDown(mouse_t *tMouse, int nDown)
{
	bArrowDown = false;

	if((tMouse->X >= iX+iWidth-16 || cScrollbar.getGrabbed()) && bGotScrollbar && bDropped) {
		cScrollbar.MouseDown(tMouse, nDown);
		return CMB_NONE;
	}

	if(tMouse->X >= iX && tMouse->X <= iX+iWidth)
		if(tMouse->Y >= iY && tMouse->Y < iY+iHeight) {

            //
            // If we aren't dropped, shift the scroll bar
            //
            if(!bDropped) {
                if(iSelected >= 0 && (size_t)iSelected < tItems.size()) {
					// Setup the scroll bar so it shows this item in the middle
					cScrollbar.setValue( iSelected - cScrollbar.getItemsperbox() / 2 );
                }
            }


            // Drop or close it
			iNow = GetTime();
			if (tMouse->FirstDown)  {
				if (!bDropped)  {  // Not dropped, drop it
					bArrowDown = true;
					bDropped = true;
					iKeySelectedItem = -1;
					iDropTime = iNow;
				} else {
					// If clicked the arrow or body again, close the combobox
					int mainbitheight = MAX(tLX->cFont.GetHeight()+1, 16);
					if (tMouse->Y < iY + mainbitheight)  {
						bDropped = false;
						Menu_redrawBufferRect(iX, iY, iWidth, iHeight);
					}
				}
			}

			if (iNow-iDropTime <= 20 && iDropTime != AbsTime() && (tMouse->Y < iY+20))  // let the arrow pushed a bit longer
				bArrowDown = true;


		}

	return CMB_NONE;
}

int CCombobox::getItemHeight() {
	int ItemHeight = tLX->cFont.GetHeight() + 1;
	if(!tItems.empty())
		if(tItems.begin()->tImage.get())
			if ((tItems.begin()->tImage.get()->h + 1) > ItemHeight)
				ItemHeight = MAX(ItemHeight, tItems.begin()->tImage.get()->h + 1);
	return ItemHeight;
}

///////////////////
// Mouse up event
int CCombobox::MouseUp(mouse_t *tMouse, int nDown)
{
	bArrowDown = false;

	if(tMouse->X >= iX+iWidth-16 && bGotScrollbar && bDropped) {
		cScrollbar.MouseUp(tMouse, nDown);
		return CMB_NONE;
	}

	// Count the item height
	int ItemHeight = getItemHeight();

	// Go through the items checking for a mouse click
	int y = iY+ItemHeight+4;
	int w = iX+iWidth-1;
	if(bGotScrollbar)
		w -= 16;

	int index = 0;
	// TODO: this loop is just unneeded here, remove it
	for(std::list<cb_item_t>::const_iterator item = tItems.begin(); item != tItems.end(); item++, index++) {
		if(index < cScrollbar.getValue())
			continue;

		if(tMouse->X > iX && tMouse->X < w)
			if(tMouse->Y >= y && tMouse->Y < y + ItemHeight)
				if(tMouse->Up & SDL_BUTTON(1)) {
					iSelected = index;
					bDropped = false;
					return CMB_CHANGED;
				}


		y += ItemHeight;
		if(y > iY+iHeight)
			break;
	}


	return CMB_NONE;
}

bool CCombobox::selectNext() {
	if ((size_t)iSelected + 1 < tItems.size()) {
		iSelected++;
		return true;
	}
	return false;
}

bool CCombobox::selectPrev() {
	if(iSelected > 0) {
		iSelected--;
		return true;
	}
	return false;
}

///////////////////
// Mouse wheel down event
int CCombobox::MouseWheelDown(mouse_t *tMouse)
{
	if(bGotScrollbar && bDropped)
		cScrollbar.MouseWheelDown(tMouse);

	if(!bDropped)  {
		if(selectNext())
			return CMB_CHANGED;
	}

	return CMB_NONE;
}


///////////////////
// Mouse wheel up event
int CCombobox::MouseWheelUp(mouse_t *tMouse)
{
	if(bGotScrollbar && bDropped)
		cScrollbar.MouseWheelUp(tMouse);

	if(!bDropped)  {
		if(selectPrev())
			return CMB_CHANGED;
	}

	return CMB_NONE;
}

int CCombobox::findItem(UnicodeChar startLetter) {
	if(startLetter <= 31) return -1;
	
	bool letterAlreadySelected = false;
	int first = -1; // save first found index here
	int index = 0; // current index for loop
	startLetter = UnicodeToLower(startLetter);
	for(std::list<cb_item_t>::const_iterator it = tItems.begin(); it != tItems.end(); it++, index++) {
		UnicodeChar c = GetUnicodeFromUtf8(it->sName, 0);
		c = UnicodeToLower(c);
		if(c == startLetter) {
			if(first < 0) first = index;
			if(index > iSelected && letterAlreadySelected) return index;
			if(index == iSelected) letterAlreadySelected = true;
		}
	}
	
	return first;
}


//////////////////
// Key down event
int CCombobox::KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)
{
#if 0 /* This will disallow to navigate menu with arrow keys */
	// Search for items by pressed key
	if (!bCanSearch)
		return CMB_NONE;

	// TODO: why?
	bCanSearch = false;

	int index = findItem(c);
	if(index >= 0) {
		iSelected = index;
		iKeySelectedItem = index;
		cScrollbar.setValue( index - cScrollbar.getItemsperbox() / 2 );
		return CMB_CHANGED;
	}
#endif
	// TODO: this doesn't work as expected atm if the mouse is over
	// Handle key up/down
	if (bDropped)  {
		if (keysym == SDLK_DOWN)  {
			if (selectNext())  {
				// Move the scrollbar if necessary
				cScrollbar.setValue( iSelected - cScrollbar.getItemsperbox() / 2 );
				iKeySelectedItem = iSelected;
				PlaySoundSample(sfxGeneral.smpClick);
			}
			return CMB_CHANGED;
		} else
		if (keysym == SDLK_PAGEDOWN) {
			bool changed = false;
			for (int count = 0; count < 10; count++) {
				if (selectNext())  {
					// Move the scrollbar if necessary
					cScrollbar.setValue( iSelected - cScrollbar.getItemsperbox() / 2 );
					iKeySelectedItem = iSelected;
					changed = true;
				}
			}
			if (changed)
				PlaySoundSample(sfxGeneral.smpClick);
			return CMB_CHANGED;
		} else
		if (keysym == SDLK_UP)  {
			if (selectPrev())  {
				// Move the scrollbar if necessary
				cScrollbar.setValue( iSelected - cScrollbar.getItemsperbox() / 2 );
				iKeySelectedItem = iSelected;
				PlaySoundSample(sfxGeneral.smpClick);
			}
			return CMB_CHANGED;
		} else
		if (keysym == SDLK_PAGEUP) {
			bool changed = false;
			for (int count = 0; count < 10; count++) {
				if (selectPrev())  {
					// Move the scrollbar if necessary
					cScrollbar.setValue( iSelected - cScrollbar.getItemsperbox() / 2 );
					iKeySelectedItem = iSelected;
					changed = true;
				}
			}
			if (changed)
				PlaySoundSample(sfxGeneral.smpClick);
			return CMB_CHANGED;
		} else
		if (keysym == SDLK_RETURN ||
			keysym == SDLK_KP_ENTER ||
			keysym == SDLK_LALT ||
			keysym == SDLK_LCTRL ||
			keysym == SDLK_LSHIFT ||
			keysym == SDLK_x ||
			keysym == SDLK_z) {
			bDropped = false;
			PlaySoundSample(sfxGeneral.smpClick);
			return CMB_CHANGED;
		}
	} else {
		if (keysym == SDLK_RETURN ||
			keysym == SDLK_KP_ENTER ||
			keysym == SDLK_LALT ||
			keysym == SDLK_LCTRL ||
			keysym == SDLK_LSHIFT ||
			keysym == SDLK_x ||
			keysym == SDLK_z) {
			bDropped = true;
			cScrollbar.setValue( iSelected - cScrollbar.getItemsperbox() / 2 );
			iKeySelectedItem = iSelected;
			PlaySoundSample(sfxGeneral.smpClick);
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
			return iSelected;
			break;

		// Get the current item
		case CBM_GETCURITEM:
			return (DWORD)getItemRW(iSelected); // TODO: 64bit unsafe

		// Set the current item
		case CBM_SETCURSEL:
			setCurItem(Param1);
			break;

        // Set the current item based on the int index
        case CBM_SETCURINDEX:
            setCurItem(Param1);
            break;

		// Return true, if the combobox is dropped
		case CBM_ISDROPPED:
			return (int)bDropped;
			break;

		// Set the sorted property
		case CBM_SETSORTED:
			setSorted(Param1);
			break;

		// Set the unique property
		case CBM_SETUNIQUE:
			setUnique(Param1 != 0);
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
		addItem(0, sStr, *((std::string *) Param)); // TODO: 64bit unsafe
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
		if (getItemRW(iSelected))  {
			*sStr = getItemRW(iSelected)->sIndex;
			return 1;
		}
		else  {
			*sStr = "";
			return 0;
		}
		break;

	// Get the current item's name
	case CBS_GETCURNAME:
		if (getItemRW(iSelected))  {
			*sStr = getItemRW(iSelected)->sName;
			return 1;
		} else {
			*sStr = "";
			return 0;
		}
		break;
	}

	return 0;
}


int CCombobox::addItem(const std::string& sindex, const std::string& name, const SmartPointer<DynDrawIntf>& img, int tag)
{
	return addItem(-1, sindex, name, img, tag);
}

////////////////////////
// Returns the lower bound iterator for the item
std::list<cb_item_t>::iterator CCombobox::lowerBound(const cb_item_t& item, int *index, bool *equal)
{
	if (tItems.size() == 0) return tItems.end();

	size_t n = tItems.size();
	std::list<cb_item_t>::iterator result = tItems.begin();
	*equal = false;
	*index = 0;

	// HINT: check if we're lucky and the last item is the lower bound (this is surprisingly pretty often when adding items)
	{
		const int res = compare_items(*tItems.rbegin(), item);
		if (res <= 0)  {
			result = tItems.end();
			*index = (int)tItems.size();
			*equal = (res == 0);
			return result;
		}
	}

	// Binary search
	while (n)  {
		size_t mid = n / 2;
		std::list<cb_item_t>::iterator mid_value = result;
		std::advance(mid_value, mid);
		const int res = compare_items(*mid_value, item);
		if (res < 0)  {
			result = ++mid_value;
			++mid;
			n -= mid;
			*index += (int)mid;
		} else if (res > 0)
			n = mid;
		else  { // Exact match
			*index += (int)mid;
			*equal = true;
			return mid_value;
		}
	}

	return result;
}

////////////////////////
// Returns the upper bound iterator for the item
std::list<cb_item_t>::iterator CCombobox::upperBound(const cb_item_t& item, int *index, bool *equal)
{ 
	if (tItems.size() == 0) return tItems.end();

	size_t n = tItems.size();
	std::list<cb_item_t>::iterator result = tItems.begin();
	*equal = false;
	*index = 0;

	// HINT: check if we're lucky and the first item is the upper bound (this is surprisingly pretty often when adding items)
	{
		const int res = compare_items(*tItems.begin(), item);
		if (res <= 0)  {
			result = tItems.begin();
			*index = 0;
			*equal = (res == 0);
			return result;
		}
	}

	// Binary search
	while (n)  {
		size_t mid = n / 2;
		std::list<cb_item_t>::iterator mid_value = result;
		std::advance(mid_value, mid);
		const int res = compare_items(*mid_value, item);
		if (res > 0)  {
			result = ++mid_value;
			++mid;
			n -= mid;
			*index += (int)mid;
		} else if (res < 0)
			n = mid;
		else  { // Exact match
			*index += (int)mid;
			*equal = true;
			return mid_value;
		}
	}

	return result;
}

///////////////////
// Add an item to the combo box
int CCombobox::addItem(int index, const std::string& sindex, const std::string& name, const SmartPointer<DynDrawIntf>& img, int tag)
{
	cb_item_t item;

	// Fill in the info
	item.sIndex = sindex;
	item.sName = name;
	item.tImage = img;
	item.iTag = tag;

	//
	// Add it to the list
	//

	// First item
	if (tItems.size() == 0)  {
		tItems.push_back(item);
		index = 0;

	// Not first item
	} else switch (iSortDirection)  {

	// Ascending sorting
	case SORT_ASC:  {
		bool equal;
		std::list<cb_item_t>::iterator it = lowerBound(item, &index, &equal);

		if (!bUnique || !equal)
			tItems.insert(it, item);
	} break;

	// Descending sorting
	case SORT_DESC:  {
		bool equal;
		std::list<cb_item_t>::iterator it = upperBound(item, &index, &equal);

		if (!bUnique || !equal)  {
			tItems.insert(it, item);
		}
	} break;


	// Not sorted
	default:
		if (bUnique)
			if (getItem(item.sName))
				return 0;
	
		if(index >= 0 && (size_t)index < tItems.size()) {
			std::list<cb_item_t>::iterator it = tItems.begin();
			std::advance(it, index);
			tItems.insert(it, item);
		} else {
			index = (int)tItems.size();
			tItems.push_back(item);
		}
	}

	// current selection invalid
	if (iSelected < 0 || (size_t)iSelected >= tItems.size())  {
		// select this item
		iSelected = index;
	}

    cScrollbar.setMax( (int)tItems.size() );	
	bGotScrollbar = tItems.size() > 6;

	return index;
}


///////////////////
// Set the current item based on count
void CCombobox::setCurItem(int index)
{
	iSelected = index;
}

///////////////////
// Set the current item based on item pointer
void CCombobox::setCurItem(const cb_item_t *it)
{
	if (it == NULL)
		return;

	setCurItem( getItemIndex(it) );
}


///////////////////
// Set the current item based on string index
void CCombobox::setCurSIndexItem(const std::string& szString)
{
	int index = getIndexBySIndex(szString);
	if(index >= 0)
		iSelected = index;
	else
		warnings << "setCurSIndexItem: could not set the current item, sIndex '" << szString << "' not found in the list" << endl;
}

void CCombobox::setCurItemByName(const std::string& szString)
{
	int index = getIndexByName(szString);
	if(index >= 0)
		iSelected = index;
	else
		warnings << "setCurItemByName: could not set the current item, sName '" << szString << "' not found in the list" << endl;
}

int CCombobox::getIndexByName(const std::string& szString) {
	if (iSortDirection == SORT_NONE)  {
		int index = 0;
		for(std::list<cb_item_t>::const_iterator i = tItems.begin(); i != tItems.end(); i++, index++) {
			if( stringcasecmp(i->sName, szString) == 0 ) {
				return index;
			}
		}
	} else {
		cb_item_t tmp; tmp.sName = szString; tmp.tImage = NULL;
		int index = -1;
		bool found = false;
		lowerBound(tmp, &index, &found);
		return found ? index : -1;
	}
    return -1;
}

int CCombobox::getIndexBySIndex(const std::string& szString) {
	int index = 0;
	for(std::list<cb_item_t>::const_iterator i = tItems.begin(); i != tItems.end(); i++, index++) {
        if( stringcasecmp(i->sIndex, szString) == 0 ) {
            return index;
        }
    }
    return -1;
}


///////////////////
// Set the image for the specified item
void CCombobox::setImage(const SmartPointer<DynDrawIntf>& img, int ItemIndex)
{
	cb_item_t* item = getItemRW(ItemIndex);
	if(item) item->tImage = img;
}


///////////////////
// Clear the data
void CCombobox::clear()
{	
    Destroy();
    Create();
}


///////////////
// Get the item based on its index property
const cb_item_t* CCombobox::getItem(int index) const
{
	if(index < 0 || (size_t)index >= tItems.size()) return NULL;
	std::list<cb_item_t>::const_iterator it = tItems.begin();
	std::advance(it, index);
	return &*it;
}

cb_item_t* CCombobox::getItemRW(int index)
{
	if(index < 0 || (size_t)index >= tItems.size()) return NULL;
	std::list<cb_item_t>::iterator it = tItems.begin();
	std::advance(it, index);
	return &*it;
}

/////////////
// Get the number of items
int	CCombobox::getItemsCount() {
	return (int)tItems.size();
}

/////////////
// Get the item based on its displayed name
const cb_item_t* CCombobox::getItem(const std::string& name) const {
	for(std::list<cb_item_t>::const_iterator it = tItems.begin(); it != tItems.end(); it++) {
		if(stringcasecmp(it->sName, name) == 0)
			return &*it;
	}

	return NULL;
}

/////////////
// Get the item based on its string index
const cb_item_t* CCombobox::getSIndexItem(const std::string& sIndex) const {
	// TODO: make it faster by using a sorted list (or map)
	for(std::list<cb_item_t>::const_iterator it = tItems.begin(); it != tItems.end(); it++) {
		if(stringcasecmp(it->sIndex, sIndex) == 0)
			return &*it;
	}
	return NULL;
}

int CCombobox::getSelectedIndex() 
{ 
	return iSelected;
}

const cb_item_t* CCombobox::getLastItem() {
	if(tItems.empty())
		return NULL;
	else
		return &*tItems.rbegin();
}

const cb_item_t* CCombobox::getSelectedItem() {
	return getItem(iSelected);
}

int CCombobox::getItemIndex(const cb_item_t* item) {
	int index = 0;
	for(std::list<cb_item_t>::iterator it = tItems.begin(); it != tItems.end(); it++, index++) {
		if(&*it == item)
			return index;
	}
	
	return -1;
}

}; // namespace DeprecatedGUI
