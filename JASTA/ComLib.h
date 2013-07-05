#ifndef ComLibH
#define ComLibH
//ja7ude 0525
#include <ComCtrls.hpp>
#include <inifiles.hpp>
#include <Grids.hpp>
#include <values.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mbstring.h>
#include <dir.h>

#define DEBUG   0
#if DEBUG
#include <assert.h>
#define ASSERT(c)   assert(c)
#else
#define ASSERT(c)
#endif

#define VERBETA ""
#define	VERID	"Ver 1.14"
#define	VERTTL2	"MMJASTA "VERID VERBETA
#define	VERTTL  VERTTL2" (C) JE3HHT 2002-2010."

extern  LCID lcid;

extern  int     MsgEng;
extern	char	BgnDir[256];
extern  char    MMLogDir[256];

extern	const char	MONN[];
extern	const char	MONU[];

#ifndef LPCUSTR
typedef const unsigned char *	LPCUSTR;
typedef unsigned char *	LPUSTR;
#endif

enum TFontPitch { fpDefault, fpVariable, fpFixed };	//ja7ude 0525

#define ABS(c)	(((c)<0)?(-(c)):(c))
#define AN(p)	(sizeof(p)/sizeof(*(p)))
#define	CR		0x0d
#define	LF		0x0a
#define	TAB		'\t'

typedef struct {
	int         m_HelpNotePad;
	int         m_Year;
	AnsiString  m_CName;

	AnsiString  m_Call;
	int         m_First;
	int         m_Category;
	AnsiString  m_Name;
	AnsiString  m_Zip;
	AnsiString  m_Adr;
	AnsiString  m_Mail;
	AnsiString  m_License;
	AnsiString  m_Pow;
    BOOL		m_Sexual;
    int			m_TSize;		// S, M, L, LL, アクテイブ賞のＴ－シャツ当選時の希望サイズ
}SYSSET;
extern SYSSET   sys;
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

///---------------------------------------------------------
///  テキストバッファストリーマー
class StrText{
public:
	char	*Bp;
	char	*Wp;
	inline StrText(int max){
		Bp = new char[max];
		Clear();
	};
	inline ~StrText(){
		delete Bp;
	};
	inline char *Printf(char *ct, ...){
		va_list	pp;

		va_start(pp, ct);
		vsprintf(Wp, ct, pp );
		va_end(pp);
		ct = Wp;
		Wp += strlen(Wp);
		return(ct);
	};
	inline void Add(LPCSTR sp){
		strcpy(Wp, sp);
		Wp += strlen(Wp);
	};
	inline void Write(void *p, int n){
		memcpy(Wp, p, n);
		Wp += n;
		*Wp = 0;
	};
	inline int GetCnt(void){
		return Wp - Bp;
	};
	inline void Clear(void){
		Wp = Bp;
		*Wp = 0;
	};
	inline char *GetText(void){
		return Bp;
	};
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

int IsFile(LPCSTR pName);

LPSTR StrDupe(LPCSTR s);

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
void SetCurDir(LPSTR t, int size);
void SetDirName(LPSTR t, LPCSTR pName);
void ClipLF(LPSTR sp);
void DeleteComment(LPSTR bf);
LPSTR FillSpace(LPSTR s, int n);
LPSTR SkipSpace(LPSTR sp);
LPCSTR SkipSpace(LPCSTR sp);
LPSTR DelLastSpace(LPSTR t);
LPSTR StrDlm(LPSTR &t, LPSTR p);
LPSTR StrDlm(LPSTR &t, LPSTR p, char c);
void ChgString(LPSTR t, char a, char b);
void DelChar(LPSTR t, char a);
int atoin(const char *p, int n);
int htoin(const char *p, int n);

int InvMenu(TMenuItem *pItem);

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

void NumCopy(LPSTR t, LPCSTR p);
int IsNumbs(LPCSTR p);
int IsNumbAll(LPCSTR p);
int IsAlphas(LPCSTR p);
int IsRST(LPCSTR p);
int IsCallChar(char c);
int IsCall(LPCSTR p);
int IsJA(const char *s);
LPCSTR ClipCall(LPCSTR s);
LPCSTR ClipCC(LPCSTR s);

/* ja7ude 0525
inline LPUSTR StrDlm(LPUSTR &t, LPUSTR p, char c){return (LPUSTR)StrDlm(LPSTR(t), LPSTR(p), c);};
inline LPUSTR StrDlm(LPUSTR &t, LPUSTR p){return (LPUSTR)StrDlm(LPSTR(t), LPSTR(p));};
inline LPUSTR StrDlm(LPUSTR &t, LPSTR p, char c){return (LPUSTR)StrDlm(LPSTR(t), p, c);};
inline LPUSTR StrDlm(LPUSTR &t, LPSTR p){return (LPUSTR)StrDlm(LPSTR(t), p);};
*/

void __fastcall SetGroupEnabled(TGroupBox *gp);
void KeyEvent(const short *p);

typedef struct {
	LPSTR   pStr;
	int     Count;
}MULTSET;

class CMULT
{
private:
	int     m_CNT;
	int     m_MAX;
	MULTSET *m_pBase;
//    LPSTR   *m_pStr;
//    LPSTR   m_pStr[MULTMAX];
private:
	void Alloc(void);

public:
	CMULT();
	~CMULT(){
		Clear();
	};
	void Clear(void);
	int Add(LPCSTR pKey);
	int Set(LPCSTR pKey, int n);
	int GetCount(void){
		return m_CNT;
	};
	void Sort(void);
	void SortCount(void);
	LPCSTR  GetText(int n){
		if( (n >= 0) && (n < m_CNT) ){
			return m_pBase[n].pStr;
		}
		else {
			return NULL;
		}
	};
	int GetCount(int n){
		if( (n >= 0) && (n < m_CNT) ){
			return m_pBase[n].Count;
		}
		else {
			return 0;
		}
	};
	int GetCount(LPCSTR pKey);
	int GetTotal(void);
};
#endif

