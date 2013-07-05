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

#include "Comm.h"
#include "ComLib.h"

#define	WAITSTAT	0

#define	DEFFSOUND	3

COMMPARA	COMM;
void InitCOMMPara(void)
{
	COMM.change = 1;
}

__fastcall CComm::CComm(void)
{
	m_CreateON = FALSE;	// クリエイトフラグ
	m_Command = 0;
	m_fHnd = NULL;		// ファイルハンドル
	m_ptt = m_scan = 0;
	m_pEXT = NULL;
}

/*#$%
==============================================================
	通信回線をクローズする
--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
==============================================================
*/
void __fastcall CComm::Close(void)
{
	if( m_CreateON == TRUE ){
		if( m_pEXT != NULL ){
			delete m_pEXT;
			m_pEXT = NULL;
		}
		else {
			::CloseHandle(m_fHnd);
		}
		m_CreateON = FALSE;
	}
}

/*#$%
==============================================================
	通信回線をオープンする
--------------------------------------------------------------
PortName : 回線の名前
pCP		 : COMMPARAのポインタ（ヌルの時はデフォルトで初期化）
RBufSize : 受信バッファのサイズ(default=2048)
TBufSize : 送信バッファのサイズ(default=2048)
--------------------------------------------------------------
TRUE/FALSE
--------------------------------------------------------------
==============================================================
*/
BOOL __fastcall CComm::Open(LPCTSTR PortName)
{
	if( m_CreateON == TRUE ) Close();
	m_fHnd = ::CreateFile( PortName, GENERIC_READ | GENERIC_WRITE,
						0, NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL
	);
	if( m_fHnd == INVALID_HANDLE_VALUE ){
		AnsiString as = "\\\\.\\";
        as += PortName;
		m_fHnd = ::CreateFile( as.c_str(), GENERIC_READ | GENERIC_WRITE,
						0, NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL
	);
	}
	if( m_fHnd == INVALID_HANDLE_VALUE ){
		m_pEXT = new CEXTFSK(PortName);
		if( m_pEXT->IsLib() ){
			LONG para;
			para = (45 << 16) | (5 << 2);
			m_pEXT->Open(para);
			m_CreateON = TRUE;
			return TRUE;
		}
		else {
			delete m_pEXT;
			m_pEXT = NULL;
		}
		return FALSE;
	}
	// setup device buffers
	if( ::SetupComm( m_fHnd, DWORD(1024), DWORD(2) ) == FALSE ){
		::CloseHandle(m_fHnd);
		return FALSE;
	}

	// purge any information in the buffer
	::PurgeComm( m_fHnd, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );

	// set up for overlapped I/O
	COMMTIMEOUTS TimeOut;

	TimeOut.ReadIntervalTimeout = 0xffffffff;
	TimeOut.ReadTotalTimeoutMultiplier = 0;
	TimeOut.ReadTotalTimeoutConstant = 0;
	TimeOut.WriteTotalTimeoutMultiplier = 0;
	TimeOut.WriteTotalTimeoutConstant = 20000;
//	TimeOut.WriteTotalTimeoutConstant = 1;
	if( !::SetCommTimeouts( m_fHnd, &TimeOut ) ){
		::CloseHandle( m_fHnd );
		return FALSE;
	}
	::GetCommState( m_fHnd, &m_dcb );
	m_dcb.BaudRate = 9600;
	m_dcb.fBinary = TRUE;
	m_dcb.ByteSize = 8;
	m_dcb.Parity = NOPARITY;
	m_dcb.StopBits = ONESTOPBIT;
	m_dcb.XonChar = 0x11;	// XON
	m_dcb.XoffChar = 0x13;	// XOFF
	m_dcb.fParity = 0;
	m_dcb.fOutxCtsFlow = FALSE;
	m_dcb.fInX = m_dcb.fOutX = FALSE;
	m_dcb.fOutxDsrFlow = FALSE;
	m_dcb.EvtChar = 0x0d;

	m_dcb.fRtsControl = RTS_CONTROL_DISABLE;		// 送信禁止
	m_dcb.fDtrControl = DTR_CONTROL_DISABLE;		// 送信禁止

//	m_dcb.fTXContinueOnXoff = TRUE;
	m_dcb.XonLim = USHORT(1024/4);			// 1/4 of RBufSize
	m_dcb.XoffLim = USHORT(1024*3/4);		// 3/4 of RBufSize
	m_dcb.DCBlength = sizeof( DCB );

	if( !::SetCommState( m_fHnd, &m_dcb ) ){
		::CloseHandle( m_fHnd );
		return FALSE;
	}

	// get any early notifications
	if( !::SetCommMask( m_fHnd, EV_RXFLAG ) ){
		::CloseHandle(m_fHnd);
		return FALSE;
	}
	m_CreateON = TRUE;
	return TRUE;
}
//-----------------------------------------------------------------
// PTT切り替え用
int __fastcall CComm::PTTOpen(void)
{
	if( m_CreateON == FALSE ){
		if( !strcmpi(sys.m_TxRxName.c_str(), "NONE") ) return FALSE;
		Open(sys.m_TxRxName.c_str());
		if( m_CreateON == FALSE ) return FALSE;
	}
	return TRUE;
}
//-----------------------------------------------------------------
// PTT切り替え用
int __fastcall CComm::SetPTT(void)
{
	int rts = m_ptt;
	if( m_pEXT != NULL ){
		if( sys.m_RTSonRX && m_scan ) rts |= 2;
		m_pEXT->SetPTT(rts);
	}
	else {
		if( sys.m_RTSonRX ){
			rts |= m_scan;
			::EscapeCommFunction(m_fHnd, m_scan ? SETBREAK : CLRBREAK);
		}
		::EscapeCommFunction(m_fHnd, rts ? SETRTS : CLRRTS);
		::EscapeCommFunction(m_fHnd, m_ptt ? SETDTR : CLRDTR);
	}
	return rts | m_ptt;
}
//-----------------------------------------------------------------
// PTT切り替え用
void __fastcall CComm::SetPTT(int sw)
{
	m_ptt = sw;

	if( !PTTOpen() ) return;
	if( !SetPTT() && (!sys.m_TxRxLock) ) Close();
}
//-----------------------------------------------------------------
// RX-SCAN信号
void __fastcall CComm::SetScan(int scan)
{
	m_scan = scan;

	if( !sys.m_RTSonRX ) return;
	if( !PTTOpen() ) return;
	if( !SetPTT() && (!sys.m_TxRxLock) ) Close();
}

/*******************************************************************
	EXTFSK.DLL
*******************************************************************/
__fastcall CEXTFSK::CEXTFSK(LPCSTR pName)
{
	char Name[128];
	sprintf(Name, "%s.%s", pName, strcmpi(pName, "EXTFSK") ? "fsk" : "dll");

	fextfskOpen	= NULL;
	fextfskClose = NULL;
	fextfskIsTxBusy = NULL;
	fextfskPutChar = NULL;
	fextfskSetPTT = NULL;

	m_hLib = ::LoadLibrary(Name);
	if( m_hLib != NULL ){
		fextfskOpen	= (extfskOpen)GetProc("_extfskOpen");
		fextfskClose = (extfskClose)GetProc("_extfskClose");
		fextfskIsTxBusy = (extfskIsTxBusy)GetProc("_extfskIsTxBusy");
		fextfskPutChar = (extfskPutChar)GetProc("_extfskPutChar");
		fextfskSetPTT = (extfskSetPTT)GetProc("_extfskSetPTT");
	}
}

//---------------------------------------------------------------------
__fastcall CEXTFSK::~CEXTFSK()
{
	if( m_hLib != NULL ){
		Close();
		::FreeLibrary(m_hLib);
		m_hLib = NULL;
	}
}

//---------------------------------------------------------------------
FARPROC CEXTFSK::GetProc(LPCSTR pName)
{
	FARPROC fn = ::GetProcAddress(m_hLib, pName+1);
	if( fn == NULL ){
		fn = ::GetProcAddress(m_hLib, pName);
	}
	return fn;
}

long __fastcall CEXTFSK::Open(long para)
{
	if( !m_hLib || !fextfskOpen ) return FALSE;
	return fextfskOpen(para);
}

void __fastcall CEXTFSK::Close(void)
{
	if( !m_hLib || !fextfskClose ) return;
	fextfskClose();
}

long __fastcall CEXTFSK::IsTxBusy(void)
{
	if( !m_hLib || !fextfskIsTxBusy ) return FALSE;
	return fextfskIsTxBusy();
}

void __fastcall CEXTFSK::PutChar(BYTE c)
{
	if( !m_hLib || !fextfskPutChar ) return;
	fextfskPutChar(c);
}

void __fastcall CEXTFSK::SetPTT(long tx)
{
	if( !m_hLib || !fextfskSetPTT ) return;
	fextfskSetPTT(tx);
}

