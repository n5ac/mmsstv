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
//----------------------------------------------------------------------------
class TPerSpectDlg : public TForm
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
	TButton *LoadBtn;
	TButton *PasteBtn;
	TTimer *Timer;
	void __fastcall PBoxPaint(TObject *Sender);
	void __fastcall TBAXChange(TObject *Sender);
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
	void __fastcall TimerTimer(TObject *Sender);
	
private:
	int         m_DisEvent;
	Graphics::TBitmap *pBitmap;
	CObj        m_Obj;
	CObj        *m_pObj;
	TColor      m_Back;
	void __fastcall UpdateBitmap(void);

	int         m_XC, m_YC;
	double      m_Deg, m_SDeg;
	double      m_Dist, m_SDistX, m_SDistY;

	void __fastcall UpdateBtn(void);
	int         m_Loaded;
public:
	virtual __fastcall TPerSpectDlg(TComponent* AOwner);

	int __fastcall Execute(CObj *pObj);
};
//----------------------------------------------------------------------------
#endif
