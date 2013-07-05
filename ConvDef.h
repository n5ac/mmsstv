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
#ifndef ConvDefH
#define ConvDefH
//----------------------------------------------------------------------------
// JA7UDE 0428
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <StdCtrls.hpp>
#include <Controls.hpp>
#include <Forms.hpp>
#include <Graphics.hpp>
#include <Classes.hpp>
#include <SysUtils.hpp>
#include <Windows.hpp>
#include <System.hpp>
#include <Grids.hpp>

//----------------------------------------------------------------------------
#include "LogConv.h"
//----------------------------------------------------------------------------
#define	PREREADMAX	100
class TConvDefDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TStringGrid *Grid;
	TComboBox *ConvText;
	TLabel *LConv;
	TGroupBox *GrpRef;
	TSpeedButton *SBBack;
	TSpeedButton *SBNext;
	TRadioGroup *ConvSel;
	TButton *InitBtn;
	TButton *UpBtn;
	TButton *DownBtn;
	TButton *InitMax;
	TCheckBox *CheckUTC;
	TCheckBox *CheckDBL;void __fastcall ConvTextChange(TObject *Sender);
	void __fastcall SBBackClick(TObject *Sender);
	void __fastcall SBNextClick(TObject *Sender);


	void __fastcall InitBtnClick(TObject *Sender);
	void __fastcall UpBtnClick(TObject *Sender);
	void __fastcall DownBtnClick(TObject *Sender);
	void __fastcall InitMaxClick(TObject *Sender);
	void __fastcall ConvSelClick(TObject *Sender);
	void __fastcall GridSetEditText(TObject *Sender, int ACol, int ARow,
		  const AnsiString Value);
	void __fastcall GridSelectCell(TObject *Sender, int Col, int Row,
		  bool &CanSelect);
	void __fastcall GridGetEditText(TObject *Sender, int ACol, int ARow,
		  AnsiString &Value);
	void __fastcall GridDrawCell(TObject *Sender, int Col, int Row,
		  TRect &Rect, TGridDrawState State);
private:
	TCONV	Conv[TEXTCONVMAX];

	int		m_Mode;
	int		m_DisEvent;

	int		m_Bgn;
	int		m_End;
	int		m_Cur;

	void __fastcall UpdateUI(int n);
	void __fastcall SetupComBox(void);

	AnsiString	m_text[PREREADMAX];
	AnsiString	m_line[TEXTCONVMAX];

	void __fastcall LoadText(LPCSTR pName);
	void __fastcall UpdateLine(void);

public:
	virtual __fastcall TConvDefDlg(TComponent* AOwner);

	int __fastcall Execute(TCONV *tp, int &delm, int &utc, int &dbl, int b, int e);
	int __fastcall Execute(TCONV *tp, int &delm, int &utc, int &dbl, LPCSTR pName);
};
//----------------------------------------------------------------------------
//extern TConvDefDlg *ConvDefDlg;
//----------------------------------------------------------------------------
#endif
