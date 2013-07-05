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
#ifndef SstvH
#define SstvH
//---------------------------------------------------------------------------
#include "Fir.h"

#define	BITDEBUG	0		// キャラクタ長の測定
#define	TXDDEBUG	0		// FSKのキャラクタ長の測定
#define	FSKDEBUG	0		// FSKのデバッグモード
#define	DEBUG_AFC	0		// AFCのテスト(+/-の周波数)

extern double	g_dblToneOffset;

#pragma pack(push, 4)

#ifndef SampFreq
extern	double	SampFreq;
#endif

class CTICK
{
public:
	int		*ptbl[2];
	int		m_wsel;
	int		*m_wp;
	int		m_wcnt;
	int		m_Trig;
	int		m_Samp;
public:
	CTICK();
	~CTICK();
	void Init(void);
	void Write(double d);
	inline int GetTrig(void){return m_Trig;};
	int *GetData(void);
};

class CVCO
{
private:
	double	m_c1;	// VCOの利得
	double	m_c2;	// フリーランニング周波数
	double	m_z;

	int		m_TableSize;
	double	*pSinTbl;
	double	m_FreeFreq;
	double	m_SampleFreq;
	int     m_vlock;
public:
	CVCO();
	~CVCO();
	void VirtualLock(void);
	void InitPhase(void);
	void SetGain(double gain);
	void SetSampleFreq(double f);
	void SetFreeFreq(double f);
	double Do(double d);
};

class CSmooz{
private:
	double	*bp;
	int	Wp;
public:
	int	Max;
	int	Cnt;
private:
	inline void IncWp(void){
		Wp++;
		if( Wp >= Max ) Wp = 0;
	};
	inline double Avg(void){
		double	d = 0.0;
		int		i;
		for( i = 0; i < Cnt; i++ ){
			d += bp[i];
		}
		if( Cnt ){
			return d/double(Cnt);
		}
		else {
			return 0;
		}
	};
public:
	inline CSmooz(int max = 2){
		Max = max;
		bp = new double[max];
		Cnt = 0;
		Wp = 0;
	};
	inline ~CSmooz(void){
		delete bp;
	};
	inline void SetCount(int n){
		if( !n ) n = 1;
		if( Max != n ){
			double *np = new double[n];
			Max = n;
			Cnt = Wp = 0;
			double *op = bp;
			bp = np;
			delete op;
		}
		else {
			Cnt = Wp = 0;
		}
	};
	inline double SetData(double d){
		for( int i = 0; i < Max; i++ ){
			bp[i] = d;
		}
		Wp = 0;
		Cnt = Max;
		return d;
	};
	inline double Avg(double d){
		bp[Wp] = d;
		IncWp();
		if( Cnt < Max ){
			Cnt++;
		}
		return Avg();
	};
};

class CPLL
{
private:
	CIIR	loopLPF;
	CIIR	outLPF;

	double	m_err;
	double	m_out;
	double	m_vcoout;
	double	m_SampleFreq;
	double	m_FreeFreq;
	double	m_Shift;

	double  m_Max;
	double  m_Min;
	double  m_d;
	double  m_agc, m_agca;
public:
	int		m_loopOrder;
	double	m_loopFC;
	int		m_outOrder;
	double	m_outFC;
	double	m_vcogain;
	double  m_outgain;
public:
	CVCO	vco;
	CPLL();
	void SetWidth(int fNarrow);
	void SetVcoGain(double g);
	void MakeLoopLPF(void);
	void MakeOutLPF(void);
	void SetFreeFreq(double f1, double f2);
	void SetSampleFreq(double f);
	double Do(double d);
	inline double GetErr(void){return m_err*32768;};	// Phase Det
	inline double GetOut(void){return m_out*32768;};	// Phase Det
	inline double GetVco(void){return m_vcoout;};
};


#define	SCOPESIZE	8192
class CScope
{
private:
	int		m_DataFlag;

	int		m_wp;

public:
	int		m_ScopeSize;
	double	*pScopeData;

public:
	CScope();
	~CScope();
	void InitMem(void);
	void WriteData(double d);
	void UpdateData(double d);
	inline int GetFlag(void){m_DataFlag && (pScopeData != NULL); return 0;};	//ja7ude 0521
	void Collect(int size);
};

#define	NOISEBPFTAP	12
class CNoise		// M系列ノイズ N=22 (Tap=1)
{
private:
	double	H[NOISEBPFTAP+1];
	double	Z[NOISEBPFTAP+1];

	DWORD	reg;

public:
	CNoise();
	double GetNoise(void);
};

class CLVL
{
public:
	double m_Cur;

	double m_PeakMax;
	double m_PeakAGC;
	double m_Peak;
	double m_CurMax;
	double m_Max;
	double m_agc;
	int    m_CntPeak;
	int    m_agcfast;

	int    m_Cnt;
	int    m_CntMax;
public:
	CLVL(){
		m_agcfast = 0;
		m_CntMax = SampFreq * 100 / 1000.0;
		Init();
	};
	void Init(void){
		m_PeakMax = 0;
		m_PeakAGC = 0;
		m_Peak = 0;
		m_Cur = 0;
		m_CurMax = 0.0;
		m_Max = 0;
		m_agc = 1.0;
		m_CntPeak = 0;
		m_Cnt = 0;
	};
	void Do(double d ){
		m_Cur = d;
		if( d < 0.0 ) d = -d;
		if( m_Max < d ) m_Max = d;
		m_Cnt++;
	};
	void Fix(void){
		if( m_Cnt < m_CntMax ) return;	// did not store yet
		m_Cnt = 0;
		m_CntPeak++;
		if( m_Peak < m_Max ) m_Peak = m_Max;
		if( m_CntPeak >= 5 ){
			m_CntPeak = 0;
			m_PeakMax = m_Max;
			m_PeakAGC = (m_PeakAGC + m_Max) * 0.5;
			m_Peak = 0;
			if( !m_agcfast ){
				if( (m_PeakAGC > 32) && m_PeakMax ){
					m_agc = 16384.0 / m_PeakMax;
				}
				else {
					m_agc = 16384.0 / 32.0;
				}
			}
		}
		else {
			if( m_PeakMax < m_Max ) m_PeakMax = m_Max;
		}
		m_CurMax = m_Max;
		if( m_agcfast ){
			if( m_CurMax > 32 ){
				m_agc = 16384.0 / m_CurMax;
			}
			else {
				m_agc = 16384.0 / 32.0;
			}
		}
		m_Max = 0;
	};
	double AGC(double d){
		return d * m_agc;
	};
};

class CSLVL
{
public:
	double  m_Max;
	double  m_Min;
	double  m_Lvl;

	int    m_Cnt;
	int    m_CntMax;
public:
	CSLVL(){
		m_CntMax = SampFreq * 100 / 1000.0;
		Init();
	};
	void Init(void){
		m_Max = 0;
		m_Min = 16384;
		m_Lvl = 0;
		m_CntMax = 0;
		m_Cnt = 0;
	};
	void Do(double d){
		if( m_Max < d ) m_Max = d;
		if( m_Min > d ) m_Min = d;
		m_Cnt++;
	};
	void Fix(void){
		if( m_Cnt < m_CntMax ) return;	// did not store yet
		m_Cnt = 0;
		m_Lvl = m_Max - m_Min;
		m_Max = 0;
		m_Min = 16384;
	};
	double GetLvl(void){
		return m_Lvl;
	};
};

#if 0
class CAGC
{
public:
	double  m_CurMax;
	double  m_Max;
	int     m_Cnt;
	int     m_CntMax;
	CSmooz  m_sm;
public:
	CAGC(){
		m_Max = 0;
		m_CurMax = 1.0;
		m_Cnt = 0;
		m_CntMax = 100 * SampFreq / 1000.0;
		m_sm.SetCount(5);
	};
	double Do(double d){
		double sd = d;
		if( d < 0.0 ) d = -d;
		if( m_Max < d ) m_Max = d;
		if( m_Cnt >= m_CntMax ){
			m_CurMax = m_sm.Avg(m_Max);
			if( m_CurMax > 0.0 ){
				m_CurMax = 16384.0 / m_CurMax;
			}
			m_Max = 0;
			m_Cnt = 0;
		}
		return sd * m_CurMax;
	};
};
#endif

#define	HILLDOUBLEBUF	FALSE	// 速度が速いほうを選択
#define HILLTAP 48
class CHILL
{
public:
#if HILLDOUBLEBUF
	CFIR2	m_FIR;
#else
	double  Z[HILLTAP+1];
#endif
	double  H[HILLTAP+1];
	double  m_A[4];
	double  m_OFF;
	double  m_OUT;
	double  *m_ph;
	int     m_htap;
	int     m_df;
	int     m_tap;
	CIIR    m_iir;
public:
	CHILL();
	void __fastcall SetWidth(int fNarrow);
	double Do(double d);
};

#define ZEROFQ	(-1900.0/400.0)

class CFQC {
public:
	int     m_Mode;
	int     m_Count;
	double  m_ACount;
	double  m_d;
	double  m_fq;
	double  m_out;
	double  m_SampFreq;

	double	m_CenterFQ;
    double	m_HighFQ;
    double	m_LowFQ;

    double	m_HighVal;
    double	m_LowVal;
    double	m_BWH;
public:
	int     m_Type;
	int     m_Limit;
	CIIR    m_iir;

	double  m_SmoozFq;
	CSmooz  m_fir;

	int     m_outOrder;
	double  m_outFC;

	int		m_Timer;
	int		m_STimer;

	void    CalcLPF(void);
	void    Clear(void);
	void    SetSampFreq(double fq);

public:
	CFQC();
	void SetWidth(int fNarrow);
	double Do(double s);
};

#define	NARROW_SYNC		1900
#define	NARROW_LOW		2044
#define	NARROW_HIGH		2300
#define	NARROW_CENTER	((NARROW_HIGH+NARROW_LOW)/2)
#define	NARROW_BW		(NARROW_HIGH - NARROW_LOW)
#define	NARROW_BWH		(NARROW_BW/2)
#define NARROW_BPFLOW	1600
#define	NARROW_BPFHIGH	2500
#define	NARROW_AFCLOW	1800
#define	NARROW_AFCHIGH	1950
enum {
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
	smMR73,
	smMR90,
	smMR115,
	smMR140,
	smMR175,
	smMP73,
	smMP115,
	smMP140,
	smMP175,
	smML180,
	smML240,
	smML280,
	smML320,
	smR24,
	smRM8,
	smRM12,
    smMN73,
    smMN110,
    smMN140,
    smMC110,
    smMC140,
    smMC180,
	smEND,
};
extern BOOL __fastcall IsNarrowMode(int mode);
extern LPCSTR SSTVModeList[];
extern const BYTE SSTVModeOdr[];

class CSSTVSET {
private:
public:
	int     m_Mode;
	int     m_TxMode;
	double  m_TW;
	double  m_KS;
	double  m_KS2;
	double  m_OF;
	double  m_OFP;
	double  m_CG;
	double  m_CB;
	double  m_SG;
	double  m_SB;
	int     m_WD;
	int     m_L;
	int     m_LM;
	int     m_OFS;
    int		m_IOFS;
	double  m_SampFreq;

	double  m_KSS;
	double  m_KS2S;
	int     m_KSB;

	int     m_TWD;
	int     m_TL;
	double  m_TTW;
	double  m_TxSampFreq;

	DWORD   m_MS[smEND];
	DWORD   m_MSLL;
	DWORD   m_MSL;
	DWORD   m_MSH;

	int     m_AFCW;
	int     m_AFCB;
	int     m_AFCE;

	int		m_fNarrow;
    int		m_fTxNarrow;
public:
	CSSTVSET();
	void SetMode(int mode);
	void SetTxMode(int mode);
	void SetSampFreq(void);
	void SetTxSampFreq(void);
	void InitIntervalPara(void);
	void GetBitmapSize(int &w, int &h, int mode);
	void GetPictureSize(int &w, int &h, int &hp, int mode);
	double GetTiming(int mode);
};
extern  CSSTVSET    SSTVSET;

typedef struct {    // リピータ用の設定
	CIIRTANK    m_iirrep;
	CIIR        m_lpfrep;
	CLMS        m_lmsrep;
}REPSET;

#define MSYNCLINE       8
class CSYNCINT {
public:
	DWORD       m_MSyncList[MSYNCLINE];
	DWORD       m_MSyncCnt;
	DWORD       m_MSyncACnt;
	int         m_MSyncTime;
	int         m_MSyncIntPos;
	int         m_MSyncIntMax;
    BOOL		m_fNarrow;
    int			m_SyncPhase;
public:
	CSYNCINT(){
        m_fNarrow = 0;
		Reset();
	};
	void Reset(void){
		memset(m_MSyncList, 0, sizeof(m_MSyncList));
		m_MSyncACnt = m_MSyncCnt = 0;
		m_MSyncIntMax = 0;
		m_MSyncIntPos = 0;
        m_SyncPhase = 0;
	};

	int SyncCheckSub(int am);
	int SyncCheck(void);
	void SyncInc(void);
	void SyncTrig(int d);
	void SyncMax(int d);
	int SyncStart(void);
};

#define	SSTVDEMBUFMAX	24
class CSSTVDEM
{
private:
	double  HBPF[TAPMAX+1];
	double  HBPFS[TAPMAX+1];
	double  HBPFN[TAPMAX+1];
//	double  Z[TAPMAX+1];
	CFIR2	m_BPF;
public:
	double   m_ad;
	int      m_OverFlow;
	int      m_bpf;
	int      m_bpftap;
	int      m_Type;
	int      m_LevelType;

	CIIRTANK m_iir11;
	CIIRTANK m_iir12;
	CIIRTANK m_iir13;
	CIIRTANK m_iir19;
	CIIRTANK m_iirfsk;
	CIIR     m_lpf11;
	CIIR     m_lpf12;
	CIIR     m_lpf13;
	CIIR     m_lpf19;
	CIIR     m_lpffsk;
	CPLL	 m_pll;
	CFQC     m_fqc;
	CLVL     m_lvl;
	CSLVL    m_SyncLvl;
	CHILL    m_hill;

	REPSET   *pRep;

	int         m_Skip;
	int         m_Sync;
	int         m_SyncRestart;
	int         m_SyncMode;
	int         m_SyncTime;
	int         m_SyncATime;
	int         m_VisData;
	int         m_VisCnt;
	int         m_VisTrig;
	int         m_SyncErr;
	int         m_NextMode;
	int         m_SyncAVT;

	int         m_wBase;
	int         m_wPage;
	int         m_rPage;
	int         m_wCnt;
	int         m_wLine;
	int         m_wBgn;
	int         m_rBase;

	int         m_ReqSave;
	int         m_LoopBack;

	int         m_wStgPage;
	int         m_wStgLine;

	int         m_Lost;
public:
	int         m_BWidth;
	short       *m_Buf;
	short       *m_B12;

	int         m_RxBufAllocSize;
	short       *m_StgBuf;
	short       *m_StgB12;
public:
	void        SetSenseLvl(void);
	int         m_SenseLvl;
	double      m_SLvl;
	double      m_SLvl2;
	double      m_SLvl3;

	int         m_ScopeFlag;
	CScope      m_Scope[2];

	CTICK       *pTick;
	int         m_Tick;
	int         m_TickFreq;
	void        SetTickFreq(int f);

	double      m_CurSig;
	CSmooz      m_Avg;
	CSmooz      m_AFCAVG;
	int         m_afc;
	int         m_AFCCount;
	double      m_AFCData;
	double      m_AFCLock;
	double      m_AFCDiff;
	int         m_AFCFQ;
	int         m_AFCFlag;
	int         m_AFCGard;
	int         m_AFCDis;
	int         m_AFCInt;

	double		m_AFC_LowVal;		// (Center - SyncLow) * 16384 / BWH
	double		m_AFC_HighVal;		// (Center - SyncHigh) * 16384 / BWH
	double		m_AFC_SyncVal;		// (Center - Sync) * 16384 / BWH
	double		m_AFC_BWH;			// BWH / 16384.0;

    double		m_AFC_OFFSET;		// 128
public:
	int         m_MSync;
	CSYNCINT    m_sint1;
	CSYNCINT    m_sint2;
    CSYNCINT	m_sint3;

public:
#define FSKGARD     100
#define FSKINTVAL   22
#define FSKSPACE    2100
	int         m_fskdecode;
	int         m_fskrec;
	int         m_fskmode;
	int         m_fsktime;
	int         m_fskcnt;
	int         m_fskbcnt;
	int         m_fsknexti;
	double      m_fsknextd;
	BYTE        m_fsks;
	BYTE        m_fskc;
	BYTE        m_fskdata[20];
	char        m_fskcall[20];
	int			m_fskNRrec;
    int			m_fskNR;
	char		m_fskNRS[20];

	//------ リピータ
	int         m_Repeater;
	int         m_RepSQ;
	int         m_RepTone;
	int         m_repmode;
	int         m_reptime;
	int         m_repcount;
	int         m_repsig;
	int         m_repANS;
	int         m_repRLY;
	int         m_repRX;
	int         m_repTX;

	int         m_RSLvl;
	int         m_RSLvl2;

    int			m_fNarrow;
#if 0
	int         m_repD1, m_repD2;
#endif
	void Repeater(int d17, int d12, int d19);
	void SetRepeater(int sw);
	void SetRepSenseLvl(void);
	void InitRepeater(void);

public:
	CSSTVDEM();
	~CSSTVDEM();
	void Start(void);
	void Start(int mode, int f);
	void Stop(void);
	void Do(double d);
	void IncWP(void);
	void Idle(double d);
	void CalcBPF(double *H1, double *H2, double *H3, int &bpftap, int bpf, int mode);
	void CalcNarrowBPF(double *H3, int bpftap, int bpf, int mode);
	void CalcBPF(void);
	void SetBPF(int bpf);
	void FreeRxBuff(void);
	void OpenCloseRxBuff(void);
	void SyncFreq(double d);
	void DecodeFSK(int m, int s);
	void InitAFC(void);
	void InitTone(int dfq);
	void SetWidth(int fNarrow);

	double  m_d;
	double  m_dd;
	int     m_n;
};

class CSSTVMOD
{
public:
	int         m_wLine;

	int         m_wPnt;
	int         m_rPnt;
	int         m_Cnt;
	int         m_rCnt;

	int         m_iPos;
	double      m_dPos;

	int         m_sCnt;

	CVCO        m_vco;

	int         m_bpf;
	int         m_bpftap;
	int         m_lpf;
	double      m_lpffq;
	double      m_outgain;
	double      m_outgainG;
	double      m_outgainB;
	double      m_outgainR;
	int         m_TuneFreq;

	int         m_tune;
	int         m_Lost;
public:
	CFIR2		m_BPF;
//	double      HBPF[TAPMAX+1];
//	double      Z[TAPMAX+1];

	int         m_TXBufLen;
	int         m_TXMax;
	short       *m_TXBuf;
	CSmooz      avgLPF;

	int         m_RowCnt;
	short       *pRow;

	int         m_VariOut;
	UINT        m_VariR;
	UINT        m_VariG;
	UINT        m_VariB;
public:
	CSSTVMOD();
	~CSSTVMOD();

	void        InitTXBuf(void);
	void        Write(short fq);
	void        Write(short fq, double tim);
	void        WriteC(short fq, double cnt);
	double      Do(void);
	void        CalcFilter(void);
	inline int  GetBufCnt(void){return m_Cnt;};
	void        WriteCWID(char c);
	void        WriteFSK(BYTE c);
	void        OpenTXBuf(int s);
	void        CloseTXBuf(void);
	inline void SetRow(short *p, int len){
		if( p != NULL ){
			pRow = p;
			m_RowCnt = len;
		}
		else {
			m_RowCnt = 0;
			pRow = NULL;
		}
	};
	inline int  GetRowCnt(void){
		return m_RowCnt;
	};
	void        InitGain(void);
};
#pragma pack(pop)
#endif

