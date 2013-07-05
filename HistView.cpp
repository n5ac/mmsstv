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

#include "Main.h"
#include "HistView.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall THistViewDlg::THistViewDlg(TComponent* Owner)
	: TForm(Owner)
{
	for( int i = 0; i < AHDMAX; i++ ){
		pBitmapH[i] = NULL;
		pPanel[i] = NULL;
		pBox[i] = NULL;
	}
	pPopup = NULL;
	m_Col = 6;
	m_Line = 1;
	m_RectS.Left = 0;
	m_RectS.Top = 0;
	m_RectS.Right = 80;
	m_RectS.Bottom = 64;
	m_Max = m_Col = m_Line = 0;
	SetThumbnailSize(m_RectS, sys.m_HistViewSize);
	m_DisEvent = 0;
}
//---------------------------------------------------------------------------
__fastcall THistViewDlg::~THistViewDlg()
{
	for( int i = 0; i < AHDMAX; i++ ){
		if( pBitmapH[i] != NULL ){
			delete pBitmapH[i];
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::UpdateTitle(LPCSTR pTime)
{
	if( BorderStyle == bsNone ) return;

	LPCSTR pTTL;

	if( pTime != NULL ){
		pTTL = pTime;
	}
	else {
		pTTL = MsgEng ? "Thumbnails of recent history" : "Å‹ß‚Ì—š—ð‚ÌƒTƒ€ƒlƒCƒ‹";
	}
	Caption = pTTL;
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::UpdateTitlebar(void)
{
	if( m_TitleBar ){
		BorderStyle = bsSizeable;
	}
	else {
		BorderStyle = bsNone;
	}
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::FormResize(TObject *Sender)
{
	if( m_DisEvent ) return;

	int WW = m_RectS.Right + 6;
	int HH = m_RectS.Bottom + 6;
	if( BorderStyle == bsNone ){
		int ch = m_Line * HH;
		int cw = m_Col * WW;
		m_DisEvent++;
		ClientHeight = ch;
		ClientWidth = cw;
		m_DisEvent--;
		return;
	}
	int OldMax = m_Max;
	int h = int((double(ClientHeight)/ HH) + 0.5);
	if( !h ) h = 1;
	int w = int((double(ClientWidth)/ WW) + 0.5);
	if( !w ) w = 1;
	while( (h * w) > AHDMAX ){
		if( w > 1 ){
			w--;
		}
		else if( h > 1 ){
			h--;
		}
	}
	int ch = h * HH;
	int cw = w * WW;
	m_Col = w;
	m_Line = h;
	m_Max = h * w;
	int i;
	for( i = 0; i < m_Max; i++ ){
		if( pBitmapH[i] == NULL ){
			pBitmapH[i] = new Graphics::TBitmap();
			AdjustBitmapFormat(pBitmapH[i]);
		}
		pBitmapH[i]->Width = m_RectS.Right;
		pBitmapH[i]->Height = m_RectS.Bottom;
		if( pPanel[i] == NULL ){
			pPanel[i] = new TPanel(this);
			InsertControl(pPanel[i]);
			pPanel[i]->Parent = this;
			pPanel[i]->Width = WW;
			pPanel[i]->Height = HH;
			pPanel[i]->BorderStyle = bsSingle;
			pBox[i] = new TPaintBox(this);
			pPanel[i]->InsertControl(pBox[i]);
			pBox[i]->Parent = pPanel[i];
			pBox[i]->Align = alClient;
			pBox[i]->OnPaint = PBPaint;
			pBox[i]->OnMouseDown = PBMouseDown;
			pBox[i]->OnMouseMove = PBMouseMove;
			pBox[i]->OnDblClick = PBDblClick;
			pBox[i]->PopupMenu = pPopup;
		}
		else {
			pPanel[i]->Visible = FALSE;
			pPanel[i]->Width = WW;
			pPanel[i]->Height = HH;
		}
		MultProc();
	}
	for( ; i < AHDMAX; i++ ){
		if( pBitmapH[i] != NULL ){
			delete pBitmapH[i];
			pBitmapH[i] = NULL;
		}
		if( pPanel[i] != NULL ){
			pPanel[i]->Visible = FALSE;
		}
		MultProc();
	}
	m_DisEvent++;
	ClientHeight = ch;
	ClientWidth = cw;
	for( i = 0; i < m_Max; i++ ){
		pPanel[i]->Left = (i % m_Col) * WW;
		pPanel[i]->Top = (i / m_Col) * HH;
		pPanel[i]->Visible = TRUE;
	}
	m_DisEvent--;
	if( OldMax < m_Max ){
		LoadHist(OldMax);
	}
}
//---------------------------------------------------------------------------
int __fastcall THistViewDlg::IsPBox(TObject *Sender)
{
	for( int i = 0; i < m_Max; i++ ){
		if( Sender == pBox[i] ) return i;
	}
	return -1;
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::UpdateBitmap(void)
{
	for( int i = 0; i < m_Max; i++ ){
		pBox[i]->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::SetPopup(TPopupMenu *tp)
{
	pPopup = tp;
	for( int i = 0; i < m_Max; i++ ){
		pBox[i]->PopupMenu = tp;
	}
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::LoadHist(int b)
{
	if( !m_Max ) return;
	CWaitCursor wait;
	Graphics::TBitmap *pBitmap = new Graphics::TBitmap();
	pBitmap->Width = 320;
	pBitmap->Height = 256;

	int SaveCur = Mmsstv->RxHist.m_CurRead;
	int n = (Mmsstv->RxHist.m_Head.m_wPnt - 1) - b;
	int i;
	for( i = b; (i < m_Max) && (i < Mmsstv->RxHist.m_Head.m_Cnt); i++, n-- ){
		if( n < 0 ) n += Mmsstv->RxHist.m_Head.m_Max;
		if( n >= Mmsstv->RxHist.m_Head.m_Max ) n -= Mmsstv->RxHist.m_Head.m_Max;
		Mmsstv->RxHist.Read(pBitmap, n);
		::SetStretchBltMode(pBitmapH[i]->Canvas->Handle, HALFTONE);
		pBitmapH[i]->Canvas->StretchDraw(m_RectS, pBitmap);
		MultProc();
	}
	for( ; i < m_Max; i++ ){
		FillBitmap(pBitmapH[i], clWhite);
		MultProc();
	}
	Mmsstv->RxHist.m_CurRead = SaveCur;
	delete pBitmap;
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::Add(Graphics::TBitmap *pBitmap)
{
	for( int i = m_Max-1; i; i-- ){
		pBitmapH[i]->Assign(pBitmapH[i-1]);
	}
	::SetStretchBltMode(pBitmapH[0]->Canvas->Handle, HALFTONE);
	pBitmapH[0]->Canvas->StretchDraw(m_RectS, pBitmap);
	UpdateBitmap();
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::PBPaint(TObject *Sender)
{
	int r = IsPBox(Sender);
	if( r >= 0 ){
		pBox[r]->Canvas->Draw(0, 0, pBitmapH[r]);
	}
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::PBClick(int n)
{
	if( n < Mmsstv->RxHist.m_Head.m_Cnt ){
		if( Mmsstv->UDHist->Position != SHORT(n) ){
			Mmsstv->UDHist->Position = SHORT(n);
			Mmsstv->UpdateHist();
		}
		else {
			Mmsstv->DispHistStat();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::PBDblClick(TObject *Sender)
{
	Mmsstv->AdjustPage(pgHist);
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::PBMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
//    if( Mmsstv->SBTX->Down ) return;

	int r = IsPBox(Sender);
	if( r >= 0 ){
		PBClick(r);
		m_XX = X;
		m_YY = Y;
//        if( Button == mbLeft ){
//            ((TPaintBox *)Sender)->BeginDrag(TRUE);
//        }
	}
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::PBMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	TShiftState sc1, sc2;
	sc1 << ssLeft;
	sc2 << ssLeft;
	sc1 *= Shift;
	if( sc1 == sc2 ){			// Left button
		if( (ABS(m_XX-X) + ABS(m_YY-Y)) >= 10 ){
			((TPaintBox *)Sender)->BeginDrag(TRUE,0);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::FormClose(TObject *Sender,
	  TCloseAction &Action)
{
	Mmsstv->m_FileViewClose = 1;
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::FormKeyDown(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	Mmsstv->FormKeyDown(Sender, Key, Shift);
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::FormKeyUp(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	Mmsstv->FormKeyUp(Sender, Key, Shift);
}
//---------------------------------------------------------------------------
void __fastcall THistViewDlg::FormDeactivate(TObject *Sender)
{
	UpdateTitle(NULL);    
}
//---------------------------------------------------------------------------

