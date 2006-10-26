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


///////////////////
// The create event
void CParser::Create(void)
{
	// Destroy any previous data
	Destroy();

	iLines = -1;

	tObjects = NULL;
	
	iPos = iLength = 0;
	sData = NULL;
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
// Load the cmht file
int CParser::Load(char *sFilename)
{
	FILE *fp;
	
	// Open the file
	fp = fopen(sFilename,"rb");
	if(fp == NULL)  {
		Error(1,"Could not open file: %s",sFilename);
		return false;
	}

	fseek(fp,0,SEEK_END);
	iLength = ftell(fp);
	fseek(fp,0,SEEK_SET);

	// Allocate room for the data
	sData = new char[iLength];
	if(sData == NULL) {
		fclose(fp);
		Error(2,"%s","Not enough of free memory.");
		return false;
	}

	// Load the data
	fread(sData,sizeof(char),iLength,fp);
	fclose(fp);


	// Go through the file
	iPos=0;
	iLines=-1;
	while(iPos<iLength) {
		ReadObject();
		iPos++;
	}


	// Free the data
	if(sData) {
		delete[] sData;
		sData = NULL;
	}
	
	return true;
}


///////////////////
// Destroy function
void CParser::Destroy(void)
{
	// Go through freeing each object
	tag_object_t *obj = tObjects;
	tag_object_t *o;

	for(; obj ; obj=o) {
		o = obj->tNext;
		
		if(obj->strText) {
			delete[] obj->strText;
			obj->strText = NULL;
		}
		
		if(obj)
			delete obj;
	}
	
	tObjects = NULL;
}


///////////////////
// Read an object
void CParser::ReadObject(void)
{
	// Check first character
	char c = sData[iPos];

	// Comment
	if(c == '/' && sData[iPos+1] == '/') {
		// Read until the end of the line
		ReadNewline();
		return;
	}

	// Tag
	if(c == '<') {
		ReadTag();
		return;
	}

	// Newline
	if(c == '\n' || c == '\r') {
		return;
	}

	// Normal text
	ReadText();
}


///////////////////
// Set the position to the next line
void CParser::ReadNewline(void)
{
	while(iPos < iLength) {
		if(sData[iPos] == '\n') {
			iPos--;
			return;
		}
		iPos++;
	}
}


///////////////////
// Read a tag
void CParser::ReadTag(void)
{
	char sName[32];
	property_t cProperties[16];
	int i = 0;
	int val = 0;
	int end = false;
	int iNumProperties = 0;
	
	sName[0] = 0;

	// Tag list
	char	*sTags[] = {"b","u","i","s","frame","img","button","checkbox","combobox","inputbox","label","listview","menu","scrollbar","slider","textbox"};
	int		iNumTags = 15;

	iPos++;

	// Check if it's an end tag
	if(sData[iPos] == '/') {
		end=true;
		iPos++;
	}

	// Read the tag name until a space, arrow or newline
	while(iPos < iLength) {
		if(sData[iPos] == ' ' || sData[iPos] == '>' || sData[iPos] == '\r' || sData[iPos] == '\n') {
			sName[i] = '\0';
			break;
		}

		sName[i++] = sData[iPos++];
	}

	// Convert the name to lower case
	strlwr(sName);

			
	// Some tags contain properties (and not the end tags)
	if(stricmp(sName,"frame") == 0 ||
	   stricmp(sName,"img") == 0 ||
	   stricmp(sName,"button") == 0 ||
	   stricmp(sName,"checkbox") == 0 ||
	   stricmp(sName,"combobox") == 0 ||
	   stricmp(sName,"inputbox") == 0 ||
	   stricmp(sName,"label") == 0 ||
	   stricmp(sName,"listview") == 0 ||
	   stricmp(sName,"scrollbar") == 0 ||
	   stricmp(sName,"slider") == 0 ||
	   stricmp(sName,"textbox") == 0  &&
	   !end) {
		
		iPos++;
		i=0;
		int curProperty = 0;
		iNumProperties = 1;
		bool quotesOpened = false;
		bool readingName = true;
		bool escapeSeq = false;

		//
		// Read the tag properties
		//
		while(iPos < iLength) {
			if(sData[iPos] == ' ' || sData[iPos] == '>' || sData[iPos] == '\n' || sData[iPos] == '\r') {
				// Ignore these characters when they're in quotes
				if(!quotesOpened)  {
					cProperties[curProperty].sValue[i] = '\0';  // Finalize the value

					// Remove spaces
					TrimSpaces(cProperties[curProperty].sValue);
					TrimSpaces(cProperties[curProperty].sName);

					// Convert to lower case
					strlwr(cProperties[curProperty].sName);
					strlwr(cProperties[curProperty].sValue);

					// Convert the value to numeric (if possible)
					// Must be after TrimSpaces
					cProperties[curProperty].iValue = atoi(cProperties[curProperty].sValue);

					// End of tag?
					if (sData[iPos] == '>')
						break;

					// Read next value
					else  {
						// Ignore blank properties (for example two spaces in a row)
						if(strlen(cProperties[curProperty].sName) > 1)  {
							curProperty++;
							iNumProperties++;
						}

						// Reset variables and check for errors
						i = 0;
						if(readingName)
							Error(4,"The %s property doesn't have any value",cProperties[curProperty].sName);
						readingName = true;
						if (escapeSeq)
							Error(5,"%s","The backslash character in the wrong position");
						escapeSeq = false;
						continue;
					}
				// Ignore the \r and \n characters in any case
				} else if (sData[iPos] == '\n' || sData[iPos] == '\r')  {
					iPos++;
					continue;
				}
			}

			// Escape sequence
			if(sData[iPos] == '\\' && !readingName && !escapeSeq)  {
				escapeSeq = true;
				iPos++;
				continue;
			}

			// Quote character
			if(sData[iPos] == '\"' && !escapeSeq)  {
				quotesOpened = !quotesOpened;
				continue;
			}

			// Character '='
			if(sData[iPos] == '=' && !quotesOpened)  {
				readingName = false;
				cProperties[curProperty].sName[i] = '\0';  // Finalize the property name
				i = 0;
				continue;
			}


			// Read the property name and value
			if (readingName)
				cProperties[curProperty].sName[i++]  = sData[iPos++];
			else
				cProperties[curProperty].sValue[i++] = sData[iPos++];
		}
	}

	// Find the tag id
	int id = -1;
	for(i=0;i<iNumTags;i++) {
		if(stricmp(sTags[i],sName) == 0) {
			id = i+O_BOLD;
			break;
		}
	}

	if(id == -1) {
		// Error
		Error(3,"Unknown tag: %s",sName);
		return;
	}


	// Add the object to the list
	AddObject(sName,cProperties,iNumProperties,id,end);
}


///////////////////
// Read text as an object
void CParser::ReadText(void)
{
	char	sText[4096];
	int i=0;

	// Read the text until a tag
	while(iPos < iLength) {
		if(sData[iPos] == '<') {
			iPos--;
			break;
		}

		sText[i++] = sData[iPos++];

		if(i>=4095)
			break;
	}
	sText[i] = '\0';

	AddObject(sText,NULL,0,HTO_TEXT,0);
}


///////////////////
// Add an object to the list
void CParser::AddObject(char *sText, property_t *cProperties, int iNumProperties, int iType, int iEnd)
{
	tag_object_t *obj;
//	int r,g,b;

	// Allocate a new object
	obj = new tag_object_t;
	if(obj == NULL) {
		// Out of memory
		Error(ERR_OUTOFMEMORY,"Out of memory");
		return;
	}

	// Allocate room for the text
	obj->strText = new char[strlen(sText)+1];
	if(obj->strText == NULL) {
		// Out of memory
		Error(ERR_OUTOFMEMORY,"Out of memory");
		return;
	}

	// Set the properties
	strcpy(obj->strText,sText);
	obj->iType = iType;
	obj->iEnd = iEnd;
	obj->iNumProperties = iNumProperties;
	obj->tNext = NULL;

	// Allocate the tag properties
	obj->cProperties = new property_t[iNumProperties];
	if (!obj->cProperties)  {
		// Out of memory
		Error(ERR_OUTOFMEMORY,"Out of memory");
		return;
	}

	// Copy the tag properties
	for (int i=0; i<iNumProperties; i++)  {
		obj->cProperties[i].iValue = cProperties[i].iValue;
		strcpy(obj->cProperties[i].sValue,cProperties[i].sValue);
		strcpy(obj->cProperties[i].sName,cProperties[i].sName);
	}
		

	// Values get translated differently for different types of objects
	// End tags don't get translated
	/*if(!iEnd) {
		switch(iType) {

			// Normal integer
			case HTO_TAB:
			case HTO_STAB:
				obj->iValue = atoi(sVal);
				break;

			// Triple value
			case HTO_COLOUR:
			case HTO_BOX:
				char *tok = strtok(sVal,",");
				if(tok)	r = atoi(tok);	tok = strtok(NULL,",");
				if(tok)	g = atoi(tok);  tok = strtok(NULL,",");
				if(tok)	b = atoi(tok);

				obj->iValue = MakeColour(r,g,b);
				break;
		}
	}*/


	// Add this object to the list
	if(tObjects) {
		tag_object_t *o = tObjects;
		for(;o; o = o->tNext) {
			if(o->tNext == NULL) {
				o->tNext = obj;
				break;
			}
		}
	} else
		tObjects = obj;
}

int CParser::GetIdByName(char *name)
{
 // TODO: get the widget id according to its name
	return 0;	
}

/////////////////////
// Creates widgets specified in the loaded file
void CParser::BuildLayout(CGuiLayout *Layout)
{
	tag_object_t *obj = tObjects;

	// Go through all objects
	for(;obj;obj=obj->tNext) {

	// Button
	if(!strcmp(obj->strText,"button")) {
		int left,top,width,height;
		char onclick[32],name[32];

		for (int i=0; i<obj->iNumProperties; i++)  {
			if (strcmp(obj->cProperties[i].sName,"left"))  left = obj->cProperties[i].iValue;
			else if (strcmp(obj->cProperties[i].sName,"top"))   top = obj->cProperties[i].iValue;
			else if (strcmp(obj->cProperties[i].sName,"width")) width = obj->cProperties[i].iValue;
			else if (strcmp(obj->cProperties[i].sName,"height"))  height = obj->cProperties[i].iValue;
			else if (strcmp(obj->cProperties[i].sName,"onclick"))  strcpy(onclick,obj->cProperties[i].sValue);
			else if (strcmp(obj->cProperties[i].sName,"name"))  strcpy(name,obj->cProperties[i].sValue);
			else Error(ERR_UNKNOWNPROPERTY,"Unknown property %s in tag %s",obj->cProperties[i].sName,obj->strText);
		}

		Layout->Add(new CButton(),GetIdByName(name),left,top,width,height);
	}

	// Checkbox
	else if(!strcmp(obj->strText,"checkbox")) {
		int left,top,width,height;
		char onclick[32],name[32];

		for (int i=0; i<obj->iNumProperties; i++)  {
			if (strcmp(obj->cProperties[i].sName,"left"))  left = obj->cProperties[i].iValue;
			else if (strcmp(obj->cProperties[i].sName,"top"))   top = obj->cProperties[i].iValue;
			else if (strcmp(obj->cProperties[i].sName,"width")) width = obj->cProperties[i].iValue;
			else if (strcmp(obj->cProperties[i].sName,"height"))  height = obj->cProperties[i].iValue;
			else if (strcmp(obj->cProperties[i].sName,"onclick"))  strcpy(onclick,obj->cProperties[i].sValue);
			else if (strcmp(obj->cProperties[i].sName,"name"))  strcpy(name,obj->cProperties[i].sValue);
			else Error(ERR_UNKNOWNPROPERTY,"Unknown property %s in tag %s",obj->cProperties[i].sName,obj->strText);
		}

		Layout->Add(new CCheckbox(false),GetIdByName(name),left,top,width,height);
	}

	// TODO: other tags



	}

}



///////////////////
// Render the browser
/*void CBrowser::Draw(SDL_Surface *bmpDest)
{
	int x,y,s,p,w,c;
	ht_object_t *obj = tObjects;
	CFont *fnt = &tLX->cFont;
	char buf[64];
	int lcount = 0;

	DrawRectFill(bmpDest, iX+1, iY+1, iX+iWidth-1, iY+iHeight-1, 0xffff);
	
	Menu_DrawBoxInset(bmpDest, iX, iY, iX+iWidth, iY+iHeight);

	// Setup the clipping rectangle
	SDL_Rect r;
	r.x = iX+2;
	r.y = iY+2;
	r.w = iWidth-3;
	r.h = iHeight-3;
	SDL_SetClipRect(bmpDest,&r);


	// 10 pixel margin
	int Margin = 6;
	x=iX + Margin;
	y=iY + Margin - cScrollbar.getValue();

	int RightWidth = iX + iWidth - Margin;
	if(iUseScroll)
		RightWidth -= 14;


	iProperties=0;
	iTextColour = 0;

	if(iUseScroll)
		cScrollbar.Draw(bmpDest);

	// Go through rendering each object
	for(;obj;obj=obj->tNext) {

		if(y > iY+iHeight+Margin && iLines>=0)
			break;

		switch(obj->iType) {

			// Property - Bold
			case HTO_BOLD:
				if(obj->iEnd)
					iProperties &= ~PRP_BOLD;
				else
					iProperties |= PRP_BOLD;
				break;

			// Property - Underline
			case HTO_UNDERLINE:
				if(obj->iEnd)
					iProperties &= ~PRP_UNDERLINE;
				else
					iProperties |= PRP_UNDERLINE;
				break;

			// Property - Shadow
			case HTO_SHADOW:
				if(obj->iEnd)
					iProperties &= ~PRP_SHADOW;
				else
					iProperties |= PRP_SHADOW;
				break;

			// Colour
			case HTO_COLOUR:
				iTextColour = obj->iValue;
				break;

			// Tab
			case HTO_TAB:
				s =	fnt->GetWidth(" ");
				p = x + (obj->iValue*4)*s;
				p/=4;
				p*=4;
				x = p;
				break;

			// Start line Tab
			case HTO_STAB:
				s =	fnt->GetWidth(" ");
				p = (obj->iValue*4)*s;
				p/=4;
				p*=4;
				x = iX+p;
				break;

			// Line
			case HTO_LINE:
				if(y > iY-fnt->GetHeight() && iLines>=0) {
					DrawHLine(bmpDest,iX+Margin,RightWidth,y,MakeColour(28,36,47));
					DrawHLine(bmpDest,iX+Margin,RightWidth,y+1,MakeColour(50,65,82));
				}
				break;

			// Box
			case HTO_BOX:
				if(y > iY-fnt->GetHeight() && iLines>=0)
					DrawRectFill(bmpDest,iX+Margin,y,RightWidth,y+fnt->GetHeight(),obj->iValue);
				break;

			// Newline
			case HTO_NEWLINE:
				x = iX + Margin;
				y += fnt->GetHeight();
				lcount++;
				break;
			

			// Text
			case HTO_TEXT:

				// Go through each word in the text
				// If a word is going to be over the window border, move the word down a line and
				// to the start

				p=0;
				s = strlen(obj->strText);
				while(p<s) {

					// Go through until a space
					for(c=p;c<s;c++) {
						if(obj->strText[c] == ' ') {
							c++;
							break;
						}
					}
					strncpy(buf,obj->strText+p,c-p);
					buf[c-p]='\0';
					p=c;

					w = fnt->GetWidth(buf);
					if(x+w > RightWidth) {
						// Drop down a line
						x = iX+Margin;
						y += fnt->GetHeight();
						lcount++;
					}

					// Make sure it's within the window
					if(y > iY-fnt->GetHeight() && iLines>=0) {

						// Draw the properties
						if(iProperties & PRP_SHADOW)
							fnt->Draw(bmpDest,x+1,y+1,MakeColour(200,200,200),"%s",buf);					
						if(iProperties & PRP_BOLD)
							fnt->Draw(bmpDest,x+1,y+1,iTextColour,"%s",buf);
						if(iProperties & PRP_UNDERLINE)
							DrawHLine(bmpDest,x,x+w,y+fnt->GetHeight()-1,iTextColour);
				
						// Draw the text
						fnt->Draw(bmpDest,x,y,iTextColour,"%s",buf);
					}
				
					x+=w;
				}
				break;
		}
	}


	// If this is the first render, set the scrollbar to show the number of lines
	if(iLines == -1) {
		cScrollbar.setMax((lcount+1) * fnt->GetHeight() + Margin);
		iLines = lcount;

		if(cScrollbar.getMax() > iHeight)
			iUseScroll = true;

		//printl("Browser size: %d",iHeight);
		//printl("Page size: %d",lcount * fnt->GetHeight());
	}


	// Reset the clipping rectangle to full size
	SDL_SetClipRect(bmpDest,NULL);
}*/