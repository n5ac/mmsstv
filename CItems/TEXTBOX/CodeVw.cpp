//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "CodeVw.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//TCodeView *CodeView;
//---------------------------------------------------------------------------
__fastcall TCodeView::TCodeView(TComponent* Owner)
	: TForm(Owner)
{
	if( sys.m_MsgEng ){
		Font->Name = "Arial";
		Font->Charset = ANSI_CHARSET;

//		Caption = "Character code list";
        SBClose->Caption = "Close";
        SBOut->Caption = "EUDC";
        SBEdit->Caption = "Edit";
        SBClear->Caption = "Clear";

        SBFont->Hint = "Choose font";
	    SBClear->Font->Size = 10;
        SBOut->Font->Size = 10;
        SBEdit->Font->Size = 10;
	}
    SBA->Hint = "Webdings";
    SBB->Hint = "Wingdings";
    SBC->Hint = "Symbol";
    SBUS->Hint = "English(ANSI)";
    SBJA->Hint = sys.m_MsgEng ? "Japanese(Shift-JIS)" : "日本語(シフトJIS)";
    SBHL->Hint = sys.m_MsgEng ? "Korean(Hangul)" : "韓国語(ハングル)";
    SBBY->Hint = sys.m_MsgEng ? "Chinese(GB2312)" : "中国語(簡体)";
    SBBV->Hint = sys.m_MsgEng ? "Chinese(BIG5)" : "中国語(繁体)";
    SBOut->Hint = "EUDC(End-user-defined-characters)";
	m_hWnd = NULL;
    m_pFont = NULL;
	m_Base = 0;
    m_MouseDown = 0;
	CheckFontCharset();
	SBJA->Enabled = sys.m_tFontFam[fmJA];
	SBHL->Enabled = sys.m_tFontFam[fmHL] || sys.m_tFontFam[fmJOHAB];
	SBBV->Enabled = sys.m_tFontFam[fmBV];
	SBBY->Enabled = sys.m_tFontFam[fmBY];

	UpdateXW();
    int YW = GetSystemMetrics(SM_CYFULLSCREEN);
    Left = sys.m_CodeLeft;
    Top = sys.m_CodeTop;
    if( Left < 0 ) Left = 0;
    if( Top + Height > YW ) Top = YW - Height;
	SBEdit->Enabled = sys.m_fEudc;
    ShowHint = sys.m_WinNT;
}
//---------------------------------------------------------------------------
WORD __fastcall TCodeView::GetEUDC(int y, int x)
{
const WORD _tOut[]={0xf040, 0xf050, 0xf060, 0xf070,
					0xf080, 0xf090, 0xf0a0, 0xf0b0,
                    0xf0c0, 0xf0d0, 0xf0e0, 0xf0f0,
                    0xf140, 0xf150, 0xf160, 0xf170,
};
const WORD _tOutHL[]={0xc9a0, 0xc9b0, 0xc9c0, 0xc9d0,
					0xc9e0, 0xc9f0, 0xfea0, 0xfeb0,
                    0xfec0, 0xfed0, 0xfee0, 0xfef0,
                    0, 0, 0, 0,
};
const WORD _tOutBV[]={0xfa40, 0xfa50, 0xfa60, 0xfa70,
					0xfaa0, 0xfab0, 0xfac0, 0xfad0,
                    0xfae0, 0xfaf0, 0xfb40, 0xfb50,
                    0xfb60, 0xfb70, 0xfba0, 0xfbb0,
};
const WORD _tOutBY[]={0xaaa0, 0xaab0, 0xaac0, 0xaad0,
					0xaae0, 0xaaf0, 0xaba0, 0xabb0,
                    0xabc0, 0xabd0, 0xabe0, 0xabf0,
                    0xaca0, 0xacb0, 0xacc0, 0xacd0,
};
	int c;
	switch(m_pFont->Charset){
		case SHIFTJIS_CHARSET:
			c = _tOut[y] + x;
			if( (c==0xf07f)||(c==0xf0fd)||(c==0xf0fe)||(c==0xf0ff)||(c==0xf17f) ) c = 0;
            break;
		case HANGEUL_CHARSET:
			c = _tOutHL[y];
            if( c ){
	            c += x;
    	        if( (c==0xc9a0)||(c==0xfea0)||(c==0xc9ff)||(c==0xfeff) ) c = 0;
            }
            break;
		case CHINESEBIG5_CHARSET:   // 台湾
			c = _tOutBV[y] + x;
            if( (c==0xfa7f)||(c==0xfaa0)||(c==0xfaff)||(c==0xfb7f)||(c==0xfba0) ) c = 0;
            break;
		case 134:                   // 中国語簡略
			c = _tOutBY[y] + x;
			if( (c==0xaaa0)||(c==0xaaff)||(c==0xaba0)||(c==0xabff)||(c==0xaca0) ) c = 0;
            break;
        default:
        	c = 0;
            break;
    }
    return WORD(c);
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::UpdateTitle(void)
{
	AnsiString as = sys.m_MsgEng ? "Character code list" : "文字コード表";
    as += " [";
	as += m_pFont->Name;
    AddStyle(as, m_pFont->Charset, 0);
    as += "]";
    Caption = as;
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::UpdateXW(void)
{
	if( SBOut->Down || m_Base ){
		m_XW = PBox->Width / 17;
        m_XW--;
    	m_YW1 = m_YW = PBox->Height / 17;
        m_XW1 = PBox->Width - (m_XW * 16);
	    SBEdit->Visible = SBOut->Down;
        UDMB->Visible = m_Base ? TRUE : FALSE;
    }
    else {
		m_XW1 = m_XW = PBox->Width / 17;
    	m_YW1 = m_YW = PBox->Height / 17;
	    SBEdit->Visible = FALSE;
        UDMB->Visible = FALSE;
    }
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::SetPBoxFont(TCanvas *pCanvas)
{
	pCanvas->Font = m_pFont;
   	pCanvas->Font->Height = -(m_YW-5);
   	TFontStyles fs;
   	fs = Code2FontStyle(0);
   	pCanvas->Font->Style = fs;
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::DrawChar(TCanvas *pCanvas, int c, BOOL f)
{

	int x = c % 16;
    int y = c / 16;
	char bf[32];
	LPSTR t = bf;
	if( SBOut->Down ){
		c = GetEUDC(y, x);
    }
    else if( c ){
		c += m_Base;
    }
	if( !c ) return;
	if( c & 0x00ff00 ) *t++ = BYTE(c>>8);
    *t++ = BYTE(c);
    *t = 0;
    pCanvas->Font->Color = f ? clWhite : clBlack;
	int fw = pCanvas->TextWidth(bf);
	int fh = pCanvas->TextHeight(bf);
	int xx = m_XW1 + x * m_XW + (m_XW-fw)/2;
	int yy = m_YW1 + y * m_YW + (m_YW-fh)/2;
	::SetBkMode(pCanvas->Handle, TRANSPARENT);
	pCanvas->TextOut(xx, yy, bf);
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::DrawCursor(TCanvas *pCanvas, int c, BOOL f)
{
	TRect rc;

	int x = c % 16;
    int y = c / 16;
    rc.Left = 1 + m_XW1 + x * m_XW;
	rc.Top = 1 + m_YW1 + y * m_YW;
    rc.Right = rc.Left + m_XW - 1;
    rc.Bottom = rc.Top + m_YW - 1;
	TColor col;
	if( f ){
		col = clBlue;
    }
    else if( (!SBOut->Down && !m_Base) && m_tPfx[c] ){
		col = clGray;
    }
    else {
		col = clWhite;
    }
    pCanvas->Pen->Color = col;
    pCanvas->Brush->Color = col;
    pCanvas->FillRect(rc);
    DrawChar(pCanvas, c, f);
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::PBoxPaint(TObject *Sender)
{
	if( !m_pFont ) return;

	CWaitCursor w;
	Graphics::TBitmap *pBmp = new Graphics::TBitmap;
    pBmp->Width = PBox->Width;
    pBmp->Height = PBox->Height;
	TCanvas *pCanvas = pBmp->Canvas;
	BOOL eud = SBOut->Down;
	if( !m_Base && !eud ) memset(m_tPfx, 0, sizeof(m_tPfx));
	SetPBoxFont(pCanvas);
	if( !eud ) SetMBCP(m_pFont->Charset);
	int x, y, xx, yy, fw, fh;
    char bf[16];
	int c = 0;
	for( y = 0; y < 16; y++ ){
		for( x = 0; x < 16; x++ ){
			if( eud ){
				DrawChar(pCanvas, c, FALSE);
            }
            else {
				if( !m_Base && (_mbsbtype((const unsigned char *)&c, 0) == _MBC_LEAD) ){
					m_tPfx[c] = TRUE;
    	        	DrawCursor(pCanvas, c, FALSE);
        	    }
            	else {
					DrawChar(pCanvas, c, FALSE);
            	}
            }
           	c++;
        }
    }
	pCanvas->Font = Font;
    pCanvas->Pen->Color = clBlack;
	::SetBkMode(pCanvas->Handle, TRANSPARENT);
    for( y = 0; y < 16; y++ ){
		wsprintf(bf, "%X", y);
		fw = pCanvas->TextWidth(bf);
        fh = pCanvas->TextHeight(bf);
        xx = m_XW1 + y*m_XW + (m_XW-fw)/2;
        yy = (m_YW1-fh)/2;
        pCanvas->TextOut(xx, yy, bf);

		if( eud ){
			c = GetEUDC(y, 1) & 0xfff0;
            if( c ){
		        wsprintf(bf, "%04x", c);
            }
            else {
				bf[0] = 0;
            }
        }
		else if( m_Base ){
	        wsprintf(bf, "%04x", y*16 + m_Base);
        }
        else {
	        wsprintf(bf, "%02X", y*16);
        }
		fw = pCanvas->TextWidth(bf);
        fh = pCanvas->TextHeight(bf);
        xx = (m_XW1-fw)/2;
        yy = m_YW1 + y*m_YW + (m_YW-fh)/2;
        pCanvas->TextOut(xx, yy, bf);

		pCanvas->Pen->Width = y ? 1 : 2;
		pCanvas->MoveTo(0, m_YW1 + y*m_YW);
        pCanvas->LineTo(PBox->Width, m_YW1 + y*m_YW);
		pCanvas->MoveTo(m_XW1 + y*m_XW, 0);
        pCanvas->LineTo(m_XW1 + y*m_XW, PBox->Height);
    }
    PBox->Canvas->Draw(0, 0, pBmp);
    delete pBmp;
	m_pfxCount = 0;
    for( x = 0; x < 256; x++ ){
		if( m_tPfx[x] ){
			m_tPfxIdx[m_pfxCount] = x;
        	m_pfxCount++;
        }
    }
	if( m_pfxCount != UDMB->Max ) UDMB->Max = short(m_pfxCount - 1);
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::ReqClose(void)
{
	sys.m_CodeLeft = Left;
    sys.m_CodeTop = Top;
    if( SBOut->Visible ) sys.m_CodeOut = SBOut->Down;
	::PostMessage(m_hWnd, WM_FORMCLOSE, 0, DWORD(this));
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::FormClose(TObject *Sender, TCloseAction &Action)
{
	if( Action == caHide ) ReqClose();
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::Execute(HWND hWnd, TFont *pFont)
{
	int f = Visible;
	if( f && SBOut->Visible ) sys.m_CodeOut = SBOut->Down;
	m_hWnd = hWnd;
    m_pFont = pFont;
    m_Base = 0;
	SBOut->Visible = GetEUDC(0, 1) ? TRUE : FALSE;
	SBOut->Down = SBOut->Visible && sys.m_CodeOut;
	UpdateXW();
    UpdateTitle();
	Visible = TRUE;
    if( f ) PBoxPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::SBCloseClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::PBoxMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( (Button == mbRight) && (m_Base || SBOut->Down) ){
		if( m_Base ){
			m_Base = 0;
        }
		else {
			SBOut->Down = FALSE;
        }
        UpdateXW(); PBoxPaint(NULL);
        return;
    }
	if( Button != mbLeft ) return;

    X -= m_XW1;
    Y -= m_YW1;
    if( X < 0 ) return;
    if( Y < 0 ) return;
    X /= m_XW;
    Y /= m_YW;
	if( X >= 16 ) X = 15;
    if( Y >= 16 ) Y = 15;
	int code = (Y * 16) + X;
    m_MouseDown = code + 1;
	TCanvas *pCanvas = PBox->Canvas;
	SetPBoxFont(pCanvas);
    DrawCursor(pCanvas, code, TRUE);
	if( SBOut->Down ){
		code = GetEUDC(Y, X);
        if( !code ) return;
    }
	else {
		if( m_Base ){
			code |= m_Base | 0x00010000;
//			PBoxPaint(NULL);
        }
		else if( m_tPfx[code] ){
			for( int i = 0; i < m_pfxCount; i++ ){
				if( m_tPfxIdx[i] == code ){
					UDMB->Position = short(i);
					break;
                }
            }
			m_Base = (code << 8);
            UpdateXW(); PBoxPaint(NULL);
            m_MouseDown = FALSE;
            return;
        }
    }
	::PostMessage(m_hWnd, WM_CODEINSERT, code, DWORD(this));
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::PBoxMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( !m_MouseDown ) return;

	m_MouseDown--;
	TCanvas *pCanvas = PBox->Canvas;
	SetPBoxFont(pCanvas);
    DrawCursor(pCanvas, m_MouseDown, FALSE);
    m_MouseDown = 0;
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::SBClearClick(TObject *Sender)
{
	::PostMessage(m_hWnd, WM_CODEINSERT, 0xffffffff, DWORD(this));
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::SBFontClick(TObject *Sender)
{
	::PostMessage(m_hWnd, WM_CODEINSERT, 0xfffffffe, DWORD(this));
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::SBOutClick(TObject *Sender)
{
    m_Base = 0;
	UpdateXW();
	PBoxPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::SBEditClick(TObject *Sender)
{
	LPCSTR pName = "EUDCEDIT.EXE";
	if( sys.m_Eudc.IsEmpty() ){
		char dir[MAX_PATH];
		char bf[512];
		if( ::GetSystemDirectory(dir, sizeof(dir)) ){
	        wsprintf(bf, "%s\\%s", dir, pName);
            if( IsFile(bf) ) sys.m_Eudc = bf;
		}
		if( sys.m_Eudc.IsEmpty() && ::GetWindowsDirectory(dir, sizeof(dir)) ){
	        wsprintf(bf, "%s\\%s", dir, pName);
            if( IsFile(bf) ) sys.m_Eudc = bf;
        }
		if( sys.m_Eudc.IsEmpty() ){
	        wsprintf(bf, "%c:\\Program Files\\Accessories\\%s", dir[0], pName);
            if( IsFile(bf) ) sys.m_Eudc = bf;
        }
		if( sys.m_Eudc.IsEmpty() ) sys.m_Eudc = pName;
    }
	if( ::WinExec(sys.m_Eudc.c_str(), SW_SHOWDEFAULT) > 31 ){
	    ReqClose();
    }
    else {
		SBEdit->Enabled = FALSE;
        sys.m_fEudc = FALSE;
    }
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::SBAClick(TObject *Sender)
{
	DWORD dw = 0xfffffff2;
	if( Sender == SBA ){
		dw = 0xfffffff0;
    }
    else if( Sender == SBB ){
		dw = 0xfffffff1;
    }
	::PostMessage(m_hWnd, WM_CODEINSERT, dw, DWORD(this));
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::SBUSClick(TObject *Sender)
{
	DWORD sw;
    if( Sender == SBJA ){
		sw = 0xfffffff4;
    }
    else if( Sender == SBHL ){
		sw = 0xfffffff5;
    }
    else if( Sender == SBBY ){
		sw = 0xfffffff6;
    }
    else if( Sender == SBBV ){
		sw = 0xfffffff7;
    }
    else {
		sw = 0xfffffff3;
    }
	::PostMessage(m_hWnd, WM_CODEINSERT, sw, DWORD(this));
}
//---------------------------------------------------------------------------
void __fastcall TCodeView::UDMBClick(TObject *Sender, TUDBtnType Button)
{
	m_Base = m_tPfxIdx[UDMB->Position] << 8;
    UpdateXW(); PBoxPaint(NULL);
}
//---------------------------------------------------------------------------
int CALLBACK EnumFontFamExProc(ENUMLOGFONT FAR* lpelf, NEWTEXTMETRIC FAR* lpntm, DWORD FontType, LPARAM lParam)
{	switch(lpelf->elfLogFont.lfCharSet){		case SHIFTJIS_CHARSET:			sys.m_tFontFam[fmJA] = TRUE;        	break;		case HANGEUL_CHARSET:			sys.m_tFontFam[fmHL] = TRUE;            break;		case JOHAB_CHARSET:			sys.m_tFontFam[fmJOHAB] = TRUE;        	break;		case CHINESEBIG5_CHARSET:   // 台湾        	sys.m_tFontFam[fmBV] = TRUE;            break;		case 134:                   // 中国語簡略			sys.m_tFontFam[fmBY] = TRUE;            break;        default:        	break;    }	int r = FALSE;    for( int i = 0; i < fmEND; i++ ){		if( !sys.m_tFontFam[i] ) r = TRUE;    }	return r;}//---------------------------------------------------------------------------
void __fastcall TCodeView::CheckFontCharset(void)
{
	if( sys.m_fFontFam ) return;

	sys.m_fFontFam = TRUE;
	memset(sys.m_tFontFam, 0, sizeof(sys.m_tFontFam));
    LOGFONT logfont;
    memset(&logfont, 0, sizeof(logfont));
    logfont.lfCharSet = DEFAULT_CHARSET;
	HDC hDC = ::CreateDC("DISPLAY", NULL, NULL, NULL);
    ::EnumFontFamiliesEx(hDC, &logfont, (int (_stdcall *)())EnumFontFamExProc, LPARAM(this), 0);
    ::DeleteDC(hDC);
}
//---------------------------------------------------------------------------

