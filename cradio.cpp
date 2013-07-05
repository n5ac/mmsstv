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

#include "Cradio.h"
#include "ComLib.h"

#define	WAITSTAT	0

CRADIOPARA	RADIO;
void InitRADIOPara(void)
{
	RADIO.change = 1;
	strcpy(RADIO.StrPort, "NONE");	// ポートの名前
	RADIO.BaudRate = 4800;			// ボーレート
	RADIO.BitLen = 1;				// 0-7Bit, 1-8Bit
	RADIO.Stop = 1;					// 0-1Bit, 1-2Bit
	RADIO.Parity = 0;				// 0-PN, 1-PE, 2-PO
	RADIO.flwXON = 0;				// Xon/Xoff ON
	RADIO.flwCTS = 0;				// CTS-RTS ON
	RADIO.usePTT = 0;
	RADIO.Cmdxx = 0;
	RADIO.CmdInit = "";
	RADIO.CmdRx = "\\$000000000F";
	RADIO.CmdTx = "\\$000000010F\\w10";
	RADIO.ByteWait = 0;
	RADIO.cmdGNR = "";
	RADIO.openGNR = 0;

	RADIO.PollType = 0;
	RADIO.PollInterval = 5;
	RADIO.PollScan = 0;
}

void LoadRADIOSetup(TMemIniFile *pIniFile)
{
	AnsiString as = RADIO.StrPort;
	as = pIniFile->ReadString("RADIO", "PortName", as);
	StrCopy(RADIO.StrPort, as.c_str(), 31);
	RADIO.BaudRate = pIniFile->ReadInteger("RADIO", "BaudRate", RADIO.BaudRate);
	RADIO.BitLen = pIniFile->ReadInteger("RADIO", "BitLen", RADIO.BitLen);
	RADIO.Stop = pIniFile->ReadInteger("RADIO", "Stop", RADIO.Stop);
	RADIO.Parity = pIniFile->ReadInteger("RADIO", "Parity", RADIO.Parity);
	RADIO.flwXON = pIniFile->ReadInteger("RADIO", "flwXON", RADIO.flwXON);
	RADIO.flwCTS = pIniFile->ReadInteger("RADIO", "flwCTS", RADIO.flwCTS);
	RADIO.usePTT = pIniFile->ReadInteger("RADIO", "usePTT", RADIO.usePTT);

	RADIO.ByteWait = pIniFile->ReadInteger("RADIO", "ByteWait", RADIO.ByteWait);

	RADIO.Cmdxx = pIniFile->ReadInteger("RADIO", "Cmdxx", RADIO.Cmdxx);
	RADIO.CmdInit = pIniFile->ReadString("RADIO", "CmdInit", RADIO.CmdInit);
	RADIO.CmdRx = pIniFile->ReadString("RADIO", "CmdRx", RADIO.CmdRx);
	RADIO.CmdTx = pIniFile->ReadString("RADIO", "CmdTx", RADIO.CmdTx);

	RADIO.cmdGNR = pIniFile->ReadString("RADIO", "FileGNR", RADIO.cmdGNR);
	RADIO.openGNR = pIniFile->ReadInteger("RADIO", "OpenGNR", RADIO.openGNR);

	RADIO.PollType = pIniFile->ReadInteger("RADIO", "PollType", RADIO.PollType);
	RADIO.PollInterval = pIniFile->ReadInteger("RADIO", "PollInterval", RADIO.PollInterval);
}
void SaveRADIOSetup(TMemIniFile *pIniFile)
{
	pIniFile->WriteString("RADIO", "PortName", RADIO.StrPort);
	pIniFile->WriteInteger("RADIO", "BaudRate", RADIO.BaudRate);
	pIniFile->WriteInteger("RADIO", "BitLen", RADIO.BitLen);
	pIniFile->WriteInteger("RADIO", "Stop", RADIO.Stop);
	pIniFile->WriteInteger("RADIO", "Parity", RADIO.Parity);
	pIniFile->WriteInteger("RADIO", "flwXON", RADIO.flwXON);
	pIniFile->WriteInteger("RADIO", "flwCTS", RADIO.flwCTS);
	pIniFile->WriteInteger("RADIO", "usePTT", RADIO.usePTT);

	pIniFile->WriteInteger("RADIO", "ByteWait", RADIO.ByteWait);

	pIniFile->WriteInteger("RADIO", "Cmdxx", RADIO.Cmdxx);
	pIniFile->WriteString("RADIO", "CmdInit", RADIO.CmdInit);
	pIniFile->WriteString("RADIO", "CmdRx", RADIO.CmdRx);
	pIniFile->WriteString("RADIO", "CmdTx", RADIO.CmdTx);

	pIniFile->WriteString("RADIO", "FileGNR", RADIO.cmdGNR);
	pIniFile->WriteInteger("RADIO", "OpenGNR", RADIO.openGNR);

	pIniFile->WriteInteger("RADIO", "PollType", RADIO.PollType);
	pIniFile->WriteInteger("RADIO", "PollInterval", RADIO.PollInterval);
}
//---------------------------------------------------------------------------
//   注意: VCL オブジェクトのメソッドとプロパティを使用するには, Synchronize
//         を使ったメソッド呼び出しでなければなりません。次に例を示します。
//
//      Synchronize(UpdateCaption);
//
//   ここで, UpdateCaption は次のように記述できます。
//
//      void __fastcall CCradio::UpdateCaption()
//      {
//        Form1->Caption = "スレッドから書き換えました";
//      }
//---------------------------------------------------------------------------
__fastcall CCradio::CCradio(bool CreateSuspended)
	: TThread(CreateSuspended)
{
	m_CreateON = FALSE;	// クリエイトフラグ
	m_fHnd = NULL;		// ファイルハンドル
	m_wHnd = NULL;		// 親のウインドウハンドル
	m_uMsg = WM_USER;
	m_ID = 0;			// メッセージのＩＤ番号
	m_Command = 0;		// スレッドへのコマンド
	m_TxAbort = 0;		// 送信中止フラグ
	m_txwp = m_txrp = m_txcnt = 0;
	m_PSKGNRId = 0;
	m_OpenGNR = 0;

	m_PollCnt = 0;
	m_rxcnt = 0;
	m_FreqEvent = 0;
	m_Freq[0] = 0;

	m_ScanAddr = 0;
	m_pRadio = NULL;
}

//---------------------------------------------------------------------------
void __fastcall CCradio::Execute()
{
	//---- スレッドのコードをここに記述 ----
//	Priority = tpLower;
	while(1){
		if( Terminated == TRUE ){
			return;
		}
		if( m_Command == CRADIO_CLOSE ){
			m_Command = 0;
			return;
		}
		if( m_CreateON == TRUE ){
			if( m_txcnt ){
				if( m_pRadio != NULL ){
					if( !(m_pRadio->GetStatus() & mmrpstatusTXBUSY) ){
						m_pRadio->PutChar(m_txbuf[m_txrp]);
						m_txrp++;
						if( m_txrp >= RADIO_TXBUFSIZE ){
							m_txrp = 0;
						}
						m_txcnt--;
					}
				}
				else if( !TxBusy() ){
					DWORD	size=0;
					::WriteFile( m_fHnd, &m_txbuf[m_txrp], 1, &size, NULL );
					if( size ){
						m_txrp++;
						if( m_txrp >= RADIO_TXBUFSIZE ){
							m_txrp = 0;
						}
						m_txcnt--;
					}
					if( RADIO.ByteWait ) ::Sleep(RADIO.ByteWait);
				}
				::Sleep(1);
			}
			else if( m_pRadio != NULL ){
				if( m_pRadio->GetStatus() & mmrpstatusFREQ ){
					long fq = m_pRadio->GetFreq();
					if( fq ) UpdateFreq(double(fq)/1e4);
				}
				while(m_pRadio->GetStatus() & mmrpstatusRX){
					CatchPoll(m_pRadio->GetChar());
				}
				::Sleep(10);
			}
			else {
				BYTE dmy[256];
				while(1){
					int len = RecvLen();
					if( !len ) break;
					if( len >= (int)sizeof(dmy) ) len = (int)sizeof(dmy);
					Read(dmy, len);
					if( RADIO.PollType ){
						BYTE *p = dmy;
						for( ; len; p++, len-- ){
							CatchPoll(*p);
						}
					}
				}
				::Sleep(10);
			}
		}
		else {
			::Sleep(10);
		}
	}
}
//---------------------------------------------------------------------------
/*#$%
==============================================================
	通信回線をオープンしスレッドをアクティブにする
--------------------------------------------------------------
PortName : 回線の名前
pCP		 : COMMPARAのポインタ（ヌルの時はデフォルトで初期化）
pWnd     : メッセージ送信先のウインドウクラスのポインタ（ヌルの時はﾒｲﾝﾌﾚｰﾑｳｲﾝﾄﾞｳ）
nID		 : データ受信時のメッセージＩＤ
RBufSize : 受信バッファのサイズ(default=2048)
TBufSize : 送信バッファのサイズ(default=2048)
--------------------------------------------------------------
TRUE/FALSE
--------------------------------------------------------------
==============================================================
*/
BOOL CCradio::Open(CRADIOPARA *cp, HWND hwnd, UINT uMsg, UINT nID)
{
	if( m_CreateON == TRUE ) Close();
	m_TxAbort = FALSE;
	m_PSKGNRId = 0;
	m_OpenGNR = 0;
	if( !strcmpi(cp->StrPort, "PSKGNR") || !strcmpi(cp->StrPort, "WD5GNR") || !strcmpi(cp->StrPort, "LOGGER")){
		m_PSKGNRId = ::RegisterWindowMessage("PSKGNRFUNC");
		m_CreateON = TRUE;
		if( RADIO.openGNR && (!RADIO.cmdGNR.IsEmpty()) && (strcmpi(cp->StrPort, "LOGGER")) ){
			if( FindWindow("ThunderRT6Main", NULL) == NULL ){
				::WinExec(RADIO.cmdGNR.c_str(), SW_HIDE);
				m_OpenGNR = 1;
			}
		}
		return m_CreateON;
	}
	m_fHnd = ::CreateFile(cp->StrPort, GENERIC_READ | GENERIC_WRITE,
						0, NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL
	);
	if( m_fHnd == INVALID_HANDLE_VALUE ){
		AnsiString as = "\\\\.\\";
        as += cp->StrPort;
		m_fHnd = ::CreateFile(as.c_str(), GENERIC_READ | GENERIC_WRITE,
							0, NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL
		);
    }
	m_wHnd = hwnd;
	m_uMsg = uMsg;
	m_ID = nID;
	if( m_fHnd == INVALID_HANDLE_VALUE ) goto _mmr;
	// setup device buffers
	if( ::SetupComm( m_fHnd, DWORD(RADIO_COMBUFSIZE), DWORD(RADIO_COMBUFSIZE) ) == FALSE ){
		::CloseHandle(m_fHnd);
_mmr:;
		m_pRadio = new CMMRadio(hwnd, uMsg);
		if( m_pRadio->Open(cp->StrPort) ){
			m_CreateON = TRUE;
			Priority = tpLower;
			Resume();			// スレッドの実行
			return TRUE;
		}
		else {
			delete m_pRadio;
			m_pRadio = NULL;
			return FALSE;
		}
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
	m_dcb.BaudRate = cp->BaudRate;
	m_dcb.fBinary = TRUE;
	m_dcb.ByteSize = USHORT(cp->BitLen ? 8 : 7);
	const UCHAR _tp[]={NOPARITY, EVENPARITY, ODDPARITY};
	m_dcb.Parity = _tp[cp->Parity];
	const UCHAR _ts[]={ONESTOPBIT,TWOSTOPBITS};
	m_dcb.StopBits = _ts[cp->Stop];
	if( cp->usePTT ){
		m_dcb.fRtsControl = RTS_CONTROL_DISABLE;		// 送信禁止
		m_dcb.fDtrControl = DTR_CONTROL_DISABLE;		// 送信禁止
	}
	else {
		m_dcb.fRtsControl = RTS_CONTROL_ENABLE;
	}
	m_dcb.fOutxCtsFlow = (cp->flwCTS && !cp->usePTT) ? TRUE : FALSE;
	m_dcb.fInX = m_dcb.fOutX = cp->flwXON ? TRUE : FALSE;
	m_dcb.XonChar = 0x11;
	m_dcb.XoffChar = 0x13;
	m_dcb.fParity = FALSE;
	m_dcb.EvtChar = 0x0d;	// dummy setting
//	m_dcb.fTXContinueOnXoff = TRUE;
	m_dcb.XonLim = USHORT(RADIO_COMBUFSIZE/4);		// 1/4 of RBufSize
	m_dcb.XoffLim = USHORT(RADIO_COMBUFSIZE*3/4);		// 3/4 of RBufSize
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
	Priority = tpLower;
	Resume();			// スレッドの実行
	return TRUE;
}
/*#$%
==============================================================
	通信回線をクローズする
--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
	スレッドが終了するまで待つ
==============================================================
*/
void CCradio::Close(void)
{
	if( m_CreateON == TRUE ){
		if( m_PSKGNRId ){
			if( (m_OpenGNR || RADIO.change) && RADIO.openGNR ) ::SendMessage(HWND_BROADCAST, m_PSKGNRId, 1 , 0);
			m_PSKGNRId = 0;
		}
		else {
			if( m_ID ){
				m_Command = CRADIO_CLOSE;	// スレッド終了コマンド
				Priority = tpNormal;		//スレッドは通常の優先度である
				WaitFor();
			}
			if( m_pRadio != NULL ){
				delete m_pRadio;
				m_pRadio = NULL;
			}
			else {
				::CloseHandle(m_fHnd);
			}
		}
	}
	m_CreateON = FALSE;
	m_TxAbort = TRUE;
}
void CCradio::ReqClose(void)
{
	if( m_CreateON == TRUE ){
		if( m_PSKGNRId ){
			return;
		}
		else {
			if( m_ID ){
				m_Command = CRADIO_CLOSE;	// スレッド終了コマンド
				Priority = tpNormal;		//スレッドは通常の優先度である
			}
		}
	}
}
void CCradio::WaitClose(void)
{
	if( m_CreateON == TRUE ){
		if( m_PSKGNRId ){
			if( (m_OpenGNR || RADIO.change) && RADIO.openGNR ) ::SendMessage(HWND_BROADCAST, m_PSKGNRId, 1 , 0);
			m_PSKGNRId = 0;
		}
		else {
			if( m_ID && m_Command ){
				WaitFor();
			}
			if( m_pRadio != NULL ){
				delete m_pRadio;
				m_pRadio = NULL;
			}
			else {
				::CloseHandle(m_fHnd);
			}
		}
		m_CreateON = FALSE;
	}
	m_TxAbort = TRUE;
}
/*#$%
==============================================================
	受信バッファ内の格納データ長を得る
--------------------------------------------------------------
--------------------------------------------------------------
	データの長さ
--------------------------------------------------------------
==============================================================
*/
DWORD CCradio::RecvLen(void)
{
	COMSTAT ComStat;
	DWORD	dwErrorFlags;

	::ClearCommError( m_fHnd, &dwErrorFlags, &ComStat );
	return ComStat.cbInQue;
}

/*#$%
==============================================================
	送信ビジーかどうか調べる
--------------------------------------------------------------
--------------------------------------------------------------
 : 送信ビジー状態
--------------------------------------------------------------
==============================================================
*/
int CCradio::TxBusy(void)
{
#if 0
	COMSTAT ComStat;
	DWORD	dwErrorFlags;

	if( m_TxAbort ) return FALSE;
	ClearCommError( m_fHnd, &dwErrorFlags, &ComStat );
	int f = ComStat.fRlsdHold;
	if( f ) return TRUE;
	if( m_dcb.fOutxCtsFlow ){
		f |= ComStat.fCtsHold;
	}
	if( m_dcb.fOutX ){
		f |= ComStat.fXoffHold;
	}
	return f ? TRUE : FALSE;
#else
	COMSTAT ComStat;
	DWORD	dwErrorFlags;

	::ClearCommError( m_fHnd, &dwErrorFlags, &ComStat );
	return ComStat.cbOutQue;
#endif
}

/*#$%
==============================================================
	通信回線からデータを取り出す
--------------------------------------------------------------
p	: バッファのポインタ
len : バッファのサイズ
--------------------------------------------------------------
実際に受信したサイズ
--------------------------------------------------------------
==============================================================
*/
DWORD CCradio::Read(BYTE *p, DWORD len)
{
	DWORD	size=0;

	if( m_CreateON == TRUE ){
		::ReadFile( m_fHnd, p, len, &size, NULL );
	}
	return size;
}

/*#$%
==============================================================
	通信回線にデータを送信する
--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
==============================================================
*/
void CCradio::PutChar(char c)
{
	if( m_CreateON == TRUE ){
		if( m_PSKGNRId ) return;
		if( m_txcnt < RADIO_TXBUFSIZE ){
			m_txbuf[m_txwp] = c;
			m_txwp++;
			if( m_txwp >= RADIO_TXBUFSIZE ) m_txwp = 0;
			m_txcnt++;
		}
	}
}

/*#$%
==============================================================
	通信回線にデータを送信する
--------------------------------------------------------------
p	: バッファのポインタ
len : 送信するサイズ
--------------------------------------------------------------
実際に送信したサイズ
--------------------------------------------------------------
==============================================================
*/
void CCradio::Write(void *s, DWORD len)
{
	if( m_CreateON == TRUE ){
		if( m_PSKGNRId ) return;
		char	*p;
		for( p = (char *)s; len; len--, p++ ){
			PutChar(*p);
		}
	}
}

/*#$%
==============================================================
	通信回線にデータを送信する
--------------------------------------------------------------
p	: バッファのポインタ
len : 送信するサイズ
--------------------------------------------------------------
実際に送信したサイズ
--------------------------------------------------------------
==============================================================
*/
void CCradio::OutStr(LPCSTR fmt, ...)
{
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);
	Write(bf, strlen(bf));
}

void CCradio::OutLine(LPCSTR fmt, ...)
{
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);
	Write(bf, strlen(bf));
	strcat( bf, "\r" );	//ja7ude 0521
	Write(bf, 1);     //ja7ude 0521
}

void CCradio::SendCommand(LPCSTR p)
{
	int	c;
	int f;

	for(f = 0; *p; p++){
		if( *p == '\\' ){
			f = 0;
			p++;
			switch(*p){
				case '$':
					f = 1;
					continue;
				case 'x':
				case 'X':
					p++;
					if( *p == 'x' ){
						c = RADIO.Cmdxx;
					}
					else {
						c = htoin(p, 2);
					}
					p++;
					break;
				case 'r':
					c = CR;
					break;
				case 'n':
					c = LF;
					break;
				case 'w':
					p++;
					c = atoin(p, 2);
					if( (c < 0) || (c >= 100) ) c = 100;
					if( c ) ::Sleep(c * 10);
					p++;
					continue;
				case '\\':
					c = '\\';
					break;
				case 'c':	// comment
					return;
			}
		}
		else if( f ){
			p = SkipSpace(p);
			if( *p == 'x' ){
				c = RADIO.Cmdxx;
			}
			else {
				c = htoin(p, 2);
			}
			p++;
		}
		else {
			c = *p;
		}
		PutChar(BYTE(c));
	}
}

void CCradio::SetPTT(int sw)
{
	if( m_PSKGNRId ){
		::SendMessage(HWND_BROADCAST, m_PSKGNRId, 0 , sw ? 1 : 0);
		if( sw ) ::Sleep(50);
	}
	else {
		if( sw ){
			if( m_pRadio != NULL ){
				m_pRadio->SetPTT(sw);
			}
			else if( RADIO.usePTT ){
				::EscapeCommFunction(m_fHnd, SETRTS);
				::EscapeCommFunction(m_fHnd, SETDTR);
			}
			SendCommand(RADIO.CmdTx.c_str());
		}
		else {
			if( m_pRadio != NULL ){
				m_pRadio->SetPTT(sw);
			}
			else if( RADIO.usePTT ){
				::EscapeCommFunction(m_fHnd, CLRRTS);
				::EscapeCommFunction(m_fHnd, CLRDTR);
			}
			SendCommand(RADIO.CmdRx.c_str());
		}
	}
}

//--------------------------------------------------------
// 周波数が変化しているかどうか調べる
int CCradio::IsFreqChange(LPCSTR pFreq)
{
	if( RADIO.PollType ){
		if( m_FreqEvent ) return 1;
		if( m_Freq[0] ){
			if( strcmp(m_Freq, pFreq) ) return 1;
		}
	}
	return 0;
}

//--------------------------------------------------------
// タイマー処理
void CCradio::Timer(int tx, int interval)
{
	if( m_CreateON == TRUE ){
		if( m_PSKGNRId ) return;
		if( m_pRadio != NULL ){
			if( m_pRadio->GetStatus() & mmrpstatusDEFCMD ){
				LPCSTR p;
				switch(m_pRadio->GetDefCommand()){
                    case 1:
						p = RADIO.CmdTx.c_str();
                        break;
                    case 2:
						p = RADIO.CmdRx.c_str();
                        break;
					default:
						p = RADIO.CmdInit.c_str();
                        break;
                }
				SendCommand(p);
				m_PollCnt = (5 + RADIO.PollInterval);
            }
        }
		if( (!tx) && RADIO.PollType ){
			if( !m_PollCnt ){
				if( m_pRadio != NULL ) m_pRadio->Polling();
				if( m_ScanAddr ){		// アドレススキャン
					m_PollCnt = 4;
					if( m_ScanAddr <= 3 ){
						m_ScanAddr++;
					}
					else {
						RADIO.Cmdxx++;
						if( RADIO.Cmdxx >= 0x80 ){
							RADIO.Cmdxx = 0;
						}
					}
				}
				else {
					m_PollCnt = (5 + RADIO.PollInterval);
				}
				if( interval ){
					m_PollCnt = m_PollCnt * 100 / interval;
				}
				switch(RADIO.PollType){
					case RADIO_POLLYAESUHF:
					case RADIO_POLLFT1000D:
					case RADIO_POLLFT920:
						m_rxcnt = 0;
						SendCommand("\\$0000000210");
						break;
					case RADIO_POLLYAESUVU:
						m_rxcnt = 0;
						SendCommand("\\$0000000003");
						break;
					case RADIO_POLLICOM:
					case RADIO_POLLOMNIVI:
						m_rxcnt = 0;
						SendCommand("\\$FEFExxE003FD");
						break;
					case RADIO_POLLICOMN:
					case RADIO_POLLOMNIVIN:
						if( !m_Freq[0] || m_ScanAddr ){
							m_rxcnt = 0;
							SendCommand("\\$FEFExxE003FD");
						}
						break;
					case RADIO_POLLKENWOOD:
						m_rxcnt = 0;
						SendCommand("IF;");
						break;
					case RADIO_POLLKENWOODN:
						if( !m_Freq[0] ){
							m_rxcnt = 0;
							SendCommand("AI1;");
						}
						break;
					case RADIO_POLLJST245:
						m_rxcnt = 0;
						SendCommand("I\r\n");
						break;
					case RADIO_POLLJST245N:
						if( !m_Freq[0] ){
							m_rxcnt = 0;
							SendCommand("I1\r\nL\r\n");
						}
						break;
                    //1.66B AA6YQ
                    case RADIO_POLLFT9000:
                    case RADIO_POLLFT2000:
                    case RADIO_POLLFT950:
 					case RADIO_POLLFT450:
						m_rxcnt = 0;
						SendCommand("IF;");
						break;
					default:
						break;
				}
			}
			m_PollCnt--;
		}
	}
}

//--------------------------------------------------------
// 周波数ポーリングデータの受信
void CCradio::CatchPoll(BYTE c)
{
	switch(RADIO.PollType){
		case RADIO_POLLYAESUHF:
		case RADIO_POLLFT1000D:
		case RADIO_POLLFT920:
			if( m_rxcnt < 5 ){
				m_rxbuf[m_rxcnt] = c;
				m_rxcnt++;
				if( m_rxcnt == 5 ){
					FreqYaesuHF();
				}
			}
			break;
		case RADIO_POLLYAESUVU:
			if( m_rxcnt < 5  ){
				m_rxbuf[m_rxcnt] = c;
				m_rxcnt++;
				if( m_rxcnt == 5 ){
					FreqYaesuVU();
				}
			}
			break;
		case RADIO_POLLICOM:
		case RADIO_POLLICOMN:
		case RADIO_POLLOMNIVI:
		case RADIO_POLLOMNIVIN:
			switch(m_rxcnt){
				case 0:
					if( c == 0xfe ){
						m_rxbuf[m_rxcnt] = c;		// プリアンブル
						m_rxcnt++;
					}
					break;
				case 1:
					if( c != 0xfe ){
						m_rxbuf[m_rxcnt] = c;		// PC-Addr
						m_rxcnt++;
					}
					break;
				case 2:
					if( (c == RADIO.Cmdxx) ||
						(m_ScanAddr && c) ){
						m_rxbuf[m_rxcnt] = c;		// Radio-Addr
						m_rxcnt++;
					}
					else {
						m_rxcnt = 0;
					}
					break;
				case 3:
					if( (c == 0x03) || ((c == 0x00)&&((RADIO.PollType == RADIO_POLLICOMN)||(RADIO.PollType == RADIO_POLLOMNIVIN))) ){
						m_rxbuf[m_rxcnt] = c;		// Respons-command
						m_rxcnt++;
					}
					else {
						m_rxcnt = 0;
					}
					break;
				default:
					if( m_rxcnt < (int)sizeof(m_rxbuf) ){
						m_rxbuf[m_rxcnt] = c;		// Data
						m_rxcnt++;
						if( c == 0xfd ){
							if( m_rxcnt >= 9 ){
								FreqICOM();
								if( m_ScanAddr && m_rxbuf[2] ){
									RADIO.Cmdxx = m_rxbuf[2];
									RADIO.PollScan = 0;
									m_ScanAddr = 0;
								}
							}
							m_rxcnt = 0;
						}
					}
					else {
						m_rxcnt = 0;
					}
					break;
			}
			break;
		case RADIO_POLLKENWOOD:
		case RADIO_POLLKENWOODN:
			if( m_rxcnt < (int)sizeof(m_rxbuf) ){
				if( (c != 0x0d) && (c != 0x0f) ){
					if( (c != ' ') || m_rxcnt ){
						m_rxbuf[m_rxcnt] = c;		// Data
						m_rxcnt++;
						if( c == ';' ){
							if( (m_rxbuf[0] == 'I') && (m_rxbuf[1]=='F') ){
								if( m_rxcnt >= 13 ) FreqKenwood();
							}
							m_rxcnt = 0;
						}
					}
				}
			}
			else {
				m_rxcnt = 0;
			}
			break;
		case RADIO_POLLJST245:
		case RADIO_POLLJST245N:
			if( m_rxcnt < (int)sizeof(m_rxbuf) ){
				if( (c == 'I') || m_rxcnt ){
					m_rxbuf[m_rxcnt] = c;		// Data
					m_rxcnt++;
					if( (c == 0x0d) || (c == 0x0f) ){
						if( m_rxcnt >= 12 ) FreqJST245();
						m_rxcnt = 0;
					}
				}
			}
			else {
				m_rxcnt = 0;
			}
			break;
        //1.66B AA6YQ
		case RADIO_POLLFT9000:
		case RADIO_POLLFT2000:
        case RADIO_POLLFT950:
        case RADIO_POLLFT450:
			if( m_rxcnt < (int)sizeof(m_rxbuf) ){
				if( (c != 0x0d) && (c != 0x0f) ){
					if( (c != ' ') || m_rxcnt ){
						m_rxbuf[m_rxcnt] = c;		// Data
						m_rxcnt++;
						if( c == ';' ){
							if( (m_rxbuf[0] == 'I') && (m_rxbuf[1]=='F') ){
								if( m_rxcnt >= 13 ) FreqYaesu9K2K();
							}
							m_rxcnt = 0;
						}
					}
				}
			}
			else {
				m_rxcnt = 0;
			}
			break;
		default:
			break;
	}
}
//----------------------------------------------------------
// 周波数の更新 (freq=MHz)
//
void CCradio::UpdateFreq(double freq)
{
	if( freq < 0.001 ) return;

	char bf[32];
	sprintf(bf, "%.3lf", freq);
	if( strcmp(m_Freq, bf) ){
		strcpy(m_Freq, bf);
		m_FreqEvent = 1;
	}
}

void CCradio::FreqYaesuHF(void)
{
	ULONG	fq;
	fq = m_rxbuf[1];
	fq = fq << 8;
	fq |= m_rxbuf[2];
	fq = fq << 8;
	fq |= m_rxbuf[3];
	fq = fq << 8;
	fq |= m_rxbuf[4];

	double f;
	switch(RADIO.PollType){
		case RADIO_POLLFT1000D:         // FT1000D
			f = 25600000.0;
			break;
		case RADIO_POLLFT920:           // FT920
			f = 1000000.0;
			break;
		default:                        // FT1000MP
			f = 1600000.0;
			break;
	}

	UpdateFreq(double(fq)/f);
}

void CCradio::FreqYaesuVU(void)
{
	ULONG fq;
	fq = m_rxbuf[0] >> 4;
	fq *= 10;
	fq += m_rxbuf[0] & 0x0f;
	fq *= 10;
	fq += m_rxbuf[1] >> 4;
	fq *= 10;
	fq += m_rxbuf[1] & 0x0f;
	fq *= 10;
	fq += m_rxbuf[2] >> 4;
	fq *= 10;
	fq += m_rxbuf[2] & 0x0f;

	UpdateFreq( double(fq) / 1000.0 );
}

void CCradio::FreqICOM(void)
{
// 0  1  2  3  4  5  6  7  8  9
// fe e0 40 03 90 09 02 07 00 fd
// fe e0 40 03 90 09 02 07 fd

	ULONG fq = 0;
	if( m_rxbuf[8] != 0xfd ){
		fq = (m_rxbuf[8] >> 4);
		fq *= 10;
		fq += (m_rxbuf[8] & 0x0f);
	}
	fq *= 10;
	fq += m_rxbuf[7] >> 4;
	fq *= 10;
	fq += m_rxbuf[7] & 0x0f;
	fq *= 10;
	fq += m_rxbuf[6] >> 4;
	fq *= 10;
	fq += m_rxbuf[6] & 0x0f;
	fq *= 10;
	fq += m_rxbuf[5] >> 4;
	fq *= 10;
	fq += m_rxbuf[5] & 0x0f;
	fq *= 10;
	fq += m_rxbuf[4] >> 4;
	fq *= 10;
	fq += m_rxbuf[4] & 0x0f;

	UpdateFreq(double(fq)/1e6);
}

void CCradio::FreqKenwood(void)
{
//0123456789012
//IF00021155000     +001000 0002000008 ;
//abcdefghijklmnopqrstuvwxyz1234567890      <----　桁位置
//c - m  周波数　21.155MHz

	ULONG fq = 0;

	m_rxbuf[13] = 0;
	if( sscanf((LPCSTR)&m_rxbuf[2], "%lu", &fq) == 1 ){
		if( fq ) UpdateFreq(double(fq)/1e6);
	}
}

void CCradio::FreqJST245(void)
{
//     0 1 2 3 4 5 6 7 8 9 10 11 12
//　”Ｉａｂｄｆｆｆｆｆｆｆｆｇ”
//　　Ｉ：ヘッダー記号
//　　ａ：使用アンテナ１桁（１～３）
//　　ｂ：バンド幅１桁（０～２）
//　　ｄ：モード１桁（０～５）
//　　ｆ：送受信周波数８桁（００１０００００～５３９９９９９８）
//　　ｇ：ＡＧＣ１桁（０～２）

	ULONG fq = 0;

	m_rxbuf[12] = 0;
	LPCSTR p = (LPCSTR)&m_rxbuf[4];
	for( ; *p; p++ ) if( !isdigit(*p) ) return;
	if( sscanf((LPCSTR)&m_rxbuf[4], "%lu", &fq) == 1 ){
		if( fq ) UpdateFreq(double(fq)/1e6);
	}
}

//AA6YQ 1.66B
void CCradio::FreqYaesu9K2K(void)
{
//0123456789012
//IF00021155000     +001000 0002000008 ;
//abcdefghijklmnopqrstuvwxyz1234567890      <----　桁位置
//f - m  周波数　21.155MHz

	ULONG fq = 0;

	m_rxbuf[13] = 0;
	if( sscanf((LPCSTR)&m_rxbuf[5], "%lu", &fq) == 1 ){
		if( fq ) UpdateFreq(double(fq)/1e6);
	}
}

