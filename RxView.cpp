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
#include "RxView.h"
#include "sstv.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall TRxViewDlg::TRxViewDlg(TComponent* Owner)
	: TForm(Owner)
{
	Font->Name = sys.m_WinFontName;
	Font->Charset = sys.m_WinFontCharset;
	m_DisEvent = 1;
	pBitmap = NULL;
	ClientWidth = 320;
	ClientHeight = 256;
	m_MaxX = ::GetSystemMetrics(SM_CXSCREEN);
	m_MaxY = ::GetSystemMetrics(SM_CYSCREEN);
	m_rc.Left = 0;
	m_rc.Top = 0;
	m_rc.Right = PBox->Width;
	m_rc.Bottom = PBox->Height;
	m_DisEvent = 0;
	m_Mode = -1;
	m_RF = -1;
}

//---------------------------------------------------------------------------
void __fastcall TRxViewDlg::SetViewPos(int &x, int &y, int w, int h, Graphics::TBitmap *pbmp)
{
	pBitmap = pbmp;
	m_DisEvent++;
	ClientWidth = w;
	ClientHeight = h;

	if( y < 0 ){
		y = 0;
	}
	else if( y > (m_MaxY - Height) ){
		y = m_MaxY - Height;
	}
	if( x < 0 ){
		x = 0;
	}
	else if( x > (m_MaxX - Width) ){
		x = m_MaxX - Width;
	}
	Top = y;
	Left = x;

	m_rc.Left = 0;
	m_rc.Top = 0;
	m_rc.Right = PBox->Width;
	m_rc.Bottom = PBox->Height;
	m_DisEvent--;
}

//---------------------------------------------------------------------------
void __fastcall TRxViewDlg::GetViewPos(int &x, int &y, int &w, int &h)
{
	y = Top;
	x = Left;
	w = ClientWidth;
	h = ClientHeight;
}
//---------------------------------------------------------------------------
void __fastcall TRxViewDlg::UpdateSyncTitle(double fq)
{
	char bf[128];
	char bbf[64];
	if( sys.m_SampFreq != fq ){
		sprintf(bbf, "%.2lfHz (%.2lf)", fq, (fq - sys.m_SampFreq));
	}
	else {
		sprintf(bbf, "%.2lfHz", fq);
	}
	sprintf(bf, "%s - %s", MsgEng ? "Sync Viewer" : "“¯Šú‰æ–Ê", bbf );
	Caption = bf;
}
//---------------------------------------------------------------------------
void __fastcall TRxViewDlg::UpdateTitle(int mode, int rf)
{
	if( mode >= 0 ){
//		if( (mode != m_Mode) || (rf != m_RF) ){
			m_Mode = mode;
			m_RF = rf;
			char bf[128];
			sprintf(bf, "%s - %s", MsgEng ? "RX Viewer" : "ŽóM‰æ–Ê", SSTVModeList[mode]);
			if( rf ){
            	strcat(bf, " (Rep)");
            }
            else {
				SYSTEMTIME tim = Mmsstv->m_StartTime;
				if( Log.m_LogSet.m_TimeZone == 'I' ) UTCtoJST(&tim);
				sprintf(&bf[strlen(bf)], " (%02u%02u)", tim.wHour, tim.wMinute);
            }
			Caption = bf;
//		}
	}
	else {
		m_Mode = -1;
		m_RF = -1;
		Caption = MsgEng ? "RX Viewer" : "ŽóM‰æ–Ê";
	}
}
//---------------------------------------------------------------------------
void __fastcall TRxViewDlg::PBoxPaint(TObject *Sender)
{
	if( pBitmap == NULL ) return;

	if( (pBitmap->Width == PBox->Width)&&
		(pBitmap->Height == PBox->Height)
	){
		PBox->Canvas->Draw(0, 0, pBitmap);
	}
	else {
		int Sop = ::SetStretchBltMode(PBox->Canvas->Handle, HALFTONE);
		PBox->Canvas->StretchDraw(m_rc, (TGraphic*)pBitmap);
		::SetStretchBltMode(PBox->Canvas->Handle, Sop);
	}
}
//---------------------------------------------------------------------------
void __fastcall TRxViewDlg::FormResize(TObject *Sender)
{
	if( pBitmap == NULL ) return;

	if( (pBitmap->Width != PBox->Width)||
		(pBitmap->Height != PBox->Height)
	){
		if( m_DisEvent ) return;
		m_DisEvent++;
		if( ClientHeight < 100 ) ClientHeight = 100;
		if( ClientWidth < 100 ) ClientWidth = 100;

		double XW = 320;
		double YW = 256;
		double xw = ClientWidth;
		double yw = ClientHeight;
		if( Width > m_MaxX ) xw = m_MaxX;
		if( Height > m_MaxY ) yw = m_MaxY;

		double xv = (xw / Width) * XW;
		double yv = (yw / Height) * YW;
		if( yv < xv ){
			yw = xw * YW / XW;
		}
		else {
			xw = yw * XW / YW;
		}
		if( ABS(xw - ClientWidth) >= 2 ) ClientWidth = xw;
		if( ABS(yw - ClientHeight) >= 2 ) ClientHeight = yw;
		m_DisEvent--;
	}
	m_rc.Left = 0;
	m_rc.Top = 0;
	m_rc.Right = PBox->Width;
	m_rc.Bottom = PBox->Height;
}
//---------------------------------------------------------------------------
void __fastcall TRxViewDlg::PBoxClick(TObject *Sender)
{
	::SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
}
//---------------------------------------------------------------------------


void __fastcall TRxViewDlg::FormKeyDown(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	Mmsstv->FormKeyDown(Sender, Key, Shift);
}
//---------------------------------------------------------------------------

void __fastcall TRxViewDlg::FormKeyUp(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	Mmsstv->FormKeyUp(Sender, Key, Shift);
}
//---------------------------------------------------------------------------


