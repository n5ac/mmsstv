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
#include "ComLib.h"
#include "PicRect.h"
#include "PicFilte.h"
#include "BitMask.h"
#include "Main.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TBitMaskDlg *BitMaskDlg;
//---------------------------------------------------------------------
void __fastcall MakeBitmapPtn(Graphics::TBitmap *pBmp, int hw, int sw, TColor col1, TColor col2)
{
	CWaitCursor wait;
	AdjustBitmapFormat(pBmp);
	hw /= 3;
	TCanvas *pCanvas = pBmp->Canvas;
	int x, y, f, v;
	for( y = 0; y < pBmp->Height; y++ ){
		for( x = 0; x < pBmp->Width; x++ ){
			f = x / hw;
			switch(sw){
			case 0:
				if( (y/2) & 1 ){
					v = (x/2) & 1;
				}
				else {
					v = !((x/2) & 1);
				}
				break;
			case 1:
				if( (y/4) & 1 ){
					v = (x/2) & 1;
				}
				else {
					v = !((x/2) & 1);
				}
				break;
			case 2:
				if( (y/2) & 1 ){
					v = (x/4) & 1;
				}
				else {
					v = !((x/4) & 1);
				}
				break;
			case 3:
				if( (y/4) & 1 ){
					v = (x/4) & 1;
				}
				else {
					v = !((x/4) & 1);
				}
				break;
			case 4:
				switch(f){
					case 0:
						v = (y/2) & 1;
                    	break;
                    case 1:
						v = (y/4) & 1;
                    	break;
                    default:
						v = (y/8) & 1;
                    	break;
                }
				break;
			case 5:
				switch(f){
					case 0:
						v = (x/2) & 1;
                    	break;
                    case 1:
						v = (x/4) & 1;
                    	break;
                    default:
						v = (x/8) & 1;
                    	break;
                }
				break;
			case 6:
				switch(f){
					case 0:
						v = ((y+x)/2) & 1;
                    	break;
                    case 1:
						v = ((y+x)/4) & 1;
                    	break;
                    default:
						v = ((y+x)/8) & 1;
                    	break;
                }
				break;
			case 7:
				switch(f){
					case 0:
						v = ((x-y+1024)/2) & 1;
                    	break;
                    case 1:
						v = ((x-y+1024)/4) & 1;
                    	break;
                    default:
						v = ((x-y+1024)/8) & 1;
                    	break;
                }
				break;
			}
			pCanvas->Pixels[x][y] = v ? col1 : col2;
    	}
    }
#if 0

    //    pBmp->PixelFormat = pf24bit;
	TCanvas *pCanvas = pBmp->Canvas;
	int x, y;
	for( y = 0; y < pBmp->Height; y++ ){
		for( x = 0; x < pBmp->Width; x++ ){
			switch(sw){
			case 0:
				if( (y/2) & 1 ){
					pCanvas->Pixels[x][y] = (x/2) & 1 ? col1 : col2;
				}
				else {
					pCanvas->Pixels[x][y] = (x/2) & 1 ? col2 : col1;
				}
				break;
			case 1:
				if( (y/4) & 1 ){
					pCanvas->Pixels[x][y] = (x/2) & 1 ? col1 : col2;
				}
				else {
					pCanvas->Pixels[x][y] = (x/2) & 1 ? col2 : col1;
				}
				break;
			case 2:
				if( (y/2) & 1 ){
					pCanvas->Pixels[x][y] = (x/4) & 1 ? col1 : col2;
				}
				else {
					pCanvas->Pixels[x][y] = (x/4) & 1 ? col2 : col1;
				}
				break;
			case 3:
				if( (y/4) & 1 ){
					pCanvas->Pixels[x][y] = (x/4) & 1 ? col1 : col2;
				}
				else {
					pCanvas->Pixels[x][y] = (x/4) & 1 ? col2 : col1;
				}
				break;
			case 4:
				pCanvas->Pixels[x][y] = (y/4) & 1 ? col1 : col2;
				break;
			case 5:
				pCanvas->Pixels[x][y] = (x/2) & 1 ? col1 : col2;
				break;
			case 6:
				pCanvas->Pixels[x][y] = ((y+x)/4) & 1 ? col1 : col2;
				break;
			case 7:
				pCanvas->Pixels[x][y] = ((x-y+1024)/2) & 1 ? col1 : col2;
				break;
			}
    	}
    }
#endif
}
//---------------------------------------------------------------------
__fastcall TBitMaskDlg::TBitMaskDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	m_DisEvent = TRUE;
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		Caption = "Bitmap mask";
		CancelBtn->Caption = "Cancel";
		L1->Caption = "Size";
		GB2->Caption = "Make and Edit mask";
		PaintBtn->Caption = "Edit";

        SBPaste->Hint = "Paste mask from the clipboard";
		SBCopy->Hint = "Copy mask to the clipboard";
		SBColAdj->Hint = "Adjusts mask image";
		GB1->Caption = "User definition mask";
        L3->Caption = "Drag&&Drop for registration";
	}
    SBPaste->Glyph->Assign(Mmsstv->SBPaste->Glyph);
    SBColAdj->Glyph->Assign(Mmsstv->SBHistFil->Glyph);
    SBCopy->Glyph->Assign(Mmsstv->SBCopy->Glyph);
	pBitmap = new Graphics::TBitmap();
	AdjustBitmapFormat(pBitmap);
	pBitmap->Width = PBox->Width;
	pBitmap->Height = PBox->Height;
    m_pCustomBmp = NULL;
	m_pUserBmp = NULL;
	m_pListBmp = NULL;
	m_X = m_Y = 0;
	m_Size = 0;
	char name[256];
    sprintf(name, "%sTextArt.bmp", BgnDir);
    m_ListName = name;
    UDL->Position = (SHORT)sys.m_MaskUserPage;
	PC1->Color = sys.m_MaskCol1;
	PC2->Color = sys.m_MaskCol2;
    m_hClipNext = ::SetClipboardViewer(Handle);
    ShowHint = WinNT;
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::FormDestroy(TObject *Sender)
{
	sys.m_MaskUserPage = UDL->Position;
	sys.m_MaskCol1 = PC1->Color;
	sys.m_MaskCol2 = PC2->Color;
	::ChangeClipboardChain(Handle, m_hClipNext);
	g_ExecPB.Cancel();
    if( pBitmap != NULL ){
		delete pBitmap;
        pBitmap = NULL;
    }
    if( m_pCustomBmp != NULL ){
		delete m_pCustomBmp;
        m_pCustomBmp = NULL;
    }
    if( m_pListBmp != NULL ){
		if( m_pListBmp->Modified ){
        	m_pListBmp->SaveToFile(m_ListName);
		}
		delete m_pListBmp;
        m_pListBmp = NULL;
    }
    if( m_pUserBmp != NULL ){
		delete m_pUserBmp;
        m_pUserBmp = NULL;
    }
}
//---------------------------------------------------------------------------
int __fastcall TBitMaskDlg::Execute(Graphics::TBitmap *pBmp, CDrawText *pText)
{
	m_Text.Start(PBoxText->Canvas, 0, 0);       // キャンバスのセット
	m_Text.Copy(pText);
	m_Text.UpdateText();
	pBitmap->Width = pBmp->Width;
	pBitmap->Height = pBmp->Height;
    m_XW = pBitmap->Width;
    m_YW = pBitmap->Height;
	UpdateCBSize(m_XW, m_YW);
	FillBitmap(pBitmap, clGray);
	pBitmap->Canvas->Draw(0, 0, pBmp);
	UpdateSB();
	m_DisEvent = FALSE;
	if( ShowModal() == IDOK ){
		pBmp->Width = m_XW < pBitmap->Width ? m_XW : pBitmap->Width;
		pBmp->Height = m_YW < pBitmap->Height ? m_YW : pBitmap->Height;
		pBmp->Canvas->Draw(-m_X, -m_Y, pBitmap);
		return TRUE;
	}
	return FALSE;
}
//---------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxPaint(TObject *Sender)
{
#if 0
	TRect rc;
    rc.Left = 0; rc.Top = 0;
    rc.Right = PBox->Width; rc.Bottom = PBox->Height;
    TCanvas *pCanvas = PBox->Canvas;
    pCanvas->Brush->Color = clBtnFace;
    pCanvas->FillRect(rc);
#endif
	PBox->Canvas->Draw(-SBH->Position, -SBV->Position, pBitmap);
	DrawCursor();
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::UpdateSB(void)
{
	SBH->Position = 0; SBV->Position = 0;
	int hmax, vmax;
	if( pBitmap ){
		int xw = pBitmap->Width;
	    int yw = pBitmap->Height;
	    hmax = xw - PBox->ClientWidth;
	    vmax = yw - PBox->ClientHeight;
    }
    else {
		hmax = vmax = -1;
    }
    SBH->Max = (hmax > 0) ? hmax : 0;
    SBV->Max = (vmax > 0) ? vmax : 0;
	SBH->Enabled = (hmax > 0) ? TRUE : FALSE;
    SBV->Enabled = (vmax > 0) ? TRUE : FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::LoadBtnClick(TObject *Sender)
{
	TOpenDialog *pDialog = Mmsstv->OpenDialog;
	pDialog->Options >> ofCreatePrompt;
	pDialog->Options << ofFileMustExist;
	if( MsgEng ){
		pDialog->Title = "Load picture from file";
	}
	else {
		pDialog->Title = "画像のロード";
	}
	pDialog->Filter = GetPicFilter();
	pDialog->FileName = "";
	pDialog->DefaultExt = "jpg";
	pDialog->InitialDir = BitmapDir;
	SetDisPaint();
	NormalWindow(this);
	if( Mmsstv->OpenDialogExecute(TRUE) == TRUE ){
		SetDirName(BitmapDir, AnsiString(pDialog->FileName).c_str());
		if( LoadImage(pBitmap, AnsiString(pDialog->FileName).c_str()) == TRUE ){
			UpdateSrc(TRUE);
            UpdateUser();
		}
	}
	ResDisPaint();
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::UpdateCBSize(int xw, int yw)
{
	m_DisEvent++;
    int f = TRUE;
	if( xw == yw ){
		switch(xw){
			case 8:
				CBSize->ItemIndex = 0;
            	break;
            case 16:
				CBSize->ItemIndex = 1;
            	break;
            case 24:
				CBSize->ItemIndex = 2;
            	break;
            case 32:
				CBSize->ItemIndex = 3;
            	break;
            default:
            	f = FALSE;
                break;
        }
    }
    else {
		f = FALSE;
    }
	if( f ){
    	if( CBSize->Items->Count > 4 ) CBSize->Items->Delete(4);
    }
    else {
		char bf[256];
        sprintf(bf, "%ux%u", xw, yw);
    	if( CBSize->Items->Count > 4 ){
	        CBSize->Items->Strings[4] = bf;
        }
        else {
	        CBSize->Items->Add(bf);
        }
        CBSize->ItemIndex = 4;
    }
    m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::AdjustSize(void)
{
	m_X = m_Y = 0;
	int xw = pBitmap->Width;
    int yw = pBitmap->Height;

    if( (xw == yw) && (xw <= 32) ){
		switch(xw){
            case 8:
            case 16:
            case 24:
            case 32:
            	m_XW = m_YW = xw;
            	break;
        }
    }
    else if( (xw >= 32) && (yw >= 32) ){
		m_XW = m_YW = 32;
    }
    UpdateCBSize(m_XW, m_YW);
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::DrawCursor(void)
{
	int xw = m_XW;
    int yw = m_YW;
	int xo = -SBH->Position;
    int yo = -SBV->Position;
	TCanvas *tp = PBox->Canvas;
	tp->Pen->Width = 1;
	tp->Pen->Style = psDot;
	tp->MoveTo(xo+m_X, yo+m_Y);
	int rop = ::SetROP2(tp->Handle, R2_NOT);
	tp->LineTo(xo+m_X+xw, yo+m_Y);
	tp->LineTo(xo+m_X+xw, yo+m_Y+yw);
	tp->LineTo(xo+m_X, yo+m_Y+yw);
	tp->LineTo(xo+m_X, yo+m_Y);
	::SetROP2(tp->Handle, rop);
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::UpdateText(void)
{
	CWaitCursor w;

	int xw = m_XW;
    int yw = m_YW;
	if( xw <= 0 ) xw = 1;
    if( yw <= 0 ) yw = 1;
	Graphics::TBitmap *pBmp = CreateBitmap(xw, yw, -1);
    pBmp->Canvas->Draw(-m_X, -m_Y, pBitmap);
	CopyBitmap(m_Text.pBrushBitmap, pBmp);
    delete pBmp;
	m_Text.UpdateText();
    PBoxTextPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::BtnClick(int n)
{
	pBitmap->Width = PBox->Width;
	pBitmap->Height = PBox->Height;
	MakeBitmapPtn(pBitmap, PBox->Width, n, PC1->Color, PC2->Color);
	m_XW = m_YW = 8;
    UpdateCBSize(m_XW, m_YW);
	UpdateSrc(FALSE);
    UpdateUser();
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PCClick(TObject *Sender)
{
	TPanel *pPanel = (TPanel *)Sender;
	TColorDialog *pDialog = Mmsstv->ColorDialog;
	InitCustomColor(pDialog);
	AddCustomColor(pDialog, PC1->Color);
	AddCustomColor(pDialog, PC2->Color);
	pDialog->Color = pPanel->Color;
	SetDisPaint();
	if( pDialog->Execute() == TRUE ){
		pPanel->Color = pDialog->Color;
		UpdateCustom();
        PBoxCSPaint(NULL);
	}
	ResDisPaint();
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbLeft ){
    	X += SBH->Position; Y += SBV->Position;
        m_X = X; m_Y = Y;
		m_Size = 1;
	}
	else {
		m_X = 0; m_Y = 0;
	}
    UpdateText();
	PBox->Invalidate();
    UpdateUser();
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if( m_Size ){
    	X += SBH->Position; Y += SBV->Position;
		if( m_Size == 1 ){
			if( (ABS(X-m_X)>4) || (ABS(Y-m_Y)>4) ){
				m_Size++;
            }
        }
		if( m_Size == 2 ){
			DrawCursor();
			m_XW = ABS(X - m_X);
			m_YW = ABS(Y - m_Y);
            if( !m_XW ) m_XW = 1;
            if( !m_YW ) m_YW = 1;
			DrawCursor();
			UpdateCBSize(m_XW, m_YW);
        }
	}
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( m_Size == 2 ){
		UpdateText();
		PBox->Invalidate();
	    UpdateUser();
	}
    m_Size = 0;
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PaintBtnClick(TObject *Sender)
{
	g_ExecPB.Exec(pBitmap, (HWND)Handle, CM_EDITEXIT, 0);
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::OnEditExit(TMessage Message)
{
	if( Message.LParam ) return;

	if( g_ExecPB.LoadBitmap(pBitmap, TRUE) ){
		UpdateSrc(FALSE);
        UpdateUser();
    }
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::OnDrawClipboard(TMessage Message)
{
	SBPaste->Enabled = Clipboard()->HasFormat(CF_BITMAP);
	if( m_hClipNext ) ::SendMessage(m_hClipNext, WM_DRAWCLIPBOARD, Message.WParam, Message.LParam);
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::OnChangeCbChain(TMessage Message)
{
	if( (HWND)Message.WParam == m_hClipNext ){
		m_hClipNext = (HWND)Message.LParam;
	}
	if( m_hClipNext ) ::SendMessage(m_hClipNext, WM_CHANGECBCHAIN, Message.WParam, Message.LParam);
    Message.Result = 0;
}
//---------------------------------------------------------------------------
static void __fastcall DrawBmpPBoxBtn(TCanvas *pCanvas, int x, int y, int w, BOOL sw)
{
	pCanvas->Pen->Style = psSolid;
    pCanvas->Pen->Width = 1;
    pCanvas->Pen->Color = sw ? clBlack : clWhite;
	w++;
    pCanvas->MoveTo(x+w, y);
    pCanvas->LineTo(x, y);
    pCanvas->LineTo(x, y+w);
    pCanvas->LineTo(x+1, y+w);
    pCanvas->Pen->Color = sw ? clWhite : clBlack;
    pCanvas->LineTo(x+w, y+w);
	pCanvas->LineTo(x+w, y);
}
#if 0
//---------------------------------------------------------------------------
static void __fastcall DrawBmpPBox(TCanvas *pCanvas, int x, int y, Graphics::TBitmap *pBmp)
{
   	pCanvas->Draw(x+1, y+1, pBmp);
    DrawBmpPBoxBtn(pCanvas, x, y, pBmp->Width, FALSE);
}
#endif
//---------------------------------------------------------------------------
static void __fastcall DrawBmpPBox(TCanvas *pCanvas, int s, int w, int ws, int x, int y, Graphics::TBitmap *pBmp)
{
	TRect src, drc;

	src.Left = ((y*8)+x)*ws;
    src.Right = src.Left + w;
    src.Top = 0; src.Bottom = w;
    drc.Left = 1 + x*s;
	drc.Right = drc.Left + w;
//    drc.Top = 1 + y*s;
    drc.Top = 1;
    drc.Bottom = drc.Top + w;
    pCanvas->CopyRect(drc, pBmp->Canvas, src);
	DrawBmpPBoxBtn(pCanvas, x*s, y*s, w, FALSE);
}
//---------------------------------------------------------------------------
static int __fastcall GetIndex(int w, int m, int x, int y)
{
	if( ((x % w) < 1) || ((x % w) >= (w-1)) ) return -1;
	if( ((y % w) < 1) || ((y % w) >= (w-1)) ) return -1;
	x /= w;
    y /= w;
	int n = (y * 8) + x;
    if( n >= m ) n = m-1;
    return n;
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::UpdateCustom(void)
{
	if( !m_pCustomBmp ) m_pCustomBmp = CreateBitmap(16*8, 16, -1);
	Graphics::TBitmap *pBmp = CreateBitmap(16, 16, -1);
    int i;
	for( i = 0; i < 8; i++ ){
		MakeBitmapPtn(pBmp, PBox->Width, i, PC1->Color, PC2->Color);
		m_pCustomBmp->Canvas->Draw(i*16, 0, pBmp);
    }
    delete pBmp;
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxCSPaint(TObject *Sender)
{
	if( !m_pCustomBmp ){
		UpdateCustom();
		if( !m_pCustomBmp ) return;
    }
    MultProc();
	int x;
	for( x = 0; x < 8; x++ ){
		DrawBmpPBox(PBoxCS->Canvas, 18, 16, 16, x, 0, m_pCustomBmp);
    }
}
//---------------------------------------------------------------------------

void __fastcall TBitMaskDlg::PBoxCSMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbLeft ){
		m_MIndex = GetIndex(18, 8, X, Y);
        if( m_MIndex >= 0 ){
		    DrawBmpPBoxBtn(PBoxCS->Canvas, 18*(m_MIndex%8), 0, 16, TRUE);
			BtnClick(m_MIndex);
		}
    }
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxCSMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( (Button == mbLeft) && (m_MIndex >= 0) ){
	    DrawBmpPBoxBtn(PBoxCS->Canvas, 18*(m_MIndex%8), 0, 16, FALSE);
	}
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::CBSizeChange(TObject *Sender)
{
    switch(CBSize->ItemIndex){
		case 0:
			m_XW = m_YW = 8;
			break;
        case 1:
			m_XW = m_YW = 16;
			break;
        case 2:
			m_XW = m_YW = 24;
			break;
        case 3:
			m_XW = m_YW = 32;
			break;
    }
	UpdateText();
	PBox->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxTextPaint(TObject *Sender)
{
	if( !m_Text.pBitmap ) m_Text.UpdateText();
	PBoxText->Canvas->Draw(0, 0, m_Text.pBitmap);
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::SBPasteClick(TObject *Sender)
{
	pBitmap->LoadFromClipboardFormat(CF_BITMAP, Clipboard()->GetAsHandle(CF_BITMAP), 0);
	UpdateSrc(TRUE);
    UpdateUser();
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::SBCopyClick(TObject *Sender)
{
	Mmsstv->CopyBitmap(pBitmap);
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::SBColAdjClick(TObject *Sender)
{
	TPicFilterDlg *pBox = new TPicFilterDlg(this);
	if( pBox->Execute(pBitmap) == TRUE ){
		UpdateSrc(FALSE);
        UpdateUser();
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::SBClipClick(TObject *Sender)
{
	Graphics::TBitmap *pBmp = DupeBitmap(pBitmap, -1);
	TPicRectDlg *pBox = new TPicRectDlg(this);
	CopyBitmap(pBox->pBitmap, pBmp);
    pBmp->Width = PBox->Width;
    pBmp->Height = PBox->Height;
	if( pBox->Execute(pBmp) == TRUE ){
		CopyBitmap(pBitmap, pBmp);
		UpdateSrc(TRUE);
    }
    delete pBox;
    delete pBmp;
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::UpdateSrc(BOOL fSize)
{
	if( fSize ) AdjustSize();
	UpdateSB();
    UpdateText();
	PBox->Invalidate();
    CBSize->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::UpdateUser(void)
{
	if( !m_pUserBmp ){
		m_pUserBmp = CreateBitmap(32, 32, pf24bit);
    }
	Graphics::TBitmap *pSrc = m_Text.pBrushBitmap;
    for( int y = 0; y < 32; y += pSrc->Height){
		for( int x = 0; x < 32; x += pSrc->Width ){
			m_pUserBmp->Canvas->Draw(x, y, pSrc);
        }
    }
    PBoxUPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxUPaint(TObject *Sender)
{
	if( !m_pUserBmp ){
		UpdateUser();
    }
    else {
		DrawBmpPBox(PBoxU->Canvas, 30, 28, 32, 0, 0, m_pUserBmp);
    }
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxUMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbLeft ){
		DrawBmpPBoxBtn(PBoxU->Canvas, 0, 0, 28, TRUE);
		if( !IsSameBitmap(m_pUserBmp, m_Text.pBrushBitmap, FALSE) ){
			CopyBitmap(pBitmap, m_pUserBmp);
			UpdateSrc(TRUE);
        }
		::Sleep(100);
		DrawBmpPBoxBtn(PBoxU->Canvas, 0, 0, 28, FALSE);
		PBoxU->BeginDrag(FALSE,0);
    }
}
//---------------------------------------------------------------------------
int __fastcall TBitMaskDlg::IsSameBitmap(Graphics::TBitmap *pDest, Graphics::TBitmap *pSrc, int fSize)
{
	int xw = pSrc->Width;
    int yw = pSrc->Height;
    if( xw > pDest->Width ) return FALSE;
    if( yw > pDest->Height ) return FALSE;
	if( fSize ){
		if( xw != pDest->Width ) return FALSE;
        if( yw != pDest->Height ) return FALSE;
    }
	int x, y;
    for( y = 0; y < yw; y++ ){
		for( x = 0; x < xw; x++ ){
			if( pSrc->Canvas->Pixels[x][y] != pDest->Canvas->Pixels[x][y] ) return FALSE;
        }
    }
    return TRUE;
}
//---------------------------------------------------------------------------
int __fastcall TBitMaskDlg::IsMList(int n)
{
	int yw = m_pListBmp->Height;
	int x, y;
    for( y = 0; y < yw; y++ ){
		LPBYTE p = (LPBYTE)m_pListBmp->ScanLine[y];
        p += (n * 32) * 3;
		for( x = 0; x < 32; x++ ){
			if( *p++ ) return TRUE;
            if( *p++ ) return TRUE;
            if( *p++ ) return TRUE;
        }
    }
    return FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxLPaint(TObject *Sender)
{
	if( !m_pListBmp ){
        if( IsFile(m_ListName.c_str()) ){
        	m_pListBmp = new Graphics::TBitmap;
			m_pListBmp->LoadFromFile(m_ListName);
        }
        else {
			m_pListBmp = CreateBitmap(32*72, 32, pf24bit);
            FillBitmap(m_pListBmp, clBlack);
        }
        m_pListBmp->Modified = FALSE;
    }
	int y = UDL->Position;
	int x;
	for( x = 0; x < 8; x++ ){
		DrawBmpPBox(PBoxL->Canvas, 30, 28, 32, x, y, m_pListBmp);
    }
	char bf[32];
    sprintf(bf, "%u/9", y + 1);
    LP->Caption = bf;
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxLMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_MIndex = GetIndex(30, 8, X, Y);
	if( m_MIndex >= 0 ){
		m_MIndex += (UDL->Position * 8);
		if( (Button == mbLeft) && IsMList(m_MIndex) ){
			DrawBmpPBoxBtn(PBoxL->Canvas, (m_MIndex%8)*30, 0, 28, TRUE);

			Graphics::TBitmap *pBmp = CreateBitmap(32, 32, pf24bit);
			pBmp->Canvas->Draw(-m_MIndex*32, 0, m_pListBmp);
			if( !IsSameBitmap(pBmp, m_Text.pBrushBitmap, TRUE) ){
	            CopyBitmap(pBitmap, pBmp);
				UpdateSrc(TRUE);
            }
            delete pBmp;

            ::Sleep(100);
			DrawBmpPBoxBtn(PBoxL->Canvas, (m_MIndex%8)*30, 0, 28, FALSE);
            PBoxL->BeginDrag(FALSE,0);
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxLDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
	Accept = FALSE;

	int index = GetIndex(30, 8, X, Y);
    if( index >= 0 ){
    	index += (UDL->Position * 8);
		if( Source == PBoxL ){
			if( index != m_MIndex ) Accept = TRUE;
        }
        else {
			Accept = TRUE;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxLDragDrop(TObject *Sender,
      TObject *Source, int X, int Y)
{
	int index = GetIndex(30, 8, X, Y);
    if( index >= 0 ){
    	index += (UDL->Position * 8);
		if( Source == PBoxL ){
			Graphics::TBitmap *pBmp = CreateBitmap(32, 32, pf24bit);
			Graphics::TBitmap *pBmp2 = CreateBitmap(32, 32, pf24bit);
			pBmp2->Canvas->Draw(-index*32, 0, m_pListBmp);
			pBmp->Canvas->Draw(-m_MIndex*32, 0, m_pListBmp);
			m_pListBmp->Canvas->Draw(index*32, 0, pBmp);
			m_pListBmp->Canvas->Draw(m_MIndex*32, 0, pBmp2);
            delete pBmp2;
	        delete pBmp;
        }
        else {
			m_pListBmp->Canvas->Draw(index*32, 0, m_pUserBmp);
        }
        PBoxUPaint(NULL);
		PBoxLPaint(NULL);
        m_pListBmp->Modified = TRUE;
	}
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxUDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
	Accept = ((Source == PBoxL) && (m_MIndex >= 0));
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::PBoxUDragDrop(TObject *Sender,
      TObject *Source, int X, int Y)
{
    if( m_MIndex >= 0 ){
		m_pUserBmp->Canvas->Draw(-m_MIndex*32, 0, m_pListBmp);
		Graphics::TBitmap *pBmp = CreateBitmap(32, 32, pf24bit);
        FillBitmap(pBmp, clBlack);
		m_pListBmp->Canvas->Draw(m_MIndex*32, 0, pBmp);
        delete pBmp;
        PBoxUPaint(NULL);
		PBoxLPaint(NULL);
    }
}
//---------------------------------------------------------------------------
void __fastcall TBitMaskDlg::UDLClick(TObject *Sender, TUDBtnType Button)
{
	PBoxLPaint(NULL);
}
//---------------------------------------------------------------------------

void __fastcall TBitMaskDlg::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
	TShiftState sf1, sf2;
	sf1 << ssShift;
	sf2 << ssShift;
	sf1 *= Shift;
	int sft = (sf1 == sf2);

	DrawCursor();
	int x = m_X;
    int y = m_Y;
    int xw = m_XW;
    int yw = m_YW;
	switch(Key){
		case VK_LEFT:
			if( sft ){
				if( m_XW >= 2 ) m_XW--;
            }
            else {
				if( m_X ) m_X--;
            }
			Key = 0;
           	break;
        case VK_RIGHT:
			if( sft ){
				m_XW++;
            }
            else {
				m_X++;
            }
			Key = 0;
           	break;
        case VK_UP:
			if( sft ){
				if( m_YW >= 2 ) m_YW--;
            }
            else {
				if( m_Y ) m_Y--;
            }
			Key = 0;
           	break;
        case VK_DOWN:
			if( sft ){
				m_YW++;
            }
            else {
				m_Y++;
            }
			Key = 0;
           	break;
    }
	DrawCursor();
    if( (x != m_X) || (y != m_Y) || (xw != m_XW) || (yw != m_YW) ){
		UpdateCBSize(m_XW, m_YW);
    	UpdateText();
        UpdateUser();
    }
}
//---------------------------------------------------------------------------

