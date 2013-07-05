//---------------------------------------------------------------------------
#ifndef CodeVwH
#define CodeVwH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>

#include "ComLib.h"
#include <ComCtrls.hpp>
#define	WM_FORMCLOSE	(WM_USER+410)
#define	WM_CODEINSERT	(WM_USER+411)
//---------------------------------------------------------------------------
class TCodeView : public TForm
{
__published:	// IDE 管理のコンポーネント
	TPanel *Panel;
	TPaintBox *PBox;
	TSpeedButton *SBClose;
	TSpeedButton *SBClear;
	TSpeedButton *SBFont;
	TSpeedButton *SBOut;
	TSpeedButton *SBEdit;
	TSpeedButton *SBA;
	TSpeedButton *SBB;
	TSpeedButton *SBC;
	TSpeedButton *SBUS;
	TSpeedButton *SBJA;
	TSpeedButton *SBHL;
	TSpeedButton *SBBY;
	TSpeedButton *SBBV;
	TUpDown *UDMB;
    void __fastcall PBoxPaint(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall SBCloseClick(TObject *Sender);
	void __fastcall PBoxMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBClearClick(TObject *Sender);
	void __fastcall SBFontClick(TObject *Sender);
	
	void __fastcall SBOutClick(TObject *Sender);
	void __fastcall SBEditClick(TObject *Sender);
	void __fastcall SBAClick(TObject *Sender);
	
	void __fastcall SBUSClick(TObject *Sender);
	void __fastcall UDMBClick(TObject *Sender, TUDBtnType Button);
	void __fastcall PBoxMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
private:	// ユーザー宣言
	HWND	m_hWnd;
    TFont	*m_pFont;
	int		m_XW1;
    int		m_YW1;
	int		m_XW;
    int		m_YW;
	int		m_MouseDown;
    int		m_Base;
    BOOL	m_tPfx[256];
    int		m_pfxCount;
    int		m_tPfxIdx[256];
	void __fastcall UpdateTitle(void);
	void __fastcall ReqClose(void);
	void __fastcall UpdateXW(void);
	void __fastcall SetPBoxFont(TCanvas *pCanvas);
	void __fastcall DrawChar(TCanvas *pCanvas, int c, BOOL f);
	void __fastcall DrawCursor(TCanvas *pCanvas, int c, BOOL f);
	WORD __fastcall GetEUDC(int y, int x);
	void __fastcall CheckFontCharset(void);

public:		// ユーザー宣言
	__fastcall TCodeView(TComponent* Owner);

	void __fastcall Execute(HWND hWnd, TFont *pFont);
};
//---------------------------------------------------------------------------
//extern PACKAGE TCodeView *CodeView;
//---------------------------------------------------------------------------
#endif
