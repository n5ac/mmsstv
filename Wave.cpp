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

#include "Wave.h"
//---------------------------------------------------------------------------
// 初期化
__fastcall CWave::CWave(void)
{
	m_Error = 0;
	m_InOpen = m_OutOpen = FALSE;
	m_hin = NULL;
	m_hout = NULL;
	m_InEvent = NULL;
	m_OutEvent = NULL;
	for( int i = 0; i < WAVE_FIFO_MAX; i++ ){
		m_pInBuff[i] = m_pOutBuff[i] = NULL;
	}
	m_pOutBase = m_pInBase = NULL;
	m_InBuffSize = 1024;
	m_OutBuffSize = 1024;
	m_OutFirst = FALSE;
	m_InFifoSize = 12;
	m_OutFifoSize = 4;
	m_SoundStereo = 1;
    m_StereoTX = 0;
	m_pDLL = NULL;
	::InitializeCriticalSection(&m_InCS);
	::InitializeCriticalSection(&m_OutCS);
}

//---------------------------------------------------------------------------
// 終了時実行ルーチン
__fastcall CWave::~CWave()
{
	::DeleteCriticalSection(&m_InCS);
	::DeleteCriticalSection(&m_OutCS);
    if( m_pDLL ){
    	delete m_pDLL;
		m_pDLL = NULL;
    }
}
//---------------------------------------------------------------------------
// サウンドカードの名前を得る
void __fastcall CWave::GetDeviceName(void)
{
	m_InDevs = 0;
	m_OutDevs = 0;

	WAVEINCAPS incaps;
	for( unsigned int i = 0; i < AN(m_tInDevName); i++ ){
		if( !waveInGetDevCaps(i, &incaps, sizeof(incaps)) ){
			m_tInDevName[i] = incaps.szPname;
			m_InDevs = i + 1;
		}
		else {
			m_tInDevName[i] = "";
		}
	}
	WAVEOUTCAPS outcaps;
    for( unsigned int i = 0; i < AN(m_tOutDevName); i++ ){
		if( !waveOutGetDevCaps(i, &outcaps, sizeof(outcaps)) ){
			m_tOutDevName[i] = outcaps.szPname;
            m_OutDevs = i + 1;
        }
        else {
        	m_tOutDevName[i] = "";
        }
    }
}
#if 0
//---------------------------------------------------------------------------
// サウンドカードの問い合わせ
BOOL __fastcall CWave::IsFormatSupported(LPWAVEFORMATEX pWFX, UINT IDDevice)
{
	return (::waveOutOpen(
		NULL,					// ptr can be NULL for query
		IDDevice,				// the device identifier
		pWFX,					// defines requested format
		NULL,					// no callback
		NULL,					// no instance data
		WAVE_FORMAT_QUERY		// query only, do not open device
	) ? FALSE : TRUE);
}
#endif
//---------------------------------------------------------------------------
// サウンドカードの入力でのオープン
//	WFX.wFormatTag = WAVE_FORMAT_PCM;
//	WFX.nChannels = 1;
//	WFX.wBitsPerSample = 16;
//	WFX.nSamplesPerSec = 11025;
//	WFX.nBlockAlign = WORD(WFX.nChannels *(WFX.wBitsPerSample/8));
//	WFX.nAvgBytesPerSec = WFX.nBlockAlign * WFX.nSamplesPerSec;
//	WFX.cbSize = 0;
BOOL __fastcall CWave::InOpen(LPWAVEFORMATEX pWFX, UINT IDDevice, DWORD Size)
{
	if( m_pDLL ){
    	m_InOpen = m_pDLL->InOpen(pWFX->nSamplesPerSec, Size);
		if( !m_InOpen ) PumpMessages();
		return m_InOpen;
	}
	int i;

	if( m_InOpen ) InClose();
	m_Error = 0;
	m_InWait = m_InOver = FALSE;
	m_InWP = m_InRP = m_InBC = 0;
	m_IWFX = *pWFX;
	m_InBuffSize = Size * (m_IWFX.wBitsPerSample/8) * m_IWFX.nChannels;
	m_InMemSize = sizeof(WAVEHDR) + m_InBuffSize;
	if( m_InMemSize & 3 ) m_InMemSize += 4 - (m_InMemSize & 3);
	m_InAllocSize = m_InMemSize * m_InFifoSize;
	m_InEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	// サウンドカードのオープン
	if( (m_Error = ::waveInOpen( &m_hin, IDDevice, pWFX, (DWORD)WaveInProc, (DWORD)this, CALLBACK_FUNCTION ) ) != MMSYSERR_NOERROR ){
		InClose();
		return FALSE;
	}
	// バッファの準備
	m_pInBase = new char[m_InAllocSize];
	::VirtualLock(m_pInBase, m_InAllocSize);
	memset(m_pInBase, 0, m_InAllocSize);
	LPSTR p = m_pInBase;
	for( i=0; i < m_InFifoSize; i++, p += m_InMemSize ){
		m_pInBuff[i] = (WAVEHDR *)p;
		((WAVEHDR *)p)->dwBufferLength = m_InBuffSize;
		((WAVEHDR *)p)->dwFlags = 0;
		((WAVEHDR *)p)->dwUser = NULL;
		((WAVEHDR *)p)->dwBytesRecorded = NULL;
		((WAVEHDR *)p)->lpData = p + sizeof(WAVEHDR);
		if( (m_Error = ::waveInPrepareHeader(m_hin, (WAVEHDR *)p, sizeof(WAVEHDR)) ) != MMSYSERR_NOERROR ){
			InClose();
			return FALSE;
		}
		if( (m_Error = ::waveInAddBuffer(m_hin, (WAVEHDR *)p, sizeof(WAVEHDR)) ) != MMSYSERR_NOERROR ){
			InClose();
			return FALSE;
		}
	}
	// 取りこみの開始
	if( (m_Error = ::waveInStart(m_hin) ) != MMSYSERR_NOERROR ){
		InClose();
		return FALSE;
	}
	m_InOpen = TRUE;
	return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall CWave::InClose()
{
	if( m_pDLL ){
		m_pDLL->InClose();
        m_InOpen = FALSE;
		PumpMessages();
		return;
	}
	int i;

	if(NULL != m_hin){
		::waveInReset(m_hin);
		if( m_pInBase != NULL ){
			for( i=0; i < m_InFifoSize; i++ ){
				if( m_pInBuff[i] != NULL ){
					if( m_pInBuff[i]->dwFlags & WHDR_PREPARED ){
						::waveInUnprepareHeader(m_hin, m_pInBuff[i], sizeof(WAVEHDR));
					}
				}
			}
		}
		::waveInClose(m_hin);
		m_hin = NULL;
		if( m_pInBase != NULL ){
			::VirtualUnlock(m_pInBase, m_InAllocSize);
			delete m_pInBase;
			m_pInBase = NULL;
		}
	}
	if(m_InEvent != NULL){
		::CloseHandle(m_InEvent);
		m_InEvent = NULL;
	}
	m_InOpen = FALSE;
}
//---------------------------------------------------------------------------
BOOL __fastcall CWave::InRead(double* pData, int Len)
{
	if( m_pDLL ){
		return m_pDLL->InRead(pData, Len);
	}
	int i;

	if( !m_InOpen ){
		m_Error = 1;
		return FALSE;
	}
	if( m_InOver ){
		m_Error = 1;
		InClose();
		return FALSE;
	}

	::EnterCriticalSection(&m_InCS);
	if( !m_InBC ){                    // まだデータが存在しない時
		m_InWait++;
		::LeaveCriticalSection(&m_InCS);
		// バッファにデータが溜まるまで待つ
		if( ::WaitForSingleObject( m_InEvent, WAVE_TIMEOUT_EVENT ) != WAIT_OBJECT_0 ){
			m_Error = 1;
			InClose();
			return FALSE;
		}
	}
	else {
		::LeaveCriticalSection(&m_InCS);
	}
	// データを浮動少数点に変換
	LPWAVEHDR hp = m_pInBuff[m_InRP];
	if( m_IWFX.wBitsPerSample == 16 ){	// 16bits
		SHORT *rp = (SHORT *)hp->lpData;
		if( m_IWFX.nChannels == 2 ){
			if( m_SoundStereo == 1 ){	// Left
				for( i = 0; i < Len; i++, pData++ ){
					*pData = double(*rp++);
					rp++;
				}
			}
			else {						// Right
				for( i = 0; i < Len; i++, pData++ ){
					rp++;
					*pData = double(*rp++);
				}
			}
		}
		else {
			for( i = 0; i < Len; i++, pData++ ){
				*pData = double(*rp++);
			}
		}
	}
	else {								// 8bits
		union {
			struct {
				BYTE	low;
				BYTE	high;
			}bd;
			SHORT	wd;
		}data;
		data.bd.low = 0;
		BYTE *rp = (BYTE *)hp->lpData;
		if( m_IWFX.nChannels == 2 ){
			if( m_SoundStereo == 1 ){	// Left
				for( i = 0; i < Len; i++, pData++ ){
					data.bd.high = BYTE((*rp++) - 128);
					*pData = double(data.wd);
					rp++;
				}
			}
			else {						// Right
				for( i = 0; i < Len; i++, pData++ ){
					rp++;
					data.bd.high = BYTE((*rp++) - 128);
					*pData = double(data.wd);
				}
			}
		}
		else {
			for( i = 0; i < Len; i++, pData++ ){
				data.bd.high = BYTE((*rp++) - 128);
				*pData = double(data.wd);
			}
		}
	}
	hp->dwBytesRecorded = NULL;
	::waveInAddBuffer(m_hin, hp, sizeof(WAVEHDR));
	m_InBC--;     // １命令で展開されるので CriticalSection は不要
	m_InRP++;
	if( m_InRP >= m_InFifoSize){
		m_InRP = 0;
	}
	return TRUE;
}
//---------------------------------------------------------------------------
//void CALLBACK WaveProc(hWave, uMsg, dwInstance, dwParam1, dwParam2)
//
//HWAVE hWave;	/* ウェーブフォーム デバイスのハンドル	*/
//UINT uMsg;	/* 送るメッセージ	*/
//DWORD dwInstance;	/* インスタンス データ	*/
//DWORD dwParam1;	/* アプリケーション定義のパラメータ	*/
//DWORD dwParam2;	/* アプリケーション定義のパラメータ	*/
void CALLBACK WaveInProc(HWAVE m_hin, UINT uMsg, CWave* pWave, DWORD dwParam1, DWORD dwParam2 )
{
	if( uMsg == MM_WIM_DATA ){
		::EnterCriticalSection(&pWave->m_InCS);
		pWave->m_InBC++;
		pWave->m_InWP++;
		if( pWave->m_InWP >= pWave->m_InFifoSize ) pWave->m_InWP = 0;
		if( pWave->m_InBC > pWave->m_InFifoSize ) pWave->m_InOver = TRUE;
		if(pWave->m_InWait){
			pWave->m_InWait--;
			::SetEvent(pWave->m_InEvent);
		}
		::LeaveCriticalSection(&pWave->m_InCS);
	}
}
//---------------------------------------------------------------------------
// サウンドカードの出力でのオープン
//	WFX.wFormatTag = WAVE_FORMAT_PCM;
//	WFX.nChannels = 1;
//	WFX.wBitsPerSample = 16;
//	WFX.nSamplesPerSec = 11025;
//	WFX.nBlockAlign = WORD(WFX.nChannels *(WFX.wBitsPerSample/8));
//	WFX.nAvgBytesPerSec = WFX.nBlockAlign * WFX.nSamplesPerSec;
//	WFX.cbSize = 0;
BOOL __fastcall CWave::OutOpen(LPWAVEFORMATEX pWFX, UINT IDDevice, DWORD Size)
{
	if( m_pDLL ){
		m_OutOpen = m_pDLL->OutOpen(pWFX->nSamplesPerSec, Size);
		if( !m_OutOpen ) PumpMessages();
        return m_OutOpen;
	}
	if( m_OutOpen ) OutAbort();
	m_Error = 0;
	m_OutBCC = 0x7fffffff;
	m_OutWait = FALSE;
	m_OutUnder = FALSE;
	m_OutWP = m_OutRP = m_OutBC = 0;
	m_OWFX = *pWFX;
	m_OutBuffSize = Size * (m_OWFX.wBitsPerSample/8) * m_OWFX.nChannels;
	m_OutMemSize = sizeof(WAVEHDR) + m_OutBuffSize;
	if( m_OutMemSize & 3 ) m_OutMemSize += 4 - (m_OutMemSize & 3);
	m_OutAllocSize = m_OutMemSize * m_OutFifoSize;
	m_OutEvent = ::CreateEvent(NULL, FALSE,FALSE,NULL);
	if( (m_Error = ::waveOutOpen( &m_hout, IDDevice , pWFX, (DWORD)WaveOutProc, (DWORD)this, CALLBACK_FUNCTION ) ) != MMSYSERR_NOERROR ){
		OutAbort();
		return FALSE;
	}
	if( (m_Error = ::waveOutPause(m_hout))!= MMSYSERR_NOERROR ){
		OutAbort();
		return FALSE;
	}
	// バッファーの準備
	m_pOutBase = new char[m_OutAllocSize];
	::VirtualLock(m_pOutBase, m_OutAllocSize);
	memset(m_pOutBase, 0, m_OutAllocSize);
	LPSTR p = m_pOutBase;
	for(int i = 0; i < m_OutFifoSize; i++, p += m_OutMemSize ){
		m_pOutBuff[i] = (WAVEHDR *)p;
		((WAVEHDR *)p)->dwBufferLength = m_OutBuffSize;
		((WAVEHDR *)p)->dwFlags = 0;
		((WAVEHDR *)p)->dwUser = NULL;
		((WAVEHDR *)p)->dwLoops = NULL;
		((WAVEHDR *)p)->lpData = p + sizeof(WAVEHDR);
		if(	(m_Error = ::waveOutPrepareHeader(m_hout, (WAVEHDR *)p, sizeof(WAVEHDR)) ) != MMSYSERR_NOERROR ){
			OutAbort();
			return FALSE;
		}
	}
	m_OutOpen = TRUE;
	m_OutFirst = TRUE;
	return TRUE;
}
#if 0
//---------------------------------------------------------------------------
DWORD __fastcall CWave::GetOutVolume(void)
{
	if( !m_OutOpen ){
		return 0x8000;
	}
	DWORD vol;
	::waveOutGetVolume(m_hout, &vol);
	return vol;
}
//---------------------------------------------------------------------------
BOOL __fastcall CWave::SetOutVolume(DWORD vol)
{
	if( !m_OutOpen ){
		return FALSE;
	}
	::waveOutSetVolume(m_hout, vol);
	return TRUE;
}
#endif
//---------------------------------------------------------------------------
BOOL __fastcall CWave::OutWrite(double *pData, int Len)
{
	if( m_pDLL ){
		return m_pDLL->OutWrite(pData, Len);
	}
	if( !Len ) return 0;
	int i;
	if( !m_OutOpen ){
		m_Error = 1;
		return FALSE;
	}
	if( m_OutUnder ){
		m_Error = 1;
		OutAbort();
		return FALSE;
	}

	// 送信バッファ空き待ち
	EnterCriticalSection(&m_OutCS);
	if( m_OutBC >= m_OutFifoSize ){
		m_OutWait++;
		::LeaveCriticalSection(&m_OutCS);
		if( ::WaitForSingleObject( m_OutEvent, WAVE_TIMEOUT_EVENT ) != WAIT_OBJECT_0 ){
			m_Error = 2;
			OutAbort();
			return FALSE;
		}
	}
	else {
		::LeaveCriticalSection(&m_OutCS);
	}

	// データの変換
	LPWAVEHDR hp = m_pOutBuff[m_OutWP];
	if(m_OWFX.wBitsPerSample == 16){		// 16bits
		SHORT *wp = (SHORT *)hp->lpData;
		if( m_OWFX.nChannels == 2 ){
			if( m_StereoTX ){
				if( m_SoundStereo == 1 ){	// Left
					for( i = 0; i < Len; i++, pData++ ){
						*wp++ = (SHORT)(*pData);
						*wp++ = 0;
					}
				}
				else {						// Right
					for( i = 0; i < Len; i++, pData++ ){
						*wp++ = 0;
						*wp++ = (SHORT)(*pData);
					}
				}
            }
            else {
				for( i = 0; i < Len; i++, pData++ ){
					*wp++ = (SHORT)(*pData);
					*wp++ = (SHORT)(*pData);
				}
            }
		}
		else {
			for( i = 0; i < Len; i++, pData++ ){
				*wp++ = (SHORT)(*pData);
			}
		}
	}
	else {									// 8bits
		BYTE *wp = (BYTE *)hp->lpData;
		union {
			struct {
				BYTE	low;
				BYTE	high;
			}bd;
			SHORT	wd;
		}data;
		if( m_OWFX.nChannels == 2 ){
			for( i = 0; i < Len; i++, pData++ ){
				data.wd = (SHORT)((*pData) + 32768);
				*wp++ = data.bd.high;
				*wp++ = data.bd.high;
			}
		}
		else {
			for( i = 0; i < Len; i++, pData++ ){
				data.wd = (SHORT)((*pData) + 32768);
				*wp++ = data.bd.high;
			}
		}
	}
	::waveOutWrite(m_hout, hp, sizeof(WAVEHDR) );
	m_OutBC++;    // １命令で展開されるので CriticalSection は不要
	if(	m_OutFirst ){
		if( (m_OutBC >= 8) || (m_OutBC >= (m_OutFifoSize-1)) ){
			m_OutFirst = FALSE;
			::waveOutRestart( m_hout );
		}
	}
	m_OutWP++;
	if( m_OutWP >= m_OutFifoSize){
		m_OutWP = 0;
	}
	return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall CWave::OutFlush()
{
	if( m_pDLL ){
		m_pDLL->OutFlush();
		return;
	}
	if(m_hout != NULL){
		// バッファ送信待ち
		while(1){
			::EnterCriticalSection(&m_OutCS);
			if( m_OutBC > 0 ){	          // 未送出データが存在する場合
				m_OutWait++;
				::LeaveCriticalSection(&m_OutCS);
				if( ::WaitForSingleObject( m_OutEvent, WAVE_TIMEOUT_EVENT ) != WAIT_OBJECT_0 ){
					m_Error = 2;
					break;
				}
			}
			else {
				::LeaveCriticalSection(&m_OutCS);
				break;
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall CWave::OutAbort()
{
	if( m_pDLL ){
		m_pDLL->OutAbort();
		m_OutOpen = FALSE;
		PumpMessages();
		return;
	}
	if(m_hout != NULL){
		::waveOutReset(m_hout);
		::Sleep(1);     // for the timing
		// バッファの解放
		if( m_pOutBase != NULL ){
			for(int i = 0; i < m_OutFifoSize; i++ ){
				if( m_pOutBuff[i] != NULL ){
					if( m_pOutBuff[i]->dwFlags & WHDR_PREPARED ){
						::waveOutUnprepareHeader(m_hout, m_pOutBuff[i], sizeof(WAVEHDR));
					}
				}
			}
		}
		::waveOutClose(m_hout);
		m_hout = NULL;
		if( m_pOutBase != NULL ){
			::VirtualUnlock(m_pOutBase, m_OutAllocSize);
			delete m_pOutBase;
			m_pOutBase = NULL;
		}
	}
	if(m_OutEvent != NULL){
		::CloseHandle(m_OutEvent);
		m_OutEvent = NULL;
	}
	m_OutOpen = FALSE;
}
//---------------------------------------------------------------------------
//void CALLBACK WaveProc(hWave, uMsg, dwInstance, dwParam1, dwParam2)
//
//HWAVE hWave;	/* ウェーブフォーム デバイスのハンドル	*/
//UINT uMsg;	/* 送るメッセージ	*/
//DWORD dwInstance;	/* インスタンス データ	*/
//DWORD dwParam1;	/* アプリケーション定義のパラメータ	*/
//DWORD dwParam2;	/* アプリケーション定義のパラメータ	*/
void CALLBACK WaveOutProc(HWAVE m_hout, UINT uMsg, CWave* pWave, DWORD dwParam1, DWORD dwParam2 )
{
	if( uMsg == WOM_DONE ){
		EnterCriticalSection(&pWave->m_OutCS);
		pWave->m_OutBCC--;
		pWave->m_OutBC--;
		pWave->m_OutRP++;
		if( pWave->m_OutRP >= pWave->m_OutFifoSize) pWave->m_OutRP = 0;
		if( !pWave->m_OutBC ) pWave->m_OutUnder = TRUE;
		if(pWave->m_OutWait){
			pWave->m_OutWait--;
			SetEvent(pWave->m_OutEvent);
		}
		LeaveCriticalSection(&pWave->m_OutCS);
	}
}

//---------------------------------------------------------------------------
int __fastcall CWave::GetInBC(void)
{
	if( m_pDLL ){
		return m_pDLL->GetInBC();
	}
	else {
		return m_InBC;
	}
}
//---------------------------------------------------------------------------
int __fastcall CWave::GetOutBC(void)
{
	if( m_pDLL ){
		return m_pDLL->GetOutBC();
	}
	else {
		return m_OutBC;
	}
}
//---------------------------------------------------------------------------
int __fastcall CWave::GetOutBCC(void)
{
	if( m_pDLL ){
		return m_pDLL->GetOutBCC();
	}
	else {
		return m_OutBCC;
	}
}
//---------------------------------------------------------------------------
void __fastcall CWave::SetOutBCC(int count)
{
	if( m_pDLL ){
		m_pDLL->SetOutBCC(count);
	}
	else {
		m_OutBCC = count;
	}
}
//---------------------------------------------------------------------------
int __fastcall CWave::IsInBufCritical(void)
{
	if( m_pDLL ){
		return m_pDLL->IsInBufCritical();
	}
	else {
		return ( m_InBC >= (m_InFifoSize/2) ) ? 1 : 0;
	}
}
//---------------------------------------------------------------------------
int __fastcall CWave::IsOutBufCritical(void)
{
	if( m_pDLL ){
		return m_pDLL->IsOutBufCritical();
	}
	else {
		return (m_OutBC <= (m_OutFifoSize/2)) ? 1 : 0;
	}
}
//---------------------------------------------------------------------------
int __fastcall CWave::IsOutBufFull(void)
{
	if( m_pDLL ){
		return m_pDLL->IsOutBufFull();
	}
	else {
		return (m_OutBC >= m_OutFifoSize) ? 1 : 0;
	}
}
//---------------------------------------------------------------------------
void __fastcall CWave::SetPTT(LONG tx)
{
	if( m_pDLL ) m_pDLL->SetPTT(tx);
}
//---------------------------------------------------------------------------
int __fastcall CWave::GetTimeout(void)
{
	if( m_pDLL ){
    	return m_pDLL->GetTimeout();
    }
    else {
		return 5000;
    }
}
//---------------------------------------------------------------------------
void __fastcall CWave::UpdateDevice(int ID)
{
	if( ID == -2 ){
		if( (m_pDLL == NULL) || m_pDLL->IsNameChange() ){
			if( m_pDLL ) delete m_pDLL;
			m_pDLL = new CXWave(sys.m_SoundMMW.c_str());
        }
    }
    else if( m_pDLL != NULL ){
		delete m_pDLL;
        m_pDLL = NULL;
    }
}
//---------------------------------------------------------------------------
void __fastcall CWave::PumpMessages(void)
{
	if( m_pDLL ){
		m_pDLL->PumpMessages();
	}
}
//***************************************************************************
// CXWave class
//---------------------------------------------------------------------------
__fastcall CXWave::CXWave(LPCSTR pName)
{
	m_ItemName = pName;

	char Name[MAX_PATH];
	if( !*GetEXT(pName) ){
		wsprintf(Name, "%s.mmw", pName);
		pName = Name;
    }

	m_hLib = ::LoadLibrary(pName);
	if( m_hLib ){

		fmmwPumpMessages = PROC(mmwPumpMessages);
		fmmwSetPTT = PROC(mmwSetPTT);
		fmmwGetTimeout = PROC(mmwGetTimeout);

		fmmwInOpen = PROC(mmwInOpen);
		fmmwInClose = PROC(mmwInClose);
		fmmwInRead = PROC(mmwInRead);

		fmmwGetInExist = PROC(mmwGetInExist);
		fmmwIsInCritical = PROC(mmwIsInCritical);

		fmmwOutOpen = PROC(mmwOutOpen);
		fmmwOutAbort = PROC(mmwOutAbort);
		fmmwOutFlush = PROC(mmwOutFlush);
		fmmwOutWrite = PROC(mmwOutWrite);

		fmmwIsOutCritical = PROC(mmwIsOutCritical);
		fmmwIsOutFull = PROC(mmwIsOutFull);
		fmmwGetOutRemaining = PROC(mmwGetOutRemaining);
		fmmwGetOutCounter = PROC(mmwGetOutCounter);
		fmmwSetOutCounter = PROC(mmwSetOutCounter);


		if( !m_hLib ){
			FreeLib();
		}
	}
}
//---------------------------------------------------------------------------
__fastcall CXWave::~CXWave(void)
{
	if( m_hLib ){
		fmmwOutAbort();
        fmmwInClose();
    }
	FreeLib();
}
//---------------------------------------------------------------------------
FARPROC __fastcall CXWave::GetProc(LPCSTR pName)
{
	if( !m_hLib ) return NULL;

	FARPROC fn = ::GetProcAddress(m_hLib, pName+1);	//ja7ude 0521
	if( fn == NULL ){
		fn = ::GetProcAddress(m_hLib, pName);
		if( fn == NULL ) FreeLib();
	}
	return fn;
}
//---------------------------------------------------------------------------
void __fastcall CXWave::FreeLib(void)
{
	if( m_hLib ){
		FreeLibrary(m_hLib);
		m_hLib = NULL;
	}
}
//---------------------------------------------------------------------------
BOOL __fastcall CXWave::IsNameChange(void)
{
	return strcmpi(m_ItemName.c_str(), sys.m_SoundMMW.c_str());
}
//---------------------------------------------------------------------------
void __fastcall CXWave::PumpMessages(void)
{
	if( !IsLib() ) return;

	fmmwPumpMessages();
}
//---------------------------------------------------------------------------
BOOL __fastcall CXWave::InOpen(int sampfreq, int size)
{
	if( !IsLib() ) return FALSE;

	return fmmwInOpen(sampfreq, size);
}
//---------------------------------------------------------------------------
void __fastcall CXWave::InClose(void)
{
	if( !IsLib() ) return;

	fmmwInClose();
}
//---------------------------------------------------------------------------
BOOL __fastcall CXWave::InRead(double *p, int len)
{
	if( !IsLib() ) return FALSE;

	int r = fmmwInRead(m_InBuff);
	if( r ){
		SHORT *s = m_InBuff;
		for( int i = 0; i < len; i++ ){
			*p++ = *s++;
		}
	}
	return r;
}
//---------------------------------------------------------------------------
int __fastcall CXWave::GetInBC(void)
{
	if( !IsLib() ) return 0;
	return fmmwGetInExist();
}
//---------------------------------------------------------------------------
int __fastcall CXWave::IsInBufCritical(void)
{
	if( !IsLib() ) return 0;
	return fmmwIsInCritical();
}
//---------------------------------------------------------------------------
BOOL __fastcall CXWave::OutOpen(int sampfreq, int size)
{
	if( !IsLib() ) return FALSE;
	return fmmwOutOpen(sampfreq, size);
}
//---------------------------------------------------------------------------
void __fastcall CXWave::OutFlush(void)
{
	if( !IsLib() ) return;
	fmmwOutFlush();
}
//---------------------------------------------------------------------------
void __fastcall CXWave::OutAbort(void)
{
	if( !IsLib() ) return;
	fmmwOutAbort();
}
//---------------------------------------------------------------------------
BOOL __fastcall CXWave::OutWrite(double *p, int len)
{
	if( !IsLib() ) return FALSE;
	SHORT *t = m_OutBuff;
	for( int i = 0; i < len; i++ ){
		*t++ = *p++;
	}
	return fmmwOutWrite(m_OutBuff);
}
//---------------------------------------------------------------------------
int __fastcall CXWave::GetOutBC(void)
{
	if( !IsLib() ) return 0;
	return fmmwGetOutRemaining();
}
//---------------------------------------------------------------------------
int __fastcall CXWave::GetOutBCC(void)
{
	if( !IsLib() ) return 0;
	return fmmwGetOutCounter();
}
//---------------------------------------------------------------------------
void __fastcall CXWave::SetOutBCC(int count)
{
	if( !IsLib() ) return;
	fmmwSetOutCounter(count);
}
//---------------------------------------------------------------------------
int __fastcall CXWave::IsOutBufCritical(void)
{
	if( !IsLib() ) return 0;
	return fmmwIsOutCritical();
}
//---------------------------------------------------------------------------
int __fastcall CXWave::IsOutBufFull(void)
{
	if( !IsLib() ) return 0;
	return fmmwIsOutFull();
}
//---------------------------------------------------------------------------
void __fastcall CXWave::SetPTT(int tx)
{
	if( !IsLib() ) return;
	fmmwSetPTT(tx);
}
//---------------------------------------------------------------------------
int __fastcall CXWave::GetTimeout(void)
{
	if( !IsLib() ) return 200;
	return fmmwGetTimeout();
}
//---------------------------------------------------------------------------

