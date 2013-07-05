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
#ifndef firH
#define firH
#include <inifiles.hpp>	//JA7UDE 0428

//---------------------------------------------------------------------------
#include "ComLib.h"
#define	TAPMAX	512

#pragma pack(push, 4)
#define	PI	3.1415926535897932384626433832795

enum {
	ffLPF,
	ffHPF,
	ffBPF,
	ffBEF,
	ffUSER,
	ffLMS,
};

typedef struct {
	int		n;
	int		typ;
	double	fs;
	double	fcl;
	double	fch;
	double	att;
	double	gain;
	double	fc;
	double	hp[TAPMAX+1];		/* 係数配列		*/
}FIR;
void MakeFilter(double *HP, int tap, int type, double fs, double fcl, double fch, double att, double gain);
void MakeFilter(double *HP, FIR *fp);
void MakeHilbert(double *H, int N, double fs, double fc1, double fc2);
void DrawGraph(Graphics::TBitmap *pBitmap, const double *H, int Tap, int Over, int &nmax, int init, TColor col);
void DrawGraphIIR(Graphics::TBitmap *pBitmap, double a0, double a1, double a2, double b1, double b2, int Over, int &nmax, int init, TColor col);

double __fastcall DoFIR(double *hp, double *zp, double d, int tap);

//---------------------------------------------------------------------------
// ダブルバッファによるFIRフィルタ
class CFIR2
{
private:
	int		m_Tap;
	double	*m_pZ;
    double	*m_pH;
	double	*m_pZP;

    int		m_W;
    int		m_TapHalf;
public:
	CFIR2();
    ~CFIR2();
	void __fastcall Create(int tap, int type, double fs, double fcl, double fch, double att, double gain);
	void __fastcall Clear(void);
    void __fastcall Create(int tap);
	double __fastcall Do(double d);
    double __fastcall Do(double d, double *hp);
	double __fastcall Do(double *hp);
	void __fastcall Do(double &d, double &j, double *hp);

    inline double GetHD(int n){return m_pH[n];};
    inline double *GetHP(void){return m_pH;};
    inline int GetTap(void){return m_Tap;};
};

#define LMSTAP  192
class CLMS
{
private:
	double	*Z;					// FIR Z Application
	double	*D;					// LMS Delay;

	double	m_lmsADJSC;			// スケール調整値
	double	m_lmsErr;			// LMS 誤差データ
	double	m_lmsMErr;			// LMS 誤差データ（×２μ）
	double  m_D;
public:
	int		m_Tap;
//	int		m_lmsInv;			// LMS InvOutput
//	int		m_lmsDelay;			// LMS Delay
	int		m_lmsAGC;			// LMS AGC
	double	m_lmsMU2;			// LMS 2μ
	double	m_lmsGM;			// LMS γ
	double	*H;					// アプリケーションフィルタの係数

	int     m_Tap_N;
	int     m_lmsDelay_N;
	double  m_lmsMU2_N;
	double  m_lmsGM_N;
public:
	CLMS();
	~CLMS();

	double Do(double d);
	double DoN(double d);
	void SetAN(int sw);
	int Sig(double d);
};

#define NOTCHTAPMAX 256
class CNotch
{
public:
	CFIR2	m_Notch;
//	double	*m_pZ;					// FIR Z Application
//	double	*m_pH;					// FIR K Application
	int		m_tap;
public:
	double	m_freq;
public:
	__fastcall CNotch();
	__fastcall ~CNotch();
	void __fastcall SetNotchFreq(double fq);
	double __fastcall Do(double d);
};

class CIIRTANK
{
private:
	double	z1, z2;
public:
	double	a0;
	double	b1, b2;
public:
	CIIRTANK();
	void SetFreq(double f, double smp, double bw);
	double Do(double d);
};

#define	IIRMAX	16
class CIIR
{
private:
	double	*Z;
public:
	double	*A;
	double	*B;
	int		m_order;
	int		m_bc;
	double	m_rp;
	CIIR();
	~CIIR();
	void MakeIIR(double fc, double fs, int order, int bc, double rp);
	double Do(double d);
	void Clear(void);
};
#pragma pack(pop)

void DrawGraphIIR(Graphics::TBitmap *pBitmap, CIIR *ip, int Over, int &nmax, int init, TColor col);
#endif

