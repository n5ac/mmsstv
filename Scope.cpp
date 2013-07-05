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

#include "Scope.h"
#include "Main.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TTScope *TScope;
static 	int		m_XW = 2048;
static	int		m_XOFF = (8192 - m_XW)/2;
static	int		m_CursorX = m_XOFF+(m_XW/2);
static	double	m_Gain = 2.0;
//---------------------------------------------------------------------
__fastcall TTScope::TTScope(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		Caption = "MMSSTV Digital Scope";
		OKBtn->Caption = "Close";
		SBUpDown->Caption = "+-";
		UpBtn->Caption = "+";
		DownBtn->Caption = "-";
		LeftBtn->Caption = "<-";
		RightBtn->Caption = "->";
	}

	pBitmap = new Graphics::TBitmap();
	pBitmap->Width = PaintBox->Width;
	pBitmap->Height = PaintBox->Height;

	TRect rc;
	rc.Top = 0;
	rc.Bottom = pBitmap->Height;
	rc.Left = 0;
	rc.Right = pBitmap->Width;
	pBitmap->Canvas->Brush->Color = clBlack;
	pBitmap->Canvas->FillRect(rc);

	pDem = NULL;
	pScope = NULL;
//    m_XW = 2048;
//    m_Gain = 1.0;
}
//---------------------------------------------------------------------
__fastcall TTScope::~TTScope()
{
	delete pBitmap;
}
//---------------------------------------------------------------------
void __fastcall TTScope::Execute(CSSTVDEM *pd)
{
	pDem = pd;
	pScope = pd->m_Scope;
	pScope[0].InitMem();
	pScope[1].InitMem();
	UpdateBtn();
	ShowModal();
}

void __fastcall TTScope::TrigNext(void)
{
	pDem->m_ScopeFlag = 0;
	pScope[0].Collect(SCOPESIZE);
	pScope[1].Collect(SCOPESIZE);
	pDem->m_ScopeFlag = 1;
}

#define GAGEWIDTH	5

void __fastcall TTScope::PaintPosition(void)
{
	TCanvas *pCanvas = pBitmap->Canvas;

	TRect rc;
	rc.Top = 0;
	rc.Bottom = GAGEWIDTH;
	rc.Left = 0;
	rc.Right = pBitmap->Width;

	pCanvas->Brush->Color = clBlack;
	pCanvas->FillRect(rc);

	int xw = (m_XW * rc.Right) / pScope->m_ScopeSize;
	int x = (m_XOFF * rc.Right) / pScope->m_ScopeSize;
	int xc = (m_CursorX * rc.Right) / pScope->m_ScopeSize;
	pCanvas->Brush->Color = clGreen;
	rc.Left = x;
	rc.Right = x + xw;
	pCanvas->FillRect(rc);

	pCanvas->Pen->Color = clYellow;
	pCanvas->Pen->Style = psSolid;
	pCanvas->MoveTo(xc, rc.Top);
	pCanvas->LineTo(xc, rc.Bottom);

	pCanvas->Font->Color = clWhite;
	pCanvas->Font->Size = 8;
	char bf[32];
	sprintf(bf, "Gain:%.1lf", m_Gain);
	::SetBkMode(pCanvas->Handle, TRANSPARENT);
	pCanvas->TextOut(0, 0, bf);
}

void __fastcall TTScope::PaintScope(CScope *sp, int n)
{
	TRect rc;

	int YW = (pBitmap->Height - GAGEWIDTH) / 2;
	rc.Top = (YW * n) + GAGEWIDTH;
	rc.Bottom = rc.Top + YW;
	rc.Left = 0;
	rc.Right = pBitmap->Width;

	TCanvas *pCanvas = pBitmap->Canvas;

	pCanvas->Brush->Color = clBlack;
	pCanvas->FillRect(rc);

	if( !sp->GetFlag() ) return;

	if( n < 2 ){
		pCanvas->Pen->Color = clWhite;
		pBitmap->Canvas->Pen->Style = psDot;
		pCanvas->MoveTo(rc.Left, rc.Top + YW/2);
		pCanvas->LineTo(rc.Right, rc.Top + YW/2);
	}

	pCanvas->Pen->Color = clWhite;
	pBitmap->Canvas->Pen->Style = psSolid;
	double d;
	int x, y, xx, xe;
	for( x = 0; x < rc.Right; x++ ){
		xx = (x * m_XW / rc.Right) + m_XOFF;
		xe = xx + (m_XW / rc.Right);
		if( xe >= sp->m_ScopeSize ) xe = sp->m_ScopeSize - 1;
		double *dp = &sp->pScopeData[xx];
		for( ; xx <= xe; xx++, dp++ ){
			d = *dp;
			if( n < 1 ){
				y = rc.Bottom - int(d * YW * m_Gain/16384.0) - 1;
			}
			else {
				y = rc.Bottom - int(d * YW * m_Gain/32768.0) - YW/2;
			}
			if( y < rc.Top ) y = rc.Top;
			if( y > rc.Bottom ) y = rc.Bottom;
			if( x ){
				pCanvas->LineTo(x, y);
			}
			else {
				pCanvas->MoveTo(x, y);
			}
		}
	}
}

void __fastcall TTScope::PaintCursor(void)
{
	int x = (m_CursorX - m_XOFF) * PaintBox->Width / m_XW;
	if( (x >= 0) && (x < PaintBox->Width) ){
		PaintBox->Canvas->Pen->Color = clLime;
		PaintBox->Canvas->Pen->Style = psDot;
		PaintBox->Canvas->MoveTo(x, 5);
		PaintBox->Canvas->LineTo(x, pBitmap->Height - GAGEWIDTH - 5);
	}
}

void __fastcall TTScope::PaintBoxPaint(TObject *Sender)
{
	if( pDem == NULL ) return;

	PaintScope(pScope, 0);
	PaintScope(pScope+1, 1);
	PaintPosition();
	PaintBox->Canvas->Draw(0, 0, (TGraphic*)pBitmap);
	PaintCursor();
}
//---------------------------------------------------------------------------
void __fastcall TTScope::TimerTimer(TObject *Sender)
{
	if( pDem == NULL ) return;

	if( pScope->GetFlag() ){
		PaintBoxPaint(NULL);
		if( SBTrig->Down ){
			SBTrig->Down = FALSE;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TTScope::SBTrigClick(TObject *Sender)
{
	TrigNext();
}
//---------------------------------------------------------------------------
void __fastcall TTScope::UpdateBtn(void)
{
	if( m_XW <= (pScope->m_ScopeSize - 512) ){
		SBUpW->Enabled = TRUE;
	}
	else {
		SBUpW->Enabled = FALSE;
	}
	if( m_XW >= 64 ){
		SBDownW->Enabled = TRUE;
	}
	else {
		SBDownW->Enabled = FALSE;
	}
	if( m_XOFF ){
		LeftBtn->Enabled = TRUE;
	}
	else {
		LeftBtn->Enabled = FALSE;
	}
	if( (m_XOFF + m_XW) < pScope->m_ScopeSize ){
		RightBtn->Enabled = TRUE;
	}
	else {
		RightBtn->Enabled = FALSE;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTScope::LeftBtnClick(TObject *Sender)
{
	if( m_XOFF ){
		m_XOFF -= m_XW/4;
		if( m_XOFF < 0 ) m_XOFF = 0;
	}
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TTScope::RightBtnClick(TObject *Sender)
{
	m_XOFF += m_XW/4;
	if( (m_XOFF + m_XW) >= pScope->m_ScopeSize ){
		m_XOFF = pScope->m_ScopeSize - m_XW;
	}
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TTScope::AdjXoff(int x)
{
	if( (x >= 0) && (x < PaintBox->Width) ){
		int xoff = x * m_XW / PaintBox->Width + m_XOFF + 1;
		m_XOFF += (m_CursorX - xoff);
		if( m_XOFF < 0 ){
			m_XOFF = 0;
		}
		else if( (m_XOFF + m_XW) > pScope->m_ScopeSize ){
			m_XOFF = pScope->m_ScopeSize - m_XW;
		}
	}
	else if( (m_XOFF + m_XW) > pScope->m_ScopeSize ){
		m_XOFF = pScope->m_ScopeSize - m_XW;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTScope::SBDownWClick(TObject *Sender)
{
	int x = (m_CursorX - m_XOFF) * PaintBox->Width / m_XW;
	if( m_XW >= 1024 ){
		m_XW -= 512;
		AdjXoff(x);
		PaintBoxPaint(NULL);
	}
	else if( m_XW >= 64 ){
		m_XW -= 32;
		AdjXoff(x);
		PaintBoxPaint(NULL);
	}
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TTScope::SBUpWClick(TObject *Sender)
{
	int x = (m_CursorX - m_XOFF) * PaintBox->Width / m_XW;
	if( m_XW < 512 ){
		m_XW += 32;
		AdjXoff(x);
		PaintBoxPaint(NULL);
	}
	else if( m_XW <= (pScope->m_ScopeSize - 512) ){
		m_XW += 512;
		AdjXoff(x);
		PaintBoxPaint(NULL);
	}
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TTScope::UpBtnClick(TObject *Sender)
{
	m_Gain *= 1.2;
	PaintBoxPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTScope::DownBtnClick(TObject *Sender)
{
	m_Gain /= 1.2;
	PaintBoxPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTScope::PaintBoxMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	m_CursorX = (X * m_XW / pBitmap->Width) + m_XOFF;
	PaintBox->Canvas->Draw(0, 0, (TGraphic*)pBitmap);
	PaintCursor();
}
//---------------------------------------------------------------------------
void __fastcall TTScope::SBUpDownClick(TObject *Sender)
{
	int x;
	double d, peak;
	peak = 0;
	double *dp = &pScope[1].pScopeData[m_XOFF];
	for( x = m_XOFF; x < (m_XOFF + m_XW); x++, dp++ ){
		d = fabs(*dp);
		if( peak < d ) peak = d;
	}
	if( peak ){
		m_Gain = 16384.0 * 0.8 / peak;
	}
	PaintBoxPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTScope::SBSetClick(TObject *Sender)
{
	Mmsstv->KOMClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTScope::SBPlayClick(TObject *Sender)
{
	if( Mmsstv->pSound->WaveFile.m_mode == 1 ){
		Mmsstv->KFPlayPosClick(NULL);
	}
	else {
		Mmsstv->KFPlayClick(NULL);
	}
}
//---------------------------------------------------------------------------

