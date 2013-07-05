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



//---------------------------------------------------------------------------
#ifndef DrawH
#define DrawH
#include <OleCtnrs.hpp>
//---------------------------------------------------------------------------
extern	LPCSTR _ShapePIC[2][7];
extern	LPCSTR _ShapeBOX[2][7];

#define TEXT24BIT	FALSE


#define POINT TPoint

TColor __fastcall GetNearestColor(TColor col);
void __fastcall DrawTrans(TCanvas *tp, int x, int y);
//---------------------------------------------------------------------------
// CLIBLクラス
typedef struct {
	LPCSTR  pName;
	//HANDLE  hLib;
	HINSTANCE  hLib;	//ja7ude 0521
}LIBD;

class CLIBL {
public:
	int     m_AMax;
	int     m_Count;
	LIBD    *m_pBase;
private:
	void __fastcall Alloc(void);
	void __fastcall Add(LPCSTR pName, HANDLE hLib);
public:
	CLIBL(){
		m_pBase = NULL;
		Delete();
	};
	~CLIBL(){
		Delete();
	};
	void __fastcall Delete(void);
	HANDLE __fastcall LoadLibrary(LPCSTR pName);
	void __fastcall DeleteLibrary(HANDLE hLib);
};
//---------------------------------------------------------------------------
typedef struct {
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
}SPERSPECT;
void __fastcall Perspect(Graphics::TBitmap *pSrc, Graphics::TBitmap *pTer, const SPERSPECT *pPar, TColor back);

enum {
	CM_SELECT,
	CM_GROUP,
	CM_LINE,
	CM_BOX,
	CM_TEXT,
	CM_PIC,
	CM_BOXS,
	CM_TITLE,
	CM_OLE,
	CM_LIB,

	CM_TLIST=0x8000,
};

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
	TColor  m_LineColor;
	TPenStyle m_LineStyle;
	int     m_LineWidth;
	int		m_BoxStyle;

public:
	int     m_Cursor;
	int     m_CX1, m_CY1;
	int     m_CX2, m_CY2;
	int     m_BX, m_BY;
	int     m_MF;
	int     m_MC;
public:
	TCanvas     *pc;
	TPaintBox   *pBox;

public:
	__fastcall CDraw();
	virtual __fastcall ~CDraw(){};
	virtual void __fastcall Start(TCanvas *tp, int X, int Y);
	virtual int __fastcall Finish(int X, int Y){return FALSE;};
	virtual void __fastcall Draw(TCanvas *tp){};
	virtual void __fastcall DrawFast(TCanvas *tp){Draw(tp);};
	virtual void __fastcall DrawSel(TCanvas *tp, int sw){};
	virtual void __fastcall FillPic(TCanvas *tp, TColor col, int sw, double sx, double sy, int off){};
	virtual void __fastcall Making(int X, int Y){};
	virtual void __fastcall UpdatePic(Graphics::TBitmap *pBmp){};
	virtual int __fastcall HitTest(int x, int y){return FALSE;};
	virtual void __fastcall StartMove(TCanvas *tp, int X, int Y, int code);
	virtual void __fastcall Moving(int X, int Y){};
	virtual void __fastcall Move(int X, int Y){};
	virtual int __fastcall IsMove(void){return m_MF;};
	virtual int __fastcall Edit(void);
	virtual int __fastcall Color(void);
	virtual void __fastcall SaveToStream(TStream *sp);
	virtual int __fastcall LoadFromStream(TCanvas *tp, TStream *sp);
	virtual void __fastcall Copy(CDraw *dp);
	virtual void __fastcall SaveString(TStream *sp, AnsiString &as);
	virtual void __fastcall LoadString(TStream *sp, AnsiString &as);
	virtual void __fastcall UpdateText(void){};
	virtual void __fastcall UpdateTimeText(void){};
	virtual int __fastcall IsTimeMacro(void){return 0;};
//    TColor GetCol(TColor c1, TColor c2, int x, int xw);
	void __fastcall Normal(void);
	void __fastcall SaveBitmap(TStream *sp, Graphics::TBitmap *pBitmap);
	int __fastcall LoadBitmap(TStream *sp, Graphics::TBitmap *pBitmap);
	void __fastcall AdjustRect(void);
	virtual void __fastcall ReleaseResource(void){};
	virtual int __fastcall IsOrgSize(void){return TRUE;};
	virtual void __fastcall SetOrgSize(void){};
	virtual void __fastcall KeepAspect(void){};
	virtual LPCSTR __fastcall GetText(void){return NULL;};
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
	void __fastcall RoundRect(TCanvas *tp, int xw, int yw);
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

class CDrawBoxS : public CDrawBox
{
private:

protected:
public:
	__fastcall CDrawBoxS();
	virtual __fastcall ~CDrawBoxS(){};

	virtual void __fastcall Draw(TCanvas *tp);
};

class CDrawTitle : public CDrawBox
{
private:
public:
	int     m_Type;
	int     m_ColVert;
	TColor  m_Col1;
	TColor  m_Col2;
	TColor  m_Col3;
	TColor  m_Col4;
    AnsiString	m_Sound;
	Graphics::TBitmap *pBitmap;

protected:
public:
	__fastcall CDrawTitle();
	__fastcall ~CDrawTitle();

//    virtual int HitTest(int x, int y){return CDrawBox::HitTest(x, y) ? HT_I : HT_NONE;};
	int __fastcall HitTest(int x, int y);
	void __fastcall Start(TCanvas *tp, int X, int Y);
	int __fastcall Finish(int X, int Y);
	void __fastcall FillPic(TCanvas *tp, TColor col, int sw, double sx, double sy, int off);
	void __fastcall Draw(TCanvas *tp);
	void __fastcall DrawFast(TCanvas *tp);
	void __fastcall StartMove(TCanvas *tp, int X, int Y, int code);
	void __fastcall Moving(int X, int Y);
	void __fastcall Move(int X, int Y);
	int __fastcall Edit(void);
	int __fastcall Color(void){return Edit();};
	void __fastcall Copy(CDraw *dp);

	void __fastcall SaveToStream(TStream *sp);
	int __fastcall LoadFromStream(TCanvas *tp, TStream *sp);
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
	Graphics::TBitmap * __fastcall ConvToDDB(Graphics::TBitmap *pBmp);
	Graphics::TBitmap * __fastcall ConvToDIB(Graphics::TBitmap *pBmp);
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
	void __fastcall UpdateFont(TFont *pf);

	void __fastcall SaveToStream(TStream *sp);
	int __fastcall LoadFromStream(TCanvas *tp, TStream *sp);
	void __fastcall Copy(CDraw *dp);
	void __fastcall Copy(CDraw *dp, int sw);
	void __fastcall UpdateText(void);
	void __fastcall UpdateTimeText(void);
	int __fastcall IsTimeMacro(void);
	void __fastcall ReleaseResource(void);
	LPCSTR __fastcall GetText(void){return m_Text.c_str();};
};

class CPolygon
{
public:
	int     XW;
	int     YW;
	int		Max;				// 最大点数
	int		Cnt;				// 現在の登録点数
	POINT	*pBase;				// ポイントの配列のポインタ
public:
	__fastcall CPolygon(){
		Max = Cnt = 0;
		pBase = NULL;
		XW = 320;
		YW = 256;
	};
	__fastcall ~CPolygon(){
		Free();
	};
	void __fastcall Free(void);
	void __fastcall AddPoint(int X, int Y);
	void __fastcall SaveToStream(TStream *sp);
	int __fastcall LoadFromStream(TStream *sp);
	void __fastcall Draw(TCanvas *tp, int loop);
	void __fastcall DrawOffset(TCanvas *tp, int x, int y);
	void __fastcall DrawOffsetSiege(TCanvas *tp, int x, int y, TColor SCol, int e);
	void __fastcall GetCenter(int &xc, int &yc);
	void __fastcall Copy(CPolygon *p);
	void __fastcall Stretch(int xw, int yw, int oxw, int oyw);
	void __fastcall Stretch(CPolygon &pol);
	void __fastcall Clear(void){
		Cnt = 0;
	};
};

class CDrawPic : public CDrawBox
{
private:
public:
	int     m_Type;
	int     m_Shape;
	int     m_Adjust;
	int     m_TransPoint;
	CPolygon    m_Polygon;
	CPolygon    m_DrawPolygon;
	Graphics::TBitmap *pBitmap;
	Graphics::TBitmap *pLoadBitmap;
private:
	void __fastcall CreateBitmap(void);
	void __fastcall AdjustSize(void);
	void __fastcall UpdateHistPic(Graphics::TBitmap *pBmp);

protected:
	void __fastcall DrawCursor(void);
public:
	__fastcall CDrawPic();
	__fastcall ~CDrawPic();

	int __fastcall Finish(int X, int Y);
	int __fastcall FinishR(int X, int Y);
	int __fastcall Color(void){return Edit();};
	void __fastcall Move(int X, int Y);
	void __fastcall Draw(TCanvas *tp);
	void __fastcall FillPic(TCanvas *tp, TColor col, int sw, double sx, double sy, int off);
	void __fastcall SetLoadBitmap(Graphics::TBitmap *pBmp);

	void __fastcall SaveToStream(TStream *sp);
	int __fastcall LoadFromStream(TCanvas *tp, TStream *sp);
	void __fastcall Copy(CDraw *dp);

	int __fastcall Edit(void);
	int __fastcall IsOrgSize(void);
	void __fastcall SetOrgSize(void);
	void __fastcall KeepAspect(void);
};

class CDrawOle : public CDrawBox
{
public:
	int           m_Trans;
	int           m_Stretch;
	TOleContainer *pContainer;
	int           m_OrgXW;
	int           m_OrgYW;
private:
	void __fastcall GetOrgSize(void);
	void __fastcall SetInitSize(void);
	void __fastcall MakeItem(void);
	void __fastcall MakeError(void);
	void __fastcall SyncVerb(void);
	void __fastcall ViewVerb(int sw);
	TForm         *pOleView;

public:
	__fastcall CDrawOle();
	__fastcall ~CDrawOle();

	int __fastcall Finish(int X, int Y);
	int __fastcall Color(void){return Edit();};
	Graphics::TBitmap* __fastcall MakeBitmap(void);
	void __fastcall Draw(TCanvas *tp);
	void __fastcall SaveToStream(TStream *sp);
	int __fastcall LoadFromStream(TCanvas *tp, TStream *sp);
	void __fastcall Copy(CDraw *dp);
	int __fastcall Edit(void);
	int __fastcall LoadFromFile(int x, int y, LPCSTR pName);
	void __fastcall LoadFromBitmap(int x, int y, Graphics::TBitmap *pBmp);
	int __fastcall Paste(void);
	void __fastcall DoPrimary(void);
	int __fastcall ObjectProperties(void);
	int __fastcall IsOrgSize(void);
	void __fastcall SetOrgSize(void);
	void __fastcall KeepAspect(void);
};

#define	DISFONTSMOOTHING	0x00000001
typedef void (__cdecl *mcmFunc)(DWORD func, FARPROC pFunc);
typedef void (__cdecl *mcmLanguage)(DWORD lang);
typedef void (__cdecl *mcmAboutDialog)(HWND hWnd);
typedef HANDLE (__cdecl *mcmCreateObject)(const BYTE *pStorage, DWORD Size);
typedef const BYTE* (__cdecl *mcmCreateStorage)(HANDLE hObj, LPDWORD pSize);
typedef void (__cdecl *mcmDeleteStorage)(HANDLE hObj, const BYTE *pStorage);
typedef void (__cdecl *mcmDeleteObject)(HANDLE hObj);
typedef DWORD (__cdecl *mcmGetItemType)(HANDLE hObj);
typedef DWORD (__cdecl *mcmEdit)(HANDLE hObj, HWND hWnd);
typedef ULONG (__cdecl *mcmGetOrgSize)(HANDLE hObj);
typedef void (__cdecl *mcmPos)(HANDLE hObj, DWORD pos, DWORD size, DWORD tsize);
typedef LPCSTR (__cdecl *mcmGetUserText)(HANDLE hObj);
typedef HBITMAP (__cdecl *mcmUpdateText)(HANDLE hObj, HBITMAP hDest, LPCSTR pText);
typedef HBITMAP (__cdecl *mcmUpdateImage)(HANDLE hObj, HBITMAP hDest, HBITMAP hSrc);
typedef void (__cdecl *mcmSetDraft)(HANDLE hObj, DWORD sw);
typedef void (__cdecl *mcmSetEx)(HANDLE hObj, DWORD ex);
typedef void (__cdecl *mcmGetFont)(HANDLE hObj, LOGFONT *pLogfont);
typedef void (__cdecl *mcmSetFont)(HANDLE hObj, LOGFONT *pLogfont);
class CDrawLib : public CDrawBox
{
public:
	int           m_Type;   // 0:Static, 1-Text, 2-Image
	AnsiString    m_Name;   // Name of custom item
	HINSTANCE     hLib;		//ja7ude 0521
	HANDLE        hObj;
	Graphics::TBitmap *pBitmap;
	int           m_OrgXW;
	int           m_OrgYW;
private:
	mcmFunc         fFunc;
	mcmLanguage     fLanguage;
	mcmAboutDialog  fAboutDialog;
	mcmGetItemType  fGetItemType;
	mcmCreateObject fCreateObject;
	mcmCreateStorage    fCreateStorage;
	mcmDeleteStorage    fDeleteStorage;
	mcmDeleteObject fDeleteObject;
	mcmEdit         fEdit;
	mcmEdit         fFont;
	mcmGetOrgSize   fGetOrgSize;
	mcmPos          fPos;
	mcmGetUserText  fGetUserText;
	mcmUpdateText   fUpdateText;
	mcmUpdateImage  fUpdateImage;
	mcmSetDraft     fSetDraft;
	mcmSetEx		fSetEx;
	mcmGetFont		fGetFont;
	mcmSetFont		fSetFont;
	AnsiString		m_Text;

	FARPROC __fastcall GetProc(LPCSTR pName);
	void __fastcall Delete(void);
	void __fastcall LoadLib(int emsg);
	void __fastcall SetBitmapSize(void);
	void __fastcall UpdateText(int f);
	void __fastcall SwapHandle(HBITMAP hb);
	void __fastcall Pos(void);
	void __fastcall Update(void);
	int __fastcall Edit(int sw);
public:
	__fastcall CDrawLib();
	__fastcall ~CDrawLib();

	int __fastcall MakeR(LPCSTR pName);
	int __fastcall Finish(int X, int Y);
	int __fastcall Color(void){return Edit();};
	void __fastcall Move(int X, int Y);
	void __fastcall Draw(TCanvas *tp);
	void __fastcall DrawFast(TCanvas *tp);
	void __fastcall SaveToStream(TStream *sp);
	int __fastcall LoadFromStream(TCanvas *tp, TStream *sp);
	void __fastcall Copy(CDraw *dp);
	int __fastcall Edit(void);
	int __fastcall Font(void);
	void __fastcall ObjectProperties(void);
	void __fastcall UpdatePic(Graphics::TBitmap *pBmp);
	void __fastcall UpdateText(void);
	void __fastcall UpdateTimeText(void);
	int __fastcall IsOrgSize(void);
	void __fastcall SetOrgSize(void);
	void __fastcall KeepAspect(void);
	LPCSTR __fastcall GetItemText(void){
		if( !hLib || !hObj || !fGetUserText ) return "???";
		return fGetUserText(hObj);
	};
	int __fastcall IsTimeMacro(void);
	LPCSTR __fastcall GetText(void){return m_Text.c_str();};

	int __fastcall GetFont(LOGFONT *pLogfont);
	int __fastcall SetFont(LOGFONT *pLogfont);
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


#if 0
//---------------------------------------------------------------------------
class CGrid {
public:
	int		m_Flag;
	int		m_Intval;
public:
	__fastcall CGrid();
	void __fastcall Draw(TCanvas *tp, int xw, int yw);
	void __fastcall Align(int &x1, int &y1, int &x2, int &y2);
	void __fastcall Align(CDraw *pItem);
};
#endif

//---------------------------------------------------------------------------
int __fastcall LoadTemplate(CDrawGroup *pItem, LPCSTR pName, TCanvas *tp);
int __fastcall SaveTemplate(CDrawGroup *pItem, LPCSTR pName);

void __fastcall SetDrawSize(int x, int y);
TColor __fastcall GetCol(TColor c1, TColor c2, int x, int xw);

extern  CDrawGroup  DrawPara;
#endif

