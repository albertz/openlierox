// Panel class

#include "CBasicContainer.h"

class CPanel : public CBasicContainer  {
public:
	// Contructor
	CPanel() {}

protected:
	void			Repaint();

private:
	CSurface		bmpBackground;
	CBorder			cBorder;

public:
	CBorder&		getBorder()					 { return cBorder; }

	void			Draw(CSurface &dest);
	void			Setup(int x, int y, int w, int h);
};