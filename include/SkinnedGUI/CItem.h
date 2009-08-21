/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Item class
// Created 23/5/08
// Karel Petranek

#ifndef __CITEM_H__SKINNED_GUI__
#define __CITEM_H__SKINNED_GUI__

#include <libxml/parser.h>
#include <string>
#include "SkinnedGUI/CBackground.h"
#include "SkinnedGUI/CBorder.h"
#include "FontHandling.h"
#include "SmartPointer.h"


namespace SkinnedGUI {

enum { 
	IMAGE_SPACING = 3,
	ITEM_SPACING = 2
};

// This class is a generalization of Listview Item, Combobox Item and
// Menu Item

// Item structure
class CItem { 
public:
	CItem(const std::string& name);
	CItem(const std::string& sindex, const std::string& name);
	CItem(const std::string& name, SmartPointer<SDL_Surface> image);
	CItem(const std::string& sindex, SmartPointer<SDL_Surface> image, const std::string& name);
	virtual ~CItem() {}

protected:
	// Attributes
	std::string		sIndex;
	std::string		sName;
	std::string		sCSSID;
	std::string		sCSSClass;
	SmartPointer<SDL_Surface> tImage;
	int				iTag;
	bool			bActive;
	bool			bDown;
	bool			bInlineStyle;
	StyleVar<TextVAlignment> iVAlign;

public:
	// Style of the item
	class CItemStyle  { public:
		CBackground cBackground;
		CBorder cBorder;
		CFontStyle cFont;
		CTextProperties cText;

		virtual CItemStyle& operator=(const CItemStyle& s)  {
			if (&s != this)  {
				cBackground = s.cBackground;
				cBorder = s.cBorder;
				cFont = s.cFont;
				cText = s.cText;
			}
			return *this;
		}

		virtual void ApplySelector(const CSSParser::Selector& sel, const std::string& prefix = "");
		virtual ~CItemStyle() {}
	};

protected:
	// Drawing
	virtual void DrawItemText(SDL_Surface *bmpDest, const SDL_Rect& itemr);
	virtual void DrawItemImage(SDL_Surface *bmpDest, const SDL_Rect& itemr);
	virtual void RepaintParent() = 0;

private:
	void	setVAlignByString(const std::string& str, size_t priority);

public:
	// Style
	CItemStyle cNormalStyle;
	CItemStyle cActiveStyle;
	CItemStyle cClickedStyle;

protected:
	// Operators
	void CopyInfoTo(CItem& i2) const  {
		if (&i2 != this)  {
			i2.sIndex = sIndex;
			i2.sName = sName;
			i2.tImage = tImage;
			i2.bActive = bActive;
			i2.bDown = bDown;
			i2.sCSSClass = sCSSClass;
			i2.sCSSID = sCSSID;
			i2.bInlineStyle = bInlineStyle;
			i2.cNormalStyle = cNormalStyle;
			i2.cActiveStyle = cActiveStyle;
			i2.cClickedStyle = cClickedStyle;
		}
	}

public:

	// Attributes
	virtual int getHeight();
	virtual int getWidth();
	virtual CItemStyle *getCurrentStyle();
	bool isActive()	const						{ return bActive; }
	void setActive(bool _s)						{ bActive = _s; }
	bool isDown() const							{ return bDown; }
	void setDown(bool _d)						{ bDown = _d; }
	const std::string& getName() const			{ return sName; }
	void setName(const std::string& name)		{ sName = name; RepaintParent(); }
	const std::string& getSIndex() const		{ return sIndex; }
	void setSIndex(const std::string& sindex)	{ sIndex = sindex; }
	SmartPointer<SDL_Surface> getImage() const	{ return tImage; }
	void setImage(SmartPointer<SDL_Surface> img){ tImage = img; RepaintParent(); }
	int getTag() const							{ return iTag; }
	void setTag(int t)							{ iTag = t; }
	TextVAlignment getAlign() const				{ return iVAlign; }
	void setItemVAlign(TextVAlignment _v)		{ iVAlign.set(_v, HIGHEST_PRIORITY); RepaintParent(); }
	const std::string& getCSSClass() const		{ return sCSSClass; }
	void setCSSClass(const std::string& c)		{ sCSSClass = c; }
	const std::string& getCSSID() const			{ return sCSSID; }
	void setCSSID(const std::string& id)		{ sCSSID = id; }
	bool hasInlineStyle()						{ return bInlineStyle; }

	virtual void ApplyTag(xmlNodePtr node);

	virtual CItem *Clone() const = 0;

	// Methods
	virtual void Draw(SDL_Surface *bmpDest, const SDL_Rect& r) = 0;
};

}; // namespace SkinnedGUI

#endif // __CITEM_H__
