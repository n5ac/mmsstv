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

#include "Draw.h"
#include "main.h"

#include "LineSet.h"
#include "TextIn.h"
#include "ColorBar.h"
#include "PicSel.h"
#include "PicRect.h"
#include "MacroKey.h"
#include "ColorSet.h"
#include "ZoomView.h"
#include "PicFilte.h"

TColor          TransCol;
CDrawGroup      DrawPara;
CLIBL           Libl;

LPCSTR _ShapePIC[2][7]={
	{ "Rectangle", "Round Rect 1", "Round Rect 2", "Round Rect 3", "Ellipse", "Polygon", NULL },
	{ "長方形", "丸長方形1", "丸長方形2", "丸長方形3", "楕円", "多角形", NULL },
};
LPCSTR _ShapeBOX[2][7]={
	{ "Rectangle", "Round Rect 1", "Round Rect 2", "Round Rect 3", "Ellipse", "Circle", NULL },
	{ "長方形", "丸長方形1", "丸長方形2", "丸長方形3", "楕円", "円", NULL },
};

int SXW;
int SYW;
void __fastcall SetDrawSize(int x, int y)
{
	if( x >= 320 ) SXW = x;
	if( y >= 256 ) SYW = y;
}
//---------------------------------------------------------------------------
static CDrawText CurText;
//---------------------------------------------------------------------------
int __fastcall IsInside(POINT &pnt, RECT &rc)
{
	if( rc.left < rc.right ){
		if( pnt.x < rc.left ) return FALSE;
		if( pnt.x > rc.right ) return FALSE;
	}
	else {
		if( pnt.x > rc.left ) return FALSE;
		if( pnt.x < rc.right ) return FALSE;
	}
	if( rc.top > rc.bottom ){
		if( pnt.y > rc.top ) return FALSE;
		if( pnt.y < rc.bottom ) return FALSE;
	}
	else {
		if( pnt.y < rc.top ) return FALSE;
		if( pnt.y > rc.bottom ) return FALSE;
	}
	return TRUE;
}

int __fastcall IsNear(POINT &pnt, POINT &tpnt, int Width)
{
	RECT	rc;
	rc.left = tpnt.x - Width;
	rc.right = tpnt.x + Width;
	rc.top = tpnt.y + Width;
	rc.bottom = tpnt.y - Width;
	return IsInside(pnt, rc);
}

int __fastcall IsLine(POINT &pnt, POINT &spnt, POINT &tpnt, int Width)
{
	int		Xw = tpnt.x - spnt.x;
	int		Yw = tpnt.y - spnt.y;
	double	A, B;
	int		C;

	if( IsNear(pnt, spnt, Width)==TRUE ){
		return HT_P1;		/* 始点と一致	*/
	}
	else if( IsNear(pnt, tpnt, Width)==TRUE ){
		return HT_P2;		/* 終点と一致	*/
	}
	if( spnt.x > tpnt.x ){		/* X方向の入れ替え	*/
		POINT	bpnt = spnt;
		spnt = tpnt;
		tpnt = bpnt;
	}
	if( (pnt.x >= (spnt.x-Width)) && (pnt.x <= (tpnt.x+Width)) ){
		if( Xw ){
			A = double(Yw)/double(Xw);
			B = double(spnt.y) - (A * double(spnt.x));
			C = int((A * double(pnt.x)) + B);
			A = A < 0.0 ? -A : A;
			if( A >= 1.0 ){
				Width = int(double(Width) * A);
			}
			if( (pnt.y < (C+Width)) && (pnt.y >= (C-Width)) ){
				return HT_PM;
			}
		}
		else if( spnt.y > tpnt.y ){
			if( (pnt.y < spnt.y) && (pnt.y > tpnt.y) ){
				return HT_PM;
			}
		}
		else if( (pnt.y > spnt.y) && (pnt.y < tpnt.y) ){
			return HT_PM;
		}
	}
	return 0;
}

int __fastcall IsLine(POINT &pnt, POINT &spnt, POINT &tpnt)
{
	int r = IsLine(pnt, spnt, tpnt, 4);
	if( (r == HT_P1)||(r == HT_P2) ) return r;
	return IsLine(pnt, spnt, tpnt, 10) ? HT_PM : 0;
}

//---------------------------------------------------------------------------
// 実現色を返す
TColor __fastcall GetNearestColor(TColor col)
{
	if( (sys.m_BitPixel < 24) && Mmsstv->pBitmapNearest ){
    	TCanvas *pCanvas = Mmsstv->pBitmapNearest->Canvas;
		pCanvas->Pixels[0][0] = TColor(col);
        col = pCanvas->Pixels[0][0];
    }
	return col;
}
//---------------------------------------------------------------------------
TColor __fastcall GetUniqueColor(TColor col)
{
#if 1
	col = TColor(DWORD(col) & 0x00f0f0f0);
    COLD c;
    c.c = col;
	DWORD r;

    while(!c.d || (c.c == col)){
		r = rand(); r = r << 4;
        c.b.r = (BYTE)(r & 0x00f0); r = r >> 4;
        c.b.g = (BYTE)(r & 0x00f0); r = r >> 4;
        c.b.b = (BYTE)(r & 0x00f0);
    }
	col = GetNearestColor(c.c);
	return col;
#else
	col = TColor(DWORD(col) & 0x00e0e0e0);
	DWORD c = DWORD(col);

	while(!c || (c == DWORD(col))){

		c = (rand() & 0x00e0) + ((rand() & 0x00e0) << 8) + ((rand() & 0x00e0) << 16);
	}
	col = GetNearestColor(TColor(c));
	return col;
#endif
}
//---------------------------------------------------------------------------
HBITMAP __cdecl mmLoadImageMenu(DWORD sw, int xw, int yw)
{
	HBITMAP hb = NULL;

	int x = xw;
	int y = yw;
	if( sw ){
		x = 320; y = 256;
	}
	Graphics::TBitmap* pBitmap = CreateBitmap(x, y, pf24bit);
	if( Mmsstv->LoadBitmapMenu(pBitmap, sw) ){
		hb = pBitmap->ReleaseHandle();
	}
	delete pBitmap;
	return hb;
}
//---------------------------------------------------------------------------
HBITMAP __cdecl mmLoadImage(LPCSTR pName)
{
	HBITMAP hb = NULL;

	Graphics::TBitmap* pBitmap = new Graphics::TBitmap;
	pBitmap->PixelFormat = pf24bit;
	if( LoadImage(pBitmap, pName) ){
		hb = pBitmap->ReleaseHandle();
	}
	delete pBitmap;
	return hb;
}
//---------------------------------------------------------------------------
HBITMAP __fastcall LoadRefImage(Graphics::TBitmap *pRef)
{
	Graphics::TBitmap* pBitmap = new Graphics::TBitmap;
	pBitmap->PixelFormat = pf24bit;
	CopyBitmap(pBitmap, pRef);
	HBITMAP hb = pBitmap->ReleaseHandle();
	delete pBitmap;
	return hb;
}
//---------------------------------------------------------------------------
HBITMAP __cdecl mmLoadHistImage(void)
{
	return LoadRefImage(Mmsstv->pBitmapHist);
}
//---------------------------------------------------------------------------
HBITMAP __cdecl mmLoadRxImage(void)
{
	return LoadRefImage(Mmsstv->pBitmapRX);
}
//---------------------------------------------------------------------------
int __cdecl mmSaveImageMenu(HBITMAP hb, LPCSTR pName, LPCSTR pFolder)
{
	Graphics::TBitmap* pBitmap = new Graphics::TBitmap;
	pBitmap->Handle = hb;
	int r = Mmsstv->SaveBitmapMenu(pBitmap, pName, pFolder);
	pBitmap->ReleaseHandle();
	delete pBitmap;
	return r;
}
//---------------------------------------------------------------------------
int __cdecl mmSaveImage(HBITMAP hb, LPCSTR pName)
{
	Graphics::TBitmap* pBitmap = new Graphics::TBitmap;
	pBitmap->Handle = hb;
	int r = SaveImage(pBitmap, pName);
	pBitmap->ReleaseHandle();
	delete pBitmap;
	return r;
}
//---------------------------------------------------------------------------
HBITMAP __cdecl mmPasteImage(int sw, int xw, int yw)
{
	HBITMAP hb = NULL;

	int x = xw;
	int y = yw;
	if( sw ){
		x = 16; y = 16;
	}
	Graphics::TBitmap* pBitmap = CreateBitmap(x, y, pf24bit);
	if( Mmsstv->PasteBitmap(pBitmap, sw) ){
		hb = pBitmap->ReleaseHandle();
	}
	delete pBitmap;
	return hb;
}
//---------------------------------------------------------------------------
int __cdecl mmGetMacKey(LPSTR tDest, int len)
{
	int r = FALSE;
	TMacroKeyDlg *pBox = new TMacroKeyDlg(Mmsstv);
	AnsiString as;
	if( pBox->Execute(as) > 0 ){
		StrCopy(tDest, as.c_str(), len - 1);
		r = TRUE;
	}
	delete pBox;
	return r;
}
//---------------------------------------------------------------------------
int __cdecl mmConvMac(LPSTR tDest, LPCSTR pKey, int len)
{
	return Mmsstv->MacroText(tDest, pKey, len-1);
}
//---------------------------------------------------------------------------
HBITMAP __cdecl mmCreateDIB(int xw, int yw)
{
	Graphics::TBitmap* pBitmap = CreateBitmap(xw, yw, pf24bit);
	HBITMAP hb = pBitmap->ReleaseHandle();
	delete pBitmap;
	return hb;
}
//---------------------------------------------------------------------------
int __cdecl mmColorComp(LPDWORD pCol)
{
	TColorSetDlg *pBox = new TColorSetDlg(Mmsstv);
	int r = pBox->Execute((TColor *)pCol);
	delete pBox;
	return r;
}
//---------------------------------------------------------------------------
void __cdecl mmView(HBITMAP hb)
{
	Graphics::TBitmap* pBitmap = new Graphics::TBitmap;
	pBitmap->Handle = hb;
	TZoomViewDlg *pBox = new TZoomViewDlg(Mmsstv);
	pBox->SetInitSize(pBitmap->Width, pBitmap->Height);
	pBox->Execute(pBitmap, FALSE);
	delete pBox;
	pBitmap->ReleaseHandle();
	delete pBitmap;
}
//---------------------------------------------------------------------------
int __cdecl mmAdjCol(HBITMAP hb)
{
	Graphics::TBitmap* pBitmap = new Graphics::TBitmap;
	pBitmap->Handle = hb;

	TPicFilterDlg *pBox = new TPicFilterDlg(Mmsstv);
	int r = pBox->Execute(pBitmap);
	delete pBox;
	pBitmap->ReleaseHandle();
    delete pBitmap;
    return r;
}
//---------------------------------------------------------------------------
int __cdecl mmClip(HBITMAP hbDest, HBITMAP hbSrc)
{
	Graphics::TBitmap* pSrc = new Graphics::TBitmap;
	pSrc->Handle = hbSrc;
	Graphics::TBitmap* pDest = new Graphics::TBitmap;
	pDest->Handle = hbDest;

	TPicRectDlg *pBox = new TPicRectDlg(Mmsstv);
    CopyBitmap(pBox->pBitmap, pSrc);
	int r = pBox->Execute(pDest);
	delete pBox;
	pSrc->ReleaseHandle();
	pDest->ReleaseHandle();
    delete pSrc; delete pDest;
    return r;
}
//
//
//
//---------------------------------------------------------------------------
__fastcall CDraw::CDraw()
{
	m_Ver = 0;
	m_Cursor = 0;
	m_CX1 = m_CY1 = 0;
	m_CX2 = m_CY2 = 0;
	m_LineColor = clBlack;
	m_LineStyle = psSolid;
	m_LineWidth = 1;
	m_X1 = m_Y1 = m_X2 = m_Y2 = 0;
	m_BX = m_BY = m_MF = 0;
	m_BoxStyle = 0;
	pc = NULL;
	pBox = NULL;
}

void __fastcall CDraw::Start(TCanvas *tp, int x, int y)
{
	pc = tp;
	m_CX1 = m_X1 = x;
	m_CY1 = m_Y1 = y;
}

void __fastcall CDraw::StartMove(TCanvas *tp, int x, int y, int code)
{
	pc = tp;
	m_BX = x;
	m_BY = y;
	m_MF = 0;
	m_MC = code;
	m_CX1 = m_X1;
	m_CX2 = m_X2;
	m_CY1 = m_Y1;
	m_CY2 = m_Y2;
}

int __fastcall CDraw::Edit(void)
{
	TLineSetDlg *pBox = new TLineSetDlg(Mmsstv);
	int r = pBox->Execute(this);
	delete pBox;
	return r;
}

int __fastcall CDraw::Color(void)
{
	TColorDialog *pDialog = Mmsstv->ColorDialog;
	InitCustomColor(pDialog);
	AddCustomColor(pDialog, m_LineColor);
	pDialog->Color = m_LineColor;
	SetDisPaint();
	if( pDialog->Execute() == TRUE ){
		m_LineColor = pDialog->Color;
		ResDisPaint();
		return TRUE;
	}
	ResDisPaint();
	return FALSE;
}

void __fastcall CDraw::SaveToStream(TStream *sp)
{
	sp->Write(&m_Command, sizeof(m_Command));
	sp->Write(&m_Ver, sizeof(m_Ver));
	sp->Write(&m_X1, sizeof(m_X1));
	sp->Write(&m_Y1, sizeof(m_Y1));
	sp->Write(&m_X2, sizeof(m_X2));
	sp->Write(&m_Y2, sizeof(m_Y2));
	sp->Write(&m_LineColor, sizeof(m_LineColor));
	sp->Write(&m_LineStyle, sizeof(m_LineStyle));
	if( m_BoxStyle ){
		DWORD bver = 0x55aa0000;
		sp->Write(&bver, sizeof(bver));
		sp->Write(&m_BoxStyle, sizeof(m_BoxStyle));
	}
	sp->Write(&m_LineWidth, sizeof(m_LineWidth));
}

int __fastcall CDraw::LoadFromStream(TCanvas *tp, TStream *sp)
{
	try {
		pc = tp;
		sp->Read(&m_Ver, sizeof(m_Ver));
		sp->Read(&m_X1, sizeof(m_X1));
		sp->Read(&m_Y1, sizeof(m_Y1));
		sp->Read(&m_X2, sizeof(m_X2));
		sp->Read(&m_Y2, sizeof(m_Y2));
		sp->Read(&m_LineColor, sizeof(m_LineColor));
		sp->Read(&m_LineStyle, sizeof(m_LineStyle));
		DWORD bver;
		sp->Read(&bver, sizeof(bver));
		if( (bver & 0xffff0000) != 0x55aa0000 ){
			m_LineWidth = bver;
			m_BoxStyle = 0;
		}
		else {
			bver &= 0x0000ffff;
			sp->Read(&m_BoxStyle, sizeof(m_BoxStyle));
			sp->Read(&m_LineWidth, sizeof(m_LineWidth));
		}
//        sp->Read(&m_LineWidth, sizeof(m_LineWidth));
		return TRUE;
	}
	catch(...){
		return FALSE;
	}
}

void __fastcall CDraw::SaveBitmap(TStream *sp, Graphics::TBitmap *pBitmap)
{
	int xw = 0;
	int yw = 0;
	if( pBitmap != NULL ){
		xw = pBitmap->Width;
		yw = pBitmap->Height;
	}
	sp->Write(&xw, sizeof(xw));
	sp->Write(&yw, sizeof(yw));
	if( xw && yw ){
		pBitmap->SaveToStream(sp);
	}
}

int __fastcall CDraw::LoadBitmap(TStream *sp, Graphics::TBitmap *pBitmap)
{
	try {
		int xw;
		int yw;
		sp->Read(&xw, sizeof(xw));
		sp->Read(&yw, sizeof(yw));
		if( xw && yw ){
			pBitmap->LoadFromStream(sp);
		}
		return TRUE;
	}
	catch(...){
		return FALSE;
	}
}

void __fastcall CDraw::Copy(CDraw *dp)
{
	m_X1 = dp->m_X1;
	m_Y1 = dp->m_Y1;
	m_X2 = dp->m_X2;
	m_Y2 = dp->m_Y2;
	m_LineColor = dp->m_LineColor;
	m_LineStyle = dp->m_LineStyle;
	m_LineWidth = dp->m_LineWidth;
	m_BoxStyle = dp->m_BoxStyle;
	pc = dp->pc;
}

void __fastcall CDraw::SaveString(TStream *sp, AnsiString &as)
{
	int len = strlen(as.c_str());
	sp->Write(&len, sizeof(len));
	if( len ){
		sp->Write(as.c_str(), len);
	}
}

void __fastcall CDraw::LoadString(TStream *sp, AnsiString &as)
{
	int len;
	sp->Read(&len, sizeof(len));
	if( len ){
		LPSTR bp = new char[len + 1];
		sp->Read(bp, len);
		bp[len] = 0;
		as = bp;
		delete bp;
	}
	else {
		as = "";
	}
}

TColor __fastcall GetCol(TColor c1, TColor c2, int x, int xw)
{
	if( xw ){
		int     R1 = DWORD(c1) & 0x000000ff;
		int     G1 = (DWORD(c1) >> 8) & 0x000000ff;
		int     B1 = (DWORD(c1) >> 16) & 0x000000ff;
		int     R2 = DWORD(c2) & 0x000000ff;
		int     G2 = (DWORD(c2) >> 8) & 0x000000ff;
		int     B2 = (DWORD(c2) >> 16) & 0x000000ff;
		if( x > xw ) x = xw;
		R1 += (R2 - R1) * x / xw;
		G1 += (G2 - G1) * x / xw;
		B1 += (B2 - B1) * x / xw;
		LimitRGB(R1, G1, B1);
		return TColor((B1 << 16) + (G1 << 8) + R1);
	}
	else {
		return c1;
	}
}

void __fastcall CDraw::Normal(void)
{
	NormalRect(m_X1, m_Y1, m_X2, m_Y2);
}

void __fastcall CDraw::AdjustRect(void)
{
	int XW = SXW - 1;
	int YW = SYW - 1;
	if( m_X1 > XW ){
		m_X1 = XW - (m_X2 - m_X1);
		m_X2 = XW;
	}
	else if( m_X2 < 0 ){
		m_X2 = m_X2 - m_X1;
		m_X1 = 0;
	}
	if( m_Y1 > YW ){
		m_Y1 = YW - (m_Y2 - m_Y1);
		m_Y2 = YW;
	}
	else if( m_Y2 < 0 ){
		m_Y2 = m_Y2 - m_Y1;
		m_Y1 = 0;
	}

}

//---------------------------------------------------------------------------
__fastcall CDrawLine::CDrawLine()
{
	m_Command = CM_LINE;
	m_Cursor = 0;
}

int __fastcall CDrawLine::Finish(int x, int y)
{
	m_X2 = x;
	m_Y2 = y;
	return ((m_X1 != m_X2)||(m_Y1 != m_Y2)) ? TRUE : FALSE;
}

void __fastcall DrawSiege(TCanvas *tp, int X1, int Y1, int X2, int Y2, TColor SCol, int e)
{
	e = (e == 5) ? 4 : 3;
	int x1 = X1;
	int y1 = Y1;
	int x2 = X2;
	int y2 = Y2;
	TColor col = SCol;
	TColor tcol = SCol == clBlack ? clWhite : clBlack;
	int i;
	for( i = 0; i < e; i++ ){
		tp->Pen->Color = GetCol(col, tcol, i, e);
		tp->MoveTo(x1, y1);
		tp->LineTo(x2, y2);
		if( ABS(x2 - x1) > ABS(y2 - y1) ){
			y1--; y2--;
		}
		else {
			x1--; x2--;
		}
	}
	x1 = X1;
	y1 = Y1;
	x2 = X2;
	y2 = Y2;
	for( i = 0; i < e; i++ ){
		tp->Pen->Color = GetCol(col, tcol, i, e);
		tp->MoveTo(x1, y1);
		tp->LineTo(x2, y2);
		if( ABS(x2 - x1) > ABS(y2 - y1) ){
			y1++; y2++;
		}
		else {
			x1++; x2++;
		}
	}
}

void __fastcall CDrawLine::Draw(TCanvas *tp)
{
	tp->Pen->Color = m_LineColor;
	if( m_LineStyle >= 5 ){
		tp->Pen->Style = psSolid;
		tp->Pen->Width = 1;
		DrawSiege(tp, m_X1, m_Y1, m_X2, m_Y2, m_LineColor, m_LineStyle);
	}
	else if( m_LineStyle >= 0 ){
		tp->Pen->Style = m_LineStyle;
		tp->Pen->Width = m_LineWidth;
		tp->MoveTo(m_X1, m_Y1);
		tp->LineTo(m_X2, m_Y2);
	}
}

void __fastcall CDrawLine::DrawSel(TCanvas *tp, int sw)
{
	tp->Pen->Color = clBlue;
	tp->Pen->Style = psDot;
	tp->Pen->Width = 1;
	tp->MoveTo(m_X1, m_Y1);
	int rop = ::SetROP2(tp->Handle, sw ? R2_NOT : R2_COPYPEN);
	tp->LineTo(m_X2, m_Y2);
	::SetROP2(tp->Handle, rop);
}

void __fastcall CDrawLine::DrawCursor(void)
{
	pc->Pen->Style = psDot;
	pc->Pen->Width = 1;
	pc->MoveTo(m_CX1, m_CY1);
	int Sop = ::SetROP2(pc->Handle, R2_NOT);
	pc->LineTo(m_CX2, m_CY2);
	::SetROP2(pc->Handle, Sop);
}

void __fastcall CDrawLine::Making(int x, int y)
{
	if( m_Cursor ) DrawCursor();
	m_CX2 = x;
	m_CY2 = y;
	DrawCursor();
	m_Cursor = 1;
}

int __fastcall CDrawLine::HitTest(int x, int y)
{
	POINT   N;
	POINT   B, E;
	N.x = x; N.y = y;
	B.x = m_X1; B.y = m_Y1;
	E.x = m_X2; E.y = m_Y2;
	return IsLine(N, B, E, 5) ? HT_I : HT_NONE;
}

void __fastcall CDrawLine::Moving(int x, int y)
{
	if( m_Cursor ) DrawCursor();
	if( !m_MF ){
		if( m_BX != x ) m_MF = 1;
		if( m_BY != y ) m_MF = 1;
	}
	x -= m_BX;
	y -= m_BY;
	m_CX1 = m_X1 + x;
	m_CX2 = m_X2 + x;
	m_CY1 = m_Y1 + y;
	m_CY2 = m_Y2 + y;
	DrawCursor();
	m_Cursor = 1;
}

void __fastcall CDrawLine::Move(int x, int y)
{
	m_X1 = m_CX1;
	m_Y1 = m_CY1;
	m_X2 = m_CX2;
	m_Y2 = m_CY2;
}

//---------------------------------------------------------------------------
__fastcall CDrawBox::CDrawBox()
{
	m_Command = CM_BOX;
}

int __fastcall CDrawBox::Finish(int x, int y)
{
	m_X2 = x;
	m_Y2 = y;
	Normal();
	return ((m_X1 != m_X2)&&(m_Y1 != m_Y2)) ? TRUE : FALSE;
}

static void __fastcall DBox(TCanvas *tp, int cmd, int x1, int y1, int x2, int y2)
{
	int xw = ABS(x2 - x1);
	int yw = ABS(y2 - y1);
	switch(cmd){
		case 0:
			tp->MoveTo(x1, y1);
			tp->LineTo(x2, y1);
			tp->LineTo(x2, y2);
			tp->LineTo(x1, y2);
			tp->LineTo(x1, y1);
			break;
		case 1:
			tp->RoundRect(x1, y1, x2, y2, xw/3, yw/3);
			break;
		case 2:
			tp->RoundRect(x1, y1, x2, y2, xw/2, yw/2);
			break;
		case 3:
			tp->RoundRect(x1, y1, x2, y2, xw*3/4, yw*3/4);
			break;
		case 4:
		case 5:
			tp->Ellipse(x1, y1, x2, y2);
			break;
	}
}

void __fastcall CDrawBox::RoundRect(TCanvas *tp, int xw, int yw)
{
	if( m_LineStyle < 0 ) return;

	if( m_LineStyle >= 5 ){
		int e = (m_LineStyle == 5) ? 4 : 3;
		tp->Pen->Style = psSolid;
		tp->Pen->Width = 1;
		int x1 = m_X1;
		int y1 = m_Y1;
		int x2 = m_X2;
		int y2 = m_Y2;
		int xxw = xw;
		int yyw = yw;
		TColor col = m_LineColor;
		TColor tcol = m_LineColor == clBlack ? clWhite : clBlack;
//            TColor tcol = TColor(DWORD(m_LineColor) ^ 0x00ffffff);
		int i;
		for( i = 0; i < e; i++ ){
			tp->Pen->Color = GetCol(col, tcol, i, e);
			tp->RoundRect(x1, y1, x2, y2, xxw, yyw);
			x1--; y1--;
			x2++; y2++;
			xxw += 2;
			yyw += 2;
		}
		x1 = m_X1+1;
		y1 = m_Y1+1;
		x2 = m_X2-1;
		y2 = m_Y2-1;
		xxw = xw;
		yyw = yw;
		for( i = 1; i < e; i++ ){
			tp->Pen->Color = GetCol(col, tcol, i, e);
			tp->RoundRect(x1, y1, x2, y2, xxw, yyw);
			x1++; y1++;
			x2--; y2--;
			xxw -= 2;
			yyw -= 2;
		}
	}
	else {
		tp->Pen->Style = m_LineStyle;
		tp->Pen->Width = m_LineWidth;
		tp->RoundRect(m_X1, m_Y1, m_X2, m_Y2, xw, yw);
	}
}

void __fastcall CDrawBox::Draw(TCanvas *tp)
{
	tp->Pen->Color = m_LineColor;
	int xw = m_X2 - m_X1;
	int yw = m_Y2 - m_Y1;
	HBRUSH hold = (HBRUSH)::SelectObject(tp->Handle, GetStockObject(NULL_BRUSH));	//ja7ude 0521
	switch(m_BoxStyle){
		case 0:
			RoundRect(tp, 0, 0);
			break;
		case 1:
			RoundRect(tp, xw/3, yw/3);
			break;
		case 2:
			RoundRect(tp, xw/2, yw/2);
			break;
		case 3:
			RoundRect(tp, xw*3/4, yw*3/4);
			break;
		case 4:
		case 5:
			RoundRect(tp, xw, yw);
			break;
	}
	::SelectObject(tp->Handle, hold);
}

void __fastcall CDrawBox::DrawSel(TCanvas *tp, int sw)
{
	if( m_BoxStyle ){
		Draw(tp);
	}
	tp->Brush->Color = clWhite;
	tp->Pen->Color = clBlue;
	tp->Pen->Width = 1;
	tp->Pen->Style = psDot;
	tp->MoveTo(m_X1, m_Y1);
	int rop = ::SetROP2(tp->Handle, sw ? R2_NOT : R2_COPYPEN);
	tp->LineTo(m_X2, m_Y1);
	tp->LineTo(m_X2, m_Y2);
	tp->LineTo(m_X1, m_Y2);
	tp->LineTo(m_X1, m_Y1);
	::SetROP2(tp->Handle, rop);
}

void __fastcall CDrawBox::DrawCursor(void)
{
	pc->Pen->Style = psDot;
	pc->Pen->Width = 1;
	pc->MoveTo(m_CX1, m_CY1);
	int Sop = ::SetROP2(pc->Handle, R2_NOT);
	pc->LineTo(m_CX2, m_CY1);
	pc->LineTo(m_CX2, m_CY2);
	pc->LineTo(m_CX1, m_CY2);
	pc->LineTo(m_CX1, m_CY1);
	::SetROP2(pc->Handle, Sop);
}

void __fastcall CDrawBox::Making(int x, int y)
{
	if( m_Cursor ) DrawCursor();
	m_CX2 = x;
	m_CY2 = y;
	DrawCursor();
	m_Cursor = 1;
}

int __fastcall CDrawBox::HitTest(int x, int y)
{
	POINT   N;
	POINT   B, E;
	N.x = x; N.y = y;
	B.x = m_X1; B.y = m_Y1;
	E.x = m_X2; E.y = m_Y1;
	int r = IsLine(N, B, E, 5);
	if( r ){
		switch(r){
			case HT_P1:
				return HT_LT;
			case HT_P2:
				return HT_RT;
			default:
				return HT_T;
		}
	}
	B.x = m_X2; B.y = m_Y1;
	E.x = m_X2; E.y = m_Y2;
	r = IsLine(N, B, E, 5);
	if( r ){
		switch(r){
			case HT_P1:
				return HT_RT;
			case HT_P2:
				return HT_RB;
			default:
				return HT_R;
		}
	}
	B.x = m_X2; B.y = m_Y2;
	E.x = m_X1; E.y = m_Y2;
	r = IsLine(N, B, E, 5);
	if( r ){
		switch(r){
			case HT_P1:
				return HT_RB;
			case HT_P2:
				return HT_LB;
			default:
				return HT_B;
		}
	}
	B.x = m_X1; B.y = m_Y2;
	E.x = m_X1; E.y = m_Y1;
	r = IsLine(N, B, E, 5);
	if( r ){
		switch(r){
			case HT_P1:
				return HT_LB;
			case HT_P2:
				return HT_LT;
			default:
				return HT_L;
		}
	}
	if( (x < m_X1) || (x > m_X2) ) return FALSE;
	if( (y < m_Y1) || (y > m_Y2) ) return FALSE;
	return HT_I;
}

void __fastcall CDrawBox::Moving(int x, int y)
{
	if( m_Cursor ) DrawCursor();
	if( !m_MF ){
		if( m_BX != x ) m_MF = 1;
		if( m_BY != y ) m_MF = 1;
	}
	x -= m_BX;
	y -= m_BY;
	int bs = 0;
	switch(m_MC){
		case HT_LT:
			m_CX1 = m_X1 + x;
			m_CY1 = m_Y1 + y;
			if( m_BoxStyle == 5 ) bs = 3;
			break;
		case HT_T:
			m_CY1 = m_Y1 + y;
			if( m_BoxStyle == 5 ) bs = 1;
			break;
		case HT_RT:
			m_CX2 = m_X2 + x;
			m_CY1 = m_Y1 + y;
			if( m_BoxStyle == 5 ) bs = 1;
			break;
		case HT_R:
			m_CX2 = m_X2 + x;
			if( m_BoxStyle == 5 ) bs = 2;
			break;
		case HT_RB:
			m_CX2 = m_X2 + x;
			m_CY2 = m_Y2 + y;
			if( m_BoxStyle == 5 ) bs = 2;
			break;
		case HT_B:
			m_CY2 = m_Y2 + y;
			if( m_BoxStyle == 5 ) bs = 1;
			break;
		case HT_LB:
			m_CX1 = m_X1 + x;
			m_CY2 = m_Y2 + y;
			if( m_BoxStyle == 5 ) bs = 2;
			break;
		case HT_L:
			m_CX1 = m_X1 + x;
			if( m_BoxStyle == 5 ) bs = 2;
			break;
		default:
			m_CX1 = m_X1 + x;
			m_CX2 = m_X2 + x;
			m_CY1 = m_Y1 + y;
			m_CY2 = m_Y2 + y;
			break;
	}
	switch(bs){
		case 1:
			m_CX2 = m_CX1 + (m_CY2 - m_CY1);
			break;
		case 2:
			m_CY2 = m_CY1 + (m_CX2 - m_CX1);
			break;
		case 3:
			m_CX1 = m_CX2 - (m_CY2 - m_CY1);
			break;
	}
	DrawCursor();
	m_Cursor = 1;
}

void __fastcall CDrawBox::Move(int x, int y)
{
	m_X1 = m_CX1;
	m_Y1 = m_CY1;
	m_X2 = m_CX2;
	m_Y2 = m_CY2;
	Normal();
	AdjustRect();
}

//---------------------------------------------------------------------------
__fastcall CDrawBoxS::CDrawBoxS()
{
	m_Command = CM_BOXS;
	m_LineStyle = TPenStyle(-1);
}

void __fastcall CDrawBoxS::Draw(TCanvas *tp)
{
	tp->Brush->Color = m_LineColor;
	tp->Pen->Color = m_LineColor;
	tp->Pen->Style = psSolid;
	tp->Pen->Style = m_LineStyle < 5 ? m_LineStyle : psSolid;
	tp->Pen->Width = m_LineWidth;
	if( m_BoxStyle ){
		tp->Pen->Style = m_LineStyle;
		tp->Pen->Width = m_LineWidth;
		DBox(tp, m_BoxStyle, m_X1, m_Y1, m_X2, m_Y2);
	}
	else {
		TRect rc;
		rc.Left = m_X1; rc.Top = m_Y1;
		rc.Right = m_X2; rc.Bottom = m_Y2;
		tp->FillRect(rc);
	}
	TColor col = m_LineColor;
	m_LineColor = clWhite;
	CDrawBox::Draw(tp);
	m_LineColor = col;
}

//---------------------------------------------------------------------------
__fastcall CDrawTitle::CDrawTitle()
{
	m_Command = CM_TITLE;
	m_Type = 1;
	m_Col1 = sys.m_ColBar[0];
	m_Col2 = sys.m_ColBar[1];
	m_Col3 = sys.m_ColBar[2];
	m_Col4 = sys.m_ColBar[3];
	m_X1 = m_Y1 = 0;
	m_X2 = 319;
	m_Y2 = 15;
	m_Ver = 3;
	m_ColVert = 0;
	m_Sound = "1356865313568888";
//	m_Sound = "5888531133865555";
//    m_Sound = "1588531133531111";
//	m_Sound = "1355135585313533";
	pBitmap = NULL;
}

__fastcall CDrawTitle::~CDrawTitle()
{
	if( pBitmap != NULL ){
		delete pBitmap;
		pBitmap = NULL;
	}
}

int __fastcall CDrawTitle::HitTest(int x, int y)
{
	int r = CDrawBox::HitTest(x, y);
	switch(r){
		case 0:
			return HT_NONE;
		case HT_B:
			return r;
		default:
			return HT_I;
	}
};

void __fastcall CDrawTitle::Start(TCanvas *tp, int x, int y)
{
	pc = tp;
	m_CY1 = m_Y1 = y;
	m_CX1 = m_X1;
	m_CX2 = m_X2;
}

int __fastcall CDrawTitle::Finish(int x, int y)
{
	m_X1 = 0;
	m_X2 = SXW - 1;
	m_Y1 = y;
	m_Y2 = y + 15;
	return TRUE;
}

void __fastcall CDrawTitle::FillPic(TCanvas *tp, TColor col, int sw, double sx, double sy, int off)
{
	TRect rc;
	switch(sw){
		case 2:
			rc.Left = m_X1;
			rc.Top = m_Y1 - 16;
			rc.Right = m_X2 + 1;
			rc.Bottom = m_Y2 - 16;
			break;
		case 4:
			rc.Left = m_X1;
			rc.Top = m_Y1 * 240 / 256;
			rc.Right = m_X2 + 1;
			rc.Bottom = (m_Y2 * 240 / 256);
			break;
		case -1:
			rc.Left = m_X1;
			rc.Top = m_Y1 + off;
			rc.Right = (m_X2 + 1);
			rc.Bottom = m_Y2 + off;
			break;
		default:
			rc.Left = m_X1 * sx;
			rc.Top = m_Y1 * sy;
			rc.Right = (m_X2 + 1) * sx;
			rc.Bottom = m_Y2 * sy;
			break;
	}
	rc.Bottom++;
	tp->Brush->Color = col;
	tp->Pen->Color = col;
	tp->FillRect(rc);
}

void __fastcall CDrawTitle::Draw(TCanvas *tp)
{
	m_X2 = SXW - 1;
	int x, y;
	int X1, X2, Y1, Y2;
	int XW, YW;
	tp->Pen->Style = psSolid;
	switch(m_Type){
		case 0:
			{
				TRect rc;
				rc.Top = m_Y1;
				rc.Left = m_X1;
				rc.Right = m_X2 + 1;
				rc.Bottom = m_Y2 + 1;
				tp->Brush->Style = bsSolid;
				tp->Brush->Color = m_Col1;
				tp->FillRect(rc);
            }
			break;
		case 1:
			if( m_ColVert ){
				for( y = m_Y1; y <= m_Y2; y++ ){
					tp->Pen->Color = GetCol(m_Col1, m_Col2, y, m_Y2);
                    tp->MoveTo(m_X1, y); tp->LineTo(m_X2+1, y);
				}
			}
			else {
				for( x = m_X1; x <= m_X2; x++ ){
					tp->Pen->Color = GetCol(m_Col1, m_Col2, x, m_X2);
                    tp->MoveTo(x, m_Y1); tp->LineTo(x, m_Y2+1);
				}
			}
			break;
		case 2:
			if( m_ColVert ){
				YW = (m_Y2 - m_Y1) / 3;
				Y1 = YW + m_Y1;
				Y2 = YW + Y1;
				for( y = m_Y1; y < Y1; y++ ){
					tp->Pen->Color = GetCol(m_Col1, m_Col2, y - m_Y1, YW);
                    tp->MoveTo(m_X1, y); tp->LineTo(m_X2+1, y);
				}
				for( ; y < Y2; y++ ){
					tp->Pen->Color = GetCol(m_Col2, m_Col3, y - Y1, YW);
                    tp->MoveTo(m_X1, y); tp->LineTo(m_X2+1, y);
				}
				for( ; y <= m_Y2; y++ ){
					tp->Pen->Color = GetCol(m_Col3, m_Col4, y - Y2, YW);
                    tp->MoveTo(m_X1, y); tp->LineTo(m_X2+1, y);
				}
			}
			else {
				XW = (m_X2 - m_X1) / 3;
				X1 = XW + m_X1;
				X2 = XW + X1;
				for( x = m_X1; x < X1; x++ ){
					tp->Pen->Color = GetCol(m_Col1, m_Col2, x - m_X1, XW);
                    tp->MoveTo(x, m_Y1); tp->LineTo(x, m_Y2+1);
				}
				for( ; x < X2; x++ ){
					tp->Pen->Color = GetCol(m_Col2, m_Col3, x - X1, XW);
                    tp->MoveTo(x, m_Y1); tp->LineTo(x, m_Y2+1);
				}
				for( ; x <= m_X2; x++ ){
					tp->Pen->Color = GetCol(m_Col3, m_Col4, x - X2, XW);
                    tp->MoveTo(x, m_Y1); tp->LineTo(x, m_Y2+1);
				}
			}
			break;
		case 3:
			if( pBitmap != NULL ){
				if( pBitmap->Height != (m_Y2 - m_Y1 + 1) ){
					TRect rc;
					rc.Left = m_X1; rc.Top = m_Y1;
					rc.Right = m_X2 + 1;
					rc.Bottom = m_Y2 + 1;
					int Sop = ::SetStretchBltMode(tp->Handle, HALFTONE);
					tp->StretchDraw(rc, pBitmap);
					::SetStretchBltMode(tp->Handle, Sop);
				}
				else {
					tp->Draw(m_X1, m_Y1, pBitmap);
				}
			}
            break;
        case 4:
        	{
            	const BYTE _ft[]={0, 29, 60, 92, 126, 162, 201, 242};
                LPCSTR p = m_Sound.c_str();
                DWORD col;
				for( y = m_Y1; *p; p++, y++ ){
					if( (*p >= '1') && (*p <= '8') ){
                       	col = _ft[(*p & 0x0f) - 1];
	                    tp->Pen->Color = TColor(col + (col * 256) + (col * 65536));
	                    tp->MoveTo(m_X1, y); tp->LineTo(m_X2+1, y);
                    }
                }
            }
            break;
	}
}

void __fastcall CDrawTitle::DrawFast(TCanvas *tp)
{
	m_X2 = SXW - 1;
	TRect rc;
	rc.Left = m_X1;
	rc.Top = m_Y1;
	rc.Right = m_X2 + 1;
	rc.Bottom = m_Y2;
	tp->Brush->Color = m_Col1;
	tp->Pen->Color = m_Col1;
	tp->FillRect(rc);
}

void __fastcall CDrawTitle::Moving(int x, int y)
{
	if( m_Cursor ) DrawCursor();
	if( !m_MF ){
		if( m_BY != y ) m_MF = 1;
	}
	y -= m_BY;
	int h = m_Y2 - m_Y1;
	switch(m_MC){
		case HT_I:
			y += m_Y1;
			if( y < 0 ) y = 0;
			if( y > (SYW - h) ) y = SYW - h;
			m_CY1 = y;
			m_CY2 = m_CY1 + h;
			break;
		case HT_B:
			y += m_Y2;
			if( y < 0 ) y = 0;
			if( y > (SYW-1) ) y = SYW-1;
			if( y != m_CY1 ) m_CY2 = y;
			break;
	}
	DrawCursor();
	m_Cursor = 1;
}

void __fastcall CDrawTitle::StartMove(TCanvas *tp, int x, int y, int code)
{
	pc = tp;
	m_BY = y;
	m_MF = 0;
	m_MC = code;
	m_CX1 = m_X1;
	m_CX2 = m_X2;
	m_Cursor = 0;
}

void __fastcall CDrawTitle::Move(int x, int y)
{
	if( m_CY1 < m_CY2 ){
		m_Y1 = m_CY1;
		m_Y2 = m_CY2;
	}
	else {
		m_Y1 = m_CY2;
		m_Y2 = m_CY1;
	}
}

void __fastcall CDrawTitle::Copy(CDraw *dp)
{
	CDrawBox::Copy(dp);

	CDrawTitle *sp = (CDrawTitle *)dp;
	m_Type = sp->m_Type;
	m_ColVert = sp->m_ColVert;
	m_Col1 = sp->m_Col1;
	m_Col2 = sp->m_Col2;
	m_Col3 = sp->m_Col3;
	m_Col4 = sp->m_Col4;
    m_Sound = sp->m_Sound;
	if( sp->pBitmap != NULL ){
		if( pBitmap == NULL ){
			pBitmap = new Graphics::TBitmap();
			AdjustBitmapFormat(pBitmap);
		}
		pBitmap->Assign(sp->pBitmap);
	}
	else if( pBitmap != NULL ){
		delete pBitmap;
		pBitmap = NULL;
	}
}

int __fastcall CDrawTitle::Edit(void)
{
	TColorBarDlg *pBox = new TColorBarDlg(Mmsstv);
	int r = pBox->Execute(this);
	delete pBox;
	if( r == TRUE ){
		m_X2 = SXW - 1;
		sys.m_ColBar[0] = m_Col1;
		sys.m_ColBar[1] = m_Col2;
		sys.m_ColBar[2] = m_Col3;
		sys.m_ColBar[3] = m_Col4;
	}
	return r;
}

void __fastcall CDrawTitle::SaveToStream(TStream *sp)
{
	m_Ver = 3;
	CDrawBox::SaveToStream(sp);

	sp->Write(&m_Type, sizeof(m_Type));
	sp->Write(&m_ColVert, sizeof(m_ColVert));
	sp->Write(&m_Col1, sizeof(m_Col1));
	sp->Write(&m_Col2, sizeof(m_Col2));
	sp->Write(&m_Col3, sizeof(m_Col3));
	sp->Write(&m_Col4, sizeof(m_Col4));
	if( m_Type == 4 ) SaveString(sp, m_Sound);
	if( m_Type == 3 ){
		SaveBitmap(sp, pBitmap);
	}
}

int __fastcall CDrawTitle::LoadFromStream(TCanvas *tp, TStream *sp)
{
	if( CDrawBox::LoadFromStream(tp, sp) == FALSE ) return FALSE;

	try {
		if( m_Ver >= 1 ){
			sp->Read(&m_Type, sizeof(m_Type));
			if( m_Ver >= 2 ){
				sp->Read(&m_ColVert, sizeof(m_ColVert));
			}
			sp->Read(&m_Col1, sizeof(m_Col1));
			sp->Read(&m_Col2, sizeof(m_Col2));
			sp->Read(&m_Col3, sizeof(m_Col3));
			sp->Read(&m_Col4, sizeof(m_Col4));
            if( (m_Ver >= 3) && (m_Type == 4) ){
				LoadString(sp, m_Sound);
            }
			if( m_Type == 3 ){
				if( pBitmap == NULL ){
					pBitmap = new Graphics::TBitmap();
					AdjustBitmapFormat(pBitmap);
				}
				LoadBitmap(sp, pBitmap);
			}
		}
		return TRUE;
	}
	catch(...){
		return FALSE;
	}
}
//---------------------------------------------------------------------------
__fastcall CDrawText::CDrawText()
{
	m_Command = CM_TEXT;

	m_Grade = sys.m_TextGrade;
	m_Shadow = sys.m_TextShadow;
	m_Zero = sys.m_TextZero;
	m_Rot = sys.m_TextRot;
	m_RightAdj = 0;
	m_X2 = m_Y2 = 0;
	m_LineColor = clBlack;
	m_LineStyle = psSolid;
	m_Stack = sys.m_TextStack >> 16;
	m_StackPara = sys.m_TextStack & 0x0000ffff;
	m_Vert = sys.m_TextVert;
	m_VertH = sys.m_TextVertH;

	m_Text = sys.m_TextList[0].c_str();
	m_Col1 = sys.m_ColText[0];
	m_Col2 = sys.m_ColText[1];
	m_Col3 = sys.m_ColText[2];
	m_Col4 = sys.m_ColText[3];
	m_ColS = sys.m_ColText[4];
	m_ColB = sys.m_ColText[5];

	pFont = NULL;
	pBitmap = NULL;
	pBrushBitmap = NULL;

	m_PerSpect = 0;
	m_sperspect.ax = 2.0; m_sperspect.ay = 6.0;
	m_sperspect.px = 0.05;
	m_sperspect.py = m_sperspect.pz = 0.0;
	m_sperspect.rz = -180; m_sperspect.rx = -70; m_sperspect.ry = 0;
	m_sperspect.v = 10; m_sperspect.s = 4.9;

	m_Ver = 7;
}

//---------------------------------------------------------------------------
__fastcall CDrawText::~CDrawText()
{
	if( pFont != NULL ){
		delete pFont;
		pFont = NULL;
	}
	if( pBitmap != NULL ){
		delete pBitmap;
		pBitmap = NULL;
	}
	if( pBrushBitmap != NULL ){
		delete pBrushBitmap;
		pBrushBitmap = NULL;
	}
}

void __fastcall CDrawText::ReleaseResource(void)
{
	if( pBitmap != NULL ){
		delete pBitmap;
		pBitmap = NULL;
	}
}

int __fastcall CDrawText::ZeroConv(BYTE charset, LPSTR s)
{
	int f = 0;

	char bf[1228];
	LPSTR t = bf;
	LPSTR p = s;
	for( ; *p; p++ ){
		if( *p == '0' ){
			switch(charset){
				case SHIFTJIS_CHARSET:
					*t++ = 0x83;
					*t++ = 0xd3;
					break;
				case HANGEUL_CHARSET:
					*t++ = 0xa8;    // 0xa9
					*t++ = 0xaa;
					break;
				case CHINESEBIG5_CHARSET:   // 台湾
					*t++ = 0xa3;
					*t++ = 0x58;
					break;
				case 134:                   // 中国語簡略
					*t++ = 0xa6;
					*t++ = 0xd5;
					break;
				default:
					if( (lcid == LANG_KOREAN) || (lcid == LANG_CHINESE) ){
						f++;
						*t++ = '0';
					}
					else {
						*t++ = 'ﾘ';
					}
					break;
			}
		}
		else {
			*t++ = *p;
		}
	}
	*t = 0;
	StrCopy(s, bf, 1023);
	return f;
}

int __fastcall CDrawText::GetMaxWidth(int &n, LPCSTR p)
{
	n = 0;
	int max = 0;

	char bf[4];
	while(*p){
		if( (_mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD) && *(p+1) ){
//        if( IsDBCSLeadByte(*(const unsigned char *)p) && *(p+1) ){
			bf[0] = *p++;
			bf[1] = *p++;
			bf[2] = 0;
		}
		else {
			bf[0] = *p++;
			bf[1] = 0;
		}
		int len = pc->TextWidth(bf);
		if( max < len ){
			max = len;
		}
		n++;
	}
	return max;
}

inline TXOut(TCanvas *cp, int x, int y, LPCSTR p){cp->TextOut(x, y, p); return 0;};

void __fastcall CDrawText::DrawTextVert(TCanvas *tp, int xc, int y, int fh, LPCSTR p)
{
	char bf[4];

	int tm = ::SetBkMode(tp->Handle, TRANSPARENT);
	while(*p){
		if( (_mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD) && *(p+1) ){
			bf[0] = *p++;
			bf[1] = *p++;
			bf[2] = 0;
		}
		else if( (*p == '0') && m_Zero ){
			bf[0] = 'ﾘ';
			bf[1] = 0;
			p++;
		}
		else {
			bf[0] = *p++;
			bf[1] = 0;
		}
		int FW = tp->TextWidth(bf);
		int FH = tp->TextHeight(bf);
#if 1
		TXOut(tp, xc - (FW/2), y, bf);
#else
		tp->TextOut(xc - (FW/2), y, bf);            
#endif
		y += (FH + fh);
	}
	::SetBkMode(tp->Handle, tm);
}

void __fastcall CDrawText::DrawTextHoriANSI(TCanvas *tp, int x, int y, LPCSTR p)
{
	char bf[4];

	int tm = ::SetBkMode(tp->Handle, TRANSPARENT);
	int FW;
	double rot = m_Rot * PI/180.0;
	while(*p){
		if( (_mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD) && *(p+1) ){
			bf[0] = *p++;
			bf[1] = *p++;
			bf[2] = 0;
		}
		else if( *p == '0' ){
			bf[0] = 'ﾘ';
			bf[1] = 0;
			p++;
		}
		else {
			bf[0] = *p++;
			bf[1] = 0;
		}
		FW = tp->TextWidth(bf);
#if 1
		TXOut(tp, x, y, bf);
#else
		tp->TextOut(x, y, bf);
#endif
		if( m_Rot ){
			x += FW * cos(rot);
			y -= FW * sin(rot);
		}
		else {
			x += FW;
		}
	}
	::SetBkMode(tp->Handle, tm);
}

void __fastcall CDrawText::DrawText(TCanvas *cp, int x, int y, LPCSTR p, int zc)
{
	if( m_Vert ){
		DrawTextVert(cp, x, y, m_VertH, p);
	}
	else if( zc ){
		DrawTextHoriANSI(cp, x, y, p);
	}
	else {
#if 1
		TXOut(cp, x, y, p);
#else
		cp->TextOut(x, y, p);
#endif
//        DrawTextHoriANSI(cp, x, y, p);    // test of Korean Windows
	}
}

Graphics::TBitmap * __fastcall CDrawText::DupeB(TColor col)
{
	MultProc();
	pBitmap->Transparent = TRUE;
	pBitmap->TransparentMode = tmAuto;
	Graphics::TBitmap *pBit = new Graphics::TBitmap();
//	AdjustBitmapFormat(pBit);
	pBit->Assign(pBitmap);
	MultProc();
	FillBitmap(pBit, col);
	return pBit;
}

Graphics::TBitmap * __fastcall CDrawText::ConvToDDB(Graphics::TBitmap *pBmp)
{
	if( (sys.m_BitPixel == 16) && (pBmp->PixelFormat == pf24bit) ){
		pBmp->Transparent = FALSE;
		Graphics::TBitmap *pNew = DupeBitmap(pBmp, -1);
		pNew->Canvas->Font->Assign(pBmp->Canvas->Font);
        delete pBmp;
        pBmp = pNew;
	}
    return pBmp;
}

Graphics::TBitmap * __fastcall CDrawText::ConvToDIB(Graphics::TBitmap *pBmp)
{
	if( pBmp->PixelFormat != pf24bit ){
		pBmp->Transparent = FALSE;
		Graphics::TBitmap *pNew = DupeBitmap(pBmp, pf24bit);
        delete pBmp;
        pBmp = pNew;
	}
    return pBmp;
}

void __fastcall CDrawText::MakeBitmap(void)
{
	if( m_Text.IsEmpty() ) return;
	char bf[1024];
	Mmsstv->MacroText(bf, m_Text.c_str(), sizeof(bf)-1);
	int ZC = 0;
	if( m_Zero ) ZC = ZeroConv(pFont->Charset, bf);

	CWaitCursor wait;
	if( pBitmap != NULL ){
		delete pBitmap;
	}
	pc->Font->Assign(pFont);
	int wx = (pFont->Size + 15)/ 16;
	int x2 = m_X2;
	int y2 = m_Y2;
	if( m_Rot ){
		m_X1 = m_X;
		m_Y1 = m_Y;
	}
	else {
		m_X = m_X1;
		m_Y = m_Y1;
	}
	if( m_Vert ){
		SetMBCP(pFont->Charset);
		int len;
		m_X2 = m_X1 + GetMaxWidth(len, bf);
		m_Y2 = m_Y1 + ((pc->TextHeight(bf) + m_VertH) * len) + wx + wx;
	}
	else {
		m_X2 = m_X1 + pc->TextWidth(bf) + wx + wx;
		m_Y2 = m_Y1 + pc->TextHeight(bf) + wx;
		if( ZC ) m_X2 += (pc->TextWidth("0") * ZC);
	}
	int xt, yt;
	switch(m_Shadow){
		case 1:
			xt = yt = m_LineWidth + 1;
			break;
		case 4:
		case 5:
			if( !m_Stack ){
				xt = yt = 0;
				break;
			}
		case 6:
		case 7:
			xt = yt = 2;
			break;
		default:
			xt = yt = 0;
			break;
	}
	if( m_Vert ) xt += (m_X2-m_X1)/2;
	if( m_Rot ){
		m_Y1 -= wx;
		m_X1 -= wx;
		m_X2 += wx;
		m_Y2 += wx;
		double s = 2 * PI * m_Rot / 360;
		double cosk = cos(s);
		double sink = sin(s);
		int xc = (m_X1 + m_X2)/2;
		int yc = (m_Y1 + m_Y2)/2;
		POINT N[5];
		N[0].x = m_X1 - xc;
		N[0].y = m_Y1 - yc;
		N[1].x = m_X2 - xc;
		N[1].y = m_Y1 - yc;
		N[2].x = m_X2 - xc;
		N[2].y = m_Y2 - yc;
		N[3].x = m_X1 - xc;
		N[3].y = m_Y2 - yc;
		N[4].x = xt + m_X1 - xc;
		N[4].y = yt + m_Y1 - yc;
		int i;
		for( i = 0; i < 5; i++ ){
			int x = N[i].x * cosk + N[i].y * sink;
			int y = -N[i].x * sink + N[i].y * cosk;
			N[i].x = x + xc; N[i].y = y + yc;
		}
		m_X1 = m_X2 = N[0].x; m_Y1 = m_Y2 = N[0].y;
		for( i = 1; i < 4; i++ ){
			if( m_X1 > N[i].x ) m_X1 = N[i].x;
			if( m_X2 < N[i].x ) m_X2 = N[i].x;
			if( m_Y1 > N[i].y ) m_Y1 = N[i].y;
			if( m_Y2 < N[i].y ) m_Y2 = N[i].y;
		}
		xt = N[4].x - m_X1;
		yt = N[4].y - m_Y1;
	}
	switch(m_Shadow){
		case 0:
			break;
		case 1:
			m_X2 += m_LineWidth + m_LineWidth + 2;
			m_Y2 += m_LineWidth + m_LineWidth + 2;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 7:
			m_X2 += wx;
			m_Y2 += wx;
		case 6:
			m_X2 += wx + wx;
			m_Y2 += wx;
			break;
	}
	int ax, ay;
	if( m_Stack ){
		ax = int(char(m_StackPara & 0x000000ff));
		ay = int(char((m_StackPara >> 8) & 0x000000ff));
		if( ax > 0 ){
			xt += ax;
		}
		m_X2 += ABS(ax);
		if( ay > 0 ){
			yt += ay;
		}
		m_Y2 += ABS(ay);
		ax = -ax; ay = -ay;
	}
	AdjustRect();
	TFontStyles ts = pFont->Style;
	int fsw = FontStyle2Code(ts);
	if( fsw & FSITALIC ) m_X2 += wx * 3;
	int pf24 = sys.m_Temp24;
	pBitmap = CreateBitmap(m_X2 - m_X1 + 1, m_Y2 - m_Y1 + 1, pf24 ? pf24bit : -1);
	COLD cBack;
    cBack.c = TransCol;
	if( cBack.c == m_Col1 ){
		cBack.c = GetUniqueColor(m_Col1);
	}
	if( m_Shadow && (cBack.c == m_ColS) ){
		cBack.c = GetUniqueColor(cBack.c);
	}
	FillBitmap(pBitmap, cBack.c);
	cBack.c = pBitmap->Canvas->Pixels[0][0];
	if( m_RightAdj ){
		if( m_Vert && y2 ){
			y2 = m_Y2 - y2;
			m_Y1 -= y2; m_Y2 -= y2;
		}
		else if( x2 ){
			x2 = m_X2 - x2;
			m_X1 -= x2; m_X2 -= x2;
		}
	}
	MultProc();

	if( m_Rot || sys.m_DisFontSmooth ){
		LOGFONT lf;
		memset(&lf, 0, sizeof(lf));
		lf.lfHeight = pFont->Height;
		lf.lfWidth = 0;
		lf.lfEscapement = m_Rot * 10;
		lf.lfOrientation = m_Rot * 10;
		lf.lfWeight = fsw & FSBOLD ? 700 : 400;
		lf.lfItalic = BYTE(fsw & FSITALIC ? TRUE : FALSE);
        lf.lfUnderline = BYTE(fsw & FSUNDERLINE ? TRUE : FALSE);
        lf.lfStrikeOut = BYTE(fsw & FSSTRIKEOUT ? TRUE : FALSE);
		lf.lfCharSet = pFont->Charset;
		lf.lfOutPrecision = OUT_CHARACTER_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = BYTE(sys.m_DisFontSmooth ? NONANTIALIASED_QUALITY : PROOF_QUALITY);
		lf.lfPitchAndFamily = DEFAULT_PITCH;
		strcpy(lf.lfFaceName, AnsiString(pFont->Name).c_str());	//ja7ude 0521

		pBitmap->Canvas->Font->Handle = CreateFontIndirect(&lf);
	}
	else {
		pBitmap->Canvas->Font = pFont;
	}
	pBitmap->Canvas->Font->Color = m_Col1;
	DrawText(pBitmap->Canvas, xt, yt, bf, ZC);

	COLD col;
	int x, y;
	int XW = pBitmap->Width;
	int YW = pBitmap->Height;
	int XL = XW;
	int XR = 0;
	int YT = YW;
	int YB = 0;
	BYTE **pY = NULL;
	BYTE *bp = NULL;
	BYTE *sp;
	BYTE *wp;
	if( m_Grade || m_Shadow ){
		bp = new BYTE[XW * YW];
        wp = bp;
		if( pf24 ){
			col.d = 0;
			pY = new BYTE*[YW];
			for( y = 0; y < YW; y++ ){
				sp = (BYTE *)pBitmap->ScanLine[y];
                pY[y] = sp;
				for( x = 0; x < XW; x++, wp++ ){
					col.b.b = *sp++;
					col.b.g = *sp++;
					col.b.r = *sp++;
					if( col.c != cBack.c ){
						if( XL > x ) XL = x;
						if( XR < x ) XR = x;
						if( YT > y ) YT = y;
						if( YB < y ) YB = y;
						*wp = 1;
					}
					else {
						*wp = 0;
					}
				}
			}
		}
		else {
			for( y = 0; y < YW; y++ ){
				for( x = 0; x < XW; x++, wp++ ){
					if( pBitmap->Canvas->Pixels[x][y] != cBack.c ){
						if( XL > x ) XL = x;
						if( XR < x ) XR = x;
						if( YT > y ) YT = y;
						if( YB < y ) YB = y;
						*wp = 1;
					}
					else {
						*wp = 0;
					}
				}
			}
		}
		MultProc();
	}
	int X1, X2, Y1, Y2;
	switch(m_Grade){
		case 0:
			if( m_Shadow ){
				col.c = m_Col1;
				if( pf24 ){
					for( y = YT; y <= YB; y++ ){
						sp = &bp[y*XW + XL];
//						wp = (BYTE *)pBitmap->ScanLine[y] + (XL * 3);
						wp = (BYTE *)pY[y] + (XL * 3);
						for( x = XL; x <= XR; x++, sp++ ){
							if( *sp ){
								*wp++ = col.b.b;
								*wp++ = col.b.g;
								*wp++ = col.b.r;
							}
							else {
								wp += 3;
							}
						}
					}
				}
				else {
					for( y = YT; y <= YB; y++ ){
						sp = &bp[y*XW + XL];
						for( x = XL; x <= XR; x++, sp++ ){
							if( *sp ){
								pBitmap->Canvas->Pixels[x][y] = col.c;
							}
						}
					}
				}
			}
			break;
		case 1:
			if( pf24 ){
				X1 = (XR - XL) / 3;
				for( x = XL; x < X1+XL; x++ ){
					col.c = GetCol(m_Col1, m_Col2, x - XL, X1);
					sp = &bp[YT*XW + x];
					for( y = YT; y <= YB; y++, sp += XW ){
						if( *sp ){
//							wp = (BYTE *)pBitmap->ScanLine[y] + (x * 3);
							wp = (BYTE *)pY[y] + (x * 3);
							*wp++ = col.b.b;
							*wp++ = col.b.g;
							*wp = col.b.r;
						}
					}
				}
				MultProc();
				X2 = X1+X1;
				for( ; x < X2+XL; x++ ){
					col.c = GetCol(m_Col2, m_Col3, x - X1 - XL, X1);
					sp = &bp[YT*XW + x];
					for( y = YT; y <= YB; y++, sp += XW ){
						if( *sp ){
//							wp = (BYTE *)pBitmap->ScanLine[y] + (x * 3);
							wp = (BYTE *)pY[y] + (x * 3);
							*wp++ = col.b.b;
							*wp++ = col.b.g;
							*wp = col.b.r;
						}
					}
				}
				MultProc();
				for( ; x <= XR; x++ ){
					col.c = GetCol(m_Col3, m_Col4, x - X2 - XL, X1);
					sp = &bp[YT*XW + x];
					for( y = YT; y <= YB; y++, sp += XW ){
						if( *sp ){
//							wp = (BYTE *)pBitmap->ScanLine[y] + (x * 3);
							wp = (BYTE *)pY[y] + (x * 3);
							*wp++ = col.b.b;
							*wp++ = col.b.g;
							*wp = col.b.r;
						}
					}
				}
			}
			else {
				X1 = (XR - XL) / 3;
				for( x = XL; x < X1+XL; x++ ){
					col.c = GetCol(m_Col1, m_Col2, x - XL, X1);
					sp = &bp[YT*XW + x];
					for( y = YT; y <= YB; y++, sp += XW ){
						if( *sp ){
							pBitmap->Canvas->Pixels[x][y] = col.c;
						}
					}
				}
				MultProc();
				X2 = X1+X1;
				for( ; x < X2+XL; x++ ){
					col.c = GetCol(m_Col2, m_Col3, x - X1 - XL, X1);
					sp = &bp[YT*XW + x];
					for( y = YT; y <= YB; y++, sp += XW ){
						if( *sp ){
							pBitmap->Canvas->Pixels[x][y] = col.c;
						}
					}
				}
				MultProc();
				for( ; x <= XR; x++ ){
					col.c = GetCol(m_Col3, m_Col4, x - X2 - XL, X1);
					sp = &bp[YT*XW + x];
					for( y = YT; y <= YB; y++, sp += XW ){
						if( *sp ){
							pBitmap->Canvas->Pixels[x][y] = col.c;
						}
					}
				}
			}
			break;
		case 2:
			if( pf24 ){
				Y1 = (YB - YT) / 3;
				for( y = YT; y < Y1+YT; y++ ){
					col.c = GetCol(m_Col1, m_Col2, y - YT, Y1);
					sp = &bp[y*XW + XL];
//					wp = (BYTE *)pBitmap->ScanLine[y] + (XL * 3);
					wp = (BYTE *)pY[y] + (XL * 3);
					for( x = XL; x <= XR; x++, sp++ ){
						if( *sp ){
							*wp++ = col.b.b;
							*wp++ = col.b.g;
							*wp++ = col.b.r;
						}
						else {
							wp += 3;
						}
					}
				}
				MultProc();
				Y2 = Y1+Y1;
				for( ; y < Y2 + YT; y++ ){
					col.c = GetCol(m_Col2, m_Col3, y - Y1 - YT, Y1);
					sp = &bp[y*XW + XL];
//					wp = (BYTE *)pBitmap->ScanLine[y] + (XL * 3);
					wp = (BYTE *)pY[y] + (XL * 3);
					for( x = XL; x <= XR; x++, sp++ ){
						if( *sp ){
							*wp++ = col.b.b;
							*wp++ = col.b.g;
							*wp++ = col.b.r;
						}
						else {
							wp += 3;
						}
					}
				}
				MultProc();
				for( ; y <= YB; y++ ){
					col.c = GetCol(m_Col3, m_Col4, y - Y2 - YT, Y1);
					sp = &bp[y*XW + XL];
//					wp = (BYTE *)pBitmap->ScanLine[y] + (XL * 3);
					wp = (BYTE *)pY[y] + (XL * 3);
					for( x = XL; x <= XR; x++, sp++ ){
						if( *sp ){
							*wp++ = col.b.b;
							*wp++ = col.b.g;
							*wp++ = col.b.r;
						}
						else {
							wp += 3;
						}
					}
				}
			}
			else {
				Y1 = (YB - YT) / 3;
				for( y = YT; y < Y1+YT; y++ ){
					col.c = GetCol(m_Col1, m_Col2, y - YT, Y1);
					sp = &bp[y*XW + XL];
					for( x = XL; x <= XR; x++, sp++ ){
						if( *sp ){
							pBitmap->Canvas->Pixels[x][y] = col.c;
						}
					}
				}
				MultProc();
				Y2 = Y1+Y1;
				for( ; y < Y2 + YT; y++ ){
					col.c = GetCol(m_Col2, m_Col3, y - Y1 - YT, Y1);
					sp = &bp[y*XW + XL];
					for( x = XL; x <= XR; x++, sp++ ){
						if( *sp ){
							pBitmap->Canvas->Pixels[x][y] = col.c;
						}
					}
				}
				MultProc();
				for( ; y <= YB; y++ ){
					col.c = GetCol(m_Col3, m_Col4, y - Y2 - YT, Y1);
					sp = &bp[y*XW + XL];
					for( x = XL; x <= XR; x++, sp++ ){
						if( *sp ){
							pBitmap->Canvas->Pixels[x][y] = col.c;
						}
					}
				}
			}
			break;
		case 3:
			if( (pBrushBitmap != NULL) && pBrushBitmap->Width && pBrushBitmap->Height ){
				if( pf24 ){
					for( y = YT; y <= YB; y++ ){
						sp = &bp[y*XW + XL];
//						wp = (BYTE *)pBitmap->ScanLine[y] + (XL * 3);
						wp = (BYTE *)pY[y] + (XL * 3);
						for( x = XL; x <= XR; x++, sp++ ){
							if( *sp ){
								col.c = pBrushBitmap->Canvas->Pixels[x % pBrushBitmap->Width][y % pBrushBitmap->Height];
								*wp++ = col.b.b;
								*wp++ = col.b.g;
								*wp++ = col.b.r;
							}
							else {
								wp += 3;
							}
						}
					}
				}
				else {
					for( y = YT; y <= YB; y++ ){
						sp = &bp[y*XW + XL];
						for( x = XL; x <= XR; x++, sp++ ){
							if( *sp ){
								pBitmap->Canvas->Pixels[x][y] = pBrushBitmap->Canvas->Pixels[x % pBrushBitmap->Width][y % pBrushBitmap->Height];
							}
						}
					}
				}
			}
			break;
	}

	MultProc();

	Graphics::TBitmap *pBit;
	int f, lw;
	switch(m_Shadow){
		case 1:
 //           if( wx > 1 ) wx--;
			col.c = m_ColS;
			lw = m_LineWidth;
_sh1:
			if( pf24 ){
				for( x = 0; x < XW; x++ ){
					f = 0;
					sp = &bp[x];
					for( y = 0; y < YW; y++, sp += XW ){
						if( *sp ){
							f = lw;
						}
						else if( f ){
//							wp = (BYTE *)pBitmap->ScanLine[y] + (x * 3);
							wp = (BYTE *)pY[y] + (x * 3);
							*wp++ = col.b.b;
							*wp++ = col.b.g;
							*wp = col.b.r;
							*sp = 1;
							f--;
						}
					}
					f = 0;
					y = YW - 1;
					sp = &bp[y*XW+x];
					for( ; y > 0; y--, sp -= XW ){
						if( *sp ){
							f = lw;
						}
						else if( f ){
//							wp = (BYTE *)pBitmap->ScanLine[y] + (x * 3);
							wp = (BYTE *)pY[y] + (x * 3);
							*wp++ = col.b.b;
							*wp++ = col.b.g;
							*wp = col.b.r;
							*sp = 1;
							f--;
						}
					}
				}
				MultProc();
				for( y = 0; y < YW; y++ ){
					f = 0;
					sp = &bp[y*XW];
//					wp = (BYTE *)pBitmap->ScanLine[y];
					wp = (BYTE *)pY[y];
					for( x = 0; x < XW; x++, sp++, wp+=3 ){
						if( *sp ){
							f = lw;
						}
						else if( f ){
							*wp++ = col.b.b;
							*wp++ = col.b.g;
							*wp = col.b.r;
							wp -= 2;
							*sp = 1;
							f--;
						}
					}
					f = 0;
					x = XW - 1;
					sp = &bp[y*XW+x];
//					wp = (BYTE *)pBitmap->ScanLine[y] + (x * 3);
					wp = (BYTE *)pY[y] + (x * 3);
					for( ; x > 0; x--, sp--, wp-=3 ){
						if( *sp ){
							f = lw;
						}
						else if( f ){
							*wp++ = col.b.b;
							*wp++ = col.b.g;
							*wp = col.b.r;
							wp -= 2;
							f--;
						}
					}
				}
			}
			else {
				for( x = 0; x < XW; x++ ){
					f = 0;
					sp = &bp[x];
					for( y = 0; y < YW; y++, sp += XW ){
						if( *sp ){
							f = lw;
						}
						else if( f ){
							pBitmap->Canvas->Pixels[x][y] = col.c;
							*sp = 1;
							f--;
						}
					}
					f = 0;
					y = YW - 1;
					sp = &bp[y*XW+x];
					for( ; y > 0; y--, sp -= XW ){
						if( *sp ){
							f = lw;
						}
						else if( f ){
							pBitmap->Canvas->Pixels[x][y] = col.c;
							*sp = 1;
							f--;
						}
					}
				}
				MultProc();
				for( y = 0; y < YW; y++ ){
					f = 0;
					sp = &bp[y*XW];
					for( x = 0; x < XW; x++, sp++ ){
						if( *sp ){
							f = lw;
						}
						else if( f ){
							pBitmap->Canvas->Pixels[x][y] = col.c;
							*sp = 1;
							f--;
						}
					}
					f = 0;
					x = XW - 1;
					sp = &bp[y*XW+x];
					for( ; x > 0; x--, sp-- ){
						if( *sp ){
							f = lw;
						}
						else if( f ){
							pBitmap->Canvas->Pixels[x][y] = col.c;
							f--;
						}
					}
				}
			}
			break;
		case 2:
			if( pf24 ) pBitmap = ConvToDDB(pBitmap);
			pBit = DupeB(cBack.c);
			pBit->Canvas->Font = pBitmap->Canvas->Font;
			pBit->Canvas->Font->Color = m_ColS;
			DrawText(pBit->Canvas, xt+wx, yt+wx, bf, ZC);
			pBit->Canvas->Draw(0, 0, pBitmap);
			delete pBitmap;
			pBitmap = pBit;
			break;
		case 3:
			if( pf24 ){
				col.c = m_ColS;
				for( x = 0; x < XW; x++ ){
					f = 0;
					sp = &bp[x];
					for( y = 0; y < YW; y++, sp += XW ){
						if( *sp ){
							f = wx;
						}
						else if( f ){
//							wp = (BYTE *)pBitmap->ScanLine[y] + (x * 3);
							wp = (BYTE *)pY[y] + (x * 3);
							*wp++ = col.b.b;
							*wp++ = col.b.g;
							*wp = col.b.r;
							*sp = 1;
							f--;
						}
					}
				}
				MultProc();
				for( y = 0; y < YW; y++ ){
					f = 0;
					sp = &bp[y*XW];
//					wp = (BYTE *)pBitmap->ScanLine[y];
					wp = (BYTE *)pY[y];
					for( x = 0; x < XW; x++, sp++, wp+=3 ){
						if( *sp ){
							f = wx;
						}
						else if( f ){
							*wp++ = col.b.b;
							*wp++ = col.b.g;
							*wp = col.b.r;
							wp -= 2;
							f--;
						}
					}
				}
			}
			else {
				col.c = m_ColS;
				for( x = 0; x < XW; x++ ){
					f = 0;
					sp = &bp[x];
					for( y = 0; y < YW; y++, sp += XW ){
						if( *sp ){
							f = wx;
						}
						else if( f ){
							pBitmap->Canvas->Pixels[x][y] = col.c;
							*sp = 1;
							f--;
						}
					}
				}
				MultProc();
				for( y = 0; y < YW; y++ ){
					f = 0;
					sp = &bp[y*XW];
					for( x = 0; x < XW; x++, sp++ ){
						if( *sp ){
							f = wx;
						}
						else if( f ){
							pBitmap->Canvas->Pixels[x][y] = col.c;
							f--;
						}
					}
				}
			}
			break;
		case 4:
		case 5:
		case 6:
			if( pf24 ) pBitmap = ConvToDDB(pBitmap);
			pBit = DupeB(cBack.c);
			pBit->Canvas->Font = pBitmap->Canvas->Font;
			{
				if( !m_Stack ){
					switch(m_Shadow){
						case 4:
							ax = wx;
							ay = wx*2;
							break;
						case 5:
							ax = wx*2;
							ay = wx;
							break;
						case 6:
							ax = wx*2;
							ay = wx*2;
							break;
					}
					ax++; ay++;
				}
				int m = ABS(ax);
				if( ABS(ax) < ABS(ay) ) m = ABS(ay);
				if( m_StackPara ){
					for( f = m; f > 0; f-- ){
						col.c = GetCol(m_ColS, m_ColB, f-1, m);
						pBit->Canvas->Font->Color = col.c;
						::SetBkMode(pBit->Canvas->Handle, TRANSPARENT);
						DrawText(pBit->Canvas, xt+(f*ax/m), yt+(f*ay/m), bf, ZC);
					}
				}
				MultProc();
				pBit->Canvas->Draw(0, 0, pBitmap);
				delete pBitmap;
				pBitmap = pBit;
			}
			if( (m_Shadow == 6) || m_Stack ){
				lw = 1;
				switch(m_Shadow){
					case 4:
						col.c = clBlack;
						break;
					case 5:
						col.c = clWhite;
						break;
					case 6:
						col.c = m_LineColor;
						lw = m_LineWidth;
						break;
				}
				MultProc();
				if( pf24 ) pBitmap = ConvToDIB(pBitmap);
				if( pY ){
					for( y = 0; y < YW; y++ ){
						pY[y] = (BYTE *)pBitmap->ScanLine[y];
                    }
                }
				goto _sh1;
			}
			break;
		case 7:
			if( pf24 ) pBitmap = ConvToDDB(pBitmap);
			pBit = DupeB(cBack.c);
			pBit->Canvas->Font = pBitmap->Canvas->Font;
			pBit->Canvas->Font->Color = m_ColS;
			::SetBkMode(pBit->Canvas->Handle, TRANSPARENT);
			DrawText(pBit->Canvas, xt-1, yt-1, bf, ZC);
			pBit->Canvas->Font->Color = m_ColB;
			::SetBkMode(pBit->Canvas->Handle, TRANSPARENT);
			DrawText(pBit->Canvas, xt+1, yt+1, bf, ZC);
			pBit->Canvas->Draw(0, 0, pBitmap);
			delete pBitmap;
			pBitmap = pBit;
			break;
	}
	if( bp != NULL ) delete bp;
    if( pY != NULL ) delete pY;
	if( m_Stack && m_StackPara && ((m_Shadow < 4) || (m_Shadow > 6)) ){
		MultProc();
		if( pf24 ) pBitmap = ConvToDDB(pBitmap);
		pBit = DupeB(cBack.c);
		int m = ABS(ax);
		if( ABS(ax) < ABS(ay) ) m = ABS(ay);
		for( f = m; f >= 0; f-- ){
			pBit->Canvas->Draw(f*ax/m, f*ay/m, pBitmap);
		}
		delete pBitmap;
		pBitmap = pBit;
	}
	if( m_PerSpect ){
		MultProc();
		pBit = CreateBitmap(pBitmap->Width, pBitmap->Height, -1);
		pBitmap->Transparent = FALSE;
		Perspect(pBitmap, pBit, &m_sperspect, cBack.c);
		delete pBitmap;
		pBitmap = pBit;
		pBitmap->Transparent = TRUE;
		pBitmap->TransparentMode = tmFixed;
		pBitmap->TransparentColor = pBitmap->Canvas->Pixels[0][0];
	}
	else {
		if( pf24 ) pBitmap = ConvToDDB(pBitmap);
		pBitmap->Transparent = TRUE;
		pBitmap->TransparentMode = tmAuto;
	}
	if( m_Vert ) SetMBCP();
}

#if 0
void __fastcall CDrawText::MakeBitmap16(void)
{
	if( m_Text.IsEmpty() ) return;
	char bf[1024];
	Mmsstv->MacroText(bf, m_Text.c_str(), sizeof(bf)-1);
	int ZC = 0;
	if( m_Zero ) ZC = ZeroConv(pFont->Charset, bf);

	CWaitCursor wait;
	if( pBitmap != NULL ){
		delete pBitmap;
	}
	pc->Font->Assign(pFont);
	int wx = (pFont->Size + 15)/ 16;
	int x2 = m_X2;
	int y2 = m_Y2;
	if( m_Rot ){
		m_X1 = m_X;
		m_Y1 = m_Y;
	}
	else {
		m_X = m_X1;
		m_Y = m_Y1;
	}
	if( m_Vert ){
		SetMBCP(pFont->Charset);
		int len;
		m_X2 = m_X1 + GetMaxWidth(len, bf);
		m_Y2 = m_Y1 + ((pc->TextHeight(bf) + m_VertH) * len) + wx + wx;
	}
	else {
		m_X2 = m_X1 + pc->TextWidth(bf) + wx + wx;
		m_Y2 = m_Y1 + pc->TextHeight(bf) + wx;
		if( ZC ) m_X2 += (pc->TextWidth("0") * ZC);
	}
	int xt, yt;
	switch(m_Shadow){
		case 1:
			xt = yt = m_LineWidth + 1;
			break;
		case 4:
		case 5:
			if( !m_Stack ){
				xt = yt = 0;
				break;
			}
		case 6:
		case 7:
			xt = yt = 2;
			break;
		default:
			xt = yt = 0;
			break;
	}
	if( m_Vert ) xt += (m_X2-m_X1)/2;
	if( m_Rot ){
		m_Y1 -= wx;
		m_X1 -= wx;
		m_X2 += wx;
		m_Y2 += wx;
		double s = 2 * PI * m_Rot / 360;
		double cosk = cos(s);
		double sink = sin(s);
		int xc = (m_X1 + m_X2)/2;
		int yc = (m_Y1 + m_Y2)/2;
		POINT N[5];
		N[0].x = m_X1 - xc;
		N[0].y = m_Y1 - yc;
		N[1].x = m_X2 - xc;
		N[1].y = m_Y1 - yc;
		N[2].x = m_X2 - xc;
		N[2].y = m_Y2 - yc;
		N[3].x = m_X1 - xc;
		N[3].y = m_Y2 - yc;
		N[4].x = xt + m_X1 - xc;
		N[4].y = yt + m_Y1 - yc;
		int i;
		for( i = 0; i < 5; i++ ){
			int x = N[i].x * cosk + N[i].y * sink;
			int y = -N[i].x * sink + N[i].y * cosk;
			N[i].x = x + xc; N[i].y = y + yc;
		}
		m_X1 = m_X2 = N[0].x; m_Y1 = m_Y2 = N[0].y;
		for( i = 1; i < 4; i++ ){
			if( m_X1 > N[i].x ) m_X1 = N[i].x;
			if( m_X2 < N[i].x ) m_X2 = N[i].x;
			if( m_Y1 > N[i].y ) m_Y1 = N[i].y;
			if( m_Y2 < N[i].y ) m_Y2 = N[i].y;
		}
		xt = N[4].x - m_X1;
		yt = N[4].y - m_Y1;
	}
	switch(m_Shadow){
		case 0:
			break;
		case 1:
			m_X2 += m_LineWidth + m_LineWidth + 2;
			m_Y2 += m_LineWidth + m_LineWidth + 2;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 7:
			m_X2 += wx;
			m_Y2 += wx;
		case 6:
			m_X2 += wx + wx;
			m_Y2 += wx;
			break;
	}
	int ax, ay;
	if( m_Stack ){
		ax = int(char(m_StackPara & 0x000000ff));
		ay = int(char((m_StackPara >> 8) & 0x000000ff));
		if( ax > 0 ){
			xt += ax;
		}
		m_X2 += ABS(ax);
		if( ay > 0 ){
			yt += ay;
		}
		m_Y2 += ABS(ay);
		ax = -ax; ay = -ay;
	}
	AdjustRect();
	TFontStyles ts = pFont->Style;
	int fsw = FontStyle2Code(ts);
	if( fsw & FSITALIC ) m_X2 += wx * 3;
	pBitmap = CreateBitmap(m_X2 - m_X1 + 1, m_Y2 - m_Y1 + 1, -1);
	TColor cBack = TransCol;
	if( cBack == m_Col1 ){
		cBack = GetUniqueColor(m_Col1);
	}
	if( m_Shadow && (cBack == m_ColS) ){
		cBack = GetUniqueColor(cBack);
	}
	FillBitmap(pBitmap, cBack);
	cBack = pBitmap->Canvas->Pixels[0][0];

	if( m_RightAdj ){
		if( m_Vert && y2 ){
			y2 = m_Y2 - y2;
			m_Y1 -= y2; m_Y2 -= y2;
		}
		else if( x2 ){
			x2 = m_X2 - x2;
			m_X1 -= x2; m_X2 -= x2;
		}
	}

	MultProc();

	if( m_Rot || sys.m_DisFontSmooth ){
		LOGFONT lf;
		memset(&lf, 0, sizeof(lf));
		lf.lfHeight = pFont->Height;
		lf.lfWidth = 0;
		lf.lfEscapement = m_Rot * 10;
		lf.lfOrientation = m_Rot * 10;
		lf.lfWeight = fsw & FSBOLD ? 700 : 400;
		lf.lfItalic = BYTE(fsw & FSITALIC ? TRUE : FALSE);
        lf.lfUnderline = BYTE(fsw & FSUNDERLINE ? TRUE : FALSE);
        lf.lfStrikeOut = BYTE(fsw & FSSTRIKEOUT ? TRUE : FALSE);
		lf.lfCharSet = pFont->Charset;
		lf.lfOutPrecision = OUT_CHARACTER_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = BYTE(sys.m_DisFontSmooth ? NONANTIALIASED_QUALITY : PROOF_QUALITY);
		lf.lfPitchAndFamily = DEFAULT_PITCH;
		strcpy(lf.lfFaceName, pFont->Name.c_str());

		pBitmap->Canvas->Font->Handle = CreateFontIndirect(&lf);
	}
	else {
		pBitmap->Canvas->Font = pFont;
	}
	pBitmap->Canvas->Font->Color = m_Col1;
	DrawText(pBitmap->Canvas, xt, yt, bf, ZC);

	TColor col;
	int x, y;
	int XW = pBitmap->Width;
	int YW = pBitmap->Height;
	int XL = XW;
	int XR = 0;
	int YT = YW;
	int YB = 0;
	BYTE *bp = NULL;
	BYTE *sp;
	if( m_Grade || m_Shadow ){
		bp = new BYTE[XW * YW];
		sp = bp;
		for( y = 0; y < YW; y++ ){
			for( x = 0; x < XW; x++, sp++ ){
				if( pBitmap->Canvas->Pixels[x][y] != cBack ){
					if( XL > x ) XL = x;
					if( XR < x ) XR = x;
					if( YT > y ) YT = y;
					if( YB < y ) YB = y;
					*sp = 1;
				}
				else {
					*sp = 0;
				}
			}
		}
		MultProc();
	}
	int X1, X2, Y1, Y2;
	switch(m_Grade){
		case 0:
			if( m_Shadow ){
				for( y = YT; y <= YB; y++ ){
					sp = &bp[y*XW + XL];
					for( x = XL; x <= XR; x++, sp++ ){
						if( *sp ){
							pBitmap->Canvas->Pixels[x][y] = m_Col1;
						}
					}
				}
			}
			break;
		case 1:
			X1 = (XR - XL) / 3;
			for( x = XL; x < X1+XL; x++ ){
				col = GetCol(m_Col1, m_Col2, x - XL, X1);
				sp = &bp[YT*XW + x];
				for( y = YT; y <= YB; y++, sp += XW ){
					if( *sp ){
						pBitmap->Canvas->Pixels[x][y] = col;
					}
				}
			}
			MultProc();
			X2 = X1+X1;
			for( ; x < X2+XL; x++ ){
				col = GetCol(m_Col2, m_Col3, x - X1 - XL, X1);
				sp = &bp[YT*XW + x];
				for( y = YT; y <= YB; y++, sp += XW ){
					if( *sp ){
						pBitmap->Canvas->Pixels[x][y] = col;
					}
				}
			}
			MultProc();
			for( ; x <= XR; x++ ){
				col = GetCol(m_Col3, m_Col4, x - X2 - XL, X1);
				sp = &bp[YT*XW + x];
				for( y = YT; y <= YB; y++, sp += XW ){
					if( *sp ){
						pBitmap->Canvas->Pixels[x][y] = col;
					}
				}
			}
			break;
		case 2:
			Y1 = (YB - YT) / 3;
			for( y = YT; y < Y1+YT; y++ ){
				col = GetCol(m_Col1, m_Col2, y - YT, Y1);
				sp = &bp[y*XW + XL];
				for( x = XL; x <= XR; x++, sp++ ){
					if( *sp ){
						pBitmap->Canvas->Pixels[x][y] = col;
					}
				}
			}
			MultProc();
			Y2 = Y1+Y1;
			for( ; y < Y2 + YT; y++ ){
				col = GetCol(m_Col2, m_Col3, y - Y1 - YT, Y1);
				sp = &bp[y*XW + XL];
				for( x = XL; x <= XR; x++, sp++ ){
					if( *sp ){
						pBitmap->Canvas->Pixels[x][y] = col;
					}
				}
			}
			MultProc();
			for( ; y <= YB; y++ ){
				col = GetCol(m_Col3, m_Col4, y - Y2 - YT, Y1);
				sp = &bp[y*XW + XL];
				for( x = XL; x <= XR; x++, sp++ ){
					if( *sp ){
						pBitmap->Canvas->Pixels[x][y] = col;
					}
				}
			}
			break;
		case 3:
			if( (pBrushBitmap != NULL) && pBrushBitmap->Width && pBrushBitmap->Height ){
				for( y = YT; y <= YB; y++ ){
					sp = &bp[y*XW + XL];
					for( x = XL; x <= XR; x++, sp++ ){
						if( *sp ){
							pBitmap->Canvas->Pixels[x][y] = pBrushBitmap->Canvas->Pixels[x % pBrushBitmap->Width][y % pBrushBitmap->Height];
						}
					}
				}
			}
			break;
	}

	MultProc();

	Graphics::TBitmap *pBit;
	int f, lw;
	switch(m_Shadow){
		case 1:
 //           if( wx > 1 ) wx--;
			col = m_ColS;
			lw = m_LineWidth;
_sh1:
			for( x = 0; x < XW; x++ ){
				f = 0;
				sp = &bp[x];
				for( y = 0; y < pBitmap->Height; y++, sp += XW ){
					if( *sp ){
						f = lw;
					}
					else if( f ){
						pBitmap->Canvas->Pixels[x][y] = col;
						*sp = 1;
						f--;
					}
				}
				f = 0;
				y = YW - 1;
				sp = &bp[y*XW+x];
				for( ; y > 0; y--, sp -= XW ){
					if( *sp ){
						f = lw;
					}
					else if( f ){
						pBitmap->Canvas->Pixels[x][y] = col;
						*sp = 1;
						f--;
					}
				}
			}
			MultProc();
			for( y = 0; y < YW; y++ ){
				f = 0;
				sp = &bp[y*XW];
				for( x = 0; x < XW; x++, sp++ ){
					if( *sp ){
						f = lw;
					}
					else if( f ){
						pBitmap->Canvas->Pixels[x][y] = col;
						*sp = 1;
						f--;
					}
				}
				f = 0;
				x = XW - 1;
				sp = &bp[y*XW+x];
				for( ; x > 0; x--, sp-- ){
					if( *sp ){
						f = lw;
					}
					else if( f ){
						pBitmap->Canvas->Pixels[x][y] = col;
						f--;
					}
				}
			}
			break;
		case 2:
			pBit = DupeB(cBack);
			pBit->Canvas->Font = pBitmap->Canvas->Font;
			pBit->Canvas->Font->Color = m_ColS;
			DrawText(pBit->Canvas, xt+wx, yt+wx, bf, ZC);
			pBit->Canvas->Draw(0, 0, pBitmap);
			delete pBitmap;
			pBitmap = pBit;
			break;
		case 3:
			col = m_ColS;
			for( x = 0; x < XW; x++ ){
				f = 0;
				sp = &bp[x];
				for( y = 0; y < YW; y++, sp += XW ){
					if( *sp ){
						f = wx;
					}
					else if( f ){
						pBitmap->Canvas->Pixels[x][y] = col;
						*sp = 1;
						f--;
					}
				}
			}
			MultProc();
			for( y = 0; y < YW; y++ ){
				f = 0;
				sp = &bp[y*XW];
				for( x = 0; x < XW; x++, sp++ ){
					if( *sp ){
						f = wx;
					}
					else if( f ){
						pBitmap->Canvas->Pixels[x][y] = col;
						f--;
					}
				}
			}
			break;
		case 4:
		case 5:
		case 6:
			pBit = DupeB(cBack);
			pBit->Canvas->Font = pBitmap->Canvas->Font;
			{
				if( !m_Stack ){
					switch(m_Shadow){
						case 4:
							ax = wx;
							ay = wx*2;
							break;
						case 5:
							ax = wx*2;
							ay = wx;
							break;
						case 6:
							ax = wx*2;
							ay = wx*2;
							break;
					}
					ax++; ay++;
				}
				int m = ABS(ax);
				if( ABS(ax) < ABS(ay) ) m = ABS(ay);
				if( m_StackPara ){
					for( f = m; f > 0; f-- ){
						col = GetCol(m_ColS, m_ColB, f-1, m);
						pBit->Canvas->Font->Color = col;
						::SetBkMode(pBit->Canvas->Handle, TRANSPARENT);
						DrawText(pBit->Canvas, xt+(f*ax/m), yt+(f*ay/m), bf, ZC);
					}
				}
				MultProc();
				pBit->Canvas->Draw(0, 0, pBitmap);
				delete pBitmap;
				pBitmap = pBit;
			}
			if( (m_Shadow == 6) || m_Stack ){
				lw = 1;
				switch(m_Shadow){
					case 4:
						col = clBlack;
						break;
					case 5:
						col = clWhite;
						break;
					case 6:
						col = m_LineColor;
						lw = m_LineWidth;
						break;
				}
				MultProc();
				goto _sh1;
			}
			break;
		case 7:
			pBit = DupeB(cBack);
			pBit->Canvas->Font = pBitmap->Canvas->Font;
			pBit->Canvas->Font->Color = m_ColS;
			::SetBkMode(pBit->Canvas->Handle, TRANSPARENT);
			DrawText(pBit->Canvas, xt-1, yt-1, bf, ZC);
			pBit->Canvas->Font->Color = m_ColB;
			::SetBkMode(pBit->Canvas->Handle, TRANSPARENT);
			DrawText(pBit->Canvas, xt+1, yt+1, bf, ZC);
			pBit->Canvas->Draw(0, 0, pBitmap);
			delete pBitmap;
			pBitmap = pBit;
			break;
	}
	if( bp != NULL ) delete bp;
	if( m_Stack && m_StackPara && ((m_Shadow < 4) || (m_Shadow > 6)) ){
		MultProc();
		pBit = DupeB(cBack);
		int m = ABS(ax);
		if( ABS(ax) < ABS(ay) ) m = ABS(ay);
		for( f = m; f >= 0; f-- ){
			pBit->Canvas->Draw(f*ax/m, f*ay/m, pBitmap);
		}
		delete pBitmap;
		pBitmap = pBit;
	}
	if( m_PerSpect ){
		MultProc();
		pBit = CreateBitmap(pBitmap->Width, pBitmap->Height, pBitmap->PixelFormat);
		pBitmap->Transparent = FALSE;
		Perspect(pBitmap, pBit, &m_sperspect, cBack);
		delete pBitmap;
		pBitmap = pBit;
		pBitmap->Transparent = TRUE;
		pBitmap->TransparentMode = tmFixed;
		pBitmap->TransparentColor = pBitmap->Canvas->Pixels[0][0];
	}
	else {
		pBitmap->Transparent = TRUE;
		pBitmap->TransparentMode = tmAuto;
	}
	if( m_Vert ) SetMBCP();
}
#endif

void __fastcall CDrawText::Move(int x, int y)
{
	if( (m_CX1 == m_CX2) || (m_CY1 == m_CY2) ){
		return;
	}
	if( m_MC != HT_I ){
		double dx = fabs(double(m_CX2 - m_CX1)/double(m_X2 - m_X1));
		double dy = fabs(double(m_CY2 - m_CY1)/double(m_Y2 - m_Y1));
		double dxx = fabs(dx - 1.0);
		double dyy = fabs(dy - 1.0);
		if( dxx < dyy ) dx = dy;
		int sz = pFont->Size;
		sz = sz * dx + 0.5;
		if( sz && (sz != pFont->Size) ){
			pFont->Size = sz;
			CDrawBox::Move(x, y);
			MakeBitmap();
		}
	}
	else {
		int xx = m_X1;
		int yy = m_Y1;
		CDrawBox::Move(x, y);
		m_X += m_X1 - xx;
		m_Y += m_Y1 - yy;
	}
}

int __fastcall CDrawText::Finish(int x, int y)
{
	m_X2 = x;
	m_Y2 = y;
	m_X = m_X1;
	m_Y = m_Y1;
	pFont = new TFont();
	if( CurText.pFont != NULL ){
		pFont->Assign(CurText.pFont);
	}
	else {
		pFont->Assign(pc->Font);
		pFont->Size = 18;
	}
	MakeBitmap();
	return Edit();
}

void __fastcall CDrawText::Draw(TCanvas *tp)
{
	if( pBitmap == NULL ){
		MakeBitmap();
		if( pBitmap == NULL ) return;
	}
	tp->Draw(m_X1, m_Y1, pBitmap);
}

void __fastcall CDrawText::DrawFast(TCanvas *tp)
{
//    CDrawBoxS::Draw(tp);

	if( m_Text.IsEmpty() ) return;
	char bf[1024];
	Mmsstv->MacroText(bf, m_Text.c_str(), sizeof(bf)-1);

	tp->Font->Assign(pFont);
	tp->Font = pFont;
	tp->Font->Color = (m_Col1 != tp->Brush->Color) ? m_Col1 : TColor(m_Col1 ^ 0x00ffffff);
	int tm = ::SetBkMode(tp->Handle, TRANSPARENT);
	if( m_Vert ){
		DrawTextVert(tp, m_X1 + ((m_X2 - m_X1)/2), m_Y1, m_VertH, bf);
	}
	else {
		int x = m_X1;
        int y = m_Y1;
		if( m_RightAdj ){
			x = m_X2 - tp->TextWidth(bf);
        }
        if( m_Shadow == 1 ){
			x += m_LineWidth + 1;
            y += m_LineWidth + 1;
        }
		tp->TextOut(x, y, bf);
    }
	::SetBkMode(tp->Handle, tm);
}

int __fastcall CDrawText::Edit(void)
{
	TTextInDlg *pBox = new TTextInDlg(Mmsstv);
	int r = pBox->Execute(this);
	delete pBox;
	if( r == TRUE ){
		if( m_Text.IsEmpty() ){
			r = FALSE;
		}
		else {
			MakeBitmap();
			sys.m_ColText[0] = m_Col1;
			sys.m_ColText[1] = m_Col2;
			sys.m_ColText[2] = m_Col3;
			sys.m_ColText[3] = m_Col4;
			sys.m_ColText[4] = m_ColS;
			sys.m_ColText[5] = m_ColB;

			sys.m_TextGrade = m_Grade;
			sys.m_TextShadow = m_Shadow;
			sys.m_TextZero = m_Zero;
			sys.m_TextRot = m_Rot;
			sys.m_TextVert = m_Vert;
			sys.m_TextVertH = m_VertH;
			sys.m_TextStack = (m_Stack << 16) | m_StackPara;
			if( CurText.pFont == NULL ){
				CurText.pFont = new TFont;
			}
			CurText.pFont->Assign(pFont);
		}
	}
	return r;
}

void __fastcall CDrawText::UpdateFont(TFont *pf)
{
	if( pFont == NULL ){
		pFont = new TFont();
	}
	pFont->Assign(pf);
	MakeBitmap();
	if( CurText.pFont == NULL ){
		CurText.pFont = new TFont();
	}
	CurText.pFont->Assign(pFont);
}

void __fastcall CDrawText::SaveToStream(TStream *sp)
{
	m_Ver = 7;
	CDrawBox::SaveToStream(sp);

	sp->Write(&m_Grade, sizeof(m_Grade));
	sp->Write(&m_Shadow, sizeof(m_Shadow));
	sp->Write(&m_Zero, sizeof(m_Zero));
	sp->Write(&m_Rot, sizeof(m_Rot));
	sp->Write(&m_X, sizeof(m_X));
	sp->Write(&m_Y, sizeof(m_Y));
	sp->Write(&m_RightAdj, sizeof(m_RightAdj));
	sp->Write(&m_Stack, sizeof(m_Stack));
	sp->Write(&m_StackPara, sizeof(m_StackPara));
	sp->Write(&m_PerSpect, sizeof(m_PerSpect));
	if( m_PerSpect ){
		sp->Write(&m_sperspect, sizeof(m_sperspect));
	}
	sp->Write(&m_Vert, sizeof(m_Vert));
	sp->Write(&m_VertH, sizeof(m_VertH));

	SaveString(sp, m_Text);
	sp->Write(&m_Col1, sizeof(m_Col1));
	sp->Write(&m_Col2, sizeof(m_Col2));
	sp->Write(&m_Col3, sizeof(m_Col3));
	sp->Write(&m_Col4, sizeof(m_Col4));
	sp->Write(&m_ColS, sizeof(m_ColS));
	sp->Write(&m_ColB, sizeof(m_ColB));

	AnsiString pFontName = pFont->Name;
	SaveString(sp, pFontName);	//ja7ude 0521
	int d = pFont->Charset;
	sp->Write(&d, sizeof(d));
	d = pFont->Height;
	if( d >= 0 ) d = -d;
	sp->Write(&d, sizeof(d));
	TFontStyles ts = pFont->Style;
	d = FontStyle2Code(ts);
	sp->Write(&d, sizeof(d));
	d = 0;
	sp->Write(&d, sizeof(d));   // dummy
	if( m_Grade == 3 ){
		SaveBitmap(sp, pBrushBitmap);
	}
}

int __fastcall CDrawText::LoadFromStream(TCanvas *tp, TStream *sp)
{
	if( CDrawBox::LoadFromStream(tp, sp) == FALSE ) return FALSE;

	try {
		sp->Read(&m_Grade, sizeof(m_Grade));
		sp->Read(&m_Shadow, sizeof(m_Shadow));
		if( m_Ver >= 1 ){
			sp->Read(&m_Zero, sizeof(m_Zero));
		}
		if( m_Ver >= 2 ){
			sp->Read(&m_Rot, sizeof(m_Rot));
			sp->Read(&m_X, sizeof(m_X));
			sp->Read(&m_Y, sizeof(m_Y));
		}
		else {
			m_X = m_X1; m_Y = m_Y1;
		}
		if( m_Ver >= 4 ){
			sp->Read(&m_RightAdj, sizeof(m_RightAdj));
		}
		else {
			m_X2 = 0;
		}
		if( m_Ver >= 5 ){
			sp->Read(&m_Stack, sizeof(m_Stack));
			sp->Read(&m_StackPara, sizeof(m_StackPara));

			sp->Read(&m_PerSpect, sizeof(m_PerSpect));
			if( m_PerSpect ){
				sp->Read(&m_sperspect, sizeof(m_sperspect));
			}
		}
		else {
			m_Stack = 0;
			m_PerSpect = 0;
		}
		if( m_Ver >= 6 ){
			sp->Read(&m_Vert, sizeof(m_Vert));
			sp->Read(&m_VertH, sizeof(m_VertH));
		}
		else {
			m_Vert = 0; m_VertH = -6;
		}
		LoadString(sp, m_Text);
		sp->Read(&m_Col1, sizeof(m_Col1));
		sp->Read(&m_Col2, sizeof(m_Col2));
		sp->Read(&m_Col3, sizeof(m_Col3));
		sp->Read(&m_Col4, sizeof(m_Col4));
		sp->Read(&m_ColS, sizeof(m_ColS));
		if( m_Ver >= 3 ){
			sp->Read(&m_ColB, sizeof(m_ColB));
		}
		if( pFont == NULL ){
			pFont = new TFont();
		}

		AnsiString as;
		LoadString(sp, as);
		pFont->Name = as;
		int d;
		sp->Read(&d, sizeof(d));
		pFont->Charset = BYTE(d);
		sp->Read(&d, sizeof(d));
		if( d < 0 ){
			pFont->Height = d;
		}
		else {
			pFont->Size = d;
		}
		sp->Read(&d, sizeof(d));
		TFontStyles ts = Code2FontStyle(d);
		pFont->Style = ts;
		sp->Read(&d, sizeof(d)); // dummy
		if( m_Grade == 3 ){
			if( pBrushBitmap == NULL ){
				pBrushBitmap = new Graphics::TBitmap();
				AdjustBitmapFormat(pBrushBitmap);
			}
			LoadBitmap(sp, pBrushBitmap);
		}
		if( (m_Ver <= 6) && (m_Shadow == 6) && !m_Stack ){
			m_LineColor = m_ColS;
		}
		return TRUE;
	}
	catch(...){
		return FALSE;
	}
}


void __fastcall CDrawText::Copy(CDraw *dp)
{
	Copy(dp, 1);
}

void __fastcall CDrawText::Copy(CDraw *dp, int sw)
{
	CDrawBox::Copy(dp);
	CDrawText *sp = (CDrawText *)dp;

	m_Grade = sp->m_Grade;
	m_Shadow = sp->m_Shadow;
	m_Zero = sp->m_Zero;
	m_Rot = sp->m_Rot;
	m_RightAdj = sp->m_RightAdj;
	m_PerSpect = sp->m_PerSpect;
	memcpy(&m_sperspect, &sp->m_sperspect, sizeof(m_sperspect));
	m_Stack = sp->m_Stack;
	m_StackPara = sp->m_StackPara;
	m_Vert = sp->m_Vert;
	m_VertH = sp->m_VertH;
	m_X = sp->m_X;
	m_Y = sp->m_Y;
	m_Text = sp->m_Text;
	if( pFont == NULL ){
		pFont = new TFont;
	}
	pFont->Assign(sp->pFont);
	if( sp->pBrushBitmap != NULL ){
		if( pBrushBitmap == NULL ){
			pBrushBitmap = new Graphics::TBitmap();
			AdjustBitmapFormat(pBrushBitmap);
		}
		pBrushBitmap->Assign(sp->pBrushBitmap);
	}
	else if( pBrushBitmap != NULL ){
		delete pBrushBitmap;
		pBrushBitmap = NULL;
	}
	m_Col1 = sp->m_Col1;
	m_Col2 = sp->m_Col2;
	m_Col3 = sp->m_Col3;
	m_Col4 = sp->m_Col4;
	m_ColS = sp->m_ColS;
	m_ColB = sp->m_ColB;
	if( sw ){
		MakeBitmap();
	}
	else if( pBitmap == NULL ){
		pBitmap = new Graphics::TBitmap();
		AdjustBitmapFormat(pBitmap);
	}
}

void __fastcall CDrawText::UpdateText(void)
{
	MakeBitmap();
}

void __fastcall CDrawText::UpdateTimeText(void)
{
	if( IsTimeMacro() ){
		MakeBitmap();
	}
}

int __fastcall CDrawText::IsTimeMacro(void)
{
	char bf[1024];
	return Mmsstv->MacroText(bf, m_Text.c_str(), sizeof(bf)-1);
}

//---------------------------------------------------------------------------
__fastcall CDrawPic::CDrawPic()
{
	m_Command = CM_PIC;
	m_Type = 0;
	m_Shape = sys.m_PicShape;
	m_Adjust = sys.m_PicAdjust;
	m_TransPoint = 0;
	m_Ver = 5;
	pBitmap = NULL;
	pLoadBitmap = NULL;
	m_LineStyle = TPenStyle(sys.m_PicLineStyle);
	m_LineColor = sys.m_PicLineColor;
}

__fastcall CDrawPic::~CDrawPic()
{
	if( pBitmap != NULL ){
		delete pBitmap;
		pBitmap = NULL;
	}
	if( pLoadBitmap != NULL ){
		delete pLoadBitmap;
		pLoadBitmap = NULL;
	}
}

int __fastcall CDrawPic::Finish(int x, int y)
{
	m_X2 = x;
	m_Y2 = y;
	Normal();
	if( !((m_X1 != m_X2) && (m_Y1 != m_Y2)) ) return FALSE;
	CreateBitmap();
	StretchCopy(pBitmap, Mmsstv->pBitmapHistF, HALFTONE);
	SetLoadBitmap(Mmsstv->pBitmapHistF);
	return Edit();
}

void __fastcall CDrawPic::CreateBitmap(void)
{
	pBitmap = ::CreateBitmap(m_X2 - m_X1 - 1, m_Y2 - m_Y1 - 1, -1);
}

int __fastcall CDrawPic::FinishR(int x, int y)
{
	m_X2 = x;
	m_Y2 = y;
	Normal();
	if( !((m_X1 != m_X2) && (m_Y1 != m_Y2)) ) return FALSE;
	CreateBitmap();
	return TRUE;
}

void __fastcall CDrawPic::SetLoadBitmap(Graphics::TBitmap *pBmp)
{
	if( pLoadBitmap != NULL ) delete pLoadBitmap;
	pLoadBitmap = DupeBitmap(pBmp, -1);
}

void __fastcall CDrawPic::AdjustSize(void)
{
	ASSERT(pBitmap);
	if( (m_Type == 2) && (sys.m_BitPixel == 16) ){
		pBitmap = RemakeBitmap(pBitmap, -1);
	}
	pBitmap->Width = m_X2 - m_X1 - 1;
	pBitmap->Height = m_Y2 - m_Y1 - 1;
	if( m_Type && (pLoadBitmap != NULL) ){
		if( m_Type == 2 ){
			StretchCopy(pBitmap, pLoadBitmap, COLORONCOLOR);
		}
		else {
			StretchCopy(pBitmap, NULL, pLoadBitmap, NULL);
		}
	}
	m_DrawPolygon.Copy(&m_Polygon);
	m_DrawPolygon.XW = pBitmap->Width;
	m_DrawPolygon.YW = pBitmap->Height;
	m_DrawPolygon.Stretch(m_Polygon);
}

//---------------------------------------------------------------------------
int __fastcall CDrawPic::IsOrgSize(void)
{
	Graphics::TBitmap *pSrc = m_Type ? pLoadBitmap : Mmsstv->pBitmapHist;
	if( pSrc == NULL ) return TRUE;

	if( (pBitmap->Width) != pSrc->Width ) return FALSE;
	if( (pBitmap->Height) != pSrc->Height ) return FALSE;
	return TRUE;
}

//---------------------------------------------------------------------------
void __fastcall CDrawPic::SetOrgSize(void)
{
	Graphics::TBitmap *pSrc = m_Type ? pLoadBitmap : Mmsstv->pBitmapHist;
	if( pSrc == NULL ) return;

	m_X2 = m_X1 + pSrc->Width + 1;
	m_Y2 = m_Y1 + pSrc->Height + 1;
	AdjustSize();
}

//---------------------------------------------------------------------------
void __fastcall CDrawPic::KeepAspect(void)
{
	Graphics::TBitmap *pSrc = m_Type ? pLoadBitmap : Mmsstv->pBitmapHist;
	if( pSrc == NULL ) return;

	int lx = pBitmap->Width;
	int ly = pBitmap->Height;
	::KeepAspect(lx, ly, pSrc->Width, pSrc->Height);
	m_X2 = m_X1 + lx + 1;
	m_Y2 = m_Y1 + ly + 1;
	AdjustSize();
}

void __fastcall CDrawPic::Move(int x, int y)
{
	CDrawBox::Move(x, y);
	if( (m_X1 == m_X2) || (m_Y1 == m_Y2) ){
		return;
	}
	if( m_MC != HT_I ){
		if( pBitmap == NULL ){
			pBitmap = new Graphics::TBitmap();
		}
		else if( m_Type && (pLoadBitmap == NULL) ){
			pBitmap->Transparent = FALSE;
			pLoadBitmap = DupeBitmap(pBitmap, -1);
		}
		AdjustBitmapFormat(pBitmap);
		AdjustSize();
	}
}

#if 0
void __fastcall CDrawPic::RoundRect(TCanvas *tp, int xw, int yw)
{
	if( m_LineStyle < 0 ) return;

	if( m_LineStyle >= 5 ){
		int e = (m_LineStyle == 5) ? 4 : 3;
		tp->Pen->Style = psSolid;
		tp->Pen->Width = 1;
		int x1 = m_X1;
		int y1 = m_Y1;
		int x2 = m_X2;
		int y2 = m_Y2;
		int xxw = xw;
		int yyw = yw;
		TColor col = m_LineColor;
		TColor tcol = m_LineColor == clBlack ? clWhite : clBlack;
//            TColor tcol = TColor(DWORD(m_LineColor) ^ 0x00ffffff);
		int i;
		for( i = 0; i < e; i++ ){
			tp->Pen->Color = GetCol(col, tcol, i, e);
			tp->RoundRect(x1, y1, x2, y2, xxw, yyw);
			x1--; y1--;
			x2++; y2++;
			xxw += 2;
			yyw += 2;
		}
		x1 = m_X1+1;
		y1 = m_Y1+1;
		x2 = m_X2-1;
		y2 = m_Y2-1;
		xxw = xw;
		yyw = yw;
		for( i = 1; i < e; i++ ){
			tp->Pen->Color = GetCol(col, tcol, i, e);
			tp->RoundRect(x1, y1, x2, y2, xxw, yyw);
			x1++; y1++;
			x2--; y2--;
			xxw -= 2;
			yyw -= 2;
		}
	}
	else {
		tp->Pen->Style = m_LineStyle;
		tp->RoundRect(m_X1, m_Y1, m_X2, m_Y2, xw, yw);
	}
}
#endif

void __fastcall CDrawPic::Draw(TCanvas *tp)
{
	if( m_Type ){
		if( pBitmap == NULL ) return;
	}
	else {
		if( pBitmap == NULL ) CreateBitmap();
		UpdateHistPic(Mmsstv->pBitmapHistF);
	}

	if( m_Type == 2 ){      // オバーレイ
		int x = m_TransPoint & 0x0000ffff;
		int y = m_TransPoint >> 16;
		if( m_TransPoint && (pLoadBitmap != NULL) ){
			x = x * pBitmap->Width / pLoadBitmap->Width;
			y = y * pBitmap->Height / pLoadBitmap->Height;
		}
		pBitmap->Transparent = TRUE;
		pBitmap->TransparentMode = tmFixed;
		pBitmap->TransparentColor = pBitmap->Canvas->Pixels[x][y];
		tp->Draw(m_X1 + 1, m_Y1 + 1, pBitmap);
		pBitmap->Transparent = FALSE;
	}
	else if( m_Shape ){
		TRect rc;
		rc.Left = 0; rc.Top = 0;
		rc.Right = pBitmap->Width;
		rc.Bottom = pBitmap->Height;

		Graphics::TBitmap *pbmpS = DupeBitmap(pBitmap, pBitmap->PixelFormat);
		MultProc();
		Graphics::TBitmap *pbmp = ::CreateBitmap(pBitmap->Width, pBitmap->Height, -1);
		pbmp->Canvas->Brush->Color = tp->Brush->Color;
		pbmp->Canvas->Pen->Color = tp->Brush->Color;
		pbmp->Canvas->FillRect(rc);
#if 1
		TColor bcol = pbmp->Canvas->Pixels[0][0];
#else
		TColor bcol = tp->Brush->Color;
#endif
		if( bcol != clBlack ){
			pbmp->Canvas->Brush->Color = clBlack;
			pbmp->Canvas->Pen->Color = clBlack;
		}
		else {
			pbmp->Canvas->Brush->Color = clYellow;
			pbmp->Canvas->Pen->Color = clYellow;
		}
		switch(m_Shape){
			case 1:
			case 2:
			case 3:
			case 4:
				DBox(pbmp->Canvas, m_Shape, 0, 0, pBitmap->Width, pBitmap->Height);
				break;
			case 5:
				pbmp->Canvas->Polygon((TPoint*)m_DrawPolygon.pBase, m_DrawPolygon.Cnt - 1);
				break;
		}
		MultProc();
		pbmp->Transparent = TRUE;
		pbmp->TransparentMode = tmFixed;
		pbmp->TransparentColor = pbmp->Canvas->Brush->Color;
		pbmpS->Canvas->Draw(0, 0, pbmp);
		MultProc();
		pbmpS->Transparent = TRUE;
		pbmpS->TransparentMode = tmFixed;
		pbmpS->TransparentColor = bcol;
		MultProc();
		tp->Draw(m_X1 + 1, m_Y1 + 1, pbmpS);
		delete pbmp;
		delete pbmpS;
		MultProc();
//        if( (m_LineStyle >= 0) && ((m_LineColor != clWhite) || (m_LineStyle != psSolid)) ){
		if( m_LineStyle >= 0 ){
			tp->Pen->Color = m_LineColor;
			tp->Pen->Style = m_LineStyle < 5 ? m_LineStyle : psSolid;
			tp->Pen->Width = m_LineWidth;
			tp->Brush->Style = bsClear;
			switch(m_Shape){
				case 1:
					RoundRect(tp, pBitmap->Width/3, pBitmap->Height/3);
					break;
				case 2:
					RoundRect(tp, pBitmap->Width/2, pBitmap->Height/2);
					break;
				case 3:
					RoundRect(tp, pBitmap->Width*3/4, pBitmap->Height*3/4);
					break;
				case 4:
					RoundRect(tp, pBitmap->Width, pBitmap->Height);
//                    tp->Ellipse(m_X1, m_Y1, m_X2, m_Y2);
					break;
				case 5:
					if( m_LineStyle >= 5 ){
						CPolygon Pol;
						Pol.Copy(&m_DrawPolygon);
						Pol.DrawOffsetSiege(tp, m_X1, m_Y1, m_LineColor, m_LineStyle);
					}
					else {
						CPolygon Pol;
						Pol.Copy(&m_DrawPolygon);
						Pol.Stretch(m_X2-m_X1+1, m_Y2-m_Y1+1, Pol.XW, Pol.YW);
						Pol.DrawOffset(tp, m_X1, m_Y1);
					}
					break;
			}
			tp->Brush->Style = bsSolid;
		}
	}
	else {
		tp->Draw(m_X1 + 1, m_Y1 + 1, pBitmap);
		CDrawBox::Draw(tp);
	}
}

void __fastcall CDrawPic::FillPic(TCanvas *tp, TColor col, int sw, double sx, double sy, int off)
{
	if( pBitmap == NULL ) return;
	if( m_Type == 2 ) return;

	TRect rc;
	switch(sw){
		case 2:
			rc.Left = m_X1;
			rc.Top = m_Y1 - 16;
			rc.Right = m_X2;
			rc.Bottom = m_Y2 - 16;
			break;
		case 4:
			rc.Left = m_X1;
			rc.Top = m_Y1 * 240 / 256;
			rc.Right = m_X2;
			rc.Bottom = (m_Y2 * 240 / 256);
			break;
		case -1:
			rc.Left = m_X1;
			rc.Top = m_Y1 + off;
			rc.Right = (m_X2 + 1);
			rc.Bottom = m_Y2 + off;
			break;
		default:
			rc.Left = m_X1 * sx;
			rc.Top = m_Y1 * sy;
			rc.Right = m_X2 * sx;
			rc.Bottom = m_Y2 * sy;
			break;
	}
	if( m_LineStyle == 5 ){
		rc.Top -= 2;
		rc.Left -= 2;
		rc.Right += 2;
		rc.Bottom += 2;
	}
	else if( m_LineStyle == 6 ){
		rc.Top--;
		rc.Left--;
	}
	else if( m_LineStyle < 0 ){
		rc.Top++;
		rc.Left++;
	}
	tp->Brush->Color = col;
	tp->Pen->Color = col;
	switch(m_Shape){
		case 0:
			tp->FillRect(rc);
			break;
		case 1:
			tp->RoundRect(rc.Left, rc.Top, rc.Right, rc.Bottom, pBitmap->Width/3, pBitmap->Height/3);
			break;
		case 2:
			tp->RoundRect(rc.Left, rc.Top, rc.Right, rc.Bottom, pBitmap->Width/2, pBitmap->Height/2);
			break;
		case 3:
			tp->RoundRect(rc.Left, rc.Top, rc.Right, rc.Bottom, pBitmap->Width*3/4, pBitmap->Height*3/4);
			break;
		case 4:
			tp->Ellipse(rc.Left, rc.Top, rc.Right, rc.Bottom);
			break;
		case 5:
			{
				CPolygon Pol;
				Pol.Copy(&m_DrawPolygon);
				Pol.Stretch(rc.Right-rc.Left, rc.Bottom-rc.Top, m_X2-m_X1, m_Y2-m_Y1);
				Pol.DrawOffset(tp, rc.Left+1, rc.Top+1);
			}
			break;
	}
}

void __fastcall CDrawPic::UpdateHistPic(Graphics::TBitmap *pBmp)
{
	if( pBitmap == NULL ) return;
	if( m_Type ) return;

	TRect rc;
	rc.Left = 0; rc.Top = 0;
	rc.Right = pBitmap->Width;
	rc.Bottom = pBitmap->Height;
	int Sop = ::SetStretchBltMode(pBitmap->Canvas->Handle, HALFTONE);
	if( m_Adjust && (m_Shape != 5) ){
		int x;
		int y = pBmp->Height - 1;
		TColor col = pBmp->Canvas->Pixels[0][y];
		for( ; y >= 0; y-- ){
			if( pBmp->Canvas->Pixels[0][y] == col ){
				for( x = 1; x < pBmp->Width; x++ ){
					if( pBmp->Canvas->Pixels[x][y] != col ) break;
				}
				if( x != pBmp->Width ) break;
			}
			else {
				break;
			}
		}
		if( y == (pBmp->Height - 1) ){
			pBitmap->Canvas->StretchDraw(rc, (TGraphic*)pBmp);
		}
		else if( y ){
			TRect sc;
			sc.Left = 0; sc.Top = 0;
			sc.Right = pBmp->Width;
			sc.Bottom = y + 1;
			pBitmap->Canvas->CopyRect(rc, pBmp->Canvas, sc);
		}
		else {
			FillBitmap(pBitmap, col);
		}
	}
	else {
		pBitmap->Canvas->StretchDraw(rc, (TGraphic*)pBmp);
	}
	::SetStretchBltMode(pBitmap->Canvas->Handle, Sop);
}

void __fastcall CDrawPic::SaveToStream(TStream *sp)
{
	m_Ver = 5;
	CDrawBox::SaveToStream(sp);

	sp->Write(&m_Type, sizeof(m_Type));
	sp->Write(&m_Shape, sizeof(m_Shape));
	sp->Write(&m_Adjust, sizeof(m_Adjust));
	sp->Write(&m_TransPoint, sizeof(m_TransPoint));
	if( m_Type ){
		SaveBitmap(sp, pBitmap);
	}
	if( m_Shape == 5 ){
		m_Polygon.SaveToStream(sp);
	}
}

int __fastcall CDrawPic::LoadFromStream(TCanvas *tp, TStream *sp)
{
	if( CDrawBox::LoadFromStream(tp, sp) == FALSE ) return FALSE;

	try {
		if( pLoadBitmap != NULL ){
			delete pLoadBitmap;
			pLoadBitmap = NULL;
		}
		if( pBitmap != NULL ){
			delete pBitmap;
			pBitmap = NULL;
		}
		CreateBitmap();
		if( m_Ver >= 1 ){
			sp->Read(&m_Type, sizeof(m_Type));
			if( m_Ver >= 2 ){
				sp->Read(&m_Shape, sizeof(m_Shape));
			}
			if( m_Ver >= 4 ){
				sp->Read(&m_Adjust, sizeof(m_Adjust));
			}
			if( m_Ver >= 5 ){
				sp->Read(&m_TransPoint, sizeof(m_TransPoint));
			}
			if( m_Type ){
				LoadBitmap(sp, pBitmap);
				if( (m_Type == 2) && (sys.m_BitPixel == 16) ){
					Graphics::TBitmap *pbmp = DupeBitmap(pBitmap, -1);
					delete pBitmap;
					pBitmap = pbmp;
				}
			}
			if( m_Shape == 5 ){
				m_Polygon.LoadFromStream(sp);
				m_DrawPolygon.Copy(&m_Polygon);
				m_DrawPolygon.XW = pBitmap->Width;
				m_DrawPolygon.YW = pBitmap->Height;
				m_DrawPolygon.Stretch(m_Polygon);
			}
		}
		return TRUE;
	}
	catch(...){
		return FALSE;
	}
}

void __fastcall CDrawPic::Copy(CDraw *dp)
{
	CDrawBox::Copy(dp);
	if( pBitmap != NULL ){
		delete pBitmap;
		pBitmap = NULL;
	}
	if( pLoadBitmap != NULL ){
		delete pLoadBitmap;
		pLoadBitmap = NULL;
	}
	CreateBitmap();
	CDrawPic *sp = (CDrawPic *)dp;
	m_Type = sp->m_Type;
	m_Shape = sp->m_Shape;
	m_Adjust = sp->m_Adjust;
	m_TransPoint = sp->m_TransPoint;
	if( m_Type && (sp->pBitmap != NULL) ){
		pBitmap->Assign(sp->pBitmap);
	}
	m_Polygon.Copy(&sp->m_Polygon);
	m_DrawPolygon.Copy(&m_Polygon);
	m_DrawPolygon.XW = pBitmap->Width;
	m_DrawPolygon.YW = pBitmap->Height;
	m_DrawPolygon.Stretch(m_Polygon);
}

int __fastcall CDrawPic::Edit(void)
{
	TPicSelDlg *pBox = new TPicSelDlg(Mmsstv);
	int r = pBox->Execute(this);
	if( r == TRUE ){
		if( m_Shape != 5 ){
			sys.m_PicShape = m_Shape;
			sys.m_PicAdjust = m_Adjust;
			sys.m_PicLineStyle = m_LineStyle;
			sys.m_PicLineColor = m_LineColor;
		}
	}
	delete pBox;
	return r;
}
///
///
//---------------------------------------------------------------------------
__fastcall CDrawOle::CDrawOle()
{
	m_Command = CM_OLE;
	m_Ver = 1;
	m_Trans = 1;
	m_Stretch = 0;
	pContainer = NULL;
	m_LineStyle = TPenStyle(-1);
	m_LineColor = clWhite;
	m_X1 = 0; m_Y1 = 0;
	m_X2 = m_OrgXW = 160; m_Y2 = m_OrgYW = 128;
	pOleView = NULL;
}
//---------------------------------------------------------------------------
__fastcall CDrawOle::~CDrawOle()
{
	if( pContainer != NULL ){
		delete pContainer;
		pContainer = NULL;
	}
	if( pOleView != NULL ){
		delete pOleView;
	}
}
//---------------------------------------------------------------------------
int __fastcall CDrawOle::Finish(int x, int y)
{
	m_X2 = x;
	m_Y2 = y;
	Normal();
	if( !((m_X1 != m_X2) && (m_Y1 != m_Y2)) ) return FALSE;
	return Edit();
}

//---------------------------------------------------------------------------
Graphics::TBitmap* __fastcall CDrawOle::MakeBitmap(void)
{
	if( pContainer == NULL ) return NULL;

	Graphics::TBitmap *pBitmap = new Graphics::TBitmap();
	AdjustBitmapFormat(pBitmap);
	if( m_Stretch ){
		pContainer->SizeMode = smAutoSize;
		pBitmap->Width = pContainer->ClientWidth;
		pBitmap->Height = pContainer->ClientHeight;
	}
	else {
		pBitmap->Width = m_X2 - m_X1;
		pBitmap->Height = m_Y2 - m_Y1;
		pContainer->ClientWidth = ABS(m_X2 - m_X1);
		pContainer->ClientHeight = ABS(m_Y2 - m_Y1);
	}
	pBitmap->Transparent = FALSE;
	pContainer->PaintTo(pBitmap->Canvas->Handle, 0, 0);
	if( m_Stretch ){
		pContainer->SizeMode = smStretch;
		Graphics::TBitmap *pBmp = CreateBitmap(m_X2 - m_X1, m_Y2 - m_Y1, -1);
		TRect rc;
		rc.Left = 0; rc.Right = pBmp->Width;
		rc.Top = 0; rc.Bottom = pBmp->Height;
		::SetStretchBltMode(pBmp->Canvas->Handle, m_Trans ? COLORONCOLOR : HALFTONE);
		pBmp->Canvas->StretchDraw(rc, pBitmap);
		delete pBitmap;
		pBitmap = pBmp;
	}
	return pBitmap;
}
//---------------------------------------------------------------------------
void __fastcall CDrawOle::Draw(TCanvas *tp)
{
	if( pContainer == NULL ) return;

	Graphics::TBitmap *pBitmap = MakeBitmap();
	if( m_Trans ){
		pBitmap->Transparent = TRUE;
		pBitmap->TransparentMode = tmFixed;
		pBitmap->TransparentColor = pBitmap->Canvas->Pixels[0][0];
	}
	tp->Draw(m_X1, m_Y1, pBitmap);
	delete pBitmap;
	CDrawBox::Draw(tp);
}
//---------------------------------------------------------------------------
void __fastcall CDrawOle::SaveToStream(TStream *sp)
{
	m_Ver = 1;
	CDrawBox::SaveToStream(sp);

	sp->Write(&m_Trans, sizeof(m_Trans));
	sp->Write(&m_Stretch, sizeof(m_Stretch));
	pContainer->SaveToStream(sp);
}
//---------------------------------------------------------------------------
int __fastcall CDrawOle::LoadFromStream(TCanvas *tp, TStream *sp)
{
	if( CDrawBox::LoadFromStream(tp, sp) == FALSE ) return FALSE;

	sp->Read(&m_Trans, sizeof(m_Trans));
	sp->Read(&m_Stretch, sizeof(m_Stretch));
	try {
		MakeItem();
		pContainer->LoadFromStream(sp);
		if( pContainer->Linked ) pContainer->UpdateObject();
		GetOrgSize();
		return TRUE;
	}
	catch(...){
		return FALSE;
	}
}
//---------------------------------------------------------------------------
void __fastcall CDrawOle::Copy(CDraw *dp)
{
	CDrawOle *sp = (CDrawOle *)dp;
	if( sp->pContainer == NULL ) return;

	CDrawBox::Copy(dp);
	m_Trans = sp->m_Trans;
	m_Stretch = sp->m_Stretch;
	m_OrgXW = sp->m_OrgXW;
	m_OrgYW = sp->m_OrgYW;
	TMemoryStream *tm = new TMemoryStream;
	tm->Seek(0, soFromBeginning);
	sp->pContainer->SaveToStream(tm);
	MakeItem();
	tm->Seek(0, soFromBeginning);
	pContainer->LoadFromStream(tm);
	delete tm;
}
//---------------------------------------------------------------------------
void __fastcall CDrawOle::GetOrgSize(void)
{
	pContainer->SizeMode = smAutoSize;
	m_OrgXW = pContainer->ClientWidth;
	m_OrgYW = pContainer->ClientHeight;
	pContainer->SizeMode = smStretch;
	if( m_OrgXW <= 0 ) m_OrgXW = 16;
	if( m_OrgYW <= 0 ) m_OrgYW = 16;
}
//---------------------------------------------------------------------------
void __fastcall CDrawOle::SetInitSize(void)
{
	int lx = 256;
	int ly = 200;
	if( lx > m_OrgXW ) lx = m_OrgXW;
	if( ly > m_OrgYW ) ly = m_OrgYW;
	::KeepAspect(lx, ly, m_OrgXW, m_OrgYW);
	if( m_X1 < 0 ){
		m_X1 = (Mmsstv->DrawMain.m_SX - lx) / 2;
		m_Y1 = (Mmsstv->DrawMain.m_SY - ly) / 2;
	}
	m_X2 = m_X1 + lx;
	m_Y2 = m_Y1 + ly;
}
//---------------------------------------------------------------------------
void __fastcall CDrawOle::MakeItem(void)
{
	if( pContainer != NULL ){
		delete pContainer;
		pContainer = NULL;
	}
	pContainer = new TOleContainer(Mmsstv);
	pContainer->Visible = FALSE;
	pContainer->Caption = "MMSSTV";
	pContainer->Parent = Mmsstv->PanelTemp;
	pContainer->TabStop = FALSE;
	pContainer->Left = 0;
	pContainer->Top = 0;
	pContainer->ClientWidth = 256;
	pContainer->ClientHeight = 200;
	pContainer->BorderStyle = bsNone;
	pContainer->SizeMode = smStretch;
	pContainer->AllowInPlace = FALSE;
	pContainer->AutoActivate = aaManual;
	pContainer->AllowActiveDoc = FALSE;
	pContainer->AutoVerbMenu = FALSE;
	pContainer->OldStreamFormat = FALSE;
}
//---------------------------------------------------------------------------
void __fastcall CDrawOle::SyncVerb(void)
{
	while(pContainer->State == osOpen){
		Application->ProcessMessages();
		if( !Mmsstv->Timer->Enabled ) return;
		MultProc();
		::Sleep(1);
	}
}
//---------------------------------------------------------------------------
void __fastcall CDrawOle::ViewVerb(int sw)
{
	if( sw ){
		if( pOleView == NULL ){
			pOleView = new TForm(Mmsstv);
			pOleView->AutoScroll = FALSE;
			pOleView->Scaled = FALSE;
		}
		pOleView->SetBounds(-m_OrgXW, -m_OrgYW, m_OrgXW, m_OrgYW);
		pContainer->Parent = pOleView;
		pContainer->Visible = TRUE;
		pOleView->Visible = TRUE;
	}
	else if( pOleView != NULL ){
		pOleView->Visible = FALSE;
		pContainer->Visible = FALSE;
		pContainer->Parent = Mmsstv->PanelTemp;
		delete pOleView;
		pOleView = NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall CDrawOle::MakeError(void)
{
	delete pContainer;
	pContainer = NULL;
	InfoMB( "Can not handle the object." );
}
//---------------------------------------------------------------------------
int __fastcall CDrawOle::Edit(void)
{
	SetDisPaint();
	int First = FALSE;
	CWaitCursor w;
	if( pContainer == NULL ){
		First = TRUE;
		MakeItem();
		MultProc();
		if( pContainer->InsertObjectDialog() != TRUE ){
			MultProc();
			ResDisPaint();
			return FALSE;
		}
	}
	MultProc();
	try {
		if( !First || !pContainer->Linked ){
			pContainer->AllowInPlace = FALSE;
			ViewVerb(1);
			pContainer->DoVerb(ovShow);
			SyncVerb();
			ViewVerb(0);
		}
		GetOrgSize();
		if( First ){
			SetInitSize();
		}
	}
	catch(...){
		ViewVerb(0);
		if( First ){
			MakeError();
		}
	}
	ResDisPaint();
	return (pContainer != NULL) ? TRUE : FALSE;
}
//---------------------------------------------------------------------------
int __fastcall CDrawOle::LoadFromFile(int x, int y, LPCSTR pName)
{
	int r = TRUE;
	CWaitCursor w;

	MakeItem();
	m_X1 = x;
	m_Y1 = y;
	try {
		MultProc();
		pContainer->CreateObjectFromFile(pName, FALSE);
		MultProc();
		GetOrgSize();
		SetInitSize();
	}
	catch(...){
		MakeError();
		r = FALSE;
	}
	return r;
}
//---------------------------------------------------------------------------
void __fastcall CDrawOle::LoadFromBitmap(int x, int y, Graphics::TBitmap *pBmp)
{
	CWaitCursor w;

	char name[256];
	sprintf(name, "%sTEMP.bmp", BgnDir);
	if( SaveBitmap24(pBmp, name) == TRUE ){
		LoadFromFile(x, y, name);
	}
	unlink(name);
}
//---------------------------------------------------------------------------
int __fastcall CDrawOle::Paste(void)
{
	int r = TRUE;
	MakeItem();
	try {
		pContainer->Paste();
		GetOrgSize();
		SetInitSize();
	}
	catch(...){
		MakeError();
		r = FALSE;
	}
	return r;
}
//---------------------------------------------------------------------------
void __fastcall CDrawOle::DoPrimary(void)
{
	if( pContainer != NULL ){
		SetDisPaint();
		try {
			ViewVerb(1);
			pContainer->DoVerb(ovPrimary);
			SyncVerb();
			ViewVerb(0);
			GetOrgSize();
		}
		catch(...){
			ViewVerb(0);
		}
		ResDisPaint();
	}
}
//---------------------------------------------------------------------------
int __fastcall CDrawOle::ObjectProperties(void)
{
	SetDisPaint();
	int r = FALSE;
	try {
		ViewVerb(1);
		r = pContainer->ObjectPropertiesDialog();
		ViewVerb(0);
	}
	catch(...){
		ViewVerb(0);
	}
	ResDisPaint();
	return r;
}
//---------------------------------------------------------------------------
int __fastcall CDrawOle::IsOrgSize(void)
{
	if( (m_X2 - m_X1) != m_OrgXW ) return FALSE;
	if( (m_Y2 - m_Y1) != m_OrgYW ) return FALSE;
	return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall CDrawOle::SetOrgSize(void)
{
	m_X2 = m_X1 + m_OrgXW;
	m_Y2 = m_Y1 + m_OrgYW;
}
//---------------------------------------------------------------------------
void __fastcall CDrawOle::KeepAspect(void)
{
	int lx = m_X2 - m_X1;
	int ly = m_Y2 - m_Y1;
	::KeepAspect(lx, ly, m_OrgXW, m_OrgYW);
	m_X2 = m_X1 + lx + 1;
	m_Y2 = m_Y1 + ly + 1;
}
//***************************************************************************
//DrawLibクラス
//---------------------------------------------------------------------------
__fastcall CDrawLib::CDrawLib()
{
	m_Command = CM_LIB;
	m_Ver = 1;
	m_Type = 0;
	m_Name = "";
	hLib = NULL;
	hObj = NULL;
	pBitmap = NULL;
	m_LineStyle = TPenStyle(-1);
	m_LineColor = clWhite;
	m_X1 = 0; m_Y1 = 0;
	m_X2 = m_OrgXW = 160; m_Y2 = m_OrgYW = 128;
	m_Text = "";
	Delete();
}
//---------------------------------------------------------------------------
__fastcall CDrawLib::~CDrawLib()
{
	Delete();
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::Delete(void)
{
	if( hObj != NULL ){
		if( hLib && fDeleteObject ) fDeleteObject(hObj);
		hObj = NULL;
	}
	hLib = NULL;
	if( pBitmap != NULL ){
		delete pBitmap;
		pBitmap = NULL;
	}

	fFunc=NULL;
	fLanguage=NULL;
	fAboutDialog = NULL;
	fGetItemType = NULL;
	fCreateObject = NULL;
	fCreateStorage = NULL;
	fDeleteStorage = NULL;
	fDeleteObject = NULL;
	fEdit = NULL;
	fFont = NULL;
	fGetOrgSize = NULL;
	fPos = NULL;
	fGetUserText = NULL;
	fUpdateText = NULL;
	fUpdateImage = NULL;
	fSetDraft = NULL;
	fSetEx = NULL;
    fGetFont = NULL;
    fSetFont = NULL;
	m_Type = 0;
}
//---------------------------------------------------------------------------
FARPROC __fastcall CDrawLib::GetProc(LPCSTR pName)
{
	FARPROC fp = ::GetProcAddress(hLib, pName);
	if( fp != NULL ) return fp;
	fp = ::GetProcAddress(hLib, pName+1);
	return fp;
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::LoadLib(int emsg)
{
	hLib = NULL;
	if( m_Name.IsEmpty() ) return;
	LPCSTR pName = m_Name.c_str();
	hLib = (HINSTANCE)Libl.LoadLibrary(pName);
	if( hLib != NULL ){
		fGetItemType = (mcmGetItemType)GetProc("_mcmGetItemType");
		fCreateObject = (mcmCreateObject)GetProc("_mcmCreateObject");
		fDeleteObject = (mcmDeleteObject)GetProc("_mcmDeleteObject");
		if( !fGetItemType || !fCreateObject || !fDeleteObject ){
			Libl.DeleteLibrary(hLib);
			hLib = NULL;
			if( emsg ) InfoMB( "Can not invoke %s as MMSSTV's custom item.", pName);
			return;
		}
		fFunc = (mcmFunc)GetProc("_mcmFunc");
		fLanguage = (mcmLanguage)GetProc("_mcmLanguage");
		fAboutDialog = (mcmAboutDialog)GetProc("_mcmAboutDialog");
		fCreateStorage = (mcmCreateStorage)GetProc("_mcmCreateStorage");
		fDeleteStorage = (mcmDeleteStorage)GetProc("_mcmDeleteStorage");
		fEdit = (mcmEdit)GetProc("_mcmEdit");
		fFont = (mcmEdit)GetProc("_mcmFont");
		fGetOrgSize = (mcmGetOrgSize)GetProc("_mcmGetOrgSize");
		fPos = (mcmPos)GetProc("_mcmPos");
		fGetUserText = (mcmGetUserText)GetProc("_mcmGetUserText");
		fUpdateText = (mcmUpdateText)GetProc("_mcmUpdateText");
		fUpdateImage = (mcmUpdateImage)GetProc("_mcmUpdateImage");
		fSetDraft = (mcmSetDraft)GetProc("_mcmSetDraft");
		fSetEx = (mcmSetEx)GetProc("_mcmSetEx");
		fGetFont = (mcmGetFont)GetProc("_mcmGetFont");
		fSetFont = (mcmSetFont)GetProc("_mcmSetFont");
		if( fFunc ){
			fFunc(0, (FARPROC)mmLoadImageMenu);
			fFunc(1, (FARPROC)mmLoadImage);
			fFunc(2, (FARPROC)mmLoadHistImage);
			fFunc(3, (FARPROC)mmSaveImageMenu);
			fFunc(4, (FARPROC)mmSaveImage);
			fFunc(5, (FARPROC)mmPasteImage);
			fFunc(6, (FARPROC)mmGetMacKey);
			fFunc(7, (FARPROC)mmConvMac);
			fFunc(8, (FARPROC)mmCreateDIB);
			fFunc(9, (FARPROC)MultProcA);
			fFunc(10, (FARPROC)MultProc);
			fFunc(11, (FARPROC)mmColorComp);
			fFunc(12, (FARPROC)mmLoadRxImage);
			fFunc(13, (FARPROC)mmView);
            fFunc(14, (FARPROC)mmAdjCol);
            fFunc(15, (FARPROC)mmClip);
		}
		if( fLanguage ) fLanguage(MsgEng);
	}
	else if( emsg ){
		InfoMB( "Can not load '%s'", pName);
	}
}
//---------------------------------------------------------------------------
int __fastcall CDrawLib::MakeR(LPCSTR pName)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char name[_MAX_FNAME];
	char ext[_MAX_EXT];
	::_splitpath( pName, drive, dir, name, ext );
	m_Name = drive;
	m_Name += dir;
	if( !strcmpi(m_Name.c_str(), BgnDir) ){
		m_Name = name;
		m_Name += ext;
	}
	else {
		m_Name = pName;
	}
	if( m_Name.IsEmpty() ) return FALSE;
	LoadLib(TRUE);
	if( hLib == NULL ) return FALSE;
	hObj = fCreateObject(NULL, 0);
	if( hObj == NULL ){
		InfoMB( "Fail to create the object from '%s'", m_Name.c_str());
		return FALSE;
	}
	Pos();
	m_Type = fGetItemType(hObj);
	if( m_Type & 3 ) Update();
	return TRUE;
}
//---------------------------------------------------------------------------
int __fastcall CDrawLib::Finish(int x, int y)
{
	m_X2 = x;
	m_Y2 = y;
	Normal();
	if( !((m_X1 != m_X2) && (m_Y1 != m_Y2)) ) return FALSE;
	TOpenDialog *pDialog = Mmsstv->OpenDialog;
	pDialog->Options >> ofCreatePrompt;
	pDialog->Options << ofFileMustExist;
	pDialog->Title = Mmsstv->KTCI->Caption;
	pDialog->Filter = GetLibFilter();
	pDialog->FileName = "";
	pDialog->DefaultExt = "dll";
	pDialog->InitialDir = BgnDir;
	SetDisPaint();
	if( Mmsstv->OpenDialogExecute(FALSE) == TRUE ){
		ResDisPaint();
		if( MakeR(AnsiString(pDialog->FileName).c_str()) ){	//ja7ude 0521
			return Edit();
		}
		else {
			return FALSE;
		}
	}
	ResDisPaint();
	return FALSE;
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::Move(int x, int y)
{
	CDrawBox::Move(x, y);
	if( (m_X1 == m_X2) || (m_Y1 == m_Y2) ){
		return;
	}
	if( (m_MC != HT_I) || (m_Type & 0x00000010) ){
		Update();
	}
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::SwapHandle(HBITMAP hb)
{
	if( hb ){
		pBitmap->ReleaseHandle();
		pBitmap->Handle = hb;
		int x2 = m_X2;
		int y2 = m_Y2;
		m_X2 = m_X1 + pBitmap->Width;
		m_Y2 = m_Y1 + pBitmap->Height;
		Pos();
		m_Type = fGetItemType(hObj);
		if( m_Type & 0x00400000 ){  // X中央
			int xw = (m_X2 - x2)/2;
			m_X1 -= xw; m_X2 -= xw;
		}
		else if( m_Type & 0x00100000 ){  // 右詰
			int xw = m_X2 - x2;
			m_X1 -= xw; m_X2 -= xw;
		}
		if( m_Type & 0x00800000 ){  // Y中央
			int yw = (m_Y2 - y2)/2;
			m_Y1 -= yw; m_Y2 -= yw;
		}
		else if( m_Type & 0x00200000 ){  // 下詰
			int yw = m_Y2 - y2;
			m_Y1 -= yw; m_Y2 -= yw;
		}
		AdjustRect();
		Pos();
	}
	if( fGetOrgSize ){
		ULONG size = fGetOrgSize(hObj);
		m_OrgXW = size & 0x0000ffff;
		m_OrgYW = size >> 16;
	}
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::Pos(void)
{
	if( !fPos ) return;

	DWORD   pos = m_X1 + (m_Y1 << 16);
	DWORD   size = (m_X2 - m_X1) + ((m_Y2 - m_Y1) << 16);
	DWORD   tsize = SXW + (SYW << 16);
	fPos(hObj, pos, size, tsize);
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::Update(void)
{
	if( !hLib ) return;
	if( !hObj ) return;

	switch(m_Type & 3){
		case 0:
			if( !fUpdateImage ) return;

			SetBitmapSize();
			Pos();
			SwapHandle(fUpdateImage(hObj, pBitmap->Handle, NULL));
			break;
		case 1:
			UpdatePic(Mmsstv->pBitmapHistF);
			break;
		default:
			UpdateText();
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::SetBitmapSize(void)
{
	if( pBitmap == NULL ){
		pBitmap = new Graphics::TBitmap;
	}
	pBitmap->PixelFormat = pf24bit;
	pBitmap->Width = m_X2 - m_X1;
	pBitmap->Height = m_Y2 - m_Y1;
	FillBitmap(pBitmap, TransCol);
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::DrawFast(TCanvas *tp)
{
	if( hLib && hObj ){
		if( fSetDraft ) fSetDraft(hObj, 1);
		Draw(tp);
		if( fSetDraft ) fSetDraft(hObj, 0);
	}
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::Draw(TCanvas *tp)
{
	if( !hLib || !hObj  ){
		SetBitmapSize();
		pBitmap->Canvas->Font->Color = clRed;
		pBitmap->Canvas->TextOut(1, 1, m_Name);
		m_Type = 0;
	}
	if( pBitmap == NULL ){
		Update();
		if( pBitmap == NULL ) return;
	}

	ASSERT(pBitmap);

	Graphics::TBitmap *pbmp = pBitmap;
	if( sys.m_BitPixel == 16 ){
		pbmp = DupeBitmap(pBitmap, -1);
	}

	if( m_Type & 0x00010000 ){
		pbmp->Transparent = TRUE;
		pbmp->TransparentMode = tmFixed;
		pbmp->TransparentColor = pbmp->Canvas->Pixels[0][0];
	}

	tp->Draw(m_X1, m_Y1, pbmp);
	pbmp->Transparent = FALSE;
	if( pbmp != pBitmap ) delete pbmp;
	CDrawBox::Draw(tp);
}
//---------------------------------------------------------------------------
int __fastcall CDrawLib::Edit(int sw)
{
	int r;
	if( hLib ){
		if( !hObj ) return FALSE;

		if( fLanguage ) fLanguage(MsgEng);
		SetDisPaint();
		MultProc();
		if( sw ){
			r = fFont(hObj, Screen->ActiveForm->Handle);
		}
		else {
			r = fEdit(hObj, Screen->ActiveForm->Handle);
		}
		MultProc();
		ResDisPaint();
		if( r ){
			Pos();
			m_Type = fGetItemType(hObj);
			Update();
		}
	}
	else {
		r = Finish(m_X2, m_Y2);
	}
	return r;
}
//---------------------------------------------------------------------------
int __fastcall CDrawLib::Edit(void)
{
	if( !fEdit ) return FALSE;
	return Edit(0);
}
//---------------------------------------------------------------------------
int __fastcall CDrawLib::Font(void)
{
	if( !fFont ) return Edit();
	return Edit(1);
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::ObjectProperties(void)
{
	if( hLib ){
		if( !fAboutDialog ) return;

		if( fLanguage ) fLanguage(MsgEng);
		fAboutDialog(Screen->ActiveForm->Handle);
	}
	else {
		InfoMB( "Could not load '%s', the item is inactive.", m_Name.c_str());
	}
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::SaveToStream(TStream *sp)
{
	m_Ver = 1;
	CDrawBox::SaveToStream(sp);

	SaveString(sp, m_Name);
	ULONG size = 0;
	const BYTE *p = NULL;
	if( fCreateStorage ) p = fCreateStorage(hObj, &size);
	if( p == NULL ) size = 0;
	sp->Write(&size, sizeof(size));
	if( size ) sp->Write(p, size);
	if( fDeleteStorage ) fDeleteStorage(hObj, p);
}
//---------------------------------------------------------------------------
int __fastcall CDrawLib::LoadFromStream(TCanvas *tp, TStream *sp)
{
	Delete();
	if( CDrawBox::LoadFromStream(tp, sp) == FALSE ) return FALSE;

	LoadString(sp, m_Name);
	ULONG size = 0;
	BYTE *p = NULL;
	sp->Read(&size, sizeof(size));
	if( size ){
		p = new BYTE[size];
		sp->Read(p, size);
	}
	LoadLib(FALSE);
	if( hLib && fCreateObject ){
		hObj = fCreateObject(p, size);
		if( hObj != NULL ){
			Pos();
			m_Type = fGetItemType(hObj);
		}
	}
	if( p != NULL ) delete p;
	return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::Copy(CDraw *dp)
{
	CDrawLib *sp = (CDrawLib *)dp;

	TMemoryStream *tm = new TMemoryStream;
	tm->Seek(0, soFromBeginning);
	sp->SaveToStream(tm);
	tm->Seek(0, soFromBeginning);
	int cmd;
	tm->Read(&cmd, sizeof(cmd));
	if( cmd == CM_LIB ){
		LoadFromStream(Mmsstv->pBitmapTemp->Canvas, tm);
	}
	delete tm;
}
//---------------------------------------------------------------------------
int __fastcall CDrawLib::IsOrgSize(void)
{
	if( !hLib ) return TRUE;

	if( m_OrgXW != (m_X2 - m_X1) ) return FALSE;
	if( m_OrgYW != (m_Y2 - m_Y1) ) return FALSE;
	return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::SetOrgSize(void)
{
	if( !hLib ) return;

	m_X2 = m_X1 + m_OrgXW;
	m_Y2 = m_Y1 + m_OrgYW;
	Update();
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::KeepAspect(void)
{
	if( !hLib ) return;

	int lx = m_X2 - m_X1;
	int ly = m_Y2 - m_Y1;
	::KeepAspect(lx, ly, m_OrgXW, m_OrgYW);
	m_X2 = m_X1 + lx + 1;
	m_Y2 = m_Y1 + ly + 1;
	Update();
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::UpdatePic(Graphics::TBitmap *pBmp)
{
	if( !(m_Type & 1) ) return;
	if( !hLib ) return;
	if( !hObj ) return;
	if( !fUpdateImage ) return;

	SetBitmapSize();
	Pos();
	Graphics::TBitmap *ps = (pBmp->PixelFormat != pf24bit) ? DupeBitmap(pBmp, pf24bit) : pBmp;
	SwapHandle(fUpdateImage(hObj, pBitmap->Handle, ps->Handle));
	if( ps != pBmp ) delete ps;
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::UpdateText(int f)
{
	if( !(m_Type & 2) ) return;
	if( !hLib ) return;
	if( !hObj ) return;
	if( !fGetUserText ) return;
	if( !fUpdateText ) return;

	if( fSetEx ) fSetEx(hObj, sys.m_DisFontSmooth);

	char bf[1024];
	LPCSTR pText = fGetUserText(hObj);
	if( Mmsstv->MacroText(bf, pText, sizeof(bf)-1) || f ){
		SetBitmapSize();
		Pos();
		SwapHandle(fUpdateText(hObj, pBitmap->Handle, bf));
	}
	m_Text = pText;
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::UpdateText(void)
{
	UpdateText(1);
}
//---------------------------------------------------------------------------
void __fastcall CDrawLib::UpdateTimeText(void)
{
	UpdateText(0);
}
//---------------------------------------------------------------------------
int __fastcall CDrawLib::IsTimeMacro(void)
{
	char bf[1024];
	return Mmsstv->MacroText(bf, m_Text.c_str(), sizeof(bf)-1);
}
//---------------------------------------------------------------------------
int __fastcall CDrawLib::GetFont(LOGFONT *pLogfont)
{
	if( !hLib || !hObj || !fGetFont ) return FALSE;
	fGetFont(hObj, pLogfont);
	return TRUE;
}
//---------------------------------------------------------------------------
int __fastcall CDrawLib::SetFont(LOGFONT *pLogfont)
{
	if( !hLib || !hObj || !fGetFont ) return FALSE;
	fSetFont(hObj, pLogfont);
	Update();
	return TRUE;
}
//***************************************************************************
//DrawGroupクラス
//---------------------------------------------------------------------------
__fastcall CDrawGroup::CDrawGroup()
{
	m_Command = CM_GROUP;
	m_Cnt = 0;
	m_Max = 0;
	m_TransX = 319;
	m_TransY = 255;
	m_TransCol = TColor(0x00f8f8f8);
	m_SX = 320;
	m_SY = 256;
	m_Ver = 2;
	pBase = NULL;
	pSel = NULL;
}

__fastcall CDrawGroup::~CDrawGroup()
{
	Free();
}

void __fastcall CDrawGroup::Free(void)
{
	if( pBase != NULL ){
		delete pBase;
		pBase = NULL;
	}
	m_Cnt = m_Max = 0;
}

void __fastcall CDrawGroup::FreeItem(void)
{
	if( pBase == NULL ) return;

	for( int i = 0; i < m_Cnt; i++ ){
		if( pBase[i]->m_Command == CM_GROUP ){
			((CDrawGroup *)(pBase[i]))->FreeItem();
		}
		delete pBase[i];
	}
	delete pBase;
	pBase = NULL;
	m_Cnt = m_Max = 0;
	pSel = NULL;
	m_TransX = 319;
	m_TransY = 255;
	m_TransCol = TColor(0x00f8f8f8);
}

void __fastcall CDrawGroup::ReleaseResource(void)
{
	if( pBase == NULL ) return;

	for( int i = 0; i < m_Cnt; i++ ){
		pBase[i]->ReleaseResource();
	}
}

void __fastcall CDrawGroup::UpdatePic(Graphics::TBitmap *pBmp)
{
	if( pBase == NULL ) return;

	TransCol = m_TransCol;
	for( int i = 0; i < m_Cnt; i++ ){
		pBase[i]->UpdatePic(pBmp);
	}
}

void __fastcall CDrawGroup::AddItem(CDraw *dp)
{
	if( m_Cnt >= m_Max ){
		m_Max = m_Max ? m_Max * 2 : 16;
		CDraw **np = new CDraw*[m_Max];
		if( pBase != NULL ){
			memcpy(np, pBase, sizeof(CDraw*)*m_Cnt);
			delete pBase;
		}
		pBase = np;
	}
	pBase[m_Cnt] = dp;
	m_Cnt++;
}

void __fastcall CDrawGroup::DeleteItem(CDraw *dp)
{
	if( pBase == NULL ) return;

	for( int i = 0; i < m_Cnt; i++ ){
		if( pBase[i] == dp ){
			delete pBase[i];
			for( ; i < (m_Cnt - 1); i++ ){
				pBase[i] = pBase[i+1];
			}
			pBase[i] = NULL;
			m_Cnt--;
			return;
		}
	}
}

void __fastcall CDrawGroup::ReleaseItem(CDraw *dp)
{
	if( pBase == NULL ) return;

	for( int i = 0; i < m_Cnt; i++ ){
		if( pBase[i] == dp ){
			for( ; i < (m_Cnt - 1); i++ ){
				pBase[i] = pBase[i+1];
			}
			pBase[i] = NULL;
			m_Cnt--;
			return;
		}
	}
}

CDraw * __fastcall CDrawGroup::AddItemCopy(CDraw *dp)
{
	CDraw *pItem = MakeItem(dp->m_Command);
	pItem->Copy(dp);
	AddItem(pItem);
	return pItem;
}

void __fastcall CDrawGroup::FillPic(TCanvas *tp, TColor col, int sw, double sx, double sy, int off)
{
	if( pBase == NULL ) return;

	for( int i = 0; i < m_Cnt; i++ ){
		pBase[i]->FillPic(tp, col, sw, sx, sy, off);
	}
}

void __fastcall CDrawGroup::Draw(TCanvas *tp)
{
	if( pBase == NULL ) return;

	TransCol = m_TransCol;
	SetDrawSize(m_SX, m_SY);
	for( int i = 0; i < m_Cnt; i++ ){
		tp->Brush->Color = m_TransCol;
		pBase[i]->Draw(tp);
	}
}

void __fastcall CDrawGroup::DrawFast(TCanvas *tp)
{
	if( pBase == NULL ) return;

	TransCol = m_TransCol;
	SetDrawSize(m_SX, m_SY);
	for( int i = 0; i < m_Cnt; i++ ){
		tp->Brush->Color = m_TransCol;
		pBase[i]->DrawFast(tp);
	}
}

void __fastcall CDrawGroup::DrawNotSel(TCanvas *tp)
{
	if( pBase == NULL ) return;

	TransCol = m_TransCol;
	SetDrawSize(m_SX, m_SY);
	for( int i = 0; i < m_Cnt; i++ ){
		if( (pBase[i] != pSel) ||
			(pBase[i]->m_Command == CM_TEXT) ||
			(pBase[i]->m_Command == CM_PIC) ||
			(pBase[i]->m_Command == CM_BOXS) ||
			(pBase[i]->m_Command == CM_OLE) ||
			(pBase[i]->m_Command == CM_LIB) ||
			(pBase[i]->m_Command == CM_GROUP)
		){
			tp->Brush->Color = m_TransCol;
			pBase[i]->Draw(tp);
		}
	}
}

CDraw* __fastcall CDrawGroup::SelectItem(int X, int Y)
{
	if( pBase == NULL ) return NULL;

	for( int i = m_Cnt - 1; i >= 0; i-- ){
		if( (m_Hit = pBase[i]->HitTest(X, Y)) != 0 ){
			return pBase[i];
		}
	}
	return NULL;
}

CDraw* __fastcall CDrawGroup::MakeItem(int cmd)
{
	TransCol = m_TransCol;
	CDraw *pItem = NULL;
	switch(cmd){
		case CM_LINE:
			pItem = new CDrawLine;
			break;
		case CM_BOX:
			pItem = new CDrawBox;
			break;
		case CM_BOXS:
			pItem = new CDrawBoxS;
			break;
		case CM_TEXT:
			pItem = new CDrawText;
			break;
		case CM_PIC:
			pItem = new CDrawPic;
			break;
		case CM_TITLE:
			pItem = new CDrawTitle;
			break;
		case CM_OLE:
			pItem = new CDrawOle;
			break;
		case CM_LIB:
			pItem = new CDrawLib;
			break;
		case CM_GROUP:
			pItem = new CDrawGroup;
			break;
	}
	return pItem;
}

void __fastcall CDrawGroup::SaveToStream(TStream *sp)
{
	m_Ver = 2;
	CDrawBox::SaveToStream(sp);

	sp->Write(&m_TransX, sizeof(m_TransX));
	sp->Write(&m_TransY, sizeof(m_TransY));
	sp->Write(&m_TransCol, sizeof(m_TransCol));
	sp->Write(&m_SX, sizeof(m_SX));
	sp->Write(&m_SY, sizeof(m_SY));
	sp->Write(&m_Cnt, sizeof(m_Cnt));
	for( int i = 0; i < m_Cnt; i++ ){
		pBase[i]->SaveToStream(sp);
	}
}

int __fastcall CDrawGroup::LoadFromStream(TCanvas *tp, TStream *sp)
{
	FreeItem();
	MultProc();
	try {
		int cnt, cmd;
		sp->Read(&cmd, sizeof(cmd));
		if( cmd != CM_GROUP ){
			goto _err;
		}

		if( CDrawBox::LoadFromStream(tp, sp) == FALSE ) goto _err;
		if( m_Ver >= 1 ){
			sp->Read(&m_TransX, sizeof(m_TransX));
			sp->Read(&m_TransY, sizeof(m_TransY));
			sp->Read(&m_TransCol, sizeof(m_TransCol));
		}
		if( m_Ver >= 2 ){
			sp->Read(&m_SX, sizeof(m_SX));
			sp->Read(&m_SY, sizeof(m_SY));
		}
		SetDrawSize(m_SX, m_SY);
		pSel = NULL;
		sp->Read(&cnt, sizeof(cnt));
		for( int i = 0; i < cnt; i++ ){
			MultProcA();
			sp->Read(&cmd, sizeof(cmd));
			CDraw *pItem = MakeItem(cmd);
			if( pItem != NULL ){
				if( pItem->LoadFromStream(tp, sp) == FALSE ){
					delete pItem;
					goto _err;
				}
				AddItem(pItem);
			}
			else {
				goto _err;
			}
		}
		if( !m_Cnt ) m_TransCol = TColor(0x00f8f8f8);
		m_Ver = 2;
		return TRUE;
	}
	catch(...){
	}
_err:;
	FreeItem();
	m_Ver = 1;
	return FALSE;
}

void __fastcall CDrawGroup::Copy(CDraw *dp)
{
	FreeItem();

	CDrawGroup *sp = (CDrawGroup *)dp;
	SetDrawSize(sp->m_SX, sp->m_SY);
	m_TransX = sp->m_TransX;
	m_TransY = sp->m_TransY;
	m_TransCol = sp->m_TransCol;
	m_SX = sp->m_SX;
	m_SY = sp->m_SY;
	for( int i = 0; i < sp->m_Cnt; i++ ){
		AddItemCopy(sp->pBase[i]);
	}
}

void __fastcall CDrawGroup::UpdateText(void)
{
	CWaitCursor w;

	TransCol = m_TransCol;
	SetDrawSize(m_SX, m_SY);
	for( int i = 0; i < m_Cnt; i++ ){
		pBase[i]->UpdateText();
	}
}

void __fastcall CDrawGroup::UpdateTimeText(void)
{
	TransCol = m_TransCol;
	SetDrawSize(m_SX, m_SY);
	for( int i = 0; i < m_Cnt; i++ ){
		pBase[i]->UpdateTimeText();
	}
}

int __fastcall CDrawGroup::IsMacro(LPCSTR pKey)
{
	LPCSTR pText;
	for( int i = 0; i < m_Cnt; i++ ){
		pText = pBase[i]->GetText();
		if( pText != NULL ){
			if( strstr(pText, pKey) != NULL ) return 1;
		}
#if 0
		if( pBase[i]->m_Command == CM_TEXT ){
			LPCSTR p = ((CDrawText *)pBase[i])->m_Text.c_str();
			if( strstr(p, pKey) != NULL ) return 1;
		}
#endif
	}
	return 0;
}

int __fastcall CDrawGroup::IsTimeMacro(void)
{
	for( int i = 0; i < m_Cnt; i++ ){
		if( pBase[i]->IsTimeMacro() ) return 1;
	}
	return 0;
}

void __fastcall CDrawGroup::BackItem(CDraw *pItem)
{
	int i, k;
	for( i = m_Cnt - 1; i >= 0; i-- ){
		if( pBase[i] == pItem ){
			for( k = i; k > 0; k-- ){
				pBase[k] = pBase[k-1];
			}
			pBase[0] = pItem;
			break;
		}
	}
}

void __fastcall CDrawGroup::FrontItem(CDraw *pItem)
{
	int i, k;
	for( i = 0; i < m_Cnt; i++ ){
		if( pBase[i] == pItem ){
			for( k = i; k < m_Cnt - 1; k++ ){
				pBase[k] = pBase[k+1];
			}
			pBase[k] = pItem;
			break;
		}
	}
}

int __fastcall CDrawGroup::UpItem(int n)
{
	if( n ){
		CDraw *pItem = pBase[n-1];
		pBase[n-1] = pBase[n];
		pBase[n] = pItem;
		return TRUE;
	}
	return FALSE;
}

int __fastcall CDrawGroup::DownItem(int n)
{
	if( n < (m_Cnt - 1) ){
		CDraw *pItem = pBase[n+1];
		pBase[n+1] = pBase[n];
		pBase[n] = pItem;
		return TRUE;
	}
	return FALSE;
}
//--------------------------------------------------------------------------
void __fastcall DrawTrans(TCanvas *tp, int x, int y)
{
	int rop = ::SetROP2(tp->Handle, R2_NOT);
	tp->MoveTo(x - 5, y - 5);
	tp->LineTo(x + 5, y - 5);
	tp->LineTo(x + 5, y + 5);
	tp->LineTo(x - 5, y + 5);
	tp->LineTo(x - 5, y - 5);
	tp->MoveTo(x - 5, y - 5);
	tp->LineTo(x + 5, y + 5);
	tp->MoveTo(x - 5, y + 5);
	tp->LineTo(x + 5, y - 5);
	::SetROP2(tp->Handle, rop);
}
//--------------------------------------------------------------------------
void __fastcall CDrawGroup::DrawTrans(TCanvas *tp)
{
	::DrawTrans(tp, m_TransX, m_TransY);
}
//--------------------------------------------------------------------------
void __fastcall CDrawGroup::AdjustTransPoint(void)
{
	MultProc();
	int i;
	int f = 0;
	int XW = 320;
	int YW = 256;
	for( i = 0; i < m_Cnt; i++ ){
		if( pBase[i]->HitTest(m_TransX, m_TransY) ){
			for( m_TransX = XW-1; m_TransX >= 0; m_TransX -= 5 ){
				for( m_TransY = YW-1; m_TransY >= 0; m_TransY-- ){
					f = 0;
					for( i = 0; i < m_Cnt; i++ ){
						if( pBase[i]->HitTest(m_TransX, m_TransY) ){
							f++;
							break;
						}
					}
					if( !f ) return;
				}
				MultProc();
			}
			break;
		}
	}
	if( f ){
		for( i = 0; i < m_Cnt; i++ ){
			if( pBase[i]->m_Command == CM_PIC ){
				CDrawPic *pPic = (CDrawPic *)pBase[i];
				if( pPic->m_Type == 2 ){
					m_TransX = pPic->m_X1 + (pPic->m_TransPoint & 0x0000ffff) + 1;
					m_TransY = pPic->m_Y1 + (pPic->m_TransPoint >> 16) + 1;
					if( m_TransX < 0 ) m_TransX = 0;
					if( m_TransY < 0 ) m_TransY = 0;
					if( m_TransX >= XW ) m_TransX = XW-1;
					if( m_TransY >= YW ) m_TransY = YW-1;
					return;
				}
			}
		}
		m_TransX = XW-1;
		m_TransY = YW-1;
	}
}
//--------------------------------------------------------------------------
void __fastcall CDrawGroup::AdjustTransColor(Graphics::TBitmap *pBmp)
{
	TColor org = m_TransCol;
//	m_TransCol = clWhite;
	m_TransCol = GetUniqueColor(m_TransCol);
	if( !m_Cnt ) return;

	CWaitCursor wait;

	int XW = pBmp->Width;
	int YW = pBmp->Height;
	int i, y, x, f;
	for( i = 0; i < 512; i++ ){
		f = 0;
		for( y = 0; y < YW; y++ ){
			for( x = 0; x < XW; x++ ){
				if( pBmp->Canvas->Pixels[x][y] == m_TransCol ){
					f++;
					break;
				}
			}
			if( f ) break;
		}
		if( !f ) break;
		m_TransCol = GetUniqueColor(m_TransCol);
		MultProc();
	}
	if( org != m_TransCol ){
		UpdateText();
	}
}

//--------------------------------------------------------------------------
void __fastcall CDrawGroup::GetColorBarPos(int &Y1, int &Y2)
{
	int H = Y2;
	int i;
	for( i = 0; i < m_Cnt; i++ ){
		CDraw *pItem = pBase[i];
		if( pItem->m_Command == CM_TITLE ){
			if( pItem->m_Y2 < 20 ){
				Y1 = pItem->m_Y2;
			}
			else if( pItem->m_Y1 >= (H - 20) ){
				Y2 = pItem->m_Y1;
			}
		}
	}
	if( Y1 < 0 ) Y1 = 0;
	if( Y2 > H ) Y2 = H;
}
//--------------------------------------------------------------------------
CDraw* __fastcall CDrawGroup::GetLastItem(void)
{
	return m_Cnt ? pBase[m_Cnt-1]: NULL;
}
//--------------------------------------------------------------------------
int __fastcall CDrawGroup::IsConvBitmap(CDraw *pItem)
{
	if( pItem == NULL ) return FALSE;

	switch(pItem->m_Command){
		case CM_PIC:
			{
				CDrawPic *pPic = (CDrawPic *)pItem;
				return pPic->m_Type != 1 ? TRUE : FALSE;
			}
		case CM_TEXT:
			{
				CDrawText *pText = (CDrawText *)pItem;
				if( pText->pBitmap == NULL ) return FALSE;
			}
			return TRUE;
		case CM_OLE:
			return TRUE;
		case CM_LIB:
			return TRUE;
	}
	return FALSE;
}
//--------------------------------------------------------------------------
CDrawPic* __fastcall CDrawGroup::MakePic(int x, int y, Graphics::TBitmap *pBitmap, int ov)
{
	CDrawPic *pPic = new CDrawPic;
	pPic->Start(Mmsstv->PBoxTemp->Canvas, x, y);
	if( pPic->FinishR(x + pBitmap->Width + 1, y + pBitmap->Height + 1) ){
		pPic->m_Type = ov ? 2 : 1;
		pPic->m_Shape = 0;
		pPic->m_LineStyle = TPenStyle(-1);
		pBitmap->Transparent = FALSE;
		pPic->pBitmap->Canvas->Draw(0, 0, pBitmap);
		return pPic;
	}
	return NULL;
}
//--------------------------------------------------------------------------
void __fastcall CDrawGroup::ConvBitmap(CDraw *pItem)
{
	if( pItem == NULL ) return;

	int i;
	for( i = 0; i < m_Cnt; i++ ){
		if( pBase[i] == pItem ){
			switch(pItem->m_Command){
				case CM_PIC:
					{
						CDrawPic *pPic = (CDrawPic *)pItem;
						pPic->m_Type = 1;
					}
					break;
				case CM_TEXT:
					{
						CDrawText *pText = (CDrawText *)pItem;
						if( pText->pBitmap == NULL ) return;
						pSel = MakePic(pText->m_X1 - 1, pText->m_Y1 - 1, pText->pBitmap, 1);
						if( pSel != NULL ){
							pBase[i] = pSel;
							delete pText;
						}
					}
					break;
				case CM_OLE:
					{
						CDrawOle *pOle = (CDrawOle *)pItem;
						Graphics::TBitmap *pBitmap = pOle->MakeBitmap();
						pSel = MakePic(pOle->m_X1 - 1, pOle->m_Y1 - 1, pBitmap, pOle->m_Trans);
						if( pSel != NULL ){
							pBase[i] = pSel;
							delete pOle;
						}
						delete pBitmap;
					}
					break;
				case CM_LIB:
					{
						CDrawLib *pLib = (CDrawLib *)pItem;
						if( pLib->pBitmap == NULL ) return;
						pSel = MakePic(pLib->m_X1 - 1, pLib->m_Y1 - 1, pLib->pBitmap, pLib->m_Type & 0x00010000);
						if( pSel != NULL ){
							pBase[i] = pSel;
							delete pLib;
						}
					}
					break;
			}
			break;
		}
	}
}
//**************************************************************************
//
//---------------------------------------------------------------------------
// ファイルからロード
int __fastcall LoadTemplate(CDrawGroup *pItem, LPCSTR pName, TCanvas *tp)
{
	int r = FALSE;
	try {
		TFileStream *sp = new TFileStream(pName, fmOpenRead);
		r = pItem->LoadFromStream((tp != NULL) ? tp : Mmsstv->PBoxTemp->Canvas, sp);
		delete sp;
		r = TRUE;
	}
	catch(...){
	}
	if( r == FALSE ){
		if( IsFile(pName) ){
			ErrorMB( MsgEng ? "The template file (%s) is not a correct format":"テンプレート(%s)は正しいファイル形式ではありません.", pName);
			unlink(pName);
		}
	}
	MultProc();
	return r;
}
//---------------------------------------------------------------------------
// ファイルへのセーブ
int __fastcall SaveTemplate(CDrawGroup *pItem, LPCSTR pName)
{
	int r = FALSE;
	try {
		TFileStream *sp = new TFileStream(pName, fmCreate);
		pItem->SaveToStream(sp);
		delete sp;
		r = TRUE;
	}
	catch(...){
		if( IsFile(pName) && (GetFileAttributes(pName) & FILE_ATTRIBUTE_READONLY) ){
			ErrorMB("'%s' is read-only.", pName);
		}
		else {
			ErrorMB( MsgEng ? "Cannot update '%s":"'%s'を更新できません.", pName);
		}
	}
	MultProc();
	return r;
}
//**************************************************************************
//
//--------------------------------------------------------------------------
void __fastcall CPolygon::Free(void)
{
	if( pBase != NULL ){
		delete pBase;
		pBase = NULL;
	}
}

void __fastcall CPolygon::AddPoint(int X, int Y)
{
	if( Cnt ){
		if( (pBase[Cnt-1].x == X) && (pBase[Cnt-1].y == Y) ) return;
	}
	if( Cnt >= Max ){
		int max = Max ? Max * 2 : 4;
		POINT *pNew = new POINT[max];
		if( pBase != NULL ){
			memcpy(pNew, pBase, Cnt * sizeof(POINT));
			delete pBase;
		}
		pBase = pNew;
		Max = max;
	}
	pBase[Cnt].x = X;
	pBase[Cnt].y = Y;
	Cnt++;
}

void __fastcall CPolygon::SaveToStream(TStream *sp)
{
	int id = 0x55aa2233;
	sp->Write(&id, sizeof(id));
	sp->Write(&Cnt, sizeof(Cnt));
	sp->Write(&XW, sizeof(XW));
	sp->Write(&YW, sizeof(YW));
	int i;
	POINT   *p = pBase;
	for( i = 0; i < Cnt; i++, p++ ){
		sp->Write(p, sizeof(POINT));
	}
}

int __fastcall CPolygon::LoadFromStream(TStream *sp)
{
	sp->Read(&Cnt, sizeof(Cnt));
	if( Cnt == 0x55aa2233 ){
		sp->Read(&Cnt, sizeof(Cnt));
		sp->Read(&XW, sizeof(XW));
		sp->Read(&YW, sizeof(YW));
	}
	else {
		XW = 256;
		YW = 200;
	}
	if( pBase != NULL ){
		delete pBase;
	}
	Max = Cnt;
	if( Cnt ){
		pBase = new POINT[Max];
		POINT   *p = pBase;
		for( Cnt = 0; Cnt < Max; Cnt++, p++ ){
			sp->Read(p, sizeof(POINT));
			if( XW != 320 ) p->x = p->x * 320 / XW;
			if( YW != 256 ) p->y = p->y * 256 / XW;
		}
	}
	XW = 320;
	YW = 256;
	return TRUE;
}

void __fastcall CPolygon::Draw(TCanvas *tp, int loop)
{
	if( !Cnt ) return;

	tp->Brush->Color = clWhite;
	tp->Pen->Color = sys.m_PicSelCurCol;
//    tp->Pen->Color = clBlue;
	tp->Pen->Width = 2;
	tp->Pen->Style = psSolid;
//    int rop = ::SetROP2(tp->Handle, R2_MERGEPENNOT);
	for( int i = 0; i < Cnt; i++ ){
		if( i ){
			tp->LineTo(pBase[i].x, pBase[i].y);
		}
		else {
			tp->MoveTo(pBase[i].x, pBase[i].y);
		}
	}
	if( loop ){
		tp->LineTo(pBase[0].x, pBase[0].y);
	}
//    ::SetROP2(tp->Handle, rop);
}

void __fastcall CPolygon::Copy(CPolygon *p)
{
	if( pBase != NULL ){
		delete pBase;
	}
	Max = Cnt = p->Cnt;
	pBase = new POINT[Max];
	memcpy(pBase, p->pBase, sizeof(POINT) * Cnt);
	XW = p->XW;
	YW = p->YW;
}

void __fastcall CPolygon::GetCenter(int &xc, int &yc)
{
	if( Cnt < 3 ){
		xc = XW/2;
		yc = YW/2;
	}
	else {
		int X, Y;
		int XL = 640;
		int YT = 496;
		int XR = 0;
		int YB = 0;
		int i;
		for( i = 0; i < Cnt; i++ ){
			X = pBase[i].x;
			Y = pBase[i].y;
			if( XL > X ) XL = X;
			if( XR < X ) XR = X;
			if( YT > Y ) YT = Y;
			if( YB < Y ) YB = Y;
		}
		xc = (XR + XL)/2;
		yc = (YB + YT)/2;
	}
}

void __fastcall CPolygon::DrawOffset(TCanvas *tp, int x, int y)
{
	POINT *pTemp = new POINT[Cnt];
	for( int i = 0; i < Cnt; i++ ){
		pTemp[i].x = pBase[i].x + x;
		pTemp[i].y = pBase[i].y + y;
	}
	tp->Polygon(pTemp, Cnt-1);
	delete pTemp;
}

void __fastcall CPolygon::DrawOffsetSiege(TCanvas *tp, int x, int y, TColor SCol, int e)
{
	if( Cnt < 3 ) return;
	int XC, YC;
	GetCenter(XC, YC);

	e = e == 5 ? 4 : 3;
	int i, j;
	POINT *pTemp = new POINT[Cnt];
	tp->Pen->Style = psSolid;
	tp->Pen->Width = 1;
	tp->Brush->Style = bsClear;
	TColor col = SCol;
	TColor tcol = SCol == clBlack ? clWhite : clBlack;
	double deg, r, xx, yy;
	for( i = 0; i < e; i++ ){
		for( j = 0; j < Cnt; j++ ){
#if 0
			if( pBase[j].x > XC ){
				pTemp[j].x = x + pBase[j].x + i;
			}
			else {
				pTemp[j].x = x + pBase[j].x - i;
			}
			if( pBase[j].y > YC ){
				pTemp[j].y = y + pBase[j].y + i;
			}
			else {
				pTemp[j].y = y + pBase[j].y - i;
			}
#else
			xx = pBase[j].x - XC;
			yy = YC - pBase[j].y;
			if( xx > 0 ){
				deg = atan(double(yy)/double(xx));
				r = sqrt(xx * xx + yy * yy);
			}
			else if( xx < 0 ){
				deg = atan(double(yy)/double(xx));
				deg += PI;
				r = sqrt(xx * xx + yy * yy);
			}
			else {
				deg = yy < 0 ? -PI/2 : PI/2;
				r = ABS(yy);
			}
			r += i;
			pTemp[j].x = x + XC + r * cos(deg) + 0.5;
			pTemp[j].y = y + YC - r * sin(deg) + 0.5;
#endif
		}
		tp->Pen->Color = GetCol(col, tcol, i, e);
		tp->Polygon(pTemp, Cnt - 1);
	}
	for( i = 1; i < e; i++ ){
		for( j = 0; j < Cnt; j++ ){
#if 0
			if( pBase[j].x > XC ){
				pTemp[j].x = x + pBase[j].x - i;
			}
			else {
				pTemp[j].x = x + pBase[j].x + i;
			}
			if( pBase[j].y > YC ){
				pTemp[j].y = y + pBase[j].y - i;
			}
			else {
				pTemp[j].y = y + pBase[j].y + i;
			}
#else
			xx = pBase[j].x - XC;
			yy = YC - pBase[j].y;
			if( xx > 0 ){
				deg = atan(double(yy)/double(xx));
				r = sqrt(xx * xx + yy * yy);
			}
			else if( xx < 0 ){
				deg = atan(double(yy)/double(xx));
				deg += PI;
				r = sqrt(xx * xx + yy * yy);
			}
			else {
				deg = yy < 0 ? -PI/2 : PI/2;
				r = ABS(yy);
			}
			r -= i;
			pTemp[j].x = x + XC + r * cos(deg) + 0.5;
			pTemp[j].y = y + YC - r * sin(deg) + 0.5;
#endif
		}
		tp->Pen->Color = GetCol(col, tcol, i, e);
		tp->Polygon(pTemp, Cnt - 1);
	}
	delete pTemp;
	tp->Brush->Style = bsSolid;
}

void __fastcall CPolygon::Stretch(int xw, int yw, int oxw, int oyw)
{
	if( (xw == oxw) && (oxw == oyw) ) return;

	double sx = double(xw)/double(oxw);
	double sy = double(yw)/double(oyw);
	for( int i = 0; i < Cnt; i++ ){
		pBase[i].x *= sx;
		pBase[i].y *= sy;
	}
}

void __fastcall CPolygon::Stretch(CPolygon &pol)
{
	Stretch(XW, YW, pol.XW, pol.YW);
}

//---------------------------------------------------------------------------
// 透視変換
static void __fastcall Matrix(double l[4][4], double m[4][4], double n[4][4])
{
	int i, j, k;
	double  d;

	for( i = 0; i < 4; i++ ){
		for( j = 0; j < 4; j++ ){
			for( k = 0, d = 0.0; k < 4; k++ ) d += l[i][k] * m[k][j];
			n[i][j] = d;
		}
	}
}

void __fastcall PerspectPara(double k[9], const SPERSPECT *pPar, int xs, int ys)
{
	double  l[4][4]; double m[4][4]; double n[4][4];
	double  k1, k2, k3, k4, k5, k6, k7, k8, k9;
	double  u, v, w;

	u = pPar->rx * PI / 180;
	v = pPar->ry * PI / 180;
	w = pPar->rz * PI / 180;
	memset(l, 0, sizeof(l)); memset(m, 0, sizeof(m));
	l[0][0] = 1.0/xs;   l[1][1] = -1.0/xs;  l[2][2] = 1.0;  l[3][3] = 1.0;
	m[0][0] = pPar->ax; m[1][1] = pPar->ay; m[2][2] = 1.0;  m[3][3] = 1.0;
	Matrix(l, m, n);
	memset(l, 0, sizeof(l));
	l[0][0] = 1.0;      l[1][1] = 1.0;      l[2][2] = 1.0;
	l[3][0] = pPar->px; l[3][1] = pPar->py; l[3][2] = pPar->pz; l[3][3] = 1.0;
	Matrix(n, l, m);
	memset(n, 0, sizeof(n));
	n[0][0] = cos(w);   n[0][1] = sin(w);
	n[1][0] = -n[0][1]; n[1][1] = n[0][0];
	n[2][2] = 1.0;      n[3][3] = 1.0;
	Matrix(m, n, l);
	memset(m, 0, sizeof(m));
	m[0][0] = 1.0;      m[1][1] = cos(u);   m[1][2] = sin(u);
	m[2][1] = -m[1][2]; m[2][2] = m[1][1];  m[3][3] = 1.0;
	Matrix(l, m, n);
	memset(l, 0, sizeof(l));
	l[0][0] = cos(v);   l[0][2] = sin(v);   l[1][1] = 1.0;
	l[2][0] = -l[0][2]; l[2][2] = l[0][0];  l[3][3] = 1.0;
	Matrix(n, l, m);
	memset(n, 0, sizeof(n));
	n[0][0] = 1.0;      n[1][1] = 1.0;      n[2][2] = -1.0; n[3][2] = -pPar->v; n[3][3] = 1.0;
	Matrix(m, n, l);
	memset(m, 0, sizeof(m));
	m[0][0] = 1.0;      m[1][1] = 1.0;      m[2][2] = 1/pPar->s;    m[2][3] = m[2][2];
	m[3][2] = -1.0;
	Matrix(l, m, n);
	memset(l, 0, sizeof(l));
	l[0][0] = xs;       l[1][1] = -xs;      l[2][2] = 1.0;      l[3][3] = 1.0;
	Matrix(n, l, m);

	k1 = m[0][3];   k2 = m[1][3];   k3 = m[3][3];
	k4 = m[0][0];   k5 = m[1][0];   k6 = m[3][0];
	k7 = m[0][1];   k8 = m[1][1];   k9 = m[3][1];

	k[0] = k7*k2 - k8*k1;   k[1] = k5*k1 - k4*k2;   k[2] = k4*k8 - k7*k5;
	k[3] = k8*k3 - k9*k2;   k[6] = k9*k1 - k7*k5;   k[4] = k6*k2 - k5*k3;
	k[7] = k4*k3 - k6*k1;   k[5] = k5*k9 - k8*k6;   k[8] = k7*k6 - k4*k9;
}

void __fastcall Perspect(Graphics::TBitmap *pSrc, Graphics::TBitmap *pTer, const SPERSPECT *pPar, TColor back)
{
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
		double  x, y, w, p, q;
		double  k[9];
		int xs = pT->Width / 2;
		int ys = pT->Height / 2;
		PerspectPara(k, pPar, xs, ys);
		BYTE *sp1, *sp2;
		BYTE *tp;
		sp1 = (BYTE *)pS->ScanLine[0];
		BYTE br = *sp1++;
		BYTE bg = *sp1++;
		BYTE bb = *sp1;
		int r, g, b;
		for( i = -ys; i < ys; i++ ){
			tp = (BYTE *)pT->ScanLine[i+ys];
			for( j = -xs; j < xs; j++ ){
				w = k[0] * j + k[1] * i + k[2];
				x = k[3] * j + k[4] * i + k[5];
				y = k[6] * j + k[7] * i + k[8];
				x = x / w;
				y = y / w;
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
//                        if( ABS(r-br) < 16 ) r = br;
					}
					else {
						r = *sp1;
					}
					sp1++; sp2++;
					if( (*sp1 != *sp2) || (*sp1 != *(sp1+3)) || (*sp1 != *(sp2+3)) ){
						g = (int)(qq*(pp*(*sp1) + p*(*(sp1+3)))
							+ q * (pp*(*sp2) + p*(*(sp2+3))));
//                        if( ABS(g-bg) < 16 ) g = bg;
					}
					else {
						g = *sp1;
					}
					sp1++; sp2++;
					if( (*sp1 != *sp2) || (*sp1 != *(sp1+3)) || (*sp1 != *(sp2+3)) ){
						b = (int)(qq*(pp*(*sp1) + p*(*(sp1+3)))
							+ q * (pp*(*sp2) + p*(*(sp2+3))));
//                        if( ABS(b-bb) < 16 ) b = bb;
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
//
//
//***************************************************************************
//CLIBLクラス
//***************************************************************************
//
//
//---------------------------------------------------------------------------
void __fastcall CLIBL::Alloc(void)
{
	int am = m_AMax ? (m_AMax * 2) : 16;
	LIBD *pN = new LIBD[am];
	if( m_pBase != NULL ){
		memcpy(pN, m_pBase, sizeof(LIBD)*m_Count);
		delete m_pBase;
	}
	m_pBase = pN;
	m_AMax = am;
}
//---------------------------------------------------------------------------
void __fastcall CLIBL::Delete(void)
{
	if( m_pBase != NULL ){
		LIBD *cp = m_pBase;
		for( int i = 0; i < m_Count; i++, cp++ ){
			delete cp->pName;
			::FreeLibrary(cp->hLib);
		}
		delete m_pBase;
	}
	m_pBase = NULL;
	m_AMax = 0;
	m_Count = 0;
}
//---------------------------------------------------------------------------
void __fastcall CLIBL::Add(LPCSTR pName, HANDLE hLib)
{
	if( m_Count >= m_AMax ) Alloc();
	LIBD *cp = &m_pBase[m_Count];
	cp->pName = StrDupe(pName);
	cp->hLib = (HINSTANCE)hLib;
	m_Count++;
}
//---------------------------------------------------------------------------
HANDLE __fastcall CLIBL::LoadLibrary(LPCSTR pName)
{
	LIBD *cp = m_pBase;
	for( int i = 0; i < m_Count; i++, cp++ ){
		if( !strcmpi(cp->pName, pName) ){
			if( cp->hLib == NULL ){
				cp->hLib = ::LoadLibrary(pName);
			}
			return cp->hLib;
		}
	}
	HANDLE hLib = ::LoadLibrary(pName);
	if( hLib != NULL ) Add(pName, hLib);
	return hLib;
}
//---------------------------------------------------------------------------
void __fastcall CLIBL::DeleteLibrary(HANDLE hLib)
{
	if( hLib == NULL ) return;

	LIBD *cp = m_pBase;
	for( int i = 0; i < m_Count; i++, cp++ ){
		if( cp->hLib == hLib ){
			::FreeLibrary((HINSTANCE)hLib);
			cp->hLib = NULL;
			break;
		}
	}
}

#if 0
//---------------------------------------------------------------------------
__fastcall CGrid::CGrid()
{
	m_Flag = 0;
	m_Intval = 16;
}
//---------------------------------------------------------------------------
void __fastcall CGrid::Draw(TCanvas *tp, int xw, int yw)
{
	if( !m_Flag ) return;

	int x, y, xx, yy;

	tp->Pen->Width = 1;
	int rop = ::SetROP2(tp->Handle, R2_NOT);
	for( x = 0; x < xw; x += m_Intval ){
		xx = x * 320 / xw;
		tp->MoveTo(xx, 0);
		tp->LineTo(xx, yw-1);
	}
	for( y = 0; y < yw; y += m_Intval ){
		yy = y * 256 / yw;
		tp->MoveTo(0, yy);
		tp->LineTo(xw-1, yy);
	}
	::SetROP2(tp->Handle, rop);
}
//---------------------------------------------------------------------------
void __fastcall CGrid::Align(int &x1, int &y1, int &x2, int &y2)
{
	if( !m_Flag ) return;

	int xw = x2 - x1;
	int yw = y2 - y1;
	x1 = int((double(x1) / m_Intval) + 0.5) * m_Intval;
	x2 = x1 + xw;
	y1 = int((double(y1) / m_Intval) + 0.5) * m_Intval;
	y2 = y1 + yw;
}
//---------------------------------------------------------------------------
void __fastcall CGrid::Align(CDraw *pItem)
{
	Align(pItem->m_X1, pItem->m_Y1, pItem->m_X2, pItem->m_Y2);
}
#endif

#pragma package(smart_init)

