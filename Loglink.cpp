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

#define MMSSTV  1       // MMTTYでは0
#if MMSSTV
#include "Main.h"
#endif

#include "LogLink.h"
CLogLink    LogLink;
//---------------------------------------------------------------------------
// CLogLinkクラス
CLogLink::CLogLink()
{
	m_Enabled = FALSE;
	m_Count = m_Count2 = 0;
	m_hLog = NULL;
	m_hLogIn = NULL;
	m_CStat = 0;
	m_hApp = NULL;
	m_hMain = NULL;
	m_Error = 0;
	m_1stSession = TRUE;
	m_fHLV5 = TRUE;

	m_Polling = 1;
    m_PTTEnabled = FALSE;

	m_AStat = FALSE;
	m_MMStat = FALSE;
	m_pLink = NULL;
    m_strMode[0] = 0;
}

CLogLink::~CLogLink()
{
	CloseMMLink();
}
//---------------------------------------------------------------------------
void __fastcall CLogLink::CloseMMLink(void)
{
	if( m_pLink != NULL ){
		delete m_pLink;
		m_pLink = NULL;
	}
}
//---------------------------------------------------------------------------
// 送信用ウインドウハンドルの設定
void CLogLink::SetHandle(HWND hMain, UINT uMsg)
{
	m_hMain = hMain;
	m_uMsg = uMsg;
	m_hApp = Application->Handle;
	if( m_pLink == NULL ) return;
	m_pLink->SetHandle(hMain, uMsg);
}
//---------------------------------------------------------------------------
BOOL __fastcall CLogLink::IsLink(void)
{
	switch(m_Enabled){
		case 1:
			return m_hLog != NULL ? TRUE : FALSE;
		case 2:
			return m_MMStat;
		default:
			return FALSE;
	}
}
//---------------------------------------------------------------------------
LPCSTR __fastcall CLogLink::GetSessionName(void)
{
	switch(m_Enabled){
		case 1:
//			return "Turbo HAMLOG/Win";
			return m_AppName.c_str();
		case 2:
			if( m_pLink == NULL ) return NULL;
			return m_pLink->GetSessionName();
		default:
			return NULL;
	}
}
//---------------------------------------------------------------------------
// リンクの許可設定
void CLogLink::UpdateLink(int sw)
{
	if( (m_Enabled != sw) ||
		((m_pLink != NULL) && strcmp(m_ItemName.c_str(), m_pLink->GetItemName()) )
	){
		m_Enabled = sw;
		switch(sw){
			case 0:
				m_hLog = NULL;
				m_hLogIn = NULL;
				CloseMMLink();
				break;
			case 1:
				m_Count = 0;
				m_Error = 0;
				CloseMMLink();
				break;
			case 2:
				m_Count = 0;
				m_Error = 0;
				if( !m_pLink ){
					m_pLink = new CMMLink(m_hMain, m_uMsg);
				}
				m_pLink->Open(m_ItemName.c_str());
				break;
		}
	}
}
//---------------------------------------------------------------------------
// コールサインを分離
static void DevCall(AnsiString &CALL, AnsiString &PTB, LPCSTR pCall)
{
	char bf[1024];

	CALL = "";
	PTB = "";
	strcpy(bf, pCall);
	LPSTR	pp, p2, t;
	t = bf;
	if( (pp = strchr(bf, '/')) != NULL ){		// KH6/JE3HHT or JE3HHT/KH6 の形式
		*pp = 0;
		pp++;
		int LenC = strlen(t);
		int LenP = strlen(pp);
		if( ((p2 = strchr(pp, '/')) != NULL)||
			(LenC > 7) ||
			(LenP > 7) ||
			((LenC > LenP) && (LenP>3)) ||
			((LenC < LenP) && (LenC>3))
		){
			// HAMLOGでは表現できない表記の場合
			if( p2 != NULL ){
				*p2 = 0;
				LenP = strlen(pp);
			}
		}
		if( LenC < LenP ){
			p2 = t;
			t = pp;
			pp = p2;
		}
		PTB = pp;
	}
	CALL = t;
}
//---------------------------------------------------------------------------
static void __fastcall SetHamlogMode(LPSTR bf)
{
	if( !strcmp(bf, "RTTY") ){
		StrCopy(bf, Log.m_LogSet.m_THRTTY.c_str(), 3);
	}
	else if( !strcmp(bf, "SSTV") ){
		StrCopy(bf, Log.m_LogSet.m_THSSTV.c_str(), 3);
	}
	else if( !strcmp(bf, "GMSK") ){
		strcpy(bf, "MSK");
	}
	else if( !strcmp(bf, "FSTV") ){
		strcpy(bf, "FTV");
	}
	else if( !strcmp(bf, "BPSK") ){
		strcpy(bf, "PSK");
	}
	else if( !strcmp(bf, "QPSK") ){
		strcpy(bf, "PSK");
	}
}
//---------------------------------------------------------------------------
// MM -> HamLog フォーマットの一括変換
static void MMtoHAMLOG(LPSTR tp, SDMMLOG *sp, BOOL fHLV5)
{
	char bf[512];

	AnsiString  DATE;
	AnsiString  TIME;

	int CallOrder = FALSE;

	if( sp->btime ){
		int YY = sp->year;
		int MM = sp->date / 100;
		int DD = sp->date % 100;
		int tim = sp->btime / 30;
		int HH = tim / 60;
		int mm = tim % 60;
		char tz = 'J';
		switch(Log.m_LogSet.m_THTZ){
			case 0:
				if( !IsJA(sp->call) ){
					JSTtoUTC(YY, MM, DD, HH);
					tz = 'U';
				}
				break;
			case 1:
				JSTtoUTC(YY, MM, DD, HH);
				tz = 'U';
				break;
			default:
				break;
		}
		sprintf(bf, "%02u/%02u/%02u", YY, MM, DD);
		DATE = bf;
		sprintf(bf, "%02u:%02u%c", HH, mm, tz);
		TIME = bf;
	}

	AnsiString  NAME = sp->name;
	AnsiString	QTH = sp->qth;
	AnsiString	REM1 = sp->rem;
	AnsiString	REM2 = sp->qsl;

	AnsiString  CALL;
	AnsiString  PTB;

	if( fHLV5 ){					// Ver 5.00以降
        CALL = sp->call;
    }
    else {							// Ver 4.xx
		strcpy(bf, sp->call);
		LPSTR	pp, p2, t;
		t = bf;
		if( (pp = strchr(bf, '/')) != NULL ){		// KH6/JE3HHT or JE3HHT/KH6 の形式
			*pp = 0;
			pp++;
			int LenC = strlen(t);
			int LenP = strlen(pp);
			if( ((p2 = strchr(pp, '/')) != NULL)||
				(LenC > 7) ||
				(LenP > 7) ||
				((LenC > LenP) && (LenP>3)) ||
				((LenC < LenP) && (LenC>3))
			){
				// HAMLOGでは表現できない表記の場合
				AddMMLOGKey(REM1, REM2, sp->call, "ToRadio");
				if( p2 != NULL ){
					*p2 = 0;
					LenP = strlen(pp);
				}
			}
			if( LenC < LenP ){
				p2 = t;
				t = pp;
				pp = p2;
				CallOrder = TRUE;
			}
			PTB = pp;
		}
		else if( strlen(t) > 7 ){
			AddMMLOGKey(REM1, REM2, sp->call, "ToRadio");
		}
		CALL = t;
    }
	strcpy(bf, Log.GetModeString(sp->mode));
	if( !fHLV5 ){
		SetHamlogMode(bf);
    }
	AnsiString MODE = bf;

	AnsiString HIS, MY;
	int l = GetLMode(sp->mode);
	if( fHLV5 ){
		HIS = sp->ur;
        MY = sp->my;
	}
    else {
		strcpy(bf, sp->ur);
		bf[l] = 0;
		HIS = bf;
		strcpy(bf, sp->my);
		bf[l] = 0;
		MY = bf;
    }
	if( sp->ur[l] ) AddMMLOGKey(REM1, REM2, &sp->ur[l], "SN");
	if( sp->my[l] ) AddMMLOGKey(REM1, REM2, &sp->my[l], "RN");

	strcpy(bf, Log.GetFreqString(sp->band, sp->fq));
	AnsiString FREQ = bf;

	if( !fHLV5 ){
		if( CallOrder && (!strstr(sp->rem, "$DX")) && (!strstr(sp->qsl, "$DX")) ){
			AddMMLOGKey(REM1, REM2, "$DX", NULL);
		}
    }

	if( sp->etime ){
		int tim = sp->etime / 30;
		sprintf(bf, "%02u%02u", tim / 60, tim % 60);
		AddMMLOGKey(REM1, REM2, bf, "END");
	}
	if( sp->env ){
		sprintf(bf, "%u", sp->env);
		AddMMLOGKey(REM1, REM2, bf, "ENV");
	}
	AddMMLOGKey(REM1, REM2, sp->pow, "POW");
	if( sp->cq ){
		bf[0] = sp->cq; bf[1] = 0;
		AddMMLOGKey(REM1, REM2, bf, "M");
	}
	AnsiString CODE;
	if( (strlen(sp->opt1) >= 3) && isdigit(sp->opt1[0]) && isdigit(sp->opt1[1]) ){
		CODE = sp->opt1;
	}
	sprintf(bf, "%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
		CALL.c_str(), PTB.c_str(), DATE.c_str(), TIME.c_str(),
		HIS.c_str(), MY.c_str(), FREQ.c_str(), MODE.c_str(), CODE.c_str(), "", "",
		NAME.c_str(), QTH.c_str(), REM1.c_str(), REM2.c_str()
	);
	strcpy(tp, bf);
}

//---------------------------------------------------------------------------
// ウインドウフォーカスを自身に戻す
void CLogLink::Foreground(void)
{
	if( m_Enabled != 1 ) return;

	::SetWindowPos(m_hApp, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	::SetFocus(m_hApp);
}
//---------------------------------------------------------------------------
// ログのリンクのチェック
BOOL CLogLink::CheckLink(void)
{
	if( m_Error ) return FALSE;

	switch(m_Enabled){
		case 1:
		{
			HWND hLog = ::FindWindow("TThwin", NULL);
			if( hLog != m_hLog ){
				if( hLog ){
                	m_AppVer = 0;
					char bf[128];
                    LPSTR p;
					::GetWindowText(hLog, bf, sizeof(bf));
                    clipsp(bf);
                    m_AppName = bf;
					if( (p = strstr(bf, "Ver")) != NULL ){
						p += 3;
                        LPSTR t;
                        for( t = p; *t; t++ ){
							if( (*t != '.') && !isdigit(*t) ) break;
                        }
                        *t = 0;
                        double d;
                        if( sscanf(p, "%lf", &d) == 1 ){
	                        m_AppVer = d * 100;
                        }
                    }
                    else {
						m_Count = 1000/LINKINTERVAL;
                        return FALSE;
                    }
                    m_fHLV5 = (m_AppVer >= 500);
                }
				m_hLog = hLog;
				m_CStat = TRUE;
			}
			return m_hLog != NULL ? TRUE : FALSE;
		}
		case 2:
		{
			if( m_pLink == NULL ) return FALSE;
			m_MMStat = m_pLink->IsConnected();
			if( m_MMStat != m_AStat ){
				m_AStat = m_MMStat;
				m_CStat = TRUE;
			}
			return m_MMStat;
		}
		default:
			return FALSE;
	}
}
//---------------------------------------------------------------------------
// 定期的なログのリンクの監視(200ms毎にコール)
BOOL CLogLink::TimerLogLink(void)
{
	if( !m_Enabled ) return FALSE;

	if( !m_Count ){
		m_Count = 5000/LINKINTERVAL;     // 5[s]毎のチェック
		BOOL f = m_hLog != NULL;
		CheckLink();
        if( m_hLog && !f ) m_1stSession = TRUE;
	}
	m_Count--;
	if( m_CStat ){
		m_CStat = 0;
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// 定期的な周波数ポーリング
void CLogLink::EventGetFreq(void)
{
	if( !m_Polling ) return;

	if( !m_Count2 ){
		m_Count2 = 2000/LINKINTERVAL;    // 2[s]毎のチェック
		switch(m_Enabled){
			case 1:
				{
					if( m_hLog == NULL ) return;
					int af = Application->Active;
					m_cds.dwData = 106;
					m_cds.cbData = 0;
					m_cds.lpData = NULL;
					::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hMain), LPARAM(&m_cds));
					if( af && m_1stSession ) Foreground();
                    m_1stSession = FALSE;

                }
				break;
			case 2:
				if( m_pLink == NULL ) return;
				m_pLink->EventVFO();
				break;
		}
	}
	m_Count2--;
}
//---------------------------------------------------------------------------
// 周波数の設定
void CLogLink::SetFreq(LPSTR pFreq)
{
	if( !CheckLink() ) return;

	switch(m_Enabled){
		case 1:
			m_cds.dwData = 6;
			m_cds.cbData = strlen(pFreq);
			m_cds.lpData = pFreq;
			m_hLogIn = (HWND)::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hApp), LPARAM(&m_cds));
			break;
		case 2:
			if( m_pLink == NULL ) return;
			m_pLink->SetFreq(pFreq);
			break;
	}
}
//---------------------------------------------------------------------------
// モードの設定
void CLogLink::SetMode(LPCSTR pMode)
{
	if( !CheckLink() ) return;

	switch(m_Enabled){
		case 1:
			if( pMode && (pMode != m_strMode) ){
			    StrCopy(m_strMode, pMode, sizeof(m_strMode)-1);
				if( !m_fHLV5 ){
					SetHamlogMode(m_strMode);
        	    }
            }
            if( m_strMode[0] ){
				m_cds.dwData = 7;
				m_cds.cbData = strlen(m_strMode);
				m_cds.lpData = m_strMode;
				m_hLogIn = (HWND)::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hApp), LPARAM(&m_cds));
            }
			break;
		case 2:
//			if( m_pLink == NULL ) return;
//			m_pLink->SetMode(pMode);
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall CLogLink::SetPTT(int ptt)
{
    if( !m_PTTEnabled ) return;
	if( !CheckLink() ) return;
	switch(m_Enabled){
		case 1:
        	{
				int af = Application->Active;
				m_cds.dwData = ptt ? 23 : 24;
				m_cds.cbData = 0;
				m_cds.lpData = NULL;
				::SendMessage(m_hLog, WM_COPYDATA, af ? WPARAM(m_hApp) : NULL, LPARAM(&m_cds));
				if( af && m_1stSession ) Foreground();
                m_1stSession = FALSE;
            }
			break;
		case 2:
			if( !m_pLink ) return;
			m_pLink->SetPTT(ptt);
			break;
	}
}
//---------------------------------------------------------------------------
// クリア
void CLogLink::Clear(void)
{
	if( !CheckLink() ) return;

	switch(m_Enabled){
		case 1:
		{
			int af = Application->Active;
			m_cds.dwData = 16;
			m_cds.cbData = 0;
			m_cds.lpData = NULL;
			m_hLogIn = (HWND)::SendMessage(m_hLog, WM_COPYDATA, af ? WPARAM(m_hApp) : NULL, LPARAM(&m_cds));
//    m_hLogIn = (HWND)::SendMessage(m_hLog, WM_COPYDATA, NULL, LPARAM(&m_cds));
			if( !::IsWindow(m_hLogIn) ){
				m_hLog = NULL;
				m_CStat = TRUE;
				m_Error = TRUE;
			}
			if( af ) Foreground();
			break;
		}
		case 2:
			m_QueryCall = "";
			if( m_pLink == NULL ) return;
			m_pLink->Clear();
			break;
	}
}
//---------------------------------------------------------------------------
// Hamlogにフォーカスを設定
void CLogLink::SetFocus(void)
{
	if( m_Enabled != 1 ) return;
	if( m_hLog == NULL ) return;
	if( m_hLogIn == NULL ) return;

	::SetForegroundWindow(m_hLogIn);
}
//---------------------------------------------------------------------------
// コールサインの転送
void CLogLink::SetCall(LPSTR pCall, int sw)
{
	if( !CheckLink() ) return;

	switch(m_Enabled){
		case 1:
		{
			int af = Application->Active;
			Clear();
			if( m_hLog == NULL ) return;
#if MMSSTV
			MultProc();
#endif

			AnsiString CALL;
			AnsiString PTB;

			if( m_fHLV5 ){
				CALL = pCall;
				m_cds.dwData = IsJA(pCall) ? 26 : 25;
				m_cds.cbData = 0;
				m_cds.lpData = NULL;
				m_hLogIn = (HWND)::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hApp), LPARAM(&m_cds));
				m_cds.dwData = 0;
				if( sw ) m_cds.dwData |= THW_ENTER;
				m_cds.cbData = strlen(CALL.c_str());
				m_cds.lpData = CALL.c_str();
				m_hLogIn = (HWND)::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hApp), LPARAM(&m_cds));
            }
			else {
				DevCall(CALL, PTB, pCall);
				m_cds.dwData = 0;
				if( sw ) m_cds.dwData |= THW_ENTER;
				m_cds.cbData = strlen(CALL.c_str());
				m_cds.lpData = CALL.c_str();
				m_hLogIn = (HWND)::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hApp), LPARAM(&m_cds));
#if MMSSTV
				MultProc();
#endif
				m_cds.dwData = 1;
				if( sw ) m_cds.dwData |= THW_ENTER;
				m_cds.cbData = strlen(PTB.c_str());
				m_cds.lpData = PTB.c_str();
				m_hLogIn = (HWND)::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hApp), LPARAM(&m_cds));
			}
			if( af ) Foreground();
		}
		case 2:
			m_QueryCall = pCall;
			if( m_pLink == NULL ) return;
			m_pLink->Query(pCall);
			break;
	}
}
//---------------------------------------------------------------------------
// コールサインの検索
void CLogLink::FindCall(LPSTR pCall)
{
	if( !CheckLink() ) return;

	switch(m_Enabled){
		case 1:
			{
			int af = Application->Active;
			SetCall(pCall, FALSE);
			if( m_hLog == NULL ) return;
#if MMSSTV
			MultProc();
#endif
			m_cds.dwData = 17;
			m_cds.cbData = 0;
			m_cds.lpData = NULL;
			::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hApp), LPARAM(&m_cds));

#if MMSSTV
			MultProc();
#endif

			m_cds.dwData = 115;
			m_cds.cbData = 0;
			m_cds.lpData = NULL;
			::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hMain), LPARAM(&m_cds));
			if( af ) Foreground();
			}
			break;
		case 2:
			m_QueryCall = pCall;
			if( m_pLink == NULL ) return;
			m_pLink->Query(pCall);
			break;
	}
}
//---------------------------------------------------------------------------
// ログの記録
// 0-データの設定のみ
// 1-データの設定とRETキー/問い合わせの実行(QSOの開始時)
// 2-書きこみを実行(QSOの終了時)
void CLogLink::Write(SDMMLOG *sp, int sw)
{
	if( !CheckLink() ) return;

	switch(m_Enabled){
		case 1:
			{
			int af = Application->Active;
			if( m_fHLV5 && (sw == 1) ){
				m_cds.dwData = IsJA(sp->call) ? 26 : 25;
				m_cds.cbData = 0;
				m_cds.lpData = NULL;
				m_hLogIn = (HWND)::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hApp), LPARAM(&m_cds));
				m_cds.dwData = 0;
				m_cds.cbData = strlen(sp->call);
				m_cds.lpData = sp->call;
				m_hLogIn = (HWND)::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hApp), LPARAM(&m_cds));
            }
			char bf[1024];
			MMtoHAMLOG(bf, sp, m_fHLV5);
			m_cds.dwData = 15;
			if( sw ) m_cds.dwData |= THW_ENTER;
			m_cds.cbData = strlen(bf);
			m_cds.lpData = bf;
			m_hLogIn = (HWND)::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hApp), LPARAM(&m_cds));

#if MMSSTV
			MultProc();
#endif
			switch(sw){
				case 1:
					m_cds.dwData = 115;
					m_cds.cbData = 0;
					m_cds.lpData = NULL;
					::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hMain), LPARAM(&m_cds));
					break;
				case 2:
					m_cds.dwData = 18 | THW_SAVEBOX_OFF;
					m_cds.cbData = 0;
					m_cds.lpData = NULL;
					::SendMessage(m_hLog, WM_COPYDATA, WPARAM(m_hApp), LPARAM(&m_cds));
					break;
			}
			if( af ) Foreground();
			}
			break;
		case 2:
			if( m_pLink == NULL ) return;
			m_QueryCall = sp->call;
			m_pLink->LogWrite(sp, sw);
			break;
	}
}
//---------------------------------------------------------------------------
int __fastcall GetLFCount(LPCSTR p)
{
	int n = 0;
	for(; *p; p++ ){
		if( *p == LF ) n++;
    }
    return n;
}
//---------------------------------------------------------------------------
// Hamlogからの返信メッセージからName,QTH,REM1を得る
int CLogLink::AnaData(SDMMLOG *sp, COPYDATASTRUCT *cp)
{
	BOOL r = FALSE;

	char bf[1024];
	int len = cp->cbData;
	if( len >= 1023 ) len = 1023;
	if( len && (cp->lpData != NULL) ){
		StrCopy(bf, LPCSTR(cp->lpData), len);
	    int lfLen = GetLFCount(bf);
		LPSTR t, p;
		if( len < 10 ){          // 周波数情報
			if( m_Polling ){
				t = SkipSpace(bf); ClipLF(t); clipsp(t);
				if( *t ){
					Log.SetFreq(&Log.m_sd, t);
					r = 106;
				}
			}
		}
		else if( lfLen >= 10 ){   // 検索結果情報
			p = StrDlm(t, bf, LF);  // Call
			p = StrDlm(t, p, LF);   // PTB
			p = StrDlm(t, p, LF);   // Date
			p = StrDlm(t, p, LF);   // Time
			p = StrDlm(t, p, LF);   // His
			p = StrDlm(t, p, LF);   // My
			p = StrDlm(t, p, LF);   // Freq
			p = StrDlm(t, p, LF);   // Mode
			p = StrDlm(t, p, LF);   // Code
			t = SkipSpace(t); ClipLF(t); clipsp(t);
			if( *t ){
				StrCopy(sp->opt1, t, MLOPT);
				r = 115;
			}
			p = StrDlm(t, p, LF);   // GL
			p = StrDlm(t, p, LF);   // QSL
			p = StrDlm(t, p, LF);   // Name
			if( !sp->name[0] ){
				t = SkipSpace(t); ClipLF(t); clipsp(t);
				if( *t ){
					StrCopy(sp->name, t, MLNAME);
					r = 115;
				}
			}
			p = StrDlm(t, p, LF);   // QTH
//			if( !sp->qth[0] ){
			if( !sp->qth[0] || !IsJA(sp->call) ){
				t = SkipSpace(t); ClipLF(t); clipsp(t);
				if( *t ){
					StrCopy(sp->qth, t, MLQTH);
					r = 115;
				}
			}
			StrDlm(t, p, LF);      // REM1
			if( !sp->rem[0] ){
				t = SkipSpace(t); ClipLF(t); clipsp(t);
				if( *t ){
					StrCopy(sp->rem, t, MLREM);
					r = 115;
				}
			}
		}
	}
	return r;
}
//---------------------------------------------------------------------------
void __fastcall CLogLink::NotifySession(LPCSTR pSession)
{
	if( m_pLink == NULL ) return;

	m_pLink->NotifySession(pSession);
	CheckLink();
}
//---------------------------------------------------------------------------
int __fastcall CLogLink::QReturn(SDMMLOG *sp, const mmLOGDATA *pLog)
{
	if( pLog == NULL ) return FALSE;
	int r = FALSE;

	mmLOGDATA logdata;
	memcpy(&logdata, pLog, sizeof(logdata));

	if( stricmp(logdata.m_Call, m_QueryCall.c_str()) ) return FALSE;
	LPSTR t;
	if(!sp->name[0] ){
		t = SkipSpace(logdata.m_Name); ClipLF(t); clipsp(t);
		if( *t ){
			StrCopy(sp->name, t, MLNAME);
			r = TRUE;
		}
	}
	if( !sp->qth[0] ){
		t = SkipSpace(logdata.m_QTH); ClipLF(t); clipsp(t);
		if( *t ){
			StrCopy(sp->qth, t, MLQTH);
			r = TRUE;
		}
	}
	if( !sp->rem[0] ){
		t = SkipSpace(logdata.m_Note); ClipLF(t); clipsp(t);
		if( *t ){
			StrCopy(sp->rem, t, MLREM);
			r = TRUE;
		}
	}
	return r;
}
//---------------------------------------------------------------------------
void __fastcall CLogLink::SetTime(SYSTEMTIME *tp, int sw)
{
	if( m_pLink == NULL ) return;

	m_pLink->SetTime(tp, sw);
}
//---------------------------------------------------------------------------
LPCSTR __fastcall CLogLink::GetItemName(void)
{
	return m_ItemName.c_str();
}
//---------------------------------------------------------------------------
void __fastcall CLogLink::SetItemName(LPCSTR pName)
{
	m_ItemName = pName;
}
//---------------------------------------------------------------------------
void __fastcall CLogLink::LoadMMLink(TMemIniFile *pIni)
{
	m_ItemName = pIni->ReadString("MMLink", "Name", "");
}
//---------------------------------------------------------------------------
void __fastcall CLogLink::SaveMMLink(TMemIniFile *pIni)
{
	pIni->WriteString("MMLink", "Name", m_ItemName);
}
//---------------------------------------------------------------------------
BOOL __fastcall CLogLink::IsCopyData(void)
{
	if( m_Enabled != 2 ) return FALSE;
	if( !m_pLink ) return FALSE;
	return (m_pLink->GetCaps() & capWMCOPYDATA);
}
//---------------------------------------------------------------------------

