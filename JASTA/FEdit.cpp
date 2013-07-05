//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "FEdit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFileEdit *FileEdit;
//---------------------------------------------------------------------------
__fastcall TFileEdit::TFileEdit(TComponent* Owner)
	: TForm(Owner)
{
	m_FileName = "";
	REdit->Font->Pitch = fpFixed;
	if( MsgEng ){
		REdit->Font->Name = "Courier New";
		REdit->Font->Charset = 0;
	}
	else {
		REdit->Font->Name = "ＭＳ ゴシック";
		REdit->Font->Charset = SHIFTJIS_CHARSET;

		KF->Caption = "ファイル(&F)";
		KFN->Caption = "メモ帳に切り替え(&N)";
		KFX->Caption = "終了(&X)";
		KE->Caption = "編集(&E)";
		KFA->Caption = "名前を付けて保存(&A)...";
		KEU->Caption = "元に戻す(&U)";
		KEC->Caption = "切り取り(&T)";
		KECP->Caption = "コピー(&C)";
		KEP->Caption = "貼り付け(&P)";
		KEA->Caption = "すべて選択(&L)";
		KS->Caption = "検索(&S)";
		KSS->Caption = "検索(&F)...";
		KSN->Caption = "次を検索(&N)";
	}
	for( int i = 0; i < KE->Count; i++ ){
		TMenuItem *pMenu = new TMenuItem(this);
		pMenu->Caption = KE->Items[i]->Caption;
		pMenu->OnClick = KE->Items[i]->OnClick;
		PopupMenu->Items->Add(pMenu);
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::Execute(LPCSTR pName)
{
	Caption = pName;
	CWaitCursor tw;
	m_FileName = pName;
	REdit->Lines->LoadFromFile(pName);
	REdit->Modified = FALSE;
	Show();
}
//---------------------------------------------------------------------
void __fastcall TFileEdit::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	if( REdit->Modified ){
		int r = YesNoCancelMB(MsgEng ? "Overwrite to the file (%s) ?" : "ファイル (%s) に上書きしますか?", m_FileName.c_str());
		switch(r){
			case IDYES:
				{
					CWaitCursor w;
					REdit->Lines->SaveToFile(m_FileName);
				}
				break;
			case IDCANCEL:
				CanClose = FALSE;
				return;
		}
	}
	CanClose = TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::KECClick(TObject *Sender)
{
	REdit->CutToClipboard();
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::KECPClick(TObject *Sender)
{
	REdit->CopyToClipboard();
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::KEPClick(TObject *Sender)
{
	REdit->PasteFromClipboard();
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::KEUClick(TObject *Sender)
{
	if( REdit->HandleAllocated() ){
		SendMessage(REdit->Handle, EM_UNDO, 0, 0);
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::KEAClick(TObject *Sender)
{
	REdit->SelectAll();
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::KEClick(TObject *Sender)
{
	KEU->Enabled = REdit->Modified;
	KEC->Enabled = REdit->SelLength;
	KECP->Enabled = KEC->Enabled;
	KEP->Enabled = ::IsClipboardFormatAvailable(CF_TEXT);
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::KFAClick(TObject *Sender)
{
	AnsiString  FileName = m_FileName.c_str();
	SetEXT(FileName.c_str(), "");
	LPCSTR pEXT = GetEXT(m_FileName.c_str());
	SaveDialog->FileName = FileName;
	SaveDialog->DefaultExt = pEXT;
	if( !strcmpi(pEXT, "SUM") ) SaveDialog->FilterIndex = 2;
	if( !strcmpi(pEXT, "CHK") ) SaveDialog->FilterIndex = 3;
	if( !strcmpi(pEXT, "ANA") ) SaveDialog->FilterIndex = 4;
	if( SaveDialog->Execute() ) {
		// Options + OverwritePrompt = True thus no need to check.
		REdit->Lines->SaveToFile(SaveDialog->FileName);
		m_FileName = SaveDialog->FileName;
		REdit->Modified = FALSE;
	 }
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::KFXClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::PopupMenuPopup(TObject *Sender)
{
	KEClick(NULL);
	for( int i = 0; i < PopupMenu->Items->Count; i++ ){
		PopupMenu->Items->Items[i]->Enabled = KE->Items[i]->Enabled;
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::KSSClick(TObject *Sender)
{
	FindDialog->Execute();
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::FindDialogFind(TObject *Sender)
{
	int FoundAt, StartPos, ToEnd;
	  // 検索は現在選択されている場所から開始される
	  // 何も選択されていない場合は，
	  // テキストの最初から開始される
	if(REdit->SelLength){
		StartPos = REdit->SelStart + REdit->SelLength;
	}
	else {
		StartPos = 0;
	}
	// ToEnd は検索範囲の文字数を示す
	ToEnd = REdit->Text.Length() - StartPos;


	TSearchTypes stype;
	if( FindDialog->Options.Contains(frMatchCase) ) stype << stMatchCase;
	if( FindDialog->Options.Contains(frWholeWord) ) stype << stWholeWord;
	FoundAt = REdit->FindText(FindDialog->FindText, StartPos, ToEnd, stype);
	if( FoundAt != -1 ){
		REdit->SetFocus();
		REdit->SelStart = FoundAt;
		REdit->SelLength = FindDialog->FindText.Length();
	}
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::KSClick(TObject *Sender)
{
	KSN->Enabled = !FindDialog->FindText.IsEmpty();
}
//---------------------------------------------------------------------------
void __fastcall TFileEdit::KFNClick(TObject *Sender)
{
	Close();

	char bf[256];
	sprintf(bf, "NOTEPAD %s", m_FileName.c_str());
	WinExec(bf, SW_SHOWDEFAULT);
}
//---------------------------------------------------------------------------


