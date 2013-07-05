#ifndef ComLibH
#define ComLibH
#include <ComCtrls.hpp>
#include <Inifiles.hpp>
#include <Clipbrd.hpp>
#include <values.h>
#include <stdio.h>
#include <stdlib.h>
#include <mbstring.h>
#include <math.h>

#define	MeasureAccuracy		FALSE

#define NDEBUG      // Remove this symbol, if you would like to do debug
#include <assert.h>
#define ASSERT(c)   assert(c)

#define AN(c)   (sizeof(c)/sizeof(c[0]))
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

#define VER "TextArt Version 1.06"
#define AUT "(C) JE3HHT 2002-2003."
#define TTL "MMSSTV custom item"
#define TTLJ "MMSSTV カスタムアイテム"  // Japanese code

#define ININAME     "TextArt.ini"
#define	USERMASK	"TextArt.bmp"

#define FSBOLD      1
#define FSITALIC    2
#define FSUNDERLINE 4
#define FSSTRIKEOUT 8

#define	DISFONTSMOOTHING	0x00000001

// These prototypes are the functions in the MMSSTV.
//   ---- Refer to the comment of the mcmFunc() ---
/*0*/typedef HBITMAP (__cdecl *mmLoadImageMenu)(int sw, int xw, int yw);
/*1*/typedef HBITMAP (__cdecl *mmLoadImage)(LPCSTR pName);
/*2*/typedef HBITMAP (__cdecl *mmLoadHistImage)(void);
/*3*/typedef int (__cdecl *mmSaveImageMenu)(HBITMAP hb, LPCSTR pName, LPCSTR pFolder);
/*4*/typedef int (__cdecl *mmSaveImage)(HBITMAP hb, LPCSTR pName);
/*5*/typedef HBITMAP (__cdecl *mmPasteImage)(int sw, int xw, int yw);
/*6*/typedef int (__cdecl *mmGetMacKey)(LPSTR pDest, int cbDest);
/*7*/typedef int (__cdecl *mmConvMac)(LPSTR pDest, LPCSTR pSrc, int cbDest);
/*8*/typedef HBITMAP (__cdecl *mmCreateDIB)(int xw, int yw);
/*9*/typedef void (__cdecl *mmMultProc)(void);
/*10*/typedef void (__cdecl *mmMultProcA)(void);
/*11*/typedef int (__cdecl *mmColorComp)(LPDWORD pCol);
/*12*/typedef HBITMAP (__cdecl *mmLoadRxImage)(void);
/*13*/typedef void (__cdecl *mmView)(HBITMAP hb);
/*14*/typedef int (__cdecl *mmAdjCol)(HBITMAP hb);
/*15*/typedef int (__cdecl *mmClip)(HBITMAP hbDest, HBITMAP hbSrc);

void __fastcall InitSys(HINSTANCE hinst);
void __fastcall MultProc(void);
char __fastcall LastC(LPCSTR p);
void __fastcall SetCurDir(LPSTR t, int size);
void __fastcall SetDirName(LPSTR t, LPCSTR pName);
void __fastcall Yen2CrLf(AnsiString &ws, AnsiString cs);
void __fastcall CrLf2Yen(AnsiString &ws, AnsiString cs);
int __fastcall GetActiveIndex(TPageControl *pp);
void __fastcall LoadStringsFromIniFile(TStrings *pList, LPCSTR pSect, LPCSTR pName, int msg);
void __fastcall SaveStringsToIniFile(TStrings *pList, LPCSTR pSect, LPCSTR pName, int msg);
void __fastcall InitCustomColor(TColorDialog *tp);
void __fastcall AddCustomColor(TColorDialog *tp, TColor col);
Graphics::TBitmap * __fastcall CreateBitmap(int xw, int yw);
BOOL __fastcall IsSameBitmap(Graphics::TBitmap *pDest, Graphics::TBitmap *pSrc);
void __fastcall FillBitmap(Graphics::TBitmap *pDest, UCOL ucol);
void __fastcall FillBitmap(Graphics::TBitmap *pDest, TColor col);
void __fastcall CopyBitmap(Graphics::TBitmap *pDest, Graphics::TBitmap *pSrc);
void __fastcall CopyBitmap(Graphics::TBitmap *pBitmap);
void __fastcall DrawBitmap(TPaintBox *pBox, Graphics::TBitmap *pBitmap);
void __fastcall StretchCopy(Graphics::TBitmap *pDest, Graphics::TBitmap *pSrc, int mode);

int __fastcall IsFile(LPCSTR pName);
void __fastcall SetMBCP(BYTE charset);
void __fastcall AddStyle(AnsiString &as, BYTE charset, DWORD style);
void __fastcall LoadFontFromInifile(TFont *pFont, LPCSTR pSect, TMemIniFile *pIniFile);
void __fastcall SaveFontToInifile(TFont *pFont, LPCSTR pSect, TMemIniFile *pIniFile);

void __fastcall ExecPB(LPCSTR pName);
HWND __fastcall IsPB(void);

int FontStyle2Code(TFontStyles style);
TFontStyles Code2FontStyle(int code);

//---------------------------------------------------------------------------
//CBitStreamクラス
class CBitStream {
private:
	BYTE	*m_pBase;
	int		m_BitCount;
    int		m_Alloc;
public:
	CBitStream(void *pData){ SetData(pData); };
	CBitStream(){ SetData(NULL); };
	void SetData(void *pData){
		m_pBase = (BYTE *)pData;
        m_BitCount = 0;
        m_Alloc = FALSE;
    };
    DWORD GetData(int n);
	void GetText(LPSTR p, int n);
    void Skip(int n){ m_BitCount += n;};

    void PutData(DWORD d, int n);
	void PutText(LPCSTR p, int n);
	BYTE GetSum(void);
};

//---------------------------------------------------------------------------
//CItemクラス
typedef struct {		// Do not change the format
	int     ver;
	int		flag;
	double  ax;
	double  ay;
	double  px;
	double  py;
	double  pz;
	double  rz;
	double  rx;
	double  ry;
	double  v;
	double  s;
	double  r;
}SPERSPECT;
typedef SPERSPECT* LPSPERSPECT;
typedef const SPERSPECT* LPCSPERSPECT;

class CItem
{
public:
	int         m_Ver;
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
	UCOL        m_ColF[6];
	UCOL        m_Back;
	int			m_OrgXW;
	int			m_OrgYW;
	int			m_BmpXW;
	int			m_BmpYW;
	int			m_OYW;
    DWORD		m_OYWS;
    int			m_Rot;
	int			m_Filter;

	int         m_SC;
	SPERSPECT	m_sperspect;

	Graphics::TBitmap *m_pMaskBmp;	// ビットマップマスク
#if MeasureAccuracy
	LARGE_INTEGER	m_liPCur, m_liPOld;
	DWORDLONG		m_dlDiff1, m_dlDiff2, m_dlDiff3;
	LARGE_INTEGER	m_liPCurPer, m_liPOldPer;
	DWORDLONG		m_dlDiffPer1, m_dlDiffPer2;
#endif
private:
	UCOL        m_cBack;
	int         m_Draft;
	int         m_fsCode;
	int         m_Pos;
	DWORD		m_Ex;
	int			m_FW;
    double		m_SinX;
    double		m_CosX;
    int			m_OYWC;
	int			m_Row;
	TMemoryStream   *m_pMemStream;


	// 3D展開用のワーク
    int			m_XW, m_YW;
    LPBYTE		m_pTbl;
    LPBYTE		*m_pAdr;
    RECT		m_rc;
private:
	void __fastcall LoadDefault(void);
	void __fastcall UpdateDefault(void);
	void __fastcall SetFontSize(Graphics::TBitmap *pDest);
	int __fastcall GetMaxWH(int &W, int &H, Graphics::TBitmap *pDest, LPCSTR pText);
	UINT __fastcall GetPosFormat(void);
	UINT __fastcall GetDrawFormat(void);
	void __fastcall DrawText(RECT rc, Graphics::TBitmap *pDest, LPCSTR pText, UCOL ucol);
	void __fastcall DrawText_(RECT rc, Graphics::TBitmap *pDest, LPCSTR pText, int &nc);
	void __fastcall CreateTable(Graphics::TBitmap *pDest);
	Graphics::TBitmap* __fastcall DrawExt(Graphics::TBitmap *pDest);
	Graphics::TBitmap* __fastcall AdjustMinimized(Graphics::TBitmap *pSrc);
	void __fastcall Draw3D(UCOL cl[2]);
	void __fastcall DrawColorMask(int X, int Y, UCOL col);

	void __fastcall Perspect(void);
	void __fastcall MFilter(LPBYTE pSrc);
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
	void __fastcall LoadFromInifile(LPCSTR pSect, LPCSTR pName, BOOL fMask);
	void __fastcall SaveToInifile(LPCSTR pSect, LPCSTR pName, BOOL fMask);
	void __fastcall DeleteInifile(LPCSTR pSect, LPCSTR pName);
	Graphics::TBitmap* __fastcall Draw(Graphics::TBitmap *pDest, LPCSTR pText);
	inline void __fastcall SetEx(DWORD ex){ m_Ex = ex;};
    inline DWORD __fastcall GetEx(void){return m_Ex;};
	inline void __fastcall SetPos(int pos){ m_Pos = pos;};
    inline DWORD __fastcall GetPos(void){return m_Pos;};
	int __fastcall GetTrueOYW(void);

    void __fastcall GetFont(LOGFONT *pLogfont);
	void __fastcall SetFont(LOGFONT *pLogfont);

#if MeasureAccuracy
	int __fastcall Get1stXW(){return m_XW;};
    int __fastcall Get1stYW(){return m_YW;};
#endif
};

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
	HINSTANCE		m_hInstance;

	DWORD			m_dwVersion;
	int				m_WinNT;
	UINT            m_CodePage;
	int				m_MsgEng;
	int				m_BitPixel;
	mmLoadImageMenu	m_fLoadImageMenu;
    mmPasteImage	m_fPasteImage;
	mmGetMacKey		m_fGetMacKey;
	mmConvMac		m_fConvMac;
	mmMultProc		m_fMultProc;
	mmColorComp		m_fColorComp;
    mmAdjCol		m_fAdjCol;
    mmClip			m_fClip;
	DWORD           m_DefStgSize;
	LPBYTE          m_pDefStg;
	TStringList     *m_pTextList;
	TStringList     *m_pStyleList;
	char			m_BgnDir[512];

	AnsiString      m_Msg;
    TStringList		*m_pFontList;

	Graphics::TBitmap *m_pBitmapTemp;
    int				m_DefaultPage;
    int				m_SBMListState;

    UCOL			m_MaskC1;
    UCOL			m_MaskC2;

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
