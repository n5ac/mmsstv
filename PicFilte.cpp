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

#include "Main.h"
#include "ComLIb.h"
#include "PicFilte.h"
#include "math.h"
#ifndef PI
#define	PI	3.1415926535897932384626433832795
#endif
#ifndef PI2
#define PI2 (PI*2)
#endif
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//---------------------------------------------------------------------
__fastcall TPicFilterDlg::TPicFilterDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	m_DisEvent = 1;
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		CancelBtn->Caption = "Cancel";
		L1->Caption = "Brightness";
		L2->Caption = "Contrast";
		L3->Caption = "Sharpness";
		L4->Caption = "Hue";
		L5->Caption = "Saturation";
		GBFilt->Caption = "Noise filter";
		SBST->Caption = "Stretch";
		AutoBtn->Caption = "Auto";
		ResBtn->Caption = "Reset";
        SBEdit->Caption = "Edit";
		SBBRR->Hint = "Reset";
		SBCTR->Hint = "Reset";
		SBSPR->Hint = "Reset";
		SBCPR->Hint = "Reset";
		SBCOR->Hint = "Reset";
	}
	else {
		SBBRR->Hint = "‰ðœ";
		SBCTR->Hint = "‰ðœ";
		SBSPR->Hint = "‰ðœ";
		SBCPR->Hint = "‰ðœ";
		SBCOR->Hint = "‰ðœ";
	}
	m_XW = 320;
	m_YW = 256;
	MultProc();
	pBitmap = new Graphics::TBitmap();
	pBitmap->PixelFormat = pf24bit;
	pBitmap->Width = 320;
	pBitmap->Height = 256;

	pBitmapS = new Graphics::TBitmap();
	pBitmapS->PixelFormat = pf24bit;
	pBitmapS->Width = 320;
	pBitmapS->Height = 256;
	pBitmapD = new Graphics::TBitmap();
	pBitmapD->PixelFormat = pf24bit;
	pBitmapD->Width = 320;
	pBitmapD->Height = 256;

	pH = NULL;
	pS = NULL;
	m_Make = 0;
	m_Sync = 0;
	MultProc();
}
//---------------------------------------------------------------------
__fastcall TPicFilterDlg::~TPicFilterDlg()
{
	delete pBitmap;
	delete pBitmapD;
	delete pBitmapS;
	if( pS != NULL ){
		delete pS;
		delete pH;
		pS = NULL;
		pH = NULL;
	}
	MultProc();
    g_ExecPB.Cancel();
}
//---------------------------------------------------------------------
void __fastcall TPicFilterDlg::UpdateBtn(void)
{
	SBBRR->Enabled = (TBBright->Position != 10) ? TRUE : FALSE;
	SBCTR->Enabled = (TBCont->Position != 10) ? TRUE : FALSE;
	SBSPR->Enabled = (TBShape->Position != 10) ? TRUE : FALSE;
	SBCPR->Enabled = (TBH->Position != 10) ? TRUE : FALSE;
	SBCOR->Enabled = (TBS->Position != 10) ? TRUE : FALSE;
	ResBtn->Enabled = SBBRR->Enabled || SBCTR->Enabled || SBSPR->Enabled || SBCPR->Enabled || SBCOR->Enabled || !SBFN->Down;
	if( !OKBtn->Enabled ){
		Timer->Interval = ResBtn->Enabled ? 5000 : 1500;
	}
}
//---------------------------------------------------------------------
int __fastcall TPicFilterDlg::Execute(Graphics::TBitmap *pBmp)
{
	CWaitCursor w;
	MultProc();
	m_XW = pBmp->Width;
	m_YW = pBmp->Height;
	char bf[128];
	sprintf(bf, "%s - Source:%ux%u", MsgEng ? "Image adjustment" : "‰æ‘œ’²®", m_XW, m_YW);
	Caption = bf;
	pBitmap->Width = m_XW;
	pBitmap->Height = m_YW;
	pBitmapS->Width = m_XW;
	pBitmapS->Height = m_YW;
	pBitmapD->Width = m_XW;
	pBitmapD->Height = m_YW;
	pBitmap->Canvas->Draw(0, 0, pBmp);
	pBitmapS->Canvas->Draw(0, 0, pBmp);
	pBitmapD->Canvas->Draw(0, 0, pBmp);
	CB2D->Checked = sys.m_Sharp2D;
	if( (m_XW <= 320) && (m_YW <= 256) ){
		SBST->Visible = FALSE;
	}
	UpdateBtn();
	w.Delete();
	m_DisEvent = 0;
	MultProc();
	int r = ShowModal();
	MultProc();
	sys.m_Sharp2D = CB2D->Checked;
	if( r == IDOK ){
		pBmp->Canvas->Draw(0, 0, pBitmapD);
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------
void __fastcall TPicFilterDlg::PBoxPaint(TObject *Sender)
{
	if( ((m_XW <= 320) && (m_YW <= 256)) ){
		PBox->Canvas->Draw(0, 0, pBitmapD);
	}
	else if( SBST->Down ){
		TRect tc;
		tc.Left = 0; tc.Top = 0;
		double sx = m_XW / 320.0;
		double sy = m_YW / 256.0;
		if( sx > sy ){
			tc.Right = 320;
			tc.Bottom = m_YW / sx;
		}
		else {
			tc.Bottom = 256;
			tc.Right = m_XW / sy;
		}
		::SetStretchBltMode(PBox->Canvas->Handle, HALFTONE);
		PBox->Canvas->StretchDraw(tc, pBitmapD);
	}
	else {
		int x = (320 - m_XW) / 2;
		int y = (256 - m_YW) / 2;
		PBox->Canvas->Draw(x, y, pBitmapD);
	}
}
//---------------------------------------------------------------------
void __fastcall TPicFilterDlg::Filt(int sw)
{
	MFilter(pBitmapS, pBitmap, sw);
}
//---------------------------------------------------------------------
void __fastcall TPicFilterDlg::AdjustColor(int x, int y, double &RY, double &BY)
{
	double H, S;

	if( pS == NULL ){
		pS = new double[(m_XW + 1) * (m_YW + 1)];
		pH = new double[(m_XW + 1) * (m_YW + 1)];
		m_Make = 1;
	}
	if( (RY != 0.0) || (BY != 0.0) ){
		if( TBS->Position ){
			if( m_Make ){
				S = sqrt(RY*RY + BY*BY);
				H = atan2(RY, BY);
				pS[y*m_XW+x] = S;
				pH[y*m_XW+x] = H;
			}
			else {
				S = pS[y*m_XW+x];
				H = pH[y*m_XW+x];
			}

			H += (TBH->Position - 10) * PI2 * 0.01;
			S *= 1.0 + (TBS->Position - 10) * 0.1;

			RY = S * sin(H);
			BY = S * cos(H);
		}
		else {
			RY = BY = 0;
		}
	}
	else {
		pS[y*m_XW+x] = 0;
		pH[y*m_XW+x] = 0;
	}
}
//---------------------------------------------------------------------
void __fastcall TPicFilterDlg::Bright(void)
{
	MultProc();

	int y, x;
	double B, G, R, Y, RY, BY;
	double k1, k2;
	if( TBCont->Position > 10 ){
		k1 = double(1.0 + (TBCont->Position - 10)*0.1);
	}
	else {
		k1 = double(1.0 + (TBCont->Position - 10)*0.05);
	}
	if( TBBright->Position > 10 ){
		k2 = double((TBBright->Position - 10) * 6.0);
	}
	else {
		k2 = double((TBBright->Position - 10) * 2.0);
	}
	BYTE *sp;
	BYTE *tp;
	if( TBShape->Position == 10 ){
		for( y = 0; y < m_YW; y++ ){
			sp = (BYTE*)pBitmapS->ScanLine[y];
			tp = (BYTE*)pBitmapD->ScanLine[y];
			for( x = 0; x < m_XW; x++ ){
				B = *sp++;
				G = *sp++;
				R = *sp++;
				Y = 0.3 * R + 0.59 * G + 0.11 * B;
				RY = 0.7 * R - 0.59 * G - 0.11 * B;
				BY = -0.3 * R - 0.59 * G + 0.89 * B;

				if( (TBH->Position != 10) || (TBS->Position != 10) ){
					AdjustColor(x, y, RY, BY);
				}

				Y = ((Y - 128) * k1) + 128;
				Y += k2;

				R = Y + RY;
				G = Y - 0.3 * RY / 0.59 - 0.11 * BY / 0.59;
				B = Y + BY;
				if( R > 255 ) R = 255;
				if( R < 0 ) R = 0;
				if( G > 255 ) G = 255;
				if( G < 0 ) G = 0;
				if( B > 255 ) B = 255;
				if( B < 0 ) B = 0;
				*tp++ = B;
				*tp++ = G;
				*tp++ = R;
			}
			MultProcA();
		}
	}
	else {
		double a, d;
		double Z1, Z2, Z3;
		if( CB2D->Checked ){
			a = TBShape->Position - 10;
			if( a > 0 ){
				a *= 0.2;
			}
			else {
				a *= 0.1;
			}
			double *ZV0 = new double[m_XW+1];
			double *ZV1 = new double[m_XW+1];
			double *ZV2 = new double[m_XW+1];
			memset(ZV0, 0, sizeof(double) * (m_XW+1));
			memset(ZV1, 0, sizeof(double) * (m_XW+1));
			memset(ZV2, 0, sizeof(double) * (m_XW+1));
			for( y = 0; y < (m_YW+1); y++ ){
				sp = (BYTE*)pBitmapS->ScanLine[y < m_YW ?  y : y - 1];
				tp = (BYTE*)pBitmapD->ScanLine[y ? y - 1 : y];
				Z1 = Z2 = Z3 = 0;
				for( x = 0; x < (m_XW+1); x++ ){
					if( x < m_XW ){
						B = *sp++;
						G = *sp++;
						R = *sp++;
					}
					else {
						sp -= 3;
						B = *sp++;
						G = *sp++;
						R = *sp++;
					}
					Y = 0.3 * R + 0.59 * G + 0.11 * B;
					RY = 0.7 * R - 0.59 * G - 0.11 * B;
					BY = -0.3 * R - 0.59 * G + 0.89 * B;

					if( (TBH->Position != 10) || (TBS->Position != 10) ){
						AdjustColor(x, y, RY, BY);
					}

					Y = ((Y - 128) * k1) + 128;
					Y += k2;

					d = Z3 + ((Y * -0.5 + Z1 + Z2 * -0.5) * a);
					Z2 = Z1;
					Z3 = Z1 = Y;
					Y = d;

					d = ZV2[x] + ((Y * -0.5 + ZV0[x] + ZV1[x] * -0.5) * a);
					ZV1[x] = ZV0[x];
					ZV2[x] = ZV0[x] = Y;
					Y = d;

					R = Y + RY;
					G = Y - 0.3 * RY / 0.59 - 0.11 * BY / 0.59;
					B = Y + BY;
					if( R > 255 ) R = 255;
					if( R < 0 ) R = 0;
					if( G > 255 ) G = 255;
					if( G < 0 ) G = 0;
					if( B > 255 ) B = 255;
					if( B < 0 ) B = 0;
					if( (x > 0) && (y > 0) ){
						*tp++ = B;
						*tp++ = G;
						*tp++ = R;
					}
				}
				MultProcA();
			}
			delete ZV0;
			delete ZV1;
			delete ZV2;
		}
		else {
			a = TBShape->Position - 10;
			if( a > 0 ){
				a *= 0.3;
			}
			else {
				a *= 0.1;
			}
			for( y = 0; y < m_YW; y++ ){
				sp = (BYTE*)pBitmapS->ScanLine[y];
				tp = (BYTE*)pBitmapD->ScanLine[y];
				Z1 = Z2 = Z3 = 0;
				for( x = 0; x < (m_XW+1); x++ ){
					if( x < m_XW ){
						B = *sp++;
						G = *sp++;
						R = *sp++;
					}
					else {
						sp -= 3;
						B = *sp++;
						G = *sp++;
						R = *sp++;
					}
					Y = 0.3 * R + 0.59 * G + 0.11 * B;
					RY = 0.7 * R - 0.59 * G - 0.11 * B;
					BY = -0.3 * R - 0.59 * G + 0.89 * B;

					if( (TBH->Position != 10) || (TBS->Position != 10) ){
						AdjustColor(x, y, RY, BY);
					}

					Y = ((Y - 128) * k1) + 128;
					Y += k2;

					d = Z3 + ((Y * -0.5 + Z1 + Z2 * -0.5) * a);
					Z2 = Z1;
					Z3 = Z1 = Y;
					Y = d;

					R = Y + RY;
					G = Y - 0.3 * RY / 0.59 - 0.11 * BY / 0.59;
					B = Y + BY;
					if( R > 255 ) R = 255;
					if( R < 0 ) R = 0;
					if( G > 255 ) G = 255;
					if( G < 0 ) G = 0;
					if( B > 255 ) B = 255;
					if( B < 0 ) B = 0;
					if( x > 0 ){
						*tp++ = B;
						*tp++ = G;
						*tp++ = R;
					}
				}
				MultProcA();
			}
		}
	}
	m_Make = 0;
}
//---------------------------------------------------------------------------
void __fastcall TPicFilterDlg::TBBrightChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	Bright();
	PBoxPaint(NULL);
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TPicFilterDlg::SBFNClick(TObject *Sender)
{
	MultProc();
	CWaitCursor w;

	pBitmapS->Canvas->Draw(0, 0, pBitmap);
	if( SBFA->Down ){
		Filt(0);
	}
	else if( SBFB->Down ){
		Filt(1);
	}
	if( pS != NULL ){
		delete pS;
		delete pH;
		pS = NULL;
		pH = NULL;
	}
	TBBrightChange(NULL);
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TPicFilterDlg::ResBtnClick(TObject *Sender)
{
	m_DisEvent++;
	TBBright->Position = 10;
	TBCont->Position = 10;
	TBShape->Position = 10;
	TBH->Position = 10;
	TBS->Position = 10;
	SBFN->Down = TRUE;
	if( pS != NULL ){
		delete pS;
		delete pH;
		pS = NULL;
		pH = NULL;
	}
	m_DisEvent--;
	if( !OKBtn->Enabled ) ::CopyBitmap(pBitmap, Mmsstv->pBitmapRX);
	pBitmapS->Canvas->Draw(0, 0, pBitmap);
	pBitmapD->Canvas->Draw(0, 0, pBitmap);
	PBox->Invalidate();
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TPicFilterDlg::SBBRRClick(TObject *Sender)
{
	if( TBBright->Position != 10 ){
		m_DisEvent++;
		TBBright->Position = 10;
		Bright();
		PBox->Invalidate();
		UpdateBtn();
		m_DisEvent--;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicFilterDlg::SBCTRClick(TObject *Sender)
{
	if( TBCont->Position != 10 ){
		m_DisEvent++;
		TBCont->Position = 10;
		Bright();
		PBox->Invalidate();
		UpdateBtn();
		m_DisEvent--;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicFilterDlg::SBSPRClick(TObject *Sender)
{
	if( TBShape->Position != 10 ){
		m_DisEvent++;
		TBShape->Position = 10;
		Bright();
		PBox->Invalidate();
		UpdateBtn();
		m_DisEvent--;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicFilterDlg::SBCPRClick(TObject *Sender)
{
	if( TBH->Position != 10 ){
		m_DisEvent++;
		TBH->Position = 10;
		Bright();
		PBox->Invalidate();
		UpdateBtn();
		m_DisEvent--;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicFilterDlg::SBCORClick(TObject *Sender)
{
	if( TBS->Position != 10 ){
		m_DisEvent++;
		TBS->Position = 10;
		Bright();
		PBox->Invalidate();
		UpdateBtn();
		m_DisEvent--;
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicFilterDlg::SBSTClick(TObject *Sender)
{
	PBox->Invalidate();
}
//---------------------------------------------------------------------------#if 0
void __fastcall TPicFilterDlg::AutoBtnClick(TObject *Sender)
{
	MultProc();
	CWaitCursor w;

	int B, G, R, Y, BY, RY;
	int x, y;
	BYTE *sp;

	int Hind[256];
	memset(Hind, 0, sizeof(Hind));
	int S = 0;
	for( y = 0; y < m_YW; y++ ){
		sp = (BYTE*)pBitmapS->ScanLine[y];
		for( x = 0; x < m_XW; x++ ){
			B = *sp++;
			G = *sp++;
			R = *sp++;
			Y = Limit256(0.3 * R + 0.59 * G + 0.11 * B);
			Hind[Y]++;
			RY = 0.7 * R - 0.59 * G - 0.11 * B;
			BY = -0.3 * R - 0.59 * G + 0.89 * B;
			Y = sqrt(RY*RY + BY*BY);
			if( Y > S ) S = Y;
		}
		MultProcA();
	}
#if 0
	int max = 0;
	for( x = 0; x < 256; x++ ){
		if( max < Hind[x] ){
			max = Hind[x];
		}
	}
	FILE *fp = fopen("F:\\TEST.TXT", "wt");
	for( x = 0; x < 256; x++ ){
		int e = Hind[x] * 65 / max;
		for( int j = 0; j < e; j++ ) fprintf(fp, "*");
		fprintf(fp, "(%d)\n", Hind[x]);
	}
	fclose(fp);
#endif
	int lmt = m_XW * m_YW / 2560;
	int ml = 256;
	int mh = 0;
	for( x = 0; x < 256; x++ ){
		if( Hind[x] > lmt ){
			ml = x;
			break;
		}
	}
	for( x = 255; x >= 0; x-- ){
		if( Hind[x] > lmt ){
			mh = x;
			break;
		}
	}
	if( mh > ml ){
		m_DisEvent++;
		int b = 128 - (mh + ml)/2;
		double c = 230.4 / (mh - ml);
		if( c < 1.0 ) c = 1.0;
//        c = c - 1.0 - (b / 128.0);
		c = c - 1.0;
		c = c * (c > 1.0 ? 10 : 20);
		b = b / (b > 0 ? 6 : 2);
		TBBright->Position = b + 10;
		TBCont->Position = c + 10;
		TBShape->Position = 10;
		if( S ){
			S = (((256.0 / double(S)) - 1.0) * 2.0) + 10;
		}
		if( S < 10 ) S = 10;
		TBS->Position = S;
		TBH->Position = 10;
		m_DisEvent--;
		TBBrightChange(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicFilterDlg::TimerTimer(TObject *Sender)
{
	if( !OKBtn->Enabled && (m_Sync || Mmsstv->pDem->m_Sync) ){
		if( Mmsstv->pSound->IsBusy() ) return;
		m_Sync = Mmsstv->pDem->m_Sync;

		if( pS != NULL ){
			delete pS;
			delete pH;
			pS = NULL;
			pH = NULL;
		}
		::CopyBitmap(pBitmap, Mmsstv->pBitmapRX);
		MultProcA();
		if( ResBtn->Enabled ){
			CWaitCursor w;
			if( !SBFN->Down ){
				SBFNClick(NULL);
			}
			else {
				pBitmapS->Canvas->Draw(0, 0, pBitmap);
				TBBrightChange(NULL);
			}
		}
		else {
			pBitmapS->Canvas->Draw(0, 0, pBitmap);
			pBitmapD->Canvas->Draw(0, 0, pBitmap);
			PBoxPaint(NULL);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TPicFilterDlg::SBEditClick(TObject *Sender)
{
	g_ExecPB.Exec(pBitmapD, (HWND)Handle, CM_EDITEXIT, 0);
}
//---------------------------------------------------------------------------
void __fastcall TPicFilterDlg::OnEditExit(TMessage Message)
{
	if( Message.LParam ) return;

	if( g_ExecPB.LoadBitmap(pBitmap, FALSE) ){
		ResBtnClick(NULL);
    }
}
//---------------------------------------------------------------------------
