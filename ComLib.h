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



#ifndef ComLibH
#define ComLibH
#include <ComCtrls.hpp>
#include <inifiles.hpp>	//JA7UDE 0428
#include <Grids.hpp>
#include <values.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mbstring.h>
#include <dir.h>

#define DEBUG   	FALSE
#define	DEBUGFSKID	FALSE
#define VARITEST    FALSE
#define	SHOWAFCFQ	FALSE

#if DEBUG
#include <assert.h>
#define ASSERT(c)   assert(c)
#else
#define ASSERT(c)
#endif

/*
1.12 JASTAコンテスト　重複チェックの修正
*/
#define VERBETA "A"
#define	VERID	"Ver 1.13"
#define	VERTTL2	"MMSSTV "VERID VERBETA
#define	VERTTL  VERTTL2" (C) JE3HHT 2001-2010."

#define	CLOCKMAX	48500
#define FPAGEMAX    10              // フォルダのサムネイルのページ数
#define	CWMENUMAX	128

#define HISTNAME    "History.bin"
#define TEMPV105    "CurTemp.bin"   // V1.05までの名前
#define TEMPV105B   "CurTemp.old"
#define TEMPV106C   "Current.mtm"   // V1.06からの名前
#define TEMPV106L   "List.mtm"      // V1.06からの名前
//#define SHIFTL	10
//#define	MARKL	300
//#define	SPACEH	2700

#define	FSBOLD		1
#define	FSITALIC	2
#define FSUNDERLINE 4
#define FSSTRIKEOUT 8

enum {
	pgSync,
	pgRX,
	pgHist,
	pgTX,
	pgTemp,
};

extern	LCID	lcid;				// ロケール情報
extern	int		DisPaint;
extern  int     MsgEng;
extern  int     WinNT;
extern	int		WinVista;

extern	double	SampFreq;			// サンプリング周波数
extern	double	SampBase;
extern	int		SampType;
extern  int     SampSize;
extern	int		DemOver;
extern  const int SampTable[];

extern  double  FFTSamp;
extern  int     FFTSampType;
extern	int		FFT_SIZE;

extern	char	BgnDir[256];

extern  char    HistDir[256];
extern  char    StockDir[256];
extern  char    BitmapDir[256];
extern  char    SBitmapDir[256];
extern  char    TemplateDir[256];

extern	char	MMLogDir[256];
extern	char	ExtLogDir[256];
extern	char	RecDir[256];		// 録音ファイルのディレクトリ

//extern  HPALETTE UsrPal;

extern	AnsiString	JanHelp;

extern	const char	MONN[];
extern	const char	MONU[];

#ifndef LPCUSTR
typedef const unsigned char *	LPCUSTR;
typedef unsigned char *	LPUSTR;
#endif

extern	DWORD ColorTable[128];

#pragma option -a-	// パックの指示
typedef union {
	struct {
		BYTE    r;
		BYTE    g;
		BYTE    b;
		BYTE    d;
	}b;
	TColor      c;
	DWORD       d;
}COLD;
#pragma option -a.	// パック解除の指示

#define ABS(c)	(((c)<0)?(-(c)):(c))
#define AN(p)	(int)(sizeof(p)/sizeof(*(p)))
#define	CR		0x0d
#define	LF		0x0a
#define	TAB		'\t'

typedef struct {
	AnsiString  Name;
	double  VCOGain;
	int     loopOrder;
	double  loopFC;
	int     OutOrder;
	double  OutFC;

	int     Type;
	int     crossType;
	double  crossSmooz;
	int     crossOutOrder;
	double  crossOutFC;

	double  DemOff;
	double  DemWhite;
	double  DemBlack;
	int     DemCalibration;
	double  Dem17[17];

	int     Differentiator;
	double  DiffLevel;
}PRODEM;

typedef struct {
	AnsiString	m_Name;
    BYTE		m_Charset;
    int			m_Height;
    DWORD		m_Style;
}FONTDATA;

//#define	MSGLISTMAX	64
typedef struct {
	int         m_Priority;

	int			m_SoundPriority;
	AnsiString	m_SoundDevice;
	int			m_SoundStereo;
    int			m_StereoTX;
	AnsiString	m_SoundMMW;

	AnsiString	m_Call;

	int			m_FontAdjSize;
	int			m_WindowState;

	int         m_ExtMode[8];
	AnsiString	m_ExtCmd[8];
	AnsiString	m_ExtName[8];

	AnsiString	m_TxRxName;
	int		m_TxRxLock;
	int     m_RTSonRX;

	int		m_lmsbpf;
	int		m_echo;

	int		m_AutoTimeOffset;
	int		m_TimeOffset;
	int		m_TimeOffsetMin;
	int		m_LogLink;

	AnsiString	m_LogName;

	int		m_SoundFifoRX;
	int		m_SoundFifoTX;

	int		m_Palette;
	int     m_BitPixel;

	TColor	m_ColorLow;
	TColor	m_ColorHigh;
	TColor  m_ColorFFTB;
	TColor  m_ColorFFT;
	TColor  m_ColorFFTStg;
	TColor  m_ColorFFTSync;
	TColor  m_ColorFFTFreq;

	AnsiString	m_WinFontName;
	BYTE		m_WinFontCharset;
	int			m_WinFontStyle;

	int         m_FFTType;
	int			m_FFTGain;
	int			m_FFTResp;
	int         m_FFTStg;
	int         m_FFTWidth;
	int			m_FFTAGC;
	int         m_FFTPriority;
	double		m_SampFreq;
	double		m_TxSampOff;

	int			m_StayOnTop;
	int			m_MemWindow;

	int         m_HelpNotePad;
	int         m_TuneTXTime;
	int         m_TuneSat;

	int         m_TestDem;
	double      m_DemOff;
	double      m_DemWhite;
	double      m_DemBlack;
	int         m_DemCalibration;
	double      m_Dem17[17];

	int         m_PicClipType;
	int         m_PicClipRatio;
	int         m_PicClipMode;
	int         m_PicClipView;
	TColor      m_PicClipColor;
	int         m_HistMax;

	int         m_Way240;
	int			m_AutoMargin;

	int         m_UseRxBuff;
	int         m_AutoStop;
	int         m_AutoSync;
	int         m_CWID;
	int         m_CWIDFreq;
	AnsiString  m_CWIDText;
	int         m_CWIDSpeed;
    int			m_CWIDWPM;
	AnsiString  m_MMVID;
	AnsiString	m_CWText;

	int			m_nCWMenu;
	AnsiString	m_CWMenu[CWMENUMAX];

	int         m_VOX;
	AnsiString  m_VOXSound;

	int         m_TXFSKID;

	int         m_UseJPEG;
	int         m_JPEGQuality;

	int         m_RxViewTop;
	int         m_RxViewLeft;
	int         m_RxViewWidth;
	int         m_RxViewHeight;

	int         m_SyncViewTop;
	int         m_SyncViewLeft;
	int         m_SyncViewWidth;
	int         m_SyncViewHeight;

	int         m_HistViewTB;
	int         m_HistViewTop;
	int         m_HistViewLeft;
	int         m_HistViewWidth;
	int         m_HistViewHeight;
	int         m_HistViewSize;

	int         m_PreViewTop;
	int         m_PreViewLeft;
	int         m_PreViewWidth;
	int         m_PreViewHeight;

	int         m_CtrBtnTop;
	int         m_CtrBtnLeft;
	int         m_CtrBtnWidth;
	int         m_CtrBtnHeight;

	int         m_FileViewFlag[8];
	int         m_FileViewSize[8];
	int         m_FileViewMode[8];
	int         m_FileViewTop[8];
	int         m_FileViewLeft[8];
	int         m_FileViewWidth[8];
	int         m_FileViewHeight[8];
	int         m_FileViewMaxPage[8];
	int         m_FileViewCurPage[8];
	int         m_FileViewFMPage[8][FPAGEMAX];
	AnsiString  m_FileViewFolder[8][FPAGEMAX];
	AnsiString  m_FileViewName[8][10];
	int         m_FileViewKeep;
	int         m_FileViewShowSize;

	int         m_DivMode;
	int         m_StockViewTop;
	int         m_StockViewLeft;
	int         m_StockViewWidth;
	int         m_StockViewHeight;

	int         m_WaterViewTop;
	int         m_WaterViewLeft;
	int         m_WaterViewWidth;
	int         m_WaterViewHeight;

	int         m_LogViewTop;
	int         m_LogViewLeft;
	int         m_LogViewWidth;
	int         m_LogViewHeight;

	TColor      m_ColText[6];
	TColor      m_ColBar[4];
	AnsiString  m_AutoSaveDir;
	TColor      m_ColorSet[32];

	int         m_FixedTxMode;
	AnsiString  m_TextList[16];

	int         m_TextGrade;
	int         m_TextShadow;
	int         m_TextZero;
	int         m_TextRot;
	int         m_TextVert;
	int         m_TextVertH;
	int         m_TextStack;

	int         m_PicShape;
	int         m_PicAdjust;
	int         m_PicLineStyle;
	TColor      m_PicLineColor;

	int         m_PicSelRTM;
	int         m_PicSelSmooz;
	TColor      m_PicSelCurCol;

	int         m_Sharp2D;
	int         m_Differentiator;
	double      m_DiffLevelP;
	double      m_DiffLevelM;

	TColor      m_ColorRXB;
	int         m_HDDSize;

	int         m_ShowSizeRX;
	int         m_ShowSizeHist;
	int         m_ShowSizeTX;
	int         m_ShowSizeTemp;
	int         m_ShowSizeStock;

	int         m_Repeater;
	int         m_RepSenseLvl;  // トーン検出感度
	AnsiString  m_RepAnsCW;
	int         m_RepTimeA;     // トーン検出時間
	int         m_RepTimeB;     // トーン検出からAnsCW出力までの時間
	int         m_RepTimeC;     // 受信待機のタイムアウト
	int         m_RepTimeD;     // リプレイ送信の遅延時間

	int         m_RepBeacon;
	int         m_RepBeaconMode;
	AnsiString  m_RepTempTX;
	AnsiString  m_RepTempBeacon;
	int         m_RepBottomAdj;
	int         m_RepQuietnessTime;
	int         m_RepBeaconFilter;
	AnsiString  m_RepFolder;

	int         m_UseB24;
	AnsiString  m_Msg;
	int			m_DisFontSmooth;

    int			m_TempDelay;
    FONTDATA	m_RegFont[4];

    int			m_Temp24;

    int			m_MaskUserPage;
	TColor		m_MaskCol1;
	TColor		m_MaskCol2;

    int			m_bCQ100;
}SYSSET;

extern	SYSSET	sys;

///---------------------------------------------------------
///  テキスト文字列ストリーマー
class CTextString
{
private:
	LPCSTR	rp;
public:
	inline __fastcall CTextString(LPCSTR p){
		rp = p;
	};
	inline __fastcall CTextString(AnsiString &As){
		rp = As.c_str();
	};
	int __fastcall LoadText(LPSTR tp, int len);
};
///---------------------------------------------------------
///  コントロールのアラインの管理クラス
class CAlign
{
private:
	int		BTop, BLeft;
	int		BWidth, BHeight;
	int		OTop, OLeft;
	int		OWidth, OHeight;
	int		OFontHeight;
	double	m_FontAdj;

	TControl	*tp;
	TFont		*fp;
public:
	inline CAlign(void){
		tp = NULL;
		fp = NULL;
		m_FontAdj = 1.0;
	};
	inline ~CAlign(){
	};
	void InitControl(TControl *p, TControl *pB, TFont *pF = NULL);
	void InitControl(TControl *p, RECT *rp, TFont *pF = NULL);
	void NewAlign(TControl *pB);
	inline double GetFontAdj(void){return fabs(m_FontAdj);};
	inline TControl *GetControl(void){return tp;};
	void NewAlign(TControl *pB, double hx);
	void NewFont(AnsiString &FontName, BYTE Charset, TFontStyles fs);
	void NewFixAlign(TControl *pB, int XR);
	void Resume(void);
};

///---------------------------------------------------------
///  コントロールのアラインの管理クラス
class CAlignList
{
private:
	int		Max;
	int		Cnt;
	CAlign	**AlignList;
	void Alloc(void);
public:
	CAlignList(void);
	~CAlignList();
	void EntryControl(TControl *tp, TControl *pB, TFont *pF = NULL);
	void EntryControl(TControl *tp, RECT *rp, TFont *pF = NULL);
	void EntryControl(TControl *tp, int XW, int YW, TFont *pF = NULL);
	void NewAlign(TControl *pB);
	double GetFontAdj(TControl *pB);
	void NewAlign(TControl *pB, TControl *pS, double hx);
	void NewFont(AnsiString &FontName, BYTE Charset, TFontStyles fs);
	void NewFixAlign(TControl *pB, int XR);
	void Resume(TControl *pB);
};

class CAlignGrid
{
private:
	int		BWidth;
	int		BHeight;
	int		BRowHeight;

	int		Max;
	int		*bp;
public:
	inline CAlignGrid(void){
		Max = 0;
		bp = NULL;
	};
	inline ~CAlignGrid(){
		if( bp != NULL ){
			delete bp;
		}
	};
	void InitGrid(TStringGrid *p);
	void NewAlign(TStringGrid *p);
};

class CWebRef
{
private:
	AnsiString	HTML;
public:
	CWebRef();
	inline bool IsHTML(void){
		return !HTML.IsEmpty();
	};
	void ShowHTML(LPCSTR url);
};

class CWaitCursor
{
private:
	TCursor sv;
public:
	CWaitCursor();
	~CWaitCursor();
	void Delete(void);
	void Wait(void);
};

///-------------------------------------------------------
/// CRecentMenuクラス
#define	RECMENUMAX	4	// 処理できる最大の数
class CRecentMenu
{
private:
public:
	int		InsPos;		// メニュー挿入位置の番号
	int		Max;		// 処理する数
	AnsiString	Caption[RECMENUMAX];
	TMenuItem	*pMenu;
	TMenuItem	*Items[RECMENUMAX+1];

public:
	CRecentMenu();
	void Init(int pos, TMenuItem *pmenu, int max);
	void Init(TMenuItem *pi, TMenuItem *pmenu, int max);
	int GetCount(void);
	LPCSTR GetItemText(int n);
	void SetItemText(int n, LPCSTR p);
	int IsAdd(LPCSTR pNew);
	void Add(LPCSTR pNew);
	void ExtFilter(LPCSTR pExt);
	void Clear(void);
	void Insert(TObject *op, Classes::TNotifyEvent pEvent);
	void Delete(void);
	void Delete(LPCSTR pName);
	int FindItem(TMenuItem *mp);
	LPCSTR FindItemText(TMenuItem *mp);
};

///-------------------------------------------------------
/// CBitmapHistクラス
#define HISTMAX 256
typedef struct {
	int         m_Ver;
	int         m_Max;
	int         m_Cnt;
	int         m_wPnt;
}HEADBH;
typedef struct {
	int         m_Flag;
	SYSTEMTIME  m_UTC;
}HEADFH;
class CBitmapHist
{
public:
	HEADBH      m_Head;
	HEADFH      m_File[256];
	AnsiString  m_FileName;
	int         m_CurAdd;
	int         m_CurRead;
	int         m_UseJPEG;
private:

public:
	CBitmapHist(){
		m_CurAdd = 0;
		m_CurRead = 0;
		m_FileName = "";
		m_UseJPEG = 0;

		m_Head.m_Ver = 0;
		m_Head.m_Max = 64;
		m_Head.m_Cnt = 0;
		m_Head.m_wPnt = 0;
	};
	~CBitmapHist(){
		Close();
	};
	int Open(void);
	void Close(void);
	int OverWrite(void);
	int Read(Graphics::TBitmap *pBitmap, int slot);
	void Write(Graphics::TBitmap *pBitmap, int slot);
	void Add(Graphics::TBitmap *pBitmap, SYSTEMTIME *pTime, int mode);
	void ClearAddFlag(void){m_CurAdd = 0;};
	void Update(Graphics::TBitmap *pBitmap);
	int IsAddFlag(void){return m_CurAdd;};
	void ChangeMax(int max);
	void MoveDir(LPCSTR pNewDir);
	void Delete(int n);
	void DeleteAll(void);
	void ChangeFormat(int sw);
};

///-------------------------------------------------------
/// CExecPBクラス
class CExecPB
{
private:
	int		m_Flag;
	DWORD	m_dwCount;
	DWORD	m_dwID;		// ID値
	UINT	m_uMsg;		// メッセージ値
	HWND	m_hWndMsg;	// メッセージの送信先

    AnsiString	m_Name;
    DWORD	m_nFileSizeLow;
    FILETIME	m_ftLastWriteTime;
public:
	__fastcall CExecPB();
    __fastcall ~CExecPB();

    BOOL __fastcall Timer(void);
	void __fastcall Cancel(void);
    BOOL __fastcall Exec(LPCSTR pName, HWND hWnd, UINT uMsg, DWORD nID);
    BOOL __fastcall Exec(Graphics::TBitmap *pBitmap, HWND hWnd, UINT uMsg, DWORD nID);
	BOOL __fastcall IsChanged(void);
    BOOL __fastcall LoadBitmap(Graphics::TBitmap *pBitmap, BOOL sw);
};

void __fastcall SetDisPaint(void);
void __fastcall ResDisPaint(void);

void ShowHelp(int index);
void InitSampType(void);
double NormalSampFreq(double d, double m);
int IsFile(LPCSTR pName);

void GetUTC(SYSTEMTIME *tp);
void GetLocal(SYSTEMTIME *tp);
LPSTR StrDupe(LPCSTR s);

void InitColorTable(TColor cl, TColor ch);
LPCSTR GetKeyName(WORD Key);
WORD GetKeyCode(LPCSTR pName);
LPCSTR ToDXKey(LPCSTR s);
LPCSTR ToJAKey(LPCSTR s);

//int InputMB(LPCSTR pMsg, LPCSTR pTitle, AnsiString &as);

int SetTBValue(double d, double dmax, int imax);
double GetTBValue(int d, double dmax, int imax);
LPUSTR jstrupr(LPUSTR s);
inline LPSTR jstrupr(LPSTR s){return (LPSTR)jstrupr(LPUSTR(s));};

int SetTimeOffsetInfo(int &Hour, int &Min);
WORD AdjustRolTimeUTC(WORD tim, char c);
void FormCenter(TForm *tp, int XW, int YW);
char *lastp(char *p);
char *clipsp(char *s);
LPCSTR _strdmcpy(LPSTR t, LPCSTR p, char c);
const char *StrDlmCpy(char *t, const char *p, char Dlm, int len);
const char *StrDlmCpyK(char *t, const char *p, char Dlm, int len);
void StrCopy(LPSTR t, LPCSTR s, int n);
char LastC(LPCSTR p);
LPCSTR GetEXT(LPCSTR Fname);
void SetEXT(LPSTR pName, LPSTR pExt);
BOOL __fastcall CheckEXT(LPCSTR pName, LPCSTR pExt);
void SetCurDir(LPSTR t, int size);
void SetDirName(LPSTR t, LPCSTR pName);
LPCSTR StrDbl(double d);
LPCSTR StrDblE(double d);
double ReadDoubleIniFile(TMemIniFile *p, LPCSTR c1, LPCSTR c2, double d);
void WriteDoubleIniFile(TMemIniFile *p, LPCSTR c1, LPCSTR c2, double d);
void ClipLF(LPSTR sp);
void DeleteComment(LPSTR bf);
LPSTR FillSpace(LPSTR s, int n);
LPSTR SkipSpace(LPSTR sp);
LPCSTR SkipSpace(LPCSTR sp);
LPSTR StrDlm(LPSTR &t, LPSTR p);
LPSTR StrDlm(LPSTR &t, LPSTR p, char c);
void ChgString(LPSTR t, char a, char b);
void DelChar(LPSTR t, char a);
int atoin(const char *p, int n);
int htoin(const char *p, int n);
BOOL __fastcall ATOI(int &d, LPCSTR s);

int InvMenu(TMenuItem *pItem);
void AdjustBitmapFormat(Graphics::TBitmap *pBitmap);
Graphics::TBitmap *Bitmap24bit(Graphics::TBitmap *pBitmap);
int LoadBitmap(Graphics::TBitmap *pBitmap, LPCSTR pName);
int SaveBitmap24(Graphics::TBitmap *pBitmap, LPCSTR pName);
int SaveBitmap(Graphics::TBitmap *pBitmap, LPCSTR pName);
void FillBitmap(Graphics::TBitmap *pBitmap, TColor col);
int SaveJPEG(Graphics::TBitmap *pBitmap, LPCSTR pName);
int LoadJPEG(Graphics::TBitmap *pBitmap, LPCSTR pName);
int IsPic(LPCSTR pExt);
LPCSTR GetPicFilter(void);
LPCSTR GetTempFilter(void);
LPCSTR GetTempMFilter(void);
LPCSTR GetTempIFilter(void);
LPCSTR GetLibFilter(void);
int LoadImage(Graphics::TBitmap *pBitmap, LPCSTR pName);
int SaveImage(Graphics::TBitmap *pBitmap, LPCSTR pName);

void InitCustomColor(TColorDialog *tp);
void AddCustomColor(TColorDialog *tp, TColor col);

void InfoMB(LPCSTR fmt, ...);
void ErrorMB(LPCSTR fmt, ...);
void WarningMB(LPCSTR fmt, ...);
int YesNoMB(LPCSTR fmt, ...);
int YesNoCancelMB(LPCSTR fmt, ...);
int OkCancelMB(LPCSTR fmt, ...);
void ErrorFWrite(LPCSTR pName);

int RemoveL2(LPSTR t, LPSTR ss, LPCSTR pKey, int size);
void AddL2(LPSTR t, LPCSTR pKey, LPCSTR s, UCHAR c1, UCHAR c2, int size);

void Yen2CrLf(AnsiString &ws, AnsiString cs);
void CrLf2Yen(AnsiString &ws, AnsiString cs);

void SetComboBox(TComboBox *pCombo, LPCSTR pList);
void GetComboBox(AnsiString &as, TComboBox *pCombo);
void ExecPB(LPCSTR pName);

int IsNumbs(LPCSTR p);
int IsAlphas(LPCSTR p);
int IsRST(LPCSTR p);
int IsCallChar(char c);
int IsCall(LPCSTR p);
int IsJA(const char *s);
LPCSTR ClipCall(LPCSTR s);
LPCSTR ClipCC(LPCSTR s);

/* JA7UDE 0428
inline LPUSTR StrDlm(LPUSTR &t, LPUSTR p, char c){return (LPUSTR)StrDlm(LPSTR(t), LPSTR(p), c);};
inline LPUSTR StrDlm(LPUSTR &t, LPUSTR p){return (LPUSTR)StrDlm(LPSTR(t), LPSTR(p));};
inline LPUSTR StrDlm(LPUSTR &t, LPSTR p, char c){return (LPUSTR)StrDlm(LPSTR(t), p, c);};
inline LPUSTR StrDlm(LPUSTR &t, LPSTR p){return (LPUSTR)StrDlm(LPSTR(t), p);};
*/

void TopWindow(TForm *tp);
void NormalWindow(TForm *tp);

int FontStyle2Code(TFontStyles style);
TFontStyles Code2FontStyle(int code);

void __fastcall KeepAspect(int &sx, int &sy, int bx, int by);
void __fastcall KeepAspectDraw(TCanvas *tp, int sx, int sy, Graphics::TBitmap *pBitmap);
Graphics::TBitmap *__fastcall DupeBitmap(Graphics::TBitmap *pSrc, int fmt);
Graphics::TBitmap *__fastcall RemakeBitmap(Graphics::TBitmap *pBitmap, int fmt);
Graphics::TBitmap* __fastcall CreateBitmap(int xw, int yw, int fmt);
void __fastcall StretchCopyBitmap(Graphics::TBitmap *pTer, Graphics::TBitmap *pSrc);
void __fastcall StretchCopyBitmapHW(Graphics::TBitmap *pTer, Graphics::TBitmap *pSrc);
void __fastcall CopyBitmap(Graphics::TBitmap *pTer, Graphics::TBitmap *pSrc);
void __fastcall StretchCopy(Graphics::TBitmap *pTer, Graphics::TBitmap *pSrc, int mode);
void __fastcall StretchCopy(Graphics::TBitmap *pBitmap, TRect *tc, Graphics::TBitmap *pbmp, TRect *rp);
void __fastcall SetGroupEnabled(TGroupBox *gp);

int __fastcall Limit256(int d);
void __fastcall LimitRGB(int &R, int &G, int &B);
void __fastcall YCtoRGB(int &R, int &G, int &B, int Y, int RY, int BY);
TColor __fastcall YCtoTColor(int Y, int RY, int BY);
int __fastcall ColorToFreq(int d);
int __fastcall ColorToFreqNarrow(int d);
void __fastcall MultProc(void);
void __fastcall MultProcA(void);

void __fastcall DrawBitmap(TPaintBox *pBox, Graphics::TBitmap *pBitmap);
void __fastcall ReSizeBitmap(Graphics::TBitmap *pBitmap, int w, int h);
Graphics::TBitmap *__fastcall MFilter(Graphics::TBitmap *pTer, Graphics::TBitmap *pSrc, int sw);
void __fastcall CopyAutoSize(Graphics::TBitmap *pTer, Graphics::TBitmap *pSrc);
void __fastcall GetRY(int &Y, int &RY, int &BY, TColor col);
void __fastcall DownColor(Graphics::TBitmap *pbmp, TColor bcol, int lm);

void KeyEvent(const short *p);
void __fastcall SetThumbnailSize(TRect &rc, int sw);
void __fastcall NormalRect(long *x1, long *y1, long *x2, long *y2);	//ja7ude 0521
void __fastcall NormalRect(int &x1, int &y1, int &x2, int &y2);	//ja7ude 0521
void __fastcall NormalRect(TRect *rc);
int __fastcall GetActiveIndex(TPageControl *pp);

void __fastcall ClearCodeErr(void);
void __fastcall SetMBCP(void);
void __fastcall SetMBCP(BYTE charset);
void __fastcall MoveParent(TWinControl *pNew, TWinControl *pOld, TWinControl *pCtr);
BYTE __fastcall InvFontCharset(BYTE charset);
void __fastcall InvFontCharset(TFont *pFont);

extern CExecPB g_ExecPB;
#endif

