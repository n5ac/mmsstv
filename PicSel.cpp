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
#include "PicSel.h"
#include "Main.h"
#include "PicRect.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//---------------------------------------------------------------------
__fastcall TPicSelDlg::TPicSelDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	MultProc();
	m_DisEvent = 1;
    m_hClipNext = NULL;
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		CancelBtn->Caption = "Cancel";
		Caption = "Choose picture";
		RGType->Caption = "Source";
		RGType->Items->Strings[0] = "Refer to the History";
		RGType->Items->Strings[1] = "Loaded image";
		RGType->Items->Strings[2] = "Overlay";
		CBASize->Caption = "Auto stretch";
		RGShape->Caption = "Shape";
#if 0
		RGShape->Items->Strings[0] = "Rectangle";
		RGShape->Items->Strings[1] = "Round Rect 1";
		RGShape->Items->Strings[2] = "Round Rect 2";
		RGShape->Items->Strings[3] = "Round Rect 3";
		RGShape->Items->Strings[4] = "Ellipse";
		RGShape->Items->Strings[5] = "Polygon";
#endif
		LineBtn->Caption = "Frame";
		L1->Caption = "Clipping";
		L2->Caption = "Guide";
		CBSmz->Caption = "Mouse Smoothing";
		CBR->Caption = "Update realtime";
		GBT->Caption = "Color smoothing";
		L3->Caption = "Threshold";
		CTBtn->Caption = "Do";
		CTRBtn->Caption = "Undo";
		ClipBtn->Caption = "Clipper";
        EditBtn->Caption = "Edit";
	}
	LPCSTR *pTbl = MsgEng ? _ShapePIC[0] : _ShapePIC[1];
	while(*pTbl){
		RGShape->Items->Add(*pTbl++);
	}
	MultProc();
	GBC->Caption = L1->Caption;
	SBRect->Glyph->Assign(Mmsstv->SBDBox->Glyph);
	PCW->Color = clWhite;
	PCB->Color = clBlack;
	PCR->Color = clRed;
	PCG->Color = TColor(0x00ff00);
	pItem = NULL;
	pBitmap = NULL;
	pBitmapS = NULL;
	m_ReqLineStyle = 0;
	CBR->Checked = sys.m_PicSelRTM;
	m_Count = 3;
	m_Sync = 0;
	CBT->Text = 8;
	CTRBtn->Enabled = FALSE;
	MultProc();
}
//---------------------------------------------------------------------
__fastcall TPicSelDlg::~TPicSelDlg()
{
	g_ExecPB.Cancel();
	MultProc();
	sys.m_PicSelRTM = CBR->Checked;
	if( pBitmap != NULL ){
		delete pBitmap;
		pBitmap = NULL;
	}
	if( pBitmapS != NULL ){
		delete pBitmapS;
		pBitmapS = NULL;
	}
	MultProc();
}
//---------------------------------------------------------------------
void __fastcall TPicSelDlg::UpdateBtn(void)
{
	m_DisEvent++;
	ClipBtn->Enabled = RGType->ItemIndex ? TRUE : FALSE;
	EditBtn->Enabled = ClipBtn->Enabled;
	if( RGType->ItemIndex == 2 ){
		GBC->Visible = FALSE;
		GBT->Visible = TRUE;
		RGShape->ItemIndex = 0;
		RGShape->Enabled = FALSE;
		LineBtn->Enabled = FALSE;
		CBASize->Enabled = FALSE;
		OKBtn->Enabled = TRUE;
		m_DisEvent--;
		return;
	}
	else {
		GBT->Visible = FALSE;
		GBC->Visible = TRUE;
		RGShape->Enabled = TRUE;
		LineBtn->Enabled = TRUE;
	}
	if( (RGShape->ItemIndex == 5) && (m_Polygon.Cnt < 3) ){
		OKBtn->Enabled = FALSE;
	}
	else {
		OKBtn->Enabled = TRUE;
	}
	if( RGShape->ItemIndex == 5 ){
		if( m_Make ){
			PBox->Cursor = crCross;
		}
		else {
			PBox->Cursor = crDefault;
		}
		CBASize->Enabled = FALSE;
		GBC->Enabled = TRUE;
	}
	else {
		CBASize->Enabled = RGType->ItemIndex ? FALSE : TRUE;
		GBC->Enabled = FALSE;
	}
	SetGroupEnabled(GBC);
	if( GBC->Enabled ) CBR->Enabled = !RGType->ItemIndex;
	m_DisEvent--;
}
//---------------------------------------------------------------------
int __fastcall TPicSelDlg::Execute(CDrawPic *pt)
{
	MultProc();
	pItem = pt;
	RGType->ItemIndex = pt->m_Type;
	RGShape->ItemIndex = pt->m_Shape;
	CBASize->Checked = pt->m_Adjust;
//	PasteBtn->Enabled = RGType->ItemIndex && Clipboard()->HasFormat(CF_BITMAP);
//	PstRBtn->Enabled = PasteBtn->Enabled;
	if( pItem->pBitmap != NULL ){
		pBitmap = new Graphics::TBitmap();
		AdjustBitmapFormat(pBitmap);
		pBitmap->Assign(pItem->pBitmap);
		MultProc();
	}
	m_XW = pt->m_X2 - pt->m_X1 - 1;
	m_YW = pt->m_Y2 - pt->m_Y1 - 1;
	m_Polygon.Copy(&pt->m_Polygon);
	m_Polygon.XW = PBox->Width;
	m_Polygon.YW = PBox->Height;
	switch(m_Polygon.Cnt){
		case 4:
			SBRect->Down = TRUE;
			break;
		case 8:
			SBRRect->Down = TRUE;
			break;
		case 64:
			SBElip->Down = TRUE;
			break;
		default:
			SBFree->Down = TRUE;
			break;
	}
	L2->Color = sys.m_PicSelCurCol;
	CBSmz->Checked = sys.m_PicSelSmooz;
	m_TPX = pt->m_TransPoint & 0x0000ffff;
	m_TPY = pt->m_TransPoint >> 16;
	if( pt->m_TransPoint && (pt->pLoadBitmap != NULL) ){
		m_TPX = m_TPX * pBitmap->Width / pt->pLoadBitmap->Width;
		m_TPY = m_TPY * pBitmap->Height / pt->pLoadBitmap->Height;
	}
	UpdateBtn();
	MultProc();
	m_DisEvent = 0;
    m_hClipNext = ::SetClipboardViewer(Handle);
	int r = ShowModal();
	::ChangeClipboardChain(Handle, m_hClipNext);
	if( r == IDOK ){
		MultProc();
		pt->m_Type = RGType->ItemIndex;
		pt->m_Shape = (pt->m_Type == 2) ? 0 : RGShape->ItemIndex;
		pt->m_Adjust = CBASize->Checked;
		pt->m_TransPoint = m_TPX | (m_TPY << 16);
		if( RGType->ItemIndex ){
			pBitmap->Transparent = FALSE;
			pt->m_X2 = pt->m_X1 + m_XW + 1;
			pt->m_Y2 = pt->m_Y1 + m_YW + 1;
			pItem->pBitmap->Assign(pBitmap);
			pItem->SetLoadBitmap(pBitmap);
		}
		pt->m_Polygon.Copy(&m_Polygon);
		pt->m_DrawPolygon.Copy(&m_Polygon);
		pt->m_DrawPolygon.XW = pt->pBitmap->Width;
		pt->m_DrawPolygon.YW = pt->pBitmap->Height;
		pt->m_DrawPolygon.Stretch(m_Polygon);
		if( m_ReqLineStyle ) pt->m_LineStyle = TPenStyle(m_ReqLineStyle);
		MultProc();
		return TRUE;
	}
	return FALSE;
}
//---------------------------------------------------------------------
void __fastcall TPicSelDlg::PBoxPaint(TObject *Sender)
{
	TRect rc;
	rc.Left = 0;
	rc.Top = 0;
	rc.Right = PBox->Width;
	rc.Bottom = PBox->Height;
	if( !RGType->ItemIndex && (RGShape->ItemIndex != 5) ){
		PBox->Canvas->Pen->Color = Panel->Color;
		PBox->Canvas->Brush->Color = Panel->Color;
		PBox->Canvas->FillRect(rc);
	}
	else {
		if( !RGType->ItemIndex ){
			::SetStretchBltMode(PBox->Canvas->Handle, HALFTONE);
			Graphics::TBitmap *pBmp;
			if( CBR->Checked && Mmsstv->pDem->m_Sync && (m_Sync != 2) ){
				pBmp = Mmsstv->pBitmapRX;
			}
			else {
				pBmp = Mmsstv->pBitmapHist;
			}
			PBox->Canvas->StretchDraw(rc, (TGraphic*)(pBmp));
		}
		else {
			if( pBitmap == NULL ) return;

			if( RGType->ItemIndex == 2 ){
				if( sys.m_BitPixel == 16 ){
					Graphics::TBitmap *pbmp = DupeBitmap(pBitmap, -1);
					delete pBitmap;
					pBitmap = pbmp;
				}
				pBitmap->Transparent = TRUE;
				pBitmap->TransparentMode = tmFixed;
				pBitmap->TransparentColor = pBitmap->Canvas->Pixels[m_TPX][m_TPY];
			}
			else {
				pBitmap->Transparent = FALSE;
			}
			::SetStretchBltMode(PBox->Canvas->Handle, HALFTONE);
			PBox->Canvas->StretchDraw(rc, (TGraphic*)(pBitmap));
			pBitmap->Transparent = FALSE;

			if( RGType->ItemIndex == 2 ) DrawTransPoint();
		}
	}
	if( RGShape->ItemIndex == 5 ){
		if( !m_Make ) m_Polygon.Draw(PBox->Canvas, 1);
	}
}                                       
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::RGTypeClick(TObject *Sender)
{
	LoadBtn->Enabled = RGType->ItemIndex ? TRUE : FALSE;
	LdRBtn->Enabled = LoadBtn->Enabled;
	PasteBtn->Enabled = RGType->ItemIndex && Clipboard()->HasFormat(CF_BITMAP);
	PstRBtn->Enabled = PasteBtn->Enabled;
	PBox->Invalidate();
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::AdjustSize(void)
{
	MultProc();
	int lx = Mmsstv->DrawMain.m_SX;
	int ly = Mmsstv->DrawMain.m_SY;
	m_XW = pBitmap->Width;
	m_YW = pBitmap->Height;
	if( (m_XW > lx) || (m_YW > ly) ){
		KeepAspect(lx, ly, m_XW, m_YW);
		m_XW = lx; m_YW = ly;
		MultProc();
		Graphics::TBitmap *pBmp = new Graphics::TBitmap();
		pBmp->PixelFormat = pBitmap->PixelFormat;
		pBmp->Width = m_XW;
		pBmp->Height = m_YW;
		StretchCopy(pBmp, pBitmap, RGType->ItemIndex == 2 ? COLORONCOLOR : HALFTONE);
		delete pBitmap;
		pBitmap = pBmp;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::PasteBtnClick(TObject *Sender)
{
	if( pBitmap == NULL ) return;

	if( Mmsstv->PasteBitmap(pBitmap, 0) == TRUE ){
		PBox->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::PstRBtnClick(TObject *Sender)
{
	if( pBitmap == NULL ) return;

	if( Mmsstv->PasteBitmap(pBitmap, 1) == TRUE ){
		AdjustSize();
		PBox->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::LoadBtnClick(TObject *Sender)
{
	if( Mmsstv->LoadBitmapMenu(pBitmap, 0) == TRUE ){
		PBox->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::LdRBtnClick(TObject *Sender)
{
	if( Mmsstv->LoadBitmapMenu(pBitmap, 1) == TRUE ){
		AdjustSize();
		PBox->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::TimerTimer(TObject *Sender)
{
	MultProc();

	if( !m_Count ){
		m_Count = 2000 / Timer->Interval;
		if( CBR->Checked ){
			if( !RGType->ItemIndex && (RGShape->ItemIndex == 5) ){
				if( Mmsstv->pDem->m_Sync && (m_Sync != 2) ){
_ld:;
					PBoxPaint(NULL);
					if( m_Make ){
						m_Polygon.Draw(PBox->Canvas, 0);
						DrawCursor();
						m_Sync = 1;
					}
				}
				else if( m_Sync == 1 ){
					m_Sync = 2;
					goto _ld;
				}
			}
		}
	}
	m_Count--;
	if( m_Move && m_Make && CBSmz->Checked ){
		if( Timer->Interval != 50 ){
			m_Count = 2000 / 50;
			Timer->Interval = 50;
		}
		int X = m_SmX.Avg(m_FX);
		int Y = m_SmY.Avg(m_FY);
		if( (m_AX != X) || (m_AY != Y) ){
			m_Polygon.AddPoint(X, Y);
			m_Polygon.Draw(PBox->Canvas, 0);
			m_AX = X;
			m_AY = Y;
			m_CX = m_AX;
			m_CY = m_AY;
			UpdateBtn();
		}
	}
	else if( m_Del ){
		if( Timer->Interval != 50 ){
			m_Count = 2000 / 50;
			Timer->Interval = 50;
		}
		TShiftState Shift;
		PBoxMouseDown(this, TMouseButton(mbRight), Shift, m_CX, m_CY);
	}
	else {
		if( Timer->Interval != 1000 ){
			m_Count = 2;
			Timer->Interval = 1000;
		}
//		PasteBtn->Enabled = RGType->ItemIndex && Clipboard()->HasFormat(CF_BITMAP);
//		PstRBtn->Enabled = PasteBtn->Enabled;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::PBoxMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_Move = 0;
	m_MoveCnt = ::GetTickCount();
	if( RGType->ItemIndex == 2 ){
		X = X * pBitmap->Width / PBox->Width;
		Y = Y * pBitmap->Height / PBox->Height;
		if( Button == mbRight ){
			MakeBackup();
			CopyColor(X, Y);
		}
		else {
			m_TPX = X;
			m_TPY = Y;
		}
		PBox->Invalidate();
	}
	else if( RGShape->ItemIndex == 5 ){      // 多角形作成モード
		if( !SBFree->Down ){
			if( Button == mbRight ) return;
			if( !m_Make ){
				m_Polygon.Clear();
				m_Make++;
				PBox->Invalidate();
			}
			m_AX = m_CX = X;
			m_AY = m_CY = Y;
		}
		else {
			if( m_Make ) DrawCursor();
			m_ReqLineStyle = -1;
//            m_Polygon.Draw(PBox->Canvas, 0);
			if( Button == mbRight ){
				if( m_Polygon.Cnt ){
					m_Polygon.Cnt--;
					if( m_Polygon.Cnt ){
						int n = m_Polygon.Cnt - 1;
						m_AX = m_Polygon.pBase[n].x;
						m_AY = m_Polygon.pBase[n].y;
						m_CX = X;
						m_CY = Y;
						m_Make = 1;
						PBoxPaint(NULL);
						m_Polygon.Draw(PBox->Canvas, 0);
						DrawCursor();
						m_Del = 1;
						return;
					}
					else {
						m_Make = 0;
						PBox->Invalidate();
					}
				}
			}
			else {
				if( !m_Make ){
					m_Polygon.Clear();
					m_Make++;
					PBox->Invalidate();
				}
				if( (ABS(m_AX-X)<2) && (ABS(m_AY-Y)<2) ){
					m_Make = 0;
					if( CBSmz->Checked ) Smooz();
					PBox->Invalidate();
				}
				else {
					m_Polygon.AddPoint(X, Y);
					m_Polygon.Draw(PBox->Canvas, 0);
				}
				m_FX = m_AX = X;
				m_FY = m_AY = Y;
				m_SmX.SetCount(6);
				m_SmY.SetCount(6);
			}
			m_CX = m_AX;
			m_CY = m_AY;
		}
		UpdateBtn();
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::DrawCursor(void)
{
	if( (m_AX == m_CX) && (m_AY == m_CY) ) return;

	PBox->Canvas->MoveTo(m_AX, m_AY);
	int rop = ::SetROP2(PBox->Canvas->Handle, R2_NOT);
	if( !SBFree->Down ){
		PBox->Canvas->LineTo(m_CX, m_AY);
		PBox->Canvas->LineTo(m_CX, m_CY);
		PBox->Canvas->LineTo(m_AX, m_CY);
		PBox->Canvas->LineTo(m_AX, m_AY);
	}
	else {
		PBox->Canvas->LineTo(m_CX, m_CY);
	}
	::SetROP2(PBox->Canvas->Handle, rop);
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::DrawTransPoint(void)
{
	int x = m_TPX * PBox->Width / pBitmap->Width;
	int y = m_TPY * PBox->Height / pBitmap->Height;

	DrawTrans(PBox->Canvas, x, y);
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::PBoxMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if( !m_Make ) return;

	if( RGShape->ItemIndex == 5 ){      // 多角形作成モード
		TShiftState sc1, sc2;
		sc1 << ssLeft;
		sc2 << ssLeft;
		sc1 *= Shift;
		if( SBFree->Down && (sc1 == sc2) ){			// Left button
			if( CBSmz->Checked ){
				m_FX = X;
				m_FY = Y;
				m_Move = 1;
				if( Timer->Interval != 50 ){
					m_Count = 2000 / 50;
					Timer->Interval = 50;
				}
			}
			else if( ((m_AX != X) || (m_AY != Y)) && (DWORD(m_MoveCnt + 200) < ::GetTickCount()) ){
				m_Polygon.AddPoint(X, Y);
				m_Polygon.Draw(PBox->Canvas, 0);
				m_AX = X;
				m_AY = Y;
				m_CX = m_AX;
				m_CY = m_AY;
				UpdateBtn();
				m_Move = 1;
			}
		}
		else {
			DrawCursor();
			m_CX = X;
			m_CY = Y;
			DrawCursor();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::PBoxMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_Del = 0;
	if( RGShape->ItemIndex == 5 ){      // 多角形作成モード
		if( !m_Make ) return;
		if( Button == mbRight ) return;
		if( !SBFree->Down ){
			m_Polygon.Clear();
			if( (m_AX != m_CX) || (m_AY != m_CY) ){
				if( SBRect->Down ){
					m_Polygon.AddPoint(m_AX, m_AY);
					m_Polygon.AddPoint(m_CX, m_AY);
					m_Polygon.AddPoint(m_CX, m_CY);
					m_Polygon.AddPoint(m_AX, m_CY);
				}
				else if( SBRRect->Down ){
					int w = (m_CX - m_AX + 1)/8;
					int h = (m_CY - m_AY + 1)/8;
					m_Polygon.AddPoint(m_AX+w, m_AY);
					m_Polygon.AddPoint(m_CX-w, m_AY);
					m_Polygon.AddPoint(m_CX, m_AY+h);
					m_Polygon.AddPoint(m_CX, m_CY-h);
					m_Polygon.AddPoint(m_CX-w, m_CY);
					m_Polygon.AddPoint(m_AX+w, m_CY);
					m_Polygon.AddPoint(m_AX, m_CY-h);
					m_Polygon.AddPoint(m_AX, m_AY+h);
				}
				else {      // 楕円
					double A = (ABS(m_CX - m_AX) + 1) * 0.5;
					double B = (ABS(m_CY - m_AY) + 1) * 0.5;
					double XC = (m_CX + m_AX) * 0.5;
					double YC = (m_CY + m_AY) * 0.5;
					double deg, x, y;
					int i;
					for( i = 0, deg = 0; i < 64; i++, deg += PI/32){
						x = A * cos(deg);
						y = B * sin(deg);
						m_Polygon.AddPoint(XC + x, YC + y);
					}
				}
			}
			m_Make = 0;
			PBox->Invalidate();
			UpdateBtn();
		}
		else if( m_Move && (m_Polygon.Cnt >= 3) && (DWORD(m_MoveCnt + 1000) < ::GetTickCount()) ){
			m_Make = 0;
			PBox->Invalidate();
			UpdateBtn();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::RGShapeClick(TObject *Sender)
{
	m_Make = 0;
	PBox->Invalidate();
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::LineBtnClick(TObject *Sender)
{
	int shape = pItem->m_Shape;
	pItem->m_Shape = RGShape->ItemIndex;
	if( pItem->CDraw::Edit() == TRUE ){
		m_ReqLineStyle = 0;
		if( RGShape->ItemIndex != pItem->m_Shape ){
			RGShape->ItemIndex = pItem->m_Shape;
			RGShapeClick(NULL);
		}
	}
	pItem->m_Shape = shape;
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::SBRectClick(TObject *Sender)
{
	if( m_Make ){
		m_Make = 0;
		PBox->Invalidate();
		UpdateBtn();
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::CBSmzClick(TObject *Sender)
{
	sys.m_PicSelSmooz = CBSmz->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::L2Click(TObject *Sender)
{
	TColorDialog *pDialog = Mmsstv->ColorDialog;
	InitCustomColor(pDialog);
	AddCustomColor(pDialog, sys.m_PicSelCurCol);
	pDialog->Color = sys.m_PicSelCurCol;
	SetDisPaint();
	if( pDialog->Execute() == TRUE ){
		sys.m_PicSelCurCol = pDialog->Color;
		L2->Color = sys.m_PicSelCurCol;
		m_Polygon.Draw(PBox->Canvas, 0);
	}
	ResDisPaint();
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::PCWClick(TObject *Sender)
{
	sys.m_PicSelCurCol = ((TPanel *)Sender)->Color;
	L2->Color = sys.m_PicSelCurCol;
	m_Polygon.Draw(PBox->Canvas, 1);
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::Smooz(void)
{
	if( m_Polygon.Cnt < 3 ) return;

	CPolygon Pol;
	int i, j, N;
	int AX = 0;
	int AY = 0;
	double xx, yy, d;
	POINT *pt = m_Polygon.pBase;
	for( i = 0; i <= m_Polygon.Cnt; i++, pt++ ){
		if( i == m_Polygon.Cnt ) pt = m_Polygon.pBase;
		if( i ){
			xx = pt->x - AX;
			yy = pt->y - AY;
			d = (xx*xx) + (yy*yy);
			if( d > 0.0) d = sqrt(d);
			if( d > 8.0 ){
				N = d / 8;
				for( j = 1; j < N; j++ ){
					Pol.AddPoint(AX + (xx*j/N), AY + (yy*j/N));
				}
			}
		}
		if( i == m_Polygon.Cnt ) break;
		Pol.AddPoint(pt->x, pt->y);
		AX = pt->x; AY = pt->y;
	}
	int XC, YC;
	m_Polygon.GetCenter(XC, YC);

	double  x, y, ax, ay;
	ax = ay = 0;
	pt = Pol.pBase;
	for( i = 0; i <= Pol.Cnt; i++, pt++ ){
		if( i == Pol.Cnt ) pt = Pol.pBase;
		x = pt->x - XC;
		y = pt->y - YC;
		if( i ){
			x = (x + ax)/2.0;
			y = (y + ay)/2.0;
			pt->x = XC + x + 0.5;
			pt->y = YC + y + 0.5;
		}
		ax = x; ay = y;
	}
	m_Polygon.Copy(&Pol);
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::CBRClick(TObject *Sender)
{
	if( m_DisEvent ) return;
	PBox->Invalidate();
	m_Sync = 0;
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::CopyColor(int x, int y)
{
	CWaitCursor w;

	TColor bcol = pBitmap->Canvas->Pixels[x][y];
	TColor ccol = pBitmap->Canvas->Pixels[m_TPX][m_TPY];
	for( y = 0; y < pBitmap->Height; y++ ){
		MultProc();
		for( x = 0; x < pBitmap->Width; x++ ){
			if( pBitmap->Canvas->Pixels[x][y] == bcol ){
				pBitmap->Canvas->Pixels[x][y] = ccol;
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::MakeBackup(void)
{
	if( pBitmapS == NULL ){
		MultProc();
		pBitmapS = DupeBitmap(pBitmap, pBitmap->PixelFormat);
		CTRBtn->Enabled = TRUE;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::CTBtnClick(TObject *Sender)
{
	if( RGType->ItemIndex == 2 ){
		int lm;
		sscanf(AnsiString(CBT->Text).c_str(), "%u", &lm);	//ja7ude 0521
		lm = Limit256(lm);
		if( lm ){
			MakeBackup();
			DownColor(pBitmap, pBitmap->Canvas->Pixels[m_TPX][m_TPY], lm);
			PBox->Invalidate();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::CTRBtnClick(TObject *Sender)
{
	if( RGType->ItemIndex == 2 ){
		pBitmap->Assign(pBitmapS);
		MultProc();
		delete pBitmapS;
		pBitmapS = NULL;
		CTRBtn->Enabled = FALSE;
		PBox->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::ClipBtnClick(TObject *Sender)
{
	if( pBitmap == NULL ) return;
	if( !RGType->ItemIndex ) return;

	TPicRectDlg *pBox = new TPicRectDlg(this);
	AdjustBitmapFormat(pBox->pBitmap);
	pBox->pBitmap->Width = pBitmap->Width;
	pBox->pBitmap->Height = pBitmap->Height;
	pBox->pBitmap->Canvas->Draw(0, 0, pBitmap);
	if( pBox->Execute(pBitmap) == TRUE ){
		PBox->Invalidate();
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::EditBtnClick(TObject *Sender)
{
	if( pBitmap == NULL ) return;
	if( !RGType->ItemIndex ) return;

	g_ExecPB.Exec(pBitmap, (HWND)Handle, CM_EDITEXIT, 0);
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::OnEditExit(TMessage Message)
{
	if( Message.LParam ) return;

	if( g_ExecPB.LoadBitmap(pBitmap, TRUE) ){
		AdjustSize();
		PBox->Invalidate();
    }
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::OnDrawClipboard(TMessage Message)
{
	PasteBtn->Enabled = RGType->ItemIndex && ::IsClipboardFormatAvailable(CF_BITMAP);
	PstRBtn->Enabled = PasteBtn->Enabled;
	if( m_hClipNext ) ::SendMessage(m_hClipNext, WM_DRAWCLIPBOARD, Message.WParam, Message.LParam);
}
//---------------------------------------------------------------------------
void __fastcall TPicSelDlg::OnChangeCbChain(TMessage Message)
{
	if( (HWND)Message.WParam == m_hClipNext ){
		m_hClipNext = (HWND)Message.LParam;
	}
	if( m_hClipNext ) ::SendMessage(m_hClipNext, WM_CHANGECBCHAIN, Message.WParam, Message.LParam);
    Message.Result = 0;
}
//---------------------------------------------------------------------------

