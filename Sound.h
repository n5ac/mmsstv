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
#ifndef FilterH
#define FilterH
//---------------------------------------------------------------------------
#include <Classes.hpp>	//JA7UDE 0428

#include "Wave.h"
#include "fft.h"
#include "sstv.h"
//---------------------------------------------------------------------------
class CWaveFile
{
private:
	HMMIO	m_Handle;
	int		m_dis;
	BYTE	m_Head[4];
public:
	long	m_length;
	long	m_pos;
	int		m_mode;
	int		m_pause;
	int		m_autopause;
	AnsiString	m_FileName;

	__fastcall CWaveFile();
	__fastcall ~CWaveFile();
	void __fastcall FileClose(void);
	void __fastcall Rec(LPCSTR pName);
	BOOL __fastcall Play(LPCSTR pName);
	void __fastcall Rewind(void);
	void __fastcall ReadWrite(double *s, int size);
	long __fastcall GetPos(void);
	void __fastcall Seek(int n);
	int __fastcall ChangeSampFreq(LPCSTR tName, LPCSTR pName, int sSamp);
};


class TSound : public TThread
{
private:
	BOOL			m_Stop;
	WAVEFORMATEX	m_WFX;

	double  Buff[8192];

	void __fastcall ErrorMsg(void);
	double __fastcall GetScopeRange(double &low, double center, double shift);
	void __fastcall ReadWrite(double *s, int size);

    int __fastcall UpdateFifoSize(void);
	void __fastcall JobSuspend(void);
	BOOL __fastcall ReInitSound(int sw);
public:
	CWave	Wave;

protected:
	void __fastcall Execute();


public:
	__fastcall TSound(bool CreateSuspended);
	__fastcall ~TSound();

	void __fastcall SetDeviceID(void);
	void __fastcall Stop(void);
	void __fastcall ReqStop(void);
	void __fastcall WaitStop(void);
	void __fastcall TimingWait(void);

	void __fastcall InitSound(void);
	void __fastcall SetTXRX(int sw);

	void __fastcall TaskPriority(void);

	int         m_lms;
	int         m_lms_AN;
	CLMS        lms;
#if VARITEST
	CNoise      ns;
#endif
	int			m_notch;
	CNotch		notch;
	CSSTVDEM    SSTVDEM;
	CSSTVMOD    SSTVMOD;

	CWaveFile	WaveFile;


	void __fastcall FileClose(void);
	void __fastcall Rec(LPCSTR pName);
	void __fastcall Play(LPCSTR pName);
	void __fastcall Rewind(void);

	void __fastcall InitWFX(void);

	//UINT	m_IDDevice;	//ja7ude 0525
	//UINT	m_IDTXDevice;
	int		m_IDDevice;
	int		m_IDTXDevice;

	int		m_playmode;
	int		m_susp;
	int		m_suspack;

	int		m_Tx;
	int		m_ReqTx;
	int		m_ReqSpeedTest;
    DWORD	m_SpeedValue;
	void __fastcall JobSpeedTest();

	CFFT	fftIN;

	int		m_Err;
	int		m_BuffSize;
	int     m_FifoSizeRX;
	int     m_FifoSizeTX;
	int     m_ReqFifoSize;
	inline int __fastcall GetOutCount(int sw){
		return (sw ? Wave.GetOutBCC() : Wave.GetOutBC()) * m_BuffSize;
	}
	inline void __fastcall TrigBCC(void){
		Wave.SetOutBCC(Wave.GetOutBC());
	}
	inline int __fastcall GetBCC(void){
		return Wave.GetOutBCC();
	}
	int __fastcall IsBusy(void);
};

//---------------------------------------------------------------------------
class CWaveStrage
{
private:
	HMMIO	m_Handle;
	AnsiString  m_FileName;

public:
	long	m_pos;
	long    m_wpos;
	long    m_rpos;
	short   *pData;
	short   *pSync;

	__fastcall CWaveStrage();
	__fastcall ~CWaveStrage(){Close();};
	void __fastcall Close(void);
	void __fastcall Open(void);
	void __fastcall RInit(void);
	void __fastcall WInit(void);
	int __fastcall IsOpen(void){return m_Handle != NULL ? TRUE : FALSE;};
	void __fastcall Read(short *sp, int size);
	void __fastcall Write(short *sp, int size);
	void __fastcall Seek(int n);
};
//---------------------------------------------------------------------------
#endif

