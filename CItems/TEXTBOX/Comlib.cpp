//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "ComLib.h"
#include "TextDlg.h"
SYS	sys;
//---------------------------------------------------------------------------
void __fastcall InitSys(void)
{
	sys.m_dwVersion = 0;
	sys.m_WinNT = 0;
	sys.m_CodePage = 0;
	sys.m_MsgEng = 0;
	sys.m_BitPixel = 0;
	sys.m_fGetMacKey = 0;
	sys.m_fConvMac = 0;
	sys.m_fMultProc = 0;
	sys.m_fColorComp = 0;
	sys.m_DefStgSize = 0;
	sys.m_pDefStg = NULL;
	sys.m_pTextList = NULL;
	sys.m_pStyleList = NULL;
	sys.m_BgnDir[0] = 0;
	sys.m_pFontList = NULL;
	sys.m_pBitmapTemp = NULL;

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

	m_Ver = 1;
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
	m_Pos = 0;
	m_Ex = 0;
    m_OYW = 0;
    m_Rot = 0;
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
		if( ver >= 1 ){
			pMem->Read(&m_OYW, sizeof(m_OYW));
			pMem->Read(&m_Rot, sizeof(m_Rot));
        }
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
	m_Ver = 1;
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
	m_pMemStream->Write(&m_OYW, sizeof(m_OYW));
	m_pMemStream->Write(&m_Rot, sizeof(m_Rot));
	WriteString(m_pMemStream, m_Text);
	WriteFont(m_pMemStream, m_pFont);
	*pSize = m_pMemStream->Position;
	return (LPCBYTE)m_pMemStream->Memory;
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
void __fastcall CItem::LoadFromInifile(LPCSTR pSect, LPCSTR pName)
{
	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, pName);
	TMemIniFile *pIniFile = new TMemIniFile(fname);
	MultProc();
	m_ASize = pIniFile->ReadInteger(pSect, "AutoSize", m_ASize);
	m_Align = pIniFile->ReadInteger(pSect, "Align", m_Align);
	m_Grade = pIniFile->ReadInteger(pSect, "Grade", m_Grade);
	m_Shadow = pIniFile->ReadInteger(pSect, "Shadow", m_Shadow);
	m_Line = pIniFile->ReadInteger(pSect, "Line", m_Line);
	m_Zero = pIniFile->ReadInteger(pSect, "Zero", 0);
	m_3d = pIniFile->ReadInteger(pSect, "3D", m_3d);
	m_OYW = pIniFile->ReadInteger(pSect, "OYW", m_OYW);
	m_Rot = pIniFile->ReadInteger(pSect, "Rot", m_Rot);
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
#if 1
		LoadFontFromInifile(m_pFont, pSect, pIniFile);
		UpdateFont();
#else
		m_pFont->Name = pIniFile->ReadString(pSect, "FontName", m_pFont->Name);
		m_pFont->Charset = (BYTE)pIniFile->ReadInteger(pSect, "FontSet", m_pFont->Charset);
		m_pFont->Height = pIniFile->ReadInteger(pSect, "FontSize", m_pFont->Height);
		TFontStyles ts = m_pFont->Style;
		DWORD d = FontStyle2Code(ts);
		d = pIniFile->ReadInteger(pSect, "FontStyle", d);
		m_fsCode = d;
		ts = Code2FontStyle(d);
		m_pFont->Style = ts;
#endif
	}
	delete pIniFile;
}
//---------------------------------------------------------------------------
void __fastcall CItem::SaveToInifile(LPCSTR pSect, LPCSTR pName)
{
	char    fname[512];
	sprintf(fname, "%s%s", sys.m_BgnDir, pName);

	TMemIniFile *pIniFile = new TMemIniFile(fname);
	MultProc();
	try {
		pIniFile->WriteInteger(pSect, "AutoSize", m_ASize);
		pIniFile->WriteInteger(pSect, "Align", m_Align);
		pIniFile->WriteInteger(pSect, "Grade", m_Grade);
		pIniFile->WriteInteger(pSect, "Shadow", m_Shadow);
		pIniFile->WriteInteger(pSect, "Line", m_Line);
		pIniFile->WriteInteger(pSect, "Zero", m_Zero);
		pIniFile->WriteInteger(pSect, "3D", m_3d);
		pIniFile->WriteInteger(pSect, "OYW", GetTrueOYW());
		pIniFile->WriteInteger(pSect, "Rot", m_Rot);
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
#if 1
			SaveFontToInifile(m_pFont, pSect, pIniFile);
#else
			pIniFile->WriteString(pSect, "FontName", m_pFont->Name);
			pIniFile->WriteInteger(pSect, "FontSet", m_pFont->Charset);
			pIniFile->WriteInteger(pSect, "FontSize", m_pFont->Height);
			TFontStyles ts = m_pFont->Style;
			DWORD d = FontStyle2Code(ts);
			pIniFile->WriteInteger(pSect, "FontStyle", d);
#endif
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
	if( m_ASize ){
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
	int oyh = m_OYW >> 16;
    if( oyh > 0 ){
		int fw = oyh >> 8;
        oyh &= 0x00ff;
		oyw = oyw * fw / oyh;
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
	if( m_ASize ){
		m_OrgXW = m_OrgYW = -1;
		m_TextXW = m_TextYW = -1;
	}
	else {
		m_IniSize = 1;
	}
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
	if( !m_ASize ) wFormat |= DT_WORDBREAK;
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
void __fastcall CItem::DrawText(RECT rc, int xoff, int yoff, Graphics::TBitmap *pDest, LPCSTR pText, UCOL ucol)
{
	if( m_Rot || (m_Ex & DISFONTSMOOTHING) ){
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

	if( m_OYWC || m_Rot ){
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
			DrawText_(trc, xoff, yoff, pDest, pList->Strings[i].c_str());
			int N = m_ASize ? 1 : ((txw - 1)/ xw) + 1;
			if( m_Rot ){
                rc.left += (m_FW + m_OYWC) * N * m_SinX;
                rc.right += (m_FW + m_OYWC) * N * m_SinX;
                rc.top += (m_FW + m_OYWC) * N * m_CosX;
            }
            else {
	            rc.top += (m_FW + m_OYWC) * N;
            }
        }
        delete pList;
    }
    else {
		DrawText_(rc, xoff, yoff, pDest, pText);
    }
}
//---------------------------------------------------------------------------
void __fastcall CItem::DrawText_(RECT rc, int xoff, int yoff, Graphics::TBitmap *pDest, LPCSTR pText)
{
	rc.left += xoff; rc.top += yoff;
	rc.right += xoff; rc.bottom += yoff;
	HDC hdc = pDest->Canvas->Handle;
	::SetBkMode(hdc, TRANSPARENT);
	if( m_ASize && (m_Row <= 1) ){
		pDest->Canvas->TextOut(rc.left, rc.top, pText);
    }
    else {
		::DrawText(hdc, pText, strlen(pText), &rc, GetDrawFormat());
    }
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
LPBYTE __fastcall CItem::CreateTable(LPBYTE* &pAdr, Graphics::TBitmap *pDest)
{
	int XW = pDest->Width;
	int YW = pDest->Height;
	LPBYTE pTbl = new BYTE[XW*YW];
	memset(pTbl, 0, XW*YW);
	LPBYTE wp = pTbl;
	int x, y;

	pAdr = new BYTE*[YW];
	BYTE    br = m_cBack.b.r;
	BYTE    bg = m_cBack.b.g;
	BYTE    bb = m_cBack.b.b;
	for( y = 0; y < YW; y++ ){
		LPBYTE p = (LPBYTE)pDest->ScanLine[y];
        pAdr[y] = p;
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
#if UseHWND
	TTextDlgBox *pBox = new TTextDlgBox(hWnd);
#else
	TTextDlgBox *pBox = new TTextDlgBox(NULL);
#endif
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
    if( m_Rot ){
		int x = xm * m_CosX + ym * m_SinX;
        int y = -xm * m_SinX + ym * m_CosX;
		xm = x;
        ym = y;
    }

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
	BYTE **pAdr = NULL;
	if( m_Grade || (m_Shadow == 2) ) pTbl = CreateTable(pAdr, pDest);

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
//			p = (LPBYTE)pDest->ScanLine[y];
			p = pAdr[y];
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
					for( int y = 0; y < yw; y++ ) pAdr[y] = (LPBYTE)pDest->ScanLine[y];
				}
				int f;
				int x, y;
				for( y = 0; y < yw; y++ ){
					wp = &pTbl[y*xw];
//					p = (LPBYTE)pDest->ScanLine[y];
					p = pAdr[y];
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
							*wp = BYTE((m_Line >= 2) ? 1 : f);
//							*wp = BYTE(f);
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
							*wp = BYTE((m_Line >= 2) ? 1 : f);
//							*wp = BYTE(f);
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
//							p = (LPBYTE)pDest->ScanLine[y];
							p = pAdr[y];
							p += ax;
							*p++ = m_ColB[0].b.b;
							*p++ = m_ColB[0].b.g;
							*p = m_ColB[0].b.r;
							*wp = BYTE((m_Line >= 2) ? 1 : f);
//							*wp = BYTE(f);
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
//							p = (LPBYTE)pDest->ScanLine[y];
							p = pAdr[y];
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
    if( pAdr != NULL ) delete pAdr;
	if( pTbl != NULL ) delete pTbl;

#if ADJUST_MINIMIZED
	pTbl = CreateTable(pDest);
	rc.left = rc.right; rc.right = 0;
   	rc.top = rc.bottom; rc.bottom = 0;
	wp = pTbl;
	int x, y;
	for( y = 0; y < yw; y++ ){
		for( x = 0; x < xw; x++, wp++ ){
			if( *wp ){
				if( rc.left > x ) rc.left = x;
				if( rc.right < x ) rc.right = x;
				if( rc.top > y ) rc.top = y;
				if( rc.bottom < y ) rc.bottom = y;
			}
		}
	}
   	delete pTbl;
#endif
	return pDest;
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
Graphics::TBitmap* __fastcall CItem::Draw(Graphics::TBitmap *pDest, LPCSTR pText)
{
	if( (pText == NULL) || !*pText ) return pDest;

	m_Back.c = pDest->Canvas->Pixels[0][0];
	m_Back.d &= 0x00ffffff;
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

	m_OYWC = int((short)m_OYW);
	if( m_OYW ){
		int oyh = m_OYW >> 16;
    	if( oyh <= 0 ){			// 最初のフォントサイズの設定
			m_OYW &= 0x0000ffff;
    		m_OYW |= (m_FW << 24)|(m_FW << 16);
    	}
    	else {					// フォントサイズによる換算
			m_OYW &= 0x00ffffff;
			m_OYW |= m_FW << 24;
			oyh &= 0x00ff;
			if( oyh ) m_OYWC = m_OYWC * m_FW / oyh;
    	}
    }
	int xw, yw;
	if( m_ASize ){
		int n = GetMaxWH(xw, yw, pDest, pText);
		m_FW = yw;
		m_Row = n;
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
		m_cBack = GetUniqueColor(m_cBack);
	}
	if( m_cBack.d == m_ColG[0].d ){
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

	Graphics::TBitmap *pBmp = pDest;
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
	DrawText(rc, 0, 0, pBmp, pText, col);
	Graphics::TBitmap *pNew = pBmp;
	if( !m_Draft ){
		pNew = DrawExt(pBmp, rc, pText);
#if ADJUST_MINIMIZED
        if( rc.left || rc.top ){
			xw = rc.right - rc.left + 1;
            yw = rc.bottom - rc.top + 1;
			Graphics::TBitmap *pbmp = CreateBitmap(xw, yw);
			pbmp->Canvas->Draw(-rc.left, -rc.top, pNew);
            delete pNew;
            pNew = pbmp;
        }
#endif
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