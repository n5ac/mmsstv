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



#include <vcl.h>	//ja7ude 0521
#pragma hdrstop

#include <math.h>
#include "fft.h"
#include "ComLib.h"

#define	PI	3.1415926535897932384626433832795
#define PI2 (2*PI)

#define SCALEADJ_1 (-5.5)
#define SCALEADJ_2 (-6.5)
#define SCALEADJ_3 (-7.8)
#define SCALEADJ_4 (-9.0)
#define LOGADJ (2.81458e4)

#define SCALEPOW_1 	 	(1.0/4.0)
#define SCALEPOW_ADJ_1	0.0018
#define SCALEPOW_2 	 	(1.0/5.0)
#define SCALEPOW_ADJ_2	0.008
#define SCALEPOW_3 	 	(1.0/6.0)
#define SCALEPOW_ADJ_3	0.022
#define SCALEPOW_4 	 	(1.0/7.0)
#define SCALEPOW_ADJ_4	0.048
//-------------------------------------------------
// ＦＦＴ処理クラス
CFFT::CFFT()
{
	m_FFTDIS = 0;
	memset(m_fft, 0, sizeof(m_fft));
	m_CollectFFT = 1;
	m_CollectrPage = m_CollectwPage = 0;
	m_CollectFFTCount = 0;
	m_tWindow = new double[FFT_BUFSIZE];
	m_tSinCos = new double[FFT_BUFSIZE/2];
	m_Work = new int[SQRT_FFT_SIZE+2];
	memset(m_Work, 0, sizeof(int[SQRT_FFT_SIZE+2]));
	pStgBuf = new double[FFT_BUFSIZE];
	m_Work[0] = 0;
	makewt(FFT_SIZE/4, m_Work, m_tSinCos);
	makect(FFT_SIZE/4, m_Work, m_tSinCos + m_Work[0]);
	for(int i = 0; i < FFT_SIZE; i++){
		pStgBuf[i] = 1.0;
		m_tWindow[i] = (0.5 - 0.5*cos( (PI2*i)/(FFT_SIZE-1) ));	//ハニング窓
//		m_tWindow[i] = 1.0;
	}
	m_StgSize = 1;
	m_StgScale = 1.0;
	m_StgK = 0.0;
	m_StgSW = FALSE;
	m_CollectFlag = 0;
	m_Handle = NULL;
	m_CollectCount = 0;
	m_CollectFFT = 0;

	m_fqc.SetSampFreq(FFTSamp);
	m_fqc.m_Limit = 0;
	m_fqc.m_outOrder = 2;
	m_fqc.m_outFC = 400;
	m_fqc.CalcLPF();
}

//-------------------------------------------------
// ＦＦＴ処理クラスの再初期化
void CFFT::InitFFT(void)
{
	m_CollectFFT = 0;
	m_CollectFFTCount = 0;
	m_CollectFFT = 0;
	m_CollectFFTCount = 0;
	memset(m_Work, 0, sizeof(int[SQRT_FFT_SIZE+2]));
	makewt(FFT_SIZE/4, m_Work, m_tSinCos);
	makect(FFT_SIZE/4, m_Work, m_tSinCos + m_Work[0]);
	for(int i = 0; i < FFT_SIZE; i++){
		pStgBuf[i] = 1.0;
//		m_tWindow[i] = 1.0;
//		m_tWindow[i] = (0.5 - 0.5*cos( (PI2*i)/(FFT_SIZE-1) ));	//ハニング窓
		m_tWindow[i] = (0.5 - 0.5*cos( (PI2*i)/FFT_SIZE ));	//ハニング窓
//		m_tWindow[i] = 0.46 - 0.5*cos( (PI2*i)/FFT_SIZE ) + 0.08*cos(2*PI2*i/FFT_SIZE);	//ハニング窓
	}
	m_StgSize = 1;
	m_StgScale = 1.0;
	m_StgK = 0.0;
	m_StgSW = FALSE;
	m_CollectFlag = 0;
	m_CollectCount = 0;
	m_CollectFFT = 0;
}

//-------------------------------------------------
CFFT::~CFFT()
{
	if(pStgBuf){
		delete pStgBuf;
		pStgBuf = NULL;
	}
	if(m_Work){
		delete m_Work;
		m_Work = NULL;
	}
	if(m_tSinCos){
		delete m_tSinCos;
		m_tSinCos = NULL;
	}
	if(m_tWindow){
		delete m_tWindow;
		m_tWindow = NULL;
	}
}

//-------------------------------------------------
// データ収集（スレッド外で実行する）
void CFFT::TrigFFT(void)
{
	m_CollectFFT = 1;
	m_CollectFFTCount = 0;
	m_CollectrPage = m_CollectwPage = 0;
	m_CollectCount = 0;
	m_fqc.m_fq = ZEROFQ;
	m_CollectFFT = 0;		// 次の収集をトリガ
}
//-------------------------------------------------
// データ収集（スレッド内で実行する）
void CFFT::CollectFFT(double *lp, int dsize)
{
	if( DisPaint || m_CollectFFT ) return;

	if( sys.m_FFTPriority ){
		while( (m_CollectCount < 3) && dsize ){
			double *tp;
			int i;
			int size;
			switch(FFTSampType){
				case 1:
					size = dsize / 2;
					if( (size + m_CollectFFTCount) > FFT_SIZE ){
						size = FFT_SIZE - m_CollectFFTCount;
					}
					dsize -= (size * 2);
					for( tp = &m_CollectFFTBuf[m_CollectwPage][m_CollectFFTCount], i = 0; i < size; i++, tp++, lp+=2 ){
						*tp = *lp;
					}
					break;
				case 2:
					size = dsize / 4;
					if( (size + m_CollectFFTCount) > FFT_SIZE ){
						size = FFT_SIZE - m_CollectFFTCount;
					}
					dsize -= (size * 4);
					for( tp = &m_CollectFFTBuf[m_CollectwPage][m_CollectFFTCount], i = 0; i < size; i++, tp++, lp+=4 ){
						*tp = *lp;
					}
					break;
				default:
					size = dsize;
					if( (size + m_CollectFFTCount) > FFT_SIZE ){
						size = FFT_SIZE - m_CollectFFTCount;
					}
					dsize -= size;
					memcpy(&m_CollectFFTBuf[m_CollectwPage][m_CollectFFTCount], lp, sizeof(double)*size);
					lp += size;
					break;
			}
			m_CollectFFTCount += size;
			if( m_CollectFFTCount >= FFT_SIZE ){
				int page = m_CollectwPage;
				m_CollectwPage++;
				if( m_CollectwPage >= 3 ) m_CollectwPage = 0;
				m_CollectCount++;
				switch(sys.m_FFTPriority){
					case 1:
						m_CollectFFTCount = 0;
						break;
					case 2:
						m_CollectFFTCount = 128;
						memcpy(m_CollectFFTBuf[m_CollectwPage], &m_CollectFFTBuf[page][FFT_SIZE-128], sizeof(double)*128);
						break;
					case 3:
						m_CollectFFTCount = 256;
						memcpy(m_CollectFFTBuf[m_CollectwPage], &m_CollectFFTBuf[page][FFT_SIZE-256], sizeof(double)*256);
						break;
					default:
						m_CollectFFTCount = 512;
						memcpy(m_CollectFFTBuf[m_CollectwPage], &m_CollectFFTBuf[page][FFT_SIZE-512], sizeof(double)*512);
						break;
				}
				if( m_Handle != NULL ){
					 ::PostMessage(m_Handle, CM_FFT, 0, 0);
				}
			}
		}
	}
	else if( !m_CollectCount ){
		double *tp;
		int i;
		int size;
		switch(FFTSampType){
			case 1:
				size = dsize / 2;
				if( (size + m_CollectFFTCount) > FFT_SIZE ){
					size = FFT_SIZE - m_CollectFFTCount;
				}
				for( tp = &m_CollectFFTBuf[m_CollectwPage][m_CollectFFTCount], i = 0; i < size; i++, tp++, lp+=2 ){
					*tp = *lp;
				}
				break;
			case 2:
				size = dsize / 4;
				if( (size + m_CollectFFTCount) > FFT_SIZE ){
					size = FFT_SIZE - m_CollectFFTCount;
				}
				for( tp = &m_CollectFFTBuf[m_CollectwPage][m_CollectFFTCount], i = 0; i < size; i++, tp++, lp+=4 ){
					*tp = *lp;
				}
				break;
			default:
				size = dsize;
				if( (size + m_CollectFFTCount) > FFT_SIZE ){
					size = FFT_SIZE - m_CollectFFTCount;
				}
				memcpy(&m_CollectFFTBuf[m_CollectwPage][m_CollectFFTCount], lp, sizeof(double)*size);
				break;
		}
		m_CollectFFTCount += size;
		if( m_CollectFFTCount >= FFT_SIZE ){
			m_CollectwPage++;
			if( m_CollectwPage >= 3 ) m_CollectwPage = 0;
			m_CollectCount++;
			m_CollectFFTCount = 0;
		}
	}
}
//-------------------------------------------------
// 計算（スレッド外で実行する）
void CFFT::CalcFFT(int size, double gain, int stg)
{
	if( sys.m_FFTType == 2 ){
		CalcFQC(m_CollectFFTBuf[m_CollectrPage], size, gain, stg, m_fft);
	}
	else {
		Calc(m_CollectFFTBuf[m_CollectrPage], size, gain, stg, m_fft);
	}
	m_CollectrPage++;
	if( m_CollectrPage >= 3 ) m_CollectrPage = 0;
	if( m_CollectCount ) m_CollectCount--;
}

void CFFT::makewt(int nw, int *ip, double *w)
{
	int nwh, j;
	double delta, x, y;

	ip[0] = nw;
	ip[1] = 1;
	if(nw > 2){
		nwh = nw >> 1;
		delta = atan(1.0) / nwh;
		w[0] = 1;
		w[1] = 0;
		w[nwh] = cos(delta * nwh);
		w[nwh + 1] = w[nwh];
		for(j = 2; j < nwh; j += 2){
			x = cos(delta * j);
			y = sin(delta * j);
			w[j] = x;
			w[j + 1] = y;
			w[nw - j] = y;
			w[nw - j + 1] = x;
		}
		bitrv2(nw, ip + 2, w);
	}
}
//-------------------------------------------------
// データの処理化
void CFFT::makect(int nc, int *ip, double *c)
{
	int nch, j;
	double delta;

	ip[1] = nc;
	if(nc > 1){
		nch = nc >> 1;
		delta = atan(1.0) / nch;
		c[0] = cos(delta * nch);
		c[nch] = 0.5 * c[0];
		for(j = 1; j < nch; j++){
			c[j] = 0.5 * cos(delta * j);
			c[nc - j] = 0.5 * sin(delta * j);
		}
	}
}
//-------------------------------------------------
// データの処理化
void CFFT::bitrv2(int n, int *ip, double *a)
{
	int j, j1, k, k1, l, m, m2;
	double xr, xi;

	ip[0] = 0;
	l = n;
	m = 1;
	while((m << 2) < l){
		l >>= 1;
		for (j = 0; j < m; j++) {
			ip[m + j] = ip[j] + l;
		}
		m <<= 1;
	}
	if((m << 2) > l){
		for (k = 1; k < m; k++) {
			for (j = 0; j < k; j++) {
				j1 = (j << 1) + ip[k];
				k1 = (k << 1) + ip[j];
				xr = a[j1];
				xi = a[j1 + 1];
				a[j1] = a[k1];
				a[j1 + 1] = a[k1 + 1];
				a[k1] = xr;
				a[k1 + 1] = xi;
			}
		}
	} else {
		m2 = m << 1;
		for(k = 1; k < m; k++){
			for(j = 0; j < k; j++){
				j1 = (j << 1) + ip[k];
				k1 = (k << 1) + ip[j];
				xr = a[j1];
				xi = a[j1 + 1];
				a[j1] = a[k1];
				a[j1 + 1] = a[k1 + 1];
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += m2;
				k1 += m2;
				xr = a[j1];
				xi = a[j1 + 1];
				a[j1] = a[k1];
				a[j1 + 1] = a[k1 + 1];
				a[k1] = xr;
				a[k1 + 1] = xi;
			}
		}
	}
}
//-------------------------------------------------
void CFFT::cftfsub(int n, double *a, double *w)
{
	int j, j1, j2, j3, l;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

	l = 2;
	if(n > 8){
		cft1st(n, a, w);
		l = 8;
		while((l << 2) < n){
			cftmdl(n, l, a, w);
			l <<= 2;
		}
	}
	if((l << 2) == n){
		for(j = 0; j < l; j += 2){
			j1 = j + l;
			j2 = j1 + l;
			j3 = j2 + l;
			x0r = a[j] + a[j1];
			x0i = a[j + 1] + a[j1 + 1];
			x1r = a[j] - a[j1];
			x1i = a[j + 1] - a[j1 + 1];
			x2r = a[j2] + a[j3];
			x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3];
			x3i = a[j2 + 1] - a[j3 + 1];
			a[j] = x0r + x2r;
			a[j + 1] = x0i + x2i;
			a[j2] = x0r - x2r;
			a[j2 + 1] = x0i - x2i;
			a[j1] = x1r - x3i;
			a[j1 + 1] = x1i + x3r;
			a[j3] = x1r + x3i;
			a[j3 + 1] = x1i - x3r;
		}
	} else {
		for(j = 0; j < l; j += 2){
			j1 = j + l;
			x0r = a[j] - a[j1];
			x0i = a[j + 1] - a[j1 + 1];
			a[j] += a[j1];
			a[j + 1] += a[j1 + 1];
			a[j1] = x0r;
			a[j1 + 1] = x0i;
		}
	}
}
//-------------------------------------------------
void CFFT::cft1st(int n, double *a, double *w)
{
	int j, k1, k2;
	double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

	x0r = a[0] + a[2];
	x0i = a[1] + a[3];
	x1r = a[0] - a[2];
	x1i = a[1] - a[3];
	x2r = a[4] + a[6];
	x2i = a[5] + a[7];
	x3r = a[4] - a[6];
	x3i = a[5] - a[7];
	a[0] = x0r + x2r;
	a[1] = x0i + x2i;
	a[4] = x0r - x2r;
	a[5] = x0i - x2i;
	a[2] = x1r - x3i;
	a[3] = x1i + x3r;
	a[6] = x1r + x3i;
	a[7] = x1i - x3r;
	wk1r = w[2];
	x0r = a[8] + a[10];
	x0i = a[9] + a[11];
	x1r = a[8] - a[10];
	x1i = a[9] - a[11];
	x2r = a[12] + a[14];
	x2i = a[13] + a[15];
	x3r = a[12] - a[14];
	x3i = a[13] - a[15];
	a[8] = x0r + x2r;
	a[9] = x0i + x2i;
	a[12] = x2i - x0i;
	a[13] = x0r - x2r;
	x0r = x1r - x3i;
	x0i = x1i + x3r;
	a[10] = wk1r * (x0r - x0i);
	a[11] = wk1r * (x0r + x0i);
	x0r = x3i + x1r;
	x0i = x3r - x1i;
	a[14] = wk1r * (x0i - x0r);
	a[15] = wk1r * (x0i + x0r);
	k1 = 0;
	for(j = 16; j < n; j += 16){
		k1 += 2;
		k2 = k1 << 1;
		wk2r = w[k1];
		wk2i = w[k1 + 1];
		wk1r = w[k2];
		wk1i = w[k2 + 1];
		wk3r = wk1r - 2 * wk2i * wk1i;
		wk3i = 2 * wk2i * wk1r - wk1i;
		x0r = a[j] + a[j + 2];
		x0i = a[j + 1] + a[j + 3];
		x1r = a[j] - a[j + 2];
		x1i = a[j + 1] - a[j + 3];
		x2r = a[j + 4] + a[j + 6];
		x2i = a[j + 5] + a[j + 7];
		x3r = a[j + 4] - a[j + 6];
		x3i = a[j + 5] - a[j + 7];
		a[j] = x0r + x2r;
		a[j + 1] = x0i + x2i;
		x0r -= x2r;
		x0i -= x2i;
		a[j + 4] = wk2r * x0r - wk2i * x0i;
		a[j + 5] = wk2r * x0i + wk2i * x0r;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		a[j + 2] = wk1r * x0r - wk1i * x0i;
		a[j + 3] = wk1r * x0i + wk1i * x0r;
		x0r = x1r + x3i;
		x0i = x1i - x3r;
		a[j + 6] = wk3r * x0r - wk3i * x0i;
		a[j + 7] = wk3r * x0i + wk3i * x0r;
		wk1r = w[k2 + 2];
		wk1i = w[k2 + 3];
		wk3r = wk1r - 2 * wk2r * wk1i;
		wk3i = 2 * wk2r * wk1r - wk1i;
		x0r = a[j + 8] + a[j + 10];
		x0i = a[j + 9] + a[j + 11];
		x1r = a[j + 8] - a[j + 10];
		x1i = a[j + 9] - a[j + 11];
		x2r = a[j + 12] + a[j + 14];
		x2i = a[j + 13] + a[j + 15];
		x3r = a[j + 12] - a[j + 14];
		x3i = a[j + 13] - a[j + 15];
		a[j + 8] = x0r + x2r;
		a[j + 9] = x0i + x2i;
		x0r -= x2r;
		x0i -= x2i;
		a[j + 12] = -wk2i * x0r - wk2r * x0i;
		a[j + 13] = -wk2i * x0i + wk2r * x0r;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		a[j + 10] = wk1r * x0r - wk1i * x0i;
		a[j + 11] = wk1r * x0i + wk1i * x0r;
		x0r = x1r + x3i;
		x0i = x1i - x3r;
		a[j + 14] = wk3r * x0r - wk3i * x0i;
		a[j + 15] = wk3r * x0i + wk3i * x0r;
	}
}
//-------------------------------------------------
void CFFT::cftmdl(int n, int l, double *a, double *w)
{
	int j, j1, j2, j3, k, k1, k2, m, m2;
	double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

	m = l << 2;
	for(j = 0; j < l; j += 2){
		j1 = j + l;
		j2 = j1 + l;
		j3 = j2 + l;
		x0r = a[j] + a[j1];
		x0i = a[j + 1] + a[j1 + 1];
		x1r = a[j] - a[j1];
		x1i = a[j + 1] - a[j1 + 1];
		x2r = a[j2] + a[j3];
		x2i = a[j2 + 1] + a[j3 + 1];
		x3r = a[j2] - a[j3];
		x3i = a[j2 + 1] - a[j3 + 1];
		a[j] = x0r + x2r;
		a[j + 1] = x0i + x2i;
		a[j2] = x0r - x2r;
		a[j2 + 1] = x0i - x2i;
		a[j1] = x1r - x3i;
		a[j1 + 1] = x1i + x3r;
		a[j3] = x1r + x3i;
		a[j3 + 1] = x1i - x3r;
	}
	wk1r = w[2];
	for(j = m; j < l + m; j += 2){
		j1 = j + l;
		j2 = j1 + l;
		j3 = j2 + l;
		x0r = a[j] + a[j1];
		x0i = a[j + 1] + a[j1 + 1];
		x1r = a[j] - a[j1];
		x1i = a[j + 1] - a[j1 + 1];
		x2r = a[j2] + a[j3];
		x2i = a[j2 + 1] + a[j3 + 1];
		x3r = a[j2] - a[j3];
		x3i = a[j2 + 1] - a[j3 + 1];
		a[j] = x0r + x2r;
		a[j + 1] = x0i + x2i;
		a[j2] = x2i - x0i;
		a[j2 + 1] = x0r - x2r;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		a[j1] = wk1r * (x0r - x0i);
		a[j1 + 1] = wk1r * (x0r + x0i);
		x0r = x3i + x1r;
		x0i = x3r - x1i;
		a[j3] = wk1r * (x0i - x0r);
		a[j3 + 1] = wk1r * (x0i + x0r);
	}
	k1 = 0;
	m2 = m << 1;
	for(k = m2; k < n; k += m2){
		k1 += 2;
		k2 = k1 << 1;
		wk2r = w[k1];
		wk2i = w[k1 + 1];
		wk1r = w[k2];
		wk1i = w[k2 + 1];
		wk3r = wk1r - 2 * wk2i * wk1i;
		wk3i = 2 * wk2i * wk1r - wk1i;
		for(j = k; j < l + k; j += 2){
			j1 = j + l;
			j2 = j1 + l;
			j3 = j2 + l;
			x0r = a[j] + a[j1];
			x0i = a[j + 1] + a[j1 + 1];
			x1r = a[j] - a[j1];
			x1i = a[j + 1] - a[j1 + 1];
			x2r = a[j2] + a[j3];
			x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3];
			x3i = a[j2 + 1] - a[j3 + 1];
			a[j] = x0r + x2r;
			a[j + 1] = x0i + x2i;
			x0r -= x2r;
			x0i -= x2i;
			a[j2] = wk2r * x0r - wk2i * x0i;
			a[j2 + 1] = wk2r * x0i + wk2i * x0r;
			x0r = x1r - x3i;
			x0i = x1i + x3r;
			a[j1] = wk1r * x0r - wk1i * x0i;
			a[j1 + 1] = wk1r * x0i + wk1i * x0r;
			x0r = x1r + x3i;
			x0i = x1i - x3r;
			a[j3] = wk3r * x0r - wk3i * x0i;
			a[j3 + 1] = wk3r * x0i + wk3i * x0r;
		}
		wk1r = w[k2 + 2];
		wk1i = w[k2 + 3];
		wk3r = wk1r - 2 * wk2r * wk1i;
		wk3i = 2 * wk2r * wk1r - wk1i;
		for(j = k + m; j < l + (k + m); j += 2){
			j1 = j + l;
			j2 = j1 + l;
			j3 = j2 + l;
			x0r = a[j] + a[j1];
			x0i = a[j + 1] + a[j1 + 1];
			x1r = a[j] - a[j1];
			x1i = a[j + 1] - a[j1 + 1];
			x2r = a[j2] + a[j3];
			x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3];
			x3i = a[j2 + 1] - a[j3 + 1];
			a[j] = x0r + x2r;
			a[j + 1] = x0i + x2i;
			x0r -= x2r;
			x0i -= x2i;
			a[j2] = -wk2i * x0r - wk2r * x0i;
			a[j2 + 1] = -wk2i * x0i + wk2r * x0r;
			x0r = x1r - x3i;
			x0i = x1i + x3r;
			a[j1] = wk1r * x0r - wk1i * x0i;
			a[j1 + 1] = wk1r * x0i + wk1i * x0r;
			x0r = x1r + x3i;
			x0i = x1i - x3r;
			a[j3] = wk3r * x0r - wk3i * x0i;
			a[j3 + 1] = wk3r * x0i + wk3i * x0r;
		}
	}
}
//---------------------------------------------------------------------------
void CFFT::Calc(double * InBuf, int size, double gain, int stg, int* OutBuf)
{
	if( m_FFTDIS ) return;		// for math error
	m_FFTDIS++;
	int i;

	if( stg > 1 ){
		m_StgSW = TRUE;
	}
	else {
		m_StgSW = FALSE;
	}
	m_StgSize = stg;
	if( stg ){
		m_StgScale = 1.0 / double(m_StgSize);
		m_StgK = 1.0 - m_StgScale;
	}
	else {
		m_StgScale = 1.0;
		m_StgK = 0.0;
	}
	double *dp = InBuf;
	for(i=0; i < FFT_SIZE; i++, dp++){
		if( *dp > 32768.0 ){
			*dp = 32768.0;
		}
		else if( *dp < -32768.0 ){
			*dp = -32768.0;
		}
		(*dp) *= m_tWindow[i];
	}
	bitrv2(FFT_SIZE, m_Work + 2, InBuf);
	cftfsub(FFT_SIZE, InBuf, m_tSinCos);
	rftfsub(FFT_SIZE, InBuf, m_Work[1], m_tSinCos + m_Work[0]);
	for( i = 0, dp = pStgBuf; i <= size; i++, dp+=2 ){
		OutBuf[i] = int((gain * (*dp)));
	}
	m_FFTDIS--;
}

//---------------------------------------------------------------------------
void CFFT::CalcFQC(double * InBuf, int size, double gain, int stg, int* OutBuf)
{
	double *dp = InBuf;
	double d;
	int i, r;
	memset(OutBuf, 0, sizeof(int)*size);
	int ad, ad2, max;
	switch(sys.m_FFTGain&3){
		case 0:
			ad = 30;
			max = ad * 32;
			break;
		case 1:
			ad = 30;
			max = ad * 24;
			break;
		case 2:
			ad = 40;
			max = ad * 16;
			break;
		default:
			ad = 50;
			max = ad * 12;
			break;
	}
	ad2 = ad / 2;
	int *op;
	size--;
	for( i = 0; i < FFT_SIZE; i++, dp++ ){
		d = m_fqc.Do(*dp);
		d = 1900.0 - (d * 400 / 16384.0);
		r = d * FFT_SIZE / FFTSamp;
		if( (r >= 1) && (r < size) ){
			op = &OutBuf[r];
			if( *op < max ){
				*op += ad;
				*(op-1) += ad2;
				*(op+1) += ad2;
			}
		}
	}
}
//---------------------------------------------------------------------------
void CFFT::rftfsub(int n, double *a, int nc, double *c)
{
	int j, k, kk, ks, m;
	double wkr, wki, xr, xi, yr, yi;
	double  d;

	ks = (nc << 2) / n;
	kk = 0;
	m = n >> 1;
	j = n - 2;
	if(m_StgSW){
		for (k = 2; k <= m; k += 2, j -= 2 ){
			kk += ks;
			wkr = 0.5 - c[nc - kk];
			wki = c[kk];
			xr = a[k] - a[j];
			xi = a[k + 1] + a[j + 1];
			yr = wkr * xr - wki * xi;
			yi = wkr * xi + wki * xr;
			a[k] -= yr;
			xi = a[k]*a[k];
			a[k+1] -= yi;
			xi += ( a[k+1]*a[k+1]);
			a[j] += yr;
			xr = a[j]*a[j];
			a[j+1] -= yi;
			xr += (a[j+1]*a[j+1]);
			if( xi <= 0 ) xi = 0.0001;
			if( xi >= 1e38 ) xi = 1e38;
			if( xr <= 0 ) xr = 0.0001;
			if( xr >= 1e38 ) xr = 1e38;
			if( FFT_SIZE == 1024 ){
				xi *= 4;
				xr *= 4;
			}
			switch(sys.m_FFTGain){
				case 0:
					pStgBuf[k] = m_StgK*pStgBuf[k] + m_StgScale*(log10(xi+LOGADJ) + SCALEADJ_1);
					pStgBuf[j] = m_StgK*pStgBuf[j] + m_StgScale*(log10(xr+LOGADJ) + SCALEADJ_1);
					break;
				case 1:
					pStgBuf[k] = m_StgK*pStgBuf[k] + m_StgScale*(log10(xi+LOGADJ) + SCALEADJ_2);
					pStgBuf[j] = m_StgK*pStgBuf[j] + m_StgScale*(log10(xr+LOGADJ) + SCALEADJ_2);
					break;
				case 2:
					pStgBuf[k] = m_StgK*pStgBuf[k] + m_StgScale*(log10(xi+LOGADJ) + SCALEADJ_3);
					pStgBuf[j] = m_StgK*pStgBuf[j] + m_StgScale*(log10(xr+LOGADJ) + SCALEADJ_3);
					break;
				case 3:
					pStgBuf[k] = m_StgK*pStgBuf[k] + m_StgScale*(log10(xi+LOGADJ) + SCALEADJ_4);
					pStgBuf[j] = m_StgK*pStgBuf[j] + m_StgScale*(log10(xr+LOGADJ) + SCALEADJ_4);
					break;
				case 4:
					d = xi * 32e-10;
					pStgBuf[k] = m_StgK*pStgBuf[k] + m_StgScale*d;
					d = xr * 32e-10;
					pStgBuf[j] = m_StgK*pStgBuf[j] + d;
					break;
				case 5:
					d = xi * 96e-10;
					pStgBuf[k] = m_StgK*pStgBuf[k] + m_StgScale*d;
					d = xr * 96e-10;
					pStgBuf[j] = m_StgK*pStgBuf[j] + d;
					break;
				case 6:
					d = xi * 256e-10;
					pStgBuf[k] = m_StgK*pStgBuf[k] + m_StgScale*d;
					d = xr * 256e-10;
					pStgBuf[j] = m_StgK*pStgBuf[j] + d;
					break;
				default:
					d = xi * 512e-10;
					pStgBuf[k] = m_StgK*pStgBuf[k] + m_StgScale*d;
					d = xr * 512e-10;
					pStgBuf[j] = m_StgK*pStgBuf[j] + d;
					break;
			}
		}
	}
	else {
		for (k = 2; k <= m; k += 2, j -= 2 ){
			kk += ks;
			wkr = 0.5 - c[nc - kk];
			wki = c[kk];
			xr = a[k] - a[j];
			xi = a[k + 1] + a[j + 1];
			yr = wkr * xr - wki * xi;
			yi = wkr * xi + wki * xr;
			a[k] -= yr;
			xi = a[k]*a[k];
			a[k+1] -= yi;
			xi += ( a[k+1]*a[k+1]);
			a[j] += yr;
			xr = a[j]*a[j];
			a[j+1] -= yi;
			xr += (a[j+1]*a[j+1]);
			if( xi <= 0 ) xi = 0.0001;
			if( xi >= 1e38 ) xi = 1e38;
			if( xr <= 0 ) xr = 0.0001;
			if( xr >= 1e38 ) xr = 1e38;
			if( FFT_SIZE == 1024 ){
				xi *= 4;
				xr *= 4;
			}
			switch(sys.m_FFTGain){
				case 0:
					pStgBuf[k] = log10(xi+LOGADJ)+SCALEADJ_1;
					pStgBuf[j] = log10(xr+LOGADJ)+SCALEADJ_1;
					break;
				case 1:
					pStgBuf[k] = log10(xi+LOGADJ)+SCALEADJ_2;
					pStgBuf[j] = log10(xr+LOGADJ)+SCALEADJ_2;
					break;
				case 2:
					pStgBuf[k] = log10(xi+LOGADJ)+SCALEADJ_3;
					pStgBuf[j] = log10(xr+LOGADJ)+SCALEADJ_3;
					break;
				case 3:
					pStgBuf[k] = log10(xi+LOGADJ)+SCALEADJ_4;
					pStgBuf[j] = log10(xr+LOGADJ)+SCALEADJ_4;
					break;
				case 4:
					pStgBuf[k] = xi * 32e-10;
					pStgBuf[j] = xr * 32e-10;
					break;
				case 5:
					pStgBuf[k] = xi * 96e-10;
					pStgBuf[j] = xr * 96e-10;
					break;
				case 6:
					pStgBuf[k] = xi * 256e-10;
					pStgBuf[j] = xr * 256e-10;
					break;
				default:
					pStgBuf[k] = xi * 512e-10;
					pStgBuf[j] = xr * 512e-10;
					break;
			}
		}
	}
	pStgBuf[FFT_SIZE/2] = pStgBuf[(FFT_SIZE/2) - 2];
}

