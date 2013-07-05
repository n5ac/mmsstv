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



///----------------------------------------------------------
///  Radioコミニュケーションクラス
///
///  (C) JE3HHT Makoto.Mori
///
//---------------------------------------------------------------------------
#ifndef CradioH
#define CradioH
#include "ComLib.h"
#include "MMlink.h"
//---------------------------------------------------------------------------
#include <Classes.hpp>	//JA7UDE 0428
typedef struct {
	int		change;

	char	StrPort[32];	// ポートの名前
	int		BaudRate;		// ボーレート
	int		BitLen;			// 0-7Bit, 1-8Bit
	int		Stop;			// 0-1Bit, 1-2Bit
	int		Parity;			// 0-PN, 1-PE, 2-PO
	int		flwXON;			// Xon/Xoff ON
	int		flwCTS;			// CTS-RTS ON
	int     usePTT;         // PTT control

	long	ByteWait;		// バイト間の送信ウエイト

	int		Cmdxx;

	AnsiString	CmdInit;
	AnsiString	CmdRx;
	AnsiString	CmdTx;

	AnsiString	cmdGNR;
	int		openGNR;

	int		PollType;
	int		PollInterval;

	int		PollScan;
}CRADIOPARA;
extern	CRADIOPARA	RADIO;
#define	RADIOMENUMAX	128
typedef struct {
	AnsiString	strTTL;
    AnsiString	strCMD;
}RADIOMENU;
//---------------------------------------------------------------------------
void LoadRADIOSetup(TMemIniFile *pIniFile);
void SaveRADIOSetup(TMemIniFile *pIniFile);
//#define	CR	0x0d
//#define	LF	0x0a
#define	CRADIO_CLOSE	1
#define	RADIO_COMBUFSIZE	4096
#define	RADIO_TXBUFSIZE		256
#define	RADIO_RXBUFSIZE		256

enum {
	RADIO_POLLNULL,
	RADIO_POLLYAESUHF,
	RADIO_POLLYAESUVU,
	RADIO_POLLICOM,
	RADIO_POLLICOMN,
	RADIO_POLLOMNIVI,
	RADIO_POLLOMNIVIN,
	RADIO_POLLKENWOOD,
	RADIO_POLLKENWOODN,
	RADIO_POLLFT1000D,
	RADIO_POLLFT920,
	RADIO_POLLJST245,
	RADIO_POLLJST245N,
    RADIO_POLLFT9000,   //1.66B AA6YQ add new radios at end of list as this value is stored in mmtty.ini
    RADIO_POLLFT2000,   //1.66B AA6YQ
    RADIO_POLLFT950,    //1.66B AA6YQ
    RADIO_POLLFT450,    //1.66B AA6YQ
};

class CCradio : public TThread
{
public:
	BOOL	m_CreateON;		// クリエイトフラグ
	DCB		m_dcb;			// ＤＣＢ
	HANDLE	m_fHnd;			// ファイルハンドル
	HWND	m_wHnd;			// 親のウインドウハンドル
	UINT	m_uMsg;
	UINT	m_ID;			// メッセージのＩＤ番号
	volatile	int	m_Command;		// スレッドへのコマンド
	BOOL	m_TxAbort;		// 送信中止フラグ
	AnsiString	Name;
	CMMRadio	*m_pRadio;

	int		m_PSKGNRId;
	int 	m_OpenGNR;

	char	m_txbuf[RADIO_TXBUFSIZE];
	int		m_txcnt;
	int		m_txwp;
	int		m_txrp;

	int		m_PollCnt;
	int     m_PollTimer;
	unsigned char m_rxbuf[RADIO_RXBUFSIZE];
	int		m_rxcnt;
	int		m_FreqEvent;
	char	m_Freq[32];

	int		m_ScanAddr;
protected:
	void virtual __fastcall Execute();
	BOOL 	OpenPipe(CRADIOPARA *cp, HWND hwnd, UINT nID);
	void CatchPoll(BYTE c);
	void FreqYaesuHF(void);
	void FreqYaesuVU(void);
	void FreqICOM(void);
	void FreqKenwood(void);
	void FreqJST245(void);
	void FreqYaesu9K2K(void);

public:
	__fastcall CCradio(bool CreateSuspended);
	__fastcall ~CCradio(){
		Close();
	};
	inline BOOL IsOpen(void){
		return m_CreateON;
	};
	inline void UpdateHandle(HWND hwnd, UINT uMsg){
		m_wHnd = hwnd; m_uMsg = uMsg;
	};
	BOOL Open(CRADIOPARA *cp, HWND hwnd, UINT uMsg, UINT nID);
	void Close(void);
	void ReqClose(void);
	void WaitClose(void);
	DWORD RecvLen(void);
	int TxBusy(void);
	DWORD Read(BYTE *p, DWORD len);
	void Write(void *p, DWORD len);
	void PutChar(char c);
	void OutStr(LPCSTR fmt, ...);
	void OutLine(LPCSTR fmt, ...);
	void SendCommand(LPCSTR p);
	void SetPTT(int sw);
	void Timer(int tx, int interval);

	inline LPCSTR GetFreq(void){
		m_FreqEvent = 0;
		return m_Freq;
	};
	int IsFreqChange(LPCSTR p);
	void UpdateFreq(double freq);
};

void InitRADIOPara(void);
#endif

