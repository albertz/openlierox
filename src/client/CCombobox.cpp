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
	int ItemHeight = getItemHeight();
	
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
		if(getItemRW(iSelected))  {
			buf = getItemRW(iSelected)->sName;
			if (getItemRW(iSelected)->tImage)  {
				DrawImage(bmpDest,getItemRW(iSelected)->tImage,iX+3,iY+1);
				stripdot(buf,iWidth-(6+getItemRW(iSelected)->tImage->w+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+6+getItemRW(iSelected)->tImage->w, iY+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2), tLX->clDisabled, buf);
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
		if (tItems.size() < (size_t)display_count)
			display_count = tItems.size();
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

			if (item->tImage)  {
				// Draw the image
				DrawImage(bmpDest,item->tImage,iX+3,y);
				stripped = stripdot(buf,iWidth-(6+getItemRW(iSelected)->tImage->w+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+6+item->tImage->w, y, tLX->clDropDownText,buf);
				if (stripped && selected)  {
					int x1 = iX+4+getItemRW(iSelected)->tImage->w;
					int y1 = y+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2);
					int x2 = iX+4+getItemRW(iSelected)->tImage->w+tLX->cFont.GetWidth(item->sName)+4;
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
		if (getItemRW(iSelected))  {
			buf = getItemRW(iSelected)->sName;
			if (getItemRW(iSelected)->tImage)  {
				DrawImage(bmpDest,getItemRW(iSelected)->tImage,iX+3,iY+1);
				stripdot(buf,iWidth-(6+getItemRW(iSelected)->tImage->w+iGotScrollbar*15));
				tLX->cFont.Draw(bmpDest, iX+6+getItemRW(iSelected)->tImage->w, iY+(ItemHeight/2)-(tLX->cFont.GetHeight() / 2), tLX->clDropDownText, buf);
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

class comboorder  {

	bool ascending;
	comboorder(bool asc) : ascending(asc) {}
	
	// less operation, item1 < item2
	bool less(const cb_item_t& item1, const cb_item_t& item2) {
		// Swap the two items?
		bool failed1,failed2;
		int nat_cmp1 = from_string<int>(item1.sName, failed1);
		int nat_cmp2 = from_string<int>(item2.sName, failed2);
		
		// First try, if we compare numbers
		if (!failed1 && !failed2)  {
			return nat_cmp1 < nat_cmp2;
		// String comparison
		} else {
			return 0 > stringcasecmp(item1.sName, item2.sName);
		}
	}
	
	bool operator()(const cb_item_t& item1, const cb_item_t& item2) {
		if(ascending)
			return less(item1, item2);
		else
			return less(item2, item1);
	}
};

//////////////////////
// Sorts te items in the combobox
void CCombobox::Sort(bool ascending)
{
	tItems.sort();
}


///////////////////
// Create the combo box
void CCombobox::Create(void)
{
	iSelected = 0;
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
	tItems.clear();
	iSelected = 0;

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
                int index = 0;
                for(std::list<cb_item_t>::const_iterator i = tItems.begin(); i != tItems.end(); i++, index++) {
                    if(index == iSelected) {
                        // Setup the scroll bar so it shows this item in the middle
                        cScrollbar.setValue( index - cScrollbar.getItemsperbox() / 2 );
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

int CCombobox::getItemHeight() {
	int ItemHeight = tLX->cFont.GetHeight() + 1;
	if(!tItems.empty())
		if(tItems.begin()->tImage)
			if ((tItems.begin()->tImage->h + 1) > ItemHeight)
				ItemHeight = tItems.begin()->tImage->h + 1;
	return ItemHeight;
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
	int ItemHeight = getItemHeight();

	// Go through the items checking for a mouse click
	int y = iY+tLX->cFont.GetHeight()+4;
	int w = iX+iWidth-1;
	if(iGotScrollbar)
		w-=16;

	int index = 0;
	for(std::list<cb_item_t>::const_iterator item = tItems.begin(); item != tItems.end(); item++, index++) {
		if(index < cScrollbar.getValue())
			continue;

		if(tMouse->X > iX && tMouse->X < w)
			if(tMouse->Y >= y && tMouse->Y < y + ItemHeight)
				if(tMouse->Up & SDL_BUTTON(1)) {
					iSelected = index;
					iDropped = false;
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
	if(iGotScrollbar && iDropped)
		cScrollbar.MouseWheelDown(tMouse);

	if(!iDropped)  {
		if(selectNext())
			return CMB_CHANGED;
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
int CCombobox::KeyDown(UnicodeChar c, int keysym)
{
	// Search for items by pressed key
	if (!iCanSearch)
		return CMB_NONE;

	// TODO: why?
	iCanSearch = false;

	int index = findItem(c);
	if(index >= 0) {
		iSelected = index;
		iKeySelectedItem = index;
		cScrollbar.setValue( index - cScrollbar.getItemsperbox() / 2 );
		return CMB_CHANGED;
	}

	// TODO: handle up/down keys

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
			return (DWORD)getItemRW(iSelected);

		// Set the current item
		case CBM_SETCURSEL:
			setCurItem(Param1);
			break;

        // Set the current item based on the int index
        case CBM_SETCURINDEX:
            setCurItem(Param1);
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


int CCombobox::addItem(const std::string& sindex, const std::string& name)
{
	return addItem(-1, sindex, name);
}

///////////////////
// Add an item to the combo box
int CCombobox::addItem(int index, const std::string& sindex, const std::string& name)
{
	cb_item_t item;

	// Fill in the info
	item.sIndex = sindex;
	item.sName = name;
	item.tImage = NULL;

	if(bUnique && getSIndexItem(sindex) != NULL) return -1;
	
	//
	// Add it to the list
	//
	if(index >= 0 && (size_t)index < tItems.size()) {
		int i = 0;
		for(std::list<cb_item_t>::iterator it = tItems.begin(); it != tItems.end(); ++it, ++i) {
			if(i == index) {
				tItems.insert(it, item);
				break;
			}
		}
	} else {
		tItems.push_back(item);
		index = tItems.size() - 1;
	}

	// current selection invalid
	if (iSelected < 0 || (size_t)iSelected >= tItems.size())  {
		// select this item
		iSelected = index;
	}

	// List should be automatically sorted when adding
	if (bSorted)  {
		// TODO: do this faster
		// (update also Menu_Player_FillSkinCombo after)
		Sort(true);
	}

    cScrollbar.setMax( tItems.size() );
	
	iGotScrollbar = tItems.size() > 6;

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
	int index = 0;
	for(std::list<cb_item_t>::const_iterator i = tItems.begin(); i != tItems.end(); i++, index++) {
        if( stringcasecmp(i->sIndex, szString) == 0 ) {
            iSelected = index;
            return;
        }
    }
}

///////////////////
// Set the image for the specified item
void CCombobox::setImage(SDL_Surface *img, int ItemIndex)
{
	cb_item_t* item = getItemRW(ItemIndex);
	if(item) item->tImage = img;
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
const cb_item_t* CCombobox::getItem(int index) const
{
	if(index < 0 || (size_t)index >= tItems.size()) return NULL;
	std::list<cb_item_t>::const_iterator it = tItems.begin();
	for(int i = 0; i < index; i++, it++) {}
	return &*it;
}

cb_item_t* CCombobox::getItemRW(int index)
{
	if(index < 0 || (size_t)index >= tItems.size()) return NULL;
	std::list<cb_item_t>::iterator it = tItems.begin();
	for(int i = 0; i < index; i++, it++) {}
	return &*it;
}

/////////////
// Get the number of items
int	CCombobox::getItemsCount() {
	return tItems.size();
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


static bool CComboBox_WidgetRegistered = 
	CGuiSkin::RegisterWidget( "combobox", & CCombobox::WidgetCreator )
							( "items", CGuiSkin::WVT_STRING )
							( "var", CGuiSkin::WVT_STRING )
							( "click", CGuiSkin::WVT_STRING );

CWidget * CCombobox::WidgetCreator( const std::vector< CGuiSkin::WidgetVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy )
{
	CCombobox * w = new CCombobox();
	w->cClick.Init( p[2].s, w );
	layout->Add( w, id, x, y, dx, dy );
	// Items should be added to combobox AFTER the combobox is added to CGuiSkinnedLayout
	std::vector<std::string> items = explode( p[0].s, "," );
	w->iVar = CGuiSkin::GetVar( p[1].s, CGuiSkin::SVT_INT ).i;	// If combobox is int or string determined by attached var type
	if( w->iVar )
	{
		for( unsigned i = 0; i < items.size(); i++ )
		{
			std::string item = items[i];
			int index = i;
			if( item.find("#") != std::string::npos )
			{
				index = atoi( item.substr( item.find("#") + 1 ) );
				item = item.substr( 0, item.find("#") );
			};
			TrimSpaces(item);
			w->addItem( index, "", item );
		};
		w->setCurItem( *w->iVar );
	};
	w->sVar = CGuiSkin::GetVar( p[1].s, CGuiSkin::SVT_STRING ).s;
	if( w->sVar )
	{
		for( unsigned i = 0; i < items.size(); i++ )
		{
			std::string item = items[i];
			std::string index = item;
			if( item.find("#") != std::string::npos )
			{
				index = item.substr( item.find("#") + 1 );
				TrimSpaces( index );
				item = item.substr( 0, item.find("#") );
			};
			TrimSpaces(item);
			w->addItem( i, index, item );
		};
		w->setCurSIndexItem( *w->sVar );
	};
	return w;
};

void	CCombobox::ProcessGuiSkinEvent(int iEvent)
{
	if( iEvent == CMB_CHANGED )
	{
		if( iVar )
			*iVar = iSelected;
		if( sVar )
			*sVar = getItem( iSelected )->sIndex;
		cClick.Call();	// If this is "Select Skin" combobox the *this ptr may be destroyed here, so just return after this line
	};
};

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
