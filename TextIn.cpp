//Copyright+LGPL

//-----------------------------------------------------------------------------------------------------------------------------------------------
// Copyright 2000-2013 Makoto Mori, Nobuyuki Oba
//-----------------------------------------------------------------------------------------------------------------------------------------------
// This file is part of MMSSTV.

// MMSSTV is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// MMSSTV is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License along with MMTTY.  If not, see 
// <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TextIn.h"
#include "Main.h"
#include "MacroKey.h"
#include "ColorSet.h"
#include "BitMask.h"
#include "PerSpect.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
#define PARANAME    "PARALIST.BIN"
//TTextInDlg *TextInDlg;
int DrawParaF = 0;
//---------------------------------------------------------------------
__fastcall TTextInDlg::TTextInDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	m_DisEvent = 1;
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	m_pFontDialog = Mmsstv->FontDialog;
	if( MsgEng ){
		Caption = "Text & Colors";
		CancelBtn->Caption = "Cancel";
		CmdBtn->Caption = "Macro";
		Ltext->Caption = "Text";
		GBEf->Caption = "";
		RGGrade->Caption = "Gradation";
		RGGrade->Items->Strings[0] = "NONE";
		RGGrade->Items->Strings[1] = "Horizontal";
		RGGrade->Items->Strings[2] = "Vertical";
		RGGrade->Items->Strings[3] = "Bitmap mask";
		RGShadow->Caption = "Shadow";
		RGShadow->Items->Strings[0] = "NONE";
		RGShadow->Items->Strings[1] = "Siege";
		RGShadow->Items->Strings[2] = "Shadow 1";
		RGShadow->Items->Strings[3] = "Shadow 2";
		RGShadow->Items->Strings[4] = "Shadow 3";
		RGShadow->Items->Strings[5] = "Shadow 4";
		RGShadow->Items->Strings[6] = "Shadow 5";
		RGShadow->Items->Strings[7] = "Shadow 6";
		TblBtn->Caption = "Components";
		MaskBtn->Caption = "Bitmap";
		if( Font->Charset == HANGEUL_CHARSET ){
			CBZero->Caption = "Slash zero (\xa8\xaa)";
		}
		else {
			CBZero->Caption = "Slash zero (ﾘ)";
		}
		LRot->Caption = "Rotation";
		CBRA->Caption = "Right justify";
		CBTF->Caption = "Perspective";
		GBP->Caption = "Style profiles";
		MemBtn->Caption = "Add";
		DelBtn->Caption = "Del";
		CBFont->Caption = "Save font";
		CBV->Caption = "Vert";
        FontBtn->Caption = "Font";
		FontBtn->Hint = "Choose font";
		SBFD->Hint = "Make the font smaller";
		SBFI->Hint = "Make the font bigger";
        SBB->Hint = "Bold";
        SBI->Hint = "Italic";
		SBMSG->Hint = "Show messages list";
	}
	LineBtn->Caption = RGShadow->Items->Strings[1];
	TFBtn->Caption = CBTF->Caption;
	EditText->Items->Clear();
	for( int i = 0; i < 16; i++ ){
		if( sys.m_TextList[i].IsEmpty() ) break;
		EditText->Items->Add(sys.m_TextList[i]);
	}
	pBrushBitmap = new Graphics::TBitmap();
	AdjustBitmapFormat(pBrushBitmap);
	pBrushBitmap->Width = 8;
	pBrushBitmap->Height = 8;
	pItem = NULL;
	m_ReqChange = 0;
	m_DropText = 0;
	m_DropProfile = 0;
	m_MouseHold = 0;

	if( !DrawParaF ){
		DrawParaF++;
		char bf[256];
		sprintf(bf, "%s"PARANAME, StockDir);
		if( IsFile(bf) ){
			MultProc();
			LoadTemplate(&DrawPara, bf, PBox->Canvas);
		}
	}
	m_tSBF[0] = SBF1;
	m_tSBF[1] = SBF2;
	m_tSBF[2] = SBF3;
	m_tSBF[3] = SBF4;
    UpdateSBF();
	InitCMB();
	MultProc();
	Memo->Visible = FALSE;
    ShowHint = WinNT;
}
//---------------------------------------------------------------------
__fastcall TTextInDlg::~TTextInDlg()
{
	delete pBrushBitmap;
}
//---------------------------------------------------------------------
void __fastcall TTextInDlg::UpdateBoxFont(void)
{
	BYTE cs;
	switch(m_pFontDialog->Font->Charset){
		case SHIFTJIS_CHARSET:
		case HANGEUL_CHARSET:
		case CHINESEBIG5_CHARSET:   //
		case 134:       // 簡略
			cs = m_pFontDialog->Font->Charset;
			break;
		default:
			cs = Font->Charset;
			break;
	}
	if( EditText->Font->Charset != cs ){
		EditText->Font->Charset = cs;
	}
    TFontStyles ts = m_pFontDialog->Font->Style;
   	m_Style = FontStyle2Code(ts);
    SBB->Down = m_Style & FSBOLD;
    SBI->Down = m_Style & FSITALIC;
}
//---------------------------------------------------------------------
void __fastcall TTextInDlg::AddTextList(LPCSTR pStr)
{
	int i, j;
	for( i = 0; i < 16; i++ ){
		if( !strcmp(sys.m_TextList[i].c_str(), pStr) ){
			break;
		}
	}
	if( i ){
		if( i >= 16 ) i--;
		for( j = i; j > 0; j-- ){
			sys.m_TextList[j] = sys.m_TextList[j-1].c_str();
		}
		sys.m_TextList[0] = pStr;
	}
}
//---------------------------------------------------------------------
void __fastcall TTextInDlg::UpdateUI(void)
{
	int f = RGGrade->ItemIndex ? TRUE : FALSE;
	if( RGGrade->ItemIndex == 3 ){
		f = FALSE;
		PC1->Visible = f;
		TblBtn->Enabled = f;
		PC2->Visible = f;
		PC3->Visible = f;
		PC4->Visible = f;
		MaskBtn->Enabled = TRUE;
	}
	else {
		TblBtn->Enabled = f;
		PC1->Visible = TRUE;
		PC2->Visible = f;
		PC3->Visible = f;
		PC4->Visible = f;
		MaskBtn->Enabled = FALSE;
	}
	PCS->Visible = RGShadow->ItemIndex;
	PCB->Visible = RGShadow->ItemIndex >= 4 ? TRUE : FALSE;
	OKBtn->Enabled = !EditText->Text.IsEmpty();
	TFBtn->Enabled = CBTF->Checked;
	LineBtn->Enabled = (RGShadow->ItemIndex == 1)||(RGShadow->ItemIndex==6) ? TRUE : FALSE;
	f = CBStack->Checked;
	L1->Enabled = f;
	L2->Enabled = f;
	UDSX->Enabled = f;
	UDSY->Enabled = f;
	SEX->Enabled = f;
	SEY->Enabled = f;
	f = !CBV->Checked;
	Rot->Enabled = f;
	UDRot->Enabled = f;
	UDV->Enabled = !f;
    EV->Enabled = !f;
	Memo->Visible = SBMSG->Down;

	MemBtn->Enabled = !CMBPro->Text.IsEmpty();
	DelBtn->Enabled = (!CMBPro->Text.IsEmpty()) && (CMBPro->Items->IndexOf(CMBPro->Text) >= 0);
}
//---------------------------------------------------------------------
void __fastcall TTextInDlg::UpdateItem(CDrawText *pm, int fsw)
{
	m_DisEvent++;
	if( fsw ){
		m_pFontDialog->Font->Assign(pm->pFont);
		UpdateBoxFont();
	}
	RGGrade->ItemIndex = pm->m_Grade;
	RGShadow->ItemIndex = pm->m_Shadow;
	CBZero->Checked = pm->m_Zero;
	EditText->Text = pm->m_Text;
	PC1->Color = pm->m_Col1;
	PC2->Color = pm->m_Col2;
	PC3->Color = pm->m_Col3;
	PC4->Color = pm->m_Col4;
	PCS->Color = pm->m_ColS;
	PCB->Color = pm->m_ColB;
//    Rot->Text = pItem->m_Rot;
	UDRot->Position = SHORT(pm->m_Rot);
	CBRA->Checked = pm->m_RightAdj;
	CBTF->Checked = pm->m_PerSpect;
	CBStack->Checked = pm->m_Stack;
	UDSX->Position = char(pm->m_StackPara & 0x000000ff);
	UDSY->Position = char((pm->m_StackPara>>8) & 0x000000ff);
	CBV->Checked = pm->m_Vert;
	UDV->Position = short(pm->m_VertH);
	UpdateUI();
	if( pm->pBrushBitmap != NULL ){
		pBrushBitmap->Width = pm->pBrushBitmap->Width;
		pBrushBitmap->Height = pm->pBrushBitmap->Height;
		pBrushBitmap->Canvas->Draw(0, 0, pm->pBrushBitmap);
	}
	else {
		MakeBitmapPtn(pBrushBitmap, 1024, 0, pm->m_Col1, clWhite);
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------
void __fastcall TTextInDlg::UpdatePara(CDrawText *pm)
{
	pm->m_Rot = UDRot->Position;
	pm->m_Grade = RGGrade->ItemIndex;
	pm->m_Shadow = RGShadow->ItemIndex;
	pm->m_Zero = CBZero->Checked;
	pm->m_Text = EditText->Text.IsEmpty() ? "TEXT" : AnsiString(EditText->Text).c_str();	//ja7ude 0521
	pm->m_Col1 = PC1->Color;
	pm->m_Col2 = PC2->Color;
	pm->m_Col3 = PC3->Color;
	pm->m_Col4 = PC4->Color;
	pm->m_ColS = PCS->Color;
	pm->m_ColB = PCB->Color;
	pm->m_Stack = CBStack->Checked;
	pm->m_StackPara = UDSX->Position & 0x000000ff;
	pm->m_StackPara |= (UDSY->Position << 8) & 0x0000ff00;
	pm->pFont->Assign(m_pFontDialog->Font);
	if( pm->m_Grade == 3 ){
		if( pm->pBrushBitmap != NULL ){
			delete pm->pBrushBitmap;
		}
		pm->pBrushBitmap = DupeBitmap(pBrushBitmap, -1);
	}
	else if( pItem->pBrushBitmap != NULL ){
		delete pItem->pBrushBitmap;
		pItem->pBrushBitmap = NULL;
	}
	pm->m_RightAdj = CBRA->Checked;
	pm->m_PerSpect = CBTF->Checked;
	if( pm != &m_Text ){
		memcpy(&pm->m_sperspect, &m_Text.m_sperspect, sizeof(pm->m_sperspect));
		pm->m_LineWidth = m_Text.m_LineWidth;
		pm->m_LineColor = m_Text.m_LineColor;
	}
	pm->m_Vert = CBV->Checked;
	pm->m_VertH = UDV->Position;
}
//---------------------------------------------------------------------
int __fastcall TTextInDlg::Execute(CDrawText *p)
{
	pItem = p;

	m_Text.Start(PBox->Canvas, 0, 0);       // キャンバスのセット
	m_Text.Copy(pItem, 0);
//    m_Text.UpdateText();

	MultProc();
	UpdateItem(pItem, TRUE);
	UpdateBitmap();
	MultProc();
	m_DisEvent = 0;
	int r = ShowModal();
	MultProc();
	if( Memo->Modified ) sys.m_Msg = Memo->Text;
	if( r == IDOK ){
		if( EditText->Text.IsEmpty() ){
			return FALSE;
		}
		else {
			UpdatePara(pItem);
			AddTextList(pItem->m_Text.c_str());
			MultProc();
			return TRUE;
		}
	}
	return FALSE;
}
//---------------------------------------------------------------------
void __fastcall TTextInDlg::RBGradeClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	UpdateUI();
	UpdateSample();
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::PCClick(TObject *Sender)
{
	TPanel *tp = (TPanel *)Sender;
    TColorDialog *pDialog = Mmsstv->ColorDialog;
	InitCustomColor(pDialog);
	AddCustomColor(pDialog, PC1->Color);
	AddCustomColor(pDialog, PC2->Color);
	AddCustomColor(pDialog, PC3->Color);
	AddCustomColor(pDialog, PC4->Color);
	AddCustomColor(pDialog, PCS->Color);
	AddCustomColor(pDialog, PCB->Color);
	AddCustomColor(pDialog, Mmsstv->DrawMain.m_TransCol);
	pDialog->Color = tp->Color;
	SetDisPaint();
	if( pDialog->Execute() == TRUE ){
		tp->Color = pDialog->Color;
		if( (Sender == PC1)||
			(Sender == PC2)||
			(Sender == PC3)||
			(Sender == PC4)
		){
			if( (PC1->Color == PC2->Color) && (PC3->Color == PC4->Color) ){
				if( YesNoMB(MsgEng ? "Change to two colors gradation?" : "2色のグラデーションに変更しますか?" ) == IDYES ){
					PC2->Color = GetCol(PC1->Color, PC4->Color, 2, 4);
					PC3->Color = GetCol(PC1->Color, PC4->Color, 3, 4);
				}
			}
		}
		UpdateSample();
	}
	ResDisPaint();
}
//---------------------------------------------------------------------------

void __fastcall TTextInDlg::FontBtnClick(TObject *Sender)
{
	m_pFontDialog->Font->Color = PC1->Color;
	SetDisPaint();
	if( m_pFontDialog->Execute() == TRUE ){
		PC1->Color = m_pFontDialog->Font->Color;
		UpdateSample();
		UpdateBoxFont();
	}
	ResDisPaint();
}
//---------------------------------------------------------------------------

void __fastcall TTextInDlg::CmdBtnClick(TObject *Sender)
{
	TMacroKeyDlg *pBox = new TMacroKeyDlg(this);
	AnsiString as = EditText->Text;
	if( pBox->Execute(as) > 0 ){
		HWND hWnd;
		if( SBMSG->Down ){
			Memo->SetFocus();
			Memo->SelLength = 0;
			hWnd = Memo->Handle;
		}
		else {
			EditText->SetFocus();
			EditText->SelStart = strlen(AnsiString(EditText->Text).c_str());	//ja7ude 0521
			EditText->SelLength = 0;
			hWnd = EditText->Handle;
		}
		for( LPCSTR p = as.c_str(); *p; p++ ){
			::PostMessage(hWnd, WM_CHAR, *p, 0);
		}
//        m_ReqChange = 5;
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::TblBtnClick(TObject *Sender)
{
	TColorSetDlg *pBox = new TColorSetDlg(this);
	TColor col[4];
	col[0] = PC1->Color;
	col[1] = PC2->Color;
	col[2] = PC3->Color;
	col[3] = PC4->Color;
	if( pBox->Execute(col) == TRUE ){
		PC1->Color = col[0];
		PC2->Color = col[1];
		PC3->Color = col[2];
		PC4->Color = col[3];
		UpdateSample();
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::EditTextDropDown(TObject *Sender)
{
	m_DropText = 1;
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::EditTextKeyPress(TObject *Sender, char &Key)
{
	m_DropText = 0;
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::EditTextChange(TObject *Sender)
{
	UpdateUI();
	if( m_DropText ){
		UpdateSample();
	}
	else {
		m_ReqChange = 5;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::MaskBtnClick(TObject *Sender)
{
	TBitMaskDlg *pBox = new TBitMaskDlg(this);
	pBox->Execute(pBrushBitmap, &m_Text);
	delete pBox;
	UpdateSample();
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::TFBtnClick(TObject *Sender)
{
	m_Text.m_PerSpect = CBTF->Checked;
	TPerSpectDlg *pBox = new TPerSpectDlg(this);
	int r = pBox->Execute(&m_Text);
	delete pBox;
	if( r == TRUE ){
		CBTF->Checked = m_Text.m_PerSpect;
		UpdateSample();
	}
}
//---------------------------------------------------------------------
void __fastcall TTextInDlg::UpdateSample(void)
{
	UpdateBitmap();
	PBox->Invalidate();
	m_ReqChange = 0;
	m_DropText = 0;
}
//---------------------------------------------------------------------
void __fastcall TTextInDlg::UpdateBitmap(void)
{
	if( m_Text.pBitmap == NULL ) return;

	UpdatePara(&m_Text);
	m_Text.UpdateText();
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::PBoxPaint(TObject *Sender)
{
#if 0
	if( CBStack->Checked && (UDSX->Position || UDSY->Position) && (RGShadow->ItemIndex < 4) ){
		int ax = UDSX->Position;
		int ay = UDSY->Position;
		int m = ABS(ax);
		if( ABS(ax) < ABS(ay) ) m = ABS(ay);
		int i;
		for( i = 0; i < m; i++ ){
			PBox->Canvas->Draw(i*ax/m, i*ay/m, m_Text.pBitmap);
		}
	}
	else {
		PBox->Canvas->Draw(0, 0, m_Text.pBitmap);
	}
#else
	PBox->Canvas->Draw(0, 0, m_Text.pBitmap);
#endif
	if( m_MouseHold ){
		PBox->Canvas->Pen->Color = clBlue;
		PBox->Canvas->MoveTo(0, m_MouseYC);
		PBox->Canvas->LineTo(PBox->Width, m_MouseYC);
		PBox->Canvas->MoveTo(m_MouseXC, 0);
		PBox->Canvas->LineTo(m_MouseXC, PBox->Height);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::CBZeroClick(TObject *Sender)
{
	UpdateSample();
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::TimerTimer(TObject *Sender)
{
	if( m_ReqChange ){
		m_ReqChange--;
		if( !m_ReqChange ){
			UpdateSample();
		}
	}
//	MemBtn->Enabled = !CMBPro->Text.IsEmpty();
//	DelBtn->Enabled = MemBtn->Enabled;
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::LineBtnClick(TObject *Sender)
{
	if( RGShadow->ItemIndex == 6 ){
		m_Text.m_ColS = PCS->Color;
	}
	if( m_Text.CDraw::Edit() == TRUE ){
		if( RGShadow->ItemIndex !=6 ){
			PCS->Color =  m_Text.m_ColS;
		}
		UpdateSample();
	}
}
//---------------------------------------------------------------------------
/*
	int d;

	sscanf(Rot->Text.c_str(), "%d", &d);
	if( (d >= -360) && (d < 360) ){
		if( d < 0 ) d += 360;
		pm->m_Rot = d;
	}
*/
void __fastcall TTextInDlg::UDRotClick(TObject *Sender, TUDBtnType Button)
{
	UpdateSample();
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::InitCMB(void)
{
	CMBPro->Items->Clear();
	for( int i = 0; i < DrawPara.m_Cnt; i++ ){
		CDrawText *pt = (CDrawText *)DrawPara.pBase[i];
		CMBPro->Items->Add(pt->m_Text);
	}
	CMBPro->Text = MsgEng ? "Untitled" : "無題";
    CBFont->Checked = DrawPara.m_SX & 1;
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::SaveCMB(void)
{
	DrawPara.m_SX = CBFont->Checked;
	char bf[256];
	sprintf(bf, "%s"PARANAME, StockDir);
	SaveTemplate(&DrawPara, bf);
    UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::MemBtnClick(TObject *Sender)
{
	CDrawText text;
	text.Copy(&m_Text);
	text.m_Text = CMBPro->Text.c_str();
	int f = 1;
	for( int i = 0; i < DrawPara.m_Cnt; i++ ){
		CDrawText *pt = (CDrawText *)DrawPara.pBase[i];
		if( pt->m_Text == text.m_Text ){
			DrawPara.DeleteItem(pt);
			f = 0;
			break;
		}
	}
	DrawPara.AddItemCopy(&text);
	if( f ) CMBPro->Items->Add(CMBPro->Text);
	SaveCMB();
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::CMBProChange(TObject *Sender)
{
	if( m_DisEvent ) return;
	if( !m_DropProfile ){
		UpdateUI();
    	return;
    }
	m_DropProfile = 0;

	int i;

	for( i = 0; i < DrawPara.m_Cnt; i++ ){
		CDrawText *pt = (CDrawText *)DrawPara.pBase[i];
		if( pt->m_Text == CMBPro->Text ){
			AnsiString text = EditText->Text.c_str();
			memcpy(&m_Text.m_sperspect, &pt->m_sperspect, sizeof(pt->m_sperspect));
			m_Text.m_LineWidth = pt->m_LineWidth;
			m_Text.m_LineColor = pt->m_LineColor;
			UpdateItem(pt, CBFont->Checked);
			EditText->Text = text.c_str();
			UpdateSample();
			break;
		}
	}
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::DelBtnClick(TObject *Sender)
{
	for( int i = 0; i < DrawPara.m_Cnt; i++ ){
		CDrawText *pt = (CDrawText *)DrawPara.pBase[i];
		if( pt->m_Text == CMBPro->Text ){
			DrawPara.DeleteItem(pt);
			InitCMB();
			SaveCMB();
			return;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::CMBProDropDown(TObject *Sender)
{
	m_DropProfile = 1;
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::CMBProKeyPress(TObject *Sender, char &Key)
{
	m_DropProfile = 0;
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::UDSXClick(TObject *Sender, TUDBtnType Button)
{
	RBGradeClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::CBVClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	if( CBV->Checked ){
		UDRot->Position = 0;
		ClearCodeErr();
	}
	UpdateUI();
	UpdateSample();
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::CBFontClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	if( CBFont->Checked ){
		m_DropProfile = 1;
		CMBProChange(NULL);
	}
    SaveCMB();
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::PBoxMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_MouseHold = 0;
	if( m_DisEvent ) return;

	m_DisEvent++;
	if( Button == mbRight ){
		CBStack->Checked = !CBStack->Checked;
	}
	else {
		CBStack->Checked = TRUE;
		if( m_Text.pBitmap == NULL ) return;
		m_MouseHold = TRUE;
		int XW = m_Text.pBitmap->Width;
		int YW = m_Text.pBitmap->Height;
		m_MouseXC = XW/2;
		m_MouseYC = YW/2;
		X = m_MouseXC - X;
		Y = m_MouseYC - Y;
		X = X * 64 / XW;
		UDSX->Position = short(X);
		UDSY->Position = short(Y);
		Screen->Cursor = crSizeAll;
	}
	m_DisEvent--;
	RBGradeClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::PBoxMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if( m_MouseHold ){
		X = m_MouseXC - X;
		Y = m_MouseYC - Y;
		short UX = UDSX->Position;
		short UY = UDSY->Position;
		m_DisEvent++;
		UDSX->Position = short(X);
		UDSY->Position = short(Y);
		m_DisEvent--;
		if( (UX != UDSX->Position) || (UY != UDSY->Position) ){
			RBGradeClick(NULL);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::PBoxMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( m_MouseHold ){
		m_MouseHold = 0;
		PBox->Invalidate();
	}
	Screen->Cursor = crDefault;
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::SBMSGClick(TObject *Sender)
{
	Memo->Visible = SBMSG->Down;
	if( SBMSG->Down ){
		Memo->Text = sys.m_Msg;
		Memo->Modified = FALSE;
		Memo->SelLength = 0;
		Memo->SetFocus();
	}
	else {
		if( Memo->Modified ){
			sys.m_Msg = Memo->Text;
			Memo->Modified = FALSE;
		}
		EditText->SetFocus();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::MemoDblClick(TObject *Sender)
{
	if( Memo->Lines->Count ){
		int e = Memo->SelStart;
		LPCSTR p = AnsiString(Memo->Text).c_str();	//ja7ude 0521
		int n = 0;
		for( int i = 0; i < e; i++, p++ ){
			if( *p == LF ) n++;
		}
		if( n > Memo->Lines->Count ) n = Memo->Lines->Count - 1;
		EditText->Text = Memo->Lines->Strings[n];
	}
	else {
		EditText->Text = "";
	}
	SBMSG->Down = FALSE;
	SBMSGClick(NULL);
	MultProc();
	UpdateSample();
}
//---------------------------------------------------------------------------

void __fastcall TTextInDlg::FontBtnMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		InvFontCharset(m_pFontDialog->Font);
		UpdateSample();
		UpdateBoxFont();
    }
}
//---------------------------------------------------------------------------
// SBFのインデックスを得る
int __fastcall TTextInDlg::GetSBFIndex(TObject *Sender)
{
    for( int i = 0; i < 4; i++ ){
		if( Sender == m_tSBF[i] ) return i;
    }
    return 0;
}
//---------------------------------------------------------------------------
// 登録フォントの呼び出し
void __fastcall TTextInDlg::SBFClick(TObject *Sender)
{
	int N = GetSBFIndex(Sender);
    FONTDATA *pData = &sys.m_RegFont[N];
    if( !pData->m_Height ) return;

    m_pFontDialog->Font->Name = pData->m_Name;
    m_pFontDialog->Font->Charset = pData->m_Charset;
//    m_pFontDialog->Font->Height = pData->m_Height;
	TFontStyles ts = Code2FontStyle(pData->m_Style);
    m_pFontDialog->Font->Style = ts;
	UpdateSample();
	UpdateBoxFont();
}
//---------------------------------------------------------------------------
// フォントの登録
void __fastcall TTextInDlg::SBFMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		int N = GetSBFIndex(Sender);
		m_pFontDialog->Font->Color = clBlack;
		SetDisPaint();
		TFont *pBakFont = new TFont;
        pBakFont->Assign(m_pFontDialog->Font);
		if( m_pFontDialog->Execute() == TRUE ){
	    	FONTDATA *pData = &sys.m_RegFont[N];
		    pData->m_Name = m_pFontDialog->Font->Name;
			pData->m_Charset = m_pFontDialog->Font->Charset;
    		pData->m_Height = m_pFontDialog->Font->Height;
            TFontStyles ts = m_pFontDialog->Font->Style;
            pData->m_Style = FontStyle2Code(ts);
		}
		ResDisPaint();
        m_pFontDialog->Font->Assign(pBakFont);
        delete pBakFont;
        UpdateSBF();
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::UpdateSBF(void)
{
	AnsiString as;
	FONTDATA *pData = sys.m_RegFont;
	for( int i = 0; i < 4; i++, pData++ ){
		if( pData->m_Height ){
			as = pData->m_Name;
    	    AddStyle(as, pData->m_Charset, pData->m_Style);
        }
        else {
			as = MsgEng ? "Load registered font" : "登録ﾌｫﾝﾄの読み出し";
        }
        as += MsgEng ? " (Registration = Right button)" : " (登録=右ﾎﾞﾀﾝ)";
		TSpeedButton *pButton = m_tSBF[i];
        pButton->Hint = as;
        pButton->Font->Color = pData->m_Height ? clBlack : clGrayText;
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::AddStyle(AnsiString &as, BYTE charset, DWORD style)
{
    switch(charset){
		case ANSI_CHARSET:
			as += MsgEng ? "/ANSI" : "/欧文";
        	break;
		case SHIFTJIS_CHARSET:
			as += MsgEng ? "/Japanese" : "/日本語";
        	break;
		case HANGEUL_CHARSET:
			as += MsgEng ? "/Korean(Hangul)" : "/ハングル";
			break;
		case CHINESEBIG5_CHARSET:   // 台湾
			as += MsgEng ? "/Chinese" : "/中国語(台湾)";
			break;
		case 134:                   // 中国語簡略
			as += MsgEng ? "/Chinese" : "/中国語(簡略)";
			break;
        default:
        	break;
    }
	if( style & FSBOLD ) as += MsgEng ? "/Bold" : "/太字";
	if( style & FSITALIC ) as += MsgEng ? "/Italic" : "/斜体";
	if( style & FSUNDERLINE ) as += MsgEng ? "/UnderLine" : "/下線";
	if( style & FSSTRIKEOUT ) as += MsgEng ? "/StrikeOut" : "/取消し線";
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::SBIClick(TObject *Sender)
{
	if( SBI->Down ){
		m_Style |= FSITALIC;
    }
    else {
		m_Style &= ~FSITALIC;
    }
	TFontStyles ts;
    ts = Code2FontStyle(m_Style);
    m_pFontDialog->Font->Style = ts;
	UpdateSample();
	UpdateBoxFont();
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::SBBClick(TObject *Sender)
{
	if( SBB->Down ){
		m_Style |= FSBOLD;
    }
    else {
		m_Style &= ~FSBOLD;
    }
	TFontStyles ts;
    ts = Code2FontStyle(m_Style);
    m_pFontDialog->Font->Style = ts;
	UpdateSample();
	UpdateBoxFont();
}
//---------------------------------------------------------------------------
void __fastcall TTextInDlg::SBFDMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	int ad = 0;
	int h = m_pFontDialog->Font->Height;
	if( Button == mbLeft ){
		if( Sender == SBFD ){
			if( h < -2 ) ad = 1;
        }
        else {
			ad = -1;
        }
    }
    else if( Button == mbRight ){
		if( Sender == SBFD ){
			if( h < -6 ) ad = 4;
        }
        else {
			ad = -4;
        }
    }
	if( ad ){
		m_pFontDialog->Font->Height = h + ad;
		UpdateSample();
		UpdateBoxFont();
    }
}
//---------------------------------------------------------------------------
