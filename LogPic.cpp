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
#include "LogPic.h"
#include "Main.h"
#include "ZoomView.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//---------------------------------------------------------------------
__fastcall TLogPicDlg::TLogPicDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		OKBtn->Caption = "Close";
	}
	pBitmap = new Graphics::TBitmap();
	pBitmap->Width = PB->Width;
	pBitmap->Height = PB->Height;
	for( int i = 0; i < 5; i++ ){
		pBitmapS[i] = new Graphics::TBitmap();
		pBitmapS[i]->Width = PB->Width;
		pBitmapS[i]->Height = PB->Height;
	}
	m_RectS.Left = 0;
	m_RectS.Top = 0;
	m_RectS.Right = PB1->Width;
	m_RectS.Bottom = PB1->Height;
	SBCopy->Glyph->Assign(Mmsstv->SBCopy->Glyph);
	SBZoom->Glyph->Assign(Mmsstv->SBHView->Glyph);
}
//---------------------------------------------------------------------
__fastcall TLogPicDlg::~TLogPicDlg()
{
	delete pBitmap;
	pBitmap = NULL;
	for( int i = 0; i < 5; i++ ){
		delete pBitmapS[i];
		pBitmapS[i] = NULL;
	}
}
//---------------------------------------------------------------------
// 200102211234
int __fastcall IsDateFormat(LPCSTR p)
{
	if( !*p ) return 0;
	if( strlen(p) != 10 ) return 0;
	for( ; *p; p++ ){
		if( !isdigit(*p) ) return 0;
	}
	return 1;
}
//---------------------------------------------------------------------
void __fastcall TLogPicDlg::LoadPic(void)
{
	m_Cur = 0;
	int i;
	for( i = 0; i < m_Cnt; i++ ){
		char fname[256];
		char abf[2];
		abf[0] = abf[1] = 0;
		if( LastC(sys.m_AutoSaveDir.c_str()) != '\\' ) abf[0] = '\\';
		sprintf(fname, "%s%s20%s.jpg", sys.m_AutoSaveDir.c_str(), abf, m_List[i].c_str());
		if( LoadJPEG(pBitmapS[i], fname) != TRUE ){
			sprintf(fname, "%s%s20%s.bmp", sys.m_AutoSaveDir.c_str(), abf, m_List[i].c_str());
			if( LoadBitmap(pBitmapS[i], fname) != TRUE ){
				FillBitmap(pBitmapS[i], clWhite);
			}
		}
	}
	pBitmap->Assign(pBitmapS[0]);
}
//---------------------------------------------------------------------
int __fastcall IsLogPic(LPCSTR pNote)
{
	char bf[256];
	strcpy(bf, pNote);
	LPSTR t;
	LPSTR p = bf;
	while(*p){
		p = StrDlm(t, p);
		if( IsDateFormat(t) ) return 1;
	}
	return 0;
}

//---------------------------------------------------------------------
void __fastcall TLogPicDlg::Execute(LPCSTR pNote)
{
	char bf[256];
	strcpy(bf, pNote);
	LPSTR t;
	LPSTR p = bf;
	m_Cur = 0;
	m_Cnt = 0;
	while(*p){
		p = StrDlm(t, p);
		if( IsDateFormat(t) ){
			m_List[m_Cnt] = t;
			m_Cnt++;
			if( m_Cnt >= 5 ) break;
		}
	}
	LoadPic();
	ShowModal();
}
//---------------------------------------------------------------------
void __fastcall TLogPicDlg::PBPaint(TObject *Sender)
{
	DrawBitmap(PB, pBitmap);
//    PB->Canvas->Draw(0, 0, pBitmap);
}
//---------------------------------------------------------------------------
void __fastcall TLogPicDlg::PaintS(TPaintBox *tp, Graphics::TBitmap *pbmp)
{
	::SetStretchBltMode(tp->Canvas->Handle, HALFTONE);
	tp->Canvas->StretchDraw(m_RectS, pbmp);
}
//---------------------------------------------------------------------------
void __fastcall TLogPicDlg::PB1Paint(TObject *Sender)
{
	PaintS(PB1, pBitmapS[0]);
}
//---------------------------------------------------------------------------
void __fastcall TLogPicDlg::PB2Paint(TObject *Sender)
{
	PaintS(PB2, pBitmapS[1]);
}
//---------------------------------------------------------------------------
void __fastcall TLogPicDlg::PB3Paint(TObject *Sender)
{
	PaintS(PB3, pBitmapS[2]);
}
//---------------------------------------------------------------------------
void __fastcall TLogPicDlg::PB4Paint(TObject *Sender)
{
	PaintS(PB4, pBitmapS[3]);
}
//---------------------------------------------------------------------------
void __fastcall TLogPicDlg::PB5Paint(TObject *Sender)
{
	PaintS(PB5, pBitmapS[4]);
}
//---------------------------------------------------------------------------
void __fastcall TLogPicDlg::PB1Click(TObject *Sender)
{
	pBitmap->Assign(pBitmapS[0]);
	PB->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TLogPicDlg::PB2Click(TObject *Sender)
{
	pBitmap->Assign(pBitmapS[1]);
	PB->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TLogPicDlg::PB3Click(TObject *Sender)
{
	pBitmap->Assign(pBitmapS[2]);
	PB->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TLogPicDlg::PB4Click(TObject *Sender)
{
	pBitmap->Assign(pBitmapS[3]);
	PB->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TLogPicDlg::PB5Click(TObject *Sender)
{
	pBitmap->Assign(pBitmapS[4]);
	PB->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TLogPicDlg::SBCopyClick(TObject *Sender)
{
	Mmsstv->CopyBitmap(pBitmap);
}
//---------------------------------------------------------------------------

void __fastcall TLogPicDlg::SBZoomClick(TObject *Sender)
{
	TZoomViewDlg *pBox = new TZoomViewDlg(this);
	pBox->Execute(pBitmap, FALSE);
	delete pBox;
}
//---------------------------------------------------------------------------

