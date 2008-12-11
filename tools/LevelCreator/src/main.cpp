// Level Maker v1.1
// For Liero Xtreme
// By Jason Boettcher
// 29/7/03


#include <windows.h>
#include <winuser.h>
#include <stdio.h>
#include <direct.h>
#include "..\resource.h"


HINSTANCE g_hInst = 0;

LRESULT CALLBACK MainDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void	browseImage(HWND hWnd, int nTextBox);
void	browseSaveImage(HWND hWnd, int nTextBox);
void	browseSaveLevel(HWND hWnd, int nTextBox);
void	compile(HWND hWnd);
bool	checkFile(HWND hWnd, int nTextbox);

int		loadFiles(HWND hWnd, char *szFrontFile, char *szBackFile, char *szMaterialFile);
int		writeMap(HWND hWnd, char *szOutFile, char *szName);
int		createPreview(HWND hWnd, char *szPreviewFile);


void	SaveAsLevelProject(HWND hWnd);
bool	SaveLevelProject(HWND hWnd, char *szFilename);


void	StripQuotes(char *dest, char *src);
void	lx_strncpy(char *dest, char *src, int count);
char	*TrimSpaces(char *szLine);
void	LoadLevelProject(HWND hWnd);


bool	g_bSavedProject = false;
char	g_szProjectFilename[_MAX_PATH];
bool	g_bProjectDirty = false;


#define MIN(a,b)	(a) < (b) ? (a) : (b)


int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{

    g_hInst = hInstance;
    DialogBox(hInstance,MAKEINTRESOURCE(IDD_MAIN),NULL,(DLGPROC)MainDialogProc);

    return 0;
}


///////////////////
// Dialog box callback
LRESULT CALLBACK MainDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch(uMsg) {

		case WM_CLOSE:
			if(g_bProjectDirty) {
				int nRet = MessageBox(hWnd, "The project has changed. Save changes before exiting?", 
					                  "Liero Xtreme Level Maker", MB_YESNOCANCEL);
				if(nRet == IDCANCEL)
					return false;
				if(nRet == IDYES) {
					if(g_bSavedProject)
						SaveLevelProject(hWnd, g_szProjectFilename);
					else
						SaveAsLevelProject(hWnd);
				}
			}

			EndDialog(hWnd, false);
			DestroyWindow(hWnd);
			return true;

		// Command
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			
			switch (wmId)
			{

				// OK
				case IDOK:

					/*w = SendDlgItemMessage(hWnd,IDC_CMBWIDTH,CB_GETCURSEL,0,0);
					h = SendDlgItemMessage(hWnd,IDC_CMBHEIGHT,CB_GETCURSEL,0,0);
					w = mapSizes[w];
					h = mapSizes[h];

					g_pcMap->New(w,h);

					Map_Update(g_hWnd);
					Minimap_UpdateMap();*/

					// Close me
					SendMessage(hWnd,WM_CLOSE,0,0);
					break;


                // Browse for the front image
                case IDC_FRONTBROWSE:
                    browseImage(hWnd,IDC_TXT_FRONTIMAGE);
                    break;
                // Browse for the back image
                case IDC_BACKBROWSE:
                    browseImage(hWnd,IDC_TXT_BACKIMAGE);
                    break;
                // Browse for the material image
                case IDC_MATBROWSE:
                    browseImage(hWnd,IDC_TXT_MATIMAGE);
                    break;
				// Browse for the preview image
                case IDC_PVWBROWSE:
                    browseSaveImage(hWnd,IDC_TXT_PREVIMAGE);
                    break;
				// Browse for the level file
				case IDC_LXLBROWSE:
					browseSaveLevel(hWnd,IDC_TXT_LXLFILE);
                    break;

				// Changed
				case IDC_TXT_FRONTIMAGE:		// v Fallthrough v
				case IDC_TXT_BACKIMAGE:
				case IDC_TXT_MATIMAGE:
				case IDC_TXT_PREVIMAGE:
				case IDC_TXT_LXLFILE:
				case IDC_TXT_LEVELNAME:
					if(wmEvent == EN_CHANGE) {
						g_bProjectDirty = true;
					}
					break;
	

                // Compile
                case CMD_COMPILE:
                    compile(hWnd);
                    break;


				// Help->About
				case MNU_HELP_ABOUT:
					MessageBox(hWnd, 
						       "Liero Xtreme Level Maker\nVersion v1.2\n\n"
							   "By Jason Boettcher\n\nhttp://lieroxtreme.thegaminguniverse.com",
							   "About",
							   MB_OK | MB_ICONINFORMATION);
					break;

				// File->Exit
				case MNU_FILE_EXIT:
					SendMessage(hWnd,WM_CLOSE,0,0);
					break;

				// File->Load level project
				case MNU_FILE_LOADPRJ:
					LoadLevelProject(hWnd);
					break;

				// File->Save As level project
				case MNU_FILE_SAVEASPRJ:
					SaveAsLevelProject(hWnd);
					break;

				// File->Save level project
				case MNU_FILE_SAVEPRJ:
					if(g_bSavedProject)
						SaveLevelProject(hWnd, g_szProjectFilename);
					else
						SaveAsLevelProject(hWnd);
					break;

			}
			break;
	}

	return false;
}


///////////////////
// Browse for an image file
void browseImage(HWND hWnd, int nTextBox)
{
    OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];		// File name


	// Save the current working directory
	char szCurrentDir[_MAX_PATH];
	_getcwd(szCurrentDir,_MAX_PATH);

	// Clear the filename (otherwise it won't work)
	strcpy(szFile,"");

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = g_hInst;
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Image Files\0*.png;*.bmp;*.tga;*.pcx;*.jpg\0All files\0*.*";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = "Open";
	ofn.lpstrInitialDir = szCurrentDir;
	ofn.Flags = OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn)) {
        SetDlgItemText(hWnd, nTextBox, szFile);
	}
}


/*-------------------------------
 * Browse to save an image file
 *-------------------------------*/
void browseSaveImage(HWND hWnd, int nTextBox)
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];		// File name


	// Save the current working directory
	char szCurrentDir[_MAX_PATH];
	_getcwd(szCurrentDir,_MAX_PATH);

	// Clear the filename (otherwise it won't work)
	strcpy(szFile,"");

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = g_hInst;
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Image Files\0*.png;*.bmp;*.tga;*.pcx;*.jpg\0All files\0*.*";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = "Save As";
	ofn.lpstrInitialDir = szCurrentDir;
	ofn.Flags = OFN_PATHMUSTEXIST;

	if(GetSaveFileName(&ofn)) {
        SetDlgItemText(hWnd, nTextBox, szFile);
	}
}


/*-------------------------------
 * Browse to save a level file
 *-------------------------------*/
void browseSaveLevel(HWND hWnd, int nTextBox)
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];		// File name


	// Save the current working directory
	char szCurrentDir[_MAX_PATH];
	_getcwd(szCurrentDir,_MAX_PATH);

	// Clear the filename (otherwise it won't work)
	strcpy(szFile,"");

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = g_hInst;
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Liero Xtreme Level Files (*.lxl)\0*.lxl\0All files\0*.*";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = "Save As";
	ofn.lpstrInitialDir = szCurrentDir;
	ofn.Flags = OFN_PATHMUSTEXIST;

	if(GetSaveFileName(&ofn)) {
        SetDlgItemText(hWnd, nTextBox, szFile);
	}
}



///////////////////
// Compile the level
void compile(HWND hWnd)
{
    char szName[256];

    // Make sure the 3 images are valid   
    if( !checkFile(hWnd, IDC_TXT_FRONTIMAGE) )
        return;
    if( !checkFile(hWnd, IDC_TXT_BACKIMAGE) )
        return;
    if( !checkFile(hWnd, IDC_TXT_MATIMAGE) )
        return;

    if(GetDlgItemText(hWnd, IDC_TXT_LEVELNAME, szName, 255) == 0) {
        MessageBox(hWnd, "Name is invalid", "Error", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

	if(GetDlgItemText(hWnd, IDC_TXT_LXLFILE, szName, 255) == 0) {
        MessageBox(hWnd, "Level filename is invalid", "Error", MB_OK | MB_ICONEXCLAMATION);
        return;
    }



    // Find a place to save the level
    /*OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];		// File name

	// Save the current working directory
	char szCurrentDir[_MAX_PATH];
	_getcwd(szCurrentDir,_MAX_PATH);

	// Clear the filename (otherwise it won't work)
	strcpy(szFile,"");

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = g_hInst;
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Level Files (*.lxl)\0*.lxl";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = "Save As";
	ofn.lpstrInitialDir = szCurrentDir;
	ofn.Flags = OFN_PATHMUSTEXIST;

	if(GetSaveFileName(&ofn)) {*/

	char szFront[_MAX_PATH];
	char szBack[_MAX_PATH];
	char szMat[_MAX_PATH];
	char szPrev[_MAX_PATH];
	char szFile[_MAX_PATH];
	
	szFront[0] = 0;
	szBack[0] = 0;
	szMat[0] = 0;
	szPrev[0] = 0;
	szFile[0] = 0;
	
	// Get the filenames
	GetDlgItemText(hWnd, IDC_TXT_FRONTIMAGE, szFront, _MAX_PATH);
	GetDlgItemText(hWnd, IDC_TXT_BACKIMAGE, szBack, _MAX_PATH);
	GetDlgItemText(hWnd, IDC_TXT_MATIMAGE, szMat, _MAX_PATH);
	GetDlgItemText(hWnd, IDC_TXT_PREVIMAGE, szPrev, _MAX_PATH);
	GetDlgItemText(hWnd, IDC_TXT_LXLFILE, szFile, _MAX_PATH);
	
	
	
	// Append a .lxl if it doesn't exist
	if(stricmp(szFile + strlen(szFile)-4, ".lxl") != 0)
		strcat(szFile,".lxl");
	
	// Check if the file exists
	/*FILE *fp = fopen(szFile,"rb");
	if( fp ) {
		fclose(fp);
		if( MessageBox(hWnd, "File Exists\nOverwrite it?", "Confirmation",MB_YESNO) == IDNO) {
			return;
		}
	}*/
	
	
	if(!loadFiles(hWnd, szFront, szBack, szMat))
		return;
	
	if(!writeMap(hWnd, szFile, szName))
		return;
	
	createPreview(hWnd, szPrev);
	//}
}


///////////////////
// Check if an image file is valid
bool checkFile(HWND hWnd, int nTextbox)
{
    char buf[_MAX_PATH];
    char msg[_MAX_PATH];
    buf[0] = 0;

    if(GetDlgItemText(hWnd, nTextbox, buf, _MAX_PATH) == 0) {
        MessageBox(hWnd, "You must provide a filename", "Error", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    FILE *fp = fopen(buf,"rb");
    if(!fp) {
        sprintf(msg, "\"%s\" could not be opened for reading",buf);
        MessageBox(hWnd, msg, "Error", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    fclose(fp);
    return true;
}


/*-------------------------------
 * SaveAs a level project
 *-------------------------------*/
void SaveAsLevelProject(HWND hWnd)
{
	// Get the filename
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];		// File name

	// Save the current working directory
	char szCurrentDir[_MAX_PATH];
	_getcwd(szCurrentDir,_MAX_PATH);

	// Clear the filename (otherwise it won't work)
	strcpy(szFile,"");

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = g_hInst;
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Liero Xtreme Level Project Files (*.lvp)\0*.lvp\0All files\0*.*";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = "Save As";
	ofn.lpstrInitialDir = szCurrentDir;
	ofn.Flags = OFN_PATHMUSTEXIST;

	if(GetSaveFileName(&ofn)) {

		// Append a .lvp if it doesn't exist
		if(stricmp(szFile + strlen(szFile)-4, ".lvp") != 0)
			strcat(szFile,".lvp");
		
		// Check if the file exists
		FILE *fp = fopen(szFile,"rt");
		if(fp) {
			fclose(fp);
			if( MessageBox(hWnd, "File Exists\nOverwrite it?", "Confirmation",MB_YESNO) == IDNO) {
				return;
			}
		}

		if(SaveLevelProject(hWnd, szFile)) {
			// Store the info
			strcpy(g_szProjectFilename, szFile);
			g_bSavedProject = true;
		}
	}
}


/*-------------------------------
 * Load a level project
 *-------------------------------*/
void LoadLevelProject(HWND hWnd)
{
	// Get the filename
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];		// File name


	// Save the current working directory
	char szCurrentDir[_MAX_PATH];
	_getcwd(szCurrentDir,_MAX_PATH);

	// Clear the filename (otherwise it won't work)
	strcpy(szFile,"");

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = g_hInst;
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Liero Xtreme Level Project Files (*.lvp)\0*.lvp\0All files\0*.*";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = "Open";
	ofn.lpstrInitialDir = szCurrentDir;
	ofn.Flags = OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn)) {

		// Clear all the text fields
		SetDlgItemText(hWnd, IDC_TXT_FRONTIMAGE, "");
		SetDlgItemText(hWnd, IDC_TXT_BACKIMAGE, "");
		SetDlgItemText(hWnd, IDC_TXT_MATIMAGE, "");
		SetDlgItemText(hWnd, IDC_TXT_PREVIMAGE, "");
		SetDlgItemText(hWnd, IDC_TXT_LXLFILE, "");
		SetDlgItemText(hWnd, IDC_TXT_LEVELNAME, "");

		g_bProjectDirty = false;

		// Open the file
		FILE *fp = fopen(szFile, "rt");
		if(!fp) {
			MessageBox(hWnd, "Error opening project file", "Error", MB_OK);
			return;
		}
		
		// Go through line by line
		char szLine[2048];
		while(fgets(szLine, sizeof(szLine), fp) != NULL) {
			// Trim the line
			strcpy(szLine, TrimSpaces(szLine));
			
			// Empty line or only has a newline
			if(strlen(szLine) < 2)
				continue;
			
			// Comment?
			if(szLine[0] == '#')
				continue;
			
			// Variable!
			char *pDest = strchr(szLine,'=');
			if(pDest != NULL) {
				char szName[256];
				char szValue[256];
		
				// Name
				int nResult = (int)(pDest - szLine + 1);
				lx_strncpy(szName, szLine, MIN(nResult-1, sizeof(szName)));
				strcpy(szName, TrimSpaces(szName));
				
				// Value
				lx_strncpy(szValue, szLine+nResult, sizeof(szValue));
				strcpy(szValue, TrimSpaces(szValue));
				StripQuotes(szValue, szValue);
				
				if(stricmp(szName, "Name") == 0)
					SetDlgItemText(hWnd, IDC_TXT_LEVELNAME, szValue);
				else if(stricmp(szName, "FrontImg") == 0)
					SetDlgItemText(hWnd, IDC_TXT_FRONTIMAGE, szValue);
				else if(stricmp(szName, "BackImg") == 0)
					SetDlgItemText(hWnd, IDC_TXT_BACKIMAGE, szValue);
				else if(stricmp(szName, "MatImg") == 0)
					SetDlgItemText(hWnd, IDC_TXT_MATIMAGE, szValue);
				else if(stricmp(szName, "PrevImg") == 0)
					SetDlgItemText(hWnd, IDC_TXT_PREVIMAGE, szValue);
				else if(stricmp(szName, "LXLFile") == 0)
					SetDlgItemText(hWnd, IDC_TXT_LXLFILE, szValue);
			}
		}

		g_bProjectDirty = false;
		g_bSavedProject = true;
		strcpy(g_szProjectFilename, szFile);

		fclose(fp);
	}
}


/*-------------------------------
 * Save a level project file
 *-------------------------------*/
bool SaveLevelProject(HWND hWnd, char *szFilename)
{
	// Get the text entries
	char szFront[_MAX_PATH];
	char szBack[_MAX_PATH];
	char szMat[_MAX_PATH];
	char szPrev[_MAX_PATH];
	char szFile[_MAX_PATH];
	char szName[128];
	
	szFront[0] = 0;
	szBack[0] = 0;
	szMat[0] = 0;
	szPrev[0] = 0;
	szFile[0] = 0;
	szName[0] = 0;
	
	// Get the filenames
	GetDlgItemText(hWnd, IDC_TXT_FRONTIMAGE, szFront, _MAX_PATH);
	GetDlgItemText(hWnd, IDC_TXT_BACKIMAGE, szBack, _MAX_PATH);
	GetDlgItemText(hWnd, IDC_TXT_MATIMAGE, szMat, _MAX_PATH);
	GetDlgItemText(hWnd, IDC_TXT_PREVIMAGE, szPrev, _MAX_PATH);
	GetDlgItemText(hWnd, IDC_TXT_LXLFILE, szFile, _MAX_PATH);
	GetDlgItemText(hWnd, IDC_TXT_LEVELNAME, szName, 128);
	
	FILE *pProjectFile = fopen(szFilename, "wt");
	if(!pProjectFile) {
		MessageBox(hWnd, "Error opening file for writing", "Error", MB_OK);
		return false;
	}
	
	fprintf(pProjectFile, "# Liero Xtreme Level Project\n");
	fprintf(pProjectFile, "Name = \"%s\"\n", szName);
	fprintf(pProjectFile, "FrontImg = \"%s\"\n", szFront);
	fprintf(pProjectFile, "BackImg = \"%s\"\n", szBack);
	fprintf(pProjectFile, "MatImg = \"%s\"\n", szMat);
	fprintf(pProjectFile, "PrevImg = \"%s\"\n", szPrev);
	fprintf(pProjectFile, "LXLFile = \"%s\"\n", szFile);
	
	fclose(pProjectFile);

	g_bProjectDirty = false;

	return true;
}



///////////////////
// Strip quotes away from a string
void StripQuotes(char *dest, char *src)
{
	if(!dest || !src)
		return;
	 
	int pos = 0;
	int length = strlen(src);
	
	if(src[0] == '\"') {
		pos = 1;
		length--;
	}
	
	if(src[strlen(src)-1] == '\"')
		length--;

	strncpy(dest, src+pos, length);
	dest[length] = 0;
}


///////////////////
// Safe string copy routine
void lx_strncpy(char *dest, char *src, int count)
{
    while (*src && count--)
	{
		*dest++ = *src++;
	}
	if (count)
		*dest++ = 0;
}


///////////////////
// Trim the leading & ending spaces from a string
char *TrimSpaces(char *szLine)
{
    // Remove preceeding spaces
    while( !isgraph(*szLine) && isspace(*szLine) )
        szLine++;

    // Get rid of the ending spaces
    for( int i=strlen(szLine)-1; i>=0; i--) {
        if( isgraph(szLine[i]) || !isspace(szLine[i]) )
            break;
    }
    szLine[i+1] = '\0';

    return szLine;
}