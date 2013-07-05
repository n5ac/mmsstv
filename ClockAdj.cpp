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

#include "ClockAdj.h"
#include "Main.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TClockAdjDlg *ClockAdjDlg;
static int	s_Gain = 0x0108;
//---------------------------------------------------------------------
__fastcall TClockAdjDlg::TClockAdjDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	EntryAlignControl();

	pBitmap = new Graphics::TBitmap();
	if( sys.m_Palette ) pBitmap->PixelFormat = pf16bit;
	pBitmap->Width = PaintBox->Width;
	pBitmap->Height = PaintBox->Height;


	pDem = NULL;
	pTick = NULL;
	m_agcMax = 8192;
	m_agcMin = 0;
	m_agcSumMax = 8192 * AGCAVG;
	m_Point = 0;
	m_Samp = 11025;
	m_rBase = 0;
	m_MaxP = 0;
	m_MaxD = 0;
}
//---------------------------------------------------------------------
__fastcall TClockAdjDlg::~TClockAdjDlg()
{
	if( pTick != NULL ){
		delete pTick;
		pTick = NULL;
	}
	pDem = NULL;
	pBitmap->Palette = NULL;
	delete pBitmap;
	pBitmap = NULL;
}
//---------------------------------------------------------------------------
// サイズ変更コントロールの登録
void __fastcall TClockAdjDlg::EntryAlignControl(void)
{
	RECT	rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = ClientWidth-1;
	rc.bottom = ClientHeight-1;

	AlignList.EntryControl(OKBtn, &rc, OKBtn->Font);
	AlignList.EntryControl(CancelBtn, &rc, CancelBtn->Font);
	AlignList.EntryControl(Panel, &rc, NULL);
	AlignList.EntryControl(TBGAIN, &rc, NULL);
	AlignList.EntryControl(SBAGC, &rc, SBAGC->Font);
	AlignList.EntryControl(MarkFreq, &rc, MarkFreq->Font);
	AlignList.EntryControl(UDMark, &rc, NULL);
	AlignList.EntryControl(Label2, &rc, Label2->Font);
	AlignList.EntryControl(LPPM, &rc, LPPM->Font);
	AlignList.EntryControl(EditClock, &rc, EditClock->Font);
	AlignList.EntryControl(UDSamp, &rc, NULL);
	AlignList.EntryControl(Label1, &rc, Label1->Font);

//	int CX = ::GetSystemMetrics(SM_CXFULLSCREEN);
//	int CY = ::GetSystemMetrics(SM_CYFULLSCREEN);
	int CX = ::GetSystemMetrics(SM_CXSCREEN);
	int CY = ::GetSystemMetrics(SM_CYSCREEN);
	if( (CX < Width)||(CY < Height) ){
		Top = 0;
		Left = 0;
		Width = CX;
		Height = CY;
	}
#if 0
	else {
		Top = 0;
		Left = 0;
		Width = 600;
		Height = 400;
	}
#endif
	FormCenter(this, CX, CY);
	if( Owner != NULL ){
		WindowState = ((TForm *)Owner)->WindowState;
	}
	m_rBase = 0;
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::FormResize(TObject *Sender)
{
	CWaitCursor tw;
	AlignList.NewAlign(this);
	if( pBitmap != NULL ){
		delete pBitmap;
	}
	pBitmap = new Graphics::TBitmap();
	if( sys.m_Palette ) pBitmap->PixelFormat = pf16bit;
	pBitmap->Width = PaintBox->Width;
	pBitmap->Height = PaintBox->Height;

	TRect rc;
	int XL = 0;
	int XR = pBitmap->Width - 1;
	int YT = 0;
	int YB = pBitmap->Height - 1;
	rc.Left = XL;
	rc.Right = XR;
	rc.Top = YT;
	rc.Bottom = YB+1;
	TCanvas *cp = pBitmap->Canvas;
	cp->Brush->Color = clBlack;
	cp->FillRect(rc);

	m_CursorX = XR - 20;

	cp->Font->Name = Font->Name;
	cp->Font->Charset = Font->Charset;
	cp->Font->Color = clWhite;
	cp->Font->Size = 10;
	int FH = cp->TextHeight("A");
	int Y = 5;
	if( MsgEng ){
		Caption = "Calibrating the Sound Card with a Time Standard Broadcast Station";
//        "Adjust Sampling Frequency using an off-air time signal";
//		Caption = "Adjust Sampling freq. (Recive WWV tick sound on Mark freq.)";
		CancelBtn->Caption = "Cancel";

		cp->TextOut(10, Y, "1) Receive standard radio wave (e.g., WWV, BPM)."); Y+=FH;
		cp->TextOut(10, Y, "2) Tune into the tick sound."); Y+=FH;
		cp->TextOut(10, Y, "3) Continue listening to the sound for a while.  You have a vertical line."); Y+=FH;
		cp->TextOut(10, Y, "4) Click the lower point of the line."); Y+=FH;
		cp->TextOut(10, Y, "5) Click the upper point of the line."); Y+=FH;
		Y+=FH;
		cp->TextOut(10, Y, "You could use FAX broadcasting instead of WWV or JJY, but be sure it has"); Y+=FH;
		cp->TextOut(10, Y, "exact timing.");
	}
	else {
		cp->TextOut(10, Y, "1.BPMをAMまたはSSBで受信し、この画面の下にあるToneに1000Hzまたは"); Y+=FH;
		cp->TextOut(10, Y, "  1600Hzを設定します."); Y+=FH;
		cp->TextOut(10, Y, "2.SSBの場合は１秒チック音がToneに重なるように受信周波数を調整します."); Y+=FH;
		cp->TextOut(10, Y, "3.しばらく受信して縦または斜めの帯線が表示されるのを待ちます（数分）."); Y+=FH;
		cp->TextOut(10, Y, "4.表示された帯線の下の端点をクリックします."); Y+=FH;
		cp->TextOut(10, Y, "5.更に同じ帯線の上の端点をクリックするとクロック値が自動的に"); Y+=FH;
		cp->TextOut(10, Y, "  設定されます.（2点間の距離が長いほうが正確です）"); Y+=FH;
		cp->TextOut(10, Y, "傾斜が非常に大きい場合、上記の手続きをもう一度繰り返して下さい."); Y+=FH+FH;
		cp->TextOut(10, Y, "BPMの代わりに、JMH,JMGなどのFAX放送の0.5秒毎のデッドセクタを利用しても"); Y+=FH;
		cp->TextOut(10, Y, "OKのようです."); Y+=FH;
		cp->TextOut(10, Y, "左クリック - 調整の開始"); Y+=FH;
		cp->TextOut(10, Y, "右クリック - 垂直カーソル移動");
	}
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::UpdateTB(void)
{
	TBGAIN->Enabled = !SBAGC->Down;
	LG->Font->Color = SBAGC->Down ? clGrayText : clBlack;
}
//---------------------------------------------------------------------
void __fastcall TClockAdjDlg::UpdatePPM(void)
{
	double dd = ((m_Samp - SampBase) * 1000000.0 / SampBase);

	char bf[64];
	sprintf(bf, "%d ppm", int(dd+0.5));
	LPPM->Caption = bf;
}
//---------------------------------------------------------------------
int __fastcall TClockAdjDlg::Execute(TSound *p, double &Samp)
{
	pSound = p;
	pDem = &p->SSTVDEM;
	pTick = new CTICK;
	pDem->pTick = pTick;
	m_Samp = Samp;
	pTick->m_Samp = int(Samp+0.5);
	InitColorTable(clBlack, clWhite);
	char bf[128];
	sprintf(bf, "%.2lf", m_Samp);
	EditClock->Text = bf;
	pTick->Init();
	pDem->m_Tick = 1;
	m_DisEvent++;
	UDMark->Position = 1000;
	pDem->SetTickFreq(1000);
	TBGAIN->Position = s_Gain & 0x00ff;
	SBAGC->Down = s_Gain & 0xff00 ? 1 : 0;
	UpdateTB();
	m_DisEvent--;
	UpdatePPM();
	Mmsstv->PBoxG->Invalidate();
	int r = ShowModal();
	pDem->m_Tick = 0;
	pDem->pTick = NULL;
	InitColorTable(sys.m_ColorLow, sys.m_ColorHigh);
	s_Gain = (s_Gain & 0x00ff) | (SBAGC->Down ? 0x0100 : 0);
	delete pTick;
	pTick = NULL;
	pDem->SetTickFreq(0);
	Mmsstv->PBoxG->Invalidate();
	if( r == IDOK ){
		Samp = m_Samp;
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------
void __fastcall TClockAdjDlg::TimerTimer(TObject *Sender)
{
	if( pBitmap == NULL ) return;
	if( pDem == NULL ) return;
	if( pTick == NULL ) return;

	int *p = pTick->GetData();
	if( p == NULL ) return;

	TCanvas *tp = pBitmap->Canvas;

	int mx = -MAXINT;
	int mi = MAXINT;
	int x, xx, d;
	double k1 = 16384.0/double(m_agcMax - m_agcMin);
	double k2;
	if( SBAGC->Down ){
		k2 = 128.0 / 16384.0;
	}
	else {
		k2 = (128.0 * (s_Gain & 0x00ff)) / (4096 * 16);
	}
	int ax = -1;
	for( xx = 0; xx < pTick->m_Samp; xx++ ){
		int xv = xx + m_rBase;
		x = fmod(xv, m_Samp);
		x = x * pBitmap->Width / m_Samp;
		if( ax != x ){
			ax = x;
			if( !x ){
				if( m_MaxD ){
					tp->Pixels[m_MaxP][0] = clRed;
				}
				m_MaxD = 0;
				m_MaxP = 0;
				TRect rc;
				rc.Left = 0;
				rc.Right = pBitmap->Width - 1;
				rc.Top = 0;
				rc.Bottom = pBitmap->Height - 2;
				TRect src(rc);
				src.Bottom--;
				rc.Top++;
				tp->CopyRect(rc, tp, src);
			}
			d = p[xx];
			if( m_MaxD < d ){
				m_MaxD = d;
				m_MaxP = x;
			}
			if( mx < d ) mx = d;
			if( mi > d ) mi = d;
			if( SBAGC->Down ){
				d -= m_agcMin;
				if( m_agcMax > 0 ) d = double(d) * k1;
			}
			d = double(d) * k2;

			if( d >= 128 ) d = 127;
			if( d <= 0 ) d = 0;
			tp->Pixels[x][0] = TColor(ColorTable[127-d]);
		}
	}
	m_rBase += pTick->m_Samp;
#if 0
	for( x = 0; x < XR; x++){
		xx = (x * pTick->m_Samp)/pBitmap->Width;
		d = p[xx];
		if( mx < d ) mx = d;
		if( mi > d ) mi = d;
		if( SBAGC->Down ){
			d -= m_agcMin;
			if( m_agcMax > 0 ) d = double(d) * k1;
		}
		d = double(d) * k2;

		if( d >= 128 ) d = 127;
		if( d <= 0 ) d = 0;
		tp->Pixels[x][0] = TColor(ColorTable[127-d]);
	}
#endif
	m_agcSumMax -= m_agcMax;
	m_agcSumMax += mx;
	if( m_agcSumMax < (4096*AGCAVG) ) m_agcSumMax = (4096*AGCAVG);
	m_agcMax = m_agcSumMax / AGCAVG;
	m_agcMin = mi;
	if( m_agcMin > 2048 ) m_agcMin = 2048;
	m_PointY++;
	PaintBox->Canvas->Draw(0, 0, (TGraphic*)pBitmap);
	PaintCursor();
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::DrawMessage(LPCSTR p)
{
	int xr = PaintBox->Canvas->TextWidth(p);
	int xl = (PaintBox->Width - xr)/2;
	xr += xl;
	int FH = PaintBox->Canvas->TextHeight(p);
	int VC = PaintBox->Height - FH;
	PaintBox->Canvas->Pen->Color = clWhite;
	PaintBox->Canvas->Brush->Color = clBlack;
	PaintBox->Canvas->RoundRect(xl-10, VC-FH, xr+10, VC+FH, 10, 10);
	PaintBox->Canvas->Font->Color = clWhite;
	PaintBox->Canvas->TextOut(xl, VC-FH/2, p);
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::PaintCursor(void)
{
	PaintBox->Canvas->Pen->Color = clLime;
	PaintBox->Canvas->Pen->Style = psDot;
	PaintBox->Canvas->MoveTo(m_CursorX, 0);
	int rop = ::SetROP2(PaintBox->Canvas->Handle, R2_MASKPENNOT);
	PaintBox->Canvas->LineTo(m_CursorX, pBitmap->Height - 1);
	::SetROP2(PaintBox->Canvas->Handle, rop);
	if( m_Point ){
		PaintBox->Canvas->Pen->Color = clYellow;
		PaintBox->Canvas->Pen->Style = psSolid;
		PaintBox->Canvas->MoveTo(m_PointX, m_PointY);
		::SetROP2(PaintBox->Canvas->Handle, R2_MASKPENNOT);
		PaintBox->Canvas->LineTo(m_PointX2, m_PointY2);
		::SetROP2(PaintBox->Canvas->Handle, rop);
		char bf[256];
		if( MsgEng ){
			sprintf( bf, "OK : Left button,  Cancel : Right button   Clock=%.2lf", GetPointSamp());
		}
		else {
			sprintf( bf, "決定 : 左ボタン,  中止 : 右ボタン   Clock=%.2lf", GetPointSamp());
		}
		DrawMessage(bf);
		if( PaintBox->Cursor != crCross ) PaintBox->Cursor = crCross;
	}
	else {
		if( PaintBox->Cursor != crDefault ) PaintBox->Cursor = crDefault;
	}
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::PaintBoxPaint(TObject *Sender)
{
	PaintBox->Canvas->Draw(0, 0, (TGraphic*)pBitmap);
	PaintCursor();
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::PaintBoxMouseUp(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbLeft ){
		if( m_Point ){
			m_Point = 0;
			if( m_PointY2 != m_PointY ){
				m_Samp = GetPointSamp();
				ModalResult = mrOk;
			}
		}
		else {
			m_PointX2 = m_PointX = X;
			m_PointY2 = m_PointY = Y;
			m_Point = 1;
		}
	}
	else if( m_Point ){
		m_Point = 0;
	}
	else {
		m_CursorX = X;
	}
	PaintBox->Canvas->Draw(0, 0, (TGraphic*)pBitmap);
	PaintCursor();
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::MarkFreqChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	int	dd;
	if( sscanf(AnsiString(MarkFreq->Text).c_str(), "%lu", &dd ) == 1){	//ja7ude 0428
		if( (dd >= 300) && (dd <= 2700) ){
			m_DisEvent++;
			UDMark->Position = SHORT(dd);
			pDem->SetTickFreq(dd);
			Mmsstv->PBoxG->Invalidate();
			m_DisEvent--;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::TBGAINChange(TObject *Sender)
{
	s_Gain = (s_Gain & 0xff00) | TBGAIN->Position;
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::UDMarkClick(TObject *Sender, TUDBtnType Button)
{
	if( m_DisEvent ) return;

	pDem->SetTickFreq(UDMark->Position);
	Mmsstv->PBoxG->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::SBAGCClick(TObject *Sender)
{
	UpdateTB();
}
//---------------------------------------------------------------------------
double __fastcall TClockAdjDlg::GetPointSamp(void)
{
	if( m_PointY == m_PointY2 ) return m_Samp;
	double d = (m_PointX2 - m_PointX);
	d = d * m_Samp / pBitmap->Width;
	d /= (m_PointY - m_PointY2);
	d += m_Samp;
	d = NormalSampFreq(d, 100);
	return d;
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::PaintBoxMouseMove(TObject *Sender,
	TShiftState Shift, int X, int Y)
{
	if( m_Point ){
		m_PointX2 = X;
		m_PointY2 = Y;
		PaintBox->Canvas->Draw(0, 0, (TGraphic*)pBitmap);
		PaintCursor();
	}
}
//---------------------------------------------------------------------------
void __fastcall TClockAdjDlg::UDSampClick(TObject *Sender,
	  TUDBtnType Button)
{
	if( Button == btNext ){
		m_Samp += 0.02;
	}
	else {
		m_Samp -= 0.02;
	}
	m_Samp = NormalSampFreq(m_Samp, 50);
	m_rBase += (1.0 - (m_Samp / pTick->m_Samp)) * pTick->m_Samp;
	char bf[128];
	sprintf(bf, "%.2lf", m_Samp);
	EditClock->Text = bf;
	UpdatePPM();
}
//---------------------------------------------------------------------------

