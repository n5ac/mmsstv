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



//----------------------------------------------------------------------------
#ifndef TextInH
#define TextInH
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
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
//----------------------------------------------------------------------------
#include "Draw.h"
//----------------------------------------------------------------------------
class TTextInDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TLabel *Ltext;
	TGroupBox *GBEf;
	TRadioGroup *RGShadow;
	TPanel *PCS;
	TButton *FontBtn;
	TRadioGroup *RGGrade;
	TPanel *PC1;
	TPanel *PC2;
	TPanel *PC3;
	TPanel *PC4;
	TButton *CmdBtn;
	TButton *TblBtn;
	TComboBox *EditText;
	TCheckBox *CBZero;
	TLabel *LRot;
	TComboBox *Rot;
	TButton *MaskBtn;
	TPanel *PCB;
	TCheckBox *CBRA;
	TPaintBox *PBox;
	TTimer *Timer;
	TUpDown *UDRot;
	TCheckBox *CBTF;
	TGroupBox *GBP;
	TComboBox *CMBPro;
	TButton *MemBtn;
	TButton *DelBtn;
	TButton *LineBtn;
	TButton *TFBtn;
	TCheckBox *CBStack;
	TEdit *SEX;
	TLabel *L1;
	TUpDown *UDSX;
	TLabel *L2;
	TEdit *SEY;
	TUpDown *UDSY;
	TCheckBox *CBV;
	TEdit *EV;
	TUpDown *UDV;
	TCheckBox *CBFont;
	TMemo *Memo;
	TSpeedButton *SBMSG;
	TSpeedButton *SBFI;
	TSpeedButton *SBFD;
	TSpeedButton *SBF1;
	TSpeedButton *SBF2;
	TSpeedButton *SBF3;
	TSpeedButton *SBF4;
	TSpeedButton *SBB;
	TSpeedButton *SBI;
	void __fastcall RBGradeClick(TObject *Sender);
	void __fastcall PCClick(TObject *Sender);
	void __fastcall FontBtnClick(TObject *Sender);
	void __fastcall CmdBtnClick(TObject *Sender);
	void __fastcall TblBtnClick(TObject *Sender);
	void __fastcall EditTextChange(TObject *Sender);
	void __fastcall MaskBtnClick(TObject *Sender);
	
	void __fastcall TFBtnClick(TObject *Sender);
	void __fastcall PBoxPaint(TObject *Sender);
	void __fastcall CBZeroClick(TObject *Sender);
	void __fastcall TimerTimer(TObject *Sender);
	
	void __fastcall UDRotClick(TObject *Sender, TUDBtnType Button);
	void __fastcall MemBtnClick(TObject *Sender);
	void __fastcall CMBProChange(TObject *Sender);
	
	void __fastcall DelBtnClick(TObject *Sender);
	void __fastcall LineBtnClick(TObject *Sender);
	
	void __fastcall CMBProDropDown(TObject *Sender);
	void __fastcall CMBProKeyPress(TObject *Sender, char &Key);
	void __fastcall EditTextDropDown(TObject *Sender);
	void __fastcall EditTextKeyPress(TObject *Sender, char &Key);
	void __fastcall UDSXClick(TObject *Sender, TUDBtnType Button);
	void __fastcall CBVClick(TObject *Sender);
	void __fastcall CBFontClick(TObject *Sender);
	void __fastcall PBoxMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	
	
	void __fastcall SBMSGClick(TObject *Sender);
	void __fastcall MemoDblClick(TObject *Sender);
	
	void __fastcall PBoxMouseMove(TObject *Sender, TShiftState Shift, int X,
		  int Y);
	void __fastcall PBoxMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall FontBtnMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBFClick(TObject *Sender);
	void __fastcall SBFMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBIClick(TObject *Sender);
	void __fastcall SBBClick(TObject *Sender);
	void __fastcall SBFDMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	
	
private:
	int             m_DisEvent;
	TFont           *pFont;
	CDrawText       *pItem;
	Graphics::TBitmap *pBrushBitmap;
	int             m_DropText;
	int             m_ReqChange;
	CDrawText       m_Text;
	int             m_DropProfile;

	int				m_MouseXC, m_MouseYC, m_MouseHold;

    TFontDialog		*m_pFontDialog;
	void __fastcall UpdateBoxFont(void);
	void __fastcall UpdateUI(void);
	void __fastcall UpdateItem(CDrawText *pm, int fsw);
	void __fastcall UpdatePara(CDrawText *pm);
	void __fastcall AddTextList(LPCSTR pStr);
	void __fastcall UpdateBitmap(void);
	void __fastcall UpdateSample(void);
	void __fastcall InitCMB(void);
	void __fastcall SaveCMB(void);

	int __fastcall GetSBFIndex(TObject *Sender);
	void __fastcall UpdateSBF(void);
	void __fastcall AddStyle(AnsiString &as, BYTE charset, DWORD style);
	TSpeedButton	*m_tSBF[4];

    DWORD			m_Style;
public:
	virtual __fastcall TTextInDlg(TComponent* AOwner);
	__fastcall ~TTextInDlg();

	int __fastcall Execute(CDrawText *p);
};
//----------------------------------------------------------------------------
//extern PACKAGE TTextInDlg *TextInDlg;
//----------------------------------------------------------------------------
#endif
