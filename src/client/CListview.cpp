/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// gui Listview class
// Created 16/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"
#include "GfxPrimitives.h"
#include "StringUtils.h"


///////////////////
// Draw the list view
void CListview::Draw(SDL_Surface *bmpDest)
{
	bNeedsRepaint = false; // We're repainting :)

	lv_subitem_t *sub = NULL;

	if (bRedrawMenu)
		Menu_redrawBufferRect(iX,iY, iWidth,iHeight);


	// Draw the columns
	lv_column_t *col = tColumns;
	int x=iX+4;

	if (bOldStyle)  {
		for(short i=1;col;col = col->tNext,i++)   {
			tLX->cFont.Draw(bmpDest, x, iY, tLX->clNormalLabel, col->sText);
			x += col->iWidth-2;
		}
	} else {
		short col_w;
		for(short i=1;col;col = col->tNext,i++)   {
			col_w = col->iWidth;
			// Last column has to be thiner
			if (i == iNumColumns)  {
				if (x-2+col_w-1 != iX+iWidth-4)  { // Column wider than the space for it
					col_w = iWidth-x+iX+4;
					col->iWidth = col_w;  // Update it when we've counted it...
				}
				col_w--;
			}
			Menu_DrawWinButton(bmpDest,x-2,iY+2,col_w-3,tLX->cFont.GetHeight(),col->bDown);
			switch (col->iSorted)  {
			case 0:	DrawImage(bmpDest,tMenu->bmpTriangleUp,x+col_w-tMenu->bmpTriangleUp->w-9,iY+7); break;
			case 1:	DrawImage(bmpDest,tMenu->bmpTriangleDown,x+col_w-tMenu->bmpTriangleDown->w-9,iY+7); break;
			}

			tLX->cFont.DrawCentreAdv(bmpDest, x+(col_w/2)-3, iY+2, x+2, MIN(col_w-2,iX+iWidth-x), tLX->clNormalLabel, col->sText);

			x += col->iWidth-2;
		}
	}

	
	// Draw the items
	int y=iY;
	if (tColumns)  {
		if (bOldStyle)
			y = iY+tLX->cFont.GetHeight();
		else
			y = iY+tLX->cFont.GetHeight()+2;
	} 
	
	// Re-setup the scrollbar
	if (!bCustomScrollbarSetup)  {
		if (bDrawBorder)
			y += 2;
		cScrollbar.Setup(0, iX+iWidth-16, y, 14, iHeight - y + iY - 1);
	}

	x = iX+4;
	lv_item_t *item = tItems;
	int count=0;

	int right_bound = iX+iWidth-3;
	if(iGotScrollbar || bAlwaysVisibleScrollbar)
		right_bound = MIN(cScrollbar.getX() - 2, iX + iWidth - 3);

	int h=tLX->cFont.GetHeight();
	int texty = 0;


	for(;item;item = item->tNext) {
		if(count++ < cScrollbar.getValue())
			continue;

		h = tLX->cFont.GetHeight();
		x = iX+4;

		col = tColumns;

		// Find the max height
		h = MAX(h, item->iHeight);

		// Selected?
		if(item->iSelected && bShowSelect) {
			if(iFocused)
				DrawRectFill(bmpDest,x-2,y,right_bound,y+h-2,tLX->clListviewSelected);
			else
				DrawRect(bmpDest,x-2,y,right_bound,y+h-2,tLX->clListviewSelected);
		}

		// Draw the sub items
		if(item->tSubitems) {

			sub = item->tSubitems;
			col = tColumns;


			// Text vertical position
			switch (sub->iValign)  {
			case VALIGN_TOP:
				texty = y;
				break;
			case VALIGN_BOTTOM:
				texty = y + h - tLX->cFont.GetHeight();
				break;
			default: // Middle
				texty = y + (h-tLX->cFont.GetHeight())/2;
				break;
			}

			for(;sub;sub = sub->tNext) {

				if(sub->iVisible) {
					switch(sub->iType)  {
					case LVS_TEXT:  {
						if (col && !bOldStyle)
							tLX->cFont.DrawAdv(bmpDest,x,texty,MIN(col->iWidth-8,right_bound-x),item->iColour,sub->sText);
						else
							tLX->cFont.DrawAdv(bmpDest,x,texty,right_bound-x-2,item->iColour,sub->sText);
					}
					break;

					case LVS_IMAGE:

						// Draw according to valign
						switch (sub->iValign)  {
						case VALIGN_TOP:
							DrawImage(bmpDest,sub->bmpImage,x,y);
							break;

						case VALIGN_BOTTOM:
							DrawImage(bmpDest,sub->bmpImage, x, y + item->iHeight - sub->bmpImage->h);
							break;

						// Middle
						default:
							DrawImage(bmpDest,sub->bmpImage, x, y + item->iHeight/2 - sub->bmpImage->h/2);
							break;
						}
					break;
					}
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
	if(iGotScrollbar || bAlwaysVisibleScrollbar)
		cScrollbar.Draw(bmpDest);

    // Draw the rectangle last
	if (bDrawBorder)
		Menu_DrawBoxInset(bmpDest, iX, iY+(tLX->cFont.GetHeight()-1)*bOldStyle, iX+iWidth, iY+iHeight);
}


///////////////////
// Add a column to the list view
void CListview::AddColumn(const std::string& sText, int iWidth)
{
	lv_column_t *col;

	col = new lv_column_t;
	if(col == NULL) {
		// Out of memory
		return;
	}

	// Set the info
	col->sText = sText;
	col->iWidth = iWidth;
	col->tNext = NULL;
	col->bDown = false;
	col->iSorted = -1;


	// Add it to the list
	int curwidth = 0;
	lv_column_t *widest = NULL;
	if(tColumns) {
		lv_column_t *c = tColumns;
		widest = c;
		for(;c;c = c->tNext) {
			curwidth += c->iWidth;
			if(c->tNext == NULL) {
				c->tNext = col;
				break;
			}
			if (c->iWidth > widest->iWidth)
				widest = c;
		}
	}
	else
		tColumns = col;

	int maxwidth = this->iWidth-4-curwidth;

	// If the specified width is weird (negative, zero or too wide), we'll count a better one
	if (iWidth <= 0 || iWidth > maxwidth)  {
		int defwidth = tLX->cFont.GetWidth(sText)+4;
		// The current columns take whole space, so we need to make some for the new column
		if (maxwidth <= 1)  {
			// Clamp the widest column to make space for this one
			if (widest)  {
				maxwidth = widest->iWidth/2;
				defwidth = maxwidth;
				widest->iWidth /= 2;
			} else {
				defwidth = maxwidth;
			}
		// There's still some space for the current column
		} else {
			// The column has weird (really short) name, so we use maximal size
			if (defwidth <= 5)
				defwidth = maxwidth;
			// Insert the column, make sure it's width is not greater than the maximal allowed width
			else
				defwidth = MIN(defwidth,maxwidth);
		}
		col->iWidth = defwidth;
	}

	iNumColumns++;
	bNeedsRepaint = true; // We need a repaint
}


///////////////////
// Add an item to the list view
void CListview::AddItem(const std::string& sIndex, int iIndex, int iColour)
{
	lv_item_t *item = new lv_item_t;

	if(item == NULL) {
		// Out of memory
		return;
	}

	// Set the info
	item->sIndex = sIndex;
	item->iIndex = iIndex;
	item->tNext = NULL;
	item->iSelected = false;
	item->tSubitems = NULL;
	item->iHeight = tLX->cFont.GetHeight();			// Text height
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

	// We need a repaint
	bNeedsRepaint = true;
}


///////////////////
// Add a sub item to the last item
void CListview::AddSubitem(int iType, const std::string& sText, SDL_Surface *img, int iVAlign)
{
	lv_subitem_t *sub = new lv_subitem_t;

	if(sub == NULL) {
		// Out of memory
		return;
	}

	// Set the info
	sub->iType = iType;
	sub->sText = sText;
	sub->bmpImage = NULL;
	sub->tNext = NULL;
	sub->iVisible = true;
	sub->iExtra = 0;
	sub->iValign = iVAlign;
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

	bNeedsRepaint = true; // Repaint required
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
	if (tColumns)
		size += tLX->cFont.GetHeight() + 4;
	if (bDrawBorder)
		size += 4;

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

	iGotScrollbar = count > h;
}


///////////////////
// Remove an item from the list
void CListview::RemoveItem(int iIndex)
{
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

	bNeedsRepaint = true; // Repaint required
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
				bool failed1,failed2;
				int nat_cmp1 = from_string<int>(subitem1->sText,failed1);
				int nat_cmp2 = from_string<int>(subitem2->sText,failed2);
				// First try, if we compare numbers
				if (!failed1 && !failed2)  {
					if (ascending)
						swap = nat_cmp1 > nat_cmp2;
					else
						swap = nat_cmp2 > nat_cmp1;
				// String comparison
				} else {
					int tmp = stringcasecmp(subitem1->sText,subitem2->sText);
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

	bNeedsRepaint = true; // Repaint required

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
	tLastItem = NULL;
	tSelected = NULL;

	cScrollbar.setMin(0);
	cScrollbar.setMax(1);
	cScrollbar.setValue(0);
	iItemCount=0;
    iItemID = 0;
	iGotScrollbar=false;
	bNeedsRepaint = true; // Repaint required
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

	bNeedsRepaint = true; // Repaint required
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
	SetGameCursor(CURSOR_ARROW);

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
					SetGameCursor(CURSOR_RESIZE);
					return LV_NONE;
				}
				x += col->iWidth-2;
				prev = col;
			}
		}
	}

	bNeedsRepaint = true; // Repaint required

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

		// Is any of the columns grabbed? Move it
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
			SetGameCursor(CURSOR_RESIZE);

			return LV_NONE;
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
					SetGameCursor(CURSOR_ARROW);
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
	int y = iY+tLX->cFont.GetHeight()+2;
	if (!tColumns)
		y = iY+2;
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
		if(y>=iY+iHeight)
			break;
	}

	bNeedsRepaint = true; // Repaint required

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
	if (!bOldStyle && tColumns)  {
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
	int y = iY+tLX->cFont.GetHeight()+2;
	if (!tColumns)
		y = iY+2;
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

	bNeedsRepaint = true; // Repaint required

	return LV_NONE;
}

///////////////////
// Mouse wheel down event
int	CListview::MouseWheelDown(mouse_t *tMouse)
{
	if(iGotScrollbar)  {
		cScrollbar.MouseWheelDown(tMouse);
		bNeedsRepaint = true; // Repaint required
	}

	return LV_NONE;
}

///////////////////
// Mouse wheel up event
int	CListview::MouseWheelUp(mouse_t *tMouse)
{
	if(iGotScrollbar)  {
		cScrollbar.MouseWheelUp(tMouse);
		bNeedsRepaint = true; // Repaint required
	}

	return LV_NONE;
}

/////////////////
// Key down event
int CListview::KeyDown(UnicodeChar c)
{
	if (c == iLastChar && c)
		return LV_NONE;

	if (c >= 31)  {
		// TODO: handle normal characters
		return LV_NONE;
	}

	keyboard_t *kb = GetKeyboard();
	if (kb->KeyDown[iLastChar])
		return LV_NONE;

	iLastChar = c;

	// Up arrow
	if (kb->KeyDown[SDLK_DOWN]) {
		if (tSelected) {
			if (tSelected->tNext)  {
				tSelected->iSelected = false;
				tSelected = tSelected->tNext;
				tSelected->iSelected = true;
				iLastChar = SDLK_DOWN;
				if (iGotScrollbar)
					if (tSelected->_iID >= (cScrollbar.getItemsperbox()-1 + cScrollbar.getValue()))
						cScrollbar.setValue( cScrollbar.getValue()+1 );			
				return LV_NONE;
			}
		} else {
			tSelected = tItems;
		}
	}


	// Down arrow
	if (kb->KeyDown[SDLK_UP])  {
		lv_item_t *i = tItems;
		for (;i->tNext;i=i->tNext)  {
			if (i->tNext == tSelected) {
				if (tSelected)
					tSelected->iSelected = false;
				tSelected = i;
				tSelected->iSelected = true;
				iLastChar = SDLK_UP;
				if (iGotScrollbar)
					if (cScrollbar.getValue() > tSelected->_iID)
						cScrollbar.setValue( cScrollbar.getValue()-1 );	
				return LV_NONE;
			}
		}
	}

	// Home
	if (kb->KeyDown[SDLK_HOME])  {
		if (tItems)  {
			if (tSelected)
				tSelected->iSelected = false;
			tSelected = tItems;
			tSelected->iSelected = true;
			if (iGotScrollbar)
				cScrollbar.setValue(0);
		}
	}

	// End
	if (kb->KeyDown[SDLK_END])  {
		if (tLastItem)  {
			if (tSelected)
				tSelected->iSelected = false;
			tSelected = tLastItem;
			tSelected->iSelected = true;
			if (iGotScrollbar)
				cScrollbar.setValue(tSelected->_iID);
		}
	}

	// Delete
	if (kb->KeyDown[SDLK_DELETE])  {
		iLastChar = SDLK_DELETE;
		return LV_DELETE;
	}

	// Enter
	if (kb->KeyDown[SDLK_RETURN])  {
		iLastChar = SDLK_RETURN;
		return LV_ENTER;
	}

	// Enter (numeric)
	if (kb->KeyDown[SDLK_KP_ENTER])  {
		iLastChar = SDLK_KP_ENTER;
		return LV_ENTER;
	}

	bNeedsRepaint = true; // Repaint required

	return LV_NONE;
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

			// Scroll to the item if needed
			if (iGotScrollbar)  {
				cScrollbar.setValue(tSelected->_iID);
			}

            return;
        }
    }
}


///////////////////
// Scroll to the last item
void CListview::scrollLast(void)
{
	if (iGotScrollbar)  {
		cScrollbar.setValue(cScrollbar.getMax());
		cScrollbar.UpdatePos();
		bNeedsRepaint = true; // Repaint required
	}
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
DWORD CListview::SendMessage(int iMsg, DWORD Param1, DWORD Param2)
{
	std::string s = "";

	switch(iMsg) {

		// Remove an item
		case LVM_REMOVEITEM:
			RemoveItem(Param1);
			break;

		// Get the current item's index
		case LVM_GETCURINDEX:
			return getCurIndex();

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
                return (DWORD)tSelected;
            return 0;

		// Set the old-style property
		case LVM_SETOLDSTYLE:
			bOldStyle = true;
			bNeedsRepaint = true; // Repaint required
			break;

		// Get the column width
		case LVM_GETCOLUMNWIDTH:
			return GetColumnWidth(Param1);
			break;

		// Add a sub item
		case LVS_ADDSUBITEM:
			if(Param2 == LVS_IMAGE)
				AddSubitem(LVS_IMAGE, "", (SDL_Surface *)Param1);
			else
				printf("WARNING: LVS_ADDSUBITEM message got unknown type\n");
			break;
			
		default:
			printf("Bad listview message\n");
	}

	return 0;
}

DWORD CListview::SendMessage(int iMsg, const std::string& sStr, DWORD Param)
{
	switch (iMsg)  {

	// Add a column
	case LVS_ADDCOLUMN:
		AddColumn(sStr,Param);
		break;

	// Add an item
	case LVS_ADDITEM:
		AddItem(sStr,Param,tLX->clListView);
		break;

	// Add a sub item
	case LVS_ADDSUBITEM:
		if(Param == LVS_TEXT)
			AddSubitem(Param, sStr, NULL);
		else
			printf("WARNING: LVS_ADDSUBITEM message got unknown type\n");
		break;
		
	default:
		printf("Bad listview message\n");
	}

	return 0;
}


DWORD CListview::SendMessage(int iMsg, std::string *sStr, DWORD Param)
{
	switch (iMsg)  {

	// Get the current item's text index
	case LVS_GETCURSINDEX:
		*sStr = getCurSIndex();
		return *sStr != "";
		break;
	}

	return 0;
}

///////////////////
// Get an item based on the index
lv_item_t* CListview::getItem(int index) {
	for(lv_item_t* i = tItems; i; i = i->tNext) {
		if(i->iIndex == index)
			return i;
	}
	return NULL;
}

/////////////////
// Get an item based on the name
lv_item_t* CListview::getItem(const std::string& name) {
	for(lv_item_t* i = tItems; i; i = i->tNext) {
		if(stringcasecmp(i->sIndex,name) == 0)
			return i;
	}
	return NULL;
}

////////////////
// Custom scrollbar setup
void CListview::SetupScrollbar(int x, int y, int h, bool always_visible)
{
	int full_auto_setup = 0;

	if (h <= 0) { // Height should be set automatically
		full_auto_setup++;
		h = iHeight;
		if (tColumns)  {
			h -= tLX->cFont.GetHeight();
			if (!bOldStyle)
				h -= 4;
		}

		if (bDrawBorder)
			h -= 4;

		h = MAX(h, 1); // Safety
	}

	if (y < 0)  { // Y position should be set automatically
		full_auto_setup++;
		y = iY;
		if (tColumns)  {
			iY += tLX->cFont.GetHeight();
			if (!bOldStyle)
				h += 4;
		}

		if (bDrawBorder)
			y += 2;
	}

	if (x < 0)  { // X position should be set automatically
		full_auto_setup++;
		x = iX + iWidth - 14;
	}
	

	// Setup the scrollbar
	cScrollbar.Setup(0, x, y, 14, h);
	bAlwaysVisibleScrollbar = always_visible;
	bCustomScrollbarSetup = (full_auto_setup != 3); // If full auto setup is 3, all the three params should
													// be set up automatically
}
