//---------------------------------------------------------------------------
#ifndef FEditH
#define FEditH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
#include "ComLib.h"
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TFileEdit : public TForm
{
__published:	// IDE 管理のコンポーネント
	TRichEdit *REdit;
	TMainMenu *MainMenu1;
	TMenuItem *KF;
	TMenuItem *KE;
	TMenuItem *KFA;
	TMenuItem *KEC;
	TMenuItem *KECP;
	TMenuItem *KEP;
	TSaveDialog *SaveDialog;
	TMenuItem *N1;
	TMenuItem *KEU;
	TMenuItem *N2;
	TMenuItem *KEA;
	TMenuItem *N3;
	TMenuItem *KFX;
	TPopupMenu *PopupMenu;
	TFindDialog *FindDialog;
	TMenuItem *KS;
	TMenuItem *KSS;
	TMenuItem *KSN;
	TMenuItem *KFN;
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	
	void __fastcall KECClick(TObject *Sender);
	void __fastcall KECPClick(TObject *Sender);
	void __fastcall KEPClick(TObject *Sender);
	void __fastcall KFAClick(TObject *Sender);
	void __fastcall KEUClick(TObject *Sender);
	void __fastcall KEAClick(TObject *Sender);
	
	void __fastcall KEClick(TObject *Sender);
	void __fastcall KFXClick(TObject *Sender);
	void __fastcall PopupMenuPopup(TObject *Sender);
	
	void __fastcall KSSClick(TObject *Sender);
	void __fastcall FindDialogFind(TObject *Sender);
	void __fastcall KSClick(TObject *Sender);
	void __fastcall KFNClick(TObject *Sender);
	
private:	// ユーザー宣言
	AnsiString  m_FileName;
public:		// ユーザー宣言
	__fastcall TFileEdit(TComponent* Owner);
	void __fastcall Execute(LPCSTR pName);
};
//---------------------------------------------------------------------------
extern PACKAGE TFileEdit *FileEdit;
//---------------------------------------------------------------------------
#endif
