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



#ifndef WAVE_H
#define WAVE_H
//---------------------------------------------------------------------------
#include <mmsystem.h>
#include "mmw.h"
#include "ComLib.h"
//---------------------------------------------------------------------------
#ifndef PROC
#define PROC(Key) ((t##Key)GetProc("_" #Key))
#endif
//---------------------------------------------------------------------------
class CXWave
{
private:
	AnsiString	m_ItemName;
	//HANDLE	m_hLib;	//ja7ude 0521
	HINSTANCE	m_hLib;

	SHORT	m_InBuff[8192];
	SHORT	m_OutBuff[8192];

	tmmwPumpMessages		fmmwPumpMessages;
	tmmwGetTimeout			fmmwGetTimeout;
	tmmwSetPTT				fmmwSetPTT;

	tmmwInOpen				fmmwInOpen;
	tmmwInClose				fmmwInClose;
	tmmwInRead				fmmwInRead;

	tmmwGetInExist			fmmwGetInExist;
	tmmwIsInCritical		fmmwIsInCritical;

	tmmwOutOpen				fmmwOutOpen;
	tmmwOutAbort			fmmwOutAbort;
	tmmwOutFlush			fmmwOutFlush;
	tmmwOutWrite			fmmwOutWrite;

	tmmwIsOutCritical		fmmwIsOutCritical;
	tmmwIsOutFull			fmmwIsOutFull;
	tmmwGetOutRemaining		fmmwGetOutRemaining;
	tmmwGetOutCounter		fmmwGetOutCounter;
	tmmwSetOutCounter		fmmwSetOutCounter;

private:
	FARPROC __fastcall GetProc(LPCSTR pName);
	void __fastcall FreeLib(void);

public:
	__fastcall CXWave(LPCSTR pName);
	__fastcall ~CXWave(void);
	inline BOOL __fastcall IsLib(void){return m_hLib != NULL;};
	BOOL __fastcall IsNameChange(void);
public:
	BOOL __fastcall InOpen(int sampfreq, int size);
	void __fastcall InClose(void);
	BOOL __fastcall InRead(double *p, int len);

	int __fastcall GetInBC(void);
	int __fastcall IsInBufCritical(void);

	BOOL __fastcall OutOpen(int sampfreq, int size);
	void __fastcall OutFlush(void);
	void __fastcall OutAbort(void);
	BOOL __fastcall OutWrite(double *p, int len);
//    inline void __fastcall OutClose(void){OutFlush(); OutAbort();};

	int __fastcall GetOutBC(void);
	int __fastcall GetOutBCC(void);
	void __fastcall SetOutBCC(int count);
	int __fastcall IsOutBufCritical(void);
	int __fastcall IsOutBufFull(void);

	void __fastcall SetPTT(int tx);
    int __fastcall GetTimeout(void);
    void __fastcall PumpMessages(void);
};
//---------------------------------------------------------------------------
#define WAVE_TIMEOUT_EVENT  2000		// バッファリクエストのタイムアウト
#define	WAVE_FIFO_MAX	    32			// 循環バッファの最大個数
//---------------------------------------------------------------------------
class CWave
{
	friend void CALLBACK WaveInProc(HWAVE m_hin, UINT uMsg, CWave* pWave, DWORD dwParam1, DWORD dwParam2 );
	friend void CALLBACK WaveOutProc(HWAVE m_hout, UINT uMsg, CWave* pWave, DWORD dwParam1, DWORD dwParam2 );

public:
	int     m_InFifoSize;
	int     m_OutFifoSize;
	int     m_SoundStereo;
    int		m_StereoTX;

	int			m_InDevs;
    int			m_OutDevs;
    AnsiString	m_tInDevName[16];
    AnsiString	m_tOutDevName[16];
private:
	LPWAVEHDR    m_pInBuff[WAVE_FIFO_MAX];	// 入力バッファ
	LPWAVEHDR    m_pOutBuff[WAVE_FIFO_MAX];	// 出力バッファ
	WAVEFORMATEX m_OWFX;		// 出力フォーマット
	WAVEFORMATEX m_IWFX;		// 入力フォーマット

	CRITICAL_SECTION m_InCS;
	CRITICAL_SECTION m_OutCS;

	HANDLE	m_InEvent;
	HANDLE	m_OutEvent;
	BOOL	m_InWait;
	BOOL	m_InOver;
	BOOL	m_OutWait;
	BOOL	m_OutUnder;

	int		m_InWP;
	int		m_InRP;

	int		m_OutWP;
	int		m_OutRP;

	int		m_Error;
	BOOL	m_InOpen;
	BOOL	m_OutOpen;

	LPSTR   m_pInBase;
	LPSTR   m_pOutBase;
	int		m_InBuffSize;
	int		m_InMemSize;
	int     m_InAllocSize;
	int		m_OutBuffSize;
	int		m_OutMemSize;
	int     m_OutAllocSize;
	BOOL	m_OutFirst;
	//HWAVE	m_hin;	//ja7ude 0521
	//HWAVE	m_hout;	//ja7ude 0521
	HWAVEIN		m_hin;
	HWAVEOUT	m_hout;

	volatile int     m_InBC;
	volatile int     m_OutBC;
	volatile int     m_OutBCC;

	CXWave	*m_pDLL;
public:
	__fastcall CWave(void);
	__fastcall ~CWave();
//	BOOL __fastcall IsFormatSupported(LPWAVEFORMATEX pWFX, UINT IDDevice);
	BOOL __fastcall InOpen(LPWAVEFORMATEX pWFX, UINT IDDevice, DWORD Size);
	inline __fastcall BOOL IsInOpen(){return m_InOpen;};
	BOOL __fastcall InRead(double *pData, int Len);
	void __fastcall InClose();
	BOOL __fastcall OutOpen(LPWAVEFORMATEX pWFX, UINT IDDevice, DWORD Size);
	inline __fastcall BOOL IsOutOpen(){return m_OutOpen;};
	BOOL __fastcall OutWrite( double *pData, int Len);
	void __fastcall OutFlush();
	void __fastcall OutAbort();
	void __fastcall OutClose(){OutFlush();OutAbort();};
//	DWORD __fastcall GetOutVolume(void);
//	BOOL __fastcall SetOutVolume(DWORD vol);

	int __fastcall GetInBC(void);
	int __fastcall GetOutBC(void);
	int __fastcall GetOutBCC(void);
	void __fastcall SetOutBCC(int count);
	
	inline int __fastcall IsInBufNull(void){return !GetInBC();};
	int __fastcall IsInBufCritical(void);
	int __fastcall IsOutBufCritical(void);
	int __fastcall IsOutBufFull(void);

	void __fastcall SetPTT(LONG tx);
    int __fastcall GetTimeout(void);
	void __fastcall UpdateDevice(int ID);
    void __fastcall PumpMessages(void);

	void __fastcall GetDeviceName(void);
};
//---------------------------------------------------------------------------
#endif

