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
#ifndef MmcgDlgH
#define MmcgDlgH
//----------------------------------------------------------------------------
//ja7ude 0428
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
#include <Grids.hpp>
//----------------------------------------------------------------------------
#include "Mmcg.h"
//----------------------------------------------------------------------------
class TMmcgDlgBox : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TStringGrid *Grid;
	TEdit *EditYomi;
	TSpeedButton *SBMask;
	TSpeedButton *SBInc;void __fastcall EditYomiChange(TObject *Sender);
	
	
	
	
	void __fastcall EditYomiKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall EditYomiKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall SBMaskClick(TObject *Sender);
	void __fastcall SBIncClick(TObject *Sender);
	void __fastcall EditYomiKeyPress(TObject *Sender, char &Key);
	void __fastcall GridDrawCell(TObject *Sender, int Col, int Row,
		  TRect &Rect, TGridDrawState State);
private:
	int		m_DisEvent;

	void __fastcall UpdateCaption(void);
public:
	virtual __fastcall TMmcgDlgBox(TComponent* AOwner);

	int __fastcall Execute(AnsiString &call, AnsiString &qth, AnsiString &op);
};
//----------------------------------------------------------------------------
//extern TMmcgDlgBox *MmcgDlgBox;
//----------------------------------------------------------------------------
#endif    
