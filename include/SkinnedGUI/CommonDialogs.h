
#ifndef __COMMONDIALOGS_H__SKINNED_GUI__
#define __COMMONDIALOGS_H__SKINNED_GUI__

#include "SkinnedGUI/CDialog.h"
#include "GfxPrimitives.h"
#include "CGameScript.h"
#include "CWpnRest.h"


namespace SkinnedGUI {

// Some forward declarations of widgets
class CButton;
class CTabControl;
class CTextbox;
class CCheckbox;
class CSlider;
class CListview;
class CListviewItem;
class CLabel;

// Message box types
enum MessageBoxType {
	LMB_OK = 0,
	LMB_YESNO
};

enum {
	// Return types
	MBR_OK=0,
	MBR_YES,
	MBR_NO
};

class CMessageBox;
typedef void(CWidget::*MessageBoxReturnHandler)(CMessageBox *sender, int result);
#define SET_MSGBOXCLOSE(messagebox, func)  SET_EVENT(messagebox, OnReturn, MessageBoxReturnHandler, func)

class CMessageBox : public CDialog  {
public:
	CMessageBox(CContainerWidget *parent, const std::string& title, const std::string& text, MessageBoxType type);
private:
	int iType;
	CButton *btnOk;
	CButton *btnYes;
	CButton *btnNo;
	std::string sText;
	CFontStyle cFont;

	DECLARE_EVENT(OnReturn, MessageBoxReturnHandler);

	void OnButtonClick(CWidget *sender, int x, int y, int dx, int dy, int button);
	int	DoKeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate);
	void DoRepaint();
	void Close(int res);

public:
	EVENT_SETGET(OnReturn, MessageBoxReturnHandler);
};


class CFileDialog;
typedef void(CWidget::*FileDialogConfirmHandler)(CFileDialog *sender, const std::string& file);
#define SET_FILEDLGCONFIRM(filedialog, func)  SET_EVENT(filedialog, OnConfirm, FileDialogConfirmHandler, func)

class CFileDialog : public CDialog  {
public:
	CFileDialog(COMMON_PARAMS, const std::string& title, const std::string& directory, const std::string& extension);

protected:
	std::string sDirectory;
	std::string sExtension;

	CButton *btnOk;
	CButton *btnCancel;
	CListview *lsvFileList;

	DECLARE_EVENT(OnConfirm, FileDialogConfirmHandler);

public:
	EVENT_SETGET(OnConfirm, FileDialogConfirmHandler);
};

class CFileLoadDialog : public CFileDialog  {
public:
	CFileLoadDialog(COMMON_PARAMS, const std::string& title, const std::string& directory, const std::string& extension);

private:
	void OkClick(CButton *sender, MouseButton button, const ModifiersState& modstate);
	void CancelClick(CButton *sender, MouseButton button, const ModifiersState& modstate);
	void FileListDoubleClick(CListview *sender, CListviewItem *item, int index);
};

class CFileSaveDialog : public CFileDialog  {
public:
	CFileSaveDialog(COMMON_PARAMS, const std::string& title, const std::string& directory, const std::string& extension);
private:
	CTextbox *txtFileName;
	CMessageBox *dlgConfirmOverwrite;

private:
	std::string GetFileName();
	void OkClick(CButton *sender, MouseButton button, const ModifiersState& modstate);
	void CancelClick(CButton *sender, MouseButton button, const ModifiersState& modstate);
	void FileListDoubleClick(CListview *sender, CListviewItem *item, int index);
	void FileListChange(CListview *sender, CListviewItem *newitem, int newindex, bool& cancel);
	void MessageBoxClose(CMessageBox *sender, int result);
};

class CGameSettingsDialog : public CDialog  {
private:
	CTabControl *tbcMain;
	CButton *btnOk;
	CButton *btnDefault;

	// General tab
	CTextbox *txtLives;
	CTextbox *txtMaxKills;
	CSlider *sldLoadingTime;
	CLabel *lblLoadingTime;
	CTextbox *txtTimeLimit;
	CTextbox *txtRespawnTime;
	CCheckbox *chkEmptyWeapons;
	CCheckbox *chkWaveRespawn;
	CCheckbox *chkGroupTeams;
	CCheckbox *chkDecreaseScore;

	// Bonus tab
	CCheckbox *chkBonusesOn;
	CTextbox *txtBonusSpawnTime;
	CCheckbox *chkShowBonusNames;
	CTextbox *txtBonusLifeTime;
	CLabel *lblHealthChance;
	CLabel *lblWeaponChance;
	CSlider *sldWeaponToHealthChance;

private:
	void LoadFromOptions();
	void Save();
	void Default();

	void OkClick(CButton *sender, MouseButton button, const ModifiersState& modstate);
	void DefaultClick(CButton *sender, MouseButton button, const ModifiersState& modstate);

public:
	CGameSettingsDialog(COMMON_PARAMS);
};

class CWeaponOptionsDialog : public CDialog  {
public:
	CWeaponOptionsDialog(COMMON_PARAMS, const std::string& mod);
	~CWeaponOptionsDialog();

private:
	CGameScript *cGameScript;
	CWpnRest *cRestrictionList;
	
	CButton *btnOk;
	CButton *btnCycle;
	CButton *btnRandom;
	CButton *btnLoad;
	CButton *btnSave;
	CListview *lsvRestrictions;

	CFileLoadDialog *dlgLoadRest;
	CFileSaveDialog *dlgSaveRest;

private:
	void OkClick(CButton *sender, MouseButton button, const ModifiersState& modstate);
	void CycleClick(CButton *sender, MouseButton button, const ModifiersState& modstate);
	void RandomClick(CButton *sender, MouseButton button, const ModifiersState& modstate);
	void LoadClick(CButton *sender, MouseButton button, const ModifiersState& modstate);
	void SaveClick(CButton *sender, MouseButton button, const ModifiersState& modstate);
	void ItemClick(CListview *sender, CListviewItem *item, int index);
	void SaveDialogConfirm(CFileDialog *sender, const std::string& file);
	void LoadDialogConfirm(CFileDialog *sender, const std::string& file);

private:
	void UpdateListview();
};

}; // namespace SkinnedGUI

#endif // __COMMONDIALOGS_H__SKINNED_GUI__
