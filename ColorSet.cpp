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



//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "ComLib.h"
#include "ColorSet.h"
#include "Main.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//---------------------------------------------------------------------
__fastcall TColorSetDlg::TColorSetDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		Caption = "Color components";
		CancelBtn->Caption = "Cancel";
	}
}
//---------------------------------------------------------------------
TPanel *__fastcall TColorSetDlg::GetItem(int n, int x)
{
	TPanel *_tb[]={
		PC11, PC12, PC13, PC14,
		PC21, PC22, PC23, PC24,
		PC31, PC32, PC33, PC34,
		PC41, PC42, PC43, PC44,
		PC51, PC52, PC53, PC54,
		PC61, PC62, PC63, PC64,
		PC71, PC72, PC73, PC74,
		PC81, PC82, PC83, PC84,
	};
	return _tb[n * 4 + x];
}
//---------------------------------------------------------------------
int __fastcall TColorSetDlg::Execute(TColor *pcol)
{
	int i, x, f;
	int n = 0;
	int pos = -1;
	for( i = 0; i < 8; i++ ){
		f = 0;
		for( x = 0; x < 4; x++, n++ ){
			GetItem(i, x)->Color = sys.m_ColorSet[n];
			if( pcol[x] != sys.m_ColorSet[n] ) f++;
		}
		if( !f ){
			pos = i;
		}
	}
	RGSet->ItemIndex = pos;
	if( ShowModal() == IDOK ){
		n = 0;
		for( i = 0; i < 8; i++ ){
			for( x = 0; x < 4; x++, n++ ){
				sys.m_ColorSet[n] = GetItem(i, x)->Color;
			}
		}
		if( RGSet->ItemIndex >= 0 ){
			for( x = 0; x < 4; x++ ){
				pcol[x] = GetItem(RGSet->ItemIndex, x)->Color;
			}
			return TRUE;
		}
	}
	return FALSE;
}
//---------------------------------------------------------------------
void __fastcall TColorSetDlg::PCClick(TObject *Sender)
{
	int i, x, pos;
	pos = -1;
	for( i = 0; (i < 8) && (pos < 0); i++ ){
		for(x = 0; x < 4; x++ ){
			if( Sender == GetItem(i, x) ){
				pos = i;
				break;
			}
		}
	}
	TColorDialog *pDialog = Mmsstv->ColorDialog;
	InitCustomColor(pDialog);

	TPanel *pPanel[4];
    for( i = 0; i < 4; i++ ){
		pPanel[i] = GetItem(pos, i);
		AddCustomColor(pDialog, pPanel[i]->Color);
    }

	pDialog->Color = ((TPanel *)Sender)->Color;
	SetDisPaint();
	if( pDialog->Execute() == TRUE ){
		((TPanel *)Sender)->Color = pDialog->Color;
		if( (pPanel[0]->Color == pPanel[1]->Color) && (pPanel[2]->Color == pPanel[3]->Color) ){
			if( YesNoMB( MsgEng ? "Change to two colors gradation?" : "2色のグラデーションに変更しますか?" ) == IDYES ){
				pPanel[1]->Color = GetCol(pPanel[0]->Color, pPanel[3]->Color, 2, 4);
				pPanel[2]->Color = GetCol(pPanel[0]->Color, pPanel[3]->Color, 3, 4);
			}
		}
	}
	ResDisPaint();
}
//---------------------------------------------------------------------------

