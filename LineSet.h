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
#ifndef LineSetH
#define LineSetH
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
#include "Draw.h"
//----------------------------------------------------------------------------
class TLineSetDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TLabel *LS;
	TLabel *LW;
	TComboBox *LineStyle;
	TComboBox *LineWidth;
	TLabel *LC;
	TPanel *PCol;
	TLabel *LT;
	TComboBox *BoxStyle;
	void __fastcall PColClick(TObject *Sender);
	
	void __fastcall LineStyleChange(TObject *Sender);
private:
	int     m_lmode;
	void __fastcall UpdateUI(void);

public:
	virtual __fastcall TLineSetDlg(TComponent* AOwner);

	int __fastcall Execute(CDraw *pItem);
};
//----------------------------------------------------------------------------
//extern PACKAGE TLineSetDlg *LineSetDlg;
//----------------------------------------------------------------------------
#endif
