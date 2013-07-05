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
#ifndef InputWinH
#define InputWinH
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
class TInputWinDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TEdit *Edit;
	TLabel *LMsg;
	TButton *BMac;
	void __fastcall BMacClick(TObject *Sender);
private:
public:
	virtual __fastcall TInputWinDlg(TComponent* AOwner);
};
int InputMB(LPCSTR pTitle, LPCSTR pMsg, AnsiString &as, BOOL fMac);
int InputMB(LPCSTR pTitle, LPCSTR pMsg, AnsiString &as);
//----------------------------------------------------------------------------
#endif

