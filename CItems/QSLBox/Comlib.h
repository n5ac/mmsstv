#ifndef ComLibH
#define ComLibH
#include <ComCtrls.hpp>
#include <Inifiles.hpp>
#include <values.h>
#include <stdio.h>
#include <stdlib.h>
#include <mbstring.h>
#include <math.h>

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

#define VER "QSLBox Version 1.00"
#define AUT "(C) JE3HHT 2002."
#define TTL "MMSSTV custom item"
#define TTLJ "MMSSTV カスタムアイテム"  // Japanese code

#define ININAME     "QSLBox.ini"

#define FSBOLD      1
#define FSITALIC    2
#define FSUNDERLINE 4
#define FSSTRIKEOUT 8

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

void __fastcall MultProc(void);
char __fastcall LastC(LPCSTR p);
void __fastcall SetCurDir(LPSTR t, int size);
void __fastcall SetDirName(LPSTR t, LPCSTR pName);
void __fastcall Yen2CrLf(AnsiString &ws, AnsiString cs);
void __fastcall CrLf2Yen(AnsiString &ws, AnsiString cs);
void __fastcall LoadStringsFromIniFile(TStrings *pList, LPCSTR pSect, LPCSTR pName);
void __fastcall SaveStringsToIniFile(TStrings *pList, LPCSTR pSect, LPCSTR pName);
void __fastcall InitCustomColor(TColorDialog *tp);
void __fastcall AddCustomColor(TColorDialog *tp, TColor col);
Graphics::TBitmap * __fastcall CreateBitmap(int xw, int yw);
void __fastcall FillBitmap(Graphics::TBitmap *pDest, UCOL ucol);
void __fastcall CopyBitmap(Graphics::TBitmap *pDest, Graphics::TBitmap *pSrc);
void __fastcall DrawBitmap(TPaintBox *pBox, Graphics::TBitmap *pBitmap);

//***********************************************************
//CDraw item class
//***********************************************************
enum {
	HT_NONE,
	HT_P1,      // line
	HT_P2,      // line
	HT_PM,      // line
	HT_LT,      // box
	HT_RT,
	HT_LB,
	HT_RB,
	HT_L,
	HT_T,
	HT_R,
	HT_B,
	HT_I,
};

class CDraw
{
public:
	int     m_Command;
	int     m_Ver;
	int     m_X1, m_Y1;
	int     m_X2, m_Y2;
	UCOL	m_LineColor;
	TPenStyle m_LineStyle;
	int     m_LineWidth;

public:
	int     m_Cursor;
	int     m_CX1, m_CY1;
	int     m_CX2, m_CY2;
	int     m_BX, m_BY;
	int     m_MF;
	int     m_MC;
public:
	TCanvas     *m_pCanvas;

public:
	__fastcall CDraw();
	virtual __fastcall ~CDraw(){};
	virtual void __fastcall Start(TCanvas *tp, int X, int Y);
	virtual int __fastcall Finish(int X, int Y){return FALSE;};
	virtual void __fastcall Draw(TCanvas *tp){};
	virtual void __fastcall DrawFast(TCanvas *tp){Draw(tp);};
	virtual void __fastcall DrawSel(TCanvas *tp, int sw){};
	virtual void __fastcall Making(int X, int Y){};
	virtual int __fastcall HitTest(int x, int y){return FALSE;};
	virtual void __fastcall StartMove(TCanvas *tp, int X, int Y, int code);
	virtual void __fastcall Moving(int X, int Y){};
	virtual void __fastcall Move(int X, int Y){};
	virtual int __fastcall IsMove(void){return m_MF;};
	virtual int __fastcall Edit(void);
	virtual void __fastcall SaveToStream(TStream *sp);
	virtual int __fastcall LoadFromStream(TCanvas *tp, TStream *sp);
	virtual void __fastcall Copy(CDraw *dp);
	void __fastcall Normal(void);
};

class CDrawLine : public CDraw
{
private:
	void __fastcall DrawCursor(void);

protected:
public:
	__fastcall CDrawLine();
	virtual __fastcall ~CDrawLine(){};
	virtual int __fastcall Finish(int X, int Y);
	virtual void __fastcall Draw(TCanvas *tp);
	virtual void __fastcall DrawSel(TCanvas *tp, int sw);
	virtual void __fastcall Making(int X, int Y);
	virtual int __fastcall HitTest(int x, int y);
	virtual void __fastcall Moving(int X, int Y);
	virtual void __fastcall Move(int X, int Y);
};

class CDrawBox : public CDraw
{
private:

protected:
	void __fastcall DrawCursor(void);
public:
	__fastcall CDrawBox();
	virtual __fastcall ~CDrawBox(){};

	virtual int __fastcall Finish(int X, int Y);
	virtual void __fastcall Draw(TCanvas *tp);
	virtual void __fastcall DrawSel(TCanvas *tp, int sw);
	virtual void __fastcall Making(int X, int Y);
	virtual int __fastcall HitTest(int X, int Y);
	virtual void __fastcall Moving(int X, int Y);
	virtual void __fastcall Move(int X, int Y);
};

class CDrawText : public CDrawBoxS
{
public:
	int         m_Grade;
	int         m_Shadow;
	int         m_Zero;
	int         m_Rot;
	int         m_RightAdj;
	int         m_PerSpect;
	int         m_Stack;
	int         m_StackPara;
	int         m_Vert;
	int         m_VertH;
	SPERSPECT   m_sperspect;
	AnsiString  m_Text;
	TFont       *pFont;
	TColor      m_Col1;
	TColor      m_Col2;
	TColor      m_Col3;
	TColor      m_Col4;
	TColor      m_ColS;
	TColor      m_ColB;
	Graphics::TBitmap *pBitmap;
	Graphics::TBitmap *pBrushBitmap;
	int         m_X;
	int         m_Y;
private:
	Graphics::TBitmap * __fastcall DupeB(TColor col);
	void __fastcall MakeBitmap();
	int __fastcall ZeroConv(BYTE charset, LPSTR p);
	int __fastcall GetMaxWidth(int &n, LPCSTR p);
	void __fastcall DrawTextVert(TCanvas *tp, int xc, int y, int fh, LPCSTR p);
	void __fastcall DrawTextHoriANSI(TCanvas *tp, int xc, int y, LPCSTR p);
	void __fastcall DrawText(TCanvas *cp, int x, int y, LPCSTR p, int zc);
protected:
public:
	__fastcall CDrawText();
	__fastcall ~CDrawText();

//    virtual int HitTest(int x, int y){return CDrawBox::HitTest(x, y) ? HT_I : HT_NONE;};
	int __fastcall Finish(int X, int Y);
	void __fastcall Move(int X, int Y);
	void __fastcall Draw(TCanvas *tp);
	void __fastcall DrawFast(TCanvas *tp);
	int __fastcall Edit(void);
	int __fastcall Color(void){return Edit();};
	inline void __fastcall UpdateFont(TFont *pf);

	void __fastcall SaveToStream(TStream *sp);
	int __fastcall LoadFromStream(TCanvas *tp, TStream *sp);
	void __fastcall Copy(CDraw *dp);
	void __fastcall Copy(CDraw *dp, int sw);
	void __fastcall UpdateText(void);
	void __fastcall UpdateTimeText(void);
	int __fastcall IsTimeMacro(void);
	void __fastcall ReleaseResource(void);
};

class CDrawGroup : public CDrawBox
{
public:
	int     m_Max;
	int     m_Cnt;
	int     m_TransX, m_TransY;
	int     m_SX, m_SY;
	TColor  m_TransCol;
	CDraw   **pBase;
	CDraw   *pSel;
public:
	int     m_Hit;
protected:
public:
	__fastcall CDrawGroup();
	__fastcall ~CDrawGroup();
	void __fastcall Free(void);
	void __fastcall FreeItem(void);
	void __fastcall AddItem(CDraw *dp);
	void __fastcall DeleteItem(CDraw *dp);
	void __fastcall ReleaseItem(CDraw *dp);
	void __fastcall UpdatePic(Graphics::TBitmap *pBmp);
	void __fastcall FillPic(TCanvas *tp, TColor col, int sw, double sx, double sy, int off);
	void __fastcall Draw(TCanvas *tp);
	void __fastcall DrawFast(TCanvas *tp);
	void __fastcall DrawNotSel(TCanvas *tp);
	CDraw* __fastcall SelectItem(int X, int Y);
	int __fastcall HitTest(int x, int y){return CDrawBox::HitTest(x, y) ? HT_I : HT_NONE;};

	CDraw * __fastcall MakeItem(int cmd);
	CDraw * __fastcall AddItemCopy(CDraw *dp);
	virtual void __fastcall SaveToStream(TStream *sp);
	virtual int __fastcall LoadFromStream(TCanvas *tp, TStream *sp);

	virtual void __fastcall Copy(CDraw *dp);
	void __fastcall UpdateText(void);
	void __fastcall UpdateTimeText(void);
	int __fastcall IsTimeMacro(void);

	void __fastcall BackItem(CDraw *pItem);
	void __fastcall FrontItem(CDraw *pItem);
	int __fastcall UpItem(int n);
	int __fastcall DownItem(int n);
	void __fastcall DrawTrans(TCanvas *tp);

	int __fastcall IsMacro(LPCSTR pKey);
	void __fastcall AdjustTransPoint(void);
	void __fastcall AdjustTransColor(Graphics::TBitmap *pBmp);
	void __fastcall ReleaseResource(void);
	void __fastcall GetColorBarPos(int &Y1, int &Y2);
	CDraw * __fastcall GetLastItem(void);

	int __fastcall IsConvBitmap(CDraw *pItem);
	void __fastcall ConvBitmap(CDraw *pItem);
	CDrawPic* __fastcall MakePic(int x, int y, Graphics::TBitmap *pBitmap, int ov);
};

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
private:
	UCOL        m_cBack;
	int         m_Draft;
	int         m_fsCode;
	int         m_IniSize;
	TMemoryStream   *m_pMemStream;
private:
	void __fastcall LoadDefault(void);
	void __fastcall UpdateDefault(void);
	void __fastcall SetFontSize(Graphics::TBitmap *pDest);
	void __fastcall SetTextSize(Graphics::TBitmap *pDest);
	int __fastcall GetMaxWH(int &W, int &H, Graphics::TBitmap *pDest, LPCSTR pText);
	UINT __fastcall GetDrawFormat(void);
	void __fastcall DrawText(RECT rc, int xoff, int yoff, Graphics::TBitmap *pDest, LPCSTR pText, UCOL ucol);
	void __fastcall DrawTrans(Graphics::TBitmap *pDest, int x, int y, Graphics::TBitmap *pSrc);
	LPBYTE __fastcall CreateTable(Graphics::TBitmap *pDest);
	void __fastcall Draw3D(RECT &rc, Graphics::TBitmap *pDest, LPCSTR pText, UCOL cl[2]);
	Graphics::TBitmap* __fastcall DrawExt(Graphics::TBitmap *pDest, RECT &rc, LPCSTR pText);

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
};

//***********************************************************
//Common structer
//***********************************************************
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
}SYS;
extern SYS	sys;
#endif

