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
#include <vcl.h>
#pragma hdrstop

#include "StockVew.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//TStockView *StockView;
//---------------------------------------------------------------------------
__fastcall TStockView::TStockView(TComponent* Owner)
	: TForm(Owner)
{
	Font->Name = ((TForm *)Owner)->Font->Name;
	Font->Charset = ((TForm *)Owner)->Font->Charset;
}
//---------------------------------------------------------------------------
void __fastcall TStockView::FormResize(TObject *Sender)
{
	if( Visible ){
		if( this == Mmsstv->pStockView ){
			Mmsstv->AdjustStockView();
		}
		else {
			AlignList.NewAlign(this);
			Mmsstv->FormResize(NULL);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TStockView::FormKeyDown(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	Mmsstv->FormKeyDown(Sender, Key, Shift);
}
//---------------------------------------------------------------------------
void __fastcall TStockView::FormKeyUp(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	Mmsstv->FormKeyUp(Sender, Key, Shift);
}
//---------------------------------------------------------------------------
void __fastcall TStockView::FormHide(TObject *Sender)
{
	if( Mmsstv->Timer->Enabled ) Mmsstv->KVSDClick(NULL);
}
//---------------------------------------------------------------------------

