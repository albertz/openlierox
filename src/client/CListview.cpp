/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// gui Listview class
// Created 16/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"


///////////////////
// Draw the list view
void CListview::Draw(SDL_Surface *bmpDest)
{
	lv_subitem_t *sub;

    Menu_redrawBufferRect(iX,iY, iWidth,iHeight);
	

	// Draw the columns
	lv_column_t *col = tColumns;
	int x=iX+3;
		
	for(;col;col = col->tNext) {
		tLX->cFont.Draw(bmpDest, x, iY, 0xffff,"%s", col->sText);
		x += col->iWidth;
	}


	// Draw the items
	int y = iY+17;
	x = iX+4;
	lv_item_t *item = tItems;
	int count=0;
	
	int selectsize = x+iWidth-5;
	if(iGotScrollbar)
		selectsize = x+iWidth-20;

	for(;item;item = item->tNext) {
		if(count++ < cScrollbar.getValue())
			continue;

		int h = 18;
		x = iX+4;

		col = tColumns;

		// Find the max height
		h = item->iHeight;
		int texty = y + (h-18)/2;

		int colour = item->iColour;

		
		// Selected?
		if(item->iSelected && bShowSelect) {
			if(iFocused)
				DrawRectFill(bmpDest,x-2,y,selectsize,y+h-2,MakeColour(0,66,102));	
			else
				DrawRect(bmpDest,x-2,y,selectsize-1,y+h-2,MakeColour(0,66,102));	
		}

		// Draw the sub items
		if(item->tSubitems) {

			sub = item->tSubitems;
			col = tColumns;
			for(;sub;sub = sub->tNext) {

				if(sub->iVisible) {
					if(sub->iType == LVS_TEXT)
						tLX->cFont.Draw(bmpDest,x,texty,colour,"%s",sub->sText);
				
					if(sub->iType == LVS_IMAGE)
						DrawImage(bmpDest,sub->bmpImage,x,y);
				}

				if(col) {
					x += col->iWidth;
					col = col->tNext;
				}
			}
		}
			
		y+=h;
		if(y>=iY+iHeight-h)
			break;
	}

	// Draw the scrollbar
	if(iGotScrollbar)
		cScrollbar.Draw(bmpDest);

    // Draw the rectangle last
    Menu_DrawBoxInset(bmpDest, iX, iY+15, iX+iWidth, iY+iHeight);
}


///////////////////
// Add a column to the list view
void CListview::AddColumn(char *sText, int iWidth)
{
	lv_column_t *col;

	col = new lv_column_t;
	if(col == NULL) {
		// Out of memory
		return;
	}

	// Set the info
	strcpy(col->sText,sText);
	col->iWidth = iWidth;
	col->tNext = NULL;


	// Add it to the list
	if(tColumns) {
		lv_column_t *c = tColumns;
		for(;c;c = c->tNext) {
			if(c->tNext == NULL) {
				c->tNext = col;
				break;
			}
		}
	}
	else
		tColumns = col;
}


///////////////////
// Add an item to the list view
void CListview::AddItem(char *sIndex, int iIndex, int iColour)
{
	lv_item_t *item = new lv_item_t;
	
	if(item == NULL) {
		// Out of memory
		return;
	}

	// Set the info
	strcpy(item->sIndex,sIndex);
	item->iIndex = iIndex;
	item->tNext = NULL;
	item->iSelected = false;
	item->tSubitems = NULL;
	item->iHeight = 18;			// Text height
	item->iColour = iColour;
    item->_iID = iItemID++;

	// Add it to the list
	if(tItems) {
		lv_item_t *i = tItems;
		for(;i;i = i->tNext) {
			if(i->tNext == NULL) {
				i->tNext = item;
				break;
			}
		}
	}
	else {
		tItems = item;
		tSelected = item;
		tSelected->iSelected = true;
		//writeLog("Index: %d\n",tSelected->iIndex);
	}
	
	tLastItem = item;

	// Adjust the scrollbar
	iItemCount++;

	// Do we use a scrollbar?
	//if(cScrollbar.getMax()*20 >= iHeight)
	//	iGotScrollbar = true;

	// Readjust the scrollbar
	ReadjustScrollbar();
}


///////////////////
// Add a sub item to the last item
void CListview::AddSubitem(int iType, char *sText, SDL_Surface *img)
{
	lv_subitem_t *sub = new lv_subitem_t;

	if(sub == NULL) {
		// Out of memory
		return;
	}

	// Set the info
	sub->iType = iType;
	strcpy(sub->sText,sText);
	sub->bmpImage = NULL;
	sub->tNext = NULL;
	sub->iVisible = true;
	sub->iExtra = 0;
	if(iType == LVS_IMAGE)
		sub->bmpImage = img;


	// No last item
	if(!tLastItem) {
		delete sub;
		return;
	}


	// Add this sub item to the current item
	if(tLastItem->tSubitems) {
		lv_subitem_t *s = tLastItem->tSubitems;
		for(;s;s = s->tNext) {
			if(s->tNext == NULL) {
				s->tNext = sub;
				break;
			}
		}
	} else
		tLastItem->tSubitems = sub;



	// Find the max height of this item
	sub = tLastItem->tSubitems;
	for(;sub;sub = sub->tNext) {
		if(sub->iType == LVS_IMAGE) {
			tLastItem->iHeight = MAX(tLastItem->iHeight,(sub->bmpImage->h+4));
		}
	}

	// Readjust the scrollbar
	ReadjustScrollbar();
}


///////////////////
// Re-adjust the scrollbar
void CListview::ReadjustScrollbar(void)
{
	// Go through each item and find the average height
	lv_item_t *item = tItems;
	int count = 0;
	int size = 0;

	for(;item;item=item->tNext) {
		size += item->iHeight;
		count++;
	}

	// Buffer size on top & bottom
	size += 17;

	iGotScrollbar = false;
	if(size >= iHeight)
		iGotScrollbar = true;
	else
		cScrollbar.setValue(0);

	if(count == 0) {
		cScrollbar.setItemsperbox(0);
		cScrollbar.setValue(0);

		// Return to prevent a divide-by-zero
		return;
	}

	// Find the average
	size /= count;

	int h = iHeight / size;
    
	cScrollbar.setItemsperbox( h );
    cScrollbar.setMax(count);
    cScrollbar.setValue(0);

	//if(cScrollbar.getValue() + h > count)
		//cScrollbar.setValue(count-h+1);
}


///////////////////
// Remove an item from the list
void CListview::RemoveItem(int iIndex)
{
	lv_item_t *item = NULL;
	lv_item_t *prev = NULL;
	lv_item_t *i = tItems;
	lv_item_t *next = NULL;
	lv_subitem_t *s,*sub;
	int first = true;
	int found = false;

	// Find the item and it's previous item	
	prev = i;	
	for(;i;i=i->tNext) {

		if(i->iIndex == iIndex) {
			found=true;
			next = i->tNext;

			// If it's the first item, we do it differently
			if(first) {

				// Free the sub items
				for(s=i->tSubitems;s;s=sub) {
					sub = s->tNext;
					delete s;
				}
				delete i;

				// Set the first one to point to the next in the list
				tItems = next;
			} else {
				// Free the sub items
				for(s=i->tSubitems;s;s=sub) {
					sub = s->tNext;
					delete s;
				}
				delete i;

				// The previous item now points to the next one
				prev->tNext = next;
			}
			i = next;
			iItemCount--;
			//break;			
		}

		if(!i)
			break;
		
		prev = i;
		first=false;
	}

	// Find the last item
	i=tItems;
	tLastItem = NULL;
	for(;i;i=i->tNext) {
		if(i->tNext == NULL)
			tLastItem = i;
	}

	tSelected = tItems;
	if(tSelected)
		tSelected->iSelected = true;

	// Adjust the scrollbar
	//cScrollbar.setMax( cScrollbar.getMax()-1 );	

	/*// Do we use a scrollbar?
	if(cScrollbar.getMax()*20 >= iHeight)
		iGotScrollbar = true;
	else
		iGotScrollbar = false;*/

	// Readjust the scrollbar
	ReadjustScrollbar();
}


///////////////////
// Get the first sub item from the currently selected item
lv_subitem_t *CListview::getCurSub(void)
{
	if(tSelected)
		return tSelected->tSubitems;

	return NULL;
}


///////////////////
// Clear the items
void CListview::Clear(void)
{
	// Free the items
	lv_item_t *i,*item;
	lv_subitem_t *s,*sub;
	for(i=tItems;i;i=item) {
		item = i->tNext;

		// Free the sub items
		for(s=i->tSubitems;s;s=sub) {
			sub = s->tNext;

			delete s;
		}

		delete i;
	}

	tItems = NULL;

	cScrollbar.setMin(0);
	cScrollbar.setMax(1);
	cScrollbar.setValue(0);
	iItemCount=0;
    iItemID = 0;
	iGotScrollbar=false;
}


///////////////////
// Create event
void CListview::Create(void)
{
	// Destroy any previous settings
	Destroy();
	iItemCount=0;
    iItemID = 0;
	iGotScrollbar=false;
    bShowSelect = true;

	cScrollbar.Create();
	cScrollbar.Setup(0, iX+iWidth-16, iY+17, 14, iHeight-18);
	cScrollbar.setItemsperbox( iHeight / 18 );
	cScrollbar.setMin(0);
	cScrollbar.setMax(1);
	cScrollbar.setValue(0);
}


///////////////////
// Destroy event
void CListview::Destroy(void)
{
	// Free the columns
	lv_column_t *c,*col;
	for(c=tColumns ; c ; c=col) {
		col = c->tNext;

		delete c;
	}
	tColumns = NULL;


	// Free the items
	lv_item_t *i,*item;
	lv_subitem_t *s,*sub;
	for(i=tItems;i;i=item) {
		item = i->tNext;

		// Free the sub items
		for(s=i->tSubitems;s;s=sub) {
			sub = s->tNext;

			delete s;
		}

		delete i;
	}

	tItems = NULL;
}


///////////////////
// Get an index based on item count
int CListview::getIndex(int count)
{
	lv_item_t *item = tItems;
	int i;

	for(i=0; item; item=item->tNext, i++) {

		if(i==count)
			return item->iIndex;
	}

	return -1;
}




///////////////////
// Mouse over event	
int	CListview::MouseOver(mouse_t *tMouse)
{
	if(tMouse->X > iX+iWidth-20 && iGotScrollbar)
		cScrollbar.MouseOver(tMouse);

	return -1;
}


///////////////////
// Mouse down event
int	CListview::MouseDown(mouse_t *tMouse, int nDown)
{
	if(tMouse->X > iX+iWidth-20 && iGotScrollbar) {
		cScrollbar.MouseDown(tMouse, nDown);
		return LV_NONE;
	}

	if(tMouse->X < iX || tMouse->X > iX+iWidth-18)
		return LV_NONE;

    if( !(tMouse->FirstDown & SDL_BUTTON(1)) )
        return LV_NONE;
    
	iClickedSub = -1;

	// Go through the items
	int y = iY+17;
	lv_item_t *item = tItems;
	int count=0;
	for(;item;item = item->tNext) {
		if(count++ < cScrollbar.getValue())
			continue;

		// Find the max height
		int h = item->iHeight;

		if(tMouse->Y > y && tMouse->Y < y+h) {

			int event = LV_CHANGED;

			// Go through the sub items, check which one was clicked on
			lv_subitem_t *sub = item->tSubitems;
			lv_column_t *col = tColumns;
			int x = iX;
			int i=0;			
			for(;sub;sub=sub->tNext,i++) {

				if(sub->iVisible) {
					if(col) {
						if(tMouse->X > x && tMouse->X < x+col->iWidth)
							iClickedSub = i;
					}
				}

				if(col) {
					x += col->iWidth;
					col = col->tNext;
				}
			}

            if(tSelected) {
				tSelected->iSelected = false;

                // If we changed the selection, reset the mouseup var to avoid double clicks
                // when changing items
                if( tSelected->_iID != item->_iID )                    
                    fLastMouseUp = -9999;
            } else
                fLastMouseUp = -9999;


			tSelected = item;
			tSelected->iSelected = true;

			return event;
		}

		y+=h;
		if(y>=iY+iHeight-h)
			break;
	}

	return LV_NONE;
}


///////////////////
// Mouse up event
int	CListview::MouseUp(mouse_t *tMouse, int nDown)
{
	if(tMouse->X > iX+iWidth-20 && iGotScrollbar) 
		cScrollbar.MouseDown(tMouse, nDown);

	if(tMouse->X < iX || tMouse->X > iX+iWidth-18) {
		fLastMouseUp = -9999;
		return LV_NONE;	
	}

	iClickedSub = -1;

	// Go through the items
	int y = iY+17;
	lv_item_t *item = tItems;
	int count=0;
	for(;item;item = item->tNext) {
		if(count++ < cScrollbar.getValue())
			continue;

		// Find the max height
		int h = item->iHeight;

		if(tMouse->Y > y && tMouse->Y < y+h) {

			int event = LV_CHANGED;

            // Right click?
            if( tMouse->Up & SDL_BUTTON(3) )
                event = LV_RIGHTCLK;

			if(tSelected && (tMouse->Up & SDL_BUTTON(1))) {
				if(tSelected->_iID == item->_iID) {
					if(tLX->fCurTime - fLastMouseUp < 0.5f) {
						event = LV_DOUBLECLK;
						fLastMouseUp = -9999;
					}
				}
			}

			// Go through the sub items, check which one was clicked on
			lv_subitem_t *sub = item->tSubitems;
			lv_column_t *col = tColumns;
			int x = iX;
			int i=0;			
			for(;sub;sub=sub->tNext,i++) {

				if(sub->iVisible) {
					if(col) {
						if(tMouse->X > x && tMouse->X < x+col->iWidth)
							iClickedSub = i;
					}
				}

				if(col) {
					x += col->iWidth;
					col = col->tNext;
				}
			}



			if(tSelected)
				tSelected->iSelected = false;

			tSelected = item;
			tSelected->iSelected = true;

			if(event != LV_DOUBLECLK)
				fLastMouseUp = tLX->fCurTime;

			return event;
		}

		y+=h;
		if(y>=iY+iHeight-h)
			break;
	}

	return LV_NONE;
}

///////////////////
// Mouse wheel down event
int	CListview::MouseWheelDown(mouse_t *tMouse)
{
	if(iGotScrollbar)
		cScrollbar.MouseWheelDown(tMouse);

	return -1;
}

///////////////////
// Mouse wheel up event
int	CListview::MouseWheelUp(mouse_t *tMouse)
{
	if(iGotScrollbar)
		cScrollbar.MouseWheelUp(tMouse);

	return -1;
}


///////////////////
// Get the ID of the currently selected item
int CListview::getSelectedID(void)
{
	if (!this)
		return -1;
    if(tSelected)
        return tSelected->_iID;
    return -1;
}


///////////////////
// Set the cur item to the item with the matching ID
void CListview::setSelectedID(int id)
{
    lv_item_t *item = tItems;	
	for(;item;item = item->tNext) {

        if(item->_iID == id) {            
            if(tSelected)
                tSelected->iSelected = false;
            item->iSelected = true;
            tSelected = item;
            return;
        }
    }
}


///////////////////
// Scroll to the last item
void CListview::scrollLast(void)
{
    cScrollbar.setValue(cScrollbar.getMax());
    cScrollbar.UpdatePos();
}


///////////////////
// This widget is send a message
int CListview::SendMessage(int iMsg, DWORD Param1, DWORD Param2)
{
	char *s = NULL;

	switch(iMsg) {

		// Add a column
		case LVM_ADDCOLUMN:
			AddColumn( (char *)Param1, Param2);
			break;

		// Add an item
		case LVM_ADDITEM:
			AddItem( (char *)Param1, Param2, 0xffff);
			break;

		// Add a sub item
		case LVM_ADDSUBITEM:
			if(Param1 == LVS_TEXT)
				AddSubitem(Param1, (char *)Param2, NULL);
			else
				AddSubitem(Param1, "", (SDL_Surface *)Param2);
			break;

		// Remove an item
		case LVM_REMOVEITEM:
			RemoveItem(Param1);
			break;

		// Get the current item's index
		case LVM_GETCURINDEX:
			return getCurIndex();

		// Get the current item's text index
		case LVM_GETCURSINDEX:
			s = getCurSIndex();
			if(s) {
				strncpy((char *)Param1, s, Param2);
				char *p = (char *)Param1;
				p[Param2-1] = '\0';
				return true;
			}
			else
				return false;

			break;

		// Get the item count
		case LVM_GETITEMCOUNT:
			return getItemCount();

		// Clear the items
		case LVM_CLEAR:
			Clear();
			break;

		// Get the index of an item based on count
		case LVM_GETITEMINDEX:
			return getIndex(Param1);

		// Get the index of an item based on index
		//case LVM_GETINDEX:	
            
        // Return the current item
        case LVM_GETCURITEM:
            if(tSelected)
                return (int)tSelected;
            return 0;


		default:
			printf("Bad listview message\n");
	}

	return 0;
}
