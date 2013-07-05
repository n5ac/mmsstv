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
#include <vcl.h>	//ja7ude 0521
#pragma hdrstop

#include <math.h>
#include "fir.h"

//-------------------------------------------------
// ＦＩＲフィルタのたたき込み演算
double __fastcall DoFIR(double *hp, double *zp, double d, int tap)
{
	memcpy(zp, &zp[1], sizeof(double)*tap);
	zp[tap] = d;
	d = 0.0;
	for( int i = 0; i <= tap; i++, hp++, zp++ ){
		d += (*zp) * (*hp);
	}
	return d;
}
//---------------------------------------------------------------------------
CIIRTANK::CIIRTANK()
{
	b1 = b2 = a0 = z1 = z2 = 0;
	SetFreq(2000.0, SampFreq, 50.0);
}
//---------------------------------------------------------------------------
void CIIRTANK::SetFreq(double f, double smp, double bw)
{
	double lb1, lb2, la0;
	lb1 = 2 * exp(-PI * bw/smp) * cos(2 * PI * f / smp);
	lb2 = -exp(-2*PI*bw/smp);
	if( bw ){
#if 0
		const double _gt[]={18.0, 26.0, 20.0, 20.0};
		la0 = sin(2 * PI * f/smp) / (_gt[SampType] * 50 / bw);
#else
		la0 = sin(2 * PI * f/smp) / ((smp/6.0) / bw);
#endif
	}
	else {
		la0 = sin(2 * PI * f/smp);
	}
	b1 = lb1; b2 = lb2; a0 = la0;
}
//---------------------------------------------------------------------------
double CIIRTANK::Do(double d)
{
	d *= a0;
	d += (z1 * b1);
	d += (z2 * b2);
	z2 = z1;
	if( fabs(d) < 1e-37 ) d = 0.0;
	z1 = d;
	return d;
}
//---------------------------------------------------------------------------
CLMS::CLMS()
{
	Z = new double[LMSTAP+1];
	H = new double[LMSTAP+1];
	D = new double[LMSTAP+1];
	memset(Z, 0, sizeof(double[LMSTAP+1]));
	memset(H, 0, sizeof(double[LMSTAP+1]));
	memset(D, 0, sizeof(double[LMSTAP+1]));

	m_D = 0;

	m_lmsADJSC = 1.0 / double(32768 * 32768);			// スケール調整値
	m_lmsErr = m_lmsMErr = 0;

	m_Tap = int((4 * SampBase/11025) + 0.5);
//	m_Tap = int((8 * SampBase/11025) + 0.5);
	if( m_Tap > LMSTAP ) m_Tap = LMSTAP;
	m_lmsMU2 = 0.003;			// LMS 2μ
	m_lmsGM = 0.9999;			// LMS γ

	m_Tap_N = int((48 * SampBase/11025) + 0.5);
	if( m_Tap_N > LMSTAP ) m_Tap_N = LMSTAP;
//	m_lmsMU2_N = 0.00018;			// LMS 2μ
//	m_lmsGM_N = 0.999999;			// LMS γ
	m_lmsMU2_N = 0.00018;			// LMS 2μ
	m_lmsGM_N = 0.999999;			// LMS γ
	m_lmsDelay_N = int((12 * SampBase/11025) + 0.5);
	if( m_lmsDelay_N > LMSTAP ) m_lmsDelay_N = LMSTAP;
}

CLMS::~CLMS()
{
	delete D;
	delete H;
	delete Z;
}

//-------------------------------------------------
// 適応フィルタの演算
double CLMS::Do(double d)
{
	double a = 0.0;
	int i;
	double *zp = Z;
	double *hp = H;

	// トランスバーサルフィルタ
	memcpy(Z, &Z[1], sizeof(double)*m_Tap);
#if 0
	Z[m_Tap] = D[0];
#else
	Z[m_Tap] = m_D;
#endif
	for( i = 0; i <= m_Tap; i++, zp++, hp++ ){
		a += (*zp) * (*hp);
	}
	// 誤差計算
	m_lmsErr = d - a;
	m_lmsMErr = m_lmsErr * m_lmsMU2 * m_lmsADJSC;	// lmsADJSC = 1/(32768 * 32768) スケーリング調整値

	// 遅延器の移動
#if 0
	if( m_lmsDelay ) memcpy(D, &D[1], sizeof(double)*m_lmsDelay);
	D[m_lmsDelay] = d;
#else
	m_D = d;
#endif
	// 係数更新
	zp = Z;
	hp = H;
	double sum = 0;
	for( i = 0; i <= m_Tap; i++, zp++, hp++ ){
		*hp = (m_lmsMErr * (*zp)) + (*hp * m_lmsGM);
		if( *hp >= 0 ){
			sum += *hp;
		}
		else {
			sum -= *hp;
		}
	}
	if( sum > 0 ) a /= sum;
	return a;
}

//-------------------------------------------------
// 適応フィルタの演算
double CLMS::DoN(double d)
{
	double a = 0.0;
	int i;
	double *zp = Z;
	double *hp = H;

	// トランスバーサルフィルタ
	memcpy(Z, &Z[1], sizeof(double)*m_Tap_N);
	Z[m_Tap_N] = D[0];
	for( i = 0; i <= m_Tap_N; i++, zp++, hp++ ){
		a += (*zp) * (*hp);
	}
	// 誤差計算
	m_lmsErr = d - a;
	m_lmsMErr = m_lmsErr * m_lmsMU2_N * m_lmsADJSC;	// lmsADJSC = 1/(32768 * 32768) スケーリング調整値

	// 遅延器の移動
	memcpy(D, &D[1], sizeof(double)*m_lmsDelay_N);
	D[m_lmsDelay_N] = d;

	// 係数更新
	zp = Z;
	hp = H;
	for( i = 0; i <= m_Tap_N; i++, zp++, hp++ ){
		*hp = (m_lmsMErr * (*zp)) + (*hp * m_lmsGM_N);
	}
	return m_lmsErr;
}

//-------------------------------------------------
// 適応フィルタの演算
void CLMS::SetAN(int sw)
{
	m_Tap_N = int((48 * SampBase/11025) + 0.5);
	if( m_Tap_N > LMSTAP ) m_Tap_N = LMSTAP;
	m_lmsDelay_N = int((12 * SampBase/11025) + 0.5);
	if( m_lmsDelay_N > LMSTAP ) m_lmsDelay_N = LMSTAP;
	memset(Z, 0, sizeof(double[LMSTAP+1]));
	memset(H, 0, sizeof(double[LMSTAP+1]));
	memset(D, 0, sizeof(double[LMSTAP+1]));
	switch(sw){
		case 1:
			m_lmsMU2_N = 0.00018;			// LMS 2μ
			m_lmsGM_N = 0.999998;			// LMS γ
			break;
		default:
			m_lmsMU2_N = 0.00005;			// LMS 2μ
			m_lmsGM_N = 0.9999985;			// LMS γ
			break;
	}
}

//-------------------------------------------------
// 相関計算の演算
int CLMS::Sig(double d)
{
	double a = 0.0;
	int i;
	double *zp = Z;
	double *hp = H;

	// トランスバーサルフィルタ
	memcpy(Z, &Z[1], sizeof(double)*m_Tap);
#if 0
	Z[m_Tap] = D[0];
#else
	Z[m_Tap] = m_D;
#endif
	for( i = 0; i <= m_Tap; i++, zp++, hp++ ){
		a += (*zp) * (*hp);
	}
	// 誤差計算
	m_lmsErr = d - a;
	m_lmsMErr = m_lmsErr * m_lmsMU2 * m_lmsADJSC;	// lmsADJSC = 1/(32768 * 32768) スケーリング調整値

	// 遅延器の移動
#if 0
	if( m_lmsDelay ) memcpy(D, &D[1], sizeof(double)*m_lmsDelay);
	D[m_lmsDelay] = d;
#else
	m_D = d;
#endif
	// 係数更新
	zp = Z;
	hp = H;
	double sum = 0;
	for( i = 0; i <= m_Tap; i++, zp++, hp++ ){
		*hp = (m_lmsMErr * (*zp)) + (*hp * m_lmsGM);
		if( *hp >= 0 ){
			sum += *hp;
		}
		else {
			sum -= *hp;
		}
	}
	return sum * 32768.0;
}

//---------------------------------------------------------------------------
// ノッチフィルタ
__fastcall CNotch::CNotch()
{
//	m_pZ = new double[NOTCHTAPMAX+1];
//	m_pH = new double[NOTCHTAPMAX+1];
//	memset(m_pZ, 0, sizeof(double)*NOTCHTAPMAX+1);
	m_tap = 96 * SampBase / 11025.0;
	if( m_tap & 1 ) m_tap++;
	if( m_tap >= NOTCHTAPMAX ) m_tap = NOTCHTAPMAX;
	SetNotchFreq(2400);
}

__fastcall CNotch::~CNotch()
{
//	delete m_pZ;
//	delete m_pH;
}

void __fastcall CNotch::SetNotchFreq(double fq)
{
	m_freq = fq;
	double fl, fh;
	if( (fq < 1050.0) || (fq > 2350) ){
		fl = fq - 30;
		fh = fq + 30;
	}
	else {
		fl = fq - 15;
		fh = fq + 15;
	}
    m_Notch.Create(m_tap, ffBEF, SampFreq, fl, fh, 10, 1.0);
//	MakeFilter(m_pH, m_tap, ffBEF, SampFreq, fl, fh, 10, 1.0);
}

double __fastcall CNotch::Do(double d)
{
	return m_Notch.Do(d);
//	return DoFIR( m_pH, m_pZ, d, m_tap);
}

/**********************************************************************
	ＦＩＲフィルタ設計関数
**********************************************************************/
/*
====================================================
	ベッセル関数
====================================================
*/
static double I0(double x)
{
	double	sum, xj;
	int		j;

	sum = 1.0;
	xj = 1.0;
	j = 1;
	while(1){
		xj *= ((0.5 * x) / (double)j);
		sum += (xj*xj);
		j++;
		if( ((0.00000001 * sum) - (xj*xj)) > 0 ) break;
	}
	return(sum);
}

/*
====================================================
	ＦＩＲフィルタの設計
====================================================
*/
void MakeFilter(double *HP, int tap, int type, double fs, double fcl, double fch, double att, double gain)
{
	FIR	fir;

	fir.typ = type;
	fir.n = tap;
	fir.fs = fs;
	fir.fcl = fcl;
	fir.fch = fch;
	fir.att = att;
	fir.gain = gain;
	MakeFilter(HP, &fir);
}

void MakeFilter(double *HP, FIR *fp)
{
	int		j, m;
	double	alpha, win, fm, w0, sum;
	double	*hp;

	if( fp->typ == ffHPF ){
		fp->fc = 0.5*fp->fs - fp->fcl;
	}
	else if( fp->typ != ffLPF ){
		fp->fc = (fp->fch - fp->fcl)/2.0;
	}
	else {
		fp->fc = fp->fcl;
	}
	if( fp->att >= 50.0 ){
		alpha = 0.1102 * (fp->att - 8.7);
	}
	else if( fp->att >= 21 ){
		alpha = (0.5842 * pow(fp->att - 21.0, 0.4)) + (0.07886 * (fp->att - 21.0));
	}
	else {
		alpha = 0.0;
	}

	hp = fp->hp;
	sum = PI*2.0*fp->fc/fp->fs;
	if( fp->att >= 21 ){		// インパルス応答と窓関数を計算
		for( j = 0; j <= (fp->n/2); j++, hp++ ){
			fm = (double)(2 * j)/(double)fp->n;
			win = I0(alpha * sqrt(1.0-(fm*fm)))/I0(alpha);
			if( !j ){
				*hp = fp->fc * 2.0/fp->fs;
			}
			else {
				*hp = (1.0/(PI*(double)j))*sin((double)j*sum)*win;
			}
		}
	}
	else {						// インパルス応答のみ計算
		for( j = 0; j <= (fp->n/2); j++, hp++ ){
			if( !j ){
				*hp = fp->fc * 2.0/fp->fs;
			}
			else {
				*hp = (1.0/(PI*(double)j))*sin((double)j*sum);
			}
		}
	}
	hp = fp->hp;
	sum = *hp++;
	for( j = 1; j <= (fp->n/2); j++, hp++ ) sum += 2.0 * (*hp);
	hp = fp->hp;
	if( sum > 0.0 ){
		for( j = 0; j <= (fp->n/2); j++, hp++ ) (*hp) /= sum;
	}

	// 周波数変換

	if( fp->typ == ffHPF ){
		hp = fp->hp;
		for( j = 0; j <= (fp->n/2); j++, hp++ ) (*hp) *= cos((double)j*PI);
	}
	else if( fp->typ != ffLPF ){
		w0 = PI * (fp->fcl + fp->fch) / fp->fs;
		if( fp->typ == ffBPF ){
			hp = fp->hp;
			for( j = 0; j <= (fp->n/2); j++, hp++ ) (*hp) *= 2.0*cos((double)j*w0);
		}
		else {
			hp = fp->hp;
			*hp = 1.0 - (2.0 * (*hp));
			for( hp++, j = 1; j <= (fp->n/2); j++, hp++ ) (*hp) *= -2.0*cos((double)j*w0);
		}
	}
	for( m = fp->n/2, hp = &fp->hp[m], j = m; j >= 0; j--, hp-- ){
		*HP++ = (*hp) * fp->gain;
	}
	for( hp = &fp->hp[1], j = 1; j <= (fp->n/2); j++, hp++ ){
		*HP++ = (*hp) * fp->gain;
	}
}

//---------------------------------------------------------------------------
//ＦＩＲフィルタ（ヒルベルト変換フィルタ）の設計
//
void MakeHilbert(double *H, int N, double fs, double fc1, double fc2)
{
	int L = N / 2;
    double T = 1.0 / fs;

			double W1 = 2 * PI * fc1;
			double W2 = 2 * PI * fc2;

			// 2*fc2*T*SA((n-L)*W2*T) - 2*fc1*T*SA((n-L)*W1*T)

			double w;
			int n;
			double x1, x2;
			for( n = 0; n <= N; n++ ){
				if( n == L ){
					x1 = x2 = 0.0;
				}
				else if( (n - L) ){
					x1 = ((n - L) * W1 * T);
					x1 = cos(x1) / x1;
					x2 = ((n - L) * W2 * T);
					x2 = cos(x2) / x2;
				}
				else {
					x1 = x2 = 1.0;
				}
				w = 0.54 - 0.46 * cos(2*PI*n/(N));
				H[n] = -(2 * fc2 * T * x2 - 2 * fc1 * T * x1) * w;
			}

			if( N < 8 ){
				w = 0;
    			for( n = 0; n <= N; n++ ){
					w += fabs(H[n]);
    			}
				if( w ){
					w = 1.0 / w;
					for( n = 0; n <= N; n++ ){
						H[n] *= w;
					}
    			}
			}
}

//---------------------------------------------------------------------
// 周波数特性グラフ（フィルタスレッド内でコールしてはいけない）
//
//	H(ejωT) = [Σ0]Hm cos(mωT) - j[Σ1]Hm sin(mωT)
//
void DrawGraph(Graphics::TBitmap *pBitmap, const double *H, int Tap, int Over, int &nmax, int init, TColor col)
{
	int		k, x, y;
	double	f, gdb, g, pi2t, fs;
	double	max;
	char	bf[80];

	TCanvas *tp = pBitmap->Canvas;
	TRect rc;
	rc.Left = 0;
	rc.Right = pBitmap->Width;
	rc.Top = 0;
	rc.Bottom = pBitmap->Height;
	if( init ){
		tp->Brush->Color = clWhite;
		tp->FillRect(rc);
	}
	int	LM;		// 周波数表示のあるライン数
	int DM;		// 内部線の数
	int MM;		// 実線の間隔
	switch(Over){
		case 2:
			max = 3000;
			fs = SampFreq/2.0;
			break;
		case 3:
			max = 2000;
			fs = SampFreq/3.0;
			break;
		default:
			max = 4000;
			fs = SampFreq;
			break;
	}
	if( nmax ){
		max = nmax;
	}
	else {
		nmax = max;
	}
	switch(nmax){
		case 3000:
			LM = 3;
			DM = 14;
			MM = 5;
			break;
		case 100:
		case 200:
		case 2000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		case 400:
		case 800:
		case 4000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		default:		// 6000
			LM = 3;
			DM = 11;
			MM = 4;
			break;
	}
	int XL = 32;
	int XR = pBitmap->Width - 16;
	int YT = 16;
	int YB = pBitmap->Height - 24;

	int i;
	if( init ){
		tp->Pen->Color = clBlack;
		tp->Font->Size = 8;
		tp->MoveTo(XL, YT); tp->LineTo(XR, YT); tp->LineTo(XR, YB); tp->LineTo(XL, YB); tp->LineTo(XL, YT);
		tp->Pen->Color = clGray;
		for( i = 0; i < 7; i++ ){
			tp->Pen->Style = (i & 1) ? psSolid : psDot;
			y = (int)(double(i + 1) * double(YB - YT)/8.0 + YT);
			tp->MoveTo(XL, y); tp->LineTo(XR, y);
		}
		for( i = 1; i < 5; i++ ){
			y = (int)(double(i) * double(YB - YT)/4.0 + YT);
			sprintf( bf, "-%2u", (80 / 4)*i );
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(XL - 6 - tp->TextWidth(bf), y - (tp->TextHeight(bf)/2), bf);
		}
		strcpy(bf, "dB");
		tp->TextOut(XL - 6 - tp->TextWidth(bf), YT-(tp->TextHeight(bf)/2), bf);
		for( i = 1; i <= DM; i++ ){
			tp->Pen->Style = (i % MM) ? psDot : psSolid;
			x = (int)(double(i) * double(XR - XL)/double(DM+1) + XL);
			tp->MoveTo(x, YT); tp->LineTo(x, YB);
		}
		for( i = 0; i <= LM; i++ ){
			x = (int)(double(i) * double(XR - XL)/double(LM) + XL);
			sprintf(bf, "%4.0lf", (max*i)/LM);
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(x - (tp->TextWidth(bf)/2), YB + 6, bf);
		}
		tp->Pen->Color = clRed;
		tp->Pen->Style = psDot;
		x = (int)(XL + (fs/2) * (double(XR-XL)/max));
		tp->MoveTo(x, YT); tp->LineTo(x, YB);

		tp->Pen->Color = clBlue;
		tp->Pen->Style = psSolid;
	}
	int ay = 0;
	double ra, im;
	pi2t = 2.0 * PI / fs;
	tp->Pen->Color = col;
	for( x = XL, f = 0.0; x < XR; x++, f += (max/double(XR-XL)) ){
		if( Tap ){
			ra = im = 0.0;
			for( k = 0; k <= Tap; k++ ){
				ra += H[k] * cos(pi2t*f*k);
				if( k ) im -= H[k] * sin(pi2t*f*k);
			}
			if( ra * im ){
				g = sqrt(ra * ra + im * im);
			}
			else {
				g = 0.0;
			}
		}
		else {
			g = 1.0;
		}
		if( g == 0 ) g = 1.0e-38;
		gdb = 20*0.4342944*log(fabs(g)) + 80.0;
		if( gdb < 0.0 ) gdb = 0.0;
		gdb = (gdb * double(YB-YT))/80.0;
		y = YB - (int)gdb;
		if( x == XL ){
			tp->MoveTo(x, y);
			tp->LineTo(x, y);
		}
		else {
			tp->MoveTo(x-1, ay);
			tp->LineTo(x, y);
		}
		ay = y;
	}
}


//---------------------------------------------------------------------
// 周波数特性グラフ（フィルタスレッド内でコールしてはいけない）
//
//
void DrawGraphIIR(Graphics::TBitmap *pBitmap, double a0, double a1, double a2, double b1, double b2, int Over, int &nmax, int init, TColor col)
{
	int		x, y;
	double	f, gdb, g, pi2t, pi4t, fs;
	double	max;
	char	bf[80];

	TCanvas *tp = pBitmap->Canvas;
	TRect rc;
	rc.Left = 0;
	rc.Right = pBitmap->Width;
	rc.Top = 0;
	rc.Bottom = pBitmap->Height;
	if( init ){
		tp->Brush->Color = clWhite;
		tp->FillRect(rc);
	}
	int	LM;		// 周波数表示のあるライン数
	int DM;		// 内部線の数
	int MM;		// 実線の間隔
	switch(Over){
		case 2:
			max = 3000;
			fs = SampFreq/2.0;
			break;
		case 3:
			max = 2000;
			fs = SampFreq/3.0;
			break;
		default:
			max = 4000;
			fs = SampFreq;
			break;
	}
	if( nmax ){
		max = nmax;
	}
	else {
		nmax = max;
	}
	switch(nmax){
		case 3000:
			LM = 3;
			DM = 14;
			MM = 5;
			break;
		case 100:
		case 200:
		case 2000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		case 400:
		case 800:
		case 4000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		default:		// 6000
			LM = 3;
			DM = 11;
			MM = 4;
			break;
	}
	int XL = 32;
	int XR = pBitmap->Width - 16;
	int YT = 16;
	int YB = pBitmap->Height - 24;

	int i;
	if( init ){
		tp->Pen->Color = clBlack;
		tp->Font->Size = 8;
		tp->MoveTo(XL, YT); tp->LineTo(XR, YT); tp->LineTo(XR, YB); tp->LineTo(XL, YB); tp->LineTo(XL, YT);
		tp->Pen->Color = clGray;
		for( i = 0; i < 5; i++ ){
			tp->Pen->Style = (i & 1) ? psSolid : psDot;
			y = (int)(double(i + 1) * double(YB - YT)/6.0 + YT);
			tp->MoveTo(XL, y); tp->LineTo(XR, y);
		}
		for( i = 1; i < 4; i++ ){
			y = (int)(double(i) * double(YB - YT)/3.0 + YT);
			sprintf( bf, "-%2u", (60 / 3)*i );
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(XL - 6 - tp->TextWidth(bf), y - (tp->TextHeight(bf)/2), bf);
		}
		strcpy(bf, "dB");
		tp->TextOut(XL - 6 - tp->TextWidth(bf), YT-(tp->TextHeight(bf)/2), bf);
		for( i = 1; i <= DM; i++ ){
			tp->Pen->Style = (i % MM) ? psDot : psSolid;
			x = (int)(double(i) * double(XR - XL)/double(DM+1) + XL);
			tp->MoveTo(x, YT); tp->LineTo(x, YB);
		}
		for( i = 0; i <= LM; i++ ){
			x = (int)(double(i) * double(XR - XL)/double(LM) + XL);
			sprintf(bf, "%4.0lf", (max*i)/LM);
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(x - (tp->TextWidth(bf)/2), YB + 6, bf);
		}
		tp->Pen->Color = clRed;
		tp->Pen->Style = psDot;
		x = (int)(XL + (fs/2) * (double(XR-XL)/max));
		tp->MoveTo(x, YT); tp->LineTo(x, YB);

		tp->Pen->Color = clBlue;
		tp->Pen->Style = psSolid;
	}
	int ay = 0;
	pi2t = 2.0 * PI / fs;
	pi4t = 2.0 * pi2t;
	tp->Pen->Color = col;
	double	A, B, C, D, P, R;
	double	cosw, sinw, cos2w, sin2w;
	for( x = XL, f = 0.0; x < XR; x++, f += (max/double(XR-XL)) ){
		cosw = cos(pi2t*f);
		sinw = sin(pi2t*f);
		cos2w = cos(pi4t*f);
		sin2w = sin(pi4t*f);
		A = a0 + a1 * cosw + a2 * cos2w;
		B = 1 + b1 * cosw + b2 * cos2w;
		C = a1 * sinw + a2 * sin2w;
		D = b1 * sinw + b2 * sin2w;
		P = A*A + C*C;
		R = B*B + D*D;
		g = sqrt(P/R);
		if( g == 0 ) g = 1.0e-38;
		gdb = 20*0.4342944*log(fabs(g)) + 60.0;
		if( gdb < 0.0 ) gdb = 0.0;
		gdb = (gdb * double(YB-YT))/60.0;
		y = YB - (int)gdb;
		if( x == XL ){
			tp->MoveTo(x, y);
			tp->LineTo(x, y);
		}
		else {
			tp->MoveTo(x-1, ay);
			tp->LineTo(x, y);
		}
		ay = y;
	}
}

//---------------------------------------------------------------------
// 周波数特性グラフ（フィルタスレッド内でコールしてはいけない）
//
//
void DrawGraphIIR(Graphics::TBitmap *pBitmap, CIIR *ip, int Over, int &nmax, int init, TColor col)
{
	int		x, y;
	double	f, gdb, g, pi2t, pi4t, fs;
	double	max;
	char	bf[80];

	TCanvas *tp = pBitmap->Canvas;
	TRect rc;
	rc.Left = 0;
	rc.Right = pBitmap->Width;
	rc.Top = 0;
	rc.Bottom = pBitmap->Height;
	if( init ){
		tp->Brush->Color = clWhite;
		tp->FillRect(rc);
	}
	int	LM;		// 周波数表示のあるライン数
	int DM;		// 内部線の数
	int MM;		// 実線の間隔
	switch(Over){
		case 2:
			max = 3000;
			fs = SampFreq/2.0;
			break;
		case 3:
			max = 2000;
			fs = SampFreq/3.0;
			break;
		default:
			max = 4000;
			fs = SampFreq;
			break;
	}
	if( nmax ){
		max = nmax;
	}
	else {
		nmax = max;
	}
	switch(nmax){
		case 3000:
			LM = 3;
			DM = 14;
			MM = 5;
			break;
		case 100:
		case 200:
		case 2000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		case 400:
		case 800:
		case 4000:
			LM = 4;
			DM = 19;
			MM = 5;
			break;
		default:		// 6000
			LM = 3;
			DM = 11;
			MM = 4;
			break;
	}
	int XL = 32;
	int XR = pBitmap->Width - 16;
	int YT = 16;
	int YB = pBitmap->Height - 24;

	int i;
	if( init ){
		tp->Pen->Color = clBlack;
		tp->Font->Size = 8;
		tp->MoveTo(XL, YT); tp->LineTo(XR, YT); tp->LineTo(XR, YB); tp->LineTo(XL, YB); tp->LineTo(XL, YT);
		tp->Pen->Color = clGray;
		for( i = 0; i < 5; i++ ){
			tp->Pen->Style = (i & 1) ? psSolid : psDot;
			y = (int)(double(i + 1) * double(YB - YT)/6.0 + YT);
			tp->MoveTo(XL, y); tp->LineTo(XR, y);
		}
		for( i = 1; i < 4; i++ ){
			y = (int)(double(i) * double(YB - YT)/3.0 + YT);
			sprintf( bf, "-%2u", (60 / 3)*i );
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(XL - 6 - tp->TextWidth(bf), y - (tp->TextHeight(bf)/2), bf);
		}
		strcpy(bf, "dB");
		tp->TextOut(XL - 6 - tp->TextWidth(bf), YT-(tp->TextHeight(bf)/2), bf);
		for( i = 1; i <= DM; i++ ){
			tp->Pen->Style = (i % MM) ? psDot : psSolid;
			x = (int)(double(i) * double(XR - XL)/double(DM+1) + XL);
			tp->MoveTo(x, YT); tp->LineTo(x, YB);
		}
		for( i = 0; i <= LM; i++ ){
			x = (int)(double(i) * double(XR - XL)/double(LM) + XL);
			sprintf(bf, "%4.0lf", (max*i)/LM);
			::SetBkMode(tp->Handle, TRANSPARENT);
			tp->TextOut(x - (tp->TextWidth(bf)/2), YB + 6, bf);
		}
		tp->Pen->Color = clRed;
		tp->Pen->Style = psDot;
		x = (int)(XL + (fs/2) * (double(XR-XL)/max));
		tp->MoveTo(x, YT); tp->LineTo(x, YB);

		tp->Pen->Color = clBlue;
		tp->Pen->Style = psSolid;
	}
	int ay = 0;
	pi2t = 2.0 * PI / fs;
	pi4t = 2.0 * pi2t;
	tp->Pen->Color = col;
	double	A, B, C, D, P, R;
	double	cosw, sinw, cos2w, sin2w;
	for( x = XL, f = 0.0; x < XR; x++, f += (max/double(XR-XL)) ){
		cosw = cos(pi2t*f);
		sinw = sin(pi2t*f);
		cos2w = cos(pi4t*f);
		sin2w = sin(pi4t*f);
		g = 1.0;
		double *ap = ip->A;
		double *bp = ip->B;
		for( i = 0; i < ip->m_order/2; i++, ap += 3, bp += 2 ){
/*
		A = a0 + a1 * cosw + a2 * cos2w;
		B = 1 + b1 * cosw + b2 * cos2w;
		C = a1 * sinw + a2 * sin2w;
		D = b1 * sinw + b2 * sin2w;
*/
			A = bp[0] + bp[1] * cosw + bp[0] * cos2w;
			B = 1 + -ap[1] * cosw + -ap[2] * cos2w;
			C = bp[1] * sinw + bp[0] * sin2w;
			D = -ap[1] * sinw + -ap[2] * sin2w;
			P = A*A + C*C;
			R = B*B + D*D;
			g *= sqrt(P/R);
		}
		if( ip->m_order & 1 ){
			A = bp[0] + bp[1] * cosw;
			B = 1 + -ap[1] * cosw;
			C = bp[1] * sinw;
			D = -ap[1] * sinw;
			P = A*A + C*C;
			R = B*B + D*D;
			g *= sqrt(P/R);
		}
		if( g == 0 ) g = 1.0e-38;
		gdb = 20*0.4342944*log(fabs(g)) + 60.0;
		if( gdb < 0.0 ) gdb = 0.0;
		gdb = (gdb * double(YB-YT))/60.0;
		y = YB - (int)gdb;
		if( x == XL ){
			tp->MoveTo(x, y);
			tp->LineTo(x, y);
		}
		else {
			tp->MoveTo(x-1, ay);
			tp->LineTo(x, y);
		}
		ay = y;
	}
}

double asinh(double x)
{
	return log(x + sqrt(x*x+1.0));
}

//------------------------------------------------------------------
// bc : 0-バターワース, 1-チェビシフ
// rp : 通過域のリップル
void MakeIIR(double *A, double *B, double fc, double fs, int order, int bc, double rp)
{
	double	w0, wa, u, zt, x;
	int		j, n;

	if( bc ){		// チェビシフ
		u = 1.0/double(order) * asinh(1.0/sqrt(pow(10.0,0.1*rp)-1.0));
	}
	wa = tan(PI*fc/fs);
	w0 = 1.0;
	n = (order & 1) + 1;
	double *pA = A;
	double *pB = B;
	double d1, d2;
	for( j = 1; j <= order/2; j++, pA+=3, pB+=2 ){
		if( bc ){	// チェビシフ
			d1 = sinh(u)*cos(n*PI/(2*order));
			d2 = cosh(u)*sin(n*PI/(2*order));
			w0 = sqrt(d1 * d1 + d2 * d2);
			zt = sinh(u)*cos(n*PI/(2*order))/w0;
		}
		else {		// バターワース
			w0 = 1.0;
			zt = cos(n*PI/(2*order));
		}
		pA[0] = 1 + wa*w0*2*zt + wa*w0*wa*w0;
		pA[1] = -2 * (wa*w0*wa*w0 - 1)/pA[0];
		pA[2] = -(1.0 - wa*w0*2*zt + wa*w0*wa*w0)/pA[0];
		pB[0] = wa*w0*wa*w0 / pA[0];
		pB[1] = 2*pB[0];
		n += 2;
	}
	if( bc && !(order & 1) ){
		x = pow( 1.0/pow(10.0,rp/20.0), 1/double(order/2) );
		pB = B;
		for( j = 1; j <= order/2; j++, pB+=2 ){
			pB[0] *= x;
			pB[1] *= x;
		}
	}
	if( order & 1 ){
		if( bc ) w0 = sinh(u);
		j = (order / 2);
		pA = A + (j*3);
		pB = B + (j*2);
		pA[0] = 1 + wa*w0;
		pA[1] = -(wa*w0 - 1)/pA[0];
		pB[0] = wa*w0/pA[0];
		pB[1] = pB[0];
	}
}

//---------------------------------------------------------------------------
CIIR::CIIR()
{
	m_order = 0;
	A = new double[IIRMAX*3];
	B = new double[IIRMAX*2];
	Z = new double[IIRMAX*2];
	memset(A, 0, sizeof(double[IIRMAX*3]));
	memset(B, 0, sizeof(double[IIRMAX*2]));
	memset(Z, 0, sizeof(double[IIRMAX*2]));
}

CIIR::~CIIR()
{
	if( A != NULL ) delete A;
	if( B != NULL ) delete B;
	if( Z != NULL ) delete Z;
}

void CIIR::Clear(void)
{
	memset(Z, 0, sizeof(double[IIRMAX*2]));
}

void CIIR::MakeIIR(double fc, double fs, int order, int bc, double rp)
{
	m_order = order;
	m_bc = bc;
	m_rp = rp;
	::MakeIIR(A, B, fc, fs, order, bc, rp);
}

double CIIR::Do(double d)
{
	double *pA = A;
	double *pB = B;
	double *pZ = Z;
	double o;
	for( int i = 0; i < m_order/2; i++, pA+=3, pB+=2, pZ+=2 ){
		d += pZ[0] * pA[1] + pZ[1] * pA[2];
		o = d * pB[0] + pZ[0] * pB[1] + pZ[1] * pB[0];
		pZ[1] = pZ[0];
		if( fabs(d) < 1e-37 ) d = 0.0;
		pZ[0] = d;
		d = o;
	}
	if( m_order & 1 ){
		d += pZ[0] * pA[1];
		o = d * pB[0] + pZ[0] * pB[0];
		if( fabs(d) < 1e-37 ) d = 0.0;
		pZ[0] = d;
		d = o;
	}
	return d;
}

//---------------------------------------------------------------------------
// CFIR2クラス
CFIR2::CFIR2()
{
	m_pZ = NULL;
    m_pH = NULL;
    m_pZP = NULL;
    m_W = 0;
    m_Tap = 0;
}

//---------------------------------------------------------------------------
CFIR2::~CFIR2()
{
	if( m_pZ ) delete m_pZ;
    if( m_pH ) delete m_pH;
}
//---------------------------------------------------------------------------
void __fastcall CFIR2::Create(int tap)
{
	if( !tap ){
		if( m_pZ ) delete m_pZ;
		m_pZ = NULL;
    }
	else if( (m_Tap != tap) || !m_pZ ){
		if( m_pZ ) delete m_pZ;
		m_pZ = new double[(tap+1)*2];
    	memset(m_pZ, 0, sizeof(double)*(tap+1)*2);
		m_W = 0;
    }
	m_Tap = tap;
    m_TapHalf = tap/2;
}
//---------------------------------------------------------------------------
void __fastcall CFIR2::Create(int tap, int type, double fs, double fcl, double fch, double att, double gain)
{
	if( (m_Tap != tap) || !m_pZ || !m_pH ){
		if( m_pZ ) delete m_pZ;
		m_pZ = new double[(tap+1)*2];
    	memset(m_pZ, 0, sizeof(double)*(tap+1)*2);
	    if( m_pH ) delete m_pH;
		m_pH = new double[tap+1];
		m_W = 0;
    }
	m_Tap = tap;
    m_TapHalf = tap/2;
	::MakeFilter(m_pH, tap, type, fs, fcl, fch, att, gain);
}
//---------------------------------------------------------------------------
void __fastcall CFIR2::Clear(void)
{
	if( m_pZ ) memset(m_pZ, 0, sizeof(double)*(m_Tap+1)*2);
}
//---------------------------------------------------------------------------
double __fastcall CFIR2::Do(double d)
{
	double *dp1 = &m_pZ[m_W+m_Tap+1];
	m_pZP = dp1;
	*dp1 = d;
    m_pZ[m_W] = d;
    d = 0;
    double *hp = m_pH;
    for( int i = 0; i <= m_Tap; i++ ){
		d += (*dp1--) * (*hp++);
    }
    m_W++;
	if( m_W > m_Tap ) m_W = 0;
    return d;
}
//---------------------------------------------------------------------------
double __fastcall CFIR2::Do(double d, double *hp)
{
	double *dp1 = &m_pZ[m_W+m_Tap+1];
	m_pZP = dp1;
	*dp1 = d;
    m_pZ[m_W] = d;
    d = 0;
    for( int i = 0; i <= m_Tap; i++ ){
		d += (*dp1--) * (*hp++);
    }
    m_W++;
	if( m_W > m_Tap ) m_W = 0;
    return d;
}
//---------------------------------------------------------------------------
double __fastcall CFIR2::Do(double *hp)
{
    double d = 0;
    double *dp = m_pZP;
    for( int i = 0; i <= m_Tap; i++ ){
		d += (*dp--) * (*hp++);
    }
    return d;
}
//---------------------------------------------------------------------------
void __fastcall CFIR2::Do(double &d, double &j, double *hp)
{
	double *dp1 = &m_pZ[m_W+m_Tap+1];
	m_pZP = dp1;
	*dp1 = d;
    m_pZ[m_W] = d;
    double dd = 0;
    for( int i = 0; i <= m_Tap; i++ ){
		dd += (*dp1--) * (*hp++);
    }
    j = dd;
    d= m_pZ[m_W+m_TapHalf+1];
    m_W++;
	if( m_W > m_Tap ) m_W = 0;
}

