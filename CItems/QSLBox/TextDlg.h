//----------------------------------------------------------------------------
#ifndef TextDlgH
#define TextDlgH
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
//----------------------------------------------------------------------------
#include "ComLib.h"
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
//----------------------------------------------------------------------------
class TTextDlgBox : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TButton *FontBtn;
	TMemo *Memo;
	TGroupBox *GBG;
	TSpeedButton *SBM;
	TSpeedButton *SBG1;
	TSpeedButton *SBG2;
	TSpeedButton *SBG3;
	TSpeedButton *SBG4;
	TPanel *PG1;
	TPanel *PG2;
	TPanel *PG3;
	TPanel *PG4;
	TSpeedButton *SBC;
	TGroupBox *GBS;
	TSpeedButton *SBS1;
	TSpeedButton *SBS2;
	TSpeedButton *SBS3;
	TSpeedButton *SBS4;
	TPanel *PB1;
	TPanel *PB2;
	TPanel *PB3;
	TUpDown *UDL;
	TGroupBox *GB3D;
	TCheckBox *CB3D;
	TUpDown *UDY;
	TUpDown *UDX;
	TSpeedButton *SB3DI;
	TSpeedButton *SB3DD;
	TLabel *L1;
	TLabel *LY;
	TLabel *LX;
	TGroupBox *GBA;
	TSpeedButton *SBA1;
	TSpeedButton *SBA2;
	TSpeedButton *SBA3;
	TCheckBox *CBFixed;
	TCheckBox *CBZero;
	TComboBox *CBList;
	TSpeedButton *SBLW;
	TSpeedButton *SBLD;
	TGroupBox *GBY;
	TComboBox *CBY;
	TLabel *L2;
	TSpeedButton *SBYW;
	TSpeedButton *SBYD;
	TPaintBox *PBox;
	TFontDialog *FontDialog;
	TColorDialog *ColorDialog;
	TTimer *Timer;
	TSpeedButton *SBFD;
	TSpeedButton *SBFI;
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall PBoxPaint(TObject *Sender);
	void __fastcall MemoChange(TObject *Sender);
	void __fastcall TimerTimer(TObject *Sender);
	void __fastcall SBG1Click(TObject *Sender);
	void __fastcall PB1Click(TObject *Sender);
	void __fastcall FormActivate(TObject *Sender);
	void __fastcall FormDeactivate(TObject *Sender);
	void __fastcall UDLClick(TObject *Sender, TUDBtnType Button);
	void __fastcall FontBtnClick(TObject *Sender);
	void __fastcall SBMClick(TObject *Sender);
	void __fastcall SBCClick(TObject *Sender);
	void __fastcall SBLWClick(TObject *Sender);
	void __fastcall SBLDClick(TObject *Sender);
	void __fastcall CBListChange(TObject *Sender);
	void __fastcall SBYWClick(TObject *Sender);
	void __fastcall SBYDClick(TObject *Sender);
	void __fastcall CBYChange(TObject *Sender);
	void __fastcall CBYKeyPress(TObject *Sender, char &Key);
	void __fastcall CBYDropDown(TObject *Sender);
	void __fastcall PBoxMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);

	void __fastcall SB3DIClick(TObject *Sender);
	void __fastcall SBFDClick(TObject *Sender);
private:
	int     m_DisEvent;
	int		m_DropProfile;
	Graphics::TBitmap *m_pBitmap;
	CItem   m_Item;
	int     m_Change;
	AnsiString  m_FText;
	void __fastcall TopMost(void);
	void __fastcall NoTopMost(void);
	void __fastcall UpdateMemoFont(BYTE charset);
	void __fastcall UpdateFText(void);
	void __fastcall UpdateUI(void);
	void __fastcall UpdateItem(CItem *pItem);
	void __fastcall UpdateDialog(CItem *pItem);
	void __fastcall UpdateBitmap(void);
	void __fastcall UD3D(int a);

public:
	virtual __fastcall TTextDlgBox(HWND hWnd);

	int __fastcall Execute(CItem *pItem);
};
//----------------------------------------------------------------------------
//extern PACKAGE TTextDlgBox *TextDlgBox;
//----------------------------------------------------------------------------
#endif
  