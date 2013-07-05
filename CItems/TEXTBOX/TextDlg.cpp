//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TextDlg.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TTextDlgBox *TextDlgBox;
//--------------------------------------------------------------------- 
#if UseHWND
__fastcall TTextDlgBox::TTextDlgBox(HWND hWnd)
	: TForm(hWnd)
#else
__fastcall TTextDlgBox::TTextDlgBox(TComponent* AOwner)
	: TForm(AOwner)
#endif
{
	MultProc();
	m_DisEvent = 1;
	m_pBitmap = NULL;
	m_Change = 0;
	m_DropProfile = 0;
	m_FText = "";
	m_MemoDown = 0;
	m_MouseHold = 0;
    m_pCodeView = NULL;
	if( sys.m_MsgEng ){
		Font->Name = "Arial";
		Font->Charset = ANSI_CHARSET;

		Caption = "Set text and colors - "VER"  "TTL;
		CancelBtn->Caption = "Cancel";
		FontBtn->Caption = "Font";
		SBM->Caption = "Mac";
        SBL->Caption = "Code";
		GBG->Caption = "Gradation";
		GBS->Caption = "Shadow";
		GBA->Caption = "Misc";
		CBFixed->Caption = "Fixed size";
		CBZero->Caption = "Slash zero";
		SBLD->Caption = "Del";
		SBLW->Caption = "Add";
		GBY->Caption = "Style profiles";
		L1->Caption = "Align";
		L2->Caption = "Style name";
        L33->Caption = "V.space";
        L34->Caption = "Rot.";
		SBYD->Caption = "Del";
		SBYW->Caption = "Add";
		CBY->Text = "Untitled";
		SBM->Hint = "Insert macro-keyword";
        SBL->Hint = "Character code list";
		SBC->Hint = "Choose color's component";
		SBLD->Hint = "Delete the text in the list";
		SBLW->Hint = "Add current text into the list";
		SBYD->Hint = "Delete the style in the profiles";
		SBYW->Hint = "Add current style into the profiles";
		SB3DD->Hint = "Make the shadow shorter";
		SB3DI->Hint = "Make the shadow longer";
		SBA1->Hint = "Left";
		SBA2->Hint = "Center";
		SBA3->Hint = "Right";
		SBA4->Hint = "Auto";
//		FontBtn->Hint = "Choose font";
		SBFD->Hint = "Make the font smaller";
		SBFI->Hint = "Make the font bigger";
        SBB->Hint = "Bold";
        SBI->Hint = "Italic";
		SBMSG->Hint = "Show messages list";
		Msg->Hint = "Copy the message to the text box with double click";
	}
	else {
		Caption = "ƒeƒLƒXƒg‚ÆF‚ÌÝ’è - "VER"  "TTLJ;
		CBY->Text = "–³‘è";
	}
	SBFD->Glyph->Assign(SB3DD->Glyph);
	SBFI->Glyph->Assign(SB3DI->Glyph);
	MultProc();
	if( sys.m_pTextList == NULL ){
		sys.m_pTextList = new TStringList;
		LoadStringsFromIniFile(sys.m_pTextList, "Strings", ININAME, TRUE);
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
		LoadStringsFromIniFile(sys.m_pStyleList, "Styles", ININAME, FALSE);
		MultProc();
	}
	CBY->Items->Assign(sys.m_pStyleList);
	CBY->Sorted = TRUE;
	int i;
	if( sys.m_pFontList == NULL ){
		sys.m_pFontList = new TStringList;
		char    fname[512];
		sprintf(fname, "%s%s", sys.m_BgnDir, ININAME);
		TMemIniFile *pIniFile = new TMemIniFile(fname);
		for( i = 0; i < 5; i++ ){
		    char bf[32];
			sprintf(bf, "Font.%u", i);
			AnsiString as = pIniFile->ReadString(bf, "FontName", "");
			if( !as.IsEmpty() ){
				BYTE charset = (BYTE)pIniFile->ReadInteger(bf, "FontSet", 0);
				DWORD style = pIniFile->ReadInteger(bf, "FontStyle", 0);
                AddStyle(as, charset, style);
            }
			sys.m_pFontList->Add(as);
        }
        delete pIniFile;
    }
    for( i = 0; i < 5; i++ ) SetSBF(i);
	ShowHint = sys.m_WinNT;
	MultProc();
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::FormDestroy(TObject *Sender)
{
	if( m_pBitmap != NULL ) delete m_pBitmap;
	if( m_pCodeView ) delete m_pCodeView;
	MultProc();
	ASSERT(sys.m_pTextList);
	sys.m_pTextList->Assign(CBList->Items);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::TopMost(void)
{
#if UseHWND
	::SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
#endif
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::NoTopMost(void)
{
#if UseHWND
	::SetWindowPos(Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
#endif
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
	if( ShowHint ){
		char bf[32];
		sprintf(bf, "W = %d", UDL->Position);
		UDL->Hint = bf;
		sprintf(bf, "3D(Y) = %d", UDY->Position);
		UDY->Hint = bf;
		sprintf(bf, "3D(X) = %d", UDX->Position);
		UDX->Hint = bf;
	}
	Msg->Visible = SBMSG->Down;
	m_DisEvent--;
}
//---------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateMemoFont(BYTE charset)
{
	switch(charset){
		case SHIFTJIS_CHARSET:
		case HANGEUL_CHARSET:
		case CHINESEBIG5_CHARSET:   //
		case 134:       // ŠÈ—ª
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
	UpdateFontBtnHint();
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
	int r = ShowModal();
	if( Msg->Modified ) sys.m_Msg = Msg->Text;
	if( r == IDOK ){
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
	else if( SBA3->Down ){
		pItem->m_Align = 2;
	}
	else {
		pItem->m_Align = 3;
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
    pItem->m_OYW = UDOYW->Position & 0x0000ffff;
    pItem->m_Rot = UDRot->Position;
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
		case 3:
			SBA4->Down = TRUE;
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
		UpdateFontBtnHint();
	}
	Memo->Text = pItem->m_Text.c_str();

    UDOYW->Position = (SHORT)pItem->GetTrueOYW();
    UDRot->Position = (SHORT)pItem->m_Rot;
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
	if( m_MouseHold ){
		PBox->Canvas->Pen->Color = clBlue;
		PBox->Canvas->MoveTo(0, m_MouseYC);
		PBox->Canvas->LineTo(PBox->Width, m_MouseYC);
		PBox->Canvas->MoveTo(m_MouseXC, 0);
		PBox->Canvas->LineTo(m_MouseXC, PBox->Height);
	}
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
			HWND hWnd;
			if( SBMSG->Down ){
				Msg->SetFocus();
				hWnd = Msg->Handle;
			}
			else {
				Memo->SetFocus();
				hWnd = Memo->Handle;
			}
			for( LPCSTR p = bf; *p; p++ ){
				::PostMessage(hWnd, WM_CHAR, *p, 0);
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
        DWORD ex = m_Item.GetEx();
        int pos = m_Item.GetPos();
		CItem *pItem = new CItem;
		char bf[64];
		sprintf(bf, "Style.%u", r);
		pItem->LoadFromInifile(bf, ININAME);
		m_Item.Copy(pItem);
		delete pItem;
		m_Item.m_Text = as;
        m_Item.SetEx(ex);
        m_Item.SetPos(pos);
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
		CB3D->Checked = CB3D->Checked ? FALSE : TRUE;
	}
	else {
		CB3D->Checked = TRUE;
		m_MouseHold = TRUE;
		int xc = m_pBitmap->Width / 2;
		int yc = m_pBitmap->Height / 2;
		m_MouseXC = xc;
		m_MouseYC = yc;
		int x = X - xc;
		int y = Y - yc;
		x = x * 48 / xc;
		y = y * 48 / yc;
		double s = -UDRot->Position * PI / 180.0;
        double cosk = cos(s);
        double sink = sin(s);
        short xx = x * cosk + y * sink;
        short yy = -x * sink + y * cosk;
		UDX->Position = SHORT(xx);
		UDY->Position = SHORT(yy);
		Screen->Cursor = crSizeAll;
	}
	m_DisEvent--;
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if( m_MouseHold ){
		X = X - m_MouseXC;
		Y = Y - m_MouseYC;
		double s = -UDRot->Position * PI / 180.0;
        double cosk = cos(s);
        double sink = sin(s);
        short x = X * cosk + Y * sink;
        short y = -X * sink + Y * cosk;
		short UX = UDX->Position;
		short UY = UDY->Position;
		m_DisEvent++;
		UDX->Position = short(x);
		UDY->Position = short(y);
		m_DisEvent--;
		if( (UX != UDX->Position) || (UY != UDY->Position) ){
			UpdateBitmap();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::PBoxMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( m_MouseHold ){
		m_MouseHold = 0;
		PBox->Invalidate();
	}
	Screen->Cursor = crDefault;
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
	if( x == y ){
		int ad;
		if( x < 4 ){
			ad = 1;
		}
		else if( x < 12 ){
			ad = 2;
		}
		else {
			ad = 4;
		}
		ad *= a;
		x += ad;
		y += ad;
		if( x < 2 ) x = 2;
		if( y < 2 ) y = 2;
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
	UD3D((Sender == SB3DI) ? 1 : -1);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBMSGClick(TObject *Sender)
{
	Msg->Visible = SBMSG->Down;
	if( SBMSG->Down ){
		Msg->Text = sys.m_Msg;
		Msg->Modified = FALSE;
		Msg->SelLength = 0;
		Msg->SetFocus();
	}
	else {
		if( Msg->Modified ){
			sys.m_Msg = Msg->Text;
			Msg->Modified = FALSE;
		}
		Memo->SetFocus();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::MsgDblClick(TObject *Sender)
{
	AnsiString  as;
	AnsiString  cs;

	m_DisEvent++;
	if( Msg->Lines->Count ){
		int e = Msg->SelStart;
		LPCSTR p = Msg->Text.c_str();
		int n = 0;
		for( int i = 0; i < e; i++, p++ ){
			if( *p == '\n' ) n++;
		}
		if( n > Msg->Lines->Count ) n = Msg->Lines->Count - 1;
		as = Msg->Lines->Strings[n];
	}
	m_FText = as;
	::Yen2CrLf(cs, as);
	Memo->Text = cs;
	m_DisEvent--;
	SBMSG->Down = FALSE;
	SBMSGClick(NULL);
	MultProc();
	UpdateFText();
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::MemoStartDrag(TObject *Sender,
	  TDragObject *&DragObject)
{
	if( !SBMSG->Down ){
		SBMSG->Down = TRUE;
		SBMSGClick(NULL);
	}
}
//---------------------------------------------------------------------------

void __fastcall TTextDlgBox::MsgDragDrop(TObject *Sender, TObject *Source,
	  int X, int Y)
{
	Msg->Lines->Add(m_FText);
}
//---------------------------------------------------------------------------

void __fastcall TTextDlgBox::MsgDragOver(TObject *Sender, TObject *Source,
	  int X, int Y, TDragState State, bool &Accept)
{
	if( Source == Memo ) Accept = TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::MemoMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_MemoX = X;
	m_MemoY = Y;
	m_MemoDown = 1;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::MemoMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if( m_MemoDown == 1 ){
		X -= m_MemoX;
		Y -= m_MemoY;
		if( (X*X)+(Y*Y) > 64 ){
			m_MemoDown = 2;
			Memo->BeginDrag(TRUE);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::MemoMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_MemoDown = 0;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::FontBtnMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		BYTE cset;
		UINT cp = ::GetACP();
		switch(cp){
			case 932:
	        	cset = SHIFTJIS_CHARSET;
				break;
			case 949:
            	cset = HANGEUL_CHARSET;
				break;
			case 950:
            	cset = CHINESEBIG5_CHARSET;
				break;
			case 936:
            	cset = 134;       // ŠÈ—ª
				break;
			default:
				cset = DEFAULT_CHARSET;
				break;
		}
		int charset = m_Item.m_pFont->Charset;
		m_Item.m_pFont->Charset = BYTE((charset != ANSI_CHARSET) ? ANSI_CHARSET : cset);
		m_Item.UpdateFont();
		if( m_Item.m_pFont != NULL ){
			UpdateMemoFont(m_Item.m_pFont->Charset);
		}
		UpdateBitmap();
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::UpdateFontBtnHint(void)
{
	AnsiString as = sys.m_MsgEng ? "Choose font" : "ƒtƒHƒ“ƒg‚Ì‘I‘ð";
	if( m_Item.m_pFont ){
		as += " [";
		as += m_Item.m_pFont->Name;
		TFontStyles ts = m_Item.m_pFont->Style;
		DWORD d = FontStyle2Code(ts);
        AddStyle(as, m_Item.m_pFont->Charset, d);
        as += "]";
        SBB->Down = d & FSBOLD;
        SBI->Down = d & FSITALIC;
		if( m_pCodeView ) m_pCodeView->Execute(Handle, m_Item.m_pFont);
    }
    FontBtn->Hint = as;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SetSBF(int n)
{
	if( !sys.m_pFontList ) return;

	TSpeedButton* _tt[]={SBF1, SBF2, SBF3, SBF4, SBF5, NULL};

	TSpeedButton* pButton = _tt[n];
	AnsiString es = sys.m_MsgEng ? "Registration = Right button" : "“o˜^=‰EÎÞÀÝ";
	AnsiString fs = sys.m_pFontList->Strings[n];
    int undef = fs.IsEmpty();
    if( undef ) fs = sys.m_MsgEng ? "Load registered font" : "“o˜^Ì«ÝÄ‚Ì“Ç‚Ýo‚µ";
	char bf[256];
    sprintf(bf, "%s (%s)", fs.c_str(), es.c_str());
	pButton->Hint = bf;
	pButton->Font->Color = undef ? clGrayText : clBlack;
}
//---------------------------------------------------------------------------
int __fastcall TTextDlgBox::GetSBFIndex(TObject *Sender)
{
	TSpeedButton* _tt[]={SBF1, SBF2, SBF3, SBF4, SBF5, NULL};

    for( int i = 0; _tt[i]; i++ ){
		if( Sender == _tt[i] ) return i;
    }
    return 0;
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBFClick(TObject *Sender)
{
	if( !sys.m_pFontList ) return;
	int N = GetSBFIndex(Sender);
	if( sys.m_pFontList->Strings[N].IsEmpty() ) return;

	FontDialog->Font->Assign(m_Item.m_pFont);

	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, ININAME);
	TMemIniFile *pIniFile = new TMemIniFile(fname);
    {
	    char bf[32];
		sprintf(bf, "Font.%u", N);
		int ht = m_Item.m_pFont->Height;
		LoadFontFromInifile(m_Item.m_pFont, bf, pIniFile);
        int hs = m_Item.m_pFont->Height;
		m_Item.m_pFont->Height = ht;
        int oyw = (short)pIniFile->ReadInteger(bf, "OYW", UDOYW->Position);
		if( hs ) oyw = ht * oyw / hs;
        UDOYW->Position = (short)oyw;
    }
    delete pIniFile;

	m_Item.UpdateFont();
	if( m_Item.m_pFont != NULL ){
		UpdateMemoFont(m_Item.m_pFont->Charset);
	}
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBFMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		FontDialog->Font->Assign(m_Item.m_pFont);
		FontDialog->Font->Color = clBlack;
		NoTopMost();
		int r = FontDialog->Execute();
		TopMost();
		if( r == TRUE ){
			char    fname[512];
			sprintf(fname, "%s%s", sys.m_BgnDir, ININAME);
			TMemIniFile *pIniFile = new TMemIniFile(fname);
            {
				int N = GetSBFIndex(Sender);
			    char bf[32];
				sprintf(bf, "Font.%u", N);
				SaveFontToInifile(FontDialog->Font, bf, pIniFile);
				pIniFile->WriteInteger(bf, "OYW", UDOYW->Position);
				pIniFile->UpdateFile();
				if( sys.m_pFontList ){
					AnsiString as = FontDialog->Font->Name;
					TFontStyles ts = FontDialog->Font->Style;
					DWORD d = FontStyle2Code(ts);
                    AddStyle(as, FontDialog->Font->Charset, d);
					sys.m_pFontList->Strings[N] = as;
					SetSBF(N);
                }
            }
	        delete pIniFile;
		}
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBBClick(TObject *Sender)
{
	if( !m_Item.m_pFont ) return;

	TFontStyles ts = m_Item.m_pFont->Style;
	DWORD d = FontStyle2Code(ts);
	d ^= FSBOLD;
    ts = Code2FontStyle(d);
    m_Item.m_pFont->Style = ts;

	m_Item.UpdateFont();
	if( m_Item.m_pFont != NULL ){
		UpdateMemoFont(m_Item.m_pFont->Charset);
	}
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBIClick(TObject *Sender)
{
	if( !m_Item.m_pFont ) return;

	TFontStyles ts = m_Item.m_pFont->Style;
	DWORD d = FontStyle2Code(ts);
	d ^= FSITALIC;
    ts = Code2FontStyle(d);
    m_Item.m_pFont->Style = ts;

	m_Item.UpdateFont();
	if( m_Item.m_pFont != NULL ){
		UpdateMemoFont(m_Item.m_pFont->Charset);
	}
	UpdateBitmap();
}
//---------------------------------------------------------------------------

void __fastcall TTextDlgBox::SBFDMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( m_Item.m_pFont == NULL ) return;

	int ad = 0;
	int h = m_Item.m_pFont->Height;
	if( Sender == SBFD ){
		if( Button == mbLeft ){
			if( h < -2 ) ad = 1;
    	}
    	else if( Button == mbRight ){
			if( h < -6 ) ad = 4;
    	}
		UpdateBitmap();
	}
	else {
		if( Button == mbLeft ){
			ad = -1;
    	}
    	else if( Button == mbRight ){
			ad = -4;
    	}
	}
    if( ad ){
		m_Item.m_pFont->Height = h + ad;
		UpdateBitmap();
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::SBLClick(TObject *Sender)
{
	if( !sys.m_CodeLeft ) sys.m_CodeLeft = Left - 120;
    if( !sys.m_CodeTop ) sys.m_CodeTop = Top + 80;
	if( !m_pCodeView ){
		m_pCodeView = new TCodeView(this);
    }
    m_pCodeView->Execute((HWND)Handle, m_Item.m_pFont);
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::OnCodeViewClose(TMessage Message)
{
	if( Message.LParam == int(m_pCodeView) ){
		TCodeView *pView = m_pCodeView;
        m_pCodeView = NULL;
		delete pView;
    }
}
//---------------------------------------------------------------------------
void __fastcall TTextDlgBox::OnCodeInsert(TMessage Message)
{
	switch(Message.WParam){
		case 0xfffffff0:
		case 0xfffffff1:
		case 0xfffffff2:
		case 0xfffffff3:
		case 0xfffffff4:
		case 0xfffffff5:
		case 0xfffffff6:
		case 0xfffffff7:
			if( m_Item.m_pFont != NULL ){
				TFont *pFont = m_Item.m_pFont;
                int dfs = 0;
				switch(Message.WParam & 7){
					case 0:
			        	pFont->Name = "Webdings";
						pFont->Charset = 2;
                    	break;
					case 1:
			        	pFont->Name = "Wingdings";
						pFont->Charset = 2;
                    	break;
					case 2:
			        	pFont->Name = "Symbol";
						pFont->Charset = 2;
                    	break;
					case 3:
			        	pFont->Name = "Arial";
						pFont->Charset = ANSI_CHARSET;
                        dfs = FSBOLD;
                    	break;
					case 4:
			        	pFont->Name = sys.m_MsgEng ? "MS UI Gothic" : "‚l‚r ‚oƒSƒVƒbƒN";
						pFont->Charset = SHIFTJIS_CHARSET;
                        dfs = FSBOLD;
                    	break;
					case 5:
			        	pFont->Name = "DotumChe";
						pFont->Charset = HANGEUL_CHARSET;
                        if( !sys.m_tFontFam[fmHL] && sys.m_tFontFam[fmJOHAB] ){
							pFont->Charset = JOHAB_CHARSET;
                        }
                        dfs = FSBOLD;
                    	break;
					case 6:
			        	pFont->Name = "SimHei";
						pFont->Charset = 134;
                        dfs = FSBOLD;
                    	break;
					case 7:
			        	pFont->Name = "PMingLiU";
						pFont->Charset = CHINESEBIG5_CHARSET;
                        dfs = FSBOLD;
                    	break;
                }
			   	TFontStyles fs;
			   	fs = Code2FontStyle(dfs);
                pFont->Style = fs;
				m_Item.UpdateFont();
				UpdateMemoFont(m_Item.m_pFont->Charset);
				UpdateBitmap();
			}
        	break;
		case 0xfffffffe:
			::SetWindowPos(m_pCodeView->Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
//			m_pCodeView->Visible = FALSE;
			FontBtnClick(NULL);
			::SetWindowPos(m_pCodeView->Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
//			m_pCodeView->Visible = TRUE;
        	break;
        case 0xffffffff:
			Memo->Clear();
    	    UpdateBitmap();
        	break;
        default:
			if( Message.WParam ){
				HWND hWnd;
				Memo->SetFocus();
				hWnd = Memo->Handle;
				char bf[16];
				if( Message.WParam & 0x00ff0000 ){
					wsprintf(bf, "\\$%04x", Message.WParam & 0x00ffff);
                }
                else if( Message.WParam & 0x00ff00 ){
					bf[0] = BYTE(Message.WParam >> 8);
   	                bf[1] = BYTE(Message.WParam);
       	            bf[2] = 0;
                }
				else if( (Message.WParam == 0x0d) || (Message.WParam == 0x0a) ){
					bf[0] = 0x0d; bf[1] = 0;
                }
				else if( (Message.WParam != '%') && ((Message.WParam >= 0x20) && (Message.WParam <= 0x7f)) ){
					bf[0] = (char)Message.WParam; bf[1] = 0;
				}
        		else {
	    		    wsprintf(bf, "\\x%02x", Message.WParam);
        		}
				for( LPCSTR p = bf; *p; p++ ){
					::PostMessage(hWnd, WM_CHAR, *p & 0x00ff, 0);
				}
            }
        	break;
    }
}
//---------------------------------------------------------------------------

