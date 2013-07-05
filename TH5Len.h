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
#ifndef OCBH
#define OCBH
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
//----------------------------------------------------------------------------
class TTH5LenDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TLabel *L1;
	TLabel *L2;
	TLabel *L3;
	TLabel *L4;
	TLabel *L5;
	TLabel *L6;
	TLabel *L7;
	TEdit *EHis;
	TEdit *EMy;
	TEdit *EFreq;
	TEdit *EMode;
	TEdit *EName;
	TEdit *EQTH;
	TEdit *ERem;
	TUpDown *UHis;
	TUpDown *UMy;
	TUpDown *UFreq;
	TUpDown *UMode;
	TUpDown *UName;
	TUpDown *UQTH;
	TUpDown *URem;
private:
public:
	virtual __fastcall TTH5LenDlg(TComponent* AOwner);

	int __fastcall Execute(void);
};
//----------------------------------------------------------------------------
//extern PACKAGE TTH5LenDlg *TH5LenDlg;
//----------------------------------------------------------------------------
#endif
