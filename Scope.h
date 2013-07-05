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
#ifndef ScopeH
#define ScopeH
//----------------------------------------------------------------------------
/* ja7ude 0428	*/
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
#include "sstv.h"
class TTScope : public TForm
{
__published:
	TButton *OKBtn;
	TPanel *Panel1;
	TPaintBox *PaintBox;
	TTimer *Timer;
	TSpeedButton *SBTrig;
	TButton *LeftBtn;
	TButton *RightBtn;
	TButton *UpBtn;
	TButton *DownBtn;
	TButton *SBDownW;
	TButton *SBUpW;
	TSpeedButton *SBSet;
	TButton *SBUpDown;
	TSpeedButton *SBPlay;
	void __fastcall PaintBoxPaint(TObject *Sender);
	void __fastcall TimerTimer(TObject *Sender);
	void __fastcall SBTrigClick(TObject *Sender);
	void __fastcall LeftBtnClick(TObject *Sender);
	void __fastcall RightBtnClick(TObject *Sender);
	void __fastcall UpBtnClick(TObject *Sender);
	void __fastcall DownBtnClick(TObject *Sender);
	
	
	void __fastcall SBDownWClick(TObject *Sender);
	void __fastcall SBUpWClick(TObject *Sender);
	
	void __fastcall PaintBoxMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	
	void __fastcall SBUpDownClick(TObject *Sender);
	
	void __fastcall SBSetClick(TObject *Sender);
	void __fastcall SBPlayClick(TObject *Sender);
private:
	CSSTVDEM    *pDem;
	CScope      *pScope;

	Graphics::TBitmap *pBitmap;

	void __fastcall UpdateBtn(void);
	void __fastcall TrigNext(void);
	void __fastcall PaintCursor(void);
	void __fastcall AdjXoff(int x);


public:
	virtual __fastcall TTScope(TComponent* AOwner);
	virtual __fastcall ~TTScope();

	void __fastcall Execute(CSSTVDEM *pd);
	void __fastcall PaintPosition(void);
	void __fastcall PaintScope(CScope *sp, int n);
};
//----------------------------------------------------------------------------
//extern TTScope *TScope;
//----------------------------------------------------------------------------
#endif    
