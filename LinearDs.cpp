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
#include "LinearDs.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
//---------------------------------------------------------------------
__fastcall TLinearDspDlg::TLinearDspDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		Caption = "frequency characteristic";
	}
	pData = NULL;
}
//---------------------------------------------------------------------
void __fastcall TLinearDspDlg::Execute(double *D)
{
	pData = D;
	ShowModal();
}
//---------------------------------------------------------------------
void __fastcall TLinearDspDlg::PBoxPaint(TObject *Sender)
{
	PBox->Canvas->Font->Size = 8;
	int XL = PBox->Canvas->TextWidth("-16000") + 10;
	int YT = PBox->Canvas->TextHeight("-16000");
	int YB = PBox->Height - YT - 10;
	int XR = PBox->Width - (XL/2);

	int i, fq, x, y;
	char bf[32];
	PBox->Canvas->Pen->Color = clGray;
	for( fq = 1500, i = 0; i <= 16; i++, fq += 50 ){
		x = XL + i * (XR - XL)/16;
		PBox->Canvas->Pen->Style = i & 1 ? psDot : psSolid;
		PBox->Canvas->MoveTo(x, YT);
		PBox->Canvas->LineTo(x, YB);
		if( !(i & 1) ){
			sprintf(bf, "%4u", fq);
			int w = PBox->Canvas->TextWidth(bf)/2;
			PBox->Canvas->TextOut(x - w, YB + 5, bf);
		}
	}
	int vm;
	if( *pData >= 16000 ){
		vm = 20000;
	}
	else if( *pData >= 10000 ){
		vm = 16000;
	}
	else if( *pData >= 8000 ){
		vm = 10000;
	}
	else if( *pData >= 6000 ){
		vm = 8000;
	}
	else if( *pData >= 5000 ){
		vm = 6000;
	}
	else if( *pData >= 4000 ){
		vm = 5000;
	}
	else if( *pData >= 2000 ){
		vm = 4000;
	}
	else if( *pData >= 1000 ){
		vm = 2000;
	}
	else {
		vm = 1000;
	}
	fq = vm;
	for( i = 0; i <= 4; i++, fq -= (vm / 2) ){
		y = YT + i * (YB - YT)/4;
		PBox->Canvas->Pen->Style = i & 1 ? psDot : psSolid;
		PBox->Canvas->MoveTo(XL, y);
		PBox->Canvas->LineTo(XR, y);
		sprintf(bf, "%d", fq);
		int w = PBox->Canvas->TextWidth(bf);
		int h = PBox->Canvas->TextHeight(bf);
		if( i < 4 ) h /= 2;
		PBox->Canvas->TextOut(XL-w-3, y - h, bf);
	}
	if( pData == NULL ) return;

	PBox->Canvas->Pen->Color = clBlue;
	PBox->Canvas->Pen->Style = psSolid;
	for( i = 0; i <= 16; i++ ){
		x = XL + i * (XR - XL)/16;
		y = pData[i] * (YB - YT) / (vm * 2);
		y += (YB - YT)/2;
		y = YB - y;
		if( i ){
			PBox->Canvas->LineTo(x, y);
		}
		else {
			PBox->Canvas->MoveTo(x, y);
		}
	}
}
//---------------------------------------------------------------------------

