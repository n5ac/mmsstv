//----------------------------------------------------------------------------
#ifndef PerSpectDlgH
#define PerSpectDlgH
//----------------------------------------------------------------------------
#include <System.hpp>
#include <Windows.hpp>
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <Graphics.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Controls.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Clipbrd.hpp>
//----------------------------------------------------------------------------
#include "ComLib.h"
#include <Dialogs.hpp>
//----------------------------------------------------------------------------
class TPerDlgBox : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TTrackBar *TBAX;
	TTrackBar *TBAY;
	TTrackBar *TBRX;
	TTrackBar *TBRY;
	TTrackBar *TBRZ;
	TPaintBox *PBox;
	TTrackBar *TBS;
	TTrackBar *TBPX;
	TTrackBar *TBPY;
	TTrackBar *TBPZ;
	TLabel *L1;
	TLabel *L2;
	TLabel *L3;
	TLabel *L4;
	TLabel *L5;
	TLabel *L6;
	TLabel *L7;
	TLabel *L8;
	TLabel *L9;
	TButton *DefBtn;
	TRadioGroup *RGSRC;
	TTrackBar *TBR;
	TLabel *L10;
	TPanel *Panel;
	TSpeedButton *SB;
	TButton *ViewBtn;
	TSpeedButton *SBR;
	TButton *LoadBtn;
	TButton *PasteBtn;
	TLabel *L11;
	TComboBox *CBF;
	TPanel *PC;
	TLabel *L12;
	TColorDialog *ColorDialog;
	TSpeedButton *SBZ1;
	TSpeedButton *SBZ2;
	TSpeedButton *SBZ3;
	TSpeedButton *SBZ4;
	TSpeedButton *SBZ5;
	TSpeedButton *SBZ6;
	TSpeedButton *SBZ7;
	TSpeedButton *SBZ25;
	TSpeedButton *SBZ8;
	TSpeedButton *SBZ9;
	TSpeedButton *SBZ10;
	TSpeedButton *SBZ11;
	TSpeedButton *SBZ12;
	TSpeedButton *SBZ13;
	TSpeedButton *SBZ14;
	TSpeedButton *SBZ15;
	TSpeedButton *SBZ16;
	TSpeedButton *SBZ17;
	TSpeedButton *SBZ18;
	TSpeedButton *SBZ24;
	TSpeedButton *SBZ19;
	TSpeedButton *SBZ20;
	TSpeedButton *SBZ21;
	TSpeedButton *SBZ22;
	TSpeedButton *SBZ23;
	TSpeedButton *SBZ26;
	TSpeedButton *SBZ27;
	TSpeedButton *SBZ28;
	TSpeedButton *SBZ29;
	TSpeedButton *SBZ30;
	TSpeedButton *SBZ31;
	TSpeedButton *SBZ32;
	TSpeedButton *SBZ33;
	TSpeedButton *SBZ34;
	TSpeedButton *SBZ35;
	TSpeedButton *SBZ36;
	TSpeedButton *SBZ37;
	TSpeedButton *SBZ38;
	TScrollBar *SBSC;
	void __fastcall PBoxPaint(TObject *Sender);
	void __fastcall TBSChange(TObject *Sender);
	void __fastcall DefBtnClick(TObject *Sender);
	void __fastcall RGSRCClick(TObject *Sender);

	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall PBoxMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall PBoxMouseMove(TObject *Sender, TShiftState Shift,
		  int X, int Y);
	void __fastcall PBoxMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall LoadBtnClick(TObject *Sender);
	void __fastcall PasteBtnClick(TObject *Sender);
	void __fastcall FormActivate(TObject *Sender);
	void __fastcall FormDeactivate(TObject *Sender);
	void __fastcall ViewBtnClick(TObject *Sender);
	
	void __fastcall SBRClick(TObject *Sender);
	void __fastcall SBClick(TObject *Sender);
	void __fastcall CBFChange(TObject *Sender);
	void __fastcall PCClick(TObject *Sender);
	void __fastcall LDefMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBSCChange(TObject *Sender);
	
private:
	int         m_DisEvent;
	Graphics::TBitmap *m_pBitmap;
	CItem       m_Item;
	CItem       *m_pItem;
	TColor      m_Back;
	int         m_Wave;
	void __fastcall UpdateWaveBtn(int dir);
	void __fastcall UpdateBitmap(void);
	void __fastcall UpdateHint(void);
	void __fastcall TopMost(void);
	void __fastcall NoTopMost(void);

	int         m_XC, m_YC;
	double      m_SCX, m_SCY;
	double      m_Deg, m_SDeg;
	double      m_Dist, m_SDistX, m_SDistY;

	void __fastcall UpdateBtn(void);
	int         m_Loaded;

	void __fastcall UpdateTB(void);
	void __fastcall UpdateItem(void);
    int			m_SBar[10];
	int			m_DirTB[10];

    HWND		m_hClipNext;

public:
#if UseHWND
	virtual __fastcall TPerDlgBox(HWND hWnd);
#else
	virtual __fastcall TPerDlgBox(TComponent* AOwner);
#endif
	int __fastcall Execute(CItem *pItem);

protected:
	void __fastcall OnDrawClipboard(TMessage Message);
	void __fastcall OnChangeCbChain(TMessage Message);
BEGIN_MESSAGE_MAP
    MESSAGE_HANDLER(WM_DRAWCLIPBOARD, TMessage, OnDrawClipboard)
    MESSAGE_HANDLER(WM_CHANGECBCHAIN, TMessage, OnChangeCbChain)
END_MESSAGE_MAP(TForm)

};
//----------------------------------------------------------------------------
#endif
