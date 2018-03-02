#include <wx/wx.h>
#include <wx/cmdline.h>
#include <zlib.h>

// Some basic defines
#define WINDOW_WIDTH 300
#define BROWSE_BTN_WIDTH 30
#define SPACING 10

// Map defines
#define		MAP_VERSION	0
#define		MPT_PIXMAP	0
#define		MPT_IMAGE	1
#define		PX_EMPTY	0x01
#define		PX_DIRT		0x02
#define		PX_ROCK		0x04 

// Main application class
class LevelDecompilerApp : public wxApp
{
	virtual bool OnInit();
	virtual void OnInitCmdLine(wxCmdLineParser& parser);
	virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
	
	wxString commandLineFile;
};

// The function that does all the work :)
void DecompileLevel(const wxString& level, const wxString& outpath);

// Basic functions
int max(const int vals[], int len)
{
	int res = vals[0];
	for (int i = 1; i < len; i++)
		if (vals[i] > res) 
			res = vals[i];

	return res;
}

bool equal(const int vals[], int len)
{
	int first = vals[0];
	for (int i = 1; i < len; i++)
		if (vals[i] != first) 
			return false;

	return true;	
}

// Widget IDs
enum {
	lc_BrowseLevel,
	lc_Level,
	lc_Decompile,
	lc_Exit,
	lc_Panel
};

// Simple exception class
class Exception {
public:
	Exception() {}
	Exception(const wxString& msg) : message(msg) {}
	wxString message;
};

// Main window
class LevelDecompilerFrame : public wxFrame
{
private:

public:
	LevelDecompilerFrame(const wxString& title);
	virtual ~LevelDecompilerFrame();

private:
	// Components
	wxButton *btnBrowseLevel;
	wxButton *btnDecompile;
	wxButton *btnExit;

	wxTextCtrl *txtLevel;

	wxStaticText *lblLevel;

	wxPanel *pnlMain;

	wxFileDialog *dlgSelectFile;
	wxFileDialog *dlgSaveFile;

// Events
protected:
	// Events
	void OnBrowseLevelClick(wxCommandEvent& evt);
	void OnDecompileClick(wxCommandEvent& evt);
	void OnExitClick(wxCommandEvent& evt);

	// wxWidgets stuff
	DECLARE_EVENT_TABLE()
};

// Register events
BEGIN_EVENT_TABLE(LevelDecompilerFrame, wxFrame)
EVT_BUTTON(lc_BrowseLevel, LevelDecompilerFrame::OnBrowseLevelClick)
EVT_BUTTON(lc_Decompile, LevelDecompilerFrame::OnDecompileClick)
EVT_BUTTON(lc_Exit, LevelDecompilerFrame::OnExitClick)
END_EVENT_TABLE()

// Initialize the application
bool LevelDecompilerApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;
	
	if( commandLineFile != _("") )
	{
		wxString outFile;
#ifdef WIN32
		outFile += _(".\\");
#else
		outFile += _("./");
#endif
		outFile += commandLineFile.Mid( 0, commandLineFile.Len() - 4 ) + _(".bmp");
		try {
			DecompileLevel( commandLineFile, outFile );
		} catch ( const Exception & e )	{
			printf("Error: %s\n", (const char *)e.message.mb_str());
		}
		return false;
	}
	
	wxFrame *frame = new LevelDecompilerFrame(_T("LieroX Level Decompiler"));
	frame->Show();

	return true;
}

void LevelDecompilerApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	static const wxCmdLineEntryDesc cmdLineDesc [] =
	{
		{ wxCMD_LINE_PARAM, NULL, NULL, "LevelFile.lxl", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
		{ wxCMD_LINE_NONE }
	};

	parser.SetDesc(cmdLineDesc);
	wxApp::OnInitCmdLine(parser);
}

bool LevelDecompilerApp::OnCmdLineParsed(wxCmdLineParser& parser)
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
LevelDecompilerFrame::LevelDecompilerFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title,
                      wxPoint(0, 50), wxDefaultSize,
					  wxCAPTION | wxCLOSE_BOX | wxSYSTEM_MENU |
                      wxNO_FULL_REPAINT_ON_RESIZE |
                      wxCLIP_CHILDREN |
                      wxTAB_TRAVERSAL)

{
	this->SetClientSize(WINDOW_WIDTH, 10);

	// Allocate the components
	pnlMain = new wxPanel(this);
	lblLevel = new wxStaticText(pnlMain, wxID_ANY, _T("Level:"));
	btnBrowseLevel = new wxButton(pnlMain, lc_BrowseLevel, _T("..."));
	btnDecompile = new wxButton(pnlMain, lc_Decompile, _T("&Decompile"));
	btnExit = new wxButton(pnlMain, lc_Exit, _T("&Exit"));
	txtLevel = new wxTextCtrl(pnlMain, lc_Level);
	dlgSelectFile = new wxFileDialog(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, 
					_T("LieroX Levels (*.lxl)|*.lxl|All files (*.*)|*.*"));
	dlgSaveFile = new wxFileDialog(this, _T("Save to"), wxEmptyString, wxEmptyString, 
					_T("Bitmap Images (*.bmp)|*.bmp|PNG Images (*.png)|*.png|JPG Images (*.jpg)|*.jpg|PCX Images (*.pcx)|*.pcx|Tiff Images (*.tif, *.tiff)|*.tif;*.tiff|PNM Images (*.pnm)|*.pnm|XPM Images (*.xpm)|*.xpm|All files (*.*)|*.*"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);


	// Set component positions
	pnlMain->SetSize(this->GetClientSize());
	const int clientWidth = pnlMain->GetClientSize().GetWidth();
	const int labelWidth = lblLevel->GetSize().GetWidth();

	btnBrowseLevel->SetSize(clientWidth - BROWSE_BTN_WIDTH - SPACING, SPACING, BROWSE_BTN_WIDTH, -1);
	txtLevel->SetSize(SPACING + labelWidth, btnBrowseLevel->GetPosition().y, btnBrowseLevel->GetPosition().x - 2*SPACING - labelWidth, btnBrowseLevel->GetSize().GetHeight());
	lblLevel->SetPosition(wxPoint(SPACING, txtLevel->GetPosition().y + (txtLevel->GetSize().GetHeight() - lblLevel->GetSize().GetHeight())/2));

	btnDecompile->SetPosition(wxPoint(clientWidth/2 - btnDecompile->GetSize().GetWidth() - SPACING/2, txtLevel->GetPosition().y + txtLevel->GetSize().GetHeight() + SPACING));
	btnExit->SetPosition(wxPoint(clientWidth/2 + SPACING/2, txtLevel->GetPosition().y + txtLevel->GetSize().GetHeight() + SPACING));

	// Setup dialog
	//dlgSelectFile->SetDirectory(wxGetHomeDir());

	// Setup dimensions and position
	this->SetClientSize(WINDOW_WIDTH, 3*SPACING + txtLevel->GetSize().GetHeight()+ btnDecompile->GetSize().GetHeight());
	this->CentreOnScreen();
}

LevelDecompilerFrame::~LevelDecompilerFrame()
{
	// Free the components
	delete lblLevel;
	delete btnBrowseLevel;
	delete btnDecompile;
	delete btnExit;
	delete txtLevel;
	delete pnlMain;
	delete dlgSelectFile;
	delete dlgSaveFile;
}

void LevelDecompilerFrame::OnBrowseLevelClick(wxCommandEvent &evt)
{
	// Let the user choose the file
	if (dlgSelectFile->ShowModal() == wxID_OK)  {
		txtLevel->SetLabel(dlgSelectFile->GetPath());
	}
}

void LevelDecompilerFrame::OnDecompileClick(wxCommandEvent &evt)
{
	// Let the user choose the path to save to
	dlgSaveFile->SetDirectory(dlgSelectFile->GetDirectory());
	dlgSaveFile->SetFilename(_T(""));
	if (dlgSaveFile->ShowModal() == wxID_OK)  {
		try  {
			DecompileLevel(txtLevel->GetLabelText(), dlgSaveFile->GetPath());
			wxMessageBox(_T("Level successfully decompiled!"), _T("Success"));
		} catch (Exception &e) {
			wxMessageBox(_T("An error occured while decompiling:\n") + e.message, _T("Decompiler error"));
		}
	}
}

void LevelDecompilerFrame::OnExitClick(wxCommandEvent &evt)
{
	// Quit :)
	Close();
}

void DecompileLevel(const wxString& level, const wxString& outpath)
{
	if (outpath.size() == 0)
		throw Exception(_T("Invalid output path."));

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

	// Skip name
	fseek(fp, ftell(fp) + 64, SEEK_SET);

	// Width, height and type
	wxUint32 width, height, type;
	fread(&width, 4, 1, fp);
	fread(&height, 4, 1, fp);
	fread(&type, 4, 1, fp);
	wxUINT32_SWAP_ON_BE(width);
	wxUINT32_SWAP_ON_BE(height);
	wxUINT32_SWAP_ON_BE(type);

	// Verify the type
	if (type != MPT_IMAGE)  {
		fclose(fp);
		throw Exception(_T("The pixmap format cannot be currently decompiled."));
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
	wxImage backImage(width, height, false);
	wxImage frontImage(width, height, false);

	const wxUint32 imageSize = width * height * 3; // 3 - one byte for each color component
	if (destSize < (imageSize * 2 + width * height))  { // Check if dest is big enough (2 images + pixel flags)
		delete[] dest;
		throw Exception(_T("The data is corrupted."));
	}
	memcpy(backImage.GetData(), dest, imageSize);
	memcpy(frontImage.GetData(), dest + imageSize, imageSize);

	// Convert the pixel flags to a material image
	static const wxByte black[3] = {0, 0, 0};
	static const wxByte grey[3] = {128, 128, 128};
	static const wxByte white[3] = {255, 255, 255};

	wxImage matImage(width, height, false);
	wxByte *pf = &dest[2 * imageSize];
	for (wxUint32 y = 0; y < height; y++)
		for (wxUint32 x = 0; x < width; x++)  {
			switch (*pf)  {
			case PX_EMPTY:  // Empty = black
				memcpy(&matImage.GetData()[(y * width + x) * 3], &black, 3);
				break;
			case PX_ROCK: // Rock = grey
				memcpy(&matImage.GetData()[(y * width + x) * 3], &grey, 3);
				break;
			case PX_DIRT: // Dirt = white
				memcpy(&matImage.GetData()[(y * width + x) * 3], &white, 3);
				break;
			default: // Unknown, defaults to empty black
				memcpy(&matImage.GetData()[(y * width + x) * 3], &black, 3);
			}

			++pf;
		}

	delete[] dest; // Not needed anymore


	// Get the file name and file type
	wxString dir, name, extension;
	wxSplitPath(outpath, &dir, &name, &extension);
	if (!wxEndsWithPathSeparator(dir))
#ifdef WIN32
		dir += '\\';
#else
		dir += '/';
#endif

	// Save the images
	wxInitAllImageHandlers();
	bool saveSuccess = true;
	saveSuccess &= backImage.SaveFile(dir + name + _T("_back.") + extension);
	saveSuccess &= frontImage.SaveFile(dir + name + _T("_front.") + extension);
	saveSuccess &= matImage.SaveFile(dir + name + _T("_mat.") + extension);

	if (!saveSuccess)
		throw Exception(_T("Could not save one or more images"));
}

IMPLEMENT_APP(LevelDecompilerApp)
