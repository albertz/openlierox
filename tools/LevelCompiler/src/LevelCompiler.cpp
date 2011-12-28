#include <string>
#include <map>

#include <wx/wx.h>
#include <wx/cmdline.h>
#include <wx/mstream.h>
#include <zlib.h>

// Some basic defines
#define WINDOW_WIDTH 500
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
class LevelCompilerApp : public wxApp
{
	virtual bool OnInit();
	virtual void OnInitCmdLine(wxCmdLineParser& parser);
	virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
	
	wxString commandLineFront;
	wxString commandLineBack;
	wxString commandLineMat;
	wxString commandLineFrontHiRes;
	wxString commandLineBackHiRes;
	wxString commandLineName;
	wxString commandLineOut;
};

// The function that does all the work :)
void CompileLevel(const wxString& front, const wxString& back, const wxString& mat, const wxString& out, const wxString& name,
					const wxString& OutFile,
					const wxString& frontHiRes, const wxString& backHiRes, const std::map< std::string, std::string >& additionalData);

// Basic functions
int max(const int vals[], int size)
{
	int res = vals[0];
	for (int i = 1; i < size; i++)
		if (vals[i] > res) 
			res = vals[i];

	return res;
}

bool equal(const int vals[])
{
	int first = vals[0];
	for (int i = 1; i < sizeof(vals)/sizeof(int); i++)
		if (vals[i] != first) 
			return false;

	return true;	
}

// Widget IDs
enum {
	lc_BrowseFront,
	lc_Front,
	lc_BrowseBack,
	lc_Back,
	lc_BrowseMat,
	lc_Mat,
	lc_BrowseFrontHiRes,
	lc_FrontHiRes,
	lc_BrowseBackHiRes,
	lc_BackHiRes,
	lc_Name,
	lc_Compile,
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
class LevelCompilerFrame : public wxFrame
{
private:

public:
	LevelCompilerFrame(const wxString& title);
	virtual ~LevelCompilerFrame();

private:
	// Components
	wxButton *btnBrowseFront;
	wxButton *btnBrowseBack;
	wxButton *btnBrowseMat;
	wxButton *btnBrowseFrontHiRes;
	wxButton *btnBrowseBackHiRes;
	wxButton *btnCompile;
	wxButton *btnExit;

	wxTextCtrl *txtFront;
	wxTextCtrl *txtBack;
	wxTextCtrl *txtMat;
	wxTextCtrl *txtFrontHiRes;
	wxTextCtrl *txtBackHiRes;
	wxTextCtrl *txtName;

	wxStaticText *lblFront;
	wxStaticText *lblBack;
	wxStaticText *lblMat;
	wxStaticText *lblFrontHiRes;
	wxStaticText *lblBackHiRes;
	wxStaticText *lblName;

	wxPanel *pnlMain;

	wxFileDialog *dlgSelectFile;
	wxFileDialog *dlgSaveFile;

// Events
protected:
	// Events
	void OnBrowseFrontClick(wxCommandEvent& evt);
	void OnBrowseBackClick(wxCommandEvent& evt);
	void OnBrowseMatClick(wxCommandEvent& evt);
	void OnBrowseFrontHiResClick(wxCommandEvent& evt);
	void OnBrowseBackHiResClick(wxCommandEvent& evt);
	void OnCompileClick(wxCommandEvent& evt);
	void OnExitClick(wxCommandEvent& evt);

	// wxWidgets stuff
	DECLARE_EVENT_TABLE()
};

// Register events
BEGIN_EVENT_TABLE(LevelCompilerFrame, wxFrame)
EVT_BUTTON(lc_BrowseFront, LevelCompilerFrame::OnBrowseFrontClick)
EVT_BUTTON(lc_BrowseBack, LevelCompilerFrame::OnBrowseBackClick)
EVT_BUTTON(lc_BrowseMat, LevelCompilerFrame::OnBrowseMatClick)
EVT_BUTTON(lc_BrowseFrontHiRes, LevelCompilerFrame::OnBrowseFrontHiResClick)
EVT_BUTTON(lc_BrowseBackHiRes, LevelCompilerFrame::OnBrowseBackHiResClick)
EVT_BUTTON(lc_Compile, LevelCompilerFrame::OnCompileClick)
EVT_BUTTON(lc_Exit, LevelCompilerFrame::OnExitClick)
END_EVENT_TABLE()

// Initialize the application
bool LevelCompilerApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;
	
	if( !commandLineFront.IsEmpty() )
	{
		try {
			CompileLevel(commandLineFront, commandLineBack, commandLineMat, commandLineOut, commandLineName, commandLineOut,
							commandLineFrontHiRes, commandLineBackHiRes, std::map< std::string, std::string > () );
			printf("Level compiled to file: %s\n", (const char *)commandLineName.mb_str());
		} catch (Exception &e) {
			printf("An error occured while compiling: %s\n", (const char *)e.message.mb_str());
			return false;
		}
		return true;
	} else {
		wxFrame *frame = new LevelCompilerFrame(_T("LieroX Level Compiler"));
		frame->Show();
		return true;
	}
}

void LevelCompilerApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	static const wxCmdLineEntryDesc cmdLineDesc [] =
	{
		{ wxCMD_LINE_PARAM, NULL, NULL, ("FrontImage"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
		{ wxCMD_LINE_PARAM, NULL, NULL, ("BackImage"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
		{ wxCMD_LINE_PARAM, NULL, NULL, ("MaterialImage"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
		{ wxCMD_LINE_PARAM, NULL, NULL, ("\"Level Name\""), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
		{ wxCMD_LINE_PARAM, NULL, NULL, ("OutputFile"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
		{ wxCMD_LINE_PARAM, NULL, NULL, ("FrontHiResImage"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
		{ wxCMD_LINE_PARAM, NULL, NULL, ("BackHiResImage"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
		{ wxCMD_LINE_NONE }
	};

	parser.SetDesc(cmdLineDesc);
	wxApp::OnInitCmdLine(parser);
}

bool LevelCompilerApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	if (parser.GetParamCount() >= 4)  
	{
		commandLineFront = parser.GetParam(0);
		commandLineBack = parser.GetParam(1);
		commandLineMat = parser.GetParam(2);
		commandLineName = parser.GetParam(3);
	
		if(parser.GetParamCount() >= 5)
		{
			commandLineOut = parser.GetParam(4);
		}
		else
			commandLineOut = commandLineName + (wxChar)".lxl";
		if(parser.GetParamCount() >= 7)
		{
			commandLineFrontHiRes = parser.GetParam(5);
			commandLineBackHiRes = parser.GetParam(6);
		}
	}
	else if(parser.GetParamCount() > 0 && parser.GetParamCount() < 4)
	{
		printf("First 4 parameters are required.\nOr start with no parameters for GUI.\n");
		return false;
	}
		
	return true;
}


// Window constructor
LevelCompilerFrame::LevelCompilerFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title,
                      wxPoint(0, 50), wxDefaultSize,
					  wxCAPTION | wxCLOSE_BOX | wxSYSTEM_MENU |
                      wxNO_FULL_REPAINT_ON_RESIZE |
                      wxCLIP_CHILDREN |
                      wxTAB_TRAVERSAL)

{
	this->SetClientSize(WINDOW_WIDTH, 10);

	// Allocate the components
	pnlMain = new wxPanel(this);
	lblFront = new wxStaticText(pnlMain, wxID_ANY, _T("Front image:"));
	lblBack = new wxStaticText(pnlMain, wxID_ANY, _T("Back image:"));
	lblMat = new wxStaticText(pnlMain, wxID_ANY, _T("Material image:"));
	lblFrontHiRes = new wxStaticText(pnlMain, wxID_ANY, _T("Front hi-res image (optional):"));
	lblBackHiRes = new wxStaticText(pnlMain, wxID_ANY, _T("Back hi-res image (optional):"));
	lblName = new wxStaticText(pnlMain, wxID_ANY, _T("Level name:"));
	btnBrowseFront = new wxButton(pnlMain, lc_BrowseFront, _T("..."));
	btnBrowseBack = new wxButton(pnlMain, lc_BrowseBack, _T("..."));
	btnBrowseMat = new wxButton(pnlMain, lc_BrowseMat, _T("..."));
	btnBrowseFrontHiRes = new wxButton(pnlMain, lc_BrowseFrontHiRes, _T("..."));
	btnBrowseBackHiRes = new wxButton(pnlMain, lc_BrowseBackHiRes, _T("..."));
	btnCompile = new wxButton(pnlMain, lc_Compile, _T("&Compile"));
	btnExit = new wxButton(pnlMain, lc_Exit, _T("&Exit"));
	txtFront = new wxTextCtrl(pnlMain, lc_Front);
	txtBack = new wxTextCtrl(pnlMain, lc_Back);
	txtMat = new wxTextCtrl(pnlMain, lc_Mat);
	txtFrontHiRes = new wxTextCtrl(pnlMain, lc_FrontHiRes);
	txtBackHiRes = new wxTextCtrl(pnlMain, lc_BackHiRes);
	txtName = new wxTextCtrl(pnlMain, lc_Name);
	dlgSelectFile = new wxFileDialog(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, 
					_T("All supported files|*.bmp;*.png;*.jpg;*.gif;*.pcx;*.tif;*.tiff;*.pnm;*.xpm;*.tga|Bitmap Images (*.bmp)|*.bmp|PNG Images (*.png)|*.png|JPG Images (*.jpg)|*.jpg|GIF Images (*.gif)|*.gif|PCX Images (*.pcx)|*.pcx|Tiff Images (*.tif, *.tiff)|*.tif;*.tiff|PNM Images (*.pnm)|*.pnm|XPM Images (*.xpm)|*.xpm|Targa Images (*.tga)|*.tga|All files (*.*)|*.*"),wxFD_OPEN);
	dlgSaveFile = new wxFileDialog(this, _T("Save to"), wxEmptyString, wxEmptyString, 
					_T("LieroX Level (*.lxl)|*.lxl|All files|*.*"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);


	// Set component positions
	pnlMain->SetSize(this->GetClientSize());
	const int buttonHeight = btnBrowseFront->GetSize().GetHeight();
	const int clientWidth = pnlMain->GetClientSize().GetWidth();
	const int lblWidths[] = {lblFront->GetSize().GetWidth(), lblBack->GetSize().GetWidth(), lblMat->GetSize().GetWidth(), lblName->GetSize().GetWidth(), lblFrontHiRes->GetSize().GetWidth(), lblBackHiRes->GetSize().GetWidth()};
	const int maxLabelWidth = max(lblWidths, sizeof(lblWidths)/sizeof(lblWidths[0])) + 2*SPACING;

	btnBrowseFront->SetSize(clientWidth - BROWSE_BTN_WIDTH - SPACING, SPACING, BROWSE_BTN_WIDTH, -1);
	txtFront->SetSize(SPACING + maxLabelWidth, btnBrowseFront->GetPosition().y, btnBrowseFront->GetPosition().x - 2*SPACING - maxLabelWidth, btnBrowseFront->GetSize().GetHeight());
	lblFront->SetPosition(wxPoint(SPACING, txtFront->GetPosition().y + (txtFront->GetSize().GetHeight() - lblFront->GetSize().GetHeight())/2));

	btnBrowseBack->SetSize(clientWidth - BROWSE_BTN_WIDTH - SPACING, btnBrowseFront->GetPosition().y + btnBrowseFront->GetSize().GetHeight() + SPACING, BROWSE_BTN_WIDTH, -1);
	//btnBrowseBack->SetSize(-1, -1, BROWSE_BTN_WIDTH, -1);
	txtBack->SetSize(SPACING + maxLabelWidth, btnBrowseBack->GetPosition().y, btnBrowseBack->GetPosition().x - 2*SPACING - maxLabelWidth, btnBrowseBack->GetSize().GetHeight());
	lblBack->SetPosition(wxPoint(SPACING, txtBack->GetPosition().y + (txtBack->GetSize().GetHeight() - lblBack->GetSize().GetHeight())/2));

	btnBrowseMat->SetSize(clientWidth - BROWSE_BTN_WIDTH - SPACING, btnBrowseBack->GetPosition().y + btnBrowseBack->GetSize().GetHeight() + SPACING, BROWSE_BTN_WIDTH, -1);
	txtMat->SetSize(SPACING + maxLabelWidth, btnBrowseMat->GetPosition().y, btnBrowseMat->GetPosition().x - 2*SPACING - maxLabelWidth, btnBrowseMat->GetSize().GetHeight());
	lblMat->SetPosition(wxPoint(SPACING, txtMat->GetPosition().y + (txtMat->GetSize().GetHeight() - lblMat->GetSize().GetHeight())/2));

	btnBrowseFrontHiRes->SetSize(clientWidth - BROWSE_BTN_WIDTH - SPACING, btnBrowseMat->GetPosition().y + btnBrowseMat->GetSize().GetHeight() + SPACING, BROWSE_BTN_WIDTH, -1);
	txtFrontHiRes->SetSize(SPACING + maxLabelWidth, btnBrowseFrontHiRes->GetPosition().y, btnBrowseFrontHiRes->GetPosition().x - 2*SPACING - maxLabelWidth, btnBrowseFrontHiRes->GetSize().GetHeight());
	lblFrontHiRes->SetPosition(wxPoint(SPACING, txtFrontHiRes->GetPosition().y + (txtFrontHiRes->GetSize().GetHeight() - lblFrontHiRes->GetSize().GetHeight())/2));

	btnBrowseBackHiRes->SetSize(clientWidth - BROWSE_BTN_WIDTH - SPACING, btnBrowseFrontHiRes->GetPosition().y + btnBrowseFrontHiRes->GetSize().GetHeight() + SPACING, BROWSE_BTN_WIDTH, -1);
	txtBackHiRes->SetSize(SPACING + maxLabelWidth, btnBrowseBackHiRes->GetPosition().y, btnBrowseBackHiRes->GetPosition().x - 2*SPACING - maxLabelWidth, btnBrowseBackHiRes->GetSize().GetHeight());
	lblBackHiRes->SetPosition(wxPoint(SPACING, txtBackHiRes->GetPosition().y + (txtBackHiRes->GetSize().GetHeight() - lblBackHiRes->GetSize().GetHeight())/2));

	txtName->SetSize(SPACING + maxLabelWidth, txtBackHiRes->GetPosition().y + txtBackHiRes->GetSize().GetHeight() + SPACING, clientWidth - 2*SPACING - maxLabelWidth, txtBackHiRes->GetSize().GetHeight());
	lblName->SetPosition(wxPoint(SPACING, txtName->GetPosition().y + (txtName->GetSize().GetHeight() - lblName->GetSize().GetHeight())/2));

	btnCompile->SetPosition(wxPoint(clientWidth/2 - btnCompile->GetSize().GetWidth() - SPACING/2, txtName->GetPosition().y + txtName->GetSize().GetHeight() + SPACING));
	btnExit->SetPosition(wxPoint(clientWidth/2 + SPACING/2, txtName->GetPosition().y + txtName->GetSize().GetHeight() + SPACING));

	// Setup dialog
	dlgSelectFile->SetDirectory(wxGetHomeDir());

	// Setup dimensions and position
	this->SetClientSize(WINDOW_WIDTH, 8*SPACING + txtFront->GetSize().GetHeight() + txtBack->GetSize().GetHeight() + txtMat->GetSize().GetHeight() + txtFrontHiRes->GetSize().GetHeight() + txtBackHiRes->GetSize().GetHeight() + txtName->GetSize().GetHeight() + btnCompile->GetSize().GetHeight());
	this->CentreOnScreen();
}

LevelCompilerFrame::~LevelCompilerFrame()
{
	// Free the components
	delete lblFront;
	delete lblBack;
	delete lblMat;
	delete lblFrontHiRes;
	delete lblBackHiRes;
	delete lblName;
	delete btnBrowseFront;
	delete btnBrowseBack;
	delete btnBrowseMat;
	delete btnBrowseFrontHiRes;
	delete btnBrowseBackHiRes;
	delete btnCompile;
	delete btnExit;
	delete txtFront;
	delete txtBack;
	delete txtMat;
	delete txtFrontHiRes;
	delete txtBackHiRes;
	delete txtName;
	delete pnlMain;
	delete dlgSelectFile;
	delete dlgSaveFile;
}

void LevelCompilerFrame::OnBrowseBackClick(wxCommandEvent &evt)
{
	// Let the user choose the file
	if (dlgSelectFile->ShowModal() == wxID_OK)  {
		txtBack->SetValue(dlgSelectFile->GetPath());
	}
}

void LevelCompilerFrame::OnBrowseFrontClick(wxCommandEvent &evt)
{
	// Let the user choose the file
	if (dlgSelectFile->ShowModal() == wxID_OK)  {
		txtFront->SetValue(dlgSelectFile->GetPath());
	}
}

void LevelCompilerFrame::OnBrowseMatClick(wxCommandEvent &evt)
{
	// Let the user choose the file
	if (dlgSelectFile->ShowModal() == wxID_OK)  {
		txtMat->SetValue(dlgSelectFile->GetPath());
	}
}

void LevelCompilerFrame::OnBrowseBackHiResClick(wxCommandEvent &evt)
{
	// Let the user choose the file
	if (dlgSelectFile->ShowModal() == wxID_OK)  {
		txtBackHiRes->SetValue(dlgSelectFile->GetPath());
	}
}

void LevelCompilerFrame::OnBrowseFrontHiResClick(wxCommandEvent &evt)
{
	// Let the user choose the file
	if (dlgSelectFile->ShowModal() == wxID_OK)  {
		txtFrontHiRes->SetValue(dlgSelectFile->GetPath());
	}
}

void LevelCompilerFrame::OnCompileClick(wxCommandEvent &evt)
{
	// Let the user choose the path to save to
	dlgSaveFile->SetDirectory(dlgSelectFile->GetDirectory());
	if (dlgSaveFile->ShowModal() == wxID_OK)  {
		try  {
			CompileLevel(txtFront->GetValue(), txtBack->GetValue(), txtMat->GetValue(), 
				dlgSaveFile->GetPath(), txtName->GetValue(), _T(""),
				txtFrontHiRes->GetValue(), txtBackHiRes->GetValue(), std::map< std::string, std::string > () );
			wxMessageBox(_T("Level successfully compiled!"), _T("Success"));
		} catch (Exception &e) {
			wxMessageBox(_T("An error occured while compiling:\n") + e.message, _T("Compiler error"));
		}
	}
}

void LevelCompilerFrame::OnExitClick(wxCommandEvent &evt)
{
	// Quit :)
	Close();
}

void CompileLevel(const wxString& front, const wxString& back, const wxString& mat, const wxString& out, const wxString& name,
					const wxString& OutFile,
					const wxString& frontHiRes, const wxString& backHiRes, const std::map< std::string, std::string > & additionalData)
{
	// Name check
	if (name.size() == 0)
		throw Exception(_T("No name specified."));

	// Open the file
	FILE *fp = NULL;
	if( OutFile == _T("") )
		fp = wxFopen(out.c_str(), _T("wb"));
	else
		fp = fopen((const char *)OutFile.mb_str(), "wb");
	
	if (fp == NULL)
		throw Exception(_T("Could not create the output file."));

	// Load the images
	wxInitAllImageHandlers();
	wxImage frontImage;
	wxImage backImage;
	wxImage matImage;

	bool useHiRes = true;
	if( frontHiRes == _T("") )
		useHiRes = false;
		
	wxImage frontImageHiRes;
	wxImage backImageHiRes;

	if (!frontImage.LoadFile(front))  {
		fclose(fp);
		wxRemove(out);
		throw Exception(_T("Could not load the front image."));
	}

	if (!backImage.LoadFile(back))  {
		fclose(fp);
		wxRemove(out);
		throw Exception(_T("Could not load the background image."));
	}

	if (!matImage.LoadFile(mat))  {
		fclose(fp);
		wxRemove(out);
		throw Exception(_T("Could not load the material image."));
	}
	
    if( useHiRes )
	{
		if (!frontImageHiRes.LoadFile(frontHiRes))  {
			fclose(fp);
			wxRemove(out);
			throw Exception(_T("Could not load the hi-res front image."));
		}
    
		if (!backImageHiRes.LoadFile(backHiRes))  {
			fclose(fp);
			wxRemove(out);
			throw Exception(_T("Could not load the hi-res background image."));
		}
	}

	// Check the dimensions
	const int widths[] = {frontImage.GetWidth(), backImage.GetWidth(), matImage.GetWidth()};
	const int heights[] = {frontImage.GetHeight(), backImage.GetHeight(), matImage.GetHeight()};
	if (!equal(widths) || !equal(heights))  {
		fclose(fp);
		wxRemove(out);
		throw Exception(_T("Images do not have same dimensions."));
	}
	if (widths[0] > 4096)  {
		fclose(fp);
		wxRemove(out);
		throw Exception(_T("The level cannot be wider than 4096 pixels."));
	}
	if (heights[0] > 4096)  {
		fclose(fp);
		wxRemove(out);
		throw Exception(_T("The level cannot be higher than 4096 pixels."));
	}
	if (widths[0] < 320)  {
		fclose(fp);
		wxRemove(out);
		throw Exception(_T("The level width cannot be less than 320 pixels."))	;
	}
	if (heights[0] < 191)  {
		fclose(fp);
		wxRemove(out);
		throw Exception(_T("The level height cannot be less than 191 pixels."));
	}
	
	if( useHiRes )
	{
		if( (frontImageHiRes.GetWidth() != backImageHiRes.GetWidth()) ||
			(frontImageHiRes.GetWidth() != frontImage.GetWidth() * 2) ||
			(frontImageHiRes.GetHeight() != backImageHiRes.GetHeight()) ||
			(frontImageHiRes.GetHeight() != frontImage.GetHeight() * 2) )
		{
			fclose(fp);
			wxRemove(out);
			throw Exception(_T("Hi-res image dimensions should be 2x normal image dimensions"));
		}
	}

	// Write out the header
	const char header[32] = "LieroX Level";
	const wxUint32 version = 0;
	fwrite(header, sizeof(header), 1, fp);
	fwrite(&wxUINT32_SWAP_ON_BE(version), 4, 1, fp);

	// Write out the name
	char levelName[64];
	strncpy(levelName, name.utf8_str(), sizeof(levelName));
	levelName[63] = '\0';
	fwrite(levelName, sizeof(levelName), 1, fp);

	// Dimensions
	const wxUint32 width = (wxUint32)widths[0];
	const wxUint32 height = (wxUint32)heights[0];
	fwrite(&wxUINT32_SWAP_ON_BE(width), 4, 1, fp);
	fwrite(&wxUINT32_SWAP_ON_BE(height), 4, 1, fp);

	// Type, theme and number of objects
	const wxUint32 type = MPT_IMAGE;
	const wxUint32 numObjects = 0;
	const char theme[32] = "dirt";
	fwrite(&wxUINT32_SWAP_ON_BE(type), 4, 1, fp);
	fwrite(theme, sizeof(theme), 1, fp);
	fwrite(&wxUINT32_SWAP_ON_BE(numObjects), 4, 1, fp);

	// Encode the pixel flags
	wxByte *pxFlags = new wxByte[width * height];
	for (wxUint32 y = 0; y < height; y++) {
		for (wxUint32 x = 0; x < width; x++)  {
			wxByte color[3];
			memcpy(color, &matImage.GetData()[y * matImage.GetWidth() * 3 + x * 3], 3);
			if (color[0] == 0 && color[1] == 0 && color[2] == 0)  { // Black = empty
				pxFlags[y * matImage.GetWidth() + x] = PX_EMPTY;
				memset(&frontImage.GetData()[y * width * 3 + x * 3], 0, 3); // Set the foreground pixel to black to make the compression more effective
			} else if (color[0] == 128 && color[1] == 128 && color[2] == 128)  { // Grey = rock
				pxFlags[y * matImage.GetWidth() + x] = PX_ROCK;
				memset(&backImage.GetData()[y * width * 3 + x * 3], 0, 3); // Set the background pixel to black to make the compression more effective
			} else if (color[0] == 255 && color[1] == 255 && color[2] == 255) // White = dirt
				pxFlags[y * matImage.GetWidth() + x] = PX_DIRT;
			else { // Unknown
				fclose(fp);
				wxRemove(out);
				delete[] pxFlags;
				throw Exception(_T("Unknown color in the material image."));
			}
		}
	}

	// Compress the images and pixel flags
	wxUint32 uncompressedSize = 7 * width * height; // Frontsize * 3 + Backsize * 3 + pxflagssize
	wxByte *data = new wxByte[uncompressedSize];
	memcpy(data, backImage.GetData(), 3 * width * height); // Copy the back image
	memcpy(data + 3 * width * height, frontImage.GetData(), 3 * width * height); // Copy the front image
	memcpy(data + 6 * width * height, pxFlags, width * height); // Copy the pixel flags
	uLongf csize = (int)(uncompressedSize * 1.1f) + 12;
	wxByte *compressedData = new wxByte[csize];
	if (compress2(compressedData, &csize, data, uncompressedSize, 9) != Z_OK) {
		delete[] pxFlags;
		delete[] data;
		delete[] compressedData;
		fclose(fp);
		wxRemove(out);
		throw Exception(_T("Could not compress the data."));
	}
	if (csize > 0xFFFFFFFF)  { // Overflow check
		fclose(fp);
		wxRemove(out);
		delete[] pxFlags;
		delete[] data;
		delete[] compressedData;
		throw Exception(_T("The compressed data is too big."));
	}
	wxUint32 compressedSize = csize;

	// Write out the compressed data
	fwrite(&wxUINT32_SWAP_ON_BE(compressedSize), 4, 1, fp);
	fwrite(&wxUINT32_SWAP_ON_BE(uncompressedSize), 4, 1, fp);
	fwrite(compressedData, compressedSize, 1, fp);

	delete[] pxFlags;
	delete[] data;
	delete[] compressedData;
	
	// OLX additional data is chunked, chunk looks like this: 
	// 16 bytes of safety header, which starts with 4-byte string "OLX "
	// then goes 4 bytes of chunk size, then chunk data
	if( useHiRes )
	{
		// Move all image data to first image, so it will compress better
		wxImage combinedImage( frontImageHiRes.Resize( wxSize(width*2, height*4), wxPoint(0, 0), 0, 0, 0 ) );
		memcpy(((char *)combinedImage.GetData()) + 3 * 4 * width * height, backImageHiRes.GetData(), 3 * 4 * width * height);
		wxMemoryOutputStream str;
		if( !combinedImage.SaveFile( str, wxBITMAP_TYPE_PNG ) )
		{
			fclose(fp);
			wxRemove(out);
			throw Exception(_T("Cannot save hi-res images to stream"));
		};
		const char safetyHeaderHiRes[16] = "OLX hi-res data"; // OLX will check for this string if the file contains junk at the end
		fwrite(safetyHeaderHiRes, 16, 1, fp);
		compressedSize = str.GetOutputStreamBuffer()->Tell();
		fwrite(&wxUINT32_SWAP_ON_BE(compressedSize), 4, 1, fp);
		fwrite(str.GetOutputStreamBuffer()->GetBufferStart(), str.GetOutputStreamBuffer()->Tell(), 1, fp);
	}
	
	if( !additionalData.empty() )
	{
		// TODO: this is untested!
		std::string additionalDataRaw;
		for( std::map< std::string, std::string >::const_iterator it = additionalData.begin(); 
				it != additionalData.end(); it++ )
		{
			wxUint32 size = wxUINT32_SWAP_ON_BE( it->first.size() );
			additionalDataRaw += std::string( (char *)(&size), 4 );
			additionalDataRaw += it->first;
			size = wxUINT32_SWAP_ON_BE( it->second.size() );
			additionalDataRaw += std::string( (char *)(&size), 4 );
			additionalDataRaw += it->second;
		}
		
		uncompressedSize = additionalDataRaw.size() + 4;

		data = new wxByte[uncompressedSize];
		wxUint32 size = wxUINT32_SWAP_ON_BE( additionalDataRaw.size() );
		memcpy(data, &size, 4 );
		memcpy(data + 4, additionalDataRaw.c_str(), additionalDataRaw.size() );
		csize = (int)(uncompressedSize * 1.1f) + 12;
		compressedData = new wxByte[csize];
		if (compress2(compressedData, &csize, data, uncompressedSize, 9) != Z_OK) 
		{
			delete[] data;
			delete[] compressedData;
			fclose(fp);
			wxRemove(out);
			throw Exception(_T("Could not compress the data."));
		}
		compressedSize = csize;
		// Write out the compressed data
		const char safetyHeaderData[17] = "OLX level config"; // OLX will check for this string if the file contains junk at the end
		fwrite(safetyHeaderData, 16, 1, fp);
		wxUint32 SizeTotal = compressedSize+4;
		fwrite(&wxUINT32_SWAP_ON_BE(SizeTotal), 4, 1, fp); // Count 4 bytes of uncompressedSize with compressedData
		fwrite(&wxUINT32_SWAP_ON_BE(uncompressedSize), 4, 1, fp);
		fwrite(compressedData, compressedSize, 1, fp);
	}

	// Cleanups
	fclose(fp);
}

IMPLEMENT_APP(LevelCompilerApp)
