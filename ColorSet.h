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
#ifndef ColorSetH
#define ColorSetH
//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
class TColorSetDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TRadioGroup *RGSet;
	TPanel *PC11;
	TPanel *PC12;
	TPanel *PC13;
	TPanel *PC14;
	TPanel *PC21;
	TPanel *PC22;
	TPanel *PC23;
	TPanel *PC24;
	TPanel *PC31;
	TPanel *PC32;
	TPanel *PC33;
	TPanel *PC34;
	TPanel *PC41;
	TPanel *PC42;
	TPanel *PC43;
	TPanel *PC44;
	TPanel *PC51;
	TPanel *PC52;
	TPanel *PC53;
	TPanel *PC54;
	TPanel *PC61;
	TPanel *PC62;
	TPanel *PC63;
	TPanel *PC64;
	TPanel *PC71;
	TPanel *PC72;
	TPanel *PC73;
	TPanel *PC74;
	TPanel *PC81;
	TPanel *PC82;
	TPanel *PC83;
	TPanel *PC84;
	void __fastcall PCClick(TObject *Sender);
private:
	TPanel *__fastcall GetItem(int n, int x);

public:
	virtual __fastcall TColorSetDlg(TComponent* AOwner);

	int __fastcall Execute(TColor *pcol);
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
