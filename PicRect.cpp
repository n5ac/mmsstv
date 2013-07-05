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

#include "Clipbrd.hpp"
#include "PicRect.h"
#include "PicFilte.h"
#include "Main.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//---------------------------------------------------------------------
#define MARGIN  4
#define MARGINW (MARGIN+MARGIN)
//---------------------------------------------------------------------
__fastcall TPicRectDlg::TPicRectDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	EntryAlignControl();
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		CancelBtn->Caption = "Cancel";
		Label1->Caption = "Stretch mode";
		SBRatio->Hint = "Keep aspect ratio";
		SBStrach->Hint = "Stretching";
		SBNStrach->Hint = "No stretching";

        SBRL->Hint = "Rotates left";
        SBRR->Hint = "Rotates right";
        SBMIR->Hint = "Right and left turning over";

		KFS->Caption = "&Save...";
		KFSC->Caption = "Save clipping image...";
		KEC->Caption = "&Copy";
		KECC->Caption = "Copy clipping image";
		KVAS->Caption = "Show image as true size";

		KEFC->Caption = "Choose space color";
		KEFCP->Caption = "Point";
		KEFCM->Caption = "Manual...";
		KEE->Caption = "&Edit image...";
		KEF->Caption = "Make space";
		KEFD->Caption = "Delete space";
        KER->Caption = "Rotation...";
		KEMC->Caption = "Centering";
		KEMM->Caption = "Clipping by maximum range";
	}
    KF->Caption = Mmsstv->KFile->Caption;
    KE->Caption = Mmsstv->KEdit->Caption;
    KV->Caption = Mmsstv->KView->Caption;
    KEP->Caption = Mmsstv->SBPaste->Hint;
    KEU->Caption = Mmsstv->KTU->Caption;
    SBC->Hint = KEF->Caption;
    SBMax->Hint = KEMM->Caption;
    SBFil->Hint = Mmsstv->SBTXFil->Hint;
    SBRot->Hint = KER->Caption;
	pBitmap = new Graphics::TBitmap();
	AdjustBitmapFormat(pBitmap);
	m_Stretch = 0;
	m_Cursor = 0;
	m_MouseCapture = 0;
	PBoxM->Left = 0;
	PBoxM->Top = 0;
	PBox->Left = MARGIN;
	PBox->Top = MARGIN;
	SBFil->Glyph->Assign(Mmsstv->SBHistFil->Glyph);
	pBmpSrc = NULL;
	m_Space = 0;
	m_MouseColor = 0;
    m_RotFlag = 0;
    m_RotDeg = 0;
}

__fastcall TPicRectDlg::~TPicRectDlg()
{
	g_ExecPB.Cancel();
	if( pBmpSrc != NULL ){
		delete pBmpSrc;
		pBmpSrc = NULL;
	}
	if( pBitmap != NULL ){
		delete pBitmap;
		pBitmap = NULL;
	}
}

//---------------------------------------------------------------------------
// ƒTƒCƒY•ÏXƒRƒ“ƒgƒ[ƒ‹‚Ì“o˜^
void __fastcall TPicRectDlg::EntryAlignControl(void)
{
	RECT	rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = 792-1;
//	rc.bottom = 569-1;
	rc.bottom = 546-1;

	AlignList.EntryControl(OKBtn, &rc, OKBtn->Font);
	AlignList.EntryControl(CancelBtn, &rc, CancelBtn->Font);
	AlignList.EntryControl(Panel, &rc, NULL);
	AlignList.EntryControl(SB256, &rc, NULL);
	AlignList.EntryControl(SB240T, &rc, NULL);
	AlignList.EntryControl(SB240B, &rc, NULL);
	AlignList.EntryControl(SBRatio, &rc, NULL);
	AlignList.EntryControl(SBStrach, &rc, NULL);
	AlignList.EntryControl(SBNStrach, &rc, NULL);

	AlignList.EntryControl(SBC, &rc, NULL);
	AlignList.EntryControl(SBMax, &rc, NULL);

	AlignList.EntryControl(Label1, &rc, NULL);
	AlignList.EntryControl(SMode, &rc, SMode->Font);

	AlignList.EntryControl(SBRL, &rc, NULL);
	AlignList.EntryControl(SBRR, &rc, NULL);
	AlignList.EntryControl(SBMIR, &rc, NULL);
	AlignList.EntryControl(SBRot, &rc, NULL);
	AlignList.EntryControl(SBFil, &rc, NULL);

	int CX = ::GetSystemMetrics(SM_CXFULLSCREEN);
	int CY = ::GetSystemMetrics(SM_CYFULLSCREEN);
//	int CX = ::GetSystemMetrics(SM_CXSCREEN);
//	int CY = ::GetSystemMetrics(SM_CYSCREEN);
	if( (CX < Width)||(CY < Height) ){
		Top = 0;
		Left = 0;
		Width = CX;
		Height = CY;
	}
	FormCenter(this, CX, CY);
	if( Owner != NULL ){
		WindowState = ((TForm *)Owner)->WindowState;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::AdjustRatio(void)
{
	double XW = pwBmp->Width;
	double YW = pwBmp->Height;
	if( (YW >= 256) && (SB240T->Down || SB240B->Down) ) YW -= 16;

	if( SBRatio->Down ){
		double xw = m_X2 - m_X1 + 1;
		double yw = m_Y2 - m_Y1 + 1;

		if( (xw != XW) || (yw != YW) ){
			double xv = (xw * XW) / pBitmap->Width;
			double yv = (yw * YW) / pBitmap->Height;
			if( yv < xv ){
				yw = xw * YW / XW;
			}
			else {
				xw = yw * XW / YW;
			}
#if 0
			int xx = ABS(m_X2 - (m_X1 + xw));
			int yy = ABS(m_Y2 - (m_Y1 + yw));
			if( (xx <= 1) && (yy <= 1) ) return;
#endif
			m_X2 = m_X1 + xw;
			m_Y2 = m_Y1 + yw;
			if( m_X2 >= pBitmap->Width ){
				int w = m_X2 - pBitmap->Width;
				m_Y2 -= w * YW / XW;
			}
			if( m_Y2 >= pBitmap->Height ){
				int w = m_Y2 - pBitmap->Height;
				m_X2 -= w * XW / YW;
			}
			if( m_X2 >= pBitmap->Width ) m_X2 = pBitmap->Width - 1;
			if( m_Y2 >= pBitmap->Height ) m_Y2 = pBitmap->Height - 1;
		}
	}
	else if( SBStrach->Down ){
		if( m_X2 >= pBitmap->Width ) m_X2 = pBitmap->Width - 1;
		if( m_Y2 >= pBitmap->Height ) m_Y2 = pBitmap->Height - 1;
	}
	else {
		m_X2 = m_X1 + XW - 1;
		m_Y2 = m_Y1 + YW - 1;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::AdjustXY(int &X, int &Y)
{
	if( m_Stretch ){
		X /= m_sc;
		Y /= m_sc;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SetPBoxSize(void)
{
	if( (!KVAS->Checked) && (
		((pBitmap->Height + MARGINW) > Panel->ClientHeight) ||
		((pBitmap->Height + MARGINW) > Panel->ClientHeight) )
	){
		m_Stretch = 1;
		PBox->Width = Panel->ClientWidth - MARGINW;
		PBox->Height = Panel->ClientHeight - MARGINW;

		m_Src.Left = 0; m_Src.Top = 0;
		m_sx = double(pBitmap->Width) / double(PBox->Width);
		m_sy = double(pBitmap->Height) / double(PBox->Height);
		if( m_sx > m_sy ){
			m_Src.Right = PBox->Width;
			m_Src.Bottom = pBitmap->Height / m_sx;
			m_sc = 1.0/m_sx;
		}
		else {
			m_Src.Bottom = PBox->Height;
			m_Src.Right = pBitmap->Width / m_sy;
			m_sc = 1.0/m_sy;
		}

		SBH->Position = 0;
		SBH->Enabled = FALSE;
		SBH->Visible = FALSE;
		SBV->Position = 0;
		SBV->Enabled = FALSE;
		SBV->Visible = FALSE;
	}
	else {
		m_Stretch = 0;
		int max;
		if( (pBitmap->Width + MARGINW) > Panel->ClientWidth ){
			if( (pBitmap->Height + MARGINW) > Panel->ClientHeight ){
				PBox->Width = SBV->Left - MARGINW;
				max = pBitmap->Width + MARGINW - SBV->Left - 1;
			}
			else {
				PBox->Width = Panel->ClientWidth - MARGINW;
				max = pBitmap->Width + MARGINW - Panel->ClientWidth - 1;
			}
			if( max < 0 ) max = 0;
			SBH->Max = max;
			SBH->Enabled = TRUE;
			SBH->Visible = TRUE;
		}
		else {
			PBox->Width = pBitmap->Width;
			SBH->Position = 0;
			SBH->Enabled = FALSE;
			SBH->Visible = FALSE;
		}
		if( (pBitmap->Height + MARGINW) > Panel->ClientHeight ){
			if( SBH->Enabled == TRUE ){
				PBox->Height = SBH->Top - MARGINW;
				max = pBitmap->Height + MARGINW - SBH->Top;
			}
			else {
				PBox->Height = Panel->ClientHeight - MARGINW;
				max = pBitmap->Height + MARGINW - Panel->ClientHeight;
			}
			if( max < 0 ) max = 0;
			SBV->Max = max;
			SBV->Enabled = TRUE;
			SBV->Visible = TRUE;
		}
		else {
			PBox->Height = pBitmap->Height;
			SBV->Position = 0;
			SBV->Enabled = FALSE;
			SBV->Visible = FALSE;
		}
	}
	PBoxM->Height = PBox->Height + MARGINW;
	PBoxM->Width = PBox->Width + MARGINW;
	UpdateTitle();
}

//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::FormResize(TObject *Sender)
{
	if( pBitmap == NULL ) return;

	CWaitCursor tw;
	SBH->Position = 0;
	SBV->Position = 0;
	AlignList.NewAlign(this);
	SetPBoxSize();
//    PBoxPaint(NULL);
	PBox->Invalidate();
}
//---------------------------------------------------------------------
void __fastcall TPicRectDlg::UpdateTitle(void)
{
	if( pBitmap == NULL ) return;
	if( pwBmp == NULL ) return;
	char bf[128];
	int h = pwBmp->Height;
	if( (h >= 256) && (SB240T->Down || SB240B->Down) ) h -= 16;
	sprintf(bf, "%s - Source:%ux%u,  Target:%ux%u", MsgEng ? "Picture clipper" : "‰æ‘œƒNƒŠƒbƒp[", pBitmap->Width, pBitmap->Height, pwBmp->Width, h);
	Caption = bf;
}
//---------------------------------------------------------------------
int __fastcall TPicRectDlg::Copy(Graphics::TBitmap *pbmp)
{
	if( (pBitmap->Width == pbmp->Width) && (pBitmap->Height == pbmp->Height) ){
		::CopyBitmap(pbmp, pBitmap);
		return TRUE;
	}
	else {
		return Execute(pbmp);
	}
}
//---------------------------------------------------------------------
int __fastcall TPicRectDlg::Execute(Graphics::TBitmap *pbmp)
{
	MultProc();
	pwBmp = pbmp;
	if( pwBmp->Height < 256 ){
		SB256->Enabled = FALSE;
		SB240T->Enabled = FALSE;
		SB240B->Enabled = FALSE;
	}
	KVAS->Checked = !sys.m_PicClipView;
	SetPBoxSize();
	m_X1 = 0;
	m_Y1 = 0;
	m_X2 = pBitmap->Width - 1;
	m_Y2 = pBitmap->Height - 1;
	switch(sys.m_PicClipType){
		case 0:
			SB256->Down = TRUE;
			break;
		case 1:
			SB240T->Down = TRUE;
			break;
		case 2:
			SB240B->Down = TRUE;
			break;
	}
	SMode->ItemIndex = sys.m_PicClipMode;
	switch(sys.m_PicClipRatio){
		case 0:
			SBStrach->Down = 1;
			break;
		case 1:
			SBRatio->Down = 1;
			break;
		default:
			SBNStrach->Down = 1;
			break;
	}
	AdjustRatio();

	if( SBRatio->Down ) SBMaxClick(NULL);
	MultProc();
	int r = ShowModal();
	MultProc();
	sys.m_PicClipView = !KVAS->Checked;
	if( SBRatio->Down ){
		sys.m_PicClipRatio = 1;
	}
	else if( SBStrach->Down ){
		sys.m_PicClipRatio = 0;
	}
	else {
		sys.m_PicClipRatio = 2;
	}
	if( SB240T->Down ){
		sys.m_PicClipType = 1;
	}
	else if( SB240B->Down ){
		sys.m_PicClipType = 2;
	}
	else {
		sys.m_PicClipType = 0;
	}
	sys.m_PicClipMode = SMode->ItemIndex;
	if( r == IDOK ){
		TRect rc;
		FillBitmap(pwBmp, sys.m_PicClipColor);
		if( pwBmp->Height >= 256 ){
			rc.Left = 0; rc.Top = SB240B->Down ? 16 : 0;
			rc.Right = pwBmp->Width;
			rc.Bottom = SB240T->Down ? pwBmp->Height - 16 : pwBmp->Height;
		}
		else {
			rc.Left = 0; rc.Top = 0;
			rc.Right = pwBmp->Width;
			rc.Bottom = pwBmp->Height;
		}
		TRect sc;
		sc.Left = m_X1;
		sc.Top = m_Y1;
		sc.Right = m_X2 + 1;
		sc.Bottom = m_Y2 + 1;
		if( SBNStrach->Down ){
			if( sc.Right > pBitmap->Width ){
				int w = sc.Right - pBitmap->Width;
				sc.Right -= w;
				rc.Right -= w;
			}
			if( sc.Bottom > pBitmap->Height ){
				int w = sc.Bottom - pBitmap->Height;
				sc.Bottom -= w;
				rc.Bottom -= w;
			}
		}
		pwBmp->Canvas->CopyMode = cmSrcCopy;
		int Sop;
		if( (sys.m_PicClipMode == 0) || SBNStrach->Down ){
			Sop = ::SetStretchBltMode(pwBmp->Canvas->Handle, HALFTONE);
		}
		else if( sys.m_PicClipMode == 1 ){
			Sop = ::SetStretchBltMode(pwBmp->Canvas->Handle, WHITEONBLACK);
		}
		else if( sys.m_PicClipMode == 2 ){
			Sop = ::SetStretchBltMode(pwBmp->Canvas->Handle, COLORONCOLOR);
		}
		else if( sys.m_PicClipMode == 3 ){
			Sop = ::SetStretchBltMode(pwBmp->Canvas->Handle, BLACKONWHITE);
		}
		else {
			NormalRect(&sc);	//ja7ude 0521
			::StretchCopy(pwBmp, &rc, pBitmap, &sc);
			MultProc();
			return TRUE;
		}
		MultProc();
		pwBmp->Canvas->CopyRect(rc, pBitmap->Canvas, sc);
		::SetStretchBltMode(pwBmp->Canvas->Handle, Sop);
		MultProc();
		return TRUE;
	}
	return FALSE;
}
//---------------------------------------------------------------------
void __fastcall TPicRectDlg::PBoxPaint(TObject *Sender)
{
	if( m_Stretch ){
		::SetStretchBltMode(PBox->Canvas->Handle, HALFTONE);
		PBox->Canvas->StretchDraw(m_Src, pBitmap);
	}
	else {
		PBox->Canvas->Draw(-SBH->Position, -SBV->Position, pBitmap);
	}
	DrawCursor();
	m_Cursor = 1;
    if( m_RotFlag ){
		PBox->Canvas->Brush->Color = clWhite;
		PBox->Canvas->Font->Size = 16;
		PBox->Canvas->TextOut(0, 0, MsgEng ? "[Rotation] Draw a base line" : "[‰ñ“]] ƒx[ƒXƒ‰ƒCƒ“‚ðˆø‚¢‚Ä‚­‚¾‚³‚¢");
    }
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::DrawCursor(void)
{
	int X1 = (m_RotFlag ? m_RotX1 : m_X1) - SBH->Position;
	int Y1 = (m_RotFlag ? m_RotY1 : m_Y1) - SBV->Position;
	int X2 = (m_RotFlag ? m_RotX2 : m_X2) - SBH->Position;
	int Y2 = (m_RotFlag ? m_RotY2 : m_Y2) - SBV->Position;

	if( m_Stretch ){
		X1 *= m_sc;
		Y1 *= m_sc;
		X2 *= m_sc;
		Y2 *= m_sc;
	}
	PBox->Canvas->Pen->Style = psDot;
	PBox->Canvas->Pen->Color = clYellow;
	PBox->Canvas->Pen->Width = 2;
	PBox->Canvas->MoveTo(X1, Y1);
	int Sop = ::SetROP2(PBox->Canvas->Handle, R2_NOT);
	if( m_RotFlag ){
		PBox->Canvas->LineTo(X2, Y2);
	}
    else {
		PBox->Canvas->LineTo(X2, Y1);
		PBox->Canvas->LineTo(X2, Y2);
		PBox->Canvas->LineTo(X1, Y2);
		PBox->Canvas->LineTo(X1, Y1);
    }
	::SetROP2(PBox->Canvas->Handle, Sop);
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::PBoxMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbLeft ){
		AdjustXY(X, Y);
		if( m_RotFlag ){
			if( m_Cursor ) DrawCursor();

			m_RotX1 = m_RotX2 = X - MARGIN + SBH->Position;
			m_RotY1 = m_RotY2 = Y - MARGIN + SBV->Position;
			m_MouseCapture = 1;
        }
		else if( m_MouseColor ){
			X = X - MARGIN + SBH->Position;
			Y = Y - MARGIN + SBV->Position;
			m_MouseColor = 0;
			if( X < 0 ) X = 0;
			if( X >= pBitmap->Width ) X = pBitmap->Width - 1;
			if( Y < 0 ) Y = 0;
			if( Y >= pBitmap->Height ) Y = pBitmap->Height - 1;
			sys.m_PicClipColor = pBitmap->Canvas->Pixels[X][Y];
			KEFClick(NULL);
		}
		else {
			m_CW = m_X2 - m_X1;
			m_CH = m_Y2 - m_Y1;
			if( m_Cursor ) DrawCursor();

			m_X1 = X - MARGIN + SBH->Position;
			m_Y1 = Y - MARGIN + SBV->Position;
			if( m_X1 < 0 ) m_X1 = 0;
			if( m_Y1 < 0 ) m_Y1 = 0;
			m_Cursor = 0;
			if( !SBNStrach->Down ){
				m_X2 = m_X1;
				m_Y2 = m_Y1;
				m_MouseCapture = 1;
			}
			else {
				m_X2 = m_X1 + pwBmp->Width - 1;
				m_Y2 = m_Y1 + pwBmp->Height - 1;
				if( (pwBmp->Height == 256) && !SB256->Down ) m_Y2 -= 16;
				PBoxPaint(NULL);
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::PBoxMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if( !m_MouseCapture ) return;

	AdjustXY(X, Y);
	m_MouseCapture = 2;
	if( m_RotFlag ){
		if( m_Cursor ) DrawCursor();
		m_RotX2 = X + SBH->Position;
		m_RotY2 = Y + SBV->Position;
		DrawCursor();
		m_Cursor = 1;
    }
	else if( !m_Stretch && (
		(X > PBox->Width) ||
		(Y > PBox->Height) ||
		(X < 0) ||
		(Y < 0) )
	){
		m_X2 = X - MARGIN + SBH->Position;
		m_Y2 = Y - MARGIN + SBV->Position;
		if( X >= PBox->Width + MARGIN ){
			if( SBH->Enabled ){
				SBH->Position = m_X2 - PBox->Width;
			}
			else {
				m_X2 = pBitmap->Width;
			}
		}
		else if( X < 0 ){
			if( SBH->Enabled && SBH->Position ){
				int x = SBH->Position + X;
				if( x < 0 ) x = 0;
				SBH->Position = x;
			}
			else {
				m_X2 = 0;
			}
		}
		if( Y >= PBox->Height + MARGIN ){
			if( SBV->Enabled ){
				SBV->Position = m_Y2 - PBox->Height;
			}
			else {
				m_Y2 = pBitmap->Height;
			}
		}
		else if( Y < 0 ){
			if( SBV->Enabled && SBV->Position ){
				int y = SBV->Position + Y;
				if( y < 0 ) y = 0;
				SBV->Position = y;
			}
			else {
				m_Y2 = 0;
			}
		}
		if( m_X2 < 0 ) m_X2 = 0;
		if( m_Y2 < 0 ) m_Y2 = 0;
		if( m_X2 >= pBitmap->Width ) m_X2 = pBitmap->Width - 1;
		if( m_Y2 >= pBitmap->Height ) m_Y2 = pBitmap->Height - 1;
		AdjustRatio();
		PBoxPaint(NULL);
	}
	else {
		if( m_Cursor ) DrawCursor();
		m_X2 = X + SBH->Position;
		m_Y2 = Y + SBV->Position;
		if( m_X2 < 0 ) m_X2 = 0;
		if( m_Y2 < 0 ) m_Y2 = 0;
		if( m_X2 >= pBitmap->Width ) m_X2 = pBitmap->Width - 1;
		if( m_Y2 >= pBitmap->Height ) m_Y2 = pBitmap->Height - 1;
		AdjustRatio();
		DrawCursor();
		m_Cursor = 1;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::PBoxMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( m_MouseCapture ){
		AdjustXY(X, Y);
		m_MouseCapture = 0;
		if( m_RotFlag ){
			m_RotX2 = X + SBH->Position;
			m_RotY2 = Y + SBV->Position;
            RotPic(-atan2(m_RotY2 - m_RotY1, m_RotX2 - m_RotX1) * 180.0 / PI);
        }
		else if( (ABS(m_X1-m_X2)<4) || (ABS(m_Y1-m_Y2)<4)  ){
			m_X2 = m_X1 + m_CW;
			m_Y2 = m_Y2 + m_CH;
			if( !SBNStrach->Down ){
				AdjustRatio();
			}
		}
		PBoxPaint(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SBVChange(TObject *Sender)
{
	PBoxPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SBRatioClick(TObject *Sender)
{
	if( SBRatio->Down ){
		if( m_X2 >= pBitmap->Width ) m_X2 = pBitmap->Width - 1;
		if( m_Y2 >= pBitmap->Height ) m_Y2 = pBitmap->Height - 1;
	}
	AdjustRatio();
	UpdateTitle();
	PBoxPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SBRLClick(TObject *Sender)
{
	if( pBitmap == NULL ) return;

	CWaitCursor w;
	pBitmap = Bitmap24bit(pBitmap);
	Graphics::TBitmap *pBmp = new Graphics::TBitmap();
	pBmp->PixelFormat = pBitmap->PixelFormat;
	pBmp->Height = pBitmap->Width;
	pBmp->Width = pBitmap->Height;
	int x, y;
	BYTE *sp, *tp;
	for( y = 0; y < pBitmap->Height; y++ ){
		sp = (BYTE *)pBitmap->ScanLine[y];
		for( x = 0; x < pBitmap->Width; x++ ){
			tp = (BYTE *)pBmp->ScanLine[pBitmap->Width - x - 1];
			tp += (y * 3);
			*tp++ = *sp++;
			*tp++ = *sp++;
			*tp = *sp++;
		}
	}
	delete pBitmap;
	pBitmap = pBmp;
	SetPBoxSize();
	AdjustRatio();
	PBox->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SBRRClick(TObject *Sender)
{
	if( pBitmap == NULL ) return;

	CWaitCursor w;
	pBitmap = Bitmap24bit(pBitmap);
	Graphics::TBitmap *pBmp = new Graphics::TBitmap();
	pBmp->PixelFormat = pBitmap->PixelFormat;
	pBmp->Height = pBitmap->Width;
	pBmp->Width = pBitmap->Height;
	int x, y;
	BYTE *sp, *tp;
	for( y = 0; y < pBitmap->Height; y++ ){
		sp = (BYTE *)pBitmap->ScanLine[y];
		for( x = 0; x < pBitmap->Width; x++ ){
			tp = (BYTE *)pBmp->ScanLine[x];
			tp += ((pBitmap->Height - y - 1) * 3);
			*tp++ = *sp++;
			*tp++ = *sp++;
			*tp = *sp++;
		}
	}
	delete pBitmap;
	pBitmap = pBmp;
	SetPBoxSize();
	AdjustRatio();
	PBox->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SBMIRClick(TObject *Sender)
{
	if( pBitmap == NULL ) return;

	CWaitCursor w;
	pBitmap = Bitmap24bit(pBitmap);
	Graphics::TBitmap *pBmp = new Graphics::TBitmap();
	pBmp->PixelFormat = pBitmap->PixelFormat;
	pBmp->Height = pBitmap->Height;
	pBmp->Width = pBitmap->Width;
	int x, y;
	BYTE *sp, *tp;
	for( y = 0; y < pBitmap->Height; y++ ){
		sp = (BYTE *)pBitmap->ScanLine[y];
		tp = (BYTE *)pBmp->ScanLine[y];
		tp += ((pBitmap->Width - 1) * 3);
		for( x = 0; x < pBitmap->Width; x++ ){
			*tp++ = *sp++;
			*tp++ = *sp++;
			*tp = *sp++;
			tp -= 5;
		}
	}
	delete pBitmap;
	pBitmap = pBmp;
	SetPBoxSize();
	AdjustRatio();
	PBox->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SBFilClick(TObject *Sender)
{
	TPicFilterDlg *pBox = new TPicFilterDlg(this);
	if( pBox->Execute(pBitmap) == TRUE ){
		PBox->Invalidate();
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KFClick(TObject *Sender)
{
	KECC->Enabled = m_Cursor;
	KFSC->Enabled = m_Cursor;
	KEF->Enabled = !m_Space;
	KEFD->Enabled = m_Space;
	KEP->Enabled = ::IsClipboardFormatAvailable(CF_BITMAP);
    KEU->Enabled = pBmpSrc != NULL;
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KEFCMClick(TObject *Sender)
{
	TColor col = sys.m_PicClipColor;
	Mmsstv->KSBClick(NULL);
	if( col != sys.m_PicClipColor ){
		MultProc();
		KEFClick(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KEFClick(TObject *Sender)
{
	CWaitCursor w;
	SaveSrc();
	if( pBmpSrc != NULL ){
        m_RotDeg = 0;
		pBitmap->PixelFormat = pf24bit;
		int W = pwBmp->Width > pBmpSrc->Width ? pwBmp->Width : pBmpSrc->Width;
		int H = pwBmp->Height > pBmpSrc->Height ? pwBmp->Height : pBmpSrc->Height;
		pBitmap->Width = (W * 5)/2;
		pBitmap->Height = (H * 5)/2;
		FillBitmap(pBitmap, sys.m_PicClipColor);
		MultProc();
		int x = (pBitmap->Width - pBmpSrc->Width) / 2;
		int y = (pBitmap->Height - pBmpSrc->Height) / 2;
		pBitmap->Canvas->Draw(x, y, pBmpSrc);
		m_Space = 1; SBC->Down = TRUE;
		SetPBoxSize();
		SetCenter();
		if( SBV->Visible ) SBV->Position = SBV->Max / 2;
        if( SBH->Visible ) SBH->Position = SBH->Max / 2;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KEFDClick(TObject *Sender)
{
	if( pBmpSrc != NULL ){
        m_RotDeg = 0;
        delete pBitmap;
        pBitmap = pBmpSrc;
        pBmpSrc = NULL;
		m_Space = 0; SBC->Down = FALSE;
        m_RotFlag = 0; SBRot->Down = FALSE;
		SetPBoxSize();
		SBMaxClick(NULL);
		SetCenter();
	}
}
//---------------------------------------------------------------------------
Graphics::TBitmap * __fastcall TPicRectDlg::GetClipBitmap(void)
{
	int x1 = m_X1;
	int x2 = m_X2;
	int y1 = m_Y1;
	int y2 = m_Y2;
	NormalRect(x1, y1, x2, y2);
	int xw = x2 - x1 + 1;
	int yw = y2 - y1;
	Graphics::TBitmap *pBmp = new Graphics::TBitmap();
	pBmp->PixelFormat = pf24bit;
	pBmp->Width = xw;
	pBmp->Height = yw;
	TRect rc;
	TRect sc;
	rc.Left = 0; rc.Top = 0;
	rc.Right = xw;
	rc.Bottom = yw;
	sc.Left = x1; sc.Top = y1;
	sc.Right = x1 + xw;
	sc.Bottom = y1 + yw;
	pBmp->Canvas->CopyRect(rc, pBitmap->Canvas, sc);
	return pBmp;
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KECClick(TObject *Sender)
{
	Mmsstv->CopyBitmap(pBitmap);
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KECCClick(TObject *Sender)
{
	Graphics::TBitmap *pBmp = GetClipBitmap();
	Mmsstv->CopyBitmap(pBmp);
	delete pBmp;
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KFSClick(TObject *Sender)
{
	Mmsstv->SaveBitmapMenu(pBitmap, "Image", NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KFSCClick(TObject *Sender)
{
	Graphics::TBitmap *pBmp = GetClipBitmap();
	Mmsstv->SaveBitmapMenu(pBmp, "Image", NULL);
	delete pBmp;
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SetCenter(void)
{
#if 0
	int xw = ABS(m_X2 - m_X1 + 1);
	int yw = ABS(m_Y2 - m_Y1 + 1);
#else
	int xw = ABS(m_X2 - m_X1 + 1);
	int yw = ABS(m_Y2 - m_Y1 + 1);
    if( m_Space && (pBmpSrc != NULL) ){
		if( xw < pBmpSrc->Width ){
			yw = yw * pBmpSrc->Width / xw;
            xw = pBmpSrc->Width;
        }
        else if( yw < pBmpSrc->Height ){
			xw = xw * pBmpSrc->Height / yw;
            yw = pBmpSrc->Height;
        }
    }
#endif
	m_X1 = (pBitmap->Width - xw) / 2;
	m_Y1 = (pBitmap->Height - yw) / 2;
	if( m_X1 < 0 ) m_X1 = 0;
	if( m_Y1 < 0 ) m_Y1 = 0;
	m_X2 = m_X1 + xw - 1;
	m_Y2 = m_Y1 + yw - 1;
	AdjustRatio();
	PBox->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KEMCClick(TObject *Sender)
{
    SetCenter();
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KEMMClick(TObject *Sender)
{
	m_X1 = 0;
	m_Y1 = 0;
	m_X2 = pBitmap->Width - 1;
	m_Y2 = pBitmap->Height - 1;
	AdjustRatio();
	PBoxPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SBMaxClick(TObject *Sender)
{
	KEMMClick(NULL);
	KEMCClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KVASClick(TObject *Sender)
{
	InvMenu(KVAS);
	SetPBoxSize();
	PBox->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KEFCPClick(TObject *Sender)
{
	m_MouseColor = 1;
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::FormKeyDown(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	if( Key == 0 ) return;

	TShiftState sf1, sf2;

	sf1 << ssShift;
	sf2 << ssShift;
	sf1 *= Shift;
	if( m_Cursor ) DrawCursor();
	if( sf1 == sf2  ){      // Shift+Any
		switch(Key){
			case VK_UP:
				m_Y2 -= 1;
				Key = 0;
				break;
			case VK_DOWN:
				m_Y2 += 1;
				Key = 0;
				break;
			case VK_LEFT:
				m_X2 -= 1;
				Key = 0;
				break;
			case VK_RIGHT:
				m_X2 += 1;
				Key = 0;
				break;
		}
		if( Key == 0 && !(SBStrach->Down) ){
			SBStrach->Down = TRUE;
		}
	}
	else {
		TShiftState ct1, ct2;
		ct1 << ssCtrl;
		ct2 << ssCtrl;
		ct1 *= Shift;
		int d = (ct1 == ct2) ? 16 : 1;
		switch(Key){
			case VK_UP:
				m_Y1 -= d;
				m_Y2 -= d;
				Key = 0;
				break;
			case VK_DOWN:
				m_Y1 += d;
				m_Y2 += d;
				Key = 0;
				break;
			case VK_LEFT:
				m_X1 -= d;
				m_X2 -= d;
				Key = 0;
				break;
			case VK_RIGHT:
				m_X1 += d;
				m_X2 += d;
				Key = 0;
				break;
		}
	    if( !m_Space ){
        	if( m_X1 < 0 ){ d = -m_X1; m_X1 += d; m_X2 += d; }
        	if( m_Y1 < 0 ){ d = -m_Y1; m_Y1 += d; m_Y2 += d; }
            if( m_X2 > pBitmap->Width ){ d = m_X2 - pBitmap->Width; m_X1 -= d; m_X2 -= d; }
            if( m_Y2 > pBitmap->Height ){ d = m_Y2 - pBitmap->Height; m_Y1 -= d; m_Y2 -= d; }
        }
	}
	if( m_Cursor ) DrawCursor();
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SBCClick(TObject *Sender)
{
	if( SBC->Down ){
		KEFClick(NULL);
    }
    else {
		KEFDClick(NULL);
    }
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SBCMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ) KEFCMClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SBMaxMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ) KEMCClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KEPClick(TObject *Sender)
{
	CWaitCursor w;
	TClipboard *pCB = Clipboard();
	if (pCB->HasFormat(CF_BITMAP)){
		if( pBmpSrc != NULL ){
			delete pBmpSrc;
	        pBmpSrc = NULL;
        }
		Graphics::TBitmap *pBmp = new Graphics::TBitmap();
		try
		{
			pBmp->LoadFromClipboardFormat(CF_BITMAP, pCB->GetAsHandle(CF_BITMAP), 0);
	        delete pBitmap;
    	    pBitmap = pBmp;
			m_RotDeg = 0;
			SetPBoxSize();
			SBMaxClick(NULL);
//			AdjustRatio();
			PBox->Invalidate();
		}
		catch (...)
		{
		}
	}
}
//---------------------------------------------------------------------------
// Rot pic
static void __fastcall RotPic(Graphics::TBitmap *pSrc, Graphics::TBitmap *pTer, double deg, TColor back)
{
	deg *= PI / 180.0;
    double sinx = sin(deg);
    double cosx = cos(deg);

	if( (pSrc->Width < 2) || (pSrc->Height < 2) ){
		CopyBitmap(pTer, pSrc);
	}
	else {
		Graphics::TBitmap *pS = DupeBitmap(pSrc, pf24bit);
		Graphics::TBitmap *pT = new Graphics::TBitmap;
		pT->PixelFormat = pf24bit;
		pT->Width = pTer->Width;
		pT->Height = pTer->Height;
		if( pT->Width & 1 ) pT->Width--;
		if( pT->Height & 1 ) pT->Height--;
		int i, j, m, n;
		double  x, y, p, q;
		int xs = pT->Width / 2;
		int ys = pT->Height / 2;
		BYTE *sp1, *sp2;
		BYTE *tp;
		BYTE br = (BYTE)(back >> 16);
		BYTE bg = (BYTE)(back >> 8);
		BYTE bb = (BYTE)back;
		int r, g, b;
		for( i = -ys; i < ys; i++ ){
			tp = (BYTE *)pT->ScanLine[i+ys];
			for( j = -xs; j < xs; j++ ){
				x = cosx * j + sinx * i;
				y = -sinx * j + cosx * i;
				if( y > 0 ){ m = int(y); } else { m = int(y-1); }
				if( x > 0 ){ n = int(x); } else { n = int(x-1); }
				q = y - m;
				p = x - n;
				double pp = 1.0 - p;
				double qq = 1.0 - q;
				if( (m >= -ys) && (m < ys-1) && (n >= -xs) && (n < xs-1) ){
					sp1 = (BYTE *)pS->ScanLine[m+ys] + (n+xs)*3;
					sp2 = (BYTE *)pS->ScanLine[m+ys+1] + (n+xs)*3;
					if( (*sp1 != *sp2) || (*sp1 != *(sp1+3)) || (*sp1 != *(sp2+3)) ){
						r = (int)(qq*(pp*(*sp1) + p*(*(sp1+3)))
							+ q * (pp*(*sp2) + p*(*(sp2+3))));
					}
					else {
						r = *sp1;
					}
					sp1++; sp2++;
					if( (*sp1 != *sp2) || (*sp1 != *(sp1+3)) || (*sp1 != *(sp2+3)) ){
						g = (int)(qq*(pp*(*sp1) + p*(*(sp1+3)))
							+ q * (pp*(*sp2) + p*(*(sp2+3))));
					}
					else {
						g = *sp1;
					}
					sp1++; sp2++;
					if( (*sp1 != *sp2) || (*sp1 != *(sp1+3)) || (*sp1 != *(sp2+3)) ){
						b = (int)(qq*(pp*(*sp1) + p*(*(sp1+3)))
							+ q * (pp*(*sp2) + p*(*(sp2+3))));
					}
					else {
						b = *sp1;
					}
					if( r < 0 ) r = 0;
					if( r > 255 ) r = 255;
					if( g < 0 ) g = 0;
					if( g > 255 ) g = 255;
					if( b < 0 ) b = 0;
					if( b > 255 ) b = 255;
					*tp++ = BYTE(r);
					*tp++ = BYTE(g);
					*tp++ = BYTE(b);
				}
				else {
					*tp++ = br;
					*tp++ = bg;
					*tp++ = bb;
				}
			}
		}
		pTer->Width = pT->Width;
		pTer->Height = pT->Height;
		pTer->Palette = NULL;
		pTer->Canvas->Draw(0, 0, pT);
		delete pT;
		delete pS;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::RotPic(double deg)
{
	CWaitCursor w;
	if( m_Space ) KEFDClick(NULL);
	SaveSrc();
    if( pBmpSrc != NULL ){
		deg += m_RotDeg;
    	m_RotDeg = deg;
		double rdeg = deg * PI/180.0;
        double sinx = sin(rdeg);
        double cosx = cos(rdeg);
		int x = pBmpSrc->Width / 2;
        int y = pBmpSrc->Height / 2;
        POINT P[4];
		P[0].x = -x * cosx + y * sinx;
        P[0].y = -x * sinx - y * cosx;
		P[1].x = x * cosx + y * sinx;
        P[1].y = x * sinx - y * cosx;
		P[2].x = x * cosx - y * sinx;
        P[2].y = x * sinx + y * cosx;
		P[3].x = -x * cosx - y * sinx;
        P[3].y = -x * sinx + y * cosx;
		int xmin = P[0].x;
        int ymin = P[0].y;
        int xmax = xmin;
        int ymax = ymin;
		for( int i = 1; i < 4; i++ ){
			if( xmin > P[i].x ) xmin = P[i].x;
            if( ymin > P[i].y ) ymin = P[i].y;
			if( xmax < P[i].x ) xmax = P[i].x;
            if( ymax < P[i].y ) ymax = P[i].y;
        }
		Graphics::TBitmap *pSrc = new Graphics::TBitmap;
        int xw = xmax - xmin;
        int yw = ymax - ymin;
		if( xw < pBmpSrc->Width ) xw = pBmpSrc->Width;
        if( yw < pBmpSrc->Height ) yw = pBmpSrc->Height;
        pBitmap->Width = xw;
        pBitmap->Height = yw;
        pSrc->Width = xw;
        pSrc->Height = yw;
        FillBitmap(pSrc, sys.m_PicClipColor);
		x = (pSrc->Width - pBmpSrc->Width) / 2;
		y = (pSrc->Height - pBmpSrc->Height) / 2;
		pSrc->Canvas->Draw(x, y, pBmpSrc);
		MultProc();
		::RotPic(pSrc, pBitmap, deg, sys.m_PicClipColor);
        delete pSrc;
		SetPBoxSize();
		SetCenter();
		if( SBV->Visible ) SBV->Position = SBV->Max / 2;
       	if( SBH->Visible ) SBH->Position = SBH->Max / 2;
    }
	m_RotFlag = 0;
	SBRot->Down = FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KERClick(TObject *Sender)
{
	AnsiString as;
	if( InputMB(NULL, MsgEng ? "Enter rotation degree" : "‰ñ“]Šp(‹)‚ð“ü—Í", as) == IDOK ){
		double deg;
		if( sscanf(as.c_str(), "%lf", &deg) != 1 ) return;
        RotPic(deg);
    }
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SBRotClick(TObject *Sender)
{
	m_RotFlag = SBRot->Down;
	m_RotX1 = m_RotX2 = m_RotY1 = m_RotY2 = -1;
    PBoxPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SBRotMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ) KERClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KEUClick(TObject *Sender)
{
	KEFDClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::SaveSrc(void)
{
	if( pBmpSrc == NULL ){
		pBmpSrc = DupeBitmap(pBitmap, pf24bit);
		MultProc();
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::KEEClick(TObject *Sender)
{
	g_ExecPB.Exec(pBitmap, (HWND)Handle, CM_EDITEXIT, 0);
}
//---------------------------------------------------------------------------
void __fastcall TPicRectDlg::OnEditExit(TMessage Message)
{
	if( Message.LParam ) return;

	if( g_ExecPB.LoadBitmap(pBitmap, TRUE) ){
		if( pBmpSrc != NULL ){
			delete pBmpSrc;
	        pBmpSrc = NULL;
        }
		m_RotDeg = 0;
		SetPBoxSize();
		SBMaxClick(NULL);
		PBox->Invalidate();
    }
}
//---------------------------------------------------------------------------
