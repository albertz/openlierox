// Tabs class

void CTabs::Draw(CSurface &dest)
{
	// Buffer not yet initialized...
	if (!tBuffer)
		return;

	if (bRepaintRequired)
		Repaint();

	dest.DrawImage(tBuffer, 0, 0);
}

void CTabs::Repaint()
{
	for (ContainerList::iterator c = tChildren.begin(); c != tChildren.end(); c++)  {
		if ( (*c)->bFocused )
			(*c)->Draw(tBuffer);
		else
			((CSheet *) (*c))->DrawTab(tBuffer);
	}
}

void CTabs::Setup(int x, int y, int w, int h)
{
	for (ContainerList::iterator c = tChildren.begin(); c != tChildren.end(); c++)
		(*c)->Setup(x, y, w, h);
}

void CTabs::setActiveTab(int _p)
{
	iPage = CLAMP(_p, 0, (int)tChildren.size() - 1);

	int i = 0;
	for (ContainerList::iterator c = tChildren.begin(); c != tChildren.end(); c++, i++)  {
		if (i == iPage)  {
			(*c)->bFocused = true;
			break;
		}
	}
}

void CTabs::AddSheet(CSheet *sheet)
{
	int width = 0;
	for (ContainerList::iterator c = tChildren.begin(); c != tChildren.end(); c++)  {
		((CSheet *) (*c))->get

	}

	AddChild(sheet);
}