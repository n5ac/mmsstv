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

#include "FreqDisp.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
TFreqDispDlg *FreqDispDlg;
//---------------------------------------------------------------------
__fastcall TFreqDispDlg::TFreqDispDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;

	m_H1 = NULL;
	m_Tap1 = 0;
	m_Over1 = 1;
	m_H2 = NULL;
	m_Tap2 = 0;
	m_Over2 = 1;
	m_Max = 0;

	pBitmap = new Graphics::TBitmap();
	pBitmap->Width = PaintBox->Width;
	pBitmap->Height = PaintBox->Height;
	if( MsgEng ){
		CancelBtn->Caption = "Close";
		BwBtn->Caption = "Width";
		Caption = "frequency characteristic";
	}
}
__fastcall TFreqDispDlg::~TFreqDispDlg()
{
	delete pBitmap;
}
//---------------------------------------------------------------------
void __fastcall TFreqDispDlg::Execute(const double *H, int Tap, int Over)
{
	m_type = 0;
	m_H1 = H;
	m_Tap1 = Tap;
	m_Over1 = Over;
	m_Tap2 = 0;
	ShowModal();
}
//---------------------------------------------------------------------
void __fastcall TFreqDispDlg::Execute(const double *H1, const double *H2, int Tap, int Over)
{
	m_type = 0;
	m_H1 = H1;
	m_Tap1 = Tap;
	m_Over1 = Over;
	m_H2 = H2;
	m_Tap2 = Tap;
	m_Over2 = Over;
	ShowModal();
}
//---------------------------------------------------------------------
void __fastcall TFreqDispDlg::Execute(double a10, double b11, double b12, double a20, double b21, double b22, int Over)
{
	m_type = 1;
	m_Over1 = Over;
	m_a10 = a10;
	m_b11 = b11;
	m_b12 = b12;
	m_a20 = a20;
	m_b21 = b21;
	m_b22 = b22;
	ShowModal();
}
//---------------------------------------------------------------------
void __fastcall TFreqDispDlg::Execute(CIIR *ip, int Over, double max)
{
	m_type = 2;
	m_piir = ip;
	m_Over1 = Over;
	m_Max = max;
	ShowModal();
}
//---------------------------------------------------------------------
void __fastcall TFreqDispDlg::Execute(const double *H, int Tap, int Over, double max)
{
	m_type = 3;
	m_H1 = H;
	m_Tap1 = Tap;
	m_Over1 = Over;
	m_Tap2 = 0;
	m_Max = max;
	ShowModal();
}
//---------------------------------------------------------------------
void __fastcall TFreqDispDlg::PaintBoxPaint(TObject *Sender)
{
	if( m_type == 0 ){		// FIR
		if( Timer->Enabled ){
			memcpy(m_HT, m_H1, sizeof(double)*(m_Tap1+1));
			DrawGraph(pBitmap, m_H1, m_Tap1, m_Over1, m_Max, 1, clBlue);
		}
		else {
			DrawGraph(pBitmap, m_H1, m_Tap1, m_Over1, m_Max, 1, clBlue);
			if( m_Tap2 ){
				DrawGraph(pBitmap, m_H2, m_Tap2, m_Over2, m_Max, 0, clRed);
			}
		}
	}
	else if( m_type == 1 ){	// IIR Tank
		DrawGraphIIR(pBitmap, m_a10, 0, 0, -m_b11, -m_b12, m_Over1, m_Max, 1, clBlue);
		DrawGraphIIR(pBitmap, m_a20, 0, 0, -m_b21, -m_b22, m_Over1, m_Max, 0, clRed);
	}
	else if( m_type == 2 ){	// IIR
#if 1
		DrawGraphIIR(pBitmap, m_piir, m_Over1, m_Max, 1, clBlue);
#else
		CIIR	iir;
		iir.MakeIIR(2600, SampFreq, 10, 0, 0);
		m_Max = 6000.0;
		DrawGraphIIR(pBitmap, &iir, 1, m_Max, 1, clBlue);
#endif
	}
	else if( m_type == 3 ){	// FIR-Avg
		DrawGraph(pBitmap, m_H1, m_Tap1, m_Over1, m_Max, 1, clBlue);
	}
	PaintBox->Canvas->Draw(0, 0, (TGraphic*)pBitmap);
}
//---------------------------------------------------------------------------
void __fastcall TFreqDispDlg::BwBtnClick(TObject *Sender)
{
	if( m_type >= 2 ){
		switch(m_Max){
			case 100:
				m_Max = 200;
				break;
			case 200:
				m_Max = 400;
				break;
			case 400:
				m_Max = 800;
				break;
			case 800:
				m_Max = 3000;
				break;
			default:
				m_Max = 100;
				break;
		}
	}
	else {
		switch(m_Max){
			case 3000:
				m_Max = 4000;
				break;
			case 4000:
				m_Max = 6000;
				break;
			case 6000:
				m_Max = 2000;
				break;
			default:
				m_Max = 3000;
				break;
		}
	}
	PaintBoxPaint(NULL);
}
//---------------------------------------------------------------------------
 
