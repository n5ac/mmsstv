//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "ComLib.h"
#include "TextDlg.h"
SYS	sys;
//---------------------------------------------------------------------------
void __fastcall InitSys(HINSTANCE hinst)
{
	sys.m_hInstance = hinst;
	sys.m_dwVersion = 0;
	sys.m_WinNT = 0;
	sys.m_CodePage = 0;
	sys.m_MsgEng = 0;
	sys.m_BitPixel = 0;
	sys.m_fLoadImageMenu = 0;
	sys.m_fPasteImage = 0;
	sys.m_fGetMacKey = 0;
	sys.m_fConvMac = 0;
	sys.m_fMultProc = 0;
	sys.m_fColorComp = 0;
    sys.m_fAdjCol = 0;
    sys.m_fClip = 0;
	sys.m_DefStgSize = 0;
	sys.m_pDefStg = NULL;
	sys.m_pTextList = NULL;
	sys.m_pStyleList = NULL;
	sys.m_BgnDir[0] = 0;
	sys.m_pFontList = NULL;
	sys.m_pBitmapTemp = NULL;
	sys.m_DefaultPage = 0;
	sys.m_SBMListState = 0;

    sys.m_MaskC1.c = clYellow;
    sys.m_MaskC2.c = clBlack;

    sys.m_CodeLeft = 0;
    sys.m_CodeTop = 0;
    sys.m_CodeOut = 0;

    sys.m_fFontFam = FALSE;
    memset(sys.m_tFontFam, 0, sizeof(sys.m_tFontFam));

	sys.m_dwVersion = ::GetVersion();
	sys.m_WinNT = (sys.m_dwVersion < 0x80000000) ? TRUE : FALSE;
	sys.m_CodePage = ::GetACP();	// Get codepage

	HDC hdcScreen = ::CreateDC("DISPLAY", NULL, NULL, NULL);
	sys.m_BitPixel = ::GetDeviceCaps(hdcScreen, BITSPIXEL);
	::DeleteDC(hdcScreen);

	char bf[512];
	::GetModuleFileName(hinst, bf, sizeof(bf));
	::SetDirName(sys.m_BgnDir, bf);

    if( sys.m_BitPixel < 24 ){
		sys.m_pBitmapTemp = new Graphics::TBitmap;
		sys.m_pBitmapTemp->Width = 1;
   		sys.m_pBitmapTemp->Height = 1;
    }

    sys.m_fEudc = TRUE;
    sys.m_Eudc = "";
}
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
		case 0:
			sys.m_fLoadImageMenu = (mmLoadImageMenu)pFunc;
            break;
		case 5:
        	sys.m_fPasteImage = (mmPasteImage)pFunc;
            break;
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
		case 14:
        	sys.m_fAdjCol = (mmAdjCol)pFunc;
            break;
        case 15:
        	sys.m_fClip = (mmClip)pFunc;
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
//---------------------------------------------------------------------------
int __fastcall IsFile(LPCSTR pName)
{
	WIN32_FIND_DATA fdata;
	HANDLE hFind = ::FindFirstFile(pName, &fdata);
    if( hFind != INVALID_HANDLE_VALUE ){
		FindClose(hFind);
        return TRUE;
    }
    else {
		return FALSE;
    }
}
//---------------------------------------------------------------------------
void __fastcall SetMBCP(BYTE charset)
{
	UINT cp;
	switch(charset){
		case SHIFTJIS_CHARSET:
			cp = 932;
			break;
		case HANGEUL_CHARSET:
			cp = 949;
			break;
        case JOHAB_CHARSET:
			cp = 1361;
        	break;
		case CHINESEBIG5_CHARSET:   //
			cp = 950;
			break;
		case 134:       // 簡略
			cp = 936;
			break;
        case ANSI_CHARSET:
        case SYMBOL_CHARSET:
        	cp = 1252;
            break;
		default:
			cp = _MB_CP_ANSI;
			break;
	}
	if( cp != _MB_CP_ANSI ){
		CPINFO info;
		if( GetCPInfo(cp, &info) != TRUE ){
			cp = _MB_CP_ANSI;
		}
	}
	_setmbcp(cp);
}
///----------------------------------------------------------------
void __fastcall Yen2CrLf(AnsiString &ws, AnsiString cs)
{
	_setmbcp(_MB_CP_ANSI);
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
	_setmbcp(_MB_CP_ANSI);
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
///----------------------------------------------------------------
///  文字列を数値に変換する
///
int htoin(LPCSTR sp, int n)
{
	int	d = 0;

	for( ; *sp && n; sp++, n-- ){
		d = d * 16;
		d += *sp & 0x0f;
		if( *sp >= 'A' ) d += 9;
	}
	return d;

}
//---------------------------------------------------------------------------
int __fastcall GetActiveIndex(TPageControl *pp)
{
	int i;
	for( i = 0; i < pp->PageCount; i++ ){
		if( pp->ActivePage == pp->Pages[i] ) return i;
	}
	return 0;
}
//---------------------------------------------------------------------------
void __fastcall LoadStringsFromIniFile(TStrings *pList, LPCSTR pSect, LPCSTR pName, int msg)
{
	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, pName);
	TMemIniFile *pIniFile = new TMemIniFile(fname);
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
	if( msg ){
		as = pIniFile->ReadString(pSect, "MSG", "");
		::Yen2CrLf(sys.m_Msg, as);
	}
	delete pIniFile;
}
//---------------------------------------------------------------------------
void __fastcall SaveStringsToIniFile(TStrings *pList, LPCSTR pSect, LPCSTR pName, int msg)
{
	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, pName);
	TMemIniFile *pIniFile = new TMemIniFile(fname);
	pIniFile->WriteInteger(pSect, "Count", pList->Count);
	int i;
	for( i = 0; i < pList->Count; i++ ){
		char bf[32];
		sprintf(bf, "Text%u", i);
		pIniFile->WriteString(pSect, bf, pList->Strings[i]);
	}
	if( msg ){
		AnsiString as;
		::CrLf2Yen(as, sys.m_Msg);
		pIniFile->WriteString(pSect, "MSG", as);
	}
	pIniFile->UpdateFile();
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
#if 1
//---------------------------------------------------------------------------
void __fastcall ExecPB(LPCSTR pName)
{
	char bf[512];

	MultProc();
	strcpy(bf, "mspaint.exe");
	if( pName != NULL ){
		wsprintf(&bf[strlen(bf)], " \x22%s\x22", pName);
	}
	if( ::WinExec(bf, SW_SHOWDEFAULT) <= 31 ){
		strcpy(bf, "PBRUSH.EXE");
		if( pName != NULL ){
			wsprintf(&bf[strlen(bf)], " \x22%s\x22", pName);
		}
		::WinExec(bf, SW_SHOWDEFAULT);
	}
}
//---------------------------------------------------------------------------
HWND __fastcall IsPB(void)
{
	return FindWindow("MSPaintApp", NULL);
}
#endif
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
BOOL __fastcall IsSameBitmap(Graphics::TBitmap *pDest, Graphics::TBitmap *pSrc)
{
	int xw = pSrc->Width;
    int yw = pSrc->Height;
	if( pDest->Width != xw ) return FALSE;
    if( pDest->Height != yw ) return FALSE;
	int x, y;
    for( y = 0; y < yw; y++ ){
		LPBYTE p = (LPBYTE)pSrc->ScanLine[y];
        LPBYTE t = (LPBYTE)pDest->ScanLine[y];
		for( x = 0; x < xw; x++ ){
			if( *p++ != *t++ ) return FALSE;
			if( *p++ != *t++ ) return FALSE;
			if( *p++ != *t++ ) return FALSE;
        }
    }
    return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall FillBitmap(Graphics::TBitmap *pDest, TColor col)
{
	UCOL c;
    c.c = col;
    FillBitmap(pDest, c);
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
void __fastcall CopyBitmap(Graphics::TBitmap *pBitmap)
{
	int DataHandle;
	HPALETTE APalette;
	unsigned short MyFormat;
	pBitmap->SaveToClipboardFormat(MyFormat, DataHandle, APalette);
	Clipboard()->SetAsHandle(MyFormat,DataHandle);
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
//伸縮コピー
void __fastcall StretchCopy(Graphics::TBitmap *pDest, Graphics::TBitmap *pSrc, int mode)
{
	TRect tc;
	tc.Left = tc.Top = 0;
	tc.Right = pDest->Width; tc.Bottom = pDest->Height;
	int Sop = ::SetStretchBltMode(pDest->Canvas->Handle, mode);
	pDest->Canvas->StretchDraw(tc, pSrc);
	::SetStretchBltMode(pDest->Canvas->Handle, Sop);
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
#if 0
//---------------------------------------------------------------------------
//サイズ変更
Graphics::TBitmap* __fastcall ChangeSize(Graphics::TBitmap *pSrc, int xw, int yw, int dir)
{
	int XW = xw;
    int YW = yw;
	if( dir ){		// 縮小
		XW /= 4; YW /= 4;
    }
	Graphics::TBitmap *pDest = CreateBitmap(XW, YW);
	StretchCopy(pDest, pSrc, HALFTONE);
    delete pSrc;
    return pDest;
}
#endif
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
UCOL __fastcall GetUniqueColor(UCOL ucol)
{
	ucol.d = ucol.d & 0x00f0f0f0;
	UCOL c = ucol;
	DWORD r;

	while(!c.d || (c.d == ucol.d)){
#if (RANDMAX >= 0x00ffffff)
		c.d = rand() & 0x00f0f0f0;
#else
		r = rand(); r = r << 4;
		c.b.r = (BYTE)(r & 0x00f0); r = r >> 4;
		c.b.g = (BYTE)(r & 0x00f0); r = r >> 4;
		c.b.b = (BYTE)(r & 0x00f0);
#endif
	}
	if( sys.m_pBitmapTemp ){
		sys.m_pBitmapTemp->Canvas->Pixels[0][0] = c.c;
		c.c = sys.m_pBitmapTemp->Canvas->Pixels[0][0];
	}
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
//CBitStreamクラス
///---------------------------------------------------------
const BYTE _btt[]={0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
///---------------------------------------------------------
DWORD CBitStream::GetData(int n)
{
	const BYTE *pData;
	DWORD d = 0;
    for( int i = 0; i < n; i++, m_BitCount++ ){
		d = d << 1;
		pData = &m_pBase[m_BitCount / 8];
        if( *pData & _btt[m_BitCount % 8] ) d |= 1;
    }
    return d;
}
///---------------------------------------------------------
void CBitStream::GetText(LPSTR p, int n)
{
	for( int i = 0; i < n; i++, p++ ){
		*p = (char)GetData(8);
    }
    *p = 0;
}
///---------------------------------------------------------
void CBitStream::PutData(DWORD d, int n)
{
	BYTE *pData;
    DWORD mask = 1;
	int i;
    for( i = 1; i < n; i++ ) mask = mask << 1;
    for( i = 0; i < n; i++, m_BitCount++, mask = mask >> 1 ){
		pData = &m_pBase[m_BitCount / 8];
		if( !(m_BitCount % 8) ) *pData = 0;
		if( d & mask ) *pData |= _btt[m_BitCount % 8];
    }
}
///---------------------------------------------------------
void CBitStream::PutText(LPCSTR p, int n)
{
	int i;
	for( i = 0; (i < n) && *p; i++, p++ ){
		PutData(*p, 8);
    }
    for( ; i < n; i++ ) PutData(0, 8);
}
///---------------------------------------------------------
BYTE CBitStream::GetSum(void)
{
	int n = (m_BitCount + 7) / 8;
    BYTE sum = 0;
    BYTE *pData = m_pBase;
    for( int i = 0; i < n; i++, pData++ ){
		sum += *pData;
    }
    return sum;
}
///---------------------------------------------------------
/// pBase : バッファのポインタ
/// n : チェックサムコードの位置(1 ～ )
BOOL CheckSum(const void *pBase, int n)
{
	const BYTE *p = (const BYTE *)pBase;
	n--;
	BYTE sum = 0;
    for( int i = 0; i < n; i++, p++ ){
		sum += *p;
    }
    return sum == *p;
}
///------------------------------------------------------
///
///CWaitCursor クラス
///
CWaitCursor::CWaitCursor()
{
	sv = Screen->Cursor;
	Screen->Cursor = crHourGlass;
}

CWaitCursor::CWaitCursor(BOOL sw)
{
	sv = Screen->Cursor;
	if( sw ){
		Screen->Cursor = crHourGlass;
    }
}

CWaitCursor::~CWaitCursor()
{
	Screen->Cursor = sv;
}

void CWaitCursor::Delete(void)
{
	Screen->Cursor = sv;
}

void CWaitCursor::Wait(void)
{
	Screen->Cursor = crHourGlass;
}
//
//
//---------------------------------------------------------------------------
// CItem class
//
__fastcall CItem::CItem(void)
{
	m_pMemStream = NULL;

	m_Ver = 5;
	m_Align = 0;
	m_Grade = 0;
	m_Shadow = 2;
	m_Line = 1;
	m_Zero = 0;
	m_3d = 0x00000404;
	m_Text = "de %m";
	m_pFont = NULL;
	m_ColG[0].d = 255;
	m_ColG[1].d = 16711935;
	m_ColG[2].d = 65535;
	m_ColG[3].d = 16776960;
	m_ColB[0].c = clBlack;
	m_ColB[1].c = clWhite;
	m_ColB[2].c = clBlue;
	m_ColB[3].c = clRed;

	m_ColF[0].d = RGB(255,0,0);
	m_ColF[1].d = RGB(255,255,0);
    m_ColF[2].d = RGB(0,255,0);
    m_ColF[3].d = RGB(0,255,255);
	m_ColF[4].d = RGB(0,0,255);
    m_ColF[5].d = RGB(255,0,255);

	m_OrgXW = -1;
	m_OrgYW = -1;
	m_BmpXW = -1;
	m_BmpYW = -1;
	m_Draft = 0;
	m_Pos = 0;
	m_Ex = 0;
    m_OYW = 0;
    m_OYWS = 0;
    m_Rot = 0;
	m_Filter = 0;

	// Init as default
	m_sperspect.ver = 3;
	m_sperspect.flag = 1;
	m_sperspect.ax = 3.0; m_sperspect.ay = 3.0;
	m_sperspect.px = m_sperspect.py = m_sperspect.pz = 0.0;
	m_sperspect.rz = -180; m_sperspect.rx = m_sperspect.ry = 0;
	m_sperspect.v = 10; m_sperspect.s = 1.5;
	m_sperspect.r = 0.0;
	m_SC = 0;

	m_pMaskBmp = NULL;
}
//---------------------------------------------------------------------------
__fastcall CItem::~CItem()
{
	if( m_pFont != NULL ) delete m_pFont;
	if( m_pMemStream != NULL ) delete m_pMemStream;
    if( m_pMaskBmp != NULL ) delete m_pMaskBmp;
}
//---------------------------------------------------------------------------
void __fastcall CItem::LoadDefault(void)
{
	if( sys.m_pDefStg != NULL ) return;

	CItem *pItem = new CItem;
	pItem->LoadFromInifile("Default", ININAME, FALSE);
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

	m_OYWS = 0;
	TMemoryStream *pMem = new TMemoryStream;
	pMem->Clear();
	pMem->Write(ps, size);
	pMem->Seek(soFromBeginning, 0);
	int ver;
	pMem->Read(&ver, sizeof(ver));
	if( ver <= m_Ver ){
		pMem->Read(&m_BmpXW, sizeof(m_BmpXW));
		pMem->Read(&m_BmpYW, sizeof(m_BmpYW));
		if( ver <= 1 ){
			m_BmpXW = m_BmpYW = -1;
		}
		pMem->Read(&m_OrgXW, sizeof(m_OrgXW));
		pMem->Read(&m_OrgYW, sizeof(m_OrgYW));
		pMem->Read(&m_Align, sizeof(m_Align));
		pMem->Read(&m_Grade, sizeof(m_Grade));
		pMem->Read(&m_Shadow, sizeof(m_Shadow));
		pMem->Read(&m_Line, sizeof(m_Line));
		pMem->Read(&m_Zero, sizeof(m_Zero));
		pMem->Read(&m_3d, sizeof(m_3d));
		pMem->Read(&m_ColG, sizeof(m_ColG));
		pMem->Read(&m_ColB, sizeof(m_ColB));
        if( ver >= 5 ){
			pMem->Read(&m_ColF, sizeof(m_ColF));
        }
		pMem->Read(&m_OYW, sizeof(m_OYW));
		pMem->Read(&m_Rot, sizeof(m_Rot));
        if( ver >= 4 ){
			pMem->Read(&m_Filter, sizeof(m_Filter));
        }
		ReadString(pMem, m_Text);
		if( m_pFont != NULL ) delete m_pFont;
		m_pFont = ReadFont(pMem);
		UpdateFont();
		pMem->Read(&m_SC, sizeof(m_SC));
    	pMem->Read(&m_sperspect, sizeof(m_sperspect));
		if( m_pMaskBmp ){
        	delete m_pMaskBmp;
            m_pMaskBmp = NULL;
        }
		if( ver >= 3 ){
			int fBmp;
            pMem->Read(&fBmp, sizeof(fBmp));
            if( fBmp ){
				m_pMaskBmp = new Graphics::TBitmap;
				m_pMaskBmp->LoadFromStream(pMem);
            }
        }
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
	m_Ver = 5;
	m_pMemStream->Write(&m_Ver, sizeof(m_Ver));
	m_pMemStream->Write(&m_BmpXW, sizeof(m_BmpXW));
	m_pMemStream->Write(&m_BmpYW, sizeof(m_BmpYW));
	m_pMemStream->Write(&m_OrgXW, sizeof(m_OrgXW));
	m_pMemStream->Write(&m_OrgYW, sizeof(m_OrgYW));
	m_pMemStream->Write(&m_Align, sizeof(m_Align));
	m_pMemStream->Write(&m_Grade, sizeof(m_Grade));
	m_pMemStream->Write(&m_Shadow, sizeof(m_Shadow));
	m_pMemStream->Write(&m_Line, sizeof(m_Line));
	m_pMemStream->Write(&m_Zero, sizeof(m_Zero));
	m_pMemStream->Write(&m_3d, sizeof(m_3d));
	m_pMemStream->Write(&m_ColG, sizeof(m_ColG));
	m_pMemStream->Write(&m_ColB, sizeof(m_ColB));
	m_pMemStream->Write(&m_ColF, sizeof(m_ColF));
	int oyw = GetTrueOYW();
	m_pMemStream->Write(&oyw, sizeof(oyw));
	m_pMemStream->Write(&m_Rot, sizeof(m_Rot));
	m_pMemStream->Write(&m_Filter, sizeof(m_Filter));
	WriteString(m_pMemStream, m_Text);
	WriteFont(m_pMemStream, m_pFont);
	m_pMemStream->Write(&m_SC, sizeof(m_SC));
    m_pMemStream->Write(&m_sperspect, sizeof(m_sperspect));
	int fBmp = m_pMaskBmp != NULL;
    m_pMemStream->Write(&fBmp, sizeof(fBmp));
    if( fBmp ) m_pMaskBmp->SaveToStream(m_pMemStream);

	*pSize = m_pMemStream->Position;
	return (LPCBYTE)m_pMemStream->Memory;
}
//---------------------------------------------------------------------------
void __fastcall SaveMaskToIniFile(Graphics::TBitmap *pBitmap, LPCSTR pSect, TMemIniFile *pIniFile)
{
	if( pBitmap ){
    	int xw = pBitmap->Width;
        int yw = pBitmap->Height;
		pIniFile->WriteInteger(pSect, "M.Size", (yw << 16) + xw);

		LPBYTE bp = new BYTE[xw*yw*3+1];
        LPBYTE wp = bp;
        int x, y;
		for( y = 0; y < yw; y++ ){
			LPBYTE p = (LPBYTE)pBitmap->ScanLine[y];
			for( x = 0; x < xw; x++ ){
            	*wp++ = *p++;
                *wp++ = *p++;
                *wp++ = *p++;
            }
        }
		int bsize = xw * yw * 3 * 8;
        int csize = (bsize + 6) / 7;
		char *sp = new char[csize + 1];
        LPSTR t = sp;
        CBitStream cb(bp);
        BYTE c;
		for( x = 0; x < csize; x++ ){
        	c = (BYTE)(cb.GetData(7) + 0x30);
            if( c >= 0x70 ) c += (BYTE)0x30;
			*t++ = c;
        }
        *t = 0;
		pIniFile->WriteString(pSect, "M.Data", sp);
		delete sp;
        delete bp;
    }
    else {
		pIniFile->WriteInteger(pSect, "M.Size", 0);
        pIniFile->WriteString(pSect, "M.Data", "");
    }
}
//---------------------------------------------------------------------------
Graphics::TBitmap *__fastcall LoadMaskFromIniFile(LPCSTR pSect, TMemIniFile *pIniFile)
{
	Graphics::TBitmap *pBitmap = NULL;
    int xw = pIniFile->ReadInteger(pSect, "M.Size", 0);
	if( xw ){
		AnsiString as = pIniFile->ReadString(pSect, "M.Data", "");
		int yw = xw >> 16;
        xw &= 0x0000ffff;

        int x, y;
		int bsize = xw * yw * 3 * 8;
        int csize = (bsize + 6) / 7;
		LPBYTE bp = new BYTE[xw*yw*3+1];
        CBitStream cb(bp);
		LPCSTR p = as.c_str();
        BYTE c;
        for( x = 0; (x < csize) && *p; x++, p++ ){
			c = BYTE(*p - 0x30);
            if( c >= 0x40 ) c -= (BYTE)0x30;
			cb.PutData(c, 7);
        }
		pBitmap = CreateBitmap(xw, yw);
		LPBYTE sp = bp;
        for( y = 0; y < yw; y++ ){
			LPBYTE wp = (LPBYTE)pBitmap->ScanLine[y];
			for( x = 0; x < xw; x++ ){
				*wp++ = *sp++;
				*wp++ = *sp++;
				*wp++ = *sp++;
            }
        }
        delete bp;
    }
	return pBitmap;
}
//---------------------------------------------------------------------------
void __fastcall LoadFontFromInifile(TFont *pFont, LPCSTR pSect, TMemIniFile *pIniFile)
{
	if( pFont == NULL ) return;

	pFont->Name = pIniFile->ReadString(pSect, "FontName", pFont->Name);
	pFont->Charset = (BYTE)pIniFile->ReadInteger(pSect, "FontSet", pFont->Charset);
	pFont->Height = pIniFile->ReadInteger(pSect, "FontSize", pFont->Height);
	TFontStyles ts = pFont->Style;
	DWORD d = FontStyle2Code(ts);
	d = pIniFile->ReadInteger(pSect, "FontStyle", d);
	ts = Code2FontStyle(d);
	pFont->Style = ts;
}
//---------------------------------------------------------------------------
void __fastcall SaveFontToInifile(TFont *pFont, LPCSTR pSect, TMemIniFile *pIniFile)
{
	if( pFont == NULL ) return;

	pIniFile->WriteString(pSect, "FontName", pFont->Name);
	pIniFile->WriteInteger(pSect, "FontSet", pFont->Charset);
	pIniFile->WriteInteger(pSect, "FontSize", pFont->Height);
	TFontStyles ts = pFont->Style;
	DWORD d = FontStyle2Code(ts);
	pIniFile->WriteInteger(pSect, "FontStyle", d);
}
//---------------------------------------------------------------------------
void __fastcall SavePerToInifile(LPCSTR pSect, SPERSPECT *pPer, int sc, TMemIniFile *pIniFile)
{
	pIniFile->WriteInteger(pSect, "P.Flag", pPer->flag);
	pIniFile->WriteInteger(pSect, "P.AX", pPer->ax * 1000);
	pIniFile->WriteInteger(pSect, "P.AY", pPer->ay * 1000);
	pIniFile->WriteInteger(pSect, "P.PX", pPer->px * 1000);
	pIniFile->WriteInteger(pSect, "P.PY", pPer->py * 1000);
	pIniFile->WriteInteger(pSect, "P.PZ", pPer->pz * 1000);
	pIniFile->WriteInteger(pSect, "P.RX", pPer->rx * 1000);
	pIniFile->WriteInteger(pSect, "P.RY", pPer->ry * 1000);
	pIniFile->WriteInteger(pSect, "P.RZ", pPer->rz * 1000);
	pIniFile->WriteInteger(pSect, "P.S", pPer->s * 1000);
	pIniFile->WriteInteger(pSect, "P.R", pPer->r * 1000);
	pIniFile->WriteInteger(pSect, "P.SC", sc);
}
//---------------------------------------------------------------------------
void __fastcall LoadPerFromInifile(LPCSTR pSect, SPERSPECT *pPer, int &sc, TMemIniFile *pIniFile)
{
	pPer->flag = pIniFile->ReadInteger(pSect, "P.Flag", pPer->flag);
	pPer->ax = pIniFile->ReadInteger(pSect, "P.AX", pPer->ax * 1000)/1000.0;
	pPer->ay = pIniFile->ReadInteger(pSect, "P.AY", pPer->ay * 1000)/1000.0;
	pPer->px = pIniFile->ReadInteger(pSect, "P.PX", pPer->px * 1000)/1000.0;
	pPer->py = pIniFile->ReadInteger(pSect, "P.PY", pPer->py * 1000)/1000.0;
	pPer->pz = pIniFile->ReadInteger(pSect, "P.PZ", pPer->pz * 1000)/1000.0;
	pPer->rx = pIniFile->ReadInteger(pSect, "P.RX", pPer->rx * 1000)/1000.0;
	pPer->ry = pIniFile->ReadInteger(pSect, "P.RY", pPer->ry * 1000)/1000.0;
	pPer->rz = pIniFile->ReadInteger(pSect, "P.RZ", pPer->rz * 1000)/1000.0;
	pPer->s = pIniFile->ReadInteger(pSect, "P.S", pPer->s * 1000)/1000.0;
	pPer->r = pIniFile->ReadInteger(pSect, "P.R", pPer->r * 1000)/1000.0;
	sc = pIniFile->ReadInteger(pSect, "P.SC", sc);
}
//---------------------------------------------------------------------------
void __fastcall CItem::LoadFromInifile(LPCSTR pSect, LPCSTR pName, BOOL fMask)
{
	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, pName);
	TMemIniFile *pIniFile = new TMemIniFile(fname);
	MultProc();
	m_Align = pIniFile->ReadInteger(pSect, "Align", m_Align);
	m_Grade = pIniFile->ReadInteger(pSect, "Grade", m_Grade);
	m_Shadow = pIniFile->ReadInteger(pSect, "Shadow", m_Shadow);
	m_Line = pIniFile->ReadInteger(pSect, "Line", m_Line);
	m_Zero = pIniFile->ReadInteger(pSect, "Zero", 0);
	m_3d = pIniFile->ReadInteger(pSect, "3D", m_3d);
	m_OYW = pIniFile->ReadInteger(pSect, "OYW", m_OYW);
	m_Rot = pIniFile->ReadInteger(pSect, "Rot", m_Rot);
	m_Filter = pIniFile->ReadInteger(pSect, "Filter", m_Filter);
	int i;
	for( i = 0; i < 4; i++ ){
		char bf[32];
		sprintf(bf, "ColG%u", i);
		m_ColG[i].d = pIniFile->ReadInteger(pSect, bf, m_ColG[i].d);
		sprintf(bf, "ColB%u", i);
		m_ColB[i].d = pIniFile->ReadInteger(pSect, bf, m_ColB[i].d);
	}
	m_OrgXW = m_OrgYW = -1;
	m_BmpXW = m_BmpYW = -1;
	AnsiString as, cs;
	::CrLf2Yen(cs, m_Text);
	as = pIniFile->ReadString(pSect, "Text", cs);
	::Yen2CrLf(m_Text, as);

	if( pIniFile->ReadInteger(pSect, "Font", 0) ){
		if( m_pFont != NULL ) delete m_pFont;
		m_pFont = new TFont;
		LoadFontFromInifile(m_pFont, pSect, pIniFile);
		UpdateFont();
	}
	LoadPerFromInifile(pSect, &m_sperspect, m_SC, pIniFile);

    if( fMask ){
		if( m_pMaskBmp ) delete m_pMaskBmp;
		m_pMaskBmp = LoadMaskFromIniFile(pSect, pIniFile);
    }
   	delete pIniFile;
}
//---------------------------------------------------------------------------
void __fastcall CItem::SaveToInifile(LPCSTR pSect, LPCSTR pName, BOOL fMask)
{
	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, pName);

	TMemIniFile *pIniFile = new TMemIniFile(fname);
	MultProc();
	try {
		pIniFile->WriteInteger(pSect, "Align", m_Align);
		pIniFile->WriteInteger(pSect, "Grade", m_Grade);
		pIniFile->WriteInteger(pSect, "Shadow", m_Shadow);
		pIniFile->WriteInteger(pSect, "Line", m_Line);
		pIniFile->WriteInteger(pSect, "Zero", m_Zero);
		pIniFile->WriteInteger(pSect, "3D", m_3d);
		pIniFile->WriteInteger(pSect, "OYW", GetTrueOYW());
		pIniFile->WriteInteger(pSect, "Rot", m_Rot);
		pIniFile->WriteInteger(pSect, "Filter", m_Filter);
		int i;
		for( i = 0; i < 4; i++ ){
			char bf[32];
			sprintf(bf, "ColG%u", i);
			pIniFile->WriteInteger(pSect, bf, m_ColG[i].d);
			sprintf(bf, "ColB%u", i);
			pIniFile->WriteInteger(pSect, bf, m_ColB[i].d);
		}
		pIniFile->WriteInteger(pSect, "Width", m_BmpXW);
		pIniFile->WriteInteger(pSect, "Height", m_BmpYW);
		AnsiString cs;
		::CrLf2Yen(cs, m_Text);
		pIniFile->WriteString(pSect, "Text", cs);

		pIniFile->WriteInteger(pSect, "Font", m_pFont != NULL ? 1 : 0);
		if( m_pFont != NULL ){
			SaveFontToInifile(m_pFont, pSect, pIniFile);
		}
		SavePerToInifile(pSect, &m_sperspect, m_SC, pIniFile);
        if( fMask ){
			SaveMaskToIniFile(m_pMaskBmp, pSect, pIniFile);
        }
	}
	catch(...){
	}
	pIniFile->UpdateFile();
	delete pIniFile;
}
//---------------------------------------------------------------------------
void __fastcall CItem::DeleteInifile(LPCSTR pSect, LPCSTR pName)
{
	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, pName);

	TMemIniFile *pIniFile = new TMemIniFile(fname);
	MultProc();
	try {
		pIniFile->EraseSection(pSect);
	}
	catch(...){
	}
	pIniFile->UpdateFile();
	delete pIniFile;
}
//---------------------------------------------------------------------------
DWORD __fastcall CItem::GetItemType(void)
{
	DWORD type = 0x00010002;    // Active text as Overlay
		switch(m_Align){
			case 1:
				type |= 0x00400000; // align center
				break;
			case 2:
				type |= 0x00100000; // aling right-top
				break;
			case 3:
				type |= m_Pos << 20;
				type |= 0x00000010;
				break;
			default:
				break;
		}
	return type;
}
//---------------------------------------------------------------------------
void __fastcall CItem::SetPos(DWORD pos, DWORD size, DWORD tsize)
{
	int x = pos & 0x0000ffff;
	int y = pos >> 16;
	if( x & 0x00008000 ) x |= 0xffff8000;
	if( y & 0x00008000 ) y |= 0xffff8000;
	int xw = size & 0x0000ffff;
	int yw = size >> 16;
	int txw = tsize & 0x0000ffff;
	int tyw = tsize >> 16;

	int xr = x + xw;
	int yb = y + yw;
	m_Pos = 0;
	if( xr > (txw * 7/8) ){
		m_Pos |= 1;
	}
	else if( x > (txw / 8) ){
		m_Pos |= 4;
	}
	if( yb > (tyw * 6/8) ){
		m_Pos |= 2;
	}
	else if( y > (tyw * 2 / 8) ){
		m_Pos |= 8;
	}
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
int __fastcall CItem::GetTrueOYW(void)
{
    int oyw = int((SHORT)m_OYW);
	if( m_OYWS ){
		int fwh = m_OYWS >> 16;
        int fwl = m_OYWS & 0x0000ffff;
		if( fwl ) oyw = oyw * fwh / fwl;
    }
    return oyw;
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
	m_OrgXW = m_OrgYW = -1;
//	m_TextXW = m_TextYW = -1;
	m_Pos = sp->m_Pos;
    m_Ex = sp->m_Ex;
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
		if( size == m_pFont->Height ) size--;
	}
	else if( m_OrgYW < pDest->Height ){
		size = size * pDest->Height / m_OrgYW;
		if( size == m_pFont->Height ) size--;
	}
	else if( m_OrgXW > pDest->Width ){
		size = size * pDest->Width / m_OrgXW;
		if( size == m_pFont->Height ) size++;
	}
	else if( m_OrgYW > pDest->Height ){
		size = size * pDest->Height / m_OrgYW;
		if( size == m_pFont->Height ) size++;
	}
	if( !size ) size = -1;
	m_pFont->Height = size;
}
//---------------------------------------------------------------------------
static int __fastcall TextWidth(TCanvas *pCanvas, LPCSTR p)
{
	char bf[8];

	int w = 0;
    while(*p){
		if( (_mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD) && *(p+1) ){
			bf[0] = *p++; bf[1] = *p++; bf[2] = 0;
		}
        else {
			bf[0] = *p++; bf[1] = 0;
        }
        w += pCanvas->TextWidth(bf);
    }
    return w;
}
//---------------------------------------------------------------------------
int __fastcall CItem::GetMaxWH(int &W, int &H, Graphics::TBitmap *pDest, LPCSTR pText)
{
	TCanvas *pCanvas = pDest->Canvas;
	int lf = 1;
	int n = 0;
	W = H = 0;
	char bf[1024];
    int w, h;
	LPSTR   t;
	for( t = bf; 1; pText++ ){
		if( *pText != 0x0d ){
			if( (*pText == 0x0a) || !*pText ){
				if( lf && *pText ){
					n++;
				}
				*t = 0;
				if( bf[0] ){
                	if( m_Grade == 4 ){
						w = TextWidth(pCanvas, bf);
                        h = pCanvas->TextHeight(bf);
                    }
                    else {
						DWORD wh = ::GetTabbedTextExtent(pCanvas->Handle, bf, strlen(bf), 0, NULL);
						w = wh & 0x0000ffff; h = wh >> 16;
                    }
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
UINT __fastcall CItem::GetPosFormat(void)
{
	UINT wFormat;
	switch(m_Align){
		case 1:
			wFormat = DT_CENTER;
			break;
		case 2:
			wFormat = DT_RIGHT;
			break;
		case 3:
			if( m_Pos & 4 ){
				wFormat = DT_CENTER;
			}
			else {
				wFormat = (m_Pos & 1) ? DT_RIGHT : DT_LEFT;
			}
			break;
		default:
			wFormat = DT_LEFT;
			break;
	}
    return wFormat;
}
//---------------------------------------------------------------------------
UINT __fastcall CItem::GetDrawFormat(void)
{
	UINT wFormat = DT_NOPREFIX|DT_NOCLIP|DT_EXPANDTABS;
	if( m_Rot ){
		wFormat |= DT_LEFT;
    }
    else {
		wFormat |= GetPosFormat();
    }
	return wFormat;
}
//---------------------------------------------------------------------------
void __fastcall CItem::GetFont(LOGFONT *pLogfont)
{
	memset(pLogfont, 0, sizeof(LOGFONT));
	pLogfont->lfHeight = m_pFont->Height;
	pLogfont->lfWidth = 0;
	pLogfont->lfEscapement = m_Rot * 10;
	pLogfont->lfOrientation = m_Rot * 10;
	TFontStyles ts = m_pFont->Style;
	int fsw = FontStyle2Code(ts);
	pLogfont->lfWeight = fsw & FSBOLD ? 700 : 400;
	pLogfont->lfItalic = BYTE(fsw & FSITALIC ? TRUE : FALSE);
    pLogfont->lfUnderline = BYTE(fsw & FSUNDERLINE ? TRUE : FALSE);
    pLogfont->lfStrikeOut = BYTE(fsw & FSSTRIKEOUT ? TRUE : FALSE);
	pLogfont->lfCharSet = m_pFont->Charset;
	pLogfont->lfOutPrecision = OUT_CHARACTER_PRECIS;
	pLogfont->lfClipPrecision = CLIP_DEFAULT_PRECIS;
	pLogfont->lfQuality = NONANTIALIASED_QUALITY;
	pLogfont->lfPitchAndFamily = DEFAULT_PITCH;
	strcpy(pLogfont->lfFaceName, m_pFont->Name.c_str());
}
//---------------------------------------------------------------------------
void __fastcall CItem::SetFont(LOGFONT *pLogfont)
{
	if( m_pFont == NULL ) m_pFont = new TFont;
	m_pFont->Name = pLogfont->lfFaceName;
	m_pFont->Charset = pLogfont->lfCharSet;
	if( pLogfont->lfHeight ){
		m_pFont->Height = pLogfont->lfHeight;
    }
    else {
		m_pFont->Size = pLogfont->lfWidth;
    }
	DWORD d = 0;
	if( pLogfont->lfWeight > 600 ) d |= FSBOLD;
    if( pLogfont->lfItalic ) d |= FSITALIC;
    if( pLogfont->lfUnderline ) d |= FSUNDERLINE;
    if( pLogfont->lfStrikeOut ) d |= FSSTRIKEOUT;
	m_fsCode = d;
	TFontStyles ts = Code2FontStyle(d);
	m_pFont->Style = ts;
   	m_Rot = pLogfont->lfEscapement / 10;
}
//---------------------------------------------------------------------------
static TColor __fastcall GetIndexColor(int index)
{
	index++;
	UCOL c;
    c.d = 0;
    c.b.r = BYTE(index & 1 ? 255 : 0);
    c.b.g = BYTE(index & 2 ? 255 : 0);
    c.b.b = BYTE(index & 4 ? 255 : 0);
    return c.c;
}
//---------------------------------------------------------------------------
static int __fastcall GetColorIndex(UCOL c)
{
	int n = 0;
    if( c.b.r >= 0x80 ) n |= 1;
    if( c.b.g >= 0x80 ) n |= 2;
    if( c.b.b >= 0x80 ) n |= 4;
    n--;
    if( (n < 0) || (n >= 6) ) n = 0;
    return n;
}
//---------------------------------------------------------------------------
void __fastcall CItem::DrawText(RECT rc, Graphics::TBitmap *pDest, LPCSTR pText, UCOL ucol)
{
	if( m_Rot || (m_Ex & DISFONTSMOOTHING) || (m_Grade==4) ){
		LOGFONT lf;
#if 1
		GetFont(&lf);
#else
		memset(&lf, 0, sizeof(lf));
		lf.lfHeight = m_pFont->Height;
		lf.lfWidth = 0;
		lf.lfEscapement = m_Rot * 10;
		lf.lfOrientation = m_Rot * 10;
		TFontStyles ts = m_pFont->Style;
		int fsw = FontStyle2Code(ts);
		lf.lfWeight = fsw & FSBOLD ? 700 : 400;
		lf.lfItalic = BYTE(fsw & FSITALIC ? TRUE : FALSE);
        lf.lfUnderline = BYTE(fsw & FSUNDERLINE ? TRUE : FALSE);
        lf.lfStrikeOut = BYTE(fsw & FSSTRIKEOUT ? TRUE : FALSE);
		lf.lfCharSet = m_pFont->Charset;
		lf.lfOutPrecision = OUT_CHARACTER_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = NONANTIALIASED_QUALITY;
		lf.lfPitchAndFamily = DEFAULT_PITCH;
		strcpy(lf.lfFaceName, m_pFont->Name.c_str());
#endif
		pDest->Canvas->Font->Handle = CreateFontIndirect(&lf);
	}
	else {
		pDest->Canvas->Font = m_pFont;
	}
	pDest->Canvas->Font->Color = ucol.c;

	int nc = 0;
	if( m_OYWC || m_Rot || (m_Grade == 4) ){
		TStringList *pList = new TStringList;
		pList->Text = pText;
        UINT wFormat = GetPosFormat();
		int xw = rc.right - rc.left;
        for( int i = 0; i < pList->Count; i++ ){
			RECT trc;
            trc = rc;
			int txw = pDest->Canvas->TextWidth(pList->Strings[i].c_str());
			if( m_Rot ){		// 回転時の位置補正
                int x;
				switch(wFormat){
					case DT_CENTER:
                        x = (xw - txw)/2;
                        trc.left += x * m_CosX;
                        trc.right += x * m_CosX;
                        trc.top -= x * m_SinX;
                    	break;
                    case DT_RIGHT:
                    	x = xw - txw;
                        trc.left += x * m_CosX;
                        trc.right += x * m_CosX;
                        trc.top -= x * m_SinX;
                    	break;
                }
            }
			DrawText_(trc, pDest, pList->Strings[i].c_str(), nc);
			if( m_Rot ){
                rc.left += (m_FW + m_OYWC) * m_SinX;
                rc.right += (m_FW + m_OYWC) * m_SinX;
                rc.top += (m_FW + m_OYWC) * m_CosX;
            }
            else {
	            rc.top += (m_FW + m_OYWC);
            }
        }
        delete pList;
    }
    else {
		DrawText_(rc, pDest, pText, nc);
    }
}
//---------------------------------------------------------------------------
void __fastcall CItem::DrawText_(RECT rc, Graphics::TBitmap *pDest, LPCSTR pText, int &nc)
{
	TCanvas *pCanvas = pDest->Canvas;
	HDC hdc = pCanvas->Handle;
	::SetBkMode(hdc, TRANSPARENT);
	if( (m_Grade == 4) && (m_pMaskBmp == NULL) ){
		double x = rc.left;
        double y = rc.top;
		char bf[32];
        int fw = TextWidth(pCanvas, pText);
		if( !m_Rot ){
	        int pos = GetPosFormat();
    	    switch(pos){
				case DT_CENTER:
					fw = ((rc.right-rc.left)-fw)/2;
					x += fw;
	            	break;
	            case DT_RIGHT:
					fw = ((rc.right-rc.left)-fw);
					x += fw;
    	        	break;
	        }
        }
		while(*pText){
			if( (_mbsbtype((const unsigned char *)pText, 0) == _MBC_LEAD) && *(pText+1)  ){
				bf[0] = *pText++; bf[1] = *pText++; bf[2] = 0;
			}
            else {
				bf[0] = *pText++; bf[1] = 0;
            }
            pCanvas->Font->Color = GetIndexColor(nc);
			fw = pCanvas->TextWidth(bf);
//			rc.left = x; rc.top = y;
			pCanvas->TextOut(x, y, bf);
			if( m_Rot ){
				x += fw * m_CosX;
				y -= fw * m_SinX;
        	}
            else {
	            x += fw;
            }
			if( (bf[0] != ' ') && strcmp(bf, "　") ){
	            nc++;
    	        if( nc >= AN(m_ColF) ) nc = 0;
            }
        }
    }
	else if( m_Row >= 2 ){
		::DrawText(hdc, pText, strlen(pText), &rc, GetDrawFormat());
    }
    else {
		pCanvas->TextOut(rc.left, rc.top, pText);
    }
}
#if 0
//---------------------------------------------------------------------------
void __fastcall CItem::DrawTrans(Graphics::TBitmap *pDest, Graphics::TBitmap *pSrc, LPBYTE pTbl)
{
	int xw = pSrc->Width;
    int yw = pSrc->Height;
    LPBYTE sp, tp;
	int x, y;
	for( y = 0; y < yw; y++ ){
		sp = (LPBYTE)pSrc->ScanLine[y];
        tp = (LPBYTE)pDest->ScanLine[y];
		for( x = 0; x < xw; x++, pTbl++ ){
			if( *pTbl ){
				*tp++ = *sp++;
                *tp++ = *sp++;
                *tp++ = *sp++;
            }
            else {
				tp += 3;
                sp += 3;
            }
        }
    }
}
#endif
//---------------------------------------------------------------------------
void __fastcall CItem::CreateTable(Graphics::TBitmap *pDest)
{
	int XW = pDest->Width;
	int YW = pDest->Height;
	m_XW = XW;
    m_YW = YW;

	LPBYTE pTbl = new BYTE[XW*YW];
	memset(pTbl, 0, XW*YW);
	LPBYTE wp = pTbl;
	int x, y;

	LPBYTE *pAdr = new BYTE*[YW];
	BYTE    br = m_cBack.b.r;
	BYTE    bg = m_cBack.b.g;
	BYTE    bb = m_cBack.b.b;
	if( m_Grade != 4 ){
		for( y = 0; y < YW; y++ ){
			LPBYTE p = (LPBYTE)pDest->ScanLine[y];
        	pAdr[y] = p;
			for( x = 0; x < XW; x++, wp++, p+=3 ){
				if( (bb != *p) || (bg != *(p+1)) || (br != *(p+2)) ){
					*wp = 0x80;
				}
			}
		}
    }
    else {
		UCOL c;
        c.d = 0;
		for( y = 0; y < YW; y++ ){
			LPBYTE p = (LPBYTE)pDest->ScanLine[y];
        	pAdr[y] = p;
			for( x = 0; x < XW; x++, wp++ ){
				c.b.b = *p++;
                c.b.g = *p++;
                c.b.r = *p++;
				if( (bb != c.b.b) || (bg != c.b.g) || (br != c.b.r) ){
					*wp = BYTE(0x80 + GetColorIndex(c));
				}
			}
		}
    }
	m_pAdr = pAdr;
    m_pTbl = pTbl;
}
//---------------------------------------------------------------------------
int __fastcall CItem::EditDialog(HWND hWnd)
{
	TTextDlgBox *pBox = new TTextDlgBox(NULL);
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
	int hOrg = pDlg->Font->Height;
    int h = hOrg / 4;
	pDlg->Font->Height = h;
	pDlg->Font->Color = m_ColG[0].c;
	MultProc();
	int r = pDlg->Execute();
	MultProc();
	if( r == TRUE ){
		m_pFont->Assign(pDlg->Font);
		if( h != m_pFont->Height ){
			hOrg = m_pFont->Height * 4;
        }
		m_pFont->Height = hOrg;
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
void __fastcall CItem::Draw3D(UCOL cl[2])
{
	int xm = m_3d & 0x00ff;
	int ym = (m_3d >> 8) & 0x00ff;
	if( xm & 0x0080 ) xm |= 0xffffff80;
	if( ym & 0x0080 ) ym |= 0xffffff80;
    if( m_Rot ){
		int x = xm * m_CosX + ym * m_SinX;
        int y = -xm * m_SinX + ym * m_CosX;
		xm = x;
        ym = y;
    }

	int max = ABS(xm) > ABS(ym) ? ABS(xm) : ABS(ym);
	int ax, ay;
    ax = ay = 32768;
	for( int i = max; i > 0; i-- ){
		int x = xm * i / max;
		int y = ym * i / max;
		if( (ax != x) || (ay != y) ){
			DrawColorMask(x, y, GetGrade2(cl, i, max));
            ax = x; ay = y;
        }
	}
}
//---------------------------------------------------------------------------
void __fastcall CItem::DrawColorMask(int X, int Y, UCOL col)
{
	ASSERT(m_pTbl); ASSERT(m_pAdr);

	int xw = m_XW;
    int yw = m_YW;
	int x, y, xx, yy;
	LPBYTE pTbl = m_pTbl;
    LPBYTE *pAdr = m_pAdr;
	LPBYTE wp;
	yy = Y;
    for( y = 0; y < yw; y++, yy++ ){
		if( yy < 0 ){
			pTbl += xw;
        	continue;
        }
        else if( yy >= yw ){
			break;
        }
		wp = (LPBYTE)pAdr[yy] + (X*3);
        xx = X;
		for( x = 0; x < xw; x++, xx++, pTbl++ ){
			if( *pTbl && (xx>=0) && (xx<xw) ){
				*wp++ = col.b.b;
       	        *wp++ = col.b.g;
           	    *wp++ = col.b.r;
            }
   	        else {
				wp += 3;
            }
   	    }
    }
}
//---------------------------------------------------------------------------
Graphics::TBitmap* __fastcall CItem::DrawExt(Graphics::TBitmap *pDest)
{
	ASSERT(m_pTbl); ASSERT(m_pAdr);
	MultProc();

	int xw = m_XW;
    int yw = m_YW;
    FillBitmap(pDest, m_cBack);

	LPBYTE wp, p;
	MultProc();
	switch(m_Shadow){
		case 1:
			{
				if( m_3d & 0x00010000 ){
					Draw3D(&m_ColB[1]);
				}
				else {
					for( int i = m_Line - 1; i >= 0; i-- ){
						DrawColorMask(i+1, i+1, m_ColB[0]);
					}
				}
			}
			break;
		case 2:
			{
				ASSERT(m_pTbl); ASSERT(m_pAdr);
				if( m_3d & 0x00010000 ){
					Draw3D(&m_ColB[1]);
				}
				int f;
				int x, y;
				for( y = 0; y < yw; y++ ){
					wp = &m_pTbl[y*xw];
					p = m_pAdr[y];
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
							*p++ = m_ColB[0].b.b;
							*p++ = m_ColB[0].b.g;
							*p = m_ColB[0].b.r;
							p -= 2;
							*wp = BYTE((m_Line >= 2) ? 2 : (f ? 2 : 0));
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
							*p++ = m_ColB[0].b.b;
							*p++ = m_ColB[0].b.g;
							*p = m_ColB[0].b.r;
							p -= 2;
							*wp = BYTE((m_Line >= 2) ? 2 : (f ? 2 : 0));
						}
					}

				}
				int ax = 0;
				for( x = 0; x < xw; x++, ax+=3 ){
					wp = &m_pTbl[x];
					f = 0;
					for( y = 0; y < yw; y++, wp += xw ){
						if( *wp ){
							f = m_Line;
						}
						else if( f ){
							f--;
							p = m_pAdr[y];
							p += ax;
							*p++ = m_ColB[0].b.b;
							*p++ = m_ColB[0].b.g;
							*p = m_ColB[0].b.r;
							*wp = BYTE((m_Line >= 2) ? 2 : (f ? 2 : 0));
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
							p = m_pAdr[y];
							p += ax;
							*p++ = m_ColB[0].b.b;
							*p++ = m_ColB[0].b.g;
							*p = m_ColB[0].b.r;
						}
					}
				}
			}
			break;
		case 3:
			{
				if( m_3d & 0x00010000 ){
					int xm = m_3d & 0x00ff;
					int ym = (m_3d >> 8) & 0x00ff;
					Draw3D(&m_ColB[1]);
                    DrawColorMask((xm & 0x80) ? 1 : -1, (ym & 0x80) ? 1 : -1, m_ColB[0]);
				}
				else {
					for( int i = m_Line - 1; i >= 0; i-- ){
						DrawColorMask(i+1, i+1, m_ColB[0]);
					}
					DrawColorMask(-1, -1, m_ColB[1]);
				}
			}
			break;
		default:
			break;
	}

		MultProc();
		int x, y;
		RECT mc = m_rc;
		int bw = mc.right - mc.left + 1;
		int bh = mc.bottom - mc.top + 1;
		int bd = sqrt(bw*bw + bh*bh);
		UCOL    c;
        int ao = mc.left * 3;
		int yy = mc.top;

		int Grade = m_Grade;
		int maskxw, maskyw;
		LPBYTE sp;
        if( m_pMaskBmp ){
			maskxw = m_pMaskBmp->Width;
            maskyw = m_pMaskBmp->Height;
            Grade = 5;
        }
		for( y = 0; y < bh; y++, yy++ ){
			p = m_pAdr[yy] + ao;
			wp = &m_pTbl[yy*xw] + mc.left;
            if( Grade == 5 ){
				sp = (LPBYTE)m_pMaskBmp->ScanLine[y % maskyw];
            }
			for( x = 0; x < bw; x++, wp++ ){
				if( *wp & 0x80 ){
					switch(Grade){
						case 0:
                        	c = m_ColG[0];
                            break;
						case 1:
							c = GetGrade4(m_ColG, x, bw);
							break;
						case 2:
							c = GetGrade4(m_ColG, y, bh);
							break;
						case 3:
							c = GetGrade4(m_ColG, sqrt(x*x + y*y), bd);
							break;
						case 4:
							c = m_ColF[*wp & 0x07];
							break;
						case 5:
							{
								LPBYTE s = sp + ((x % maskxw) * 3);
                                c.b.b = *s++;
                                c.b.g = *s++;
                                c.b.r = *s;
                            }
                            break;
					}
					*p++ = c.b.b;
					*p++ = c.b.g;
					*p++ = c.b.r;
				}
                else {
					p += 3;
                }
			}
		}

    if( m_pAdr != NULL ) delete m_pAdr;
	if( m_pTbl != NULL ) delete m_pTbl;

	return pDest;
}
//---------------------------------------------------------------------------
Graphics::TBitmap* __fastcall CItem::AdjustMinimized(Graphics::TBitmap *pSrc)
{
	int xw = pSrc->Width;
    int yw = pSrc->Height;
	int x, y;
    RECT rc;
	rc.left = xw; rc.right = 0;
   	rc.top = yw; rc.bottom = 0;
	BYTE    br = m_cBack.b.r;
	BYTE    bg = m_cBack.b.g;
	BYTE    bb = m_cBack.b.b;
	for( y = 0; y < xw; y++ ){
		LPBYTE p = (LPBYTE)pSrc->ScanLine[y];
		for( x = 0; x < yw; x++, p+=3 ){
			if( (bb != *p) || (bg != *(p+1)) || (br != *(p+2)) ){
				if( rc.left > x ) rc.left = x;
				if( rc.right < x ) rc.right = x;
				if( rc.top > y ) rc.top = y;
				if( rc.bottom < y ) rc.bottom = y;
			}
        }
	}
	xw = rc.right - rc.left + 1;
    yw = rc.bottom - rc.top + 1;
	if( xw <= 0 ) xw = 1;
    if( yw <= 0 ) yw = 1;
	Graphics::TBitmap *pbmp = CreateBitmap(xw, yw);
	pbmp->Canvas->Draw(-rc.left, -rc.top, pSrc);
    delete pSrc;
    return pbmp;
}
//
//
//---------------------------------------------------------------------------
// Matrix
//
//  n[] = l[] * m[]
//
static void __fastcall Matrix(double l[4][4], double m[4][4], double n[4][4])
{
	int i, j, k;
	double  d;

	for( i = 0; i < 4; i++ ){
		for( j = 0; j < 4; j++ ){
			for( k = 0, d = 0.0; k < 4; k++ ) d += l[i][k] * m[k][j];
			n[i][j] = d;
		}
	}
}
static void __fastcall Matrix(double l[3][3], double m[3][3], double n[3][3])
{
	int i, j, k;
	double  d;

	for( i = 0; i < 3; i++ ){
		for( j = 0; j < 3; j++ ){
			for( k = 0, d = 0.0; k < 3; k++ ) d += l[i][k] * m[k][j];
			n[i][j] = d;
		}
	}
}
//---------------------------------------------------------------------------
// Perspect
//
void __fastcall SetMatrix(double k[3][3], LPCSPERSPECT pPar, int xs, int ys)
{
	double  l[4][4]; double m[4][4]; double n[4][4];
	double  k1, k2, k3, k4, k5, k6, k7, k8, k9;
	double  u, v, w;

	u = pPar->rx * PI / 180;
	v = pPar->ry * PI / 180;
	w = pPar->rz * PI / 180;
	memset(l, 0, sizeof(l)); memset(m, 0, sizeof(m));
	l[0][0] = 1.0/xs;   l[1][1] = -1.0/xs;  l[2][2] = 1.0;  l[3][3] = 1.0;
	m[0][0] = pPar->ax; m[1][1] = pPar->ay; m[2][2] = 1.0;  m[3][3] = 1.0;
	Matrix(l, m, n);
	memset(l, 0, sizeof(l));
	l[0][0] = 1.0;      l[1][1] = 1.0;      l[2][2] = 1.0;
	l[3][0] = pPar->px; l[3][1] = pPar->py; l[3][2] = pPar->pz; l[3][3] = 1.0;
	Matrix(n, l, m);
	memset(n, 0, sizeof(n));
	n[0][0] = cos(w);   n[0][1] = sin(w);
	n[1][0] = -n[0][1]; n[1][1] = n[0][0];
	n[2][2] = 1.0;      n[3][3] = 1.0;
	Matrix(m, n, l);
	memset(m, 0, sizeof(m));
	m[0][0] = 1.0;      m[1][1] = cos(u);   m[1][2] = sin(u);
	m[2][1] = -m[1][2]; m[2][2] = m[1][1];  m[3][3] = 1.0;
	Matrix(l, m, n);
	memset(l, 0, sizeof(l));
	l[0][0] = cos(v);   l[0][2] = sin(v);   l[1][1] = 1.0;
	l[2][0] = -l[0][2]; l[2][2] = l[0][0];  l[3][3] = 1.0;
	Matrix(n, l, m);
	memset(n, 0, sizeof(n));
	n[0][0] = 1.0;      n[1][1] = 1.0;      n[2][2] = -1.0; n[3][2] = -pPar->v; n[3][3] = 1.0;
	Matrix(m, n, l);
	memset(m, 0, sizeof(m));
	m[0][0] = 1.0;      m[1][1] = 1.0;      m[2][2] = 1/pPar->s;    m[2][3] = m[2][2];
	m[3][2] = -1.0;
	Matrix(l, m, n);
	memset(l, 0, sizeof(l));
	l[0][0] = xs;       l[1][1] = -xs;      l[2][2] = 1.0;      l[3][3] = 1.0;
	Matrix(n, l, m);

	k1 = m[0][3];   k2 = m[1][3];   k3 = m[3][3];
	k4 = m[0][0];   k5 = m[1][0];   k6 = m[3][0];
	k7 = m[0][1];   k8 = m[1][1];   k9 = m[3][1];

	k[0][0] = k7*k2 - k8*k1;   k[0][1] = k5*k1 - k4*k2;   k[0][2] = k4*k8 - k7*k5;
	k[1][0] = k8*k3 - k9*k2;   k[2][0] = k9*k1 - k7*k5;   k[1][1] = k6*k2 - k5*k3;
	k[2][1] = k4*k3 - k6*k1;   k[1][2] = k5*k9 - k8*k6;   k[2][2] = k7*k6 - k4*k9;

	if( pPar->r != 0.0 ){       // simple rotation
		double r = pPar->r * PI / 180.0;
		double  m[3][3];
		double  z[3][3];
		memset(m, 0, sizeof(m));
		m[0][0] = cos(r); m[0][1] = sin(r);
		m[1][0] = -m[0][1]; m[1][1] = m[0][0];
		m[2][2] = 1.0;
		Matrix(k, m, z);
		memcpy(k, z, sizeof(z));
	}
}
#define TMAX    400
//---------------------------------------------------------------------------
// Create mapping table
LPCDOUBLE __fastcall MapTable(int wave, int xs, int ys, double ks)
{
#define VMAX    32767.0
#define VMIN    (1.0/VMAX)
	LPDOUBLE  pTbl, dp;
	double d;
	switch(wave){
		case 6:
			{
				double  ky = 0.5 * PI / ys;
				dp = pTbl = new double[ys*2];
				for( int y = -ys; y < ys; y++, dp++ ){
					d = cos(y * ky);
					if( ABS(d) >= VMIN ){
						d = 1.0 * ks / d;
					}
					else {
						d = VMAX;
					}
					*dp = d * 0.8;
				}
			}
			break;
		case 9:
			{
				dp = pTbl = new double[xs*2];
				double kd = xs * xs;
				for( int x = -xs; x < xs; x++, dp++ ){
					d = kd - x * x;
					if( d >= 0 ){
						d = sqrt(d);
						*dp = (d > VMIN) ? xs * ks / d : VMAX;
					}
					else {
						*dp = 1.0;
					}
				}
			}
			break;
		case 14:
			{
				dp = pTbl = new double[ys*2];
				double kd = ys * ys;
				for( int y = -ys; y < ys; y++, dp++ ){
					d = kd - y * y;
					if( d >= 0 ){
						d = sqrt(d);
						*dp = (d > VMIN) ? ys / d : VMAX;
					}
					else {
						*dp = 1.0;
					}
				}
			}
			break;
		case 10:
		case 11:
			{
				dp = pTbl = new double[xs*2];
				double kd = xs * xs;
				double kc = xs * 0.25;
				for( int x = -xs; x < xs; x++, dp++ ){
					d = kd - x * x;
					if( d >= 0 ){
						*dp = sqrt(d) * 0.5 * ks - kc;
						if( wave == 10 ) *dp = -*dp;
					}
					else {
						*dp = 1.0;
					}
				}
			}
			break;
		case 7:
		case 8:
		case 12:
		case 13:
		case 15:
        case 25:
			pTbl = NULL;
			break;
		case 16:
			{
				dp = pTbl = new double[ys*2];
				double kc = ks * PI /(4 * ys);
				double d;
				for( int y = -ys; y < ys; y++, dp++ ){
					d = fabs(sin((y + ys)*kc));
					*dp = d > VMIN ? 1.0 / d : VMAX;
				}
			}
			break;
		case 17:
			{
				dp = pTbl = new double[ys*2];
				double kc = ks * PI /(4 * ys);
				double d;
				for( int y = ys-1; y >= -ys; y--, dp++ ){
					d = fabs(sin((y + ys)*kc));
					*dp = d > VMIN ? 1.0 / d : VMAX;
				}
			}
			break;
		case 22:
			{
				dp = pTbl = new double[ys*2];
				double kc = PI * ks/ (ys * ys);
				double d;
				for( int y = -ys; y < ys; y++, dp++ ){
					d = y * y * kc + (PI/4);
					*dp = ABS(d) >= VMIN ? (1.0 / d) : VMAX;
				}
			}
			break;
        case 28:
        case 29:
			{
				dp = pTbl = new double[xs*2];
				double kd = xs * xs;
                double kc = ks*0.5 + 0.05;
				for( int x = -xs; x < xs; x++, dp++ ){
					d = kd - x * x;
					if( d >= 0 ){
						d = sqrt(d);
						*dp = (d > VMIN) ? xs * kc / d : VMAX;
					}
					else {
						*dp = 1.0;
					}
				}
			}
			break;
		case 36:
			{
				dp = pTbl = new double[xs*2];
				double kx = PI * 0.5 / xs;
				double ky = ys * ks * 0.125;
                double ko = ks * ys * 0.1875;
				for( int x = -xs; x < xs; x++, dp++ ){
					*dp = ky * ((xs-x) * kx) - ko;
				}
			}
			break;
		case 38:
			{
				dp = pTbl = new double[ys*2];
				double ky = PI * 0.5 / ys;
				double kx = xs * ks * 0.25;
                double ko = ks * xs * 0.375;
				for( int y = -ys; y < ys; y++, dp++ ){
					*dp = kx * ((ys-y) * ky) - ko;
				}
			}
			break;
		default:
			{
				double  kx = PI / xs;
				double  ky = ys * ks/ 8;
				dp = pTbl = new double[xs*2];
				for( int x = -xs; x < xs; x++, dp++ ){
					switch(wave){
						case 1:
							*dp = ky * sin(x * kx);
							break;
						case 2:
							*dp = ky * sin(-x * kx);
							break;
						case 3:
							*dp = ky * sin(x * kx + PI/2);
							break;
						case 4:
							*dp = ky * sin(-x * kx - PI/2);
							break;
						case 5:
							d = cos(x * kx * 0.5);
							if( ABS(d) >= VMIN ){
								d = 1.0 * ks/ d;
							}
							else {
								d = VMAX;
							}
							*dp = d * 0.8;
							break;
                        case 18:
							d = ABS(x) * kx + (PI/2) * ks;
							if( ABS(d) >= VMIN ){
								d = PI / d;
							}
							else {
								d = VMAX;
							}
							*dp = d * 0.8;
							break;
                        case 19:
                        case 34:
                        case 35:
							d = fabs(sin(x * kx)) + 0.25 * ks;
							if( ABS(d) >= VMIN ){
								d = 1.0 / d;
							}
							else {
								d = VMAX;
							}
							*dp = d * 0.8;
							break;
                        case 20:
                        case 30:
                        case 32:
							d = (x+xs) * kx * 0.5 + (PI/2) * ks;
							if( ABS(d) >= VMIN ){
								d = PI / d;
							}
							else {
								d = VMAX;
							}
							*dp = d * 0.8;
							break;
                        case 21:
                        case 31:
                        case 33:
							d = (xs-x) * kx * 0.5 + (PI/2) * ks;
							if( ABS(d) >= VMIN ){
								d = PI / d;
							}
							else {
								d = VMAX;
							}
							*dp = d * 0.8;
							break;
                        case 23:
							d = x * x * kx * kx + (PI * ks);
							*dp = ABS(d) >= VMIN ? (PI * 2.0 / d) : VMAX;
							break;
                        case 24:
                        case 26:
                        case 27:
							d = (xs-ABS(x)) * kx + (PI/2) * ks;
							*dp = ABS(d) >= VMIN ? (PI * 0.8 / d) : VMAX;
                        	break;
						case 37:
							*dp = ky * (ABS(x) * kx * 0.5);
							break;
   						default:
							break;
					}
				}
			}
			break;
	}
	return pTbl;
}
//---------------------------------------------------------------------------
void __fastcall CItem::Perspect(void)
{
    if( (m_XW <= 2) && (m_YW <= 2) ) return;

	LPCSPERSPECT pPar = &m_sperspect;

    LPBYTE pDest = new BYTE[m_XW*m_YW];
    LPBYTE pSrc = m_pTbl;
#if MeasureAccuracy
	QueryPerformanceCounter(&m_liPOldPer);
#endif
	int wave = pPar->flag;
	int bw = m_YW;
	int bh = m_XW;
	int i, j, m, n;
	double  x, y, w, p, q;
	double  k[3][3];
	int xs = bw / 2;
	int ys = bh / 2;
	SetMatrix(k, pPar, xs, ys);

	LPCDOUBLE pTbl = NULL;
	double ks = 1.0 + (double(m_SC) * 0.05);
	if( wave ) pTbl = MapTable(wave, xs, ys, ks);
	double kc, kc3, kx, kc7;
	switch(wave){
		case 7:
		case 8:
			kc7 = ((wave == 7) ? 0.6 : 0.8)*double(ys)/double(xs);
			kx = xs * ((wave == 7) ? 4.0 * ks : 2.994) / PI;
			kc3 = ys/3;
			if( wave == 8 ){
				kc7 *= ks;
				kc3 *= ks;
			}
			break;
		case 13:
			if( m_SC >= (88-30) ) m_SC = 88-30;
			if( m_SC <= -30 ) m_SC = -29;
			kc = 30 + m_SC;
			kc3 = 1.0/tan(kc*PI/180);
			kx = 1.0/cos(kc*PI/180);
			kc7 = 1.0/sin(kc*PI/180);
			break;
		case 14:
			kc3 = ys * 7 /8;
			break;
		case 15:
			kc3 = ks + 0.75;
			break;
        case 25:
			kc7 = 0.6*double(ys)/double(xs);
			kx = xs * (4.0 * ks) / PI;
			kc3 = ys/3;
			break;
        case 26:
        case 27:
        case 34:
        case 35:
			kc3 = (ABS(ks) > VMIN) ? (1.0 / ks) : VMAX;
           	break;
        case 28:
        case 29:
			kc3 = (ABS(ks) > VMIN) ? (ks) : VMAX;
            break;
        case 30:
        case 31:
        case 32:
        case 33:
			kc3 = (ABS(ks) > VMIN) ? (0.8 / ks) : VMAX;
            break;
	}
	kc = double(xs)/double(ys);
#if MeasureAccuracy
	if( QueryPerformanceCounter(&m_liPCurPer) ){
		m_dlDiffPer1 = m_liPCurPer.QuadPart - m_liPOldPer.QuadPart;
		m_liPOldPer = m_liPCurPer;
	}
#endif
	LPBYTE tp = pDest;
	for( i = -ys; i < ys; i++ ){
		for( j = -xs; j < xs; j++ ){
			w = k[0][0] * j + k[0][1] * i + k[0][2];
			x = k[1][0] * j + k[1][1] * i + k[1][2];
			y = k[2][0] * j + k[2][1] * i + k[2][2];
			if( w ){
				x = x / w;
				y = y / w;
            }
            else {
				x = y = 10000.0;
            }
			switch(wave){
				case 1:
				case 2:
				case 3:
				case 4:
					ASSERT(pTbl);
					m = x + xs;
					if( m >= bw ) m = bw-1;
					if( m < 0 ) m = 0;
					y += pTbl[m];
					break;
				case 5:
					ASSERT(pTbl);
//							x *= 0.8;
					m = x + xs;
					if( m >= bw ) m = bw-1;
					if( m < 0 ) m = 0;
					y *= pTbl[m];
					break;
				case 6:
					ASSERT(pTbl);
					y *= 0.8;
					m = y + ys;
					if( m >= bh ) m = bh-1;
					if( m < 0 ) m = 0;
					x *= pTbl[m];
					break;
				case 7:
				case 8:
					if( wave == 7 ){
						y -= ys;
					}
					x *= kc7;
					y *= kc7;
					q = -y * kc;
					w = (y||x) ? atan2(x, q) : 0.0;
					p = sqrt(q * q + x * x);
					x = w * kx;
					if( p >= kc3 ){
						if( wave == 8 ){
							x += xs;
							if( x < 0 ) x += 2*xs;
							x = fmod(x, 2*xs-1);
							x -= xs;
						}
						p = (p - kc3) * 3;
						y = (-(p) + ys);
					}
					else {
						y = ys;
					}
					break;
				case 9:
					ASSERT(pTbl);
					m = x + xs;
					if( m >= bw ) m = bw-1;
					if( m < 0 ) m = 0;
					y *= pTbl[m];
					break;
				case 10:
				case 11:
                case 36:
                case 37:
					ASSERT(pTbl);
					m = x + xs;
					if( m >= bw ) m = bw-1;
					if( m < 0 ) m = 0;
					y += pTbl[m];
					break;
				case 12:
					x *= 0.8;
					p = (y+ys)*0.5;
					w = xs * xs - x * x;
					if( w >= 0 ){
						w = sqrt(w) * ks/2.0 - xs*0.25;
						y -= w;
					}
					else {
						y = ys;
					}

					w = ys * ys - p * p;
					if( w > 0 ){
						w = ys / sqrt(w);
						x *= w;
					}
					else {
						x = xs;
					}
					break;
				case 13:
					x += xs / 3;
					y += ys / 3;
					x *= 0.75;
					y *= 0.75;
					if( (y >= 0) && (x >= 0) ){   // C
						y = y * 2 - ys;
						x = x * 2 - xs;
					}
					else if( (-y*kc3 >= -x) ){     // A
						x = (x - y*kc3) * 2  - xs;
						y = y * kc7 * 3 + ys;
					}
					else {                        // B
						y = (y - x/kc3) * 2 - ys;
						x = x * kx * 3 * kc + xs;
					}
//                            m = int(y); n = int(x);
//                            if( (n ==(-xs+1))||(n==(xs-1))||(m==(-ys+1))||(m==(ys-1)) ){
//                                x = xs-1; y = ys-1;
//                            }
					break;
				case 14:
					y += kc3;
					w = (x||y) ? atan2(x, y*kc) : 0.0;
					p = fabs(ks + cos(w));
					p = (p > VMIN) ? 1.0 / p : VMAX;
					y *= p;
					ASSERT(pTbl);
					m = y + ys;
					if( m >= bh ) m = bh-1;
					if( m < 0 ) m = 0;
					x *= p * pTbl[m];
					break;
				case 15:
					x *= 1.7; y *= 1.7;
					p = fabs(x); q = fabs(y);
					w = (x||y) ? atan2(p, q*kc) : 0.0;
					p = 1 + sin(w * kc3);
					p = (p > VMIN) ? 1.0 / p : VMAX;
					x *= p;
					y *= p;
					break;
				case 16:
				case 17:
                case 22:
					ASSERT(pTbl);
					m = y + ys;
					if( m >= bh ) m = bh-1;
					if( m < 0 ) m = 0;
					x *= pTbl[m];
					break;
				case 18:
                case 19:
                case 20:
                case 21:
				case 23:
                case 24:
					ASSERT(pTbl);
					m = x + xs;
					if( m >= bw ) m = bw-1;
					if( m < 0 ) m = 0;
					y *= pTbl[m];
					break;
                case 25:
					y += ys;
					x *= kc7;
					y *= kc7;
					q = y * kc;
					w = (y||x) ? atan2(x, q) : 0.0;
					p = sqrt(q * q + x * x);
					x = w * kx;
					if( p >= kc3 ){
						p = (p - kc3) * 3;
						y = p - ys;
					}
					else {
						y = -ys;
					}
					break;
                case 26:
                case 28:
                case 30:
                case 31:
                case 34:
					ASSERT(pTbl);
                    if( y < 0 ){
						m = x + xs;
						if( m >= bw ) m = bw-1;
						if( m < 0 ) m = 0;
						y *= pTbl[m];
                    }
                    else {
						y *= kc3;
                    }
					break;
                case 27:
                case 29:
                case 32:
                case 33:
                case 35:
					ASSERT(pTbl);
                    if( y > 0 ){
						m = x + xs;
						if( m >= bw ) m = bw-1;
						if( m < 0 ) m = 0;
						y *= pTbl[m];
                    }
                    else {
						y *= kc3;
                    }
					break;
                case 38:
					ASSERT(pTbl);
					m = y + ys;
					if( m >= bh ) m = bh-1;
					if( m < 0 ) m = 0;
					x += pTbl[m];
					break;
				default:
					break;
			}
			if( y > 0 ){ m = int(y); } else { m = int(y-1); }
			if( x > 0 ){ n = int(x); } else { n = int(x-1); }
#if 0
			double q = y - m;
			double p = x - n;
			double pp = 1.0 - p;
			double qq = 1.0 - q;
			if( (m >= -ys) && (m < ys-1) && (n >= -xs) && (n < xs-1) ){
				LPBYTE sp1 = &pSrc[((m+ys)*bw) + n + xs];
				LPBYTE sp2 = &pSrc[((m+ys+1)*bw) + n + xs];
				if( (*sp1 != *sp2) || (*sp1 != *(sp1+3)) || (*sp1 != *(sp2+3)) ){
					int b = (int)(qq*(pp*(*sp1) + p*(*(sp1+3)))
						+ q * (pp*(*sp2) + p*(*(sp2+3))));
                    *tp++ = b >= 0x40 ? 0x80 : 0;
				}
                else {
					*tp++ = *sp1;
                }
			}
			else {
				*tp++ = 0;
			}
#else
			if( (m >= -ys) && (m < ys-1) && (n >= -xs) && (n < xs-1) ){
				*tp++ = pSrc[((m+ys)*bw) + n + xs];
			}
			else {
				*tp++ = 0;
			}
#endif
		}
	}
    if( pTbl ) delete pTbl;
#if MeasureAccuracy
	if( QueryPerformanceCounter(&m_liPCurPer) ){
		m_dlDiffPer2 = m_liPCurPer.QuadPart - m_liPOldPer.QuadPart;
		m_liPOldPer = m_liPCurPer;
	}
#endif
	{
	    LPBYTE sp = pDest;
        RECT mc;
	    mc.left = m_XW; mc.right = 0;
    	mc.top = m_YW; mc.bottom = 0;
        int x, y;
		for( y = 0; y < m_YW; y++ ){
			for( x = 0; x < m_XW; x++, sp++ ){
				if( *sp ){
					if( mc.left > x ) mc.left = x;
					if( mc.right < x ) mc.right = x;
					if( mc.top > y ) mc.top = y;
					if( mc.bottom < y ) mc.bottom = y;
            	}
        	}
    	}
        m_rc = mc;
	}
	if( m_Filter & 0x30000 ){
		MFilter(pDest);
    }
    else {
		delete m_pTbl;
    	m_pTbl = pDest;
    }
}
//---------------------------------------------------------------------------
// メディアンフィルタ
void __fastcall CItem::MFilter(LPBYTE pSrc)
{
	int XW = m_XW;
    int YW = m_YW;
	int x, y;
	LPBYTE sp, tp, rp;
    RECT mc = m_rc;
	if( !mc.left ) mc.left++;
    if( !mc.top ) mc.top++;
	LPBYTE pDest = m_pTbl;
    memset(pDest, 0, XW*YW);
    int n;
    int count = m_Filter & 0x00ff;
    int sw = (m_Filter >> 16) - 1;
	for( int i = 0; i <= count; i++ ){
		if( i ){
			if( pSrc != m_pTbl ){
				pDest = pSrc;
                pSrc = m_pTbl;
            }
            else {
            	pSrc = pDest;
                pDest = m_pTbl;
            }
        	memset(pDest, 0, XW*YW);
    	}
		if( m_Grade == 4 ){
			LPBYTE zp;
			if( sw ){
				for( y = mc.top; y <= mc.bottom; y++ ){
					sp = &pSrc[y*XW+mc.left];
					tp = &pDest[y*XW+mc.left];
					for( x = mc.left; x <= mc.right; x++, sp++, tp++ ){
						n = 0;
						rp = sp - 1;
	    				if( *rp ){ n++; zp = rp;}
						if( *++rp ){ n++; zp = rp;}
						if( *++rp ){ n++; zp = rp;}
						rp = sp - XW - 1;
	    				if( *rp ){ n++; zp = rp;}
						if( *++rp ){ n++; zp = rp;}
						if( *++rp ){ n++; zp = rp;}
						rp = sp + XW - 1;
	    				if( *rp ){ n++; zp = rp;}
						if( *++rp ){ n++; zp = rp;}
						if( *++rp ){ n++; zp = rp;}
						if( n >= 5 ){
                    		*tp = *zp;
                    	}
					}
				}
			}
			else {
				for( y = mc.top; y <= mc.bottom; y++ ){
					sp = &pSrc[y*XW+mc.left];
					tp = &pDest[y*XW+mc.left];
					for( x = mc.left; x <= mc.right; x++, sp++, tp++ ){
						n = 0;
						rp = sp - 1;
	    				if( *rp ){ n++; zp = rp;}
	    				if( *++rp ){ n++; zp = rp;}
	    				if( *++rp ){ n++; zp = rp;}
						rp = sp - XW;
	    				if( *rp ){ n++; zp = rp;}
						rp = sp + XW;
	    				if( *rp ){ n++; zp = rp;}
						if( n >= 3 ) *tp = *zp;
					}
				}
			}
		}
        else {
			if( sw ){
				for( y = mc.top; y <= mc.bottom; y++ ){
					sp = &pSrc[y*XW+mc.left];
					tp = &pDest[y*XW+mc.left];
					for( x = mc.left; x <= mc.right; x++, sp++, tp++ ){
						n = 0;
						rp = sp - 1;
	    				if( *rp++ ) n++;
						if( *rp++ ) n++;
						if( *rp ) n++;
						rp = sp - XW - 1;
	    				if( *rp++ ) n++;
	    				if( *rp++ ) n++;
	    				if( *rp ) n++;
						rp = sp + XW - 1;
	    				if( *rp++ ) n++;
	    				if( *rp++ ) n++;
	    				if( *rp ) n++;
						if( n >= 5 ){
                    		*tp = 0x80;
                    	}
					}
				}
			}
			else {
				for( y = mc.top; y <= mc.bottom; y++ ){
					sp = &pSrc[y*XW+mc.left];
					tp = &pDest[y*XW+mc.left];
					for( x = mc.left; x <= mc.right; x++, sp++, tp++ ){
						n = 0;
						rp = sp - 1;
	    				if( *rp++ ) n++;
						if( *rp++ ) n++;
						if( *rp ) n++;
						if( *(sp-XW) ) n++;
						if( *(sp+XW) ) n++;
						if( n >= 3 ) *tp = 0x80;
					}
				}
			}
        }
	}
	if( pDest == m_pTbl ){
        delete pSrc;
    }
    else {
		delete m_pTbl;
        m_pTbl = pDest;
    }
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall CItem::ConvChar(LPSTR pStr)
{
	char bf[8192];
	LPCSTR p;
    LPSTR t = bf;
	for( p = pStr; *p; p++ ){
		if( (_mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD) && *(p+1) ){
			*t++ = *p++;
            *t++ = *p;
        }
		else if( (*p == '\\') && (*(p+1)=='x') ){
			p+=2;
			*t++ = (char)htoin(p, 2);
            p++;
        }
		else if( (*p == '\\') && (*(p+1)=='$') ){
			p+=2;
			*t++ = (char)htoin(p, 2);
            p+=2;
   			*t++ = (char)htoin(p, 2);
            p++;
        }
        else if( (*p == '\t') && (m_Align) ){
			*t++ = ' ';
        }
        else {
			*t++ = *p;
        }
    }
	*t = 0;
    strcpy(pStr, bf);
}
//---------------------------------------------------------------------------
void __fastcall AddStyle(AnsiString &as, BYTE charset, DWORD style)
{
    switch(charset){
		case ANSI_CHARSET:
			as += sys.m_MsgEng ? "/ANSI" : "/欧文";
        	break;
		case SHIFTJIS_CHARSET:
			as += sys.m_MsgEng ? "/Japanese" : "/日本語";
        	break;
		case HANGEUL_CHARSET:
			as += sys.m_MsgEng ? "/Korean(Hangul)" : "/ハングル";
			break;
		case JOHAB_CHARSET:
			as += sys.m_MsgEng ? "/Korean(Johab)" : "/ハングル(Johab)";
			break;
		case CHINESEBIG5_CHARSET:   // 台湾
			as += sys.m_MsgEng ? "/Chinese(BIG5)" : "/中国語(繁体)";
			break;
		case 134:                   // 中国語簡略
			as += sys.m_MsgEng ? "/Chinese(GB2312)" : "/中国語(簡体)";
			break;
        case SYMBOL_CHARSET:
			as += sys.m_MsgEng ? "/Symbol" : "/シンボル";
        	break;
        default:
        	break;
    }
	if( style & FSBOLD ) as += sys.m_MsgEng ? "/Bold" : "/太字";
	if( style & FSITALIC ) as += sys.m_MsgEng ? "/Italic" : "/斜体";
	if( style & FSUNDERLINE ) as += sys.m_MsgEng ? "/UnderLine" : "/下線";
	if( style & FSSTRIKEOUT ) as += sys.m_MsgEng ? "/StrikeOut" : "/取消し線";
}
//---------------------------------------------------------------------------
Graphics::TBitmap* __fastcall CItem::Draw(Graphics::TBitmap *pDest, LPCSTR pText)
{
	if( (pText == NULL) || !*pText ) return pDest;

	CWaitCursor w(!m_Draft);

	m_Back.c = pDest->Canvas->Pixels[0][0];
	m_Back.d &= 0x00ffffff;
	m_cBack = m_Back;
	if( m_pFont == NULL ){
		m_pFont = new TFont();
		m_pFont->Assign(pDest->Canvas->Font);
		m_pFont->Size = 18*6;
		UpdateFont();
	}
	else if( m_OrgXW >= 0 ){
		SetFontSize(pDest);
	}
    SetMBCP(m_pFont->Charset);

	AnsiString as = pText;
    ConvChar(as.c_str());
    pText = as.c_str();

	LPCSTR pOrgText = pText;
	if( m_Zero ){
		pText = ZeroConv(pText, m_pFont->Charset);
	}

	pDest->Canvas->Font = m_pFont;
	m_FW = pDest->Canvas->TextHeight(pText);
	if( !m_FW ) m_FW++;
	m_OYWC = int((short)m_OYW);
	if( m_OYW ){
		if( m_OYWS ){
			m_OYWS &= 0x0000ffff;
            m_OYWS |= m_FW << 16;
			int oyw = m_OYWS & 0x0000ffff;
			if( oyw ) m_OYWC = m_OYWC * m_FW / oyw;
        }
        else {						// 最初のフォントサイズの設定
			m_OYWS = (m_FW << 16) | m_FW;
        }
    }
	int xw, yw;
	int n = GetMaxWH(xw, yw, pDest, pText);
	m_Row = n;
	m_FW = yw;
	if( m_OYWC && n ){
		yw = (yw + m_OYWC) * (n - 1) + yw;
        if( yw < m_FW ) yw = m_FW;
    }
    else {
		yw *= n;
    }
	if( m_fsCode & FSITALIC ){
//			xw += yw / 12;
		xw += m_FW / 4;
	}
	if( !yw ) yw = 2;
    if( !xw ) xw = 2;

	if( m_Shadow && (m_cBack.d == m_ColB[0].d) ){
		m_cBack = GetUniqueColor(m_cBack);
	}
	if( m_Grade == 4 ){
		while(1){
			int f = TRUE;
			for( int i = 0; i < AN(m_ColF); i++ ){
				if( m_cBack.c == GetIndexColor(i) ){
            		f = FALSE;
                	break;
	            }
    	    }
            if( f ){
            	break;
            }
            else {
				m_cBack = GetUniqueColor(m_cBack);
            }
        }
    }
	else if( m_cBack.d == m_ColG[0].d ){
		m_cBack = GetUniqueColor(m_cBack);
	}

	MultProc();

	RECT rc;
	rc.left = rc.top = 0;
	rc.right = xw;
	rc.bottom = yw;

    if( m_Rot ){
        if( ABS(m_Rot - 90) < 10 ) rc.right += m_FW/6;
        if( ABS(m_Rot - 180) < 10 ) rc.right += m_FW/6;
        if( ABS(m_Rot - 270) < 10 ) rc.right += m_FW/6;
//		rc.right += m_FW/2;
//        rc.bottom += m_FW/2;
		double s = 2 * PI * m_Rot / 360;
		m_CosX = cos(s);
		m_SinX = sin(s);
		int xc = rc.right/2;
		int yc = rc.bottom/2;
		POINT N[5];
		N[0].x = -xc;
		N[0].y = -yc;
		N[1].x = xc;
		N[1].y = -yc;
		N[2].x = xc;
		N[2].y = yc;
		N[3].x = -xc;
		N[3].y = yc;
		N[4].x = rc.left - xc;
		N[4].y = rc.top - yc;
		int i;
		for( i = 0; i < 5; i++ ){
			int x = N[i].x * m_CosX + N[i].y * m_SinX;
			int y = -N[i].x * m_SinX + N[i].y * m_CosX;
			N[i].x = x + xc; N[i].y = y + yc;
		}
		int x1, x2, y1, y2;
		x1 = x2 = N[0].x; y1 = y2 = N[0].y;
		for( i = 1; i < 4; i++ ){
			if( x1 > N[i].x ) x1 = N[i].x;
			if( x2 < N[i].x ) x2 = N[i].x;
			if( y1 > N[i].y ) y1 = N[i].y;
			if( y2 < N[i].y ) y2 = N[i].y;
		}
		int xo = N[4].x - x1;
		int yo = N[4].y - y1;
        if( ABS(m_Rot - 90) < 10 ) yo += m_FW/12;
        if( ABS(m_Rot - 180) < 10 ) xo += m_FW/12;
//        if( ABS(m_Rot - 270) < 10 ) yo += m_FW/12;
        rc.left += xo; rc.right += xo;
        rc.top += yo; rc.bottom += yo;
        xw = x2 - x1 + 1; yw = y2 - y1 + 1;
    }

	if( m_3d & 0x00010000 ){
		int xm = m_3d & 0x00ff;
		int ym = (m_3d >> 8) & 0x00ff;
		if( xm & 0x0080 ) xm |= 0xffffff80;
		if( ym & 0x0080 ) ym |= 0xffffff80;
	    if( m_Rot ){
			int x = xm * m_CosX + ym * m_SinX;
        	int y = -xm * m_SinX + ym * m_CosX;
			xm = x;
        	ym = y;
    	}
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

	if( m_fsCode & FSITALIC ){		// イタリックの場合の位置補正
		int oxw = 0;
		int oyw = 0;
		switch(GetPosFormat()){
			case DT_CENTER:
				oxw = m_FW / 8;
                if( m_Rot ){
                	oxw *= m_CosX;
                }
            	break;
            case DT_RIGHT:
				oxw = m_FW / 4;
                if( m_Rot ){
                	oxw *= m_CosX;
                    oyw = -(m_FW / 4) * m_SinX;
                }
            	break;
            default:
            	break;
		}
		rc.left -= oxw;
		rc.right -= oxw;
        rc.top -= oyw;
        rc.bottom -= oyw;
	}

	if( xw & 1 ) xw++;
    if( yw & 1 ) yw++;
	Graphics::TBitmap *pBmp = pDest;
	if( !m_Draft ){
		if( xw > yw ){
			rc.top += (xw - yw)/2;
            rc.bottom += (xw - yw)/2;
			yw = xw;
	    }
    	else {
			rc.left += (yw - xw)/2;
            rc.right += (yw - xw)/2;
			xw = yw;
    	}
    }
	if( (pDest->Width != xw) || (pDest->Height != yw) ){
		pBmp = CreateBitmap(xw, yw);
		FillBitmap(pBmp, m_cBack);
	}
	else if( m_Back.d != m_cBack.d ){
		FillBitmap(pBmp, m_cBack);
	}

	MultProc();
	UCOL col = m_ColG[0];
	if( m_Draft && (col.d == m_Back.d) ) col.d = col.d ^ 0x00ffffff;
	DrawText(rc, pBmp, pText, col);
	Graphics::TBitmap *pNew = pBmp;
	if( m_Draft ){
		if( m_BmpXW < 0 ){
			double sc = m_sperspect.s * 3.0 / m_sperspect.v;
			xw *= sc;
        	yw *= sc;
        }
        else {
			xw = m_BmpXW;
            yw = m_BmpYW;
        }
		pNew = CreateBitmap(xw, yw);
		StretchCopy(pNew, pBmp, HALFTONE);
    	if( pDest != pBmp ) delete pBmp;
    	pBmp = pNew;
    }
	else if( !m_Draft ){
    	CreateTable(pBmp);

#if MeasureAccuracy
		QueryPerformanceCounter(&m_liPOld);
#endif
		Perspect();
#if MeasureAccuracy
		if( QueryPerformanceCounter(&m_liPCur) ){
			m_dlDiff1 = m_liPCur.QuadPart - m_liPOld.QuadPart;
			m_liPOld = m_liPCur;
		}
#endif
		pNew = DrawExt(pBmp);
#if MeasureAccuracy
		if( QueryPerformanceCounter(&m_liPCur) ){
			m_dlDiff2 = m_liPCur.QuadPart - m_liPOld.QuadPart;
			m_liPOld = m_liPCur;
		}
#endif
		pNew = AdjustMinimized(pNew);
        xw = pNew->Width;
        yw = pNew->Height;
#if MeasureAccuracy
		if( QueryPerformanceCounter(&m_liPCur) ){
			m_dlDiff3 = m_liPCur.QuadPart - m_liPOld.QuadPart;
			m_liPOld = m_liPCur;
		}
#endif
		m_BmpXW = xw;
		m_BmpYW = yw;
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