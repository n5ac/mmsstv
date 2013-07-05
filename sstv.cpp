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

#include "sstv.h"

double	g_dblToneOffset = 0.0;
//---------------------------------------------------------------------------
// VCOクラス
CTICK::CTICK()
{
	ptbl[0] = new int[CLOCKMAX];
	memset(ptbl[0], 0, sizeof(int[CLOCKMAX]));
	ptbl[1] = new int[CLOCKMAX];
	memset(ptbl[1], 0, sizeof(int[CLOCKMAX]));
}
CTICK::~CTICK()
{
	delete ptbl[0];
	delete ptbl[1];
}

void CTICK::Init(void)
{
	m_wsel = 0;
	m_wp = ptbl[0];
	m_wcnt = 0;
	m_Trig = 0;
}
void CTICK::Write(double d)
{
	*m_wp = int(d);
	m_wp++;
	m_wcnt++;
	if( m_wcnt >= m_Samp ){
		m_wcnt = 0;
		m_wsel++;
		m_wsel &= 1;
		m_wp = ptbl[m_wsel];
		m_Trig = 1;
	}
}
int *CTICK::GetData(void)
{
	if( !m_Trig ) return NULL;
	m_Trig = 0;
	int sel = m_wsel + 1;
	sel &= 1;
	return ptbl[sel];
}
//---------------------------------------------------------------------------
// VCOクラス
CVCO::CVCO()
{
	m_vlock = 0;
	m_SampleFreq = SampFreq;
	m_FreeFreq = 1900.0 + g_dblToneOffset;
	m_TableSize = int(SampFreq*2);
	pSinTbl = new double[m_TableSize];
	m_c1 = m_TableSize/16.0;
	m_c2 = int( double(m_TableSize) * m_FreeFreq / m_SampleFreq );
	m_z = 0;
	double pi2t = 2 * PI / double(m_TableSize);
	for( int i = 0; i < m_TableSize; i++ ){
		pSinTbl[i] = sin(double(i) * pi2t);
	}
}

CVCO::~CVCO()
{
	if( m_vlock ) ::VirtualUnlock(pSinTbl, sizeof(double)*m_TableSize);
	delete pSinTbl;
}

void CVCO::SetGain(double gain)
{
	m_c1 = double(m_TableSize) * gain / m_SampleFreq;
}

void CVCO::VirtualLock(void)
{
	if( !m_vlock ){
		::VirtualLock(pSinTbl, sizeof(double)*m_TableSize);
		m_vlock = 1;
	}
}

void CVCO::SetSampleFreq(double f)
{
	m_SampleFreq = f;
	int size = int(m_SampleFreq*2);
	if( m_TableSize != size ){
		if( pSinTbl != NULL ){
			if( m_vlock ) ::VirtualUnlock(pSinTbl, sizeof(double)*m_TableSize);
			delete pSinTbl;
		}
		m_TableSize = size;
		pSinTbl = new double[m_TableSize];
		if( m_vlock ) ::VirtualLock(pSinTbl, sizeof(double)*m_TableSize);
		double pi2t = 2 * PI / double(m_TableSize);
		for( int i = 0; i < m_TableSize; i++ ){
			pSinTbl[i] = sin(double(i) * pi2t);
		}
	}
	SetFreeFreq(m_FreeFreq);
}

void CVCO::SetFreeFreq(double f)
{
	m_FreeFreq = f;
	m_c2 = double(m_TableSize) * m_FreeFreq / m_SampleFreq;
}

void CVCO::InitPhase(void)
{
	m_z = 0;
}

double CVCO::Do(double d)
{		// -1 to 1
	m_z += (d * m_c1 + m_c2);
	while( m_z >= m_TableSize ){
		m_z -= m_TableSize;
	}
	while( m_z < 0 ){
		m_z += m_TableSize;
	}
	return pSinTbl[int(m_z)];
}



//--------------------------------------------------------
// CScopeクラス
CScope::CScope()
{
	m_ScopeSize = SCOPESIZE;
	m_DataFlag = 1;

	pScopeData = NULL;
	m_wp = 0;
}

CScope::~CScope()
{
	m_DataFlag = 1;
	if( pScopeData != NULL ){
		delete pScopeData;
		pScopeData = NULL;
	}
}

void CScope::InitMem(void)
{
	if( pScopeData == NULL ){
		pScopeData = new double[m_ScopeSize];
		memset(pScopeData, 0, sizeof(double)*m_ScopeSize);
	}
}

void CScope::WriteData(double d)
{
	if( !m_DataFlag ){
		if( m_wp < m_ScopeSize ){
			pScopeData[m_wp] = d;
			m_wp++;
			if( m_wp >= m_ScopeSize ){
				m_DataFlag = 1;
			}
		}
	}
}

void CScope::UpdateData(double d)
{
	if( !m_DataFlag ){
		if( m_wp ){
			pScopeData[m_wp-1] = d;
		}
	}
}

void CScope::Collect(int size)
{
	m_DataFlag = 1;
	InitMem();
	m_ScopeSize = size;
	m_wp = 0;
	m_DataFlag = 0;
}



//--------------------------------------------------------
// CNoiseクラス
CNoise::CNoise()
{
	reg = 0x12345;

	memset(Z, 0, sizeof(Z));
	MakeFilter(H, NOISEBPFTAP, ffLPF, SampFreq, 3000.0, 3000.0, 60, 1.0);
};

double CNoise::GetNoise(void)
{
	DWORD r = reg >> 1;
	if( (reg ^ r) & 1 ){
		r |= 0xffe00000;
	}
	else {
		r &= 0x001fffff;
	}
	reg = r;
	double d = double(reg) / 1000000.0;
//    return d;
	return DoFIR(H, Z, d, NOISEBPFTAP);		// 帯域制限
}


//--------------------------------------------------------
// CPLLクラス
CPLL::CPLL()
{
	m_err = 0;
	m_out = 0;
	m_vcoout = 0;
	m_vcogain = 1.0;
	m_outgain = 32768.0 * m_vcogain;
	m_SampleFreq = SampFreq;
//	m_Shift = 800.0;
//	m_FreeFreq = (1500 + 2300)/2;
	m_loopOrder = 1;
	m_loopFC = 1500.0;
	m_outOrder = 3;
	m_outFC = 900.0;
    SetWidth(0);
//	SetFreeFreq(1500, 2300);
	SetSampleFreq(m_SampleFreq);

	m_Max = 1.0;
	m_Min = -1.0;
	m_d = 0;
	m_agc = 1.0;
	m_agca = 0.0;
}

void CPLL::SetWidth(int fNarrow)
{
	if( fNarrow ){
		m_Shift = NARROW_BW;
		m_FreeFreq = NARROW_CENTER;
		SetFreeFreq(NARROW_LOW, NARROW_HIGH);
    }
    else {
		m_Shift = 800.0;
		m_FreeFreq = (1500 + 2300)*0.5;
		SetFreeFreq(1500, 2300);
    }
    SetVcoGain(m_vcogain);
}

void CPLL::SetVcoGain(double g)
{
	m_vcogain = g;
	vco.SetGain(-m_Shift * g);
	m_outgain = 32768.0 * m_vcogain;
}

void CPLL::MakeLoopLPF(void)
{
	loopLPF.MakeIIR(m_loopFC, m_SampleFreq, m_loopOrder, 0, 0);
}

void CPLL::MakeOutLPF(void)
{
	outLPF.MakeIIR(m_outFC, m_SampleFreq, m_outOrder, 0, 0);
}

void CPLL::SetFreeFreq(double f1, double f2)
{
	m_FreeFreq = (f1 + f2)/2.0;
	m_Shift = (f2 - f1);
	vco.SetFreeFreq(m_FreeFreq + g_dblToneOffset);
	vco.SetGain(-m_Shift * m_vcogain);
}

void CPLL::SetSampleFreq(double f)
{
	m_SampleFreq = f;
	vco.SetSampleFreq(f);
	vco.SetFreeFreq(m_FreeFreq + g_dblToneOffset);
	SetVcoGain(1.0);
	MakeLoopLPF();
	MakeOutLPF();
}

double CPLL::Do(double d)
{
	if( m_Max < d ) m_Max = d;
	if( m_Min > d ) m_Min = d;
	if( (d >= 0) && (m_d < 0) ){
		m_agc = m_Max - m_Min;
		m_d = (5.0/m_agc);
		m_agc = (m_agca + m_d) * 0.5;
		m_agca = m_d;
		m_Max = 1.0;
		m_Min = -1.0;
	}
	m_d = d;
	d *= m_agc;
// Loop Filter
	m_out = loopLPF.Do(m_err);
	if( m_out > 1.5 ){
		m_out = 1.5;
	}
	else if( m_out < -1.5 ){
		m_out = -1.5;
	}
// VCO
	m_vcoout = vco.Do(m_out);
// 位相比較
	m_err = m_vcoout * d;
	return outLPF.Do(m_out) * m_outgain;
}

//--------------------------------------------------------
// CFQCクラス
CFQC::CFQC()
{
	m_Type = 0;

	m_Limit = 1;
	m_d = 0;
	m_Count = 0;
	m_ACount = 0;
	m_fq = ZEROFQ;
	m_d = 0;
	m_out = 0;
	m_outOrder = 3;
	m_outFC = 900;
	m_SmoozFq = 2200;
	m_SampFreq = SampFreq;
	m_Timer = m_STimer = int(m_SampFreq);
    SetWidth(0);
	CalcLPF();
}

void CFQC::SetWidth(int fNarrow)
{
	if( fNarrow ){
		m_BWH = NARROW_BWH;
		m_CenterFQ = NARROW_CENTER + g_dblToneOffset;
		m_HighFQ = 2400.0 + g_dblToneOffset;
        m_LowFQ = NARROW_AFCLOW + g_dblToneOffset;
    }
    else {
		m_BWH = 400.0;
		m_CenterFQ = 1900.0 + g_dblToneOffset;
		m_HighFQ = 2400.0 + g_dblToneOffset;
        m_LowFQ = 1000.0 + g_dblToneOffset;
    }
	m_HighVal = (m_HighFQ - m_CenterFQ) / m_BWH;
    m_LowVal = (m_LowFQ - m_CenterFQ) / m_BWH;
}

void CFQC::Clear(void)
{
	m_d = 0;
	m_Count = 0;
	m_ACount = 0;
	m_fq = ZEROFQ;
	m_d = 0;
	m_out = 0;
	m_Timer = m_STimer;
}

void CFQC::SetSampFreq(double fq)
{
	m_SampFreq = fq;
	CalcLPF();
	m_STimer = int(m_SampFreq);
}

void CFQC::CalcLPF(void)
{
	m_iir.MakeIIR(m_outFC, m_SampFreq, m_outOrder, 0, 0);
	if( m_SmoozFq < 500 ) m_SmoozFq = 500.0;
	m_fir.SetCount(m_SampFreq/m_SmoozFq);
}

double CFQC::Do(double d)
{
	double count;
	double offset;

	if( d >= 0 ){
		if( m_d < 0 ){
			count = m_Count - m_ACount;
			offset = d/(d - m_d);
			m_ACount = m_Count - offset;
			count -= offset;
			if( count >= 1.0 ){
				m_fq = m_SampFreq * 0.5 / count;
				if( m_Limit ){
					if( m_fq > m_HighFQ ){
						m_fq = m_HighVal;
					}
					else if( m_fq < m_LowFQ ){
						m_fq = m_LowVal;
					}
					else {
						m_fq -= m_CenterFQ;
						m_fq /= m_BWH;
					}
				}
				else {
					m_fq -= m_CenterFQ;
					m_fq /= m_BWH;
					m_Timer = m_STimer;
				}
			}
		}
	}
	else {
		if( m_d >= 0 ){
			count = m_Count - m_ACount;
			offset = d/(d - m_d);
			m_ACount = m_Count - offset;
			count -= offset;
			if( count >= 1.0 ){
				m_fq = m_SampFreq * 0.5 / count;
				if( m_Limit ){
					if( m_fq > m_HighFQ ){
						m_fq = m_HighVal;
					}
					else if( m_fq < m_LowFQ ){
						m_fq = m_LowVal;
					}
					else {
						m_fq -= m_CenterFQ;
						m_fq /= m_BWH;
					}
				}
				else {
					m_fq -= m_CenterFQ;
					m_fq /= m_BWH;
					m_Timer = m_STimer;
				}
			}
		}
	}
	if( !m_Limit && m_Timer ){
		m_Timer--;
		if( !m_Timer ) m_fq = -m_CenterFQ/m_BWH;
	}
	switch(m_Type){
		case 0:     // IIR
			m_out = m_iir.Do(m_fq);
			break;
		case 1:     // FIR
			m_out = m_fir.Avg(m_fq);
			break;
		default:    // OFF
			m_out = m_fq;
			break;
	}
	m_d = d;
	m_Count++;
	return -(m_out * 16384);
}


CSSTVSET    SSTVSET;
LPCSTR SSTVModeList[]={
	"Robot 36", "Robot 72", "AVT 90", "Scottie 1", "Scottie 2", "ScottieDX",
	"Martin 1", "Martin 2", "SC2 180", "SC2 120", "SC2 60",
	"PD50", "PD90", "PD120", "PD160", "PD180", "PD240", "PD290",
	"P3", "P5", "P7",
	"MR73", "MR90", "MR115", "MR140", "MR175",
	"MP73", "MP115", "MP140", "MP175",
	"ML180", "ML240", "ML280", "ML320",
	"Robot 24", "B/W 8", "B/W 12",
    "MP73-N", "MP110-N", "MP140-N","MC110-N","MC140-N","MC180-N",
};
const BYTE SSTVModeOdr[]={
	smRM8,
	smRM12,
	smR24,
	smR36,
	smR72,
	smAVT,
	smSCT1,
	smSCT2,
	smSCTDX,
	smMRT1,
	smMRT2,
	smSC2_180,
	smSC2_120,
	smSC2_60,
	smPD50,
	smPD90,
	smPD120,
	smPD160,
	smPD180,
	smPD240,
	smPD290,
	smP3,
	smP5,
	smP7,
	smMP73,
	smMP115,
	smMP140,
	smMP175,
	smMR73,
	smMR90,
	smMR115,
	smMR140,
	smMR175,
	smML180,
	smML240,
	smML280,
	smML320,
    smMN73,
    smMN110,
    smMN140,
    smMC110,
    smMC140,
    smMC180,
};
//--------------------------------------------------------
BOOL __fastcall IsNarrowMode(int mode)
{
	switch(mode){
        case smMN73:
        case smMN110:
        case smMN140:
        case smMC110:
        case smMC140:
        case smMC180:
        	return TRUE;
        default:
        	return FALSE;
    }
}
//--------------------------------------------------------
// CSSTVDEMクラス
CSSTVSET::CSSTVSET()
{
	m_fNarrow = m_fTxNarrow = 0;
	m_TxMode = smSCT1;
	SetMode(smSCT1);
	InitIntervalPara();
}

void CSSTVSET::InitIntervalPara(void)
{
	for( int i = 0; i < smEND; i++ ){
		m_MS[i] = GetTiming(i) * m_SampFreq / 1000.0;
	}
	m_MS[2] = 0;                                 // AVT
//    m_MSLL = 100.0 * m_SampFreq / 1000.0;         // Lowest
//    m_MSL = 147.0 * m_SampFreq / 1000.0;         // Lowest
//    m_MSH = 1050.0 * 3 * m_SampFreq / 1000.0;    // Highest
	m_MSLL = 50.0 * m_SampFreq / 1000.0;         // Lowest
	m_MSL = 63.0 * m_SampFreq / 1000.0;         // Lowest
	m_MSH = 1390.0 * 3 * m_SampFreq / 1000.0;    // Highest
}

void CSSTVSET::SetMode(int mode)
{
	m_SampFreq = sys.m_SampFreq;
	m_Mode = mode;
    m_fNarrow = IsNarrowMode(mode);
	SetSampFreq();
	m_WD = int(m_TW);
	m_LM = int(m_TW * m_L) + 1;
}

void CSSTVSET::SetTxMode(int mode)
{
	m_TxSampFreq = sys.m_SampFreq + sys.m_TxSampOff;
	m_TxMode = mode;
    m_fTxNarrow = IsNarrowMode(mode);
	SetTxSampFreq();
	m_TWD = int(m_TTW);
}

void CSSTVSET::GetBitmapSize(int &w, int &h, int mode)
{
	switch(mode){
		case smPD120:
		case smPD180:
		case smPD240:
		case smP3:
		case smP5:
		case smP7:
		case smML180:
		case smML240:
		case smML280:
		case smML320:
			w = 640;
			h = 496;
			break;
		case smPD160:
			w = 512;
			h = 400;
			break;
		case smPD290:
			w = 800;
			h = 616;
			break;
		default:        // SCT1
			w = 320;
			h = 256;
			break;
	}
}

void CSSTVSET::GetPictureSize(int &w, int &h, int &hp, int mode)
{
	GetBitmapSize(w, h, mode);
	switch(mode){
		case smRM8:
		case smRM12:
		case smR24:
		case smR36:
		case smR72:
		case smAVT:
			hp = 240;
			break;
		default:
			hp = h;
	}
}

void CSSTVSET::SetSampFreq(void)
{
	switch(m_Mode){
		case smR36:
			m_KS = 88.0 * m_SampFreq / 1000.0;
			m_KS2 = 44.0 * m_SampFreq / 1000.0;
			m_OF = 12.0 * m_SampFreq / 1000.0;
//            m_OFP = 10.8 * m_SampFreq / 1000.0;
			m_OFP = 10.7 * m_SampFreq / 1000.0;
			m_SG = (88.0 + 1.25) * m_SampFreq / 1000.0;
			m_CG = (88.0 + 3.5) * SampFreq/1000.0;
			m_SB = 94.0 * m_SampFreq / 1000.0;
			m_CB = m_SB + m_KS2;
			m_L = 240;
			break;
		case smR72:
			m_KS = 138.0 * m_SampFreq / 1000.0;
			m_KS2 = 69.0 * m_SampFreq / 1000.0;
			m_OF = 12.0 * m_SampFreq / 1000.0;
			m_OFP = 10.7 * m_SampFreq / 1000.0;
			m_SG = 144.0 * m_SampFreq / 1000.0;
			m_CG = m_SG + m_KS2;
			m_SB = 219.0 * m_SampFreq / 1000.0;
			m_CB = m_SB + m_KS2;
			m_L = 240;
			break;
		case smAVT:
			m_KS = 125.0 * m_SampFreq / 1000.0;
			m_OF = 0.0 * m_SampFreq / 1000.0;
			m_OFP = 0.0 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 240;
			break;
		case smSCT2:
			m_KS = 88.064 * m_SampFreq / 1000.0;
			m_OF = 10.5 * m_SampFreq / 1000.0;
			m_OFP = 10.8 * m_SampFreq / 1000.0;
			m_SG = 89.564 * m_SampFreq / 1000.0;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 256;
			break;
		case smSCTDX:
			m_KS = 345.6 * m_SampFreq / 1000.0;
			m_OF = 10.5 * m_SampFreq / 1000.0;
//            m_OFP = 9.5 * m_SampFreq / 1000.0;
			m_OFP = 10.2 * m_SampFreq / 1000.0;
			m_SG = 347.1 * m_SampFreq / 1000.0;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 256;
			break;
		case smMRT1:
			m_KS = 146.432 * m_SampFreq / 1000.0;
			m_OF = 5.434 * m_SampFreq / 1000.0;
//            m_OFP = 7.3 * m_SampFreq / 1000.0;
			m_OFP = 7.2 * m_SampFreq / 1000.0;
			m_SG = 147.004 * m_SampFreq / 1000.0;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 256;
			break;
		case smMRT2:
			m_KS = 73.216 * m_SampFreq / 1000.0;
			m_OF = 5.434 * m_SampFreq / 1000.0;
			m_OFP = 7.4 * m_SampFreq / 1000.0;
			m_SG = 73.788 * m_SampFreq / 1000.0;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 256;
			break;
		case smSC2_180:
			m_KS = 235.0 * m_SampFreq / 1000.0;
			m_OF = 6.0437 * m_SampFreq / 1000.0;
//            m_OFP = 7.5 * m_SampFreq / 1000.0;
			m_OFP = 7.8 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 256;
			break;
		case smSC2_120:
			m_KS = 156.5 * m_SampFreq / 1000.0;
			m_OF = 6.02248 * m_SampFreq / 1000.0;
			m_OFP = 7.5 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 256;
			break;
		case smSC2_60:
			m_KS = 78.128 * m_SampFreq / 1000.0;
			m_OF = 6.0006 * m_SampFreq / 1000.0;
			m_OFP = 7.9 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 256;
			break;
		case smPD50:
			m_KS = 91.520 * m_SampFreq / 1000.0;
			m_OF = 22.080 * m_SampFreq / 1000.0;
			m_OFP = 19.300 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 128;
			break;
		case smPD90:
			m_KS = 170.240 * m_SampFreq / 1000.0;
			m_OF = 22.080 * m_SampFreq / 1000.0;
			m_OFP = 18.900 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 128;
			break;
		case smPD120:
			m_KS = 121.600 * m_SampFreq / 1000.0;
			m_OF = 22.080 * m_SampFreq / 1000.0;
			m_OFP = 19.400 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 248;
			break;
		case smPD160:
			m_KS = 195.584 * m_SampFreq / 1000.0;
			m_OF = 22.080 * m_SampFreq / 1000.0;
			m_OFP = 18.900 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 200;
			break;
		case smPD180:
			m_KS = 183.04 * m_SampFreq / 1000.0;
			m_OF = 22.080 * m_SampFreq / 1000.0;
			m_OFP = 18.900 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 248;
			break;
		case smPD240:
			m_KS = 244.48 * m_SampFreq / 1000.0;
			m_OF = 22.080 * m_SampFreq / 1000.0;
			m_OFP = 18.900 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 248;
			break;
		case smPD290:
			m_KS = 228.80 * m_SampFreq / 1000.0;
			m_OF = 22.080 * m_SampFreq / 1000.0;
			m_OFP = 18.900 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 616/2;
			break;
		case smP3:
			m_KS = 133.333 * m_SampFreq / 1000.0;
			m_OF = (5.208 + 1.042) * m_SampFreq / 1000.0;
			m_OFP = 7.80 * m_SampFreq / 1000.0;
			m_SG = (133.333 + 1.042) * m_SampFreq / 1000.0;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 496;
			break;
		case smP5:
			m_KS = 200.000 * m_SampFreq / 1000.0;
			m_OF = (7.813 + 1.562375) * m_SampFreq / 1000.0;
			m_OFP = 9.20 * m_SampFreq / 1000.0;
			m_SG = (200.000 + 1.562375) * m_SampFreq / 1000.0;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 496;
			break;
		case smP7:
			m_KS = 266.667 * m_SampFreq / 1000.0;
			m_OF = (10.417 + 2.083) * m_SampFreq / 1000.0;
			m_OFP = 11.50 * m_SampFreq / 1000.0;
			m_SG = (266.667 + 2.083) * m_SampFreq / 1000.0;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 496;
			break;
		case smMR73:
			//|--KS--|--KS2--|--KS2--|
			//      SG     CG=SB     CB
			m_KS = 138.0 * m_SampFreq / 1000.0;
			m_KS2 = m_KS * 0.5;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.6 * m_SampFreq / 1000.0;
			m_SG = m_KS + 0.1;
			m_CG = m_SG + m_KS2;
			m_SB = m_CG + 0.1;
			m_CB = m_SB + m_KS2;
			m_L = 256;
			break;
		case smMR90:
			m_KS = 171.0 * m_SampFreq / 1000.0;
			m_KS2 = m_KS * 0.5;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.6 * m_SampFreq / 1000.0;
			m_SG = m_KS + 0.1;
			m_CG = m_SG + m_KS2;
			m_SB = m_CG + 0.1;
			m_CB = m_SB + m_KS2;
			m_L = 256;
			break;
		case smMR115:
			m_KS = 220.0 * m_SampFreq / 1000.0;
			m_KS2 = m_KS * 0.5;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.6 * m_SampFreq / 1000.0;
			m_SG = m_KS + 0.1;
			m_CG = m_SG + m_KS2;
			m_SB = m_CG + 0.1;
			m_CB = m_SB + m_KS2;
			m_L = 256;
			break;
		case smMR140:
			m_KS = 269.0 * m_SampFreq / 1000.0;
			m_KS2 = m_KS * 0.5;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.6 * m_SampFreq / 1000.0;
			m_SG = m_KS + 0.1;
			m_CG = m_SG + m_KS2;
			m_SB = m_CG + 0.1;
			m_CB = m_SB + m_KS2;
			m_L = 256;
			break;
		case smMR175:
			m_KS = 337.0 * m_SampFreq / 1000.0;
			m_KS2 = m_KS * 0.5;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.6 * m_SampFreq / 1000.0;
			m_SG = m_KS + 0.1;
			m_CG = m_SG + m_KS2;
			m_SB = m_CG + 0.1;
			m_CB = m_SB + m_KS2;
			m_L = 256;
			break;
		case smMP73:
			m_KS = 140.0 * m_SampFreq / 1000.0;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.5 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 128;
			break;
		case smMP115:
			m_KS = 223.0 * m_SampFreq / 1000.0;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.5 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 128;
			break;
		case smMP140:
			m_KS = 270.0 * m_SampFreq / 1000.0;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.5 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 128;
			break;
		case smMP175:
			m_KS = 340.0 * m_SampFreq / 1000.0;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.5 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 128;
			break;
		case smML180:
			m_KS = 176.5 * m_SampFreq / 1000.0;
			m_KS2 = m_KS * 0.5;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.6 * m_SampFreq / 1000.0;
			m_SG = m_KS + 0.1;
			m_CG = m_SG + m_KS2;
			m_SB = m_CG + 0.1;
			m_CB = m_SB + m_KS2;
			m_L = 496;
			break;
		case smML240:
			m_KS = 236.5 * m_SampFreq / 1000.0;
			m_KS2 = m_KS * 0.5;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.6 * m_SampFreq / 1000.0;
			m_SG = m_KS + 0.1;
			m_CG = m_SG + m_KS2;
			m_SB = m_CG + 0.1;
			m_CB = m_SB + m_KS2;
			m_L = 496;
			break;
		case smML280:
			m_KS = 277.5 * m_SampFreq / 1000.0;
			m_KS2 = m_KS * 0.5;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.6 * m_SampFreq / 1000.0;
			m_SG = m_KS + 0.1;
			m_CG = m_SG + m_KS2;
			m_SB = m_CG + 0.1;
			m_CB = m_SB + m_KS2;
			m_L = 496;
			break;
		case smML320:
			m_KS = 317.5 * m_SampFreq / 1000.0;
			m_KS2 = m_KS * 0.5;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.6 * m_SampFreq / 1000.0;
			m_SG = m_KS + 0.1;
			m_CG = m_SG + m_KS2;
			m_SB = m_CG + 0.1;
			m_CB = m_SB + m_KS2;
			m_L = 496;
			break;
		case smR24:
			m_KS = 92.0 * m_SampFreq / 1000.0;
			m_KS2 = 46.0 * m_SampFreq / 1000.0;
			m_OF = 8.0 * m_SampFreq / 1000.0;
			m_OFP = 8.1 * m_SampFreq / 1000.0;
			m_SG = m_KS + 4.0 * m_SampFreq / 1000.0;
			m_CG = m_SG + m_KS2;
			m_SB = m_CG + 4.0 * m_SampFreq / 1000.0;
			m_CB = m_SB + m_KS2;
			m_L = 120;
			break;
		case smRM8:
			m_KS = 58.89709 * m_SampFreq / 1000.0;
			m_OF = 8.0 * m_SampFreq / 1000.0;
			m_OFP = 8.2 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 120;
			break;
		case smRM12:
			m_KS = 92.0 * m_SampFreq / 1000.0;
			m_OF = 8.0 * m_SampFreq / 1000.0;
			m_OFP = 8.0 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 120;
			break;
		case smMN73:
			m_KS = 140.0 * m_SampFreq / 1000.0;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.5 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 128;
			break;
		case smMN110:
			m_KS = 212.0 * m_SampFreq / 1000.0;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.5 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 128;
			break;
		case smMN140:
			m_KS = 270.0 * m_SampFreq / 1000.0;
			m_OF = 10.0 * m_SampFreq / 1000.0;
			m_OFP = 10.5 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 128;
			break;
        case smMC110:
			m_KS = 140.0 * m_SampFreq / 1000.0;
			m_OF = 8.0 * m_SampFreq / 1000.0;
			m_OFP = 8.95 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 256;
			break;
        case smMC140:
			m_KS = 180.0 * m_SampFreq / 1000.0;
			m_OF = 8.0 * m_SampFreq / 1000.0;
			m_OFP = 8.75 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 256;
			break;
        case smMC180:
			m_KS = 232.0 * m_SampFreq / 1000.0;
			m_OF = 8.0 * m_SampFreq / 1000.0;
			m_OFP = 8.75 * m_SampFreq / 1000.0;
			m_SG = m_KS;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 256;
			break;

//        case smSCT1:
		default:        // SCT1
			m_KS = 138.24 * m_SampFreq / 1000.0;
			m_OF = 10.5 * m_SampFreq / 1000.0;
			m_OFP = 10.7 * m_SampFreq / 1000.0;
			m_SG = 139.74 * m_SampFreq / 1000.0;
			m_CG = m_KS + m_SG;
			m_SB = m_SG + m_SG;
			m_CB = m_KS + m_SB;
			m_L = 256;
			break;
	}
	m_TW = GetTiming(m_Mode) * m_SampFreq / 1000.0;
	switch(m_Mode){
		case smPD120:
		case smPD160:
		case smPD180:
		case smPD240:
		case smPD290:
		case smP3:
		case smP5:
		case smP7:
			m_KSS = (m_KS - m_KS/480.0);      // TW for Y or RGB mode
			m_KS2S = (m_KS2 - m_KS2/480.0);   // TW for Ry, By
			m_KSB = (m_KSS / 1280.0);          // TW for black adjutment
			break;
		case smMP73:
        case smMN73:
		case smSCTDX:
			m_KSS = (m_KS - m_KS/1280.0);      // TW for Y or RGB mode
			m_KS2S = (m_KS2 - m_KS2/1280.0);   // TW for Ry, By
			m_KSB = m_KSS / 1280.0;
			break;
		case smSC2_180:
		case smMP115:
		case smMP140:
		case smMP175:
		case smMR90:
		case smMR115:
		case smMR140:
		case smMR175:
		case smML180:
		case smML240:
		case smML280:
		case smML320:
        case smMN110:
        case smMN140:
        case smMC110:
        case smMC140:
        case smMC180:
			m_KSS = m_KS;                   // TW for Y or RGB mode
			m_KS2S = m_KS2;                 // TW for Ry, By
			m_KSB = m_KSS / 1280.0;
			break;
		case smMR73:
			m_KSS = (m_KS - m_KS/640.0);      // TW for Y or RGB mode
			m_KS2S = (m_KS2 - m_KS2/1024.0);  // TW for Ry, By
			m_KSB = m_KSS / 1024.0;
			break;
		default:
			m_KSS = (m_KS - m_KS/240.0);      // TW for Y or RGB mode
			m_KS2S = (m_KS2 - m_KS2/240.0);   // TW for Ry, By
			m_KSB = (m_KSS / 640.0);          // TW for black adjutment
			break;
	}
	switch(m_Mode){
		case smMRT1:
		case smMRT2:
		case smSC2_60:
		case smSC2_120:
		case smSC2_180:
    	case smMC110:
        case smMC140:
        case smMC180:
			m_AFCW = 2.0 * SampFreq / 1000.0;
			m_AFCB = 1.0 * SampFreq / 1000.0;
			break;
		default:
			m_AFCW = 3.0 * SampFreq / 1000.0;
			m_AFCB = 1.5 * SampFreq / 1000.0;
			break;
	}
	if( !m_KSB ) m_KSB++;

    if( sys.m_bCQ100 ){
    	double d = m_OFP * 1000.0 / SampFreq;
		m_OFP = (d + (1100.0/g_dblToneOffset)) * SampFreq / 1000.0;
    }
	m_AFCE = m_AFCB + m_AFCW;
}

double CSSTVSET::GetTiming(int mode)
{
	switch(mode){
		case smR36:
			return 150.0;
		case smR72:
			return 300.0;
		case smAVT:
			return 375;
		case smSCT2:
			return 277.692;
		case smSCTDX:
			return 1050.3;
		case smMRT1:
			return 446.446;
		case smMRT2:
			return 226.798;
		case smSC2_180:
			return 711.0437;
		case smSC2_120:
			return 475.52248;
		case smSC2_60:
			return 240.3846;
		case smPD50:
			return 388.160;
		case smPD90:
			return 703.040;
		case smPD120:
			return 508.480;
		case smPD160:
			return 804.416;
		case smPD180:
			return 754.24;
		case smPD240:
			return 1000.00;
		case smPD290:
			return 937.28;
		case smP3:
			return 409.375;
		case smP5:
			return 614.0625;
		case smP7:
			return 818.75;
		case smMR73:
			return 286.3;
		case smMR90:
			return 352.3;
		case smMR115:
			return 450.3;
		case smMR140:
			return 548.3;   //269*2 + 10;
		case smMR175:
			return 684.3;   //337*2 + 10;
		case smMP73:
			return 570.0;
		case smMP115:
			return 902.0;
		case smMP140:
			return 1090.0;
		case smMP175:
			return 1370.0;
		case smML180:
			return 363.3;
		case smML240:
			return 483.3;
		case smML280:
			return 565.3;
		case smML320:
			return 645.3;
		case smR24:
			return 200.0;
		case smRM8:
			return 66.89709;
		case smRM12:
			return 100.0;
        case smMN73:
			return 570.0;
        case smMN110:
			return 858.0;
		case smMN140:
			return 1090.0;
		case smMC110:
			return 428.5;
        case smMC140:
        	return 548.5;
		case smMC180:
			return 704.5;
		default:    // smSCT1
			return 428.22;
	}
}

void CSSTVSET::SetTxSampFreq(void)
{
	int dm1, dm2;
	GetPictureSize(dm1, dm2, m_TL, m_TxMode);
	m_TTW = GetTiming(m_TxMode) * m_TxSampFreq / 1000.0;
}


//--------------------------------------------------------
// CSSTVDEMクラス
int CSYNCINT::SyncCheckSub(int am)
{
	int i = MSYNCLINE-1;

	int e;
	switch(am){
		case smSC2_60:
		case smSC2_120:
			return 0;
		case smR24:
		case smR36:
		case smMRT2:
		case smPD50:
		case smPD240:
			if( m_fNarrow ) return 0;
			e = MSYNCLINE - 4;
			break;
		case smRM8:
		case smRM12:
			if( m_fNarrow ) return 0;
			e = 0;
			break;
        case smMN73:
        case smMN110:
        case smMN140:
        case smMC110:
        case smMC140:
        case smMC180:
			if( !m_fNarrow ) return 0;
			e = MSYNCLINE - 5;
        	break;
		default:
			if( m_fNarrow ) return 0;
			e = MSYNCLINE - 3;
			break;
	}
	DWORD deff = (3 * SampFreq) / 1000;
	DWORD cml = SSTVSET.m_MS[am];
	DWORD cmh = cml + deff;
	cml -= deff;
	DWORD w;
	for( i--; i >= e; i-- ){
		w = m_MSyncList[i];
		int f = 0;
		if( w > SSTVSET.m_MSL ){
			if( m_fNarrow ){
				for( int k = 1; k <= 2; k++ ){
					DWORD ww = w / k;
					if( (ww > cml) && (ww < cmh) ) f = 1;
				}
			}
            else {
				for( int k = 1; k <= 3; k++ ){
					DWORD ww = w / k;
					if( (ww > cml) && (ww < cmh) ) f = 1;
				}
            }
		}
		if( !f ) return 0;
	}
	return 1;
}

int CSYNCINT::SyncCheck(void)
{
	DWORD deff = (3 * SampFreq) / 1000;

	DWORD w = m_MSyncList[MSYNCLINE-1];
	for( int k = 1; k <= 3; k++ ){
		DWORD ww = w / k;
		if( (ww > SSTVSET.m_MSL) && (ww < SSTVSET.m_MSH) ){
			for( int i = 0; i < smEND; i++ ){
				if( SSTVSET.m_MS[i] && (ww > (SSTVSET.m_MS[i]-deff)) && (ww < (SSTVSET.m_MS[i]+deff)) ){
					if( SyncCheckSub(i) ){
						return i + 1;
					}
				}
			}
		}
		else {
			break;
		}
	}
	return 0;
}

void CSYNCINT::SyncInc(void)
{
	m_MSyncCnt++;
}

void CSYNCINT::SyncTrig(int d)
{
	m_MSyncIntMax = d;
	m_MSyncIntPos = m_MSyncCnt;
}

void CSYNCINT::SyncMax(int d)
{
	if( m_MSyncIntMax < d ){
		m_MSyncIntMax = d;
		m_MSyncIntPos = m_MSyncCnt;
	}
}

int CSYNCINT::SyncStart(void)
{
	int ss = 0;
	if( m_MSyncIntMax ){
		if( (m_MSyncIntPos - m_MSyncACnt) > SSTVSET.m_MSLL ){
			m_MSyncACnt = m_MSyncIntPos - m_MSyncACnt;
			memcpy(m_MSyncList, &m_MSyncList[1], sizeof(int) * (MSYNCLINE - 1));
			m_MSyncList[MSYNCLINE - 1] = m_MSyncACnt;
			if( m_MSyncACnt > SSTVSET.m_MSL ){
				ss = SyncCheck();
			}
			m_MSyncACnt = m_MSyncIntPos;
		}
		m_MSyncIntMax = 0;
	}
	return ss;
}
//--------------------------------------------------------
// CSSTVDEMクラス
CSSTVDEM::CSSTVDEM()
{
	m_bpf = 1;      // wide
	m_ad = 0;

	m_StgBuf = NULL;
	m_StgB12 = NULL;
	m_BWidth = 1400 * SampFreq / 1000;
	int n = SSTVDEMBUFMAX * m_BWidth;
	m_Buf = new short[n];
	m_B12 = new short[n];
	::VirtualLock(m_Buf, n);
	::VirtualLock(m_B12, n);
	memset(m_Buf, 0, n);
	memset(m_B12, 0, n);

	m_pll.SetSampleFreq(SampFreq);
	m_pll.SetVcoGain(1.0);
	m_pll.SetFreeFreq(1500, 2300);
	m_pll.m_loopOrder = 1;
	m_pll.m_loopFC = 1500;
	m_pll.m_outOrder = 3;
	m_pll.m_outFC = 900;
	m_pll.MakeLoopLPF();
	m_pll.MakeOutLPF();

	memset(HBPF, 0, sizeof(HBPF));
	memset(HBPFS, 0, sizeof(HBPFS));
	memset(HBPFN, 0, sizeof(HBPFN));
//	memset(Z, 0, sizeof(Z));
	CalcBPF();

	m_iir11.SetFreq(1080 + g_dblToneOffset, SampFreq, 80.0);
	m_iir12.SetFreq(1200 + g_dblToneOffset, SampFreq, 100.0);
	m_iir13.SetFreq(1320 + g_dblToneOffset, SampFreq, 80.0);
	m_iir19.SetFreq(1900 + g_dblToneOffset, SampFreq, 100.0);
	m_iirfsk.SetFreq(FSKSPACE + g_dblToneOffset, SampFreq, 100.0);
	m_lpf11.MakeIIR(50, SampFreq, 2, 0, 0);
	m_lpf12.MakeIIR(50, SampFreq, 2, 0, 0);
	m_lpf13.MakeIIR(50, SampFreq, 2, 0, 0);
	m_lpf19.MakeIIR(50, SampFreq, 2, 0, 0);
	m_lpffsk.MakeIIR(50, SampFreq, 2, 0, 0);

	pRep = NULL;

	m_wPage = m_rPage = 0;
	m_wBase = 0;
	m_wCnt = 0;
	m_rBase = 0;
	m_Skip = 0;
	m_Sync = 0;
	m_SyncMode = 0;
	m_ScopeFlag = 0;
	m_LoopBack = 0;
	m_Lost = 0;

	m_lvl.m_agcfast = 1;
	m_afc = 1;

	m_Tick = 0;
	pTick = NULL;
	m_Avg.SetCount(2.5*SampFreq/1000.0);
	m_AFCAVG.SetCount(15);
	m_AFCFQ = 0;
	m_AFCInt = 100 * SampFreq / 1000.0;
	m_AFCDis = 0;

	m_sint1.Reset();
	m_sint2.Reset();
	m_sint3.m_fNarrow = TRUE;
    m_sint3.Reset();
	m_MSync = 1;                                    // Sync remote start ON
	m_SyncRestart = 1;
	m_SyncAVT = 0;

	m_SenseLvl = 1;
	SetSenseLvl();

	m_Type = 2;
	m_ReqSave = 0;
	m_LevelType = 0;

	m_fskrec = 0;
    m_fskNRrec = 0;
	m_fskdecode = 0;
	m_fskmode = 0;

	m_Repeater = 0;
	m_RepSQ = 6000;
	m_RepTone = 1750;
	m_repmode = 0;
	m_repANS = m_repRLY = m_repRX = m_repTX = 0;
	InitRepeater();
	SetRepSenseLvl();
}

CSSTVDEM::~CSSTVDEM()
{
	::VirtualUnlock(m_Buf, SSTVDEMBUFMAX * m_BWidth);
	::VirtualUnlock(m_B12, SSTVDEMBUFMAX * m_BWidth);
	delete m_B12;
	delete m_Buf;
	FreeRxBuff();
	if( pRep != NULL ){
		delete pRep;
	}
}

void CSSTVDEM::CalcBPF(double *H1, double *H2, double *H3, int &bpftap, int bpf, int mode)
{
	int lfq = (m_SyncRestart ? 1100 : 1200) + g_dblToneOffset;
	int lfq2 =  400 + g_dblToneOffset;
    if( lfq2 < 50 ) lfq2 = 50;
	switch(bpf){
		case 1:     // Wide
			bpftap = 24 * SampFreq / 11025.0;
			MakeFilter(H1, bpftap, ffBPF, SampFreq, lfq, 2600 + g_dblToneOffset, 20, 1.0);
			MakeFilter(H2, bpftap, ffBPF, SampFreq,  lfq2, 2500 + g_dblToneOffset, 20, 1.0);
//			MakeFilter(H3, bpftap, ffBPF, SampFreq,  NARROW_BPFLOW-200, NARROW_BPFHIGH, 20, 1.0);
			break;
		case 2:     // Narrow
			bpftap = 64 * SampFreq / 11025.0;
			MakeFilter(H1, bpftap, ffBPF, SampFreq, lfq, 2500 + g_dblToneOffset, 40, 1.0);
			MakeFilter(H2, bpftap, ffBPF, SampFreq,  lfq2, 2500 + g_dblToneOffset, 20, 1.0);
//			MakeFilter(H3, bpftap, ffBPF, SampFreq, NARROW_BPFLOW-100, NARROW_BPFHIGH, 40, 1.0);
			break;
		case 3:     // Very Narrow
			bpftap = 96 * SampFreq / 11025.0;
			MakeFilter(H1, bpftap, ffBPF, SampFreq, lfq, 2400 + g_dblToneOffset, 50, 1.0);
			MakeFilter(H2, bpftap, ffBPF, SampFreq,  lfq2, 2500 + g_dblToneOffset, 20, 1.0);
//			MakeFilter(H3, bpftap, ffBPF, SampFreq,  NARROW_BPFLOW, NARROW_BPFHIGH, 50, 1.0);
			break;
		default:
			bpftap = 0;
			break;
	}
    CalcNarrowBPF(H3, bpftap, bpf, mode);
}

void CSSTVDEM::CalcNarrowBPF(double *H3, int bpftap, int bpf, int mode)
{
	int low, high;
	switch(mode){
		case smMN73:
			low = 1600; high = 2500;
			break;
        case smMN110:
			low = 1600; high = 2500;
        	break;
        case smMN140:
			low = 1700; high = 2400;
        	break;
        case smMC110:
			low = 1600; high = 2500;
        	break;
        case smMC140:
			low = 1650; high = 2500;
        	break;
        case smMC180:
			low = 1700; high = 2400;
        	break;
        default:
			low = 1600; high = 2500;
        	break;
    }
	low += g_dblToneOffset;
    high += g_dblToneOffset;
	switch(bpf){
		case 1:     // Wide
			MakeFilter(H3, bpftap, ffBPF, SampFreq,  low-200, high, 20, 1.0);
			break;
		case 2:     // Narrow
			MakeFilter(H3, bpftap, ffBPF, SampFreq, low-100, high, 40, 1.0);
			break;
		case 3:     // Very Narrow
			MakeFilter(H3, bpftap, ffBPF, SampFreq,  low, high, 50, 1.0);
			break;
		default:
			break;
	}
}

void CSSTVDEM::CalcBPF(void)
{
	CalcBPF(HBPF, HBPFS, HBPFN, m_bpftap, m_bpf, SSTVSET.m_Mode);
	m_BPF.Create(m_bpftap);
}

void CSSTVDEM::SetBPF(int bpf)
{
	if( bpf != m_bpf ){
		int delay = m_bpftap;
		m_bpf = bpf;
		CalcBPF();
		if( m_Sync ){
			delay = (m_bpftap - delay) / 2;
			m_Skip = delay;
		}
	}
}

void CSSTVDEM::FreeRxBuff(void)
{
	if( m_StgBuf != NULL ){
		delete m_StgB12;
		delete m_StgBuf;
		m_StgBuf = NULL;
		m_StgB12 = NULL;
		m_wStgLine = 0;
	}
}

void CSSTVDEM::OpenCloseRxBuff(void)
{
	if( m_Sync ) return;

	if( sys.m_UseRxBuff == 1 ){
		if( m_StgBuf == NULL ){
			int n = 257 * 1100 * SampFreq / 1000;
			m_StgBuf = new short[n];
			m_StgB12 = new short[n];
			memset(m_StgBuf, 0, n);
			memset(m_StgB12, 0, n);
			m_RxBufAllocSize = n;
			m_wStgLine = 0;
		}
	}
	else {
		FreeRxBuff();
	}
}

void CSSTVDEM::Idle(double d)
{
	if( !sys.m_TestDem ) m_lvl.Do(d);
}

void CSSTVDEM::SetTickFreq(int f)
{
	if( !f ) f = 1200;
	m_iir12.SetFreq(f + g_dblToneOffset, SampFreq, 100.0);
	m_TickFreq = f;
}

void CSSTVDEM::InitAFC(void)
{
	m_AFCAVG.SetCount(m_AFCAVG.Max);
	if( m_fNarrow ){
		m_AFCData = m_AFCLock = (NARROW_CENTER-NARROW_SYNC)*16384/NARROW_BWH;
    }
    else {
		m_AFCData = m_AFCLock = ((1900-1200)*16384)/400.0;
    }
	m_AFCFlag = 0;
	m_AFCDiff = 0.0;
	m_AFCGard = 10;
	m_AFCCount = 0;
	m_AFCDis = 0;
	InitTone(0);
	if( m_fNarrow ){
		m_AFC_LowVal = (NARROW_CENTER - NARROW_AFCLOW) * 16384.0 / NARROW_BWH;		// (Center - SyncLow) * 16384 / BWH
		m_AFC_HighVal = (NARROW_CENTER - NARROW_AFCHIGH) * 16384.0 / NARROW_BWH;	// (Center - SyncHigh) * 16384 / BWH
		m_AFC_SyncVal = (NARROW_CENTER - NARROW_SYNC) * 16384.0 / NARROW_BWH;		// (Center - Sync) * 16384 / BWH
		m_AFC_BWH = NARROW_BWH / 16384.0;											// BWH / 16384.0;
		if( sys.m_bCQ100 ){
			m_AFC_LowVal = (NARROW_CENTER - NARROW_SYNC - 50) * 16384.0 / NARROW_BWH;		// (Center - SyncLow) * 16384 / BWH
			m_AFC_HighVal = (NARROW_CENTER - NARROW_SYNC + 50) * 16384.0 / NARROW_BWH;	// (Center - SyncHigh) * 16384 / BWH
		}
    }
    else {
		m_AFC_LowVal = (1900 - 1000) * 16384.0 / 400;	// (Center - SyncLow) * 16384 / BWH
		m_AFC_HighVal = (1900 - 1325) * 16384.0 / 400;	// (Center - SyncHigh) * 16384 / BWH
		m_AFC_SyncVal = (1900 - 1200) * 16384.0 / 400;	// (Center - Sync) * 16384 / BWH
		m_AFC_BWH = 400 / 16384.0;						// BWH / 16384.0;
		if( sys.m_bCQ100 ){
			m_AFC_LowVal = (1900 - 1200 - 50) * 16384.0 / 400;	// (Center - SyncLow) * 16384 / BWH
			m_AFC_HighVal = (1900 - 1200 + 50) * 16384.0 / 400;	// (Center - SyncHigh) * 16384 / BWH
        }
    }
}

void CSSTVDEM::InitTone(int dfq)
{
	if( m_AFCFQ != dfq ){
		m_iir11.SetFreq(1080+dfq + g_dblToneOffset, SampFreq, 80.0);
		m_iir12.SetFreq(1200+dfq + g_dblToneOffset, SampFreq, 100.0);
		m_iir13.SetFreq(1320+dfq + g_dblToneOffset, SampFreq, 80.0);
		m_iir19.SetFreq(1900+dfq + g_dblToneOffset, SampFreq, 100.0);
		m_iirfsk.SetFreq(FSKSPACE+dfq + g_dblToneOffset, SampFreq, 100.0);
		m_AFCFQ = dfq;
	}
}

void CSSTVDEM::SetWidth(int fNarrow)
{
	if( m_fNarrow != fNarrow ){
		m_fNarrow = fNarrow;
	    m_hill.SetWidth(fNarrow);
    	m_fqc.SetWidth(fNarrow);
	    m_pll.SetWidth(fNarrow);
    }
}

void CSSTVDEM::Start(void)
{
	SetWidth(IsNarrowMode(SSTVSET.m_Mode));

	InitAFC();
	m_fqc.Clear();
	m_SyncMode = -1;
	m_Sync = 0;
	m_Skip = 0;
	m_wPage = m_rPage = 0;
	m_wBase = 0;
	m_wLine = 0;
	m_wCnt = 0;
	m_rBase = 0;
	OpenCloseRxBuff();
	m_wBgn = 2;
	m_Lost = 0;

	int eg = SSTVSET.m_WD + SSTVSET.m_KSB + SSTVSET.m_KSB;
	int i, j;
	for( i = 0; i < SSTVDEMBUFMAX; i++ ){
		for( j = SSTVSET.m_WD; j < eg; j++ ){
			m_Buf[i*m_BWidth + j] = -16384;
		}
	}

	m_Sync = 1;
	m_SyncMode = 0;
    SetWidth(m_fNarrow);
    if( m_fNarrow ) CalcNarrowBPF(HBPFN, m_bpftap, m_bpf, SSTVSET.m_Mode);
}

void CSSTVDEM::Start(int mode, int f)
{
	m_fqc.Clear();
	m_sint1.Reset();
	m_sint2.Reset();
    m_sint3.Reset();
	m_wBgn = 0;
	m_rBase = 0;
	m_SyncMode = 0;
	SSTVSET.SetMode(mode);
	m_Sync = 0;
    SetWidth(IsNarrowMode(mode));
	if( f ){
		Start();
	}
	else {
		m_SyncMode = -1;
	}
}

void CSSTVDEM::Stop(void)
{
	if( m_AFCFQ ){
		if( m_fskdecode ){
			m_iir11.SetFreq(1080 + g_dblToneOffset, SampFreq, 80.0);
			m_iir12.SetFreq(1200 + g_dblToneOffset, SampFreq, 100.0);
			m_iir13.SetFreq(1320 + g_dblToneOffset, SampFreq, 80.0);
		}
		else {
			InitTone(0);
		}
	}
	m_fqc.Clear();
	m_sint1.Reset();
	m_sint2.Reset();
    m_sint3.Reset();
	m_wBgn = 0;
	m_SyncMode = 512;
	m_Sync = 0;
	m_SyncAVT = 0;
	m_Skip = 0;
	SetWidth(0);
}

void CSSTVDEM::SetSenseLvl(void)
{
	switch(m_SenseLvl){
		case 1:
			m_SLvl = 3500;
			m_SLvl2 = m_SLvl * 0.5;
			m_SLvl3 = 5700;
			break;
		case 2:
			m_SLvl = 4800;
			m_SLvl2 = m_SLvl * 0.5;
			m_SLvl3 = 6800;
			break;
		case 3:
			m_SLvl = 6000;
			m_SLvl2 = m_SLvl * 0.5;
			m_SLvl3 = 8000;
			break;
		default:
			m_SLvl = 2400;
			m_SLvl2 = m_SLvl * 0.5;
			m_SLvl3 = 5000;
			break;
	}
}

void CSSTVDEM::Do(double s)
{
	if( (s > 24578.0) || (s < -24578.0) ){
		m_OverFlow = 1;
	}
	double d = (s + m_ad) * 0.5;    // LPF
	m_ad = s;
	if( m_bpf ){
		if( m_Sync || (m_SyncMode >= 3) ){
			d = m_BPF.Do(d, m_fNarrow ? HBPFN : HBPF);
        }
        else {
			d = m_BPF.Do(d, HBPFS);
        }
	}
	m_lvl.Do(d);
	double ad = m_lvl.AGC(d);

	d = ad * 32;
	if( d > 16384.0 ) d = 16384.0;
	if( d < -16384.0 ) d = -16384.0;

	double d11;
	double d12;
	double d13;
	double d19;
	double dsp;

	d12 = m_iir12.Do(d);
	if( d12 < 0.0 ) d12 = -d12;
	d12 = m_lpf12.Do(d12);

	d19 = m_iir19.Do(d);
	if( d19 < 0.0 ) d19 = -d19;
	d19 = m_lpf19.Do(d19);

	dsp = m_iirfsk.Do(d);
	if( dsp < 0.0 ) dsp = -dsp;
	dsp = m_lpffsk.Do(dsp);
	DecodeFSK(int(d19), int(dsp));

	if( m_Repeater && !m_Sync && (pRep != NULL) ){
		double dsp;
		dsp = pRep->m_iirrep.Do(d);
		if( dsp < 0.0 ) dsp = -dsp;
		dsp = pRep->m_lpfrep.Do(dsp);
		if( m_RepSQ ){
			m_repsig = pRep->m_lmsrep.Sig(m_ad);
		}
		Repeater(int(dsp), int(d12), int(d19));
	}

	if( m_fNarrow ){
		if( m_ScopeFlag ){
			m_Scope[0].WriteData(d19);
		}
		if( m_LevelType ) m_SyncLvl.Do(d19);
    }
    else {
		if( m_ScopeFlag ){
			m_Scope[0].WriteData(d12);
		}
		if( m_LevelType ) m_SyncLvl.Do(d12);

    }
	if( m_Tick && (pTick != NULL) ){
		pTick->Write(d12);
		return;
	}

	if( !m_Sync || m_SyncRestart || m_SyncAVT ){
		m_sint1.SyncInc();
		m_sint2.SyncInc();
        m_sint3.SyncInc();
		d11 = m_iir11.Do(d);
		if( d11 < 0.0 ) d11 = -d11;
		d11 = m_lpf11.Do(d11);

		switch(m_SyncMode){
			case 0:                 // 自動開始
				if( !m_Sync && m_MSync ){
					m_VisData = m_sint1.SyncStart();
					if( m_VisData > 0 ){
						SSTVSET.SetMode(m_VisData-1);
						Start();
					}
					else if( (d12 > d19) && (d12 > m_SLvl2) && ((d12-d19) >= m_SLvl2) ){
						m_sint2.SyncMax(d12);
					}
					else {
						m_VisData = m_sint2.SyncStart();
						if( m_VisData > 0 ){
							m_VisData--;
							switch(m_VisData){
								case smSCT1:
								case smMRT1:
								case smMRT2:
								case smSC2_180:
									SSTVSET.SetMode(m_VisData);
									Start();
									break;
								default:
									break;
							}
						}
					}
#if NARROW_SYNC == 1900
					if( (d19 > d12) && (d19 > dsp) && (d19 > m_SLvl3) && ((d19-d12) >= m_SLvl3) && ((d19-dsp) >= m_SLvl) ){
						if( m_sint3.m_SyncPhase ){
							m_sint3.SyncMax(d19);
						}
						else {
							m_sint3.SyncTrig(d19);
							m_sint3.m_SyncPhase++;
						}
					}
					else if( m_sint3.m_SyncPhase ){
						m_sint3.m_SyncPhase = 0;
						m_VisData = m_sint3.SyncStart();
						if( m_VisData > 0 ){
							m_VisData--;
							SSTVSET.SetMode(m_VisData);
							Start();
						}
					}
#endif
				}
				if( (d12 > d19) && (d12 > m_SLvl) && ((d12-d19) >= m_SLvl) ){
					m_SyncMode++;
					m_SyncTime = 15 * sys.m_SampFreq/1000;
					if( !m_Sync && m_MSync ) m_sint1.SyncTrig(d12);
				}
				break;
			case 1:                 // 1200Hz(30ms)の継続チェック
				if( !m_Sync && m_MSync ){
					if( (d12 > d19) && (d12 > m_SLvl2) && ((d12-d19) >= m_SLvl2) ){
						m_sint2.SyncMax(d12);
					}
				}
				if( (d12 > d19) && (d12 > m_SLvl) && ((d12-d19) >= m_SLvl) ){
					if( !m_Sync && m_MSync ){
						m_sint1.SyncMax(d12);
					}
					m_SyncTime--;
					if( !m_SyncTime ){
						m_SyncMode++;
						m_SyncTime = 30 * sys.m_SampFreq/1000;
						m_VisData = 0;
						m_VisCnt = 8;
					}
				}
				else {
					m_SyncMode = 0;
				}
				break;
			case 2:                 // Vis decode
			case 9:
				d13 = m_iir13.Do(d);
				if( d13 < 0.0 ) d13 = -d13;
				d13 = m_lpf13.Do(d13);
				m_SyncTime--;
				if( !m_SyncTime ){
					if( ((d11 < d19) && (d13 < d19)) ||
						(fabs(d11-d13) < (m_SLvl2)) ){
						m_SyncMode = 0;
					}
					else {
						m_SyncTime = 30 * sys.m_SampFreq/1000;
						m_VisData = m_VisData >> 1;
						if( d11 > d13 ) m_VisData |= 0x0080;
						m_VisCnt--;
						if( !m_VisCnt ){
							if( m_SyncMode == 2 ){
								m_SyncMode++;
								switch(m_VisData){
									case 0x82:      // RM8
										m_NextMode = smRM8;
										break;
									case 0x86:      // RM12
										m_NextMode = smRM12;
										break;
									case 0x84:      // R24
										m_NextMode = smR24;
										break;
									case 0x88:      // R36
										m_NextMode = smR36;
										break;
									case 0x0c:      // R72
										m_NextMode = smR72;
										break;
									case 0x44:      // AVT
										m_NextMode = smAVT;
										break;
									case 0x3c:      // SCT1
										m_NextMode = smSCT1;
										break;
									case 0xb8:      // SCT2
										m_NextMode = smSCT2;
										break;
									case 0xcc:      // SCTDX
										m_NextMode = smSCTDX;
										break;
									case 0xac:      // MRT1
										m_NextMode = smMRT1;
										break;
									case 0x28:      // MRT2
										m_NextMode = smMRT2;
										break;
									case 0xb7:      // SC2-180 $37 00110111
										m_NextMode = smSC2_180;
										break;
									case 0x3f:      // SC2-120 $3f 00111111
										m_NextMode = smSC2_120;
										break;
									case 0xbb:      // SC2-60 $3b 10111011
										m_NextMode = smSC2_60;
										break;
									case 0xdd:      // PD50 $5d  01011101
										m_NextMode = smPD50;
										break;
									case 0x63:      // PD90 $63  01100011
										m_NextMode = smPD90;
										break;
									case 0x5f:      // PD120 $5f  01011111
										m_NextMode = smPD120;
										break;
									case 0xe2:      // PD160 $62  11100010
										m_NextMode = smPD160;
										break;
									case 0x60:      // PD180 $60  01100000
										m_NextMode = smPD180;
										break;
									case 0xe1:      // PD240 $61  11100001
										m_NextMode = smPD240;
										break;
									case 0xde:      // PD290 $5e  11011110
										m_NextMode = smPD290;
										break;
									case 0x71:      // P3 $71  01110001
										m_NextMode = smP3;
										break;
									case 0x72:      // P5 $71  01110010
										m_NextMode = smP5;
										break;
									case 0xf3:      // P7 $73  11110011
										m_NextMode = smP7;
										break;
									case 0x23:      // MM 拡張 VIS
										m_SyncMode = 9;
										m_VisData = 0;
										m_VisCnt = 8;
										break;
									default:
										m_SyncMode = 0;
										break;
								}
							}
							else {          // 拡張 VIS
								m_SyncMode = 3;
								switch(m_VisData){
									case 0x45:      // MR73
										m_NextMode = smMR73;
										break;
									case 0x46:      // MR90
										m_NextMode = smMR90;
										break;
									case 0x49:      // MR115
										m_NextMode = smMR115;
										break;
									case 0x4a:      // MR140
										m_NextMode = smMR140;
										break;
									case 0x4c:      // MR175
										m_NextMode = smMR175;
										break;
									case 0x25:      // MP73
										m_NextMode = smMP73;
										break;
									case 0x29:      // MP115
										m_NextMode = smMP115;
										break;
									case 0x2a:      // MP140
										m_NextMode = smMP140;
										break;
									case 0x2c:      // MP175
										m_NextMode = smMP175;
										break;
									case 0x85:      // ML180
										m_NextMode = smML180;
										break;
									case 0x86:      // ML240
										m_NextMode = smML240;
										break;
									case 0x89:      // ML280
										m_NextMode = smML280;
										break;
									case 0x8a:      // ML320
										m_NextMode = smML320;
										break;
									default:
										m_SyncMode = 0;
										break;
								}
							}
						}
					}
				}
				break;
			case 3:                 // 1200Hz(30ms)のチェック
				if( !m_Sync ){
					m_pll.Do(ad);
				}
				m_SyncTime--;
				if( !m_SyncTime ){
					if( (d12 > d19) &&(d12 > m_SLvl) ){
						if( m_Sync ){
							if( m_rBase >= (SSTVSET.m_LM * 65/100) ){
								m_ReqSave = 1;
							}
						}
						if( m_NextMode == smAVT ){
							m_SyncTime = ((9 + 910 + 910 + 5311.9424 + 0.30514375) * sys.m_SampFreq / 1000.0);
							m_SyncMode++;
							m_SyncAVT = 1;
							m_Sync = 0;
						}
						else {
							m_SyncMode = 256;
						}
						SSTVSET.SetMode(m_NextMode);
					}
					else {
						m_SyncMode = 0;
					}
				}
				break;
			case 4:                 // AVTの1900Hz信号待ち
				m_SyncTime--;
				if( !m_SyncTime ){ m_SyncMode = 256; break;}

				d = m_pll.Do(ad);
				if( (d >= -1000) && (d <= 1000) ){        // First atack
					m_SyncMode++;
					m_SyncATime = 9.7646 * 0.5 * sys.m_SampFreq / 1000;
				}
				break;
			case 5:
				m_SyncTime--;
				if( !m_SyncTime ){ m_SyncMode = 256; break;}

				d = m_pll.Do(ad);
				if( (d >= -800) && (d <= 800) ){        // 2nd atack
					m_SyncATime--;
					if( !m_SyncATime ){
						m_SyncMode++;
						m_SyncATime = 9.7646 * sys.m_SampFreq / 1000;
						m_VisData = 0;
						m_VisCnt = 16;
					}
				}
				else {
					m_SyncMode = 4;
				}
				break;
			case 6:
				m_SyncTime--;
				if( !m_SyncTime ){ m_SyncMode = 256; break;}

				d = m_pll.Do(ad);
				m_SyncATime--;
				if( !m_SyncATime ){
					if( (d >= 8000)||(d < -8000) ){
						m_SyncATime = 9.7646 * sys.m_SampFreq / 1000;
						m_VisData = m_VisData << 1;
						if( d > 0 ) m_VisData |= 0x00000001;
						m_VisCnt--;
						if( !m_VisCnt ){
							int l = m_VisData & 0x00ff;
							int h = (m_VisData >> 8) & 0x00ff;
							if( ((l + h) == 0x00ff) && (l >= 0xa0) && (l <= 0xbf) && (h >= 0x40) && (h <= 0x5f)  ){
								if( h != 0x40 ){
									m_SyncATime = 9.7646 * 0.7 * sys.m_SampFreq / 1000;
									m_SyncTime = ((double(h - 0x40) * 165.9982) - 0.8) * sys.m_SampFreq / 1000;
									m_SyncMode++;
								}
								else {
									if( !m_SyncTime || (m_SyncTime >= 9.7646 * SampFreq / 1000) ){
										m_SyncTime = ((9.7646 * 0.5) - 0.8) * sys.m_SampFreq / 1000;
									}
									m_SyncMode = 8;
								}
							}
							else {
								m_SyncMode = 4;
							}
						}
					}
					else {
						m_SyncMode = 4;
					}
				}
				break;
			case 7:         // 同期
				d = m_pll.Do(ad);
				if( (d >= -1000) && (d <= 1000) ){        // First atack
					m_SyncMode = 5;
					m_SyncATime = 9.7646 * 0.5 * sys.m_SampFreq / 1000;
				}
				else {
					m_SyncATime--;
					if( !m_SyncATime ){
						m_SyncMode = 4;
					}
				}
				break;
			case 8:
				m_SyncMode--;
				if( !m_SyncMode ){
					Start();
				}
				break;
			case 256:               // 強制開始
				Start();
				break;
			case 512:               // 0.5sのウエイト
				m_SyncTime = SampFreq * 0.5;
				m_SyncMode++;
				break;
			case 513:
				m_SyncTime--;
				if( !m_SyncTime ){
					m_SyncMode = 0;
				}
				break;
		}
	}
	if( m_Sync ){
		switch(m_Type){
			case 0:		// PLL
				if( m_afc && (m_lvl.m_CurMax > 16) && (SSTVSET.m_Mode != smAVT) ) SyncFreq(m_fqc.Do(m_lvl.m_Cur));
				d = m_pll.Do(m_lvl.m_Cur);
				break;
			case 1:		// Zero-crossing
				d = m_fqc.Do(m_lvl.m_Cur);
				if( m_afc && (m_lvl.m_CurMax > 16) && (SSTVSET.m_Mode != smAVT) ) SyncFreq(d);
				break;
			default:	// Hilbert
				d = m_hill.Do(m_lvl.m_Cur);
				if( m_afc && (m_lvl.m_CurMax > 16) && (SSTVSET.m_Mode != smAVT) ) SyncFreq(d);
				break;
		}
		if( m_afc ) d += m_AFCDiff;
		if( m_Skip ){
			if( m_Skip > 0 ){
				m_Skip--;
			}
			else {
				for( ; m_Skip; m_Skip++ ){
					int n = m_wBase + m_wCnt;
					m_Buf[n] = -d;
					m_B12[n] = 0;
					IncWP();
				}
			}
		}
		else {
			if( m_ScopeFlag ){
				m_Scope[1].WriteData(d);
			}
			int n = m_wBase + m_wCnt;
			m_Buf[n] = -d;

#if NARROW_SYNC == 1200
			if( SSTVSET.m_Mode != smAVT ){
				m_B12[n] = d12;
			}
#else
			if( m_fNarrow ){
				m_B12[n] = d19;
            }
			else if( SSTVSET.m_Mode != smAVT ){
				m_B12[n] = d12;
			}
#endif
			else {
				m_B12[n] = (d + 16384) * 0.25;
			}
			IncWP();
		}
	}
	else if( sys.m_TestDem ){
		switch(m_Type){
			case 0:
				m_CurSig = m_Avg.Avg(m_pll.Do(m_lvl.m_Cur));
				break;
			case 1:
				m_CurSig = m_Avg.Avg(m_fqc.Do(m_lvl.m_Cur));
				break;
			default:
				m_CurSig = m_Avg.Avg(m_hill.Do(m_lvl.m_Cur));
				break;
		}
	}
}

void CSSTVDEM::IncWP(void)
{
	m_wCnt++;
	if( m_wCnt >= SSTVSET.m_WD ){
		m_wCnt = 0;
		m_wPage++;
		m_wLine++;
		m_wBase += m_BWidth;
		if( m_wPage >= SSTVDEMBUFMAX ){
			m_wPage = 0;
			m_wBase = 0;
		}
	}
}

void CSSTVDEM::SyncFreq(double d)
{
/*
	double		m_AFC_LowVal;	// (Center - SyncLow) * 16384 / BWH
	double		m_AFC_HighVal;	// (Center - SyncHigh) * 16384 / BWH
	double		m_AFC_SyncVal;	// (Center - Sync) * 16384 / BWH
	double		m_AFC_BWH;		// BWH / 16384.0;
*/
	d -= 128;

	if( (d <= m_AFC_LowVal) && (d >= m_AFC_HighVal) ){
		if( !m_AFCDis && (m_AFCCount >= SSTVSET.m_AFCB) && (m_AFCCount <= SSTVSET.m_AFCE) ){
			m_AFCData = m_Avg.Avg(d);
			if( m_AFCCount == SSTVSET.m_AFCE ){
				if( m_AFCGard ){
					m_AFCLock = m_AFCAVG.SetData(m_AFCData);
					m_AFCGard = 0;
				}
				else {
					m_AFCLock = m_AFCAVG.Avg(m_AFCData);
				}
				m_AFCDiff = m_AFC_SyncVal - m_AFCLock;
				m_AFCFlag = 15;
				InitTone(m_AFCDiff * m_AFC_BWH);
				m_AFCDis = m_AFCInt;
			}
		}
		m_AFCCount++;
	}
	else {
		if( (m_AFCCount >= SSTVSET.m_AFCB) && m_AFCGard ){
			m_AFCGard--;
			if( !m_AFCGard ) m_AFCAVG.SetData(m_AFCLock);
		}
		m_AFCCount = 0;
		if( m_AFCDis ) m_AFCDis--;
	}
}

void CSSTVDEM::DecodeFSK(int m, int s)
{
	int d;
	switch(m_fskmode){
		case 0:         // スペースキャリア検出
			d = ABS(m - s);
			if( (s > m) && (d >= 2048) ){
				m_fsktime = (FSKGARD/2) * SSTVSET.m_SampFreq/1000;
				m_fskmode++;
			}
			break;
		case 1:         // スペースキャリア検出(連続)
			d = ABS(m - s);
			if( (s > m) && (d >= 2048) ){
				m_fsktime--;
				if( !m_fsktime ){
					m_fsktime = FSKGARD * SSTVSET.m_SampFreq/1000;
					m_fskmode++;
				}
			}
			else {
				m_fskmode = 0;
			}
			break;
		case 2:         // スタートビットの検出
			d = ABS(m - s);
			m_fsktime--;
			if( !m_fsktime ){
				m_fskmode = 0;
			}
			else if( (m > s) && (d >= 2048) ){
				m_fsktime = (FSKINTVAL/2) * SSTVSET.m_SampFreq/1000;
				m_fskmode++;
			}
			break;
		case 3:         // スタートビットの検出(中間点)
			m_fsktime--;
			if( !m_fsktime ){
				d = ABS(m - s);
				if( (m > s) && (d >= 2048) ){
					m_fsktime = 0;
					m_fsknextd = double(FSKINTVAL)/1000.0 * SSTVSET.m_SampFreq;
					m_fsknexti = m_fsknextd;
					m_fskbcnt = 0;
					m_fskc = 0;
					m_fskmode++;
				}
				else {
					m_fskmode = 0;
				}
			}
			break;
		default:
			m_fsktime++;
			if( m_fsktime >= m_fsknexti ){
				d = ABS(m - s);
				if( d < 2048 ){
					m_fskmode = 0;
				}
				else {
					m_fsknextd += double(FSKINTVAL)/1000.0 * SSTVSET.m_SampFreq;
					m_fsknexti = m_fsknextd;
					m_fskc = BYTE(m_fskc >> 1);
					if( m > s ) m_fskc |= 0x20;
					m_fskbcnt++;
					if( m_fskbcnt >= 6 ){
						m_fskbcnt = 0;
						switch(m_fskmode){
							case 4:         // First SYNC 0x2A
								if( m_fskc == 0x2a ){
									m_fskcnt = 0;
									m_fskbcnt = 0;
									m_fsks = 0;
									m_fskc = 0;
									m_fskmode++;
								}
                                else if( m_fskc == 0x2d ){
									m_fskcnt = 0;
									m_fskbcnt = 0;
									m_fsks = 0;
									m_fskc = 0;
									m_fskmode = 16;
                                }
								else {
									m_fskmode = 0;
								}
								break;
							case 5:         // Store data
								if( m_fskc == 0x01 ){
									if( m_fskcnt >= 1 ){
										m_fskmode++;
									}
									else {
										m_fskmode = 0;
									}
								}
								else {
									m_fsks = m_fskc ^ m_fsks;
									m_fskdata[m_fskcnt] = BYTE(m_fskc + 0x20);
									m_fskcnt++;
									if( m_fskcnt >= 17 ){
										m_fskmode = 0;
									}
								}
								break;
							case 6:         // Check XOR
								m_fsks &= 0x3f;
								if( (m_fskc == m_fsks) && m_fskdecode ){
									m_fskdata[m_fskcnt] = 0;
									StrCopy(m_fskcall, SkipSpace(LPCSTR(m_fskdata)), 16);
									clipsp(m_fskcall);
									m_fskrec = 1;
									m_fskmode++;

									m_fskcnt = 0;
									m_fsks = 0;
									m_fskc = 0;
								}
                                else {
									m_fskmode = 0;
                                }
								break;
							case 7:         // Store data
								if( m_fskc == 0x01 ){
									if( m_fskcnt >= 1 ){
										m_fskmode++;
									}
									else {
										m_fskmode = 0;
									}
								}
								else if( m_fskc == 0x02 ){
									m_fsks = 0x02;
                                    m_fskNR = 0;
                                    m_fskmode = 9;
                                }
								else if( m_fskc >= 0x10 ){
									m_fsks = m_fskc ^ m_fsks;
									m_fskNRS[m_fskcnt] = BYTE(m_fskc + 0x20);
									m_fskcnt++;
									if( m_fskcnt >= 9 ){
										m_fskmode = 0;
									}
								}
                                else {
									m_fskmode = 0;
                                }
								break;
							case 8:         // Check XOR
								m_fsks &= 0x3f;
								if( (m_fskc == m_fsks) && m_fskdecode ){
									m_fskNRS[m_fskcnt] = 0;
									clipsp(m_fskNRS);
									m_fskNRrec = 1;
								}
								m_fskmode = 0;
								break;
                            case 9:
								m_fsks = m_fskc ^ m_fsks;
                                m_fskNR = m_fskNR << 6;
                                m_fskNR += m_fskc;
								m_fskcnt++;
								if( m_fskcnt >= 2 ){
									m_fskmode++;
								}
                                break;
                            case 10:
								m_fsks &= 0x3f;
								if( m_fskc == m_fsks ){
									sprintf(m_fskNRS, "%03u", m_fskNR);
									m_fskNRrec = 1;
								}
								m_fskmode = 0;
                                break;
                            case 16:
								m_fsks = m_fskc ^ m_fsks;
								if( m_fskc == 0x15 ){
									m_fskmode++;
                                }
                                else {
									m_fskmode = 0;
                                }
                            	break;
                            case 17:
								m_fsks = m_fskc ^ m_fsks;
								m_fskdata[0] = m_fskc;
                                m_fskmode++;
                            	break;
                            case 18:
								m_fsks &= 0x3f;
								if( m_fskc == m_fsks ){
									switch(m_fskdata[0]){
										case 0x02:
											m_NextMode = smMN73;
											break;
										case 0x04:
											m_NextMode = smMN110;
											break;
										case 0x05:
											m_NextMode = smMN140;
											break;
										case 0x14:
											m_NextMode = smMC110;
											break;
										case 0x15:
											m_NextMode = smMC140;
											break;
										case 0x16:
											m_NextMode = smMC180;
											break;
										default:
											m_NextMode = 0;
											break;
									}
									if( (m_SyncRestart || !m_Sync) && m_NextMode && (m_SyncMode >= 0) ){
										SSTVSET.SetMode(m_NextMode);
										Start();
									}
								}
								m_fskmode = 0;
                                break;
						}
						m_fskc = 0;
					}
				}
			}
			break;
	}
}
//--------------------------------------------------------
// リピータ変数の初期化
void CSSTVDEM::InitRepeater(void)
{
	if( sys.m_Repeater ){
		if( pRep == NULL ) pRep = new REPSET;
		pRep->m_iirrep.SetFreq(m_RepTone + g_dblToneOffset, SampFreq, 100.0);
		pRep->m_lpfrep.MakeIIR(50, SampFreq, 2, 0, 0);
	}
	else {
		pRep = NULL;
	}
}
//--------------------------------------------------------
// リピータのON/OFF
void CSSTVDEM::SetRepeater(int sw)
{
	if( sw != m_Repeater ){
		m_repmode = 0;
		m_Repeater = sw;
	}
}
//--------------------------------------------------------
// リピータトーンの検出感度設定
void CSSTVDEM::SetRepSenseLvl(void)
{
	switch(sys.m_RepSenseLvl){
		case 0:
			m_RSLvl = 3072;
			break;
		case 1:
			m_RSLvl = 4096;
			break;
		case 2:
			m_RSLvl = 6144;
			break;
		default:
			m_RSLvl = 8192;
			break;
	}
	m_RSLvl2 = m_RSLvl / 2;
}
//--------------------------------------------------------
// リピータトーンの検出処理
void CSSTVDEM::Repeater(int d17, int d12, int d19)
{
	int d1 = ABS(d17 - d12);
	int d2 = ABS(d17 - d19);
#if 0
	m_repD1 = d1;
	m_repD2 = d2;
#endif
	switch(m_repmode){
		case 0:     // トーン検出のトリガ
			if( (d1 > m_RSLvl) && (d2 > m_RSLvl2) ){
				m_reptime = sys.m_RepTimeA * SSTVSET.m_SampFreq / 1000;
				m_repmode++;
			}
			break;
		case 1:     // トーンの持続のチェック
			if( (d1 > m_RSLvl) && (d2 > m_RSLvl2) ){
				m_reptime--;
				if( !m_reptime ){
					m_repmode++;
					m_repcount = 10000 * SSTVSET.m_SampFreq / 1000;
				}
			}
			else {
				m_repmode = 0;
			}
			break;
		case 2:     // トーンの終了の検出
			m_repcount--;
			if( !m_repcount ){
				m_repmode = 0;
			}
			if( (d1 > m_RSLvl) && (d2 > m_RSLvl2) ){
				m_reptime = sys.m_RepTimeB * SSTVSET.m_SampFreq / 1000;
				m_repcount = 10000 * SSTVSET.m_SampFreq / 1000;
				if( !m_reptime ) m_reptime++;
			}
			else if( m_RepSQ && (m_repsig > m_RepSQ) ){
				m_reptime = sys.m_RepTimeB * SSTVSET.m_SampFreq / 1000;
				if( !m_reptime ) m_reptime++;
			}
			else {
				m_reptime--;
				if( !m_reptime ){
					m_repmode++;
				}
			}
			break;
//        case 3:     // 'K'の送信待ち
//            break;
		case 4:
			m_reptime = sys.m_RepTimeC * SSTVSET.m_SampFreq / 1000;
			m_repcount = sys.m_RepTimeA * SSTVSET.m_SampFreq / 1000;
			m_repmode++;
			break;
		case 5:       // 10[s]のタイムアウト待ち
			m_reptime--;
			if( !m_reptime ){       // タイムアウトによる待機
				m_repmode = 0;
			}
			else if( (d1 > m_RSLvl) && (d2 > m_RSLvl2) ){
				m_repcount--;
				if( !m_repcount ){
					m_repmode = 2;
				}
			}
			else {
				m_repcount = sys.m_RepTimeA * SSTVSET.m_SampFreq / 1000;
			}
			break;
//        case 6:     // 受信中
//            break;
		case 7:
			m_reptime = sys.m_RepTimeD * SSTVSET.m_SampFreq / 1000;
			m_repcount = 20000 * SSTVSET.m_SampFreq / 1000;
			if( !m_reptime ) m_reptime++;
			m_repmode++;
			break;
		case 8:         // リプレイ送信前のタイマ
			m_repcount--;
			if( !m_repcount ){
				m_repmode = 0;
			}
			if( m_RepSQ && (m_repsig > m_RepSQ) ){
				m_reptime = sys.m_RepTimeD * SSTVSET.m_SampFreq / 1000;
				if( !m_reptime ) m_reptime++;
			}
			else {
				m_reptime--;
				if( !m_reptime ){
					m_repmode++;
				}
			}
			break;
//        case 9:       // 送信トリガ
//            break;
//        case 10:      // リプレイ送信中
//            break;
		default:
			break;
	}
}
//--------------------------------------------------------
// CSSTVMODクラス
CSSTVMOD::CSSTVMOD()
{
	m_TXBuf = NULL;

	m_bpf = 1;
	m_lpf = 0;
	m_lpffq = 2000;
//	memset(HBPF, 0, sizeof(HBPF));
//	memset(Z, 0, sizeof(Z));
	m_bpftap = 24;
//	MakeFilter(HBPF, m_bpftap, ffBPF, SampFreq, 700, 2800, 40, 1.0);
	int lfq = 700 + g_dblToneOffset;
    if( lfq < 100 ){
		m_BPF.Create(m_bpftap, ffLPF, SampFreq, 2800 + g_dblToneOffset, 2800 + g_dblToneOffset, 40, 1.0);
    }
    else {
		m_BPF.Create(m_bpftap, ffBPF, SampFreq, lfq, 2800 + g_dblToneOffset, 40, 1.0);
    }
	m_outgain = 24578.0;
	InitGain();
	m_vco.SetSampleFreq(SampFreq);
	m_vco.VirtualLock();
	m_vco.SetFreeFreq(1100 + g_dblToneOffset);
	m_vco.SetGain(2300 - 1100);
	InitTXBuf();
	m_tune = 0;
	m_Lost = 0;
	m_TuneFreq = 1750;
	m_VariOut = 0;
	m_VariR = 298;
	m_VariG = 588;
	m_VariB = 110;
}

CSSTVMOD::~CSSTVMOD()
{
	CloseTXBuf();
}

void CSSTVMOD::OpenTXBuf(int s)
{
	if( (s != m_TXBufLen) || (m_TXBuf == NULL) ){
		if( m_TXBuf != NULL ) delete m_TXBuf;
		if( s < 5 ) s = 5;
		if( s > 14 ) s = 14;
		m_TXBufLen = s;
		m_TXMax = m_TXBufLen * SampFreq;
		m_TXBuf = new short[m_TXMax];
	}
}

void CSSTVMOD::CloseTXBuf(void)
{
	if( m_TXBuf != NULL ){
		delete m_TXBuf;
		m_TXBuf = NULL;
	}
}

void CSSTVMOD::InitTXBuf(void)
{
	m_rPnt = m_wPnt = 0;
	m_Cnt = 0;
	m_rCnt = 0;

	m_iPos = 0;
	m_dPos = 0;

	m_wLine = 0;

	m_RowCnt = 0;
	pRow = NULL;
    m_BPF.Clear();
//	memset(Z, 0, sizeof(Z));
}

void CSSTVMOD::Write(short fq)
{
#if DEBUG_AFC
	fq += DEBUG_AFC;
#endif
	m_TXBuf[m_wPnt] = fq;
	m_wPnt++;
	m_Cnt++;
	if( m_wPnt >= m_TXMax ) m_wPnt = 0;
}

void CSSTVMOD::Write(short fq, double tim)
{
	m_dPos += (tim * SSTVSET.m_TxSampFreq)/1000.0;
	for( ; m_iPos < int(m_dPos); m_iPos++ ) Write(fq);
}

void CSSTVMOD::WriteC(short fq, double cnt)
{
	m_dPos += cnt;
	for( ; m_iPos < int(m_dPos); m_iPos++ ) Write(fq);
}

double CSSTVMOD::Do(void)
{
	double d;
	if( sys.m_TestDem ){
		d = m_vco.Do(double(sys.m_TestDem - 1100)/double(2300-1100));
		d *= m_outgain;
	}
	else if( m_tune ){
		d = m_vco.Do((m_TuneFreq - 1100)/1200.0);
		d *= m_outgain;
	}
	else if( m_Cnt ){
		int f = m_TXBuf[m_rPnt];
		if( f > 0 ){
			d = double((f & 0x0fff) - 1100)/double(2300-1100);
			if( m_lpf ) d = avgLPF.Avg(d);
			d = m_vco.Do(d);
		}
		else {
			d = 0;
		}
		m_Cnt--;
		m_rCnt++;
		m_rPnt++;
		if( m_rPnt >= m_TXMax ) m_rPnt = 0;
		if( m_VariOut ){
			switch(f & 0xf000){
				case 0x1000:
					d *= m_outgainR;
					break;
				case 0x2000:
					d *= m_outgainG;
					break;
				case 0x3000:
					d *= m_outgainB;
					break;
				default:
					d *= m_outgain;
					break;
			}
		}
		else {
#if VARITEST
			d *= m_outgain/3;
#else
			d *= m_outgain;
#endif
		}
	}
	else if( m_RowCnt ){
		d = (double)(*pRow++);
		m_RowCnt--;
		m_rCnt++;
	}
	else {
		d = double(1500-1100)/double(2300-1100);
		d *= m_outgain;
//		m_rCnt++;
	}
//	if( m_bpf ) d = DoFIR(HBPF, Z, d, m_bpftap);
	if( m_bpf ) d = m_BPF.Do(d);
	return d;
}

void CSSTVMOD::CalcFilter(void)
{
	if( m_bpftap ){
		int lfq = 700 + g_dblToneOffset;
        if( lfq < 100 ){
			m_BPF.Create(m_bpftap, ffLPF, SampFreq, 2800 + g_dblToneOffset, 2800 + g_dblToneOffset, 40, 1.0);
        }
        else {
			m_BPF.Create(m_bpftap, ffBPF, SampFreq, lfq, 2800 + g_dblToneOffset, 40, 1.0);
        }
	}
	avgLPF.SetCount(int(SampFreq/m_lpffq + 0.5));
}

void CSSTVMOD::InitGain(void)
{
	if( m_VariR > 1000 ) m_VariR = 1000;
	if( m_VariG > 1000 ) m_VariG = 1000;
	if( m_VariB > 1000 ) m_VariB = 1000;
	m_outgainR = m_outgain * double(m_VariR) * 0.001;
	m_outgainG = m_outgain * double(m_VariG) * 0.001;
	m_outgainB = m_outgain * double(m_VariB) * 0.001;
}

void CSSTVMOD::WriteFSK(BYTE c)
{
	int i;
	for( i = 0; i < 6; i++ ){
		Write(short(c & 0x01 ? 1900 : FSKSPACE), FSKINTVAL);
		c = BYTE(c >> 1);
	}
}

void CSSTVMOD::WriteCWID(char c)
{
	const USHORT _tbl[]={
		// 0       1       2       3        4       5       6       7
		0x0005, 0x8005, 0xc005, 0xe005, 0xf005, 0xf805, 0x7805, 0x3805, // 0-7
		// 8       9       :        ;       <       =       >       ?
		0x1805, 0x0805, 0x0000, 0x0000, 0x0000, 0x7005, 0xA805, 0xcc06, // 8
		// @        A       B       C       D       E       F       G
		0x0000, 0x8002, 0x7004, 0x5004, 0x6003, 0x8001, 0xd004, 0x2003, // @-G
		// H        I       J       K       L       M       N       O
		0xf004, 0xc002, 0x8004, 0x4003, 0xb004, 0x0002, 0x4002, 0x0003, // H-O
		// P        Q       R       S       T       U       V       W
		0x9004, 0x2004, 0xa003, 0xe003, 0x0001, 0xc003, 0xe004, 0x8003, // P-W
		// X        Y       Z
		0x6004, 0x4004, 0x3004, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // X-Z
	};
	int dot = sys.m_CWIDSpeed + 30;
//    int dot = sys.m_CWIDSlow ? 40 : 30;
	c = char(toupper(c));
	c &= 0x7f;
	int d;
	if( c == '.' ) c = 'R';
	if( c == '/' ){
		d = 0x6805;
	}
	else if( c == '@' ){
		Write(0, 250);
		return;
	}
	else if( (c >= '0') && (c <= 'Z') ){
		c -= '0';
		d = _tbl[c];
	}
	else {
		d = 0;
	}
	int n = d & 0x00ff;
	if( !d ){
		Write(0, dot*7);
		return;
	}
	for(int i = 0; i < n; i++ ){
		if( d & 0x8000 ){
			Write(short(sys.m_CWIDFreq), dot);
		}
		else {
			Write(short(sys.m_CWIDFreq), dot*3);
		}
		Write(0, dot);
		d = d << 1;
	}
	Write(0, dot*2);
}

CHILL::CHILL()
{
//	memset(Z, 0, sizeof(Z));
	SetWidth(0);
#if HILLDOUBLEBUF
	m_FIR.Create(m_tap);
#endif

	m_htap = m_tap / 2;
    MakeHilbert(H, m_tap, SampFreq, 100, SampFreq/2 - 100);
	m_A[0] = m_A[1] = m_A[2] = m_A[3] = 0;
	m_iir.MakeIIR(1800, SampFreq, 3, 0, 0);
#if !HILLDOUBLEBUF
	m_ph = &Z[m_htap];
#endif
}

void __fastcall CHILL::SetWidth(int fNarrow)
{
	if( fNarrow ){
		m_OFF = (2 * PI * (NARROW_CENTER + g_dblToneOffset)) / SampFreq;
		m_OUT = 32768.0 * SampFreq / (2 * PI * NARROW_BW);
    }
    else {
		m_OFF = (2 * PI * (1900 + g_dblToneOffset)) / SampFreq;
		m_OUT = 32768.0 * SampFreq / (2 * PI * 800);
    }
	if( SampBase >= 40000 ){
		m_OFF *= 4;
		m_OUT *= 0.25;
		m_tap = 48; //48
		m_df = 2;
	}
	else if( SampBase >= 16000 ){
		m_OFF *= 2;
		m_OUT *= 0.5;
		m_tap = 24; //24
		m_df = 1;
	}
	else {
		m_tap = 12; //12
		m_df = 0;
	}
	if( sys.m_bCQ100 ){
		m_tap *= 3;
    }
}

double CHILL::Do(double d)
{
#if HILLDOUBLEBUF
	double a;
    m_FIR.Do(d, a, H);
#else
	d = DoFIR(H, Z, d, m_tap);
	double a = *m_ph;
#endif
	if( a ) a = atan2(d, a);
	d = a - m_A[0];
	switch(m_df){
		case 1:
			m_A[0] = m_A[1];
			m_A[1] = a;
			break;
		case 2:
			m_A[0] = m_A[1];
			m_A[1] = m_A[2];
			m_A[2] = m_A[3];
			m_A[3] = a;
			break;
		default:
			m_A[0] = a;
			break;
	}
	if( d >= PI ){
		d = d - PI*2;
	}
	else if( d <= -PI ){
		d = d + PI*2;
	}
	d += m_OFF;
	return m_iir.Do(d * m_OUT);
}

