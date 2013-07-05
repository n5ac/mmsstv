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
#include "CodeVw.h"
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
	TSpeedButton *SBA4;
	TMemo *Msg;
	TSpeedButton *SBMSG;
	TLabel *L33;
	TComboBox *CBOYW;
	TUpDown *UDOYW;
	TLabel *L34;
	TComboBox *CBRot;
	TUpDown *UDRot;
	TSpeedButton *SBF1;
	TSpeedButton *SBF2;
	TSpeedButton *SBF3;
	TSpeedButton *SBF4;
	TSpeedButton *SBF5;
	TSpeedButton *SBB;
	TSpeedButton *SBI;
	TSpeedButton *SBL;
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
	void __fastcall SBMSGClick(TObject *Sender);
	void __fastcall MsgDblClick(TObject *Sender);
	void __fastcall MemoStartDrag(TObject *Sender,
		  TDragObject *&DragObject);
	void __fastcall MsgDragDrop(TObject *Sender, TObject *Source, int X,
		  int Y);
	void __fastcall MsgDragOver(TObject *Sender, TObject *Source, int X,
		  int Y, TDragState State, bool &Accept);
	void __fastcall MemoMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	
	void __fastcall MemoMouseMove(TObject *Sender, TShiftState Shift,
		  int X, int Y);
	void __fastcall MemoMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall PBoxMouseMove(TObject *Sender, TShiftState Shift, int X,
		  int Y);
	void __fastcall PBoxMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall FontBtnMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBFClick(TObject *Sender);
	void __fastcall SBFMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBBClick(TObject *Sender);
	void __fastcall SBIClick(TObject *Sender);
	void __fastcall SBFDMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBLClick(TObject *Sender);
private:
	int     m_DisEvent;
	int		m_DropProfile;
	Graphics::TBitmap *m_pBitmap;
	CItem   m_Item;
	int     m_Change;
	AnsiString  m_FText;
	int		m_MouseXC, m_MouseYC, m_MouseHold;
	int     m_MemoX, m_MemoY, m_MemoDown;

	void __fastcall TopMost(void);
	void __fastcall NoTopMost(void);
	void __fastcall UpdateMemoFont(BYTE charset);
	void __fastcall UpdateFText(void);
	void __fastcall UpdateUI(void);
	void __fastcall UpdateItem(CItem *pItem);
	void __fastcall UpdateDialog(CItem *pItem);
	void __fastcall UpdateBitmap(void);
	void __fastcall UD3D(int a);
	void __fastcall UpdateFontBtnHint(void);
	int __fastcall GetSBFIndex(TObject *Sender);
	void __fastcall SetSBF(int n);

    TCodeView	*m_pCodeView;
public:
#if UseHWND
	virtual __fastcall TTextDlgBox(HWND hWnd);
#else
	virtual __fastcall TTextDlgBox(TComponent* AOwner);
#endif

	int __fastcall Execute(CItem *pItem);

protected:
	void __fastcall OnCodeViewClose(TMessage Message);
	void __fastcall OnCodeInsert(TMessage Message);
BEGIN_MESSAGE_MAP
    MESSAGE_HANDLER(WM_FORMCLOSE, TMessage, OnCodeViewClose)
    MESSAGE_HANDLER(WM_CODEINSERT, TMessage, OnCodeInsert)
END_MESSAGE_MAP(TForm)
};
//----------------------------------------------------------------------------
//extern PACKAGE TTextDlgBox *TextDlgBox;
//----------------------------------------------------------------------------
#endif

