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
#ifndef ExtCmdDlgH
#define ExtCmdDlgH
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
class TExtCmdDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TEdit *EM;
	TEdit *EC;
	TCheckBox *CBR;
	TLabel *L1;
	TLabel *L2;
	TButton *RefBtn;
	void __fastcall RefBtnClick(TObject *Sender);
private:
public:
	virtual __fastcall TExtCmdDlg(TComponent* AOwner);

	int __fastcall Execute(AnsiString &Name, AnsiString &Cmd, int &Susp);
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
