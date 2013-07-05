//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TextDlg.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TTextDlgBox *TextDlgBox;
//---------------------------------------------------------------------
__fastcall TTextDlgBox::TTextDlgBox(HWND hWnd)
	: TForm(hWnd)
{
	MultProc();
	m_DisEvent = 1;
	m_pBitmap = NULL;
	m_Change = 0;
	m_DropProfile = 0;
	m_FText = "";
	if( sys.m_MsgEng ){
		Font->Name = "Arial";
		Font->Charset = ANSI_CHARSET;

		Caption = "Set text and colors - "VER"  "TTL;
		CancelBtn->Caption = "Cancel";
		FontBtn->Caption = "Font";
		SBM->Caption = "Mac";
		GBG->Caption = "Gradation";
		GBS->Caption = "Shadow";
		GBA->Caption = "Misc";
		CBFixed->Caption = "Fixed box-size";
		CBZero->Caption = "Slash zero";
		SBLD->Caption = "Del";
		SBLW->Caption = "Add";
		GBY->Caption = "Style profiles";
		L1->Caption = "Align";
		L2->Caption = "Style name";
		SBYD->Caption = "Del";
		SBYW->Caption = "Add";
		CBY->Text = "Untitled";
		SBM->Hint = "Insert macro-keyword";
		SBC->Hint = "Choose color's component";
		SBLD->Hint = "Delete the text in the list";
		SBLW->Hint = "Add current text into the list";
		SBYD->Hint = "Delete the style in the profiles";
		SBYW->Hint = "Add current style into the profiles";
		SB3DD->Hint = "Make the shadow shorter";
		SB3DI->Hint = "Make the shadow longer";
		FontBtn->Hint = "Choose font";
		SBFD->Hint = "Make the font smaller";
		SBFI->Hint = "Make the font bigger";
	}
	else {
		Caption = "テキストと色の設定 - "VER"  "TTLJ;
		CBY->Text = "無題";
	}
	SBFD->Glyph->Assign(SB3DD->Glyph);
	SBFI->Glyph->Assign(SB3DI->Glyph);
	MultProc();
	if( sys.m_pTextList == NULL ){
		sys.m_pTextList = new TStringList;
		LoadStringsFromIniFile(sys.m_pTextList, "Strings", ININAME);
		MultProc();
		if(!sys.m_pTextList->Count){
			sys.m_pTextList->Add("%c\\r\\nur %r\\r\\nde %m");
			sys.m_pTextList->Add("Tnx for nice PIC\\r\\nur %r\\r\\nde %m");
			sys.m_pTextList->Add("%n\\r\\nBest 73\\r\\nde %m");
		}
	}
	ASSERT(sys.m_pTextList);
	CBList->Items->Assign(sys.m_pTextList);
	if( sys.m_pStyleList == NULL ){
		sys.m_pStyleList = new TStringList;
		LoadStringsFromIniFile(sys.m_pStyleList, "Styles", ININAME);
		MultProc();
	}
	CBY->Items->Assign(sys.m_pStyleList);
	CBY->Sorted = TRUE;
	ShowHint = sys.m_WinNT;
	MultProc();
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::FormDestroy(TObject *Sender)
{
	if( m_pBitmap != NULL ) delete m_pBitmap;
	MultProc();
	ASSERT(sys.m_pTextList);
	sys.m_pTextList->Assign(CBList->Items);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::TopMost(void)
{
	::SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::NoTopMost(void)
{
	::SetWindowPos(Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateUI(void)
{
	m_DisEvent++;
	int f = SBG1->Down ? FALSE : TRUE;
	PG2->Visible = f;
	PG3->Visible = f;
	PG4->Visible = f;
	SBC->Visible = f && sys.m_fColorComp;
	f = !SBS1->Down;
	if( !f ) CB3D->Checked = FALSE;
	CB3D->Enabled = f;
	PB1->Visible = f && (!SBS2->Down || !CB3D->Checked);
	PB2->Visible = f && (SBS4->Down || CB3D->Checked);
	PB3->Visible = f && CB3D->Checked;
	UDL->Enabled = f && (SBS3->Down || !CB3D->Checked);
	SBLW->Enabled = (!m_FText.IsEmpty() && CBList->Items->IndexOf(m_FText) < 0) ? TRUE : FALSE;
	SBLD->Enabled = CBList->ItemIndex >= 0 ? TRUE : FALSE;
	SBYW->Enabled = !CBY->Text.IsEmpty();
	SBYD->Enabled = !CBY->Text.IsEmpty() && (sys.m_pStyleList->IndexOf(CBY->Text) >= 0);
	UDX->Enabled = CB3D->Checked;
	UDY->Enabled = CB3D->Checked;
	SB3DI->Enabled = CB3D->Checked;
	SB3DD->Enabled = CB3D->Checked;
	if( sys.m_WinNT ){
		char bf[32];
		sprintf(bf, "W = %d", UDL->Position);
		UDL->Hint = bf;
		sprintf(bf, "3D(Y) = %d", UDY->Position);
		UDY->Hint = bf;
		sprintf(bf, "3D(X) = %d", UDX->Position);
		UDX->Hint = bf;
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateMemoFont(BYTE charset)
{
	switch(charset){
		case SHIFTJIS_CHARSET:
		case HANGEUL_CHARSET:
		case CHINESEBIG5_CHARSET:   //
		case 134:       // 簡略
			break;
		default:
			charset = Font->Charset;
			break;
	}
	m_DisEvent++;
	if( Memo->Font->Charset != charset ){
		Memo->Font->Charset = charset;
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------
int __fastcall TTextDlgBox::Execute(CItem *pItem)
{
	MultProc();
	m_Item.Copy(pItem);
	UpdateDialog(&m_Item);
	UpdateBitmap();
	m_DisEvent = 0;
	MultProc();
	if( ShowModal() == IDOK ){
		MultProc();
		pItem->Copy(&m_Item);
		if( pItem->m_ASize ){
			pItem->m_OrgXW = -1;
		}
		MultProc();
		return TRUE;
	}
	else {
		MultProc();
		return FALSE;
	}
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateFText(void)
{
	AnsiString as;
	if( Memo->Text.IsEmpty() ){
		m_FText = "";
	}
	else {
		::CrLf2Yen(as, Memo->Text);
		LPSTR p = as.c_str();
		if( *p ){
			*(p+strlen(p)-1) = 0;
			m_FText = p+1;
		}
	}
	m_DisEvent++;
	CBList->ItemIndex = CBList->Items->IndexOf(m_FText);
	m_DisEvent--;
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateItem(CItem *pItem)
{
	UpdateUI();
	if( SBG1->Down ){
		pItem->m_Grade = 0;
	}
	else if( SBG2->Down ){
		pItem->m_Grade = 1;
	}
	else if( SBG3->Down ){
		pItem->m_Grade = 2;
	}
	else {
		pItem->m_Grade = 3;
	}
	if( SBS1->Down ){
		pItem->m_Shadow = 0;
	}
	else if( SBS2->Down ){
		pItem->m_Shadow = 1;
	}
	else if( SBS3->Down ){
		pItem->m_Shadow = 2;
	}
	else {
		pItem->m_Shadow = 3;
	}
	if( SBA1->Down ){
		pItem->m_Align = 0;
	}
	else if( SBA2->Down ){
		pItem->m_Align = 1;
	}
	else {
		pItem->m_Align = 2;
	}
	pItem->m_ColB[0].c = PB1->Color;
	pItem->m_ColB[1].c = PB2->Color;
	pItem->m_ColB[2].c = PB3->Color;
	pItem->m_ColG[0].c = PG1->Color;
	pItem->m_ColG[1].c = PG2->Color;
	pItem->m_ColG[2].c = PG3->Color;
	pItem->m_ColG[3].c = PG4->Color;
	pItem->m_Line = UDL->Position;
	pItem->m_ASize = !CBFixed->Checked;
	pItem->m_Zero = CBZero->Checked;
	pItem->m_Text = Memo->Text.c_str();
	pItem->m_3d = (CB3D->Enabled && CB3D->Checked) ? 0x00010000 : 0;
	pItem->m_3d |= ((UDY->Position << 8)&0x0000ff00);
	pItem->m_3d |= (UDX->Position & 0x000000ff);
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateDialog(CItem *pItem)
{
	m_DisEvent++;
	switch(pItem->m_Grade){
		case 1:
			SBG2->Down = TRUE;
			break;
		case 2:
			SBG3->Down = TRUE;
			break;
		case 3:
			SBG4->Down = TRUE;
			break;
		default:
			SBG1->Down = TRUE;
			break;
	}
	switch(pItem->m_Shadow){
		case 1:
			SBS2->Down = TRUE;
			break;
		case 2:
			SBS3->Down = TRUE;
			break;
		case 3:
			SBS4->Down = TRUE;
			break;
		default:
			SBS1->Down = TRUE;
			break;
	}
	switch(pItem->m_Align){
		case 1:
			SBA2->Down = TRUE;
			break;
		case 2:
			SBA3->Down = TRUE;
			break;
		default:
			SBA1->Down = TRUE;
			break;
	}
	PB1->Color = pItem->m_ColB[0].c;
	PB2->Color = pItem->m_ColB[1].c;
	PB3->Color = pItem->m_ColB[2].c;
	PG1->Color = pItem->m_ColG[0].c;
	PG2->Color = pItem->m_ColG[1].c;
	PG3->Color = pItem->m_ColG[2].c;
	PG4->Color = pItem->m_ColG[3].c;
	UDL->Position = SHORT(pItem->m_Line);
	CBFixed->Checked = !pItem->m_ASize;
	CBZero->Checked = pItem->m_Zero;
	CB3D->Checked = pItem->m_3d & 0x00010000;
	int xm = pItem->m_3d & 0x000000ff;
	int ym = (pItem->m_3d >> 8) & 0x000000ff;
	if( xm & 0x0080 ) xm |= 0xffffff80;
	if( ym & 0x0080 ) ym |= 0xffffff80;
	UDY->Position = SHORT(ym);
	UDX->Position = SHORT(xm);
	if( pItem->m_pFont != NULL ){
		UpdateMemoFont(pItem->m_pFont->Charset);
	}
	Memo->Text = pItem->m_Text.c_str();
	UpdateFText();
	m_DisEvent--;
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateBitmap(void)
{
	MultProc();
	m_Change = 0;
	UpdateItem(&m_Item);
	char bf[1024];
	sys.m_fConvMac(bf, m_Item.GetText(), sizeof(bf)-1);
	MultProc();
	if( m_pBitmap == NULL ){
		int xw = m_Item.m_OrgXW;
		int yw = m_Item.m_OrgYW;
		if( (xw < 0)||(yw < 0) ){
			xw = 160;
			yw = 128;
		}
		m_pBitmap = CreateBitmap(xw, yw);
	}
	FillBitmap(m_pBitmap, m_Item.m_Back);
	MultProc();
	m_pBitmap = m_Item.Draw(m_pBitmap, bf);
	PBox->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxPaint(TObject *Sender)
{
	if( m_pBitmap == NULL ) return;

	Graphics::TBitmap *pBitmap = m_pBitmap;
	if( sys.m_BitPixel == 16 ){
		pBitmap = new Graphics::TBitmap;
		pBitmap->Width = m_pBitmap->Width;
		pBitmap->Height = m_pBitmap->Height;
		pBitmap->Canvas->Draw(0, 0, m_pBitmap);
	}
	pBitmap->Transparent = TRUE;
	pBitmap->TransparentMode = tmFixed;
	pBitmap->TransparentColor = pBitmap->Canvas->Pixels[0][0];
	PBox->Canvas->Draw(0, 0, pBitmap);
	pBitmap->Transparent = FALSE;
	if( pBitmap != m_pBitmap ) delete pBitmap;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::MemoChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	m_Change = 2;
	Timer->Enabled = TRUE;
	UpdateFText();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::TimerTimer(TObject *Sender)
{
	if( m_Change ){
		m_Change--;
		if( !m_Change ){
			Timer->Enabled = FALSE;
			UpdateBitmap();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBG1Click(TObject *Sender)
{
	if( m_DisEvent ) return;

	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PB1Click(TObject *Sender)
{
	TPanel *pPanel = (TPanel *)Sender;
	InitCustomColor(ColorDialog);
	AddCustomColor(ColorDialog, PG1->Color);
	AddCustomColor(ColorDialog, PG2->Color);
	AddCustomColor(ColorDialog, PG3->Color);
	AddCustomColor(ColorDialog, PG4->Color);
	AddCustomColor(ColorDialog, PB1->Color);
	AddCustomColor(ColorDialog, PB2->Color);
	AddCustomColor(ColorDialog, m_Item.m_Back.c);
	
	ColorDialog->Color = pPanel->Color;
	NoTopMost();
	int r = ColorDialog->Execute();
	TopMost();
	if( r == TRUE ){
		pPanel->Color = ColorDialog->Color;
		UpdateBitmap();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::FormActivate(TObject *Sender)
{
	TopMost();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::FormDeactivate(TObject *Sender)
{
	NoTopMost();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UDLClick(TObject *Sender, TUDBtnType Button)
{
	if( m_DisEvent ) return;

	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::FontBtnClick(TObject *Sender)
{
	FontDialog->Font->Assign(m_Item.m_pFont);
	FontDialog->Font->Color = PG1->Color;
	NoTopMost();
	int r = FontDialog->Execute();
	TopMost();
	if( r == TRUE ){
		m_Item.m_pFont->Assign(FontDialog->Font);
		PG1->Color = FontDialog->Font->Color;
		m_Item.UpdateFont();
		if( m_Item.m_pFont != NULL ){
			UpdateMemoFont(m_Item.m_pFont->Charset);
		}
		UpdateBitmap();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBMClick(TObject *Sender)
{
	if( sys.m_fGetMacKey ){
		char bf[256];
		NoTopMost();
		int r = sys.m_fGetMacKey(bf, sizeof(bf));
		TopMost();
		if( r ){
			Memo->SetFocus();
			for( LPCSTR p = bf; *p; p++ ){
				::PostMessage(Memo->Handle, WM_CHAR, *p, 0);
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBCClick(TObject *Sender)
{
	if( sys.m_fColorComp ){
		UCOL    col[4];
		col[0].c = PG1->Color;
		col[1].c = PG2->Color;
		col[2].c = PG3->Color;
		col[3].c = PG4->Color;
		NoTopMost();
		int r = sys.m_fColorComp(&col[0].d);
		TopMost();
		if( r ){
			PG1->Color = col[0].c;
			PG2->Color = col[1].c;
			PG3->Color = col[2].c;
			PG4->Color = col[3].c;
			UpdateBitmap();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBLWClick(TObject *Sender)
{
	m_DisEvent++;
	CBList->Items->Add(m_FText);
	CBList->ItemIndex = CBList->Items->IndexOf(m_FText);
	UpdateUI();
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBLDClick(TObject *Sender)
{
	m_DisEvent++;
	if( CBList->ItemIndex >= 0 ){
		int n = CBList->ItemIndex;
		CBList->ItemIndex = -1;
		CBList->Items->Delete(n);
		UpdateUI();
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::CBListChange(TObject *Sender)
{
	if( m_DisEvent ) return;
	if( CBList->ItemIndex < 0 ) return;

	m_DisEvent++;
	AnsiString cs;
	AnsiString as = CBList->Items->Strings[CBList->ItemIndex];
	m_FText = as;
	::Yen2CrLf(cs, as);
	Memo->Text = cs;
	UpdateBitmap();
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBYWClick(TObject *Sender)
{
	m_DisEvent++;
	int r = sys.m_pStyleList->IndexOf(CBY->Text);
	if( r < 0 ) r = sys.m_pStyleList->Count;

	CItem *pItem = new CItem;
	pItem->Copy(&m_Item);
	pItem->m_Text = CBY->Text;
	char bf[64];
	sprintf(bf, "Style.%u", r);
	pItem->SaveToInifile(bf, ININAME);
	delete pItem;

	if( r == sys.m_pStyleList->Count ){
		CBY->Items->Add(CBY->Text);
		sys.m_pStyleList->Add(CBY->Text);
		UpdateUI();
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBYDClick(TObject *Sender)
{
	m_DisEvent++;
	int r = sys.m_pStyleList->IndexOf(CBY->Text);
	if( r >= 0 ){
		char bf[64];
		CItem *pItem = new CItem;
		int i;
		for( i = r; i < sys.m_pStyleList->Count - 1; i++ ){
			sprintf(bf, "Style.%u", i+1);
			pItem->LoadFromInifile(bf, ININAME);
			sprintf(bf, "Style.%u", i);
			pItem->SaveToInifile(bf, ININAME);
			MultProc();
		}
		sprintf(bf, "Style.%u", i);
		pItem->DeleteInifile(bf, ININAME);
		delete pItem;
		sys.m_pStyleList->Delete(r);
		r = CBY->Items->IndexOf(CBY->Text);
		if( r >= 0 ) CBY->Items->Delete(r);
		UpdateUI();
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::CBYChange(TObject *Sender)
{
	if( m_DisEvent ) return;
	if( !m_DropProfile ){
		UpdateUI();
		return;
	}
	m_DropProfile = 0;

	m_DisEvent++;
	int r = sys.m_pStyleList->IndexOf(CBY->Text);
	if( r >= 0 ){
		AnsiString as = m_Item.m_Text;
		CItem *pItem = new CItem;
		char bf[64];
		sprintf(bf, "Style.%u", r);
		pItem->LoadFromInifile(bf, ININAME);
		m_Item.Copy(pItem);
		delete pItem;
		m_Item.m_Text = as;
		UpdateDialog(&m_Item);
		UpdateBitmap();
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::CBYKeyPress(TObject *Sender, char &Key)
{
	m_DropProfile = 0;
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::CBYDropDown(TObject *Sender)
{
	m_DropProfile = 1;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( m_DisEvent ) return;

	m_DisEvent++;
	if( Button == mbRight ){
		CB3D->Checked = FALSE;
	}
	else {
		CB3D->Checked = TRUE;
		int xc = m_pBitmap->Width / 2;
		int yc = m_pBitmap->Height / 2;
		int x = X - xc;
		int y = Y - yc;
		x = x * 48 / xc;
		y = y * 48 / yc;
		UDX->Position = SHORT(x);
		UDY->Position = SHORT(y);
	}
	m_DisEvent--;
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UD3D(int a)
{
	m_DisEvent++;
	int x = UDX->Position;
	int y = UDY->Position;
	int xs = (x >= 0) ? 1 : -1;
	int ys = (y >= 0) ? 1 : -1;
	x = ABS(x);
	y = ABS(y);
	if( (x == y) && !(x & 1) ){
		x += a;
		y += a;
		if( x < 4 ) x = 4;
		if( y < 4 ) y = 4;
		if( x > 48 ) x = 48;
		if( y > 48 ) y = 48;
	}
	else {
		if( x < y ) x = y;
		x &= 0xfffffffc;
		y = x;
	}
	x *= xs; y *= ys;
	UDX->Position = SHORT(x);
	UDY->Position = SHORT(y);
	m_DisEvent--;
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SB3DIClick(TObject *Sender)
{
	UD3D((Sender == SB3DI) ? 4 : -4);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBFDClick(TObject *Sender)
{
	if( m_Item.m_pFont == NULL ) return;

	if( Sender == SBFD ){
		if( m_Item.m_pFont->Height < -1 ) m_Item.m_pFont->Height++;
		UpdateBitmap();
	}
	else {
		m_Item.m_pFont->Height--;
		UpdateBitmap();
	}
}
//---------------------------------------------------------------------------
 
