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
	int x=iX+4;

	for(int i=1;col;col = col->tNext,i++) {
		if (bOldStyle)  {
			tLX->cFont.Draw(bmpDest, x, iY, tLX->clNormalLabel,"%s", col->sText);
		} else {

			int col_w = col->iWidth;
			// Last column has to be thiner
			if (i == iNumColumns)
				col_w--;
			Menu_DrawWinButton(bmpDest,x-2,iY+2,col_w-3,tLX->cFont.GetHeight(),col->bDown);
			switch (col->iSorted)  {
			case 0:	DrawImage(bmpDest,tMenu->bmpTriangleUp,x+col_w-tMenu->bmpTriangleUp->w-9,iY+7); break;
			case 1:	DrawImage(bmpDest,tMenu->bmpTriangleDown,x+col_w-tMenu->bmpTriangleDown->w-9,iY+7); break;
			}

			tLX->cFont.DrawCentreAdv(bmpDest, x+(col_w/2)-3, iY+2, x+2, MIN(col_w-2,iX+iWidth-x-20), tLX->clNormalLabel,"%s", col->sText);
		}
		x += col->iWidth-2;
	}

	//DrawHLine(bmpDest,iX+3,iX+iWidth-3,iY+tLX->cFont.GetHeight()+4,0xffff);


	// Draw the items
	int y=iY;
	if (bOldStyle)
		y = iY+tLX->cFont.GetHeight()+2;
	else
		y = iY+tLX->cFont.GetHeight()+4;
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
					if(sub->iType == LVS_TEXT)  {
						if (col && !bOldStyle)
							tLX->cFont.DrawAdv(bmpDest,x,texty,MIN(col->iWidth-8,iX+iWidth-x-20),colour,"%s",sub->sText);
						else
							tLX->cFont.Draw(bmpDest,x,texty,colour,"%s",sub->sText);
					}

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
    Menu_DrawBoxInset(bmpDest, iX, iY+15*bOldStyle, iX+iWidth, iY+iHeight);
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
	fix_strncpy(col->sText,sText);
	col->iWidth = iWidth;
	col->tNext = NULL;
	col->bDown = false;
	col->iSorted = -1;


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

	iNumColumns++;
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
	fix_strncpy(item->sIndex,sIndex);
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
	fix_strncpy(sub->sText,sText);
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
//	lv_item_t *item = NULL;  // TODO: not used
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

/////////////////////
// Gets the width of a specified column
int CListview::GetColumnWidth(int id)
{
	if (id > iNumColumns)
		return 5;

	// Go through the columns
	lv_column_t *col = tColumns;
	for (int i=0;i<id && col;col=col->tNext,i++)  {}

	if (col)
		return col->iWidth;
	else
		return 5;

}

///////////////
// Sorts the list by the current sorting column
// Useful, when you're re-filling the list or adding new items
void CListview::ReSort(void)
{
	lv_column_t *col = tColumns;

	// Find the column
	int i=0;
	for (;col;col=col->tNext,i++) {
		if (col->iSorted != -1)
			break;
	}

	// Not found
	if (!col)
		return;

	// Sort
	SortBy(i,col->iSorted==1);
}

///////////////
// Sorts the listview by specified column, ascending or descending
void CListview::SortBy(int column, bool ascending)
{
	// Check
	if (column < 0 || column >= iNumColumns)
		return;

	// Get the item
	lv_item_t *item = tItems;
	if (!item)
		return;

	lv_item_t *prev_item = NULL;
	lv_subitem_t *subitem1 = NULL;
	lv_subitem_t *subitem2 = NULL;

	bool bSwapped = true;

	// Bubble sort the list
	while (bSwapped)  {
		bSwapped = false;
		prev_item = NULL;
		for(item=tItems;item->tNext;item=item->tNext) {
			subitem1=item->tSubitems;
			subitem2=item->tNext->tSubitems;

			// Get subitem 1
			int i;
			for(i=0;i != column && subitem1;subitem1=subitem1->tNext,i++) {	}

			// Get subitem 2
			for(i=0;i != column && subitem2;subitem2=subitem2->tNext,i++) { }

			bool swap = false;
			if (subitem2 && subitem1)  {
				// Swap the two items?
				int nat_cmp1 = atoi(subitem1->sText);
				int nat_cmp2 = atoi(subitem2->sText);
				// First try, if we compare numbers
				if (nat_cmp1 && nat_cmp2)  {
					if (ascending)
						swap = nat_cmp1 > nat_cmp2;
					else
						swap = nat_cmp2 > nat_cmp1;
				// String comparison
				} else {
					int tmp = strncasecmp(subitem1->sText,subitem2->sText,sizeof(subitem1->sText));
					if (ascending)
						swap = tmp > 0;
					else
						swap = tmp < 0;
				}
			} else  {
				if (ascending)
					swap = subitem1 != NULL;
				else
					swap = subitem2 != NULL;
			}

			// Swap if they're not in the right order
			if (swap)  {
				lv_item_t *it3 = item->tNext;
				lv_item_t *it4 = item->tNext->tNext;
				if (prev_item)
					prev_item->tNext = it3;
				else
					tItems = it3;
				it3->tNext = item;
				item->tNext = it4;
				if (!it4)  {
					tLastItem = it3;
					break;
				}
				bSwapped = true;
				prev_item = item;
				continue;
			}

			prev_item = item;
		}
	}

	// Update the ID of the selected item
	int i=0;
	for (item=tItems;item;item=item->tNext,i++)
		if (item == tSelected)  {
			iItemID = i;
			break;
		}

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
	if(tMouse->X > iX+iWidth-20 && tMouse->Y >= iY+20 && iGotScrollbar)
		cScrollbar.MouseOver(tMouse);

	// Reset the cursor
	iCursor = 0;

	// Go through the columns and check, if the mouse is in the space between two columns
	if (!bOldStyle)  {
		if( tMouse->Y >= iY+2 && tMouse->Y <= iY+2+tLX->cFont.GetHeight()+1)  {
			lv_column_t *col = tColumns;
			lv_column_t *prev = NULL;
			if (!col)
				return LV_NONE;

			int x = iX+col->iWidth-2;
			col = col->tNext;
			prev = col;
			for (;col;col = col->tNext)  {
				if (tMouse->X >= x && tMouse->X <= x+4)  {
					iCursor = 3;
					return LV_RESIZECURSOR;
				}
				x += col->iWidth-2;
				prev = col;
			}
		}
	}

	return LV_NONE;
}


///////////////////
// Mouse down event
int	CListview::MouseDown(mouse_t *tMouse, int nDown)
{
	if(((tMouse->X > iX+iWidth-20) && iGotScrollbar) || cScrollbar.getGrabbed()) {
		cScrollbar.MouseDown(tMouse, nDown);
		return LV_NONE;
	}

	if(tMouse->X < iX || tMouse->X > iX+iWidth-18)
		return LV_NONE;

	//
	// Column headers
	//
	if (!bOldStyle)  {
		lv_column_t *col = tColumns;
		lv_column_t *prev = NULL;
		int i=0;

		// First of all, reset the click state of all headers
		for (;col;col=col->tNext)
			col->bDown = false;

		col = tColumns;

		// Is some of the columns grabbed? Move it
		if (iGrabbed > 0)  {
			int x = iX+4;

			// Get the column
			for (i=0;i != iGrabbed && col;i++) {
				prev = col;
				x+=col->iWidth-2;
				col = col->tNext;
			}

			// Resize the two columns
			int w1,w2;
			w1=w2=0;
			if (prev)  {
				w1 = prev->iWidth + tMouse->X - iLastMouseX;
			}
			w2 = col->iWidth - tMouse->X + iLastMouseX;

			// Resize only if they both will have at least minimal width
			if (w1 > 4 && w2 > 4)  {
				prev->iWidth = w1;
				col->iWidth = w2;
			}


			iLastMouseX = tMouse->X;
			iCursor = 3;

			return LV_RESIZECURSOR;
		}

		// Not grabbed
		if( tMouse->Y >= iY+2 && tMouse->Y <= iY+2+tLX->cFont.GetHeight()+1)  {
			int x = iX+4;
			col = tColumns;
			prev = NULL;
			for (i=0;col;col = col->tNext,i++)  {
				col->bDown = false;
				// If in the area between two columns, grab
				if (tMouse->X >= x-8 && tMouse->X <= x && col != tColumns)  {
					iGrabbed = i;
					// Hack
					if (prev)
						prev->bDown = false;
				}
				// Click
				else if (tMouse->X >= x && tMouse->X <= x+col->iWidth-3 && iGrabbed <= 0)  {
					iCursor = 0;
					col->bDown = true;
				}
				x += col->iWidth-2;
				prev = col;
			}
			iLastMouseX = tMouse->X;
			return LV_NONE;
		}
	}

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
	iLastMouseX = 0;

	if((tMouse->X > iX+iWidth-20 || cScrollbar.getGrabbed()) && iGotScrollbar)
		cScrollbar.MouseUp(tMouse, nDown);

	if(tMouse->X < iX || tMouse->X > iX+iWidth-18) {
		fLastMouseUp = -9999;
		return LV_NONE;
	}

	// Column headers
	if (!bOldStyle)  {
		if( tMouse->Y >= iY+2 && tMouse->Y <= iY+2+tLX->cFont.GetHeight()+1 && tLX->fCurTime-fLastMouseUp >= 0.15f && iGrabbed <= 0)  {
			fLastMouseUp = tLX->fCurTime;
			int x = iX+4;
			lv_column_t *col = tColumns;
			for (int i=0;col;col = col->tNext,i++)  {
				col->bDown = false;
				if (tMouse->X >= x && tMouse->X <= x+col->iWidth-3)  {
					bool asc = col->iSorted == -1 || col->iSorted == 0;
					SortBy(i,asc);
					if (asc)
						col->iSorted = 1;
					else
						col->iSorted = 0;
				} else
					col->iSorted = -1;
				x += col->iWidth-2;
			}
			return LV_NONE;
		}

		iGrabbed = 0;
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

////////////////////
// Get the specified subitem of the selected item
lv_subitem_t *CListview::getCurSubitem(int index)
{
	if (!tSelected)
		return NULL;
	lv_subitem_t *sub = tSelected->tSubitems;
	for (int i=0;i<index && sub;i++,sub=sub->tNext) {}
	return sub;
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
			AddItem( (char *)Param1, Param2, tLX->clListView);
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

		// Set the old-style property
		case LVM_SETOLDSTYLE:
			bOldStyle = true;
			break;

		// Get the column width
		case LVM_GETCOLUMNWIDTH:
			return GetColumnWidth(Param1);
			break;


		default:
			printf("Bad listview message\n");
	}

	return 0;
}


lv_item_t* CListview::getItem(int index) {
	for(lv_item_t* i = tItems; i; i = i->tNext) {
		if(i->iIndex == index)
			return i;
	}
	return NULL;
}

lv_item_t* CListview::getItem(char* name) {
	for(lv_item_t* i = tItems; i; i = i->tNext) {
		if(strncmp(i->sIndex,name,sizeof(i->sIndex)) == 0)
			return i;
	}
	return NULL;
}
