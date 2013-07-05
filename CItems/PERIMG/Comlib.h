#ifndef ComLibH
#define ComLibH
#include <ComCtrls.hpp>
#include <values.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define UseHWND 0

#define NDEBUG      // Remove this symbol, if you would like to do debug
#include <assert.h>
#define ASSERT(c)   assert(c)

#define AN(c)   (sizeof(c)/sizeof(c[0]))
#define ABS(c)	(((c)<0)?(-(c)):(c))
#define	PI	3.1415926535897932384626433832795

typedef const BYTE* LPCBYTE;
typedef double* LPDOUBLE;
typedef const double* LPCDOUBLE;

extern int MsgEng;
#define VER "PerImg Version 1.08"
#define AUT "(C) JE3HHT 2002."
#define TTL "MMSSTV custom item"
#define TTLJ "MMSSTV カスタムアイテム"  // Japanese code

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
extern mmLoadImageMenu  fLoadImageMenu;
extern mmLoadHistImage  fLoadHistImage;
extern mmMultProc fMultProc;


Graphics::TBitmap * __fastcall CreateBitmap(int xw, int yw);
void __fastcall CopyBitmap(Graphics::TBitmap *pDest, Graphics::TBitmap *pSrc);
void __fastcall DrawBitmap(TPaintBox *pBox, Graphics::TBitmap *pBitmap);
void __fastcall MultProc(void);

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
//Graphics::TBitmap* __fastcall Perspect(Graphics::TBitmap* pDest, Graphics::TBitmap *pSrc, LPCSPERSPECT pPar, TColor back);

class CItem {
public:
	SPERSPECT	m_sperspect;
	Graphics::TBitmap *m_pBitmap;
	TColor      m_Back;
	TMemoryStream   *m_pMemStream;
	int         m_SC;
	int         m_OrgXW;
	int         m_OrgYW;
	int         m_Draft;
	int         m_Frame;
	TColor      m_FrameCol;
public:
	__fastcall CItem(void);
	__fastcall ~CItem();
	void __fastcall Create(LPCBYTE ps, DWORD size);
	DWORD __fastcall GetItemType(void);
	LPCBYTE __fastcall CreateStorage(LPDWORD psize);
	void __fastcall DeleteStorage(LPCBYTE pStorage);
	void __fastcall Copy(CItem *sp);
	void __fastcall CopySource(Graphics::TBitmap *pSrc);
	void __fastcall CopyFromHistory(void);
	void __fastcall DeleteSource(void);
	DWORD __fastcall GetOrgSize(void);
	void __fastcall Draw(Graphics::TBitmap *pDest);
	inline void __fastcall SetDraft(DWORD draft){
		m_Draft = draft;
	};
	Graphics::TBitmap* __fastcall Perspect(Graphics::TBitmap* pDest, Graphics::TBitmap *pSrc);
};
//
//
typedef struct {
	DWORD			m_dwVersion;
	int				m_WinNT;
	int				m_MsgEng;
	mmLoadImageMenu m_fLoadImageMenu;
	mmLoadHistImage m_fLoadHistImage;
	mmMultProc		m_fMultProc;
	mmView  		m_fView;
}SYS;
extern SYS	sys;
//
//
//
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
#endif
