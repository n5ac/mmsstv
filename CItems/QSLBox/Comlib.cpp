//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "ComLib.h"
#include "TextDlg.h"
SYS	sys;
//
//---------------------------------------------------------------------------
//void mcmFunc(DWORD index, FARPROC pFunc)
//
//MMSSTV calls this function at the first time, if the function was exported.
//You can use following functions which are in the MMSSTV.
//
//  index   : Index of the function.
//  pFunc   : Address of the function.
//
//indexes and functions are as follows.
//
//      0 : HBITMAP (__cdecl *mmLoadImageMenu)(int sw, int xw, int yw);
//              sw - 0 : Invoke image-clipper (xw and yw are the size of the bitmap)
//              sw - 1 : no image-clipper (xw and yw will be ignored)
//
//      1 : HBITMAP (__cdecl *mmLoadImage)(LPCSTR pName);
//              pName is a pathname of the image (BMP, JPG, WMF)
//
//      2 : HBITMAP (__cdecl *mmLoadHistImage)(void);
//
//      3 : int (__cdecl *mmSaveImageMenu)(HBITMAP hb, LPCSTR pName, LPCSTR pFolder);
//              pName is a filename of the image (BMP, JPG)
//              pFolder is a folder name (if it is NULL, MMSSTV uses a default)
//
//      4 : int (__cdecl *mmSaveImage)(HBITMAP hb, LPCSTR pName);
//              pName is a pathname of the image (BMP, JPG)
//
//      5 : HBITMAP (__cdecl *mmPasteImage)(int sw, int xw, int yw);
//              sw - 0 : Invoke image-clipper (xw and yw are the size of the bitmap)
//              sw - 1 : no image-clipper (xw and yw will be ignored)
//
//      6 : int (__cdecl *mmGetMacKey)(LPSTR pDest, int cbDest);
//              Invoke macro dialog box and can choose keyword
//
//      7 : int (__cdecl *mmConvMac)(LPSTR pDest, LPCSTR pSrc, int cbDest);
//              Convert macro keyword (pSrc) to the text (pDest)
//
//      8 : HBITMAP (__cdecl *mmCreateDIB)(int xw, int yw);
//              Create 24bit DIB
//
//      9 : void (__cdecl *mmMultProc)(void);
//      10 : void (__cdecl *mmMultProcA)(void);
//              Do decoding and encoding with the DSP in MMSSTV
//
//      11 : int (__cdecl *mmColorComp)(LPDWORD pCol);
//              Invoke color components dialog and can choose a color component
//
extern "C" __declspec(dllexport)
void mcmFunc(DWORD index, FARPROC pFunc)
{
	switch(index){
		case 6:
			sys.m_fGetMacKey = (mmGetMacKey)pFunc;
			break;
		case 7:
			sys.m_fConvMac = (mmConvMac)pFunc;
			break;
		case 9:
			sys.m_fMultProc = (mmMultProc)pFunc;
			break;
		case 11:
			sys.m_fColorComp = (mmColorComp)pFunc;
			break;
		default:
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall MultProc(void)
{
	if( sys.m_fMultProc ) sys.m_fMultProc();
}
//---------------------------------------------------------------------------
char __fastcall LastC(LPCSTR p)
{
	char c = 0;

	if( *p ){
		c = *(p + strlen(p) - 1);
	}
	return c;
}
//---------------------------------------------------------------------------
void __fastcall SetCurDir(LPSTR t, int size)
{
	if( !::GetCurrentDirectory(size-1, t) ){
		*t = 0;
	}
	else {
		if( LastC(t) != '\\' ){
			strcat(t, "\\");
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall SetDirName(LPSTR t, LPCSTR pName)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char name[_MAX_FNAME];
	char ext[_MAX_EXT];
	AnsiString	Dir;

	::_splitpath( pName, drive, dir, name, ext );
	Dir = drive;
	Dir += dir;
	strncpy(t, Dir.c_str(), 128);
}
///----------------------------------------------------------------
void __fastcall Yen2CrLf(AnsiString &ws, AnsiString cs)
{
	ws = "";
	LPCSTR	p;
	int		f;
	int		dlm = 0;

	p = cs.c_str();
	if( *p == 0x22 ){
		p++;
		dlm++;
	}
	for( f = 0; *p; p++ ){
		if( f ){
			f = 0;
			ws += *p;
		}
		else if(  _mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD ){
			f = 1;
			ws += *p;
		}
		else if( *p == '\\' ){
			switch(*(p+1)){
			case 'r':
				ws += "\r";
				p++;
				break;
			case 'n':
				ws += "\n";
				p++;
				break;
			case 't':
				ws += "\t";
				p++;
				break;
			case '\\':
				ws += "\\";
				p++;
				break;
			default:
				p++;
				ws += *p;
				break;
			}
		}
		else if( !dlm || (*p!=0x22) || *(p+1) ){
			ws += *p;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall CrLf2Yen(AnsiString &ws, AnsiString cs)
{
	ws = "\x22";
	LPCSTR	p;
	int f = 0;

	for( p = cs.c_str(); *p; p++ ){
		if( f ){
			ws += *p;
			f = 0;
		}
		else if(  _mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD ){
			f = 1;
			ws += *p;
		}
		else if( *p == 0x0d ){
			ws += "\\r";
		}
		else if( *p == 0x0a ){
			ws += "\\n";
		}
		else if( *p == '\t' ){
			ws += "\\t";
		}
		else if( *p == '\\' ){
			ws += "\\\\";
		}
		else {
			ws += *p;
		}
	}
	ws += "\x22";
}
//---------------------------------------------------------------------------
void __fastcall LoadStringsFromIniFile(TStrings *pList, LPCSTR pSect, LPCSTR pName)
{
	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, pName);
	TIniFile *pIniFile = new TIniFile(fname);
	int count = pIniFile->ReadInteger(pSect, "Count", 0);
	int i;
	AnsiString as;
	for( i = 0; i < count; i++ ){
		char bf[32];
		sprintf(bf, "Text%u", i);
		as = pIniFile->ReadString(pSect, bf, "");
		if( !as.IsEmpty() ){
			pList->Add(as.c_str());
		}
	}
	delete pIniFile;
}
//---------------------------------------------------------------------------
void __fastcall SaveStringsToIniFile(TStrings *pList, LPCSTR pSect, LPCSTR pName)
{
	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, pName);
	TIniFile *pIniFile = new TIniFile(fname);
	pIniFile->WriteInteger(pSect, "Count", pList->Count);
	int i;
	for( i = 0; i < pList->Count; i++ ){
		char bf[32];
		sprintf(bf, "Text%u", i);
		pIniFile->WriteString(pSect, bf, pList->Strings[i]);
	}
	delete pIniFile;
}
//---------------------------------------------------------------------
void __fastcall InitCustomColor(TColorDialog *tp)
{
	tp->CustomColors->Text = "";
}
//---------------------------------------------------------------------
void __fastcall AddCustomColor(TColorDialog *tp, TColor col)
{
	char bf[256];

	sprintf(bf, "Color%c=%06lX", tp->CustomColors->Count + 'A', DWORD(col) & 0x00ffffff);
	tp->CustomColors->Add(bf);
}
//---------------------------------------------------------------------
LPSTR __fastcall ZeroConv(LPCSTR p, BYTE charset)
{
	LPSTR bp = new char[(strlen(p)*2) + 1];
	LPSTR t = bp;
	int f = 0;
	for( ; *p; p++ ){
		if( f ){
			*t++ = *p;
			f = 0;
		}
		else if(  _mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD ){
			*t++ = *p;
			f = 1;
		}
		else if( *p == '0' ){
			switch(charset){
				case SHIFTJIS_CHARSET:
					*t++ = 0x83;
					*t++ = 0xd3;
					break;
				case HANGEUL_CHARSET:
					*t++ = 0xa8;    // 0xa9
					*t++ = 0xaa;
					break;
				case CHINESEBIG5_CHARSET:   // 台湾
					*t++ = 0xa3;
					*t++ = 0x58;
					break;
				case 134:                   // 中国語簡略
					*t++ = 0xa6;
					*t++ = 0xd5;
					break;
				default:
					switch(sys.m_CodePage){
						case 949:	// HANGEUL
							*t++ = 0xa8;    // 0xa9
							*t++ = 0xaa;
							break;
						case 950:	// CHINESEBIG5
							*t++ = 0xa3;
							*t++ = 0x58;
							break;
						case 936:	// CHINESE 簡略
							*t++ = 0xa6;
							*t++ = 0xd5;
							break;
						default:
							*t++ = 'ﾘ';
							break;
					}
					break;
			}
		}
		else {
			*t++ = *p;
		}
	}
	*t = 0;
	return bp;
}
//---------------------------------------------------------------------------
Graphics::TBitmap * __fastcall CreateBitmap(int xw, int yw)
{
	Graphics::TBitmap *pBitmap = new Graphics::TBitmap;
	pBitmap->PixelFormat = pf24bit;
	pBitmap->Width = xw;
	pBitmap->Height = yw;
	return pBitmap;
}
//---------------------------------------------------------------------------
void __fastcall FillBitmap(Graphics::TBitmap *pDest, UCOL ucol)
{
	TRect rc;
	rc.Left = rc.Top = 0;
	rc.Right = pDest->Width;
	rc.Bottom = pDest->Height;
	pDest->Canvas->Brush->Color = ucol.c;
	pDest->Canvas->FillRect(rc);
}
//---------------------------------------------------------------------------
void __fastcall CopyBitmap(Graphics::TBitmap *pDest, Graphics::TBitmap *pSrc)
{
	pDest->Width = pSrc->Width;
	pDest->Height = pSrc->Height;
	pDest->Palette = NULL;
	pDest->Canvas->Draw(0, 0, pSrc);
}
//---------------------------------------------------------------------------
//アスペクト比を保持する
void __fastcall KeepAspect(int &sx, int &sy, int bx, int by)
{
	double ass = double(sx) / double(sy);
	double asb = double(bx) / double(by);
	if( asb < ass ){    // ソースの方が横長
		sx = (double(sy) * asb) + 0.5;
	}
	else {
		sy = (double(sx) / asb) + 0.5;
	}
}
//---------------------------------------------------------------------------
//アスペクト比を保持する
void __fastcall KeepAspectCopy(Graphics::TBitmap *pDest, int xm, int ym, Graphics::TBitmap *pSrc, int mode)
{
	if( (xm < pSrc->Width) || (ym < pSrc->Height) ){
		KeepAspect(xm, ym, pSrc->Width, pSrc->Height);
		pDest->Width = xm;
		pDest->Height = ym;
		pDest->Palette = NULL;
		TRect tc;
		tc.Left = tc.Top = 0;
		tc.Right = xm; tc.Bottom = ym;
		int Sop = ::SetStretchBltMode(pDest->Canvas->Handle, mode);
		pDest->Canvas->StretchDraw(tc, pSrc);
		::SetStretchBltMode(pDest->Canvas->Handle, Sop);
	}
	else {
		CopyBitmap(pDest, pSrc);
	}
}
//---------------------------------------------------------------------------
void __fastcall DrawBitmap(TPaintBox *pBox, Graphics::TBitmap *pBitmap)
{
	if( pBitmap == NULL ) return;

	if( (pBitmap->Width != pBox->Width)||(pBitmap->Height != pBox->Height) ){
		TRect rc;
		rc.Left = 0; rc.Right = pBox->Width;
		rc.Top = 0, rc.Bottom = pBox->Height;
		int Sop = ::SetStretchBltMode(pBox->Canvas->Handle, HALFTONE);
		pBox->Canvas->StretchDraw(rc, pBitmap);
		::SetStretchBltMode(pBox->Canvas->Handle, Sop);
	}
	else {
		pBox->Canvas->Draw(0, 0, pBitmap);
	}
}
//---------------------------------------------------------------------------
UCOL __fastcall GetUniqueColor(HDC hdc, UCOL ucol)
{
	ucol.d = ucol.d & 0x00e0e0e0;
	UCOL c = ucol;

	while(c.d == ucol.d){
#if (RANDMAX >= 0x00ffffff)
		c.d = rand() & 0x00e0e0e0;
#else
		c.b.r = (BYTE)(rand() & 0x00e0);
		c.b.g = (BYTE)(rand() & 0x00e0);
		c.b.b = (BYTE)(rand() & 0x00e0);
#endif
	}
	c.d = GetNearestColor(hdc, (COLORREF)c.d);
	return c;
}
//---------------------------------------------------------------------------
UCOL __fastcall GetGrade2(UCOL s[2], int x, int xw)
{
	if( x < 0 ) x = 0;
	if( x > xw ) x = xw;
	UCOL    r;
	int c = s[0].b.b + ((int(s[1].b.b) - int(s[0].b.b)) * x / xw);
	if( c < 0 ) c = 0;
	if( c > 255 ) c = 255;
	r.b.b = BYTE(c);
	c = s[0].b.g + ((int(s[1].b.g) - int(s[0].b.g)) * x / xw);
	if( c < 0 ) c = 0;
	if( c > 255 ) c = 255;
	r.b.g = BYTE(c);
	c = s[0].b.r + ((int(s[1].b.r) - int(s[0].b.r)) * x / xw);
	if( c < 0 ) c = 0;
	if( c > 255 ) c = 255;
	r.b.r = BYTE(c);
	return r;
}
//---------------------------------------------------------------------------
UCOL __fastcall GetGrade4(UCOL s[4], int x, int xw)
{
	int xw3 = xw/3;
	if( x < xw3 ){
		return GetGrade2(s, x, xw3);
	}
	x -= xw3;
	if( x < xw3 ){
		return GetGrade2(&s[1], x, xw3);
	}
	x -= xw3;
	return GetGrade2(&s[2], x, xw3);
}
//---------------------------------------------------------------------------
void __fastcall WriteString(TStream *ps, AnsiString &as)
{
	int len = as.Length();
	ps->Write(&len, sizeof(len));
	if( len ){
		ps->Write(as.c_str(), len);
	}
}
void __fastcall ReadString(TStream *ps, AnsiString &as)
{
	as = "";
	int len;
	ps->Read(&len, sizeof(len));
	if( len ){
		LPSTR p = new char[len+1];
		ps->Read(p, len);
		p[len] = 0;
		as = p;
		delete p;
	}
}
//---------------------------------------------------------------------------
int FontStyle2Code(TFontStyles style)
{
	int code = 0;

	TFontStyles fa;
	TFontStyles fb;

	fa << fsBold;
	fb << fsBold;
	fa *= style;
	if( fa == fb ) code |= FSBOLD;

	fa >> fsBold;
	fb >> fsBold;
	fa << fsItalic;
	fb << fsItalic;
	fa *= style;
	if( fa == fb ) code |= FSITALIC;

	fa >> fsItalic;
	fb >> fsItalic;
	fa << fsUnderline;
	fb << fsUnderline;
	fa *= style;
	if( fa == fb ) code |= FSUNDERLINE;

	fa >> fsUnderline;
	fb >> fsUnderline;
	fa << fsStrikeOut;
	fb << fsStrikeOut;
	fa *= style;
	if( fa == fb ) code |= FSSTRIKEOUT;

	return code;
}

TFontStyles Code2FontStyle(int code)
{
	TFontStyles fs;

	if( code & FSBOLD ) fs << fsBold;
	if( code & FSITALIC ) fs << fsItalic;
	if( code & FSUNDERLINE ) fs << fsUnderline;
	if( code & FSSTRIKEOUT ) fs << fsStrikeOut;
	return fs;
}
//---------------------------------------------------------------------------
void __fastcall WriteFont(TStream *ps, TFont *pFont)
{
	int d;
	if( pFont != NULL ){
		d = 1;
		ps->Write(&d, sizeof(d));
		WriteString(ps, pFont->Name);
		d = pFont->Charset;
		ps->Write(&d, sizeof(d));
		d = pFont->Height;
		ps->Write(&d, sizeof(d));
		TFontStyles ts = pFont->Style;
		d = FontStyle2Code(ts);
		ps->Write(&d, sizeof(d));
	}
	else {
		d = 0;
		ps->Write(&d, sizeof(d));
	}
}
//---------------------------------------------------------------------------
TFont* __fastcall ReadFont(TStream *ps)
{
	TFont *pFont = NULL;
	int d;
	ps->Read(&d, sizeof(d));
	if( d == 1 ){
		pFont = new TFont;
		AnsiString as;
		ReadString(ps, as);
		pFont->Name = as;
		ps->Read(&d, sizeof(d));
		pFont->Charset = BYTE(d);
		ps->Read(&d, sizeof(d));
		pFont->Height = d;
		ps->Read(&d, sizeof(d));
		TFontStyles ts = Code2FontStyle(d);
		pFont->Style = ts;
	}
	return pFont;
}
//---------------------------------------------------------------------------
int __fastcall IsInside(POINT &pnt, RECT &rc)
{
	if( rc.left < rc.right ){
		if( pnt.x < rc.left ) return FALSE;
		if( pnt.x > rc.right ) return FALSE;
	}
	else {
		if( pnt.x > rc.left ) return FALSE;
		if( pnt.x < rc.right ) return FALSE;
	}
	if( rc.top > rc.bottom ){
		if( pnt.y > rc.top ) return FALSE;
		if( pnt.y < rc.bottom ) return FALSE;
	}
	else {
		if( pnt.y < rc.top ) return FALSE;
		if( pnt.y > rc.bottom ) return FALSE;
	}
	return TRUE;
}

int __fastcall IsNear(POINT &pnt, POINT &tpnt, int Width)
{
	RECT	rc;
	rc.left = tpnt.x - Width;
	rc.right = tpnt.x + Width;
	rc.top = tpnt.y + Width;
	rc.bottom = tpnt.y - Width;
	return IsInside(pnt, rc);
}

int __fastcall IsLine(POINT &pnt, POINT &spnt, POINT &tpnt, int Width)
{
	int		Xw = tpnt.x - spnt.x;
	int		Yw = tpnt.y - spnt.y;
	double	A, B;
	int		C;

	if( IsNear(pnt, spnt, Width)==TRUE ){
		return HT_P1;		/* 始点と一致	*/
	}
	else if( IsNear(pnt, tpnt, Width)==TRUE ){
		return HT_P2;		/* 終点と一致	*/
	}
	if( spnt.x > tpnt.x ){		/* X方向の入れ替え	*/
		POINT	bpnt = spnt;
		spnt = tpnt;
		tpnt = bpnt;
	}
	if( (pnt.x >= (spnt.x-Width)) && (pnt.x <= (tpnt.x+Width)) ){
		if( Xw ){
			A = double(Yw)/double(Xw);
			B = double(spnt.y) - (A * double(spnt.x));
			C = int((A * double(pnt.x)) + B);
			A = A < 0.0 ? -A : A;
			if( A >= 1.0 ){
				Width = int(double(Width) * A);
			}
			if( (pnt.y < (C+Width)) && (pnt.y >= (C-Width)) ){
				return HT_PM;
			}
		}
		else if( spnt.y > tpnt.y ){
			if( (pnt.y < spnt.y) && (pnt.y > tpnt.y) ){
				return HT_PM;
			}
		}
		else if( (pnt.y > spnt.y) && (pnt.y < tpnt.y) ){
			return HT_PM;
		}
	}
	return 0;
}

int __fastcall IsLine(POINT &pnt, POINT &spnt, POINT &tpnt)
{
	int r = IsLine(pnt, spnt, tpnt, 4);
	if( (r == HT_P1)||(r == HT_P2) ) return r;
	return IsLine(pnt, spnt, tpnt, 10) ? HT_PM : 0;
}
//***************************************************************************
//DrawGroupクラス
//---------------------------------------------------------------------------
__fastcall CDraw::CDraw()
{
	m_Ver = 0;
	m_Cursor = 0;
	m_CX1 = m_CY1 = 0;
	m_CX2 = m_CY2 = 0;
	m_LineColor.c = clBlack;
	m_LineStyle = psSolid;
	m_LineWidth = 1;
	m_X1 = m_Y1 = m_X2 = m_Y2 = 0;
	m_BX = m_BY = m_MF = 0;
	m_pCanvas = NULL;
}

void __fastcall CDraw::Start(TCanvas *pCanvas, int x, int y)
{
	m_pCanvas = pCanvas;
	m_CX1 = m_X1 = x;
	m_CY1 = m_Y1 = y;
}

void __fastcall CDraw::StartMove(TCanvas *pCanvas, int x, int y, int code)
{
	m_pCanvas = pCanvas;
	m_BX = x;
	m_BY = y;
	m_MF = 0;
	m_MC = code;
	m_CX1 = m_X1;
	m_CX2 = m_X2;
	m_CY1 = m_Y1;
	m_CY2 = m_Y2;
}

int __fastcall CDraw::Edit(void)
{
#if 0
	TLineSetDlg *pBox = new TLineSetDlg(Mmsstv);
	int r = pBox->Execute(this);
	delete pBox;
	return r;
#endif
}

int __fastcall CDraw::Color(void)
{
#if 0
	InitCustomColor(Mmsstv->ColorDialog);
	AddCustomColor(Mmsstv->ColorDialog, m_LineColor);
	Mmsstv->ColorDialog->Color = m_LineColor;
	DisPaint++;
	if( Mmsstv->ColorDialog->Execute() == TRUE ){
		m_LineColor = Mmsstv->ColorDialog->Color;
		DisPaint--;
		return TRUE;
	}
	DisPaint--;
	return FALSE;
#endif
}

void __fastcall CDraw::SaveToStream(TStream *sp)
{
	sp->Write(&m_Command, sizeof(m_Command));
	sp->Write(&m_Ver, sizeof(m_Ver));
	sp->Write(&m_X1, sizeof(m_X1));
	sp->Write(&m_Y1, sizeof(m_Y1));
	sp->Write(&m_X2, sizeof(m_X2));
	sp->Write(&m_Y2, sizeof(m_Y2));
	sp->Write(&m_LineColor, sizeof(m_LineColor));
	sp->Write(&m_LineStyle, sizeof(m_LineStyle));
	sp->Write(&m_LineWidth, sizeof(m_LineWidth));
}

int __fastcall CDraw::LoadFromStream(TStream *sp)
{
	try {
		pc = tp;
		sp->Read(&m_Ver, sizeof(m_Ver));
		sp->Read(&m_X1, sizeof(m_X1));
		sp->Read(&m_Y1, sizeof(m_Y1));
		sp->Read(&m_X2, sizeof(m_X2));
		sp->Read(&m_Y2, sizeof(m_Y2));
		sp->Read(&m_LineColor, sizeof(m_LineColor));
		sp->Read(&m_LineStyle, sizeof(m_LineStyle));
		sp->Read(&m_LineWidth, sizeof(m_LineWidth));
		return TRUE;
	}
	catch(...){
		return FALSE;
	}
}

void __fastcall CDraw::Copy(CDraw *dp)
{
}

void __fastcall CDraw::Normal(void)
{
	NormalRect(m_X1, m_Y1, m_X2, m_Y2);
}

//---------------------------------------------------------------------------
__fastcall CDrawLine::CDrawLine()
{
	m_Command = CM_LINE;
	m_Cursor = 0;
}

int __fastcall CDrawLine::Finish(int x, int y)
{
	m_X2 = x;
	m_Y2 = y;
	return ((m_X1 != m_X2)||(m_Y1 != m_Y2)) ? TRUE : FALSE;
}

void __fastcall CDrawLine::Draw(TCanvas *pCanvas)
{
	m_pCanvas = pCanvas;
	pCanvas->Pen->Color = m_LineColor;
	pCanvas->Pen->Style = m_LineStyle;
	pCanvas->Pen->Width = m_LineWidth;
	pCanvas->MoveTo(m_X1, m_Y1);
	pCanvas->LineTo(m_X2, m_Y2);
}

void __fastcall CDrawLine::DrawSel(TCanvas *pCanvas, int sw)
{
	m_pCanvas = pCanvas;
	pCanvas->Pen->Color = clBlue;
	pCanvas->Pen->Style = psDot;
	pCanvas->Pen->Width = 1;
	pCanvas->MoveTo(m_X1, m_Y1);
	int rop = ::SetROP2(pCanvas->Handle, sw ? R2_NOT : R2_COPYPEN);
	pCanvas->LineTo(m_X2, m_Y2);
	::SetROP2(pCanvas->Handle, rop);
}

void __fastcall CDrawLine::DrawCursor(void)
{
	ASSERT(m_pCanvas);
	m_pCanvas->Pen->Style = psDot;
	m_pCanvas->Pen->Width = 1;
	m_pCanvas->MoveTo(m_CX1, m_CY1);
	int Sop = ::SetROP2(m_pCanvas->Handle, R2_NOT);
	m_pCanvas->LineTo(m_CX2, m_CY2);
	::SetROP2(m_pCanvas->Handle, Sop);
}

void __fastcall CDrawLine::Making(int x, int y)
{
	if( m_Cursor ) DrawCursor();
	m_CX2 = x;
	m_CY2 = y;
	DrawCursor();
	m_Cursor = 1;
}

int __fastcall CDrawLine::HitTest(int x, int y)
{
	POINT   N;
	POINT   B, E;
	N.x = x; N.y = y;
	B.x = m_X1; B.y = m_Y1;
	E.x = m_X2; E.y = m_Y2;
	return IsLine(N, B, E, 5) ? HT_I : HT_NONE;
}

void __fastcall CDrawLine::Moving(int x, int y)
{
	if( m_Cursor ) DrawCursor();
	if( !m_MF ){
		if( m_BX != x ) m_MF = 1;
		if( m_BY != y ) m_MF = 1;
	}
	x -= m_BX;
	y -= m_BY;
	m_CX1 = m_X1 + x;
	m_CX2 = m_X2 + x;
	m_CY1 = m_Y1 + y;
	m_CY2 = m_Y2 + y;
	DrawCursor();
	m_Cursor = 1;
}

void __fastcall CDrawLine::Move(int x, int y)
{
	m_X1 = m_CX1;
	m_Y1 = m_CY1;
	m_X2 = m_CX2;
	m_Y2 = m_CY2;
}

//---------------------------------------------------------------------------
__fastcall CDrawBox::CDrawBox()
{
	m_Command = CM_BOX;
}

int __fastcall CDrawBox::Finish(int x, int y)
{
	m_X2 = x;
	m_Y2 = y;
	Normal();
	return ((m_X1 != m_X2)&&(m_Y1 != m_Y2)) ? TRUE : FALSE;
}

void __fastcall CDrawBox::Draw(TCanvas *pCanvas)
{
	if( m_LineStyle < 0 ) return;

	pCanvas->Pen->Color = m_LineColor;
	pCanvas->Pen->Style = m_LineStyle;
	pCanvas->Pen->Width = m_LineWidth;
	pCanvas->MoveTo(m_X1, m_Y1);
	pCanvas->LineTo(m_X2, m_Y1);
	pCanvas->LineTo(m_X2, m_Y2);
	pCanvas->LineTo(m_X1, m_Y2);
	pCanvas->LineTo(m_X1, m_Y1);
}

void __fastcall CDrawBox::DrawSel(TCanvas *pCanvas, int sw)
{
	pCanvas->Brush->Color = clWhite;
	pCanvas->Pen->Color = clBlue;
	pCanvas->Pen->Width = 1;
	pCanvas->Pen->Style = psDot;
	pCanvas->MoveTo(m_X1, m_Y1);
	int rop = ::SetROP2(pCanvas->Handle, sw ? R2_NOT : R2_COPYPEN);
	pCanvas->LineTo(m_X2, m_Y1);
	pCanvas->LineTo(m_X2, m_Y2);
	pCanvas->LineTo(m_X1, m_Y2);
	pCanvas->LineTo(m_X1, m_Y1);
	::SetROP2(pCanvas->Handle, rop);
}

void __fastcall CDrawBox::DrawCursor(void)
{
	ASSERT(m_pCanvas);
	m_pCanvas->Pen->Style = psDot;
	m_pCanvas->Pen->Width = 1;
	m_pCanvas->MoveTo(m_CX1, m_CY1);
	int Sop = ::SetROP2(m_pCanvas->Handle, R2_NOT);
	m_pCanvas->LineTo(m_CX2, m_CY1);
	m_pCanvas->LineTo(m_CX2, m_CY2);
	m_pCanvas->LineTo(m_CX1, m_CY2);
	m_pCanvas->LineTo(m_CX1, m_CY1);
	::SetROP2(m_pCanvas->Handle, Sop);
}

void __fastcall CDrawBox::Making(int x, int y)
{
	if( m_Cursor ) DrawCursor();
	m_CX2 = x;
	m_CY2 = y;
	DrawCursor();
	m_Cursor = 1;
}

int __fastcall CDrawBox::HitTest(int x, int y)
{
	POINT   N;
	POINT   B, E;
	N.x = x; N.y = y;
	B.x = m_X1; B.y = m_Y1;
	E.x = m_X2; E.y = m_Y1;
	int r = IsLine(N, B, E, 5);
	if( r ){
		switch(r){
			case HT_P1:
				return HT_LT;
			case HT_P2:
				return HT_RT;
			default:
				return HT_T;
		}
	}
	B.x = m_X2; B.y = m_Y1;
	E.x = m_X2; E.y = m_Y2;
	r = IsLine(N, B, E, 5);
	if( r ){
		switch(r){
			case HT_P1:
				return HT_RT;
			case HT_P2:
				return HT_RB;
			default:
				return HT_R;
		}
	}
	B.x = m_X2; B.y = m_Y2;
	E.x = m_X1; E.y = m_Y2;
	r = IsLine(N, B, E, 5);
	if( r ){
		switch(r){
			case HT_P1:
				return HT_RB;
			case HT_P2:
				return HT_LB;
			default:
				return HT_B;
		}
	}
	B.x = m_X1; B.y = m_Y2;
	E.x = m_X1; E.y = m_Y1;
	r = IsLine(N, B, E, 5);
	if( r ){
		switch(r){
			case HT_P1:
				return HT_LB;
			case HT_P2:
				return HT_LT;
			default:
				return HT_L;
		}
	}
	if( (x < m_X1) || (x > m_X2) ) return FALSE;
	if( (y < m_Y1) || (y > m_Y2) ) return FALSE;
	return HT_I;
}

void __fastcall CDrawBox::Moving(int x, int y)
{
	if( m_Cursor ) DrawCursor();
	if( !m_MF ){
		if( m_BX != x ) m_MF = 1;
		if( m_BY != y ) m_MF = 1;
	}
	x -= m_BX;
	y -= m_BY;
	switch(m_MC){
		case HT_LT:
			m_CX1 = m_X1 + x;
			m_CY1 = m_Y1 + y;
			break;
		case HT_T:
			m_CY1 = m_Y1 + y;
			break;
		case HT_RT:
			m_CX2 = m_X2 + x;
			m_CY1 = m_Y1 + y;
			break;
		case HT_R:
			m_CX2 = m_X2 + x;
			break;
		case HT_RB:
			m_CX2 = m_X2 + x;
			m_CY2 = m_Y2 + y;
			break;
		case HT_B:
			m_CY2 = m_Y2 + y;
			break;
		case HT_LB:
			m_CX1 = m_X1 + x;
			m_CY2 = m_Y2 + y;
			break;
		case HT_L:
			m_CX1 = m_X1 + x;
			break;
		default:
			m_CX1 = m_X1 + x;
			m_CX2 = m_X2 + x;
			m_CY1 = m_Y1 + y;
			m_CY2 = m_Y2 + y;
			break;
	}
	DrawCursor();
	m_Cursor = 1;
}

void __fastcall CDrawBox::Move(int x, int y)
{
	m_X1 = m_CX1;
	m_Y1 = m_CY1;
	m_X2 = m_CX2;
	m_Y2 = m_CY2;
	Normal();
	AdjustRect();
}
//
//---------------------------------------------------------------------------
__fastcall CDrawText::CDrawText()
{
	m_Command = CM_TEXT;

}


//***************************************************************************
//DrawGroupクラス
//---------------------------------------------------------------------------
__fastcall CDrawGroup::CDrawGroup()
{
	m_Command = CM_GROUP;
	m_Cnt = 0;
	m_Max = 0;
	m_TransX = 319;
	m_TransY = 255;
	m_TransCol = TColor(0x00f8f8f8);
	m_SX = 320;
	m_SY = 256;
	m_Ver = 2;
	m_pBase = NULL;
	m_pSel = NULL;
}

__fastcall CDrawGroup::~CDrawGroup()
{
	FreeItem();
	Free();
}

void __fastcall CDrawGroup::Free(void)
{
	if( m_pBase != NULL ){
		delete m_pBase;
		m_pBase = NULL;
	}
	m_Cnt = m_Max = 0;
}

void __fastcall CDrawGroup::FreeItem(void)
{
	if( m_pBase == NULL ) return;

	for( int i = 0; i < m_Cnt; i++ ){
		delete m_pBase[i];
	}
	delete pBase;
	m_pBase = NULL;
	m_Cnt = m_Max = 0;
	m_pSel = NULL;
	m_TransX = 319;
	m_TransY = 255;
	m_TransCol = TColor(0x00f8f8f8);
}

void __fastcall CDrawGroup::AddItem(CDraw *dp)
{
	if( m_Cnt >= m_Max ){
		m_Max = m_Max ? m_Max * 2 : 4;
		CDraw **pNewBase = new CDraw*[m_Max];
		if( m_pBase != NULL ){
			memcpy(pNewBase, m_pBase, sizeof(CDraw*)*m_Cnt);
			delete m_pBase;
		}
		m_pBase = pNewBase;
	}
	m_pBase[m_Cnt] = dp;
	m_Cnt++;
}

void __fastcall CDrawGroup::DeleteItem(CDraw *dp)
{
	if( m_pBase == NULL ) return;

	for( int i = 0; i < m_Cnt; i++ ){
		if( m_pBase[i] == dp ){
			delete m_pBase[i];
			for( ; i < (m_Cnt - 1); i++ ){
				m_pBase[i] = m_pBase[i+1];
			}
			m_pBase[i] = NULL;
			m_Cnt--;
			return;
		}
	}
}

void __fastcall CDrawGroup::ReleaseItem(CDraw *dp)
{
	if( m_pBase == NULL ) return;

	for( int i = 0; i < m_Cnt; i++ ){
		if( m_pBase[i] == dp ){
			for( ; i < (m_Cnt - 1); i++ ){
				m_pBase[i] = m_pBase[i+1];
			}
			m_pBase[i] = NULL;
			m_Cnt--;
			return;
		}
	}
}

CDraw * __fastcall CDrawGroup::AddItemCopy(CDraw *dp)
{
	CDraw *pItem = MakeItem(dp->m_Command);
	pItem->Copy(dp);
	AddItem(pItem);
	return pItem;
}

void __fastcall CDrawGroup::Draw(TCanvas *pCanvas)
{
	if( m_pBase == NULL ) return;

	TransCol = m_TransCol;
	for( int i = 0; i < m_Cnt; i++ ){
		pBase[i]->Draw(pCanvas);
	}
}

CDraw* __fastcall CDrawGroup::SelectItem(int X, int Y)
{
	if( m_pBase == NULL ) return NULL;

	for( int i = m_Cnt - 1; i >= 0; i-- ){
		if( (m_Hit = m_pBase[i]->HitTest(X, Y)) != 0 ){
			return m_pBase[i];
		}
	}
	return NULL;
}

CDraw* __fastcall CDrawGroup::MakeItem(int cmd)
{
	CDraw *pItem = NULL;
	switch(cmd){
		case CM_LINE:
			pItem = new CDrawLine;
			break;
		case CM_BOX:
			pItem = new CDrawBox;
			break;
		case CM_TEXT:
			pItem = new CDrawText;
			break;
		case CM_GROUP:
			pItem = new CDrawGroup;
			break;
	}
	return pItem;
}

void __fastcall CDrawGroup::SaveToStream(TStream *sp)
{
	m_Ver = 0;
	CDrawBox::SaveToStream(sp);

	sp->Write(&m_TransX, sizeof(m_TransX));
	sp->Write(&m_TransY, sizeof(m_TransY));
	sp->Write(&m_TransCol, sizeof(m_TransCol));
	sp->Write(&m_SX, sizeof(m_SX));
	sp->Write(&m_SY, sizeof(m_SY));
	sp->Write(&m_Cnt, sizeof(m_Cnt));
	for( int i = 0; i < m_Cnt; i++ ){
		m_pBase[i]->SaveToStream(sp);
	}
}

int __fastcall CDrawGroup::LoadFromStream(TStream *sp)
{
	FreeItem();
	MultProc();
	try {
		int cnt, cmd;
		sp->Read(&cmd, sizeof(cmd));
		if( cmd != CM_GROUP ){
			goto _err;
		}

		if( CDrawBox::LoadFromStream(tp, sp) == FALSE ) goto _err;
		sp->Read(&m_TransX, sizeof(m_TransX));
		sp->Read(&m_TransY, sizeof(m_TransY));
		sp->Read(&m_TransCol, sizeof(m_TransCol));
		sp->Read(&m_SX, sizeof(m_SX));
		sp->Read(&m_SY, sizeof(m_SY));
		m_pSel = NULL;
		sp->Read(&cnt, sizeof(cnt));
		for( int i = 0; i < cnt; i++ ){
			MultProcA();
			sp->Read(&cmd, sizeof(cmd));
			CDraw *pItem = MakeItem(cmd);
			if( pItem != NULL ){
				if( pItem->LoadFromStream(sp) == FALSE ){
					delete pItem;
					goto _err;
				}
				AddItem(pItem);
			}
			else {
				goto _err;
			}
		}
		if( !m_Cnt ) m_TransCol = TColor(0x00f8f8f8);
		return TRUE;
	}
	catch(...){
	}
_err:;
	FreeItem();
	return FALSE;
}

void __fastcall CDrawGroup::BackItem(CDraw *pItem)
{
	int i, k;
	for( i = m_Cnt - 1; i >= 0; i-- ){
		if( m_pBase[i] == pItem ){
			for( k = i; k > 0; k-- ){
				m_pBase[k] = m_pBase[k-1];
			}
			m_pBase[0] = pItem;
			break;
		}
	}
}

void __fastcall CDrawGroup::FrontItem(CDraw *pItem)
{
	int i, k;
	for( i = 0; i < m_Cnt; i++ ){
		if( m_pBase[i] == pItem ){
			for( k = i; k < m_Cnt - 1; k++ ){
				m_pBase[k] = m_pBase[k+1];
			}
			m_pBase[k] = pItem;
			break;
		}
	}
}

int __fastcall CDrawGroup::UpItem(int n)
{
	if( n ){
		CDraw *pItem = pBase[n-1];
		pBase[n-1] = pBase[n];
		pBase[n] = pItem;
		return TRUE;
	}
	return FALSE;
}

int __fastcall CDrawGroup::DownItem(int n)
{
	if( n < (m_Cnt - 1) ){
		CDraw *pItem = pBase[n+1];
		pBase[n+1] = pBase[n];
		pBase[n] = pItem;
		return TRUE;
	}
	return FALSE;
}
//--------------------------------------------------------------------------
void __fastcall CDrawGroup::AdjustTransPoint(void)
{
	MultProc();
	int i;
	int f = 0;
	int XW = 320;
	int YW = 256;
	for( i = 0; i < m_Cnt; i++ ){
		if( m_pBase[i]->HitTest(m_TransX, m_TransY) ){
			for( m_TransX = XW-1; m_TransX >= 0; m_TransX -= 5 ){
				for( m_TransY = YW-1; m_TransY >= 0; m_TransY-- ){
					f = 0;
					for( i = 0; i < m_Cnt; i++ ){
						if( m_pBase[i]->HitTest(m_TransX, m_TransY) ){
							f++;
							break;
						}
					}
					if( !f ) return;
				}
				MultProc();
			}
			break;
		}
	}
	if( f ){
		m_TransX = XW-1;
		m_TransY = YW-1;
	}
}
//--------------------------------------------------------------------------
CDraw* __fastcall CDrawGroup::GetLastItem(void)
{
	return m_Cnt ? pBase[m_Cnt-1]: NULL;
}
//
//
//---------------------------------------------------------------------------
// CItem class
//
__fastcall CItem::CItem(void)
{
	m_pMemStream = NULL;

	m_Ver = 0;
	m_ASize = 1;
	m_Align = 0;
	m_Grade = 0;
	m_Shadow = 2;
	m_Line = 1;
	m_Zero = 0;
	m_3d = 0x00000404;
	m_Text = "TX\tFT1000MP\r\nANT\tMagnetic loop\r\nSOFT\tHome-made";
	m_pFont = NULL;
	m_ColG[0].d = 255;
	m_ColG[1].d = 16711935;
	m_ColG[2].d = 65535;
	m_ColG[3].d = 16776960;
	m_ColB[0].c = clBlack;
	m_ColB[1].c = clWhite;
	m_ColB[2].c = clBlue;
	m_ColB[3].c = clRed;

	m_OrgXW = -1;
	m_OrgYW = -1;
	m_TextXW = -1;
	m_TextYW = -1;
	m_Draft = 0;
	m_IniSize = 0;
}
//---------------------------------------------------------------------------
__fastcall CItem::~CItem()
{
	if( m_pFont != NULL ) delete m_pFont;
	if( m_pMemStream != NULL ) delete m_pMemStream;
}
//---------------------------------------------------------------------------
void __fastcall CItem::LoadDefault(void)
{
	if( sys.m_pDefStg != NULL ) return;

	CItem *pItem = new CItem;
	pItem->LoadFromInifile("Default", ININAME);
	LPCBYTE p = pItem->CreateStorage(&sys.m_DefStgSize);
	sys.m_pDefStg = new BYTE[sys.m_DefStgSize];
	memcpy(sys.m_pDefStg, p, sys.m_DefStgSize);
	pItem->DeleteStorage(p);
	delete pItem;
}
//---------------------------------------------------------------------------
void __fastcall CItem::UpdateDefault(void)
{
	if( sys.m_pDefStg != NULL ){
		delete sys.m_pDefStg;
		sys.m_pDefStg = NULL;
	}
	LPCBYTE p = CreateStorage(&sys.m_DefStgSize);
	sys.m_pDefStg = new BYTE[sys.m_DefStgSize];
	memcpy(sys.m_pDefStg, p, sys.m_DefStgSize);
	DeleteStorage(p);
}
//---------------------------------------------------------------------------
void __fastcall CItem::Create(LPCBYTE ps, DWORD size)
{
	if( (ps == NULL) || !size ){
		if( sys.m_pDefStg == NULL ) LoadDefault();
		ps = sys.m_pDefStg;
		size = sys.m_DefStgSize;
	}

	TMemoryStream *pMem = new TMemoryStream;
	pMem->Clear();
	pMem->Write(ps, size);
	pMem->Seek(soFromBeginning, 0);
	int ver;
	pMem->Read(&ver, sizeof(ver));
	if( ver <= m_Ver ){
		pMem->Read(&m_TextXW, sizeof(m_TextXW));
		pMem->Read(&m_TextYW, sizeof(m_TextYW));
		pMem->Read(&m_OrgXW, sizeof(m_OrgXW));
		pMem->Read(&m_OrgYW, sizeof(m_OrgYW));
		pMem->Read(&m_ASize, sizeof(m_ASize));
		pMem->Read(&m_Align, sizeof(m_Align));
		pMem->Read(&m_Grade, sizeof(m_Grade));
		pMem->Read(&m_Shadow, sizeof(m_Shadow));
		pMem->Read(&m_Line, sizeof(m_Line));
		pMem->Read(&m_Zero, sizeof(m_Zero));
		pMem->Read(&m_3d, sizeof(m_3d));
		pMem->Read(&m_ColG, sizeof(m_ColG));
		pMem->Read(&m_ColB, sizeof(m_ColB));
		ReadString(pMem, m_Text);
		if( m_pFont != NULL ) delete m_pFont;
		m_pFont = ReadFont(pMem);
		UpdateFont();
	}
	delete pMem;
}
//---------------------------------------------------------------------------
LPCBYTE __fastcall CItem::CreateStorage(LPDWORD pSize)
{
	ASSERT(pSize);

	if( m_pMemStream != NULL ) delete m_pMemStream;
	m_pMemStream = new TMemoryStream;
	m_pMemStream->Clear();
	m_Ver = 0;
	m_pMemStream->Write(&m_Ver, sizeof(m_Ver));
	m_pMemStream->Write(&m_TextXW, sizeof(m_TextXW));
	m_pMemStream->Write(&m_TextYW, sizeof(m_TextYW));
	m_pMemStream->Write(&m_OrgXW, sizeof(m_OrgXW));
	m_pMemStream->Write(&m_OrgYW, sizeof(m_OrgYW));
	m_pMemStream->Write(&m_ASize, sizeof(m_ASize));
	m_pMemStream->Write(&m_Align, sizeof(m_Align));
	m_pMemStream->Write(&m_Grade, sizeof(m_Grade));
	m_pMemStream->Write(&m_Shadow, sizeof(m_Shadow));
	m_pMemStream->Write(&m_Line, sizeof(m_Line));
	m_pMemStream->Write(&m_Zero, sizeof(m_Zero));
	m_pMemStream->Write(&m_3d, sizeof(m_3d));
	m_pMemStream->Write(&m_ColG, sizeof(m_ColG));
	m_pMemStream->Write(&m_ColB, sizeof(m_ColB));
	WriteString(m_pMemStream, m_Text);
	WriteFont(m_pMemStream, m_pFont);
	*pSize = m_pMemStream->Position;
	return (LPCBYTE)m_pMemStream->Memory;
}
//---------------------------------------------------------------------------
void __fastcall CItem::LoadFromInifile(LPCSTR pSect, LPCSTR pName)
{
	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, pName);
	TIniFile *pIniFile = new TIniFile(fname);
	MultProc();
	m_ASize = pIniFile->ReadInteger(pSect, "AutoSize", m_ASize);
	m_Align = pIniFile->ReadInteger(pSect, "Align", m_Align);
	m_Grade = pIniFile->ReadInteger(pSect, "Grade", m_Grade);
	m_Shadow = pIniFile->ReadInteger(pSect, "Shadow", m_Shadow);
	m_Line = pIniFile->ReadInteger(pSect, "Line", m_Line);
	m_Zero = pIniFile->ReadInteger(pSect, "Zero", 0);
	m_3d = pIniFile->ReadInteger(pSect, "3D", m_3d);
	int i;
	for( i = 0; i < 4; i++ ){
		char bf[32];
		sprintf(bf, "ColG%u", i);
		m_ColG[i].d = pIniFile->ReadInteger(pSect, bf, m_ColG[i].d);
		sprintf(bf, "ColB%u", i);
		m_ColB[i].d = pIniFile->ReadInteger(pSect, bf, m_ColB[i].d);
	}
	if( m_ASize ){
		m_OrgXW = m_OrgYW = -1;
		m_TextXW = m_TextYW = -1;
	}
	else {
		m_TextXW = pIniFile->ReadInteger(pSect, "Width", m_TextXW);
		m_TextYW = pIniFile->ReadInteger(pSect, "Height", m_TextYW);
		m_IniSize = 1;
	}
	AnsiString as, cs;
	::CrLf2Yen(cs, m_Text);
	as = pIniFile->ReadString(pSect, "Text", cs);
	::Yen2CrLf(m_Text, as);

	if( pIniFile->ReadInteger(pSect, "Font", 0) ){
		if( m_pFont != NULL ) delete m_pFont;
		m_pFont = new TFont;
		m_pFont->Name = pIniFile->ReadString(pSect, "FontName", m_pFont->Name);
		m_pFont->Charset = (BYTE)pIniFile->ReadInteger(pSect, "FontSet", m_pFont->Charset);
		m_pFont->Height = pIniFile->ReadInteger(pSect, "FontSize", m_pFont->Height);
		TFontStyles ts = m_pFont->Style;
		DWORD d = FontStyle2Code(ts);
		d = pIniFile->ReadInteger(pSect, "FontStyle", d);
		m_fsCode = d;
		ts = Code2FontStyle(d);
		m_pFont->Style = ts;
	}
	delete pIniFile;
}
//---------------------------------------------------------------------------
void __fastcall CItem::SaveToInifile(LPCSTR pSect, LPCSTR pName)
{
	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, pName);

	TIniFile *pIniFile = new TIniFile(fname);
	MultProc();
	try {
		pIniFile->WriteInteger(pSect, "AutoSize", m_ASize);
		pIniFile->WriteInteger(pSect, "Align", m_Align);
		pIniFile->WriteInteger(pSect, "Grade", m_Grade);
		pIniFile->WriteInteger(pSect, "Shadow", m_Shadow);
		pIniFile->WriteInteger(pSect, "Line", m_Line);
		pIniFile->WriteInteger(pSect, "Zero", m_Zero);
		pIniFile->WriteInteger(pSect, "3D", m_3d);
		int i;
		for( i = 0; i < 4; i++ ){
			char bf[32];
			sprintf(bf, "ColG%u", i);
			pIniFile->WriteInteger(pSect, bf, m_ColG[i].d);
			sprintf(bf, "ColB%u", i);
			pIniFile->WriteInteger(pSect, bf, m_ColB[i].d);
		}
		pIniFile->WriteInteger(pSect, "Width", m_TextXW);
		pIniFile->WriteInteger(pSect, "Height", m_TextYW);
		AnsiString cs;
		::CrLf2Yen(cs, m_Text);
		pIniFile->WriteString(pSect, "Text", cs);

		pIniFile->WriteInteger(pSect, "Font", m_pFont != NULL ? 1 : 0);
		if( m_pFont != NULL ){
			pIniFile->WriteString(pSect, "FontName", m_pFont->Name);
			pIniFile->WriteInteger(pSect, "FontSet", m_pFont->Charset);
			pIniFile->WriteInteger(pSect, "FontSize", m_pFont->Height);
			TFontStyles ts = m_pFont->Style;
			DWORD d = FontStyle2Code(ts);
			pIniFile->WriteInteger(pSect, "FontStyle", d);
		}
	}
	catch(...){
	}
	delete pIniFile;
}
//---------------------------------------------------------------------------
void __fastcall CItem::DeleteInifile(LPCSTR pSect, LPCSTR pName)
{
	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, pName);

	TIniFile *pIniFile = new TIniFile(fname);
	MultProc();
	try {
		pIniFile->EraseSection(pSect);
	}
	catch(...){
	}
	delete pIniFile;
}
//---------------------------------------------------------------------------
DWORD __fastcall CItem::GetItemType(void)
{
	DWORD type = 0x00010002;    // Active text as Overlay
	if( m_ASize ){
		switch(m_Align){
			case 1:
				type |= 0x00400000; // align center
				break;
			case 2:
				type |= 0x00100000; // aling right-top
				break;
			default:
				break;
		}
	}
	return type;
}
//---------------------------------------------------------------------------
void __fastcall CItem::DeleteStorage(LPCBYTE pStorage)
{
	if( m_pMemStream != NULL ){
		ASSERT(pStorage == (LPCBYTE)m_pMemStream->Memory);
		delete m_pMemStream;
		m_pMemStream = NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall CItem::Copy(CItem *sp)
{
	ASSERT(sp);

	DWORD   size;
	LPCBYTE ps = sp->CreateStorage(&size);
	Create(ps, size);
	sp->DeleteStorage(ps);
	m_Back = sp->m_Back;
	if( m_ASize ){
		m_OrgXW = m_OrgYW = -1;
		m_TextXW = m_TextYW = -1;
	}
	else {
		m_IniSize = 1;
	}
}
//---------------------------------------------------------------------------
DWORD __fastcall CItem::GetOrgSize(void)
{
	if(m_OrgXW >= 0){
		DWORD w = m_OrgXW;
		DWORD h = m_OrgYW;
		return (h << 16) + w;
	}
	else {
		return (128 << 16) + 160;
	}
}
//---------------------------------------------------------------------------
void __fastcall CItem::SetFontSize(Graphics::TBitmap *pDest)
{
	ASSERT(m_OrgXW > 0);
	ASSERT(m_OrgYW > 0);
	if( (m_OrgXW <= 0) || (m_OrgYW <= 0) ) return;

	int size = m_pFont->Height;
	if( m_OrgXW < pDest->Width ){
		size = size * pDest->Width / m_OrgXW;
	}
	else if( m_OrgYW < pDest->Height ){
		size = size * pDest->Height / m_OrgYW;
	}
	else if( m_OrgXW > pDest->Width ){
		size = size * pDest->Width / m_OrgXW;
	}
	else if( m_OrgYW > pDest->Height ){
		size = size * pDest->Height / m_OrgYW;
	}
	m_pFont->Height = size;
}
//---------------------------------------------------------------------------
void __fastcall CItem::SetTextSize(Graphics::TBitmap *pDest)
{
	ASSERT(m_OrgXW > 0);
	ASSERT(m_OrgYW > 0);
	if( (m_OrgXW <= 0) || (m_OrgYW <= 0) ) return;

	if( m_OrgXW < pDest->Width ){
		m_TextXW = m_TextXW * pDest->Width / m_OrgXW;
	}
	else if( m_OrgYW < pDest->Height ){
		m_TextYW = m_TextYW * pDest->Height / m_OrgYW;
	}
	else if( m_OrgXW > pDest->Width ){
		m_TextXW = m_TextXW * pDest->Width / m_OrgXW;
	}
	else if( m_OrgYW > pDest->Height ){
		m_TextYW = m_TextYW * pDest->Height / m_OrgYW;
	}
}
//---------------------------------------------------------------------------
int __fastcall CItem::GetMaxWH(int &W, int &H, Graphics::TBitmap *pDest, LPCSTR pText)
{
	int lf = 1;
	int n = 0;
	W = H = 0;
	char bf[1024];
	LPSTR   t;
	for( t = bf; 1; pText++ ){
		if( *pText != 0x0d ){
			if( (*pText == 0x0a) || !*pText ){
				if( lf && *pText ){
					n++;
				}
				*t = 0;
				if( bf[0] ){
					DWORD wh = ::GetTabbedTextExtent(pDest->Canvas->Handle, bf, strlen(bf), 0, NULL);
					int w = wh & 0x0000ffff;
					int h = wh >> 16;
					if( H < h ) H = h;
					if( W < w ) W = w;
				}
				if( !*pText ) break;
				lf = 1;
				t = bf;
			}
			else {
				if( lf ){
					lf = 0;
					n++;
				}
				*t++ = *pText;
			}
		}
	}
	return n;
}
//---------------------------------------------------------------------------
UINT __fastcall CItem::GetDrawFormat(void)
{
	UINT wFormat = DT_NOPREFIX|DT_NOCLIP|DT_EXPANDTABS;
	switch(m_Align){
		case 1:
			wFormat |= DT_CENTER;
			break;
		case 2:
			wFormat |= DT_RIGHT;
			break;
		default:
			wFormat |= DT_LEFT;
			break;
	}
	if( !m_ASize ) wFormat |= DT_WORDBREAK;
	return wFormat;
}
//---------------------------------------------------------------------------
void __fastcall CItem::DrawText(RECT rc, int xoff, int yoff, Graphics::TBitmap *pDest, LPCSTR pText, UCOL ucol)
{
	rc.left += xoff; rc.top += yoff;
	rc.right += xoff; rc.bottom += yoff;
	pDest->Canvas->Font = m_pFont;
	pDest->Canvas->Font->Color = ucol.c;
	HDC hdc = pDest->Canvas->Handle;
	::SetBkMode(hdc, TRANSPARENT);
	::DrawText(hdc, pText, strlen(pText), &rc, GetDrawFormat());
}
//---------------------------------------------------------------------------
void __fastcall CItem::DrawTrans(Graphics::TBitmap *pDest, int x, int y, Graphics::TBitmap *pSrc)
{
	Graphics::TBitmap *pbmp = pSrc;
	if( sys.m_BitPixel == 16 ){
		pbmp = new Graphics::TBitmap;
		pbmp->Width = pSrc->Width;
		pbmp->Height = pSrc->Height;
		pbmp->Canvas->Draw(0, 0, pSrc);
	}
	pbmp->Transparent = TRUE;
	pbmp->TransparentMode = tmAuto;
	pDest->Canvas->Draw(x, y, pbmp);
	if( pbmp != pSrc ) delete pbmp;
}
//---------------------------------------------------------------------------
LPBYTE __fastcall CItem::CreateTable(Graphics::TBitmap *pDest)
{
	int XW = pDest->Width;
	int YW = pDest->Height;
	LPBYTE pTbl = new BYTE[XW*YW];
	memset(pTbl, 0, XW*YW);
	LPBYTE wp = pTbl;
	int x, y;

	BYTE    br = m_cBack.b.r;
	BYTE    bg = m_cBack.b.g;
	BYTE    bb = m_cBack.b.b;
	for( y = 0; y < YW; y++ ){
		LPBYTE p = (LPBYTE)pDest->ScanLine[y];
		for( x = 0; x < XW; x++, wp++, p+=3 ){
			if( (bb != *p) || (bg != *(p+1)) || (br != *(p+2)) ){
				*wp = 1;
			}
		}
	}
	return pTbl;
}
//---------------------------------------------------------------------------
int __fastcall CItem::EditDialog(HWND hWnd)
{
	TTextDlgBox *pBox = new TTextDlgBox(hWnd);
	if( pBox == NULL ) return FALSE;

	int r = pBox->Execute(this);
	delete pBox;
	if( r == TRUE ) UpdateDefault();
	return r;
}
//---------------------------------------------------------------------------
int __fastcall CItem::FontDialog(HWND hWnd)
{
	if( m_pFont == NULL ) m_pFont = new TFont;
	TFontDialog *pDlg = new TFontDialog(NULL);
	if( pDlg == NULL ) return FALSE;

	ASSERT(m_pFont);
	pDlg->Font->Assign(m_pFont);
	pDlg->Font->Color = m_ColG[0].c;
	MultProc();
	int r = pDlg->Execute();
	MultProc();
	if( r == TRUE ){
		m_pFont->Assign(pDlg->Font);
		m_ColG[0].c = pDlg->Font->Color;
		UpdateFont();
		UpdateDefault();
	}
	delete pDlg;
	return r;
}
//---------------------------------------------------------------------------
void __fastcall CItem::UpdateFont(void)
{
	if( m_pFont == NULL ) return;

	TFontStyles ts = m_pFont->Style;
	m_fsCode = FontStyle2Code(ts);
}
//---------------------------------------------------------------------------
void __fastcall CItem::Draw3D(RECT &rc, Graphics::TBitmap *pDest, LPCSTR pText, UCOL cl[2])
{
	int xm = m_3d & 0x00ff;
	int ym = (m_3d >> 8) & 0x00ff;
	if( xm & 0x0080 ) xm |= 0xffffff80;
	if( ym & 0x0080 ) ym |= 0xffffff80;
	int max = ABS(xm) > ABS(ym) ? ABS(xm) : ABS(ym);
	for( int i = max; i > 0; i-- ){
		int x = xm * i / max;
		int y = ym * i / max;
		DrawText(rc, x, y, pDest, pText, GetGrade2(cl, i, max));
	}
}
//---------------------------------------------------------------------------
Graphics::TBitmap* __fastcall CItem::DrawExt(Graphics::TBitmap *pDest, RECT &rc, LPCSTR pText)
{
	MultProc();
	int xw = pDest->Width;
	int yw = pDest->Height;
	LPBYTE  pTbl = NULL;
	if( m_Grade || (m_Shadow == 2) ) pTbl = CreateTable(pDest);

	LPBYTE wp, p;
	if( m_Grade ){
		MultProc();
		ASSERT(pTbl);
		RECT mc;
		mc.left = rc.right; mc.top = rc.bottom;
		mc.right = rc.left; mc.bottom = rc.top;
		wp = pTbl;
		int x, y;
		for( y = 0; y < yw; y++ ){
			for( x = 0; x < xw; x++, wp++ ){
				if( *wp ){
					if( mc.left > x ) mc.left = x;
					if( mc.right < x ) mc.right = x;
					if( mc.top > y ) mc.top = y;
					if( mc.bottom < y ) mc.bottom = y;
				}
			}
		}
		int bw = mc.right - mc.left + 1;
		int bh = mc.bottom - mc.top + 1;
		int bd = sqrt(bw*bw + bh*bh);
		UCOL    c;
		for( y = mc.top; y <= mc.bottom; y++ ){
			p = (LPBYTE)pDest->ScanLine[y];
			wp = &pTbl[y*xw];
			for( x = 0; x < xw; x++, wp++, p+=3 ){
				if( *wp ){
					int xx = x - mc.left;
					int yy = y - mc.top;
					switch(m_Grade){
						case 1:
							c = GetGrade4(m_ColG, xx, bw);
							break;
						case 2:
							c = GetGrade4(m_ColG, yy, bh);
							break;
						case 3:
							c = GetGrade4(m_ColG, sqrt(xx*xx + yy*yy), bd);
							break;
					}
					*p++ = c.b.b;
					*p++ = c.b.g;
					*p = c.b.r;
					p -= 2;
				}
			}
		}
	}
	MultProc();
	switch(m_Shadow){
		case 1:
			{
				Graphics::TBitmap* pbmp = CreateBitmap(xw, yw);
				FillBitmap(pbmp, m_cBack);
				if( m_3d & 0x00010000 ){
					Draw3D(rc, pbmp, pText, &m_ColB[1]);
				}
				else {
					for( int i = m_Line - 1; i >= 0; i-- ){
						DrawText(rc, i+1, i+1, pbmp, pText, m_ColB[0]);
					}
				}
				DrawTrans(pbmp, 0, 0, pDest);
				delete pDest;
				pDest = pbmp;
			}
			break;
		case 2:
			{
				ASSERT(pTbl);
				if( m_3d & 0x00010000 ){
					Graphics::TBitmap* pbmp = CreateBitmap(xw, yw);
					FillBitmap(pbmp, m_cBack);
					Draw3D(rc, pbmp, pText, &m_ColB[1]);
					DrawTrans(pbmp, 0, 0, pDest);
					delete pDest;
					pDest = pbmp;
				}
				int f;
				int x, y;
				for( y = 0; y < yw; y++ ){
					wp = &pTbl[y*xw];
					p = (LPBYTE)pDest->ScanLine[y];
					f = 0;
					for( x = 0; x < xw; x++, wp++, p+=3 ){
						if( *wp ){
							if( !m_Grade ){
								*p++ = m_ColG[0].b.b;
								*p++ = m_ColG[0].b.g;
								*p = m_ColG[0].b.r;
								p -= 2;
							}
							f = m_Line;
						}
						else if( f ){
							f--;
							if( !*wp ){
								*p++ = m_ColB[0].b.b;
								*p++ = m_ColB[0].b.g;
								*p = m_ColB[0].b.r;
								p -= 2;
								*wp = BYTE(f);
							}
						}
					}
					f = 0;
					wp--;
					p -= 3;
					x--;
					for( ; x >= 0; x--, wp--, p-=3 ){
						if( *wp ){
							f = m_Line;
						}
						else if( f ){
							f--;
							if( !*wp ){
								*p++ = m_ColB[0].b.b;
								*p++ = m_ColB[0].b.g;
								*p = m_ColB[0].b.r;
								p -= 2;
								*wp = BYTE(f);
							}
						}
					}

				}
				int ax = 0;
				for( x = 0; x < xw; x++, ax+=3 ){
					wp = &pTbl[x];
					f = 0;
					for( y = 0; y < yw; y++, wp += xw ){
						if( *wp ){
							f = m_Line;
						}
						else if( f ){
							f--;
							if( !*wp ){
								p = (LPBYTE)pDest->ScanLine[y];
								p += ax;
								*p++ = m_ColB[0].b.b;
								*p++ = m_ColB[0].b.g;
								*p = m_ColB[0].b.r;
								*wp = BYTE(f);
							}
						}
					}
					f = 0;
					y--;
					wp -= xw;
					for( ; y >= 0; y--, wp -= xw ){
						if( *wp ){
							f = m_Line;
						}
						else if( f ){
							f--;
							if( !*wp ){
								p = (LPBYTE)pDest->ScanLine[y];
								p += ax;
								*p++ = m_ColB[0].b.b;
								*p++ = m_ColB[0].b.g;
								*p = m_ColB[0].b.r;
							}
						}
					}
				}
			}
			break;
		case 3:
			{
				Graphics::TBitmap* pbmp = CreateBitmap(xw, yw);
				FillBitmap(pbmp, m_cBack);
				if( m_3d & 0x00010000 ){
					rc.left++; rc.right++;
					rc.top++; rc.bottom++;
					int xm = m_3d & 0x00ff;
					int ym = (m_3d >> 8) & 0x00ff;
					Draw3D(rc, pbmp, pText, &m_ColB[1]);
					DrawText(rc, (xm & 0x80) ? 1 : -1, (ym & 0x80) ? 1 : -1, pbmp, pText, m_ColB[0]);
				}
				else {
					for( int i = m_Line - 1; i >= 0; i-- ){
						DrawText(rc, i+2, i+2, pbmp, pText, m_ColB[0]);
					}
					DrawText(rc, 0, 0, pbmp, pText, m_ColB[1]);
				}
				DrawTrans(pbmp, 1, 1, pDest);
				delete pDest;
				pDest = pbmp;
			}
			break;
		default:
			break;
	}
	if( pTbl != NULL ) delete pTbl;

	return pDest;
}
//---------------------------------------------------------------------------
Graphics::TBitmap* __fastcall CItem::Draw(Graphics::TBitmap *pDest, LPCSTR pText)
{
	if( (pText == NULL) || !*pText ) return pDest;
	AnsiString as;
	if( m_Align ){
		as = pText;
		for( LPSTR t = as.c_str(); *t; t++ ){
			if( *t == '\t' ) *t = ' ';
		}
		pText = as.c_str();
	}
	m_Back.c = pDest->Canvas->Pixels[0][0];
	m_cBack = m_Back;
	if( m_pFont == NULL ){
		m_pFont = new TFont();
		m_pFont->Assign(pDest->Canvas->Font);
		m_pFont->Size = 18;
		UpdateFont();
	}
	else if( m_OrgXW >= 0 ){
		if( m_ASize ) SetFontSize(pDest);
	}

	LPCSTR pOrgText = pText;
	if( m_Zero ){
		pText = ZeroConv(pText, m_pFont->Charset);
	}

	pDest->Canvas->Font = m_pFont;

	int xw, yw;
	if( m_ASize ){
		int n = GetMaxWH(xw, yw, pDest, pText);
		yw *= n;
		if( m_fsCode & FSITALIC ){
			xw += yw / 12;
		}
	}
	else if( m_IniSize || (m_OrgXW < 0) ){
		m_IniSize = 0;
		if( m_TextXW > 0 ){
			xw = m_TextXW;
			yw = m_TextYW;
		}
		else {
			xw = pDest->Width;
			yw = pDest->Height;
		}
	}
	else {
		SetTextSize(pDest);
		xw = m_TextXW;
		yw = m_TextYW;
	}
	m_TextXW = xw;
	m_TextYW = yw;

	if( m_Shadow && (m_cBack.d == m_ColB[0].d) ){
		m_cBack = GetUniqueColor(pDest->Canvas->Handle, m_cBack);
	}
	if( m_cBack.d == m_ColG[0].d ){
		m_cBack = GetUniqueColor(pDest->Canvas->Handle, m_cBack);
	}

	MultProc();

	RECT rc;
	rc.left = rc.top = 0;
	rc.right = xw;
	rc.bottom = yw;

	if( m_3d & 0x00010000 ){
		int xm = m_3d & 0x00ff;
		int ym = (m_3d >> 8) & 0x00ff;
		if( xm & 0x0080 ) xm |= 0xffffff80;
		if( ym & 0x0080 ) ym |= 0xffffff80;
		if( xm < 0 ){
			rc.left -= xm;
			rc.right -= xm;
		}
		if( ym < 0 ){
			rc.top -= ym;
			rc.bottom -= ym;
		}
		xw += ABS(xm);
		yw += ABS(ym);
	}

	switch(m_Shadow){
		case 1:
			xw += m_Line;
			yw += m_Line;
			break;
		case 2:
			rc.left += m_Line;
			rc.right += m_Line;
			rc.top += m_Line;
			rc.bottom += m_Line;
			xw += m_Line*2;
			yw += m_Line*2;
			break;
		case 3:
			xw += m_Line*2;
			yw += m_Line*2;
			break;
	}

	if( (m_fsCode & FSITALIC) && (m_Align == 2) ){
		int ow = yw / 12;
		rc.left -= ow;
		rc.right -= ow;
	}

	Graphics::TBitmap *pBmp = pDest;
	if( (pDest->Width != xw) || (pDest->Height != yw) ){
		pBmp = CreateBitmap(xw, yw);
		FillBitmap(pBmp, m_cBack);
	}
	else if( m_Back.d != m_cBack.d ){
		FillBitmap(pBmp, m_cBack);
	}

	MultProc();
	DrawText(rc, 0, 0, pBmp, pText, m_ColG[0]);
	Graphics::TBitmap *pNew = pBmp;
	if( !m_Draft ){
		pNew = DrawExt(pBmp, rc, pText);
	}
	m_OrgXW = xw;
	m_OrgYW = yw;
	if( pBmp != pDest ) delete pDest;
	pDest = pNew;
	if( pOrgText != pText ) delete pText;

	// set transparent color to (0,0) for overlay
	LPBYTE p = (LPBYTE)pDest->ScanLine[0];
	ASSERT(p);
	*p++ = m_cBack.b.b;
	*p++ = m_cBack.b.g;
	*p = m_cBack.b.r;

	MultProc();
	return pDest;
}


