/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Custom browser class
// Created 7/6/02
// Jason Boettcher


#include "defs.h"
#include "LieroX.h"
#include "Menu.h"
#include "GUIFileParser.h"

/*						*/
/*	Helpful functions	*/
/*						*/
//////////////////
// Reads common events, that are available for almost every widget
void ReadEvents(xmlNodePtr Node, generic_events_t *Events)
{
	// Load the values
	xmlChar *onmouseover = xmlGetProp(Node,(const xmlChar *)"onmouseover");
	xmlChar *onmouseout  = xmlGetProp(Node,(const xmlChar *)"onmouseout");
	xmlChar *onmousedown = xmlGetProp(Node,(const xmlChar *)"onmousedown");
	xmlChar *onclick	 = xmlGetProp(Node,(const xmlChar *)"onclick");

	// Copy the values into the events
	if (Events->onmouseover)
		strcpy(Events->onmouseover, (char *)onmouseover);
	else
		strcpy(Events->onmouseover, "");

	if (Events->onmouseout)
		strcpy(Events->onmouseout, (char *)onmouseout);
	else
		strcpy(Events->onmouseout, "");

	if (Events->onmousedown)
		strcpy(Events->onmousedown, (char *)onmousedown);
	else
		strcpy(Events->onmousedown, "");

	if (Events->onclick)
		strcpy(Events->onclick, (char *)onclick);
	else
		strcpy(Events->onclick, "");

	// Free the data
	xmlFree(onmouseover);
	xmlFree(onmouseout);
	xmlFree(onmousedown);
	xmlFree(onclick);
}

/*					 */
/*	Widget IDs list  */
/*					 */

//////////////////
// Adds a new item to the widget ID list
// Returns the id of added item
int CWidgetList::Add(char *Name)
{
	// Find the ID of the new item
	int id = iCount+1;

	// Create new item
	widget_item_t *new_item = new widget_item_t;
	if (!new_item)
		return -1;

	// Fill in the item details
	new_item->iID = id;
	new_item->sName = new char[strlen(Name)+1];
	if(!new_item->sName)
		return -1;
	strcpy(new_item->sName,Name);
	new_item->tNext = NULL;  // It will be the last item

	// Link it in
	widget_item_t *last_item = tItems;

	// Add the new item at the end of the list
	if(last_item)  {
		while(last_item->tNext)  {
			last_item = last_item->tNext;
		}
		last_item->tNext = new_item;
	}

	// The list is emtpy
	else  {
		tItems = new_item;
	}

	// Successfully added
	iCount++;

	return id;
}

////////////////
// Get the name of widget by it's ID
char *CWidgetList::getName(int ID)
{
	// The list is empty
	if (!tItems)
		return NULL;

	// Go through the items
	widget_item_t *item = tItems;
	while(item)  {
		if (item->iID == ID)
			return item->sName;
		item = item->tNext;
	}

	// Not found
	return NULL;
}

////////////////
// Get the ID of widget by it's name
int	CWidgetList::getID(const char *Name)
{
	// The list is empty
	if (!tItems)
		return -1;

	// No name specified
	if (!Name)
		return -1;

	// Go through the items
	widget_item_t *item = tItems;
	while(item)  {
		if (!stricmp(Name,item->sName))
			return item->iID;
		item = item->tNext;
	}

	// Not found
	return -1;
}

///////////////
// Shutdown the widget IDs list
void CWidgetList::Shutdown(void)
{
	// The list is already empty
	if (!tItems)
		return;

	// Go through the list an delete each item
	widget_item_t *item = tItems;
	widget_item_t *next = NULL;
	for(;item;item=next)  {
		// Delete the name
		if (item->sName)
			delete[] item->sName;

		// Delete the item
		next = item->tNext;
		delete item;
	}
}

/*							*/
/*  Helpful XML functions	*/
/*							*/

///////////////////
// Get an integer from the specified property
int xmlGetInt(xmlNodePtr Node, const char *Name)
{
	xmlChar *sValue;
	sValue = xmlGetProp(Node,(const xmlChar *)Name);
	if(!sValue)
		return 0;
	int result = atoi((const char *)sValue);
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a float from the specified property
float xmlGetFloat(xmlNodePtr Node, const char *Name)
{
	xmlChar *sValue;
	sValue = xmlGetProp(Node,(const xmlChar *)Name);
	if (!sValue)
		return 0;
	float result = (float)atof((const char *)sValue);
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a colour from the specified property
Uint32 xmlGetColour(xmlNodePtr Node, const char *Name)
{
	char *sValue,*org_val;
	char tmp[3];
	int r,g,b;
	tmp[2] = 0;  // Third character is terminating

	// Get the value
	sValue = (char *)xmlGetProp(Node,(const xmlChar *)Name);
	org_val = sValue; // Save the original pointer

	// By default return black
	if(!sValue)
		return 0;

	// Ignore the # character
	if (*sValue == '#')
		sValue++;

	// R value
	strncpy(tmp,sValue,2);
	r = atoi(tmp);

	// G value
	strncpy(tmp,sValue+2,2);
	g = atoi(tmp);

	// B value
	strncpy(tmp,sValue+4,2);
	b = atoi(tmp);

	// Make the colour
	Uint32 result = MakeColour(r,g,b);

	xmlFree((xmlChar *)sValue);
	return result;
}


/*					  */
/*	The Parser class  */
/*					  */

///////////////////
// The create event
void CParser::Create(void)
{
	// Destroy any previous data
	Destroy();
}

////////////////////
// Notifies about the error that occured
void CParser::Error(int Code, char *Format,...)
{
	char buf[512];
	va_list	va;

	va_start(va,Format);
	vsprintf(buf,Format,va);
	va_end(va);

	printf("%i: %s",Code,buf);
}


///////////////////
// Load the XML file
int CParser::BuildLayout(CGuiLayout *Layout, char *sFilename)
{
	// Parse the document
	tDocument = xmlParseFile(sFilename);
	if (tDocument == NULL)  {
		Error(ERR_COULDNOTPARSE,"Could not parse the document %s",sFilename);
		return false;
	}

	// Get the root node
	tCurrentNode = xmlDocGetRootElement(tDocument);
	if (tCurrentNode == NULL)  {
		Error(ERR_EMPTYDOC,"The '%s' document is empty",sFilename);
		return false;
	}

	// Validate the root node
	if (CMP(tCurrentNode->name,"skin"))  {
		Error(ERR_INVALIDROOT,"The document '%s' contains invalid parent node: %s",sFilename,tCurrentNode->name);
		return false;
	}

	// Get the first child
	tCurrentNode = tCurrentNode->xmlChildrenNode;

	// Get all the nodes in document and create widgets in layout
	while (tCurrentNode != NULL)  {

		// Box
		if (CMP(tCurrentNode->name,"box"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			int round  = xmlGetInt(tCurrentNode,"round");
			Uint32 light_colour = xmlGetColour(tCurrentNode,"lightcolor");
			Uint32 dark_colour = xmlGetColour(tCurrentNode,"darkcolor");
			Uint32 bgcolour = xmlGetColour(tCurrentNode,"bgcolor");
			xmlChar *name = xmlGetProp(tCurrentNode,(const xmlChar *)"name");

			//Layout->Add(new CBox(round,light_colour,dark_colour,bgcolour),GetIdByName(name),left,top,width,height);
			xmlFree(name);
		}

		// Frame
		if (CMP(tCurrentNode->name,"frame"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			int round  = xmlGetInt(tCurrentNode,"round");
			Uint32 light_colour = xmlGetColour(tCurrentNode,"lightcolor");
			Uint32 dark_colour = xmlGetColour(tCurrentNode,"darkcolor");

			//Layout->Add(new CFrame(round,light_colour,dark_colour),-1,left,top,width,height);
		}

		// Image
		if (CMP(tCurrentNode->name,"img"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			xmlChar *src  = xmlGetProp(tCurrentNode,(const xmlChar *)"src");
			generic_events_t Events;
			ReadEvents(tCurrentNode,&Events);

			//Layout->Add(new CImage((char *) src, Events),AssignIdToName(name),left,top,width,height);
			xmlFree(name);
			xmlFree(src);

		}

		// Button
		if (CMP(tCurrentNode->name,"button"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name	 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			xmlChar *src	 = xmlGetProp(tCurrentNode,(const xmlChar *)"src");
			generic_events_t Events;
			ReadEvents(tCurrentNode,&Events);

			//Layout->Add(new CButton((char *) src, Events),GetIdByName(name),left,top,width,height);
			xmlFree(name);
			xmlFree(src);
		}


		// Checkbox
		if (CMP(tCurrentNode->name,"checkbox"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");

			//Layout->Add(new CCheckbox(false),GetIdByName(name),left,top,width,height);
			xmlFree(name);
		}


		// Combobox
		if (CMP(tCurrentNode->name,"combobox"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");

			//Layout->Add(new CCombobox(),GetIdByName(name),left,top,width,height);
			xmlFree(name);
		}

		// Inputbox
		if (CMP(tCurrentNode->name,"checkbox"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");

			//Layout->Add(new CInputbox(),GetIdByName(name),left,top,width,height);
			xmlFree(name);
		}

		// Label
		if (CMP(tCurrentNode->name,"checkbox"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			generic_events_t Events;
			ReadEvents(tCurrentNode,&Events);

			//Layout->Add(new CLabel(Events),GetIdByName(name),left,top,width,height);
			xmlFree(name);
		}

		// Listview
		if (CMP(tCurrentNode->name,"listview"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");

			//Layout->Add(new CListview(),GetIdByName(name),left,top,width,height);
			xmlFree(name);
		}

		// Scrollbar
		if (CMP(tCurrentNode->name,"scrollbar"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name	= xmlGetProp(tCurrentNode,(const xmlChar *)"name");

			//Layout->Add(new CScrollbar(),GetIdByName(name),left,top,width,height);
			xmlFree(name);
		}

		// Slider
		if (CMP(tCurrentNode->name,"slider"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");

			//Layout->Add(new CSlider(),GetIdByName(name),left,top,width,height);
			xmlFree(name);
		}

		// Textbox
		if (CMP(tCurrentNode->name,"checkbox"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name	= xmlGetProp(tCurrentNode,(const xmlChar *)"name");

			//Layout->Add(new CTextbox(),GetIdByName(name),left,top,width,height);
			xmlFree(name);
		}


		tCurrentNode = tCurrentNode->next;
	}

	return true;
}

////////////////////
// Get the widget list for specified layout
CWidgetList *CParser::GetLayoutWidgets(int LayoutID)
{ 
	if (LayoutID >=0 && LayoutID <= L_MESSAGEBOXYESNO)
		return &LayoutWidgets[LayoutID];
	else 
		return NULL;
}

////////////////////
// Get the widget ID
int CParser::GetIdByName(char *Name,int LayoutID)
{
	int ID = -1;
	// Find the standard or previously added widget
	ID = LayoutWidgets[LayoutID].getID(Name);

	// Non-standard widget, add it to the list
	if (ID == -1)
		ID = LayoutWidgets[LayoutID].Add(Name);
	return ID;
}


///////////////////
// Destroy function
void CParser::Destroy(void)
{
	// Free the XML data
	if (tDocument != NULL)
		xmlFreeDoc(tDocument);
	tDocument = NULL;
	tCurrentNode = NULL;

	// Shutdown the layouts
	for (int i=0; i<L_MESSAGEBOXYESNO; i++)
		LayoutWidgets[i].Shutdown();
}

