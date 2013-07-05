//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "ComLib.h"
#include "TextEdit.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"

//TTextEditDlg *TextEditDlg;
//---------------------------------------------------------------------
__fastcall TTextEditDlg::TTextEditDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	ap = NULL;
	Memo->Font->Pitch = fpFixed;
	EntryAlignControl();
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		CancelBtn->Caption = "Cancel";
		FontBtn->Caption = "Font";
		Memo->Font->Name = "Courier New";
		Memo->Font->Charset = 0;
	}
}
//---------------------------------------------------------------------
int __fastcall TTextEditDlg::Execute(AnsiString &as, int flag, LPCSTR pTitle /* = NULL*/)
{
	CWaitCursor tw;
	ap = as.c_str();
	if( flag != TRUE ){
		OKBtn->Visible = FALSE;
		CancelBtn->Caption = MsgEng ? "Close":"閉じる";
		Memo->ReadOnly = TRUE;
	}
	if( pTitle != NULL ){
		Caption = pTitle;
	}
	tw.Delete();
	int r = ShowModal();
	if( r == IDOK ){
		if( Memo->Modified == FALSE ) return FALSE;
		if( flag == TRUE ){
			as = Memo->Text;
		}
		return TRUE;
	}
	return FALSE;
}

void __fastcall TTextEditDlg::FontBtnClick(TObject *Sender)
{
	FontDialog->Font->Assign(Memo->Font);
	if( FontDialog->Execute() == TRUE ){
		CWaitCursor tw;
		Memo->Font->Assign(FontDialog->Font);
		Memo->Update();
	}
}
//---------------------------------------------------------------------------
// サイズ変更コントロールの登録
void __fastcall TTextEditDlg::EntryAlignControl(void)
{
	RECT	rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = 792-1;
	rc.bottom = 572-1;

	AlignList.EntryControl(OKBtn, &rc, OKBtn->Font);
	AlignList.EntryControl(CancelBtn, &rc, CancelBtn->Font);
	AlignList.EntryControl(FontBtn, &rc, FontBtn->Font);
	AlignList.EntryControl(Memo, &rc, Memo->Font);

	int CX = ::GetSystemMetrics(SM_CXFULLSCREEN);
	int CY = ::GetSystemMetrics(SM_CYFULLSCREEN);
//	int CX = ::GetSystemMetrics(SM_CXSCREEN);
//	int CY = ::GetSystemMetrics(SM_CYSCREEN);
	if( (CX < Width)||(CY < Height) ){
		Top = 0;
		Left = 0;
		Width = CX;
		Height = CY;
	}
	FormCenter(this, CX, CY);
	if( Owner != NULL ){
		WindowState = ((TForm *)Owner)->WindowState;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextEditDlg::FormResize(TObject *Sender)
{
	CWaitCursor tw;
	AlignList.NewAlign(this);
}
//---------------------------------------------------------------------------
void __fastcall TTextEditDlg::FormPaint(TObject *Sender)
{
	if( ap == NULL ) return;
	CWaitCursor tw;
	Memo->Text = ap;
	ap = NULL;
	Memo->Update();
}
//---------------------------------------------------------------------------
// テキストヘルプの表示
void __fastcall ShowHelp(TForm *pForm, LPCSTR pName)
{
	char	bf[2048];
	char	Name[256];
	FILE	*fp;

	sprintf(Name, "%s%s", BgnDir, pName);
	CWaitCursor w;
	if( (fp = fopen(Name, "rt"))!=NULL ){
		if( sys.m_HelpNotePad ){
			fclose(fp);
			sprintf( bf, "NOTEPAD.EXE %s", Name);
			WinExec(bf, SW_SHOWDEFAULT);
		}
		else {
			AnsiString	in;
			while(!feof(fp)){
				if( fgets(bf, 2048, fp) != NULL ){
					ClipLF(bf);
					in += bf;
					in += "\r\n";
				}
			}
			fclose(fp);
			TTextEditDlg *Box = new TTextEditDlg(pForm);
			Box->Execute(in, FALSE, pName);
			delete Box;
		}
	}
	else {
		ErrorMB( MsgEng ? "'%s' was not found.\r\n\r\nProbably this file will be provided in the future." : "'%s'が見つかりません.", Name);
	}
}
//---------------------------------------------------------------------------

