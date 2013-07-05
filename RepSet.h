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
#ifndef RepSetH
#define RepSetH
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
class TRepSetDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TEdit *AnsCW;
	TLabel *L1;
	TGroupBox *GB1;
	TLabel *L2;
	TLabel *L3;
	TLabel *L4;
	TLabel *L5;
	TEdit *TA;
	TEdit *TB;
	TEdit *TC;
	TEdit *TD;
	TLabel *L6;
	TLabel *L7;
	TLabel *L8;
	TLabel *L9;
	TButton *MBtn;
	TCheckBox *CBEnb;
	TTimer *Timer;
	TGroupBox *GB2;
	TLabel *LS;
	TRadioGroup *RGS;
	TGroupBox *GB3;
	TLabel *L11;
	TLabel *L12;
	TEdit *ETempT;
	TEdit *ETempB;
	TButton *BBtn;
	TGroupBox *GB4;
	TLabel *L10;
	TComboBox *CBMode;
	TEdit *ESQ;
	TLabel *L13;
	TLabel *L14;
	TComboBox *CBQT;
	TLabel *L15;
	TLabel *L16;
	TComboBox *CBTone;
	TLabel *L17;
	TButton *HBtn;
	TComboBox *CGB;
	TLabel *L18;
	TLabel *L19;
	TCheckBox *CBFLT;
	TLabel *L20;
	TEdit *EFD;
	TCheckBox *CBAS;
	void __fastcall MBtnClick(TObject *Sender);
	void __fastcall TimerTimer(TObject *Sender);
	
	void __fastcall BBtnClick(TObject *Sender);
	void __fastcall CBEnbClick(TObject *Sender);
	void __fastcall HBtnClick(TObject *Sender);
private:
	void __fastcall UpdateBtn(void);
	void __fastcall UpdateStat(void);

public:
	virtual __fastcall TRepSetDlg(TComponent* AOwner);

	int __fastcall Execute(void);
};
//----------------------------------------------------------------------------
//extern PACKAGE TRepSetDlg *RepSetDlg;
//----------------------------------------------------------------------------
#endif
