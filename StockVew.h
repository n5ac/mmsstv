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



//---------------------------------------------------------------------------
#ifndef StockVewH
#define StockVewH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
#include "ComLib.h"
//---------------------------------------------------------------------------
class TStockView : public TForm
{
__published:	// IDE 管理のコンポーネント
	void __fastcall FormResize(TObject *Sender);

	void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
		  TShiftState Shift);
	void __fastcall FormKeyUp(TObject *Sender, WORD &Key,
		  TShiftState Shift);
	
	
	void __fastcall FormHide(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall TStockView(TComponent* Owner);

	void __fastcall GetViewPos(int &x, int &y, int &w, int &h){
		x = Left;
		y = Top;
		w = ClientWidth;
		h = ClientHeight;
	};
	void __fastcall SetViewPos(int x, int y, int w, int h){
		Left = x;
		Top = y;
		ClientWidth = w;
		ClientHeight = h;
	};

	CAlignList  AlignList;
};
//---------------------------------------------------------------------------
//extern PACKAGE TStockView *StockView;
//---------------------------------------------------------------------------
#endif
