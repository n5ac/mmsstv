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

#include "ComLib.h"
#include "PrevView.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//TPrevViewBox *PrevViewBox;
//---------------------------------------------------------------------------
__fastcall TPrevViewBox::TPrevViewBox(TComponent* Owner)
	: TForm(Owner)
{
//    if( sys.m_WinFontCharset != SHIFTJIS_CHARSET ){
//        Caption = "Preview";
//    }
//	Font->Name = sys.m_WinFontName;
//	Font->Charset = sys.m_WinFontCharset;
	pBitmap = new Graphics::TBitmap;
}
//---------------------------------------------------------------------------
__fastcall TPrevViewBox::~TPrevViewBox()
{
	if( pBitmap != NULL ){
		delete pBitmap;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPrevViewBox::UpdateTitle(LPCSTR pName)
{
	if( pBitmap == NULL ) return;

	char bf[512];

	sprintf(bf, "%ux%u %s", pBitmap->Width, pBitmap->Height, pName);
	Caption = bf;
}
//---------------------------------------------------------------------------
void __fastcall TPrevViewBox::FormPaint(TObject *Sender)
{
	if( pBitmap == NULL ) return;

	if( ((pBitmap->Width <= ClientWidth) && (pBitmap->Height <= ClientHeight)) ){
		Canvas->Draw(0, 0, pBitmap);
	}
	else {
#if 0
		TRect tc;
		tc.Left = tc.Top = 0;
		double sx = double(pBitmap->Width) / ClientWidth;
		double sy = double(pBitmap->Height) / ClientHeight;
		if( sx > sy ){
			tc.Right = ClientWidth;
			tc.Bottom = pBitmap->Height / sx;
		}
		else {
			tc.Bottom = ClientHeight;
			tc.Right = pBitmap->Width / sy;
		}
		::SetStretchBltMode(Canvas->Handle, HALFTONE);
		Canvas->StretchDraw(tc, pBitmap);
#else
		KeepAspectDraw(Canvas, ClientWidth, ClientHeight, pBitmap);
#endif
	}
}
//---------------------------------------------------------------------------
void __fastcall TPrevViewBox::FormResize(TObject *Sender)
{
	if( ClientWidth < 24 ) ClientWidth = 24;
	if( ClientHeight < 24 ) ClientHeight = 24;
	Invalidate();
}
//---------------------------------------------------------------------------

  
