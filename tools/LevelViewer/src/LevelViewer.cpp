#include <wx/wx.h>
#include <wx/dnd.h>
#include <wx/cmdline.h>
#include <zlib.h>

// Some basic defines
#define		TITLE wxString(_T("LieroX Level Viewer"))

// Map defines
#define		MAP_VERSION	0
#define		MPT_PIXMAP	0
#define		MPT_IMAGE	1
#define		PX_EMPTY	0x01
#define		PX_DIRT		0x02
#define		PX_ROCK		0x04

// Main application class
class LevelViewerApp : public wxApp
{
private:
	wxFrame *mainFrame;
	wxString commandLineFile;
public:

	virtual bool OnInit();
    virtual void OnInitCmdLine(wxCmdLineParser& parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
};

// Simple map class
class Map  {
private:
	int width, height;
	float previewSize;
	wxString fileName;
	wxString fileDir;
	bool shadows;
	bool loaded;
	wxString name;
	wxImage back;
	wxImage front;
	wxImage preview; // Possibly resized preview
	wxImage preview100; // Preview in 100 %
	wxByte *pixelFlags;

private:
	void ApplyShadow();
	void GeneratePreview();

public:
	Map() : width(0), height(0), previewSize(1), shadows(false), loaded(false), pixelFlags(NULL) {}
	~Map() { Close(); }

	void Close();
	void Open(const wxString& name);
	void SavePreview(const wxString& path);

	bool getShadows()  { return shadows; }
	void setShadows(bool _s);

	const wxString& getName() { return name; }
	const wxImage& getPreview() { return preview; }
	int getWidth() { return width; }
	int getHeight() { return height; }
	bool isLoaded() { return loaded; }
	float getPreviewSize() { return previewSize; }
	bool setPreviewSize(float _s); // Returns true if the preview image has been changed
	const wxString& getFileName()  { return fileName; }
	const wxString& getFileDirectory()	{ return fileDir; }
};

// Basic functions
int max(const int vals[], int len)
{
	int res = vals[0];
	for (size_t i = 1; i < len; i++)
		if (vals[i] > res)
			res = vals[i];

	return res;
}

// Widget IDs
enum {
	lc_OpenLevel,
	lc_CloseLevel,
	lc_SavePreview,
	lc_Exit,
	lc_Shadows,
	lc_50,
	lc_100,
	lc_200,
	lc_Panel
};

// Simple exception class
class Exception {
public:
	Exception() {}
	Exception(const wxString& msg) : message(msg) {}
	wxString message;
};

// Class that handles file drag & drop
class FileDrop : public wxFileDropTarget  {
private:
	wxFrame *owner;

public:
	FileDrop(wxFrame *owner)  { this->owner = owner; }
	bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
};

// Image preview class
class ImageView : public wxScrolledWindow  {
    private:
    wxBitmap *m_bitmap;

    public:
    ImageView(wxWindow *parent) : wxScrolledWindow(parent)
    {
        m_bitmap = NULL;
    }

    ~ImageView()
    {
        if (m_bitmap)
            delete m_bitmap;
    }


    void setImage(const wxImage& img)
    {
        if (m_bitmap)
            delete m_bitmap;
        m_bitmap = new wxBitmap(img);

        SetScrollbars(1, 1, img.GetWidth(), img.GetHeight(), 0, 0, true);
    }

    void OnDraw(wxDC& dc)
    {
        wxScrolledWindow::OnDraw(dc);
        if (m_bitmap)
            dc.DrawBitmap(*m_bitmap, 0, 0, false);
    }

    void clear()
    {
        if (m_bitmap)
            delete m_bitmap;
        m_bitmap = NULL;
    }
};

// Main window
class LevelViewerFrame : public wxFrame
{
private:

public:
	LevelViewerFrame(const wxString& title);
	virtual ~LevelViewerFrame();

private:
	// Components
	wxMenuBar *mnuMain;
	wxMenu *mnuFile, *mnuView;
	FileDrop *drpMain;

	//wxPanel *pnlMain;
	ImageView *imgLevel;

	wxFileDialog *dlgOpenLevel;
	wxFileDialog *dlgSavePreview;

	// Others
	Map *map;

public:
	void OpenLevel(const wxString& path);
	void SizeAndRefresh();

// Events
protected:

	// Events
	void OnOpenLevelClick(wxCommandEvent& evt);
	void OnSavePreviewClick(wxCommandEvent& evt);
	void OnCloseLevelClick(wxCommandEvent& evt);
	void OnExitClick(wxCommandEvent& evt);
	void OnShadowsChange(wxCommandEvent& evt);
	void OnPreviewSizeClick(wxCommandEvent& evt);
	void OnClose(wxCloseEvent& evt);
	void OnKeyDown(wxKeyEvent& evt);

	// wxWidgets stuff
	DECLARE_EVENT_TABLE()
};

// Register events
BEGIN_EVENT_TABLE(LevelViewerFrame, wxFrame)
EVT_MENU(lc_OpenLevel, LevelViewerFrame::OnOpenLevelClick)
EVT_MENU(lc_SavePreview, LevelViewerFrame::OnSavePreviewClick)
EVT_MENU(lc_CloseLevel, LevelViewerFrame::OnCloseLevelClick)
EVT_MENU(lc_Exit, LevelViewerFrame::OnExitClick)
EVT_MENU(lc_Shadows, LevelViewerFrame::OnShadowsChange)
EVT_MENU(lc_50, LevelViewerFrame::OnPreviewSizeClick)
EVT_MENU(lc_100, LevelViewerFrame::OnPreviewSizeClick)
EVT_MENU(lc_200, LevelViewerFrame::OnPreviewSizeClick)
EVT_CLOSE(LevelViewerFrame::OnClose)
EVT_CHAR_HOOK(LevelViewerFrame::OnKeyDown)
END_EVENT_TABLE()

// Initialize the application
bool LevelViewerApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	mainFrame = new LevelViewerFrame(TITLE);
	if (commandLineFile.size() != 0) // Load the level specified by commandline
		((LevelViewerFrame *)mainFrame)->OpenLevel(commandLineFile);
	mainFrame->Show();

	return true;
}

// Commandline initialization
void LevelViewerApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	static const wxCmdLineEntryDesc cmdLineDesc [] =
	{
		{ wxCMD_LINE_PARAM, NULL, NULL, "input file", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
		{ wxCMD_LINE_NONE }
	};

	parser.SetDesc(cmdLineDesc);
}

// Parse the commandline parameters
bool LevelViewerApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	if (parser.GetParamCount() > 0)  {
		wxString path, name, ext;
		wxSplitPath(parser.GetParam(0), &path, &name, &ext);
		if (ext.CmpNoCase(_T("lxl")) != 0)
			return true;
		commandLineFile = parser.GetParam(0);
	}

	return true;
}

// Window constructor
LevelViewerFrame::LevelViewerFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title,
                      wxPoint(0, 50), wxDefaultSize,
					  wxCAPTION | wxCLOSE_BOX | wxMINIMIZE_BOX | wxSYSTEM_MENU |
                      wxNO_FULL_REPAINT_ON_RESIZE |
                      wxCLIP_CHILDREN |
                      wxTAB_TRAVERSAL)

{
	// Allocate the components
	map = new Map();
	//pnlMain = new wxPanel(this);
	imgLevel = new ImageView(this);
	mnuMain = new wxMenuBar();
	mnuFile = new wxMenu();
	mnuView = new wxMenu();
	drpMain = new FileDrop(this);

	// Setup the menu
	mnuFile->Append(lc_OpenLevel, _T("&Open Level...\tCtrl+O"));
	mnuFile->Append(lc_SavePreview, _T("&Save Preview Image...\tCtrl+S"));
	mnuFile->Append(lc_CloseLevel, _T("&Close"));
	mnuFile->AppendSeparator();
	mnuFile->Append(lc_Exit, _T("&Exit"));
	mnuView->AppendCheckItem(lc_Shadows, _T("S&hadows\tS"));
	mnuView->AppendSeparator();
	mnuView->AppendRadioItem(lc_50, _T("&50 %"));
	mnuView->AppendRadioItem(lc_100, _T("&100 %"));
	mnuView->AppendRadioItem(lc_200, _T("&200 %"));

	mnuMain->Append(mnuFile, _T("&File"));
	mnuMain->Append(mnuView, _T("&View"));

	// Disable specific menu items
	mnuView->Enable(lc_Shadows, false);
	mnuFile->Enable(lc_CloseLevel, false);
	mnuFile->Enable(lc_SavePreview, false);
	mnuView->Enable(lc_50, false);
	mnuView->Enable(lc_100, false);
	mnuView->Enable(lc_200, false);
	mnuView->Check(lc_100, true);

	// Setup the menu and accept file drag & drop
	SetMenuBar(mnuMain);
	SetDropTarget(drpMain);

	// Allocate the dialogs
	dlgOpenLevel = new wxFileDialog(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString,
					_T("LieroX Levels (*.lxl)|*.lxl|All files (*.*)|*.*"));
	dlgSavePreview = new wxFileDialog(this, _T("Save to"), wxEmptyString, wxEmptyString,
					_T("PNG Images (*.png)|*.png|Bitmap Images (*.bmp)|*.bmp|JPG Images (*.jpg)|*.jpg|PCX Images (*.pcx)|*.pcx|Tiff Images (*.tif, *.tiff)|*.tif;*.tiff|PNM Images (*.pnm)|*.pnm|XPM Images (*.xpm)|*.xpm|All files (*.*)|*.*"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);


	SizeAndRefresh();
}

LevelViewerFrame::~LevelViewerFrame()
{
	// Free the components
	delete imgLevel;
	delete dlgOpenLevel;
	delete dlgSavePreview;

	// Other stuff
	delete map;
}

// Opens a level
void LevelViewerFrame::OpenLevel(const wxString &path)
{
	try  {
		// Open the map
		map->setShadows(mnuView->IsChecked(lc_Shadows));
		map->setPreviewSize(1.0f);
		mnuView->Check(lc_100, true); // Reset to default size
		map->Open(path);
		//pnlMain->SetSize(map->getPreview().GetWidth(), map->getPreview().GetHeight());
		imgLevel->SetSize(map->getPreview().GetWidth(), map->getPreview().GetHeight());
		imgLevel->setImage(map->getPreview());

		// Enable the menu items
		mnuView->Enable(lc_Shadows, true);
		mnuView->Enable(lc_50, true);
		mnuView->Enable(lc_100, true);
		mnuView->Enable(lc_200, true);
		mnuFile->Enable(lc_CloseLevel, true);
		mnuFile->Enable(lc_SavePreview, true);

		// Set the window title
		SetTitle(TITLE + _T(" - ") + map->getName() + _T(" (") +
				wxString::Format(_T("%i"), map->getWidth()) + _T("x") + wxString::Format(_T("%i"), map->getHeight()) + _T(")"));

        SizeAndRefresh();
	} catch (Exception &e) {
		map->Close();
		wxMessageBox(_T("The level could not be opened:\n" + e.message), _T("Error"));
	}
}

void LevelViewerFrame::SizeAndRefresh()
{
    if (map && map->isLoaded())  {
        SetClientSize(map->getPreview().GetWidth() + 20, map->getPreview().GetHeight() + 20);
        imgLevel->SetClientSize(map->getPreview().GetWidth(), map->getPreview().GetHeight());
        SetClientSize(imgLevel->GetSize().GetWidth(), imgLevel->GetSize().GetHeight());
    } else  {
        imgLevel->SetSize(320, 240);
        SetClientSize(imgLevel->GetSize());
    }
    CentreOnScreen();
    Refresh();
}

// Open level click event
void LevelViewerFrame::OnOpenLevelClick(wxCommandEvent &evt)
{
	// Let the user choose the file
	if (dlgOpenLevel->ShowModal() == wxID_OK)  {
		OpenLevel(dlgOpenLevel->GetPath());
	}
}

// Preview save event
void LevelViewerFrame::OnSavePreviewClick(wxCommandEvent &evt)
{
	// Let the user choose the path to save to
	dlgSavePreview->SetDirectory(map->getFileDirectory());  // Use the directory of the current level
	dlgSavePreview->SetFilename(_T(""));
	if (dlgSavePreview->ShowModal() == wxID_OK)  {
		try  {
			map->SavePreview(dlgSavePreview->GetPath());
		} catch (Exception &e) {
			wxMessageBox(_T("An error occured while saving:\n") + e.message, _T("Error"));
		}
	}
}

// Close the level
void LevelViewerFrame::OnCloseLevelClick(wxCommandEvent &evt)
{
	map->Close();

	// Disable menu items
	mnuView->Enable(lc_Shadows, false);
	mnuView->Enable(lc_50, false);
	mnuView->Enable(lc_100, false);
	mnuView->Enable(lc_200, false);
	mnuFile->Enable(lc_CloseLevel, false);
	mnuFile->Enable(lc_SavePreview, false);

	imgLevel->clear();

	// Update size & title
	SetSize(320, 240);
	CentreOnScreen();
	SetTitle(TITLE);
	Refresh();
}

// Key down event
void LevelViewerFrame::OnKeyDown(wxKeyEvent& evt)
{
	// Close on escape
	if (evt.GetKeyCode() == WXK_ESCAPE)
		Close();
}

// Exit event
void LevelViewerFrame::OnExitClick(wxCommandEvent &evt)
{
	// Quit :)
	Close();
}

// Change the shadow displaying
void LevelViewerFrame::OnShadowsChange(wxCommandEvent &evt)
{
	if (map->isLoaded())  {
		map->setShadows(mnuView->IsChecked(lc_Shadows));
		imgLevel->setImage(map->getPreview());
		Refresh();
	}
}

// Change preview size event
void LevelViewerFrame::OnPreviewSizeClick(wxCommandEvent& evt)
{
	bool changed = false;
	switch (evt.GetId())  {
	case lc_50:
		changed = map->setPreviewSize(0.5f);
		break;
	case lc_100:
		changed = map->setPreviewSize(1.0f);
		break;
	case lc_200:
		changed = map->setPreviewSize(2.0f);
		break;
	default:
		changed = map->setPreviewSize(1.0f);
	}

	// If the size has changed, update the window width and the cache
	if (changed)  {
		//pnlMain->SetSize(map->getPreview().GetWidth(), map->getPreview().GetHeight())
        imgLevel->setImage(map->getPreview());
		SizeAndRefresh();
	}
}


// Close event
void LevelViewerFrame::OnClose(wxCloseEvent& evt)
{
	evt.Skip(true); // To let the application quit
}

// File drop event
bool FileDrop::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
	if (filenames.size() == 0)
		return false;

	// Check the extension
	wxString path, filename, ext;
	wxSplitPath(filenames[0], &path, &filename, &ext);
	if (ext.CmpNoCase(_T("lxl")) != 0)
		return false;

	// Open
	((LevelViewerFrame *)owner)->OpenLevel(filenames[0]);

	// Accept the files
	return true;
}

// Opens the map
void Map::Open(const wxString& level)
{
	// Close first
	Close();

	// Open the input
	FILE *fp = wxFopen(level, _T("rb"));
	if (!fp)
		throw Exception(_T("Could not open the input file."));

	// Verify ID and version
	char id[32];
	wxUint32 version;
	fread(id, sizeof(id), 1, fp);
	fread(&version, 4, 1, fp);
	wxUINT32_SWAP_ON_BE(version);
	id[31] = '\0';
	if (version != 0 || (strncmp(id, "LieroX Level", sizeof(id)) != 0 && strncmp(id, "LieroX CTF Level", sizeof(id)) != 0))  {
		fclose(fp);
		throw Exception(_T("Invalid level file."));
	}

	// Name
	char c_name[64];
	fread(c_name, sizeof(c_name), 1, fp);
	c_name[63] = '\0';
	for (int i=0; c_name[i]; ++i)
		name += c_name[i];

	// Width, height and type
	wxUint32 m_width, m_height, m_type;
	fread(&m_width, 4, 1, fp);
	fread(&m_height, 4, 1, fp);
	fread(&m_type, 4, 1, fp);
	wxUINT32_SWAP_ON_BE(m_width);
	wxUINT32_SWAP_ON_BE(m_height);
	wxUINT32_SWAP_ON_BE(m_type);
	width = m_width;
	height = m_height;

	// Verify the type
	if (m_type != MPT_IMAGE)  {
		fclose(fp);
		throw Exception(_T("The pixmap format cannot be currently opened."));
	}

	// Skip theme name + number of objects
	fseek(fp, ftell(fp) + 32 + 4, SEEK_SET);

	// Data sizes
	wxUint32 size, destSize;
	fread(&size, 4, 1, fp);
	fread(&destSize, 4, 1, fp);
	wxUINT32_SWAP_ON_BE(size);
	wxUINT32_SWAP_ON_BE(destSize);

	// Read the data
	wxByte *source = new wxByte[size];
	wxByte *dest = new wxByte[destSize];
	fread(source, size, 1, fp);

	// Close the file
	fclose(fp);

	// Uncompress the data
	uLongf uncompressedSize = destSize;
	if (uncompress(dest, &uncompressedSize, source, size) != Z_OK)  {
		delete[] source;
		delete[] dest;
		throw Exception(_T("Could not uncompress images."));
	}

	delete[] source; // Not needed anymore

	// Read the images
	back.Create(width, height, false);
	front.Create(width, height, false);

	const wxUint32 imageSize = width * height * 3; // 3 - one byte for each color component
	if (destSize < (imageSize * 2 + width * height))  { // Check if dest is big enough (2 images + pixel flags)
		delete[] dest;
		throw Exception(_T("The data is corrupted."));
	}
	memcpy(back.GetData(), dest, imageSize);
	memcpy(front.GetData(), dest + imageSize, imageSize);

	// Create the preview image according to the pixel flags
	pixelFlags = new wxByte[width * height];
	memcpy(pixelFlags, &dest[2 * imageSize], width * height);
	delete[] dest; // Cleanup
	GeneratePreview();
	loaded = true;

	// Filename + directory
	wxString filename, ext;
	wxSplitPath(level.c_str(), &fileDir, &filename, &ext);
	fileName = filename + ext;
}

// Closes the map
void Map::Close()
{
	if (pixelFlags)
		delete pixelFlags;
	width = height = 0;
	pixelFlags = NULL;
	front.Destroy();
	back.Destroy();
	preview.Destroy();
	shadows = false;
	loaded = false;
}

// Enables or disables shadows
void Map::setShadows(bool _s)
{
	if (shadows == _s)
		return;

	shadows = _s;
	if (loaded)
		GeneratePreview();
}

// Changes the preview size
bool Map::setPreviewSize(float _s)
{
	if (previewSize == _s)
		return false;

	previewSize = _s;
	if (loaded)  {
		// Resize the preview image
		if (_s != 1.0f)
			preview = preview100.ResampleBox((int)((float)width * previewSize), (int)((float)height * previewSize));
		else
			preview = preview100;
		return true;
	}

	return false;
}

// Generates the map preview
void Map::GeneratePreview()
{
	preview100.Create(width, height, false);

	wxByte *pf = pixelFlags;
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)  {
			switch (*pf)  {
			case PX_EMPTY:  // Empty
				memcpy(&preview100.GetData()[(y * width + x) * 3], &back.GetData()[(y * width + x) * 3], 3);
				break;
			case PX_ROCK: // Rock
			case PX_DIRT: // Dirt
				memcpy(&preview100.GetData()[(y * width + x) * 3], &front.GetData()[(y * width + x) * 3], 3);
				break;
			default: // Unknown, defaults to empty
				memcpy(&preview100.GetData()[(y * width + x) * 3], &back.GetData()[(y * width + x) * 3], 3);
			}

			++pf;
		}

	// Apply the shadows if wanted
	if (shadows)
		ApplyShadow();

	// Resize according to the ratio
	if (previewSize != 1.0f)
		preview = preview100.ResampleBox((int)((float)width * previewSize), (int)((float)height * previewSize));
	else
		preview = preview100;
}

// Apply a shadow to the map
void Map::ApplyShadow()
{
	static const int drop = 3;

	if (!loaded)
		return;

	for(int y = 0; y < height; y++) {

		wxByte *px = pixelFlags + y * width;

		for(int x = 0; x < width; x++) {

			wxByte flag = *px;

			if(!(flag & PX_EMPTY)) {
				int ox = x+1;
				int oy = y+1;

				// Draw the shadow
				for(int n = 0; n < drop; n++) {

					// Clipping
					if(ox >= width) break;
					if(oy >= height) break;

					wxByte *p = pixelFlags + oy * width + ox;
					if(!( (*(wxByte *)p) & PX_EMPTY))
						break;

					wxByte *pixel = &preview100.GetData()[oy * width * 3 + ox * 3];
					pixel[0] = (wxByte)(((int)pixel[0] * 3) / 4);
					pixel[1] = (wxByte)(((int)pixel[1] * 3) / 4);
					pixel[2] = (wxByte)(((int)pixel[2] * 3) / 4);

					ox++; oy++;
				}
			}

			px++;
		}
	}
}

// Saves the preview image
void Map::SavePreview(const wxString &path)
{
	if (path.size() == 0)
		throw Exception(_T("Invalid path"));

	if (!loaded)
		throw Exception(_T("No level is opened"));

	wxInitAllImageHandlers();

	if (!preview.SaveFile(path))
		throw Exception(_T("Could not save the file"));
}

// wxWidgets stuff
IMPLEMENT_APP(LevelViewerApp)
