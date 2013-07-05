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
#ifndef RMenuDlgH
#define RMenuDlgH
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
//----------------------------------------------------------------------------
class TRMenuDialog : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TLabel *L1;
	TLabel *L2;
	TEdit *ETTL;
	TEdit *ECMD;
	TSpeedButton *SBDel;
	TLabel *LH;
	TSpeedButton *SBIns;
	void __fastcall SBDelClick(TObject *Sender);
	void __fastcall SBInsClick(TObject *Sender);
	
	void __fastcall SBRefClick(TObject *Sender);
private:
    void __fastcall UpdateUI(void);
public:
	virtual __fastcall TRMenuDialog(TComponent* AOwner);

	int __fastcall Execute(AnsiString &strTTL, AnsiString &strCMD, int fAdd = FALSE);
};
//----------------------------------------------------------------------------
//extern PACKAGE TRMenuDialog *RMenuDialog;
//----------------------------------------------------------------------------
#endif
