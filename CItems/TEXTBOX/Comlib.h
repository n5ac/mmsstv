#ifndef ComLibH
#define ComLibH
#include <ComCtrls.hpp>
#include <Inifiles.hpp>
#include <values.h>
#include <stdio.h>
#include <stdlib.h>
#include <mbstring.h>
#include <math.h>

#define	UseHWND	0
#define	ADJUST_MINIMIZED	FALSE

#define NDEBUG      // Remove this symbol, if you would like to do debug
#include <assert.h>
#define ASSERT(c)   assert(c)

#define ABS(c)	(((c)<0)?(-(c)):(c))
#define	PI	3.1415926535897932384626433832795

typedef const BYTE* LPCBYTE;
typedef double* LPDOUBLE;
typedef const double* LPCDOUBLE;
typedef union {
	struct {
		BYTE    r;
		BYTE    g;
		BYTE    b;
		BYTE    s;
	}b;
	DWORD       d;
	TColor      c;
}UCOL;

#define VER "TextBox Version 1.09"
#define AUT "(C) JE3HHT 2002-2003."
#define TTL "MMSSTV custom item"
#define TTLJ "MMSSTV カスタムアイテム"  // Japanese code

#define ININAME     "TextBox.ini"

#define FSBOLD      1
#define FSITALIC    2
#define FSUNDERLINE 4
#define FSSTRIKEOUT 8

#define	DISFONTSMOOTHING	0x00000001

// These prototypes are the functions in the MMSSTV.
//   ---- Refer to the comment of the mcmFunc() ---
typedef HBITMAP (__cdecl *mmLoadImageMenu)(int sw, int xw, int yw);
typedef HBITMAP (__cdecl *mmLoadImage)(LPCSTR pName);
typedef HBITMAP (__cdecl *mmLoadHistImage)(void);
typedef int (__cdecl *mmSaveImageMenu)(HBITMAP hb, LPCSTR pName, LPCSTR pFolder);
typedef int (__cdecl *mmSaveImage)(HBITMAP hb, LPCSTR pName);
typedef HBITMAP (__cdecl *mmPasteImage)(int sw, int xw, int yw);
typedef int (__cdecl *mmGetMacKey)(LPSTR pDest, int cbDest);
typedef int (__cdecl *mmConvMac)(LPSTR pDest, LPCSTR pSrc, int cbDest);
typedef HBITMAP (__cdecl *mmCreateDIB)(int xw, int yw);
typedef void (__cdecl *mmMultProc)(void);
typedef void (__cdecl *mmMultProcA)(void);
typedef int (__cdecl *mmColorComp)(LPDWORD pCol);

void __fastcall InitSys(void);
void __fastcall MultProc(void);
char __fastcall LastC(LPCSTR p);
void __fastcall SetCurDir(LPSTR t, int size);
void __fastcall SetDirName(LPSTR t, LPCSTR pName);
void __fastcall Yen2CrLf(AnsiString &ws, AnsiString cs);
void __fastcall CrLf2Yen(AnsiString &ws, AnsiString cs);
void __fastcall LoadStringsFromIniFile(TStrings *pList, LPCSTR pSect, LPCSTR pName, int msg);
void __fastcall SaveStringsToIniFile(TStrings *pList, LPCSTR pSect, LPCSTR pName, int msg);
void __fastcall InitCustomColor(TColorDialog *tp);
void __fastcall AddCustomColor(TColorDialog *tp, TColor col);
Graphics::TBitmap * __fastcall CreateBitmap(int xw, int yw);
void __fastcall FillBitmap(Graphics::TBitmap *pDest, UCOL ucol);
void __fastcall CopyBitmap(Graphics::TBitmap *pDest, Graphics::TBitmap *pSrc);
void __fastcall DrawBitmap(TPaintBox *pBox, Graphics::TBitmap *pBitmap);

int __fastcall IsFile(LPCSTR pName);
void __fastcall SetMBCP(BYTE charset);
void __fastcall AddStyle(AnsiString &as, BYTE charset, DWORD style);
void __fastcall LoadFontFromInifile(TFont *pFont, LPCSTR pSect, TMemIniFile *pIniFile);
void __fastcall SaveFontToInifile(TFont *pFont, LPCSTR pSect, TMemIniFile *pIniFile);

class CWaitCursor
{
private:
	TCursor sv;
public:
	CWaitCursor(BOOL sw);
	CWaitCursor();
	~CWaitCursor();
	void Delete(void);
	void Wait(void);
};

int FontStyle2Code(TFontStyles style);
TFontStyles Code2FontStyle(int code);

class CItem
{
public:
	int         m_Ver;
	int         m_ASize;
	int         m_Align;
	int         m_Grade;
	int         m_Shadow;
	int         m_Line;
	int         m_Zero;
	DWORD       m_3d;
	AnsiString  m_Text;
	TFont       *m_pFont;
	UCOL        m_ColG[4];
	UCOL        m_ColB[4];
	UCOL        m_Back;
	int			m_OrgXW;
	int			m_OrgYW;
	int			m_TextXW;
	int			m_TextYW;
	int			m_OYW;
    int			m_Rot;
private:
	UCOL        m_cBack;
	int         m_Draft;
	int         m_fsCode;
	int         m_IniSize;
	int         m_Pos;
	DWORD		m_Ex;
	int			m_FW;
    double		m_SinX;
    double		m_CosX;
    int			m_OYWC;
    int			m_Row;
	TMemoryStream   *m_pMemStream;
private:
	void __fastcall LoadDefault(void);
	void __fastcall UpdateDefault(void);
	void __fastcall SetFontSize(Graphics::TBitmap *pDest);
	void __fastcall SetTextSize(Graphics::TBitmap *pDest);
	int __fastcall GetMaxWH(int &W, int &H, Graphics::TBitmap *pDest, LPCSTR pText);
	UINT __fastcall GetPosFormat(void);
	UINT __fastcall GetDrawFormat(void);
	void __fastcall DrawText(RECT rc, int xoff, int yoff, Graphics::TBitmap *pDest, LPCSTR pText, UCOL ucol);
	void __fastcall DrawText_(RECT rc, int xoff, int yoff, Graphics::TBitmap *pDest, LPCSTR pText);
	void __fastcall DrawTrans(Graphics::TBitmap *pDest, int x, int y, Graphics::TBitmap *pSrc);
	LPBYTE __fastcall CreateTable(LPBYTE* &pAdr, Graphics::TBitmap *pDest);
	void __fastcall Draw3D(RECT &rc, Graphics::TBitmap *pDest, LPCSTR pText, UCOL cl[2]);
	Graphics::TBitmap* __fastcall DrawExt(Graphics::TBitmap *pDest, RECT &rc, LPCSTR pText);

	void __fastcall ConvChar(LPSTR pStr);
protected:
public:
	__fastcall CItem(void);
	__fastcall ~CItem();
	void __fastcall Create(LPCBYTE ps, DWORD size);
	DWORD __fastcall GetItemType(void);
	LPCBYTE __fastcall CreateStorage(LPDWORD psize);
	void __fastcall DeleteStorage(LPCBYTE pStorage);
	void __fastcall Copy(CItem *sp);
	int __fastcall EditDialog(HWND hWnd);
	int __fastcall FontDialog(HWND hWnd);
	DWORD __fastcall GetOrgSize(void);
	void __fastcall SetPos(DWORD pos, DWORD size, DWORD tsize);
	inline LPCSTR __fastcall GetText(void){
		return m_Text.c_str();
	};
	inline void __fastcall SetDraft(int draft){
		m_Draft = draft;
	};
	void __fastcall UpdateFont(void);
	void __fastcall LoadFromInifile(LPCSTR pSect, LPCSTR pName);
	void __fastcall SaveToInifile(LPCSTR pSect, LPCSTR pName);
	void __fastcall DeleteInifile(LPCSTR pSect, LPCSTR pName);
	Graphics::TBitmap* __fastcall Draw(Graphics::TBitmap *pDest, LPCSTR pText);
	inline void __fastcall SetEx(DWORD ex){ m_Ex = ex;};
    inline DWORD __fastcall GetEx(void){return m_Ex;};
	inline void __fastcall SetPos(int pos){ m_Pos = pos;};
    inline DWORD __fastcall GetPos(void){return m_Pos;};
	int __fastcall GetTrueOYW(void);

    void __fastcall GetFont(LOGFONT *pLogfont);
	void __fastcall SetFont(LOGFONT *pLogfont);

};

typedef struct {
	AnsiString		m_Name;
	int				m_Height;
    BYTE			m_Charset;
    DWORD			m_Style;
}FONTDATA;

enum {
	fmJA,
    fmHL,
    fmJOHAB,
    fmBV,
    fmBY,
	fmEND,
};
typedef struct {
	DWORD			m_dwVersion;
	int				m_WinNT;
	UINT            m_CodePage;
	int				m_MsgEng;
	int				m_BitPixel;
	mmGetMacKey		m_fGetMacKey;
	mmConvMac		m_fConvMac;
	mmMultProc		m_fMultProc;
	mmColorComp		m_fColorComp;
	DWORD           m_DefStgSize;
	LPBYTE          m_pDefStg;
	TStringList     *m_pTextList;
	TStringList     *m_pStyleList;
	char			m_BgnDir[512];

	AnsiString      m_Msg;
    TStringList		*m_pFontList;

	Graphics::TBitmap *m_pBitmapTemp;

    int				m_CodeLeft;
    int				m_CodeTop;
    int				m_CodeOut;

	BOOL			m_fEudc;
    AnsiString		m_Eudc;

    BOOL			m_fFontFam;
    BOOL			m_tFontFam[fmEND];
}SYS;
extern SYS	sys;
#endif
