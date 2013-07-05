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
#include <vcl.h>		//ja7ude 0521
#pragma hdrstop

#include <io.h>

#include "Sound.h"
#include "Main.h"
//---------------------------------------------------------------------------
//   注意: VCL オブジェクトのメソッドとプロパティを使用するには, Synchronize
//         を使ったメソッド呼び出しでなければなりません。次に例を示します。
//
//      Synchronize(UpdateCaption);
//
//   ここで, UpdateCaption は次のように記述できます。
//
//      void __fastcall TSound::UpdateCaption()
//      {
//        Form1->Caption = "スレッドから書き換えました";
//      }
//---------------------------------------------------------------------------
__fastcall TSound::TSound(bool CreateSuspended)
	: TThread(CreateSuspended)
{
	::VirtualLock(this, sizeof(TSound));
	m_BuffSize = SampSize;
	m_ReqFifoSize = 0;
	m_FifoSizeRX = 12;
	m_FifoSizeTX = 8;
	m_Stop = TRUE;

	m_IDDevice = WAVE_MAPPER;
	m_IDTXDevice = WAVE_MAPPER;
	m_playmode = 0;
	m_susp = 0;
	m_suspack = 0;

	InitWFX();
	m_lms = 0;
	m_lms_AN = 0;
	m_notch = 0;
	m_Tx = m_ReqTx = 0;
    m_ReqSpeedTest = 0;
    Wave.GetDeviceName();
}

__fastcall TSound::~TSound()
{
	::VirtualUnlock(this, sizeof(TSound));
}

void __fastcall TSound::InitWFX(void)
{
	m_WFX.wFormatTag = WAVE_FORMAT_PCM;
	m_WFX.nChannels = WORD(sys.m_SoundStereo ? 2 : 1);
	m_WFX.wBitsPerSample = 16;
	m_WFX.nSamplesPerSec = int(SampBase);
	m_WFX.nBlockAlign = WORD(m_WFX.nChannels *(m_WFX.wBitsPerSample/8));
	m_WFX.nAvgBytesPerSec = m_WFX.nBlockAlign * m_WFX.nSamplesPerSec;
	m_WFX.cbSize = 0;
	Wave.m_SoundStereo = sys.m_SoundStereo;
    Wave.m_StereoTX = sys.m_StereoTX;
}

void __fastcall TSound::Stop(void)
{
	if( !m_Stop ){
		Priority = tpNormal;
		Terminate();
#if 0
		DWORD tim = GetTickCount();
		while( GetTickCount() < (tim + 1000) ){		// 1[s]のウエイト
			if( m_Stop == TRUE ) break;
			::Sleep(0);
		}
#else
		WaitFor();
#endif
	}
}

void __fastcall TSound::ReqStop(void)
{
	if( m_Stop == FALSE ){
		Priority = tpNormal;
		Terminate();
	}
}

void __fastcall TSound::WaitStop(void)
{
#if 0
	DWORD tim = GetTickCount();
	while( GetTickCount() < (tim + 1000) ){		// 1[s]のウエイト
		if( m_Stop == TRUE ) break;
		::Sleep(0);
	}
#else
	if( !m_Stop ) WaitFor();
#endif
}

void __fastcall TSound::InitSound(void)
{
	Suspend();
	InitWFX();
	m_ReqFifoSize = 1;
	Resume();
}
//---------------------------------------------------------------------------
void __fastcall TSound::ErrorMsg(void)
{
	if( m_IDDevice == -2 ){
		InfoMB("Sound I/O failed in the MMW (%s)", sys.m_SoundMMW.c_str());
    }
    else {
		ErrorMB(MsgEng?"Can't open Sound card (%d)":"サウンドカードがオープンできません.", m_Tx ? m_IDTXDevice : m_IDDevice);
    }
}
//---------------------------------------------------------------------------
BOOL __fastcall TSound::ReInitSound(int sw)
{
	int r = TRUE;
	if( Terminated == TRUE ) return r;
	if( sw ){
		Wave.OutAbort();
	}
	else {
		Wave.InClose();
	}
	InitWFX();
	if( sw ){
		if( Wave.OutOpen(&m_WFX, m_IDTXDevice, m_BuffSize) != TRUE ){
			::Sleep(200);
			if( Wave.OutOpen(&m_WFX, m_IDTXDevice, m_BuffSize) != TRUE ){
				r = FALSE;
			}
		}
	}
	else {
		if( Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize) != TRUE ){
			::Sleep(200);
			if( Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize) != TRUE ){
				r = FALSE;
			}
		}
	}
    return r;
}

void __fastcall TSound::TaskPriority(void)
{
	switch(sys.m_SoundPriority){
		case 0:
			if( Priority != tpNormal ){
				Priority = tpNormal;		//スレッドは通常の優先度である
			}
			break;
		case 1:
			if( Priority != tpHigher ){
				Priority = tpHigher;		//スレッドの優先度は通常よりも 1 ポイント高い
			}
			break;
		case 2:
			if( Priority != tpHighest ){
				Priority = tpHighest;		//スレッドの優先度は通常よりも 2 ポイント高い
			}
			break;
		default:
			if( Priority != tpTimeCritical ){
				Priority = tpTimeCritical;	//スレッドはもっとも高い優先度を取得する
			}
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TSound::SetDeviceID(void)
{
	LPSTR t, p;
    AnsiString as = sys.m_SoundDevice.c_str();
	p = StrDlm(t, as.c_str());
	int dd;
	if( ATOI(dd, t) ){
		m_IDDevice = dd;
	}
    else {
		BOOL f = FALSE;

		int i;
		for( i = 0; i < Wave.m_InDevs; i++ ){
			if( !strcmp(t, Wave.m_tInDevName[i].c_str()) ){
				m_IDDevice = i;
				f = TRUE;
                break;
            }
        }
        if( !f ){				// MMW device
			m_IDDevice = -2;
			m_IDTXDevice = -2;
	        sys.m_SoundMMW = t;
			return;
        }
    }

	if( *p ) StrDlm(t, p);

	if( ATOI(dd, t) ){
		m_IDTXDevice = dd;
    }
    else {
		BOOL f = FALSE;
		int i;
		for( i = 0; i < Wave.m_OutDevs; i++ ){
			if( !strcmp(t, Wave.m_tOutDevName[i].c_str()) ){
				m_IDTXDevice = i;
				f = TRUE;
                break;
            }
        }
        if( !f ){
			m_IDTXDevice = m_IDDevice;
        }
    }
}
//---------------------------------------------------------------------------
int __fastcall TSound::UpdateFifoSize(void)
{
	int r = 2;
	if( !m_ReqFifoSize ) return 1;

	int fi = Wave.IsInOpen();
    int fo = Wave.IsOutOpen();
    if( fi ) Wave.InClose();
    if( fo ) Wave.OutAbort();
	m_FifoSizeRX = sys.m_SoundFifoRX;
	m_FifoSizeTX = sys.m_SoundFifoTX;
	Wave.m_InFifoSize = m_FifoSizeRX;
	Wave.m_OutFifoSize = m_FifoSizeTX;

	SetDeviceID();

    Wave.UpdateDevice(m_IDDevice);
    if( fi ){
        if( !Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize) ) r = FALSE;
    }
    if( fo ){
        if( !Wave.OutOpen(&m_WFX, m_IDTXDevice, m_BuffSize) ) r = FALSE;
    }
	TaskPriority();
	m_ReqFifoSize = 0;
    return r;
}

//---------------------------------------------------------------------------
void __fastcall TSound::Execute()
{
	m_Stop = FALSE;
	memset(Buff, 0, sizeof(Buff));
	TaskPriority();
	::Sleep(200);		// 200ms
	Wave.m_InFifoSize = m_FifoSizeRX;
	Wave.m_OutFifoSize = m_FifoSizeTX;
	SetDeviceID();
    Wave.UpdateDevice(m_IDDevice);
_init:;
	Wave.InClose();
	if( Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize) != TRUE ){
		int timeout = Wave.GetTimeout() / 200;
		for( int i = 0; i < timeout; i++ ){
			::Sleep(200);
			if( Terminated == TRUE ) goto _ex;
			if( UpdateFifoSize() == 2 ) goto _init;
			if( Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize) == TRUE ) break;
		}
		if( Wave.IsInOpen() != TRUE ){
			if( timeout ) ErrorMsg();
			if( Terminated == TRUE ) goto _ex;
			int Count = 3000/50;
			while(1){							// 擬似実行
				if( Terminated == TRUE ) goto _ex;
				::Sleep(50);
		        Wave.PumpMessages();
				if( (m_Tx != m_ReqTx) || m_ReqFifoSize || !Count ){
					Count = 3000/50;
					if( UpdateFifoSize() == 2 ) goto _init;
					m_Tx = m_ReqTx;
					if( m_Tx ){
						if( Wave.OutOpen(&m_WFX, m_IDTXDevice, m_BuffSize) == TRUE ) break;
					}
					else {
						if( Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize) == TRUE ) break;
					}
				}
                else if( m_susp ){
					JobSuspend(); break;
                }
				Count--;
			}
			m_Tx = 0;
		}
	}
	m_Tx = 0;
	Wave.OutAbort();
	double *lp;
	int i;
	while(1){
		if( Terminated == TRUE ) break;
		if( Wave.IsInOpen() == TRUE ){
			if( Wave.InRead(Buff, m_BuffSize) == FALSE ){
				m_Err = 10;
				SSTVDEM.m_Lost++;
				if( !ReInitSound(0) ) goto _init;
			}
		}
		if( m_playmode && (!m_Tx || (m_playmode != 1))) WaveFile.ReadWrite(Buff, m_BuffSize);
		if( Terminated == TRUE ) break;
		if( sys.m_TestDem ){
			for( i = 0, lp = Buff; i < m_BuffSize; i++, lp++ ){
				*lp = SSTVMOD.Do();
			}
		}
		if( !m_Tx || sys.m_echo ){
			if( m_lms || m_notch ){
				for( i = 0, lp = Buff; i < m_BuffSize; i++, lp++ ){
#if VARITEST
					*lp += ns.GetNoise();
#endif
					if( m_notch ) *lp = notch.Do(*lp);
					if( m_lms ){
						if( m_lms_AN ){
							*lp = lms.DoN(*lp);
						}
						else {
							*lp = lms.Do(*lp);
						}
					}
					SSTVDEM.Do(*lp);
				}
			}
			else {
				for( i = 0, lp = Buff; i < m_BuffSize; i++, lp++ ){
#if VARITEST
					*lp += ns.GetNoise();
#endif
					SSTVDEM.Do(*lp);
				}
			}
		}
		fftIN.CollectFFT(Buff, m_BuffSize);
_skip1:;
		if( m_Tx ){
			if( (sys.m_echo == 2) && Wave.IsInBufCritical() && Wave.IsInOpen() ) goto _skip2;
			for( i = 0, lp = Buff; i < m_BuffSize; i++, lp++ ){
				*lp = SSTVMOD.Do();
				if( !sys.m_echo ) SSTVDEM.Idle(*lp);
			}
		}
		if( Wave.IsOutOpen() == TRUE  ){
			if( Wave.OutWrite(Buff, m_BuffSize) != TRUE ){
				m_Err = 10;
				SSTVMOD.m_Lost++;
				ReInitSound(1);
				if( Wave.OutWrite(Buff, m_BuffSize) != TRUE ){
					if( !ReInitSound(1) ) goto _init;
				}
			}
			if( (sys.m_echo == 2) && m_Tx && Wave.IsInBufNull() && !Wave.IsOutBufFull() ) goto _skip1;
		}
_skip2:;
		if( m_ReqFifoSize ){
        	if( !UpdateFifoSize() ) goto _init;
		}
		if( m_Tx != m_ReqTx ){
			if( m_ReqTx ){
				if( sys.m_echo != 2 ){
					Wave.InClose();
					Mmsstv->InitFFT();
					fftIN.TrigFFT();
					SSTVDEM.m_lvl.Init();
				}
				if( m_playmode != 1 ){      // 再生中でない時
					if( sys.m_echo == 2 ){
						switch(Priority){       // 優先順位を１つ上げる
							case tpNormal:
								Priority = tpHigher;
								break;
							case tpHigher:
								Priority = tpHighest;
								break;
							case tpHighest:
								Priority = tpTimeCritical;
								break;
						}
					}
					Wave.OutOpen(&m_WFX, m_IDTXDevice, m_BuffSize);
					if( !Wave.IsOutOpen() ){
						m_susp = 2;
						goto _susp;
					}
					if( sys.m_echo == 2 ){      // 最初のデータを溜める
						for( int j = 0; (j < (m_FifoSizeTX - 1)) && !Wave.IsOutBufFull(); j++ ){
							for( i = 0, lp = Buff; i < m_BuffSize; i++, lp++ ){
								*lp = SSTVMOD.Do();
							}
							if( Wave.OutWrite(Buff, m_BuffSize) != TRUE ){
								break;
							}
						}
					}
				}
				SSTVDEM.m_OverFlow = 0;
				SSTVMOD.m_Lost = 0;
				SSTVDEM.m_Lost = 0;
				m_Tx = m_ReqTx;
				Wave.SetPTT(m_Tx);
			}
			else {
				Wave.OutAbort();
				if( sys.m_echo != 2 ){
					Mmsstv->InitFFT();
					fftIN.TrigFFT();
					for( i = 0; i < SSTVDEM.m_bpftap; i++ ) SSTVDEM.Do(1);
					SSTVDEM.m_OverFlow = 0;
					SSTVDEM.m_lvl.Init();
					SSTVDEM.m_Lost = 0;
				}
				m_Tx = m_ReqTx;
				Wave.SetPTT(m_Tx);
				TaskPriority();
				if( !Wave.IsInOpen() ){
					Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize);
					if( !Wave.IsInOpen() ){
						m_susp = 2;
						goto _susp;
					}
				}
			}
			if( m_playmode == 1 ) m_playmode = 0;
		}
		if( (!m_Tx) && (WaveFile.m_mode != m_playmode) ){
			if( WaveFile.m_mode == 1 ){
				Wave.InClose();
				Wave.OutOpen(&m_WFX, m_IDTXDevice, m_BuffSize);
				SSTVDEM.m_OverFlow = 0;
			}
			else if( m_playmode == 1 ){
				Wave.OutClose();
				Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize);
				SSTVDEM.m_OverFlow = 0;
			}
			m_playmode = WaveFile.m_mode;
		}
_susp:;
		if( m_susp ){
        	JobSuspend();
		}
        if( m_ReqSpeedTest ){
			JobSpeedTest();
        }
	}
_ex:;
	Wave.InClose();
	Wave.OutAbort();
    Wave.UpdateDevice(-1);
	m_Stop = TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TSound::JobSuspend(void)
{
	Wave.InClose();
	Wave.OutAbort();
	m_suspack = 1;
	Priority = tpNormal;
	while(1){
		if( Terminated == TRUE ){
			m_Stop = TRUE;
			return;
		}
		if( m_Tx != m_ReqTx ){
			m_Tx = m_ReqTx;
			if( m_Tx && (m_susp == 2) ){
				m_susp = 0;
			}
		}
		Sleep(100);
        Wave.PumpMessages();
		if( !m_susp ){
			UpdateFifoSize();
			for( int i = 0; i < 5; i++ ){
				if( m_Tx ){
					if( Wave.OutOpen(&m_WFX, m_IDTXDevice, m_BuffSize) == TRUE ) break;
				}
				else {
					if( Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize) == TRUE ) break;
				}
				::Sleep(100);
		        Wave.PumpMessages();
			}
			if( !m_Tx && Wave.IsInOpen() ){
				m_suspack = 1;
				break;
			}
			else if( m_Tx && Wave.IsOutOpen() ){
				m_suspack = 1;
				break;
			}
			else {
				m_susp = 2;
				m_suspack = 1;
				ErrorMsg();
			}
		}
	}
	TaskPriority();
	if( m_Tx ){
		Wave.OutAbort();
		m_Tx = 0;
	}
	m_playmode = 0;
	DisPaint = FALSE;
}

//---------------------------------------------------------------------------
int __fastcall TSound::IsBusy()
{
	if( Wave.IsInOpen() && Wave.IsInBufCritical() ) return TRUE;
	if( Wave.IsOutOpen() && Wave.IsOutBufCritical() ) return TRUE;
	return FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TSound::JobSpeedTest()
{
	DWORD tc = ::GetTickCount();
	SSTVMOD.InitTXBuf();
    SSTVDEM.Stop();
    for( int i = 0; i < 2000000; i++ ){
		SSTVMOD.Do();
		SSTVDEM.Do(0);
    }
	m_SpeedValue = ::GetTickCount() - tc;
	m_ReqSpeedTest = 0;
}
//---------------------------------------------------------------------------
// CWaveFileクラス
void __fastcall CWaveFile::ReadWrite(double *s, int size)
{
	SHORT	d;

	if( m_Handle != NULL ){
		if( m_mode == 2 ){		// 書きこみ
			if( !m_pause ){
				for( ; size; s++, size-- ){
					d = SHORT(*s);
					if( mmioWrite(m_Handle, (const char*)&d, 2) != 2 ){
						mmioClose(m_Handle, 0);
						m_Handle = 0;
						m_mode = 0;
						break;
					}
					else {
						m_pos += 2;
					}
				}
			}
		}
		else {						// 読み出し
			if( m_pause || m_dis ){
				memset(s, 0, sizeof(double)*size);
			}
			else {
				for( ; size; s++, size-- ){
					if( mmioRead(m_Handle, (char *)&d, 2) == 2 ){
						*s = d;
						m_pos += 2;
					}
					else if( m_autopause ){
						m_pause = 1;
						break;
					}
					else {
						mmioClose(m_Handle, 0);
						m_Handle = 0;
						m_mode = 0;
						break;
					}
				}
				for( ; size; s++, size-- ){
					*s = 0;
				}
			}
		}
	}
}

__fastcall CWaveFile::CWaveFile()
{
	m_mode = 0;
	m_pause = 0;
	m_Handle = NULL;
	m_dis = 0;
	m_autopause = 0;
}

__fastcall CWaveFile::~CWaveFile()
{
	FileClose();
}

void __fastcall CWaveFile::FileClose(void)
{
	m_mode = 0;
	m_pause = 0;
	if( m_Handle != NULL ){
		mmioClose(m_Handle, 0);
		m_Handle = 0;
	}
}

void __fastcall CWaveFile::Rec(LPCSTR pName)
{
	FileClose();
	m_FileName = pName;
	m_Handle = mmioOpen(m_FileName.c_str(), NULL, MMIO_CREATE|MMIO_WRITE|MMIO_ALLOCBUF);
	if( m_Handle == NULL ){
		ErrorMB( MsgEng?"Can't open '%s'":"'%s'を作成できません.", pName);
		return;
	}
	m_Head[0] = 0x55;
	m_Head[1] = 0xaa;
	m_Head[2] = char(SampType);
	m_Head[3] = 0;
	mmioWrite(m_Handle, (const char *)m_Head, 4);
	m_pos = 4;
	m_mode = 2;
	m_pause = 0;
	m_dis = 0;
}

BOOL __fastcall CWaveFile::Play(LPCSTR pName)
{
	FileClose();
	FILE *fp = fopen(pName, "rb");
	if( fp == NULL ){
		ErrorMB( MsgEng?"Can't open '%s'":"'%s'をオープンできません.", pName);
		return FALSE;
	}
	m_length = filelength(fileno(fp));
	m_FileName = pName;
	fclose(fp);
	m_Handle = mmioOpen(m_FileName.c_str(), NULL, MMIO_READ|MMIO_ALLOCBUF);
	if( m_Handle == NULL ){
		ErrorMB( MsgEng?"Can't open '%s'":"'%s'をオープンできません.", pName);
		return FALSE;
	}
	m_pos = 0;
	if( mmioRead(m_Handle, (char *)m_Head, 4) == 4 ){
		int type = 0;
		if( (m_Head[0] == 0x55)&&(m_Head[1] == 0xaa) ){
			type = m_Head[2];
			m_pos = 4;
			m_length -= 4;
		}
		if( type > 9 ) type = 0;
		if( type != SampType ){
			if( YesNoMB(
				MsgEng
				? "%s\r\n\r\nThis file has been recorded based on %uHz, play it with sampling conversion?"
				: "%s\r\n\r\nこのファイルは %uHz ベースで記録されています. 周波数変換して再生しますか？",
				m_FileName.c_str(), SampTable[type] ) == IDNO ){
				mmioClose(m_Handle, 0);
				m_Handle = 0;
				return TRUE;
			}
			else {
				mmioClose(m_Handle, 0);
				m_Handle = 0;
				char bf[1024];
				strcpy(bf, pName);
				SetEXT(bf, "");
				char wName[1024];
				sprintf(wName, "%s_%u.MMV", bf, int(SampBase));
				if( ChangeSampFreq(wName, pName, SampTable[type]) == TRUE ){
					Play(wName);
				}
			}
		}
	}
	m_mode = 1;
	m_pause = 0;
	m_dis = 0;
	return TRUE;
}

int __fastcall CWaveFile::ChangeSampFreq(LPCSTR tName, LPCSTR pName, int sSamp)
{
	int rr = FALSE;
	FILE *fp = fopen(pName, "rb");
	if( fp != NULL ){
		CWaitCursor w;
		BYTE    head[4];
		memset(head, 0, sizeof(head));
		head[0] = 0x55;
		head[1] = 0xaa;
		head[2] = char(SampType);
		head[3] = 0;
		FILE *wfp = fopen(tName, "wb");
		if( wfp != NULL ){
			fwrite(head, 1, 4, wfp);

			int rsize = 16384;
			int wsize = rsize * SampBase/double(sSamp);
			short *rp = new short[rsize];
			short *wp = new short[wsize];
			short *ip = new short[rsize*4];
			CIIR    riir;
			riir.MakeIIR(2800, sSamp*4, 8, 1, 0.3);
			CIIR    wiir;
			wiir.MakeIIR(2800, SampBase, 16, 1, 0.3);
			int rlen, wlen;
			short *sp, *tp;
			int i, r;
            double k;
			while(1){
				rlen = fread(rp, 1, rsize * 2, fp);
				if( !rlen ) break;
				rlen /= 2;
				if( SampBase < sSamp ){             // デシメーション
					sp = rp;
					tp = ip;
					for( i = 0; i < rlen; i++, sp++ ){	// 一旦４倍にする
						*tp++ = riir.Do(*sp);
						*tp++ = riir.Do(*sp);
						*tp++ = riir.Do(*sp);
						*tp++ = riir.Do(*sp);
					}
					k = double(sSamp*4) / double(SampBase);
					tp = wp;
					wlen = rlen * SampBase / sSamp;
					if( wlen > wsize ) wlen = wsize;
					for( i = 0; i < wlen; i++ ){
						r = int((double(i) * k) + 0.5);
    	                if( r >= (rlen*4) ) r = (rlen*4) - 1;
						*tp++ = wiir.Do(ip[r]);
					}
				}
                else {
					k = double(sSamp) / double(SampBase);
					tp = wp;
					wlen = rlen * SampBase / sSamp;
					if( wlen > wsize ) wlen = wsize;
					for( i = 0; i < wlen; i++ ){
						r = int((double(i) * k) + 0.5);
    	                if( r >= rlen ) r = rlen - 1;
						*tp++ = wiir.Do(rp[r]);
					}
                }
				if( fwrite(wp, 1, wlen * 2, wfp) != size_t(wlen * 2) ) break;
			}
			if( !fclose(wfp) ) rr = TRUE;
			delete rp;
			delete wp;
		}
		fclose(fp);
	}
	return rr;
}

void __fastcall CWaveFile::Rewind(void)
{
	if( m_Handle != NULL ){
		m_dis++;
		if( m_mode == 2 ){
			mmioSeek(m_Handle, 4, SEEK_SET);
			m_pos = 4;
		}
		else {
			mmioSeek(m_Handle, 0, SEEK_SET);
			m_pos = 0;
		}
		m_dis--;
	}
}

void __fastcall CWaveFile::Seek(int n)
{
	if( m_Handle != NULL ){
		m_dis++;
		if( (m_Head[0] == 0x55)&&(m_Head[1] == 0xaa) ){
			mmioSeek(m_Handle, n + 4, SEEK_SET);
			m_pos = n + 4;
		}
		else {
			mmioSeek(m_Handle, n, SEEK_SET);
			m_pos = n;
		}
		m_dis--;
	}
}

long __fastcall CWaveFile::GetPos(void)
{
	if( (m_Head[0] == 0x55)&&(m_Head[1] == 0xaa) ){
		long n = m_pos - 4;
		if( n < 0 ) n = 0;
		return n;
	}
	else {
		return m_pos;
	}
}

//---------------------------------------------------------------------------
// CWaveStrageクラス

__fastcall CWaveStrage::CWaveStrage()
{
	m_Handle = NULL;

	m_pos = m_rpos = m_wpos = 0;
	pData = pSync = NULL;
}

void __fastcall CWaveStrage::Close(void)
{
	if( m_Handle != NULL ){
		mmioClose(m_Handle, 0);
		m_Handle = 0;
		unlink(m_FileName.c_str());
	}
	if( pData != NULL ){
		delete pData;
		pData = NULL;
	}
	if( pSync != NULL ){
		delete pSync;
		pSync = NULL;
	}
}

void __fastcall CWaveStrage::Open(void)
{
	Close();
	char bf[256];
	sprintf(bf, "%sSound.tmp", BgnDir);
	m_FileName = bf;
	m_Handle = mmioOpen(bf, NULL, MMIO_READWRITE|MMIO_CREATE|MMIO_ALLOCBUF);
	m_wpos = m_rpos = m_pos = 0;
	pData = new short[1400*SampFreq/1000];
	pSync = new short[1400*SampFreq/1000];
}

void __fastcall CWaveStrage::RInit(void)
{
	m_rpos = 0;
}

void __fastcall CWaveStrage::WInit(void)
{
	m_wpos = 0;
}

void __fastcall CWaveStrage::Seek(int n)
{
	mmioSeek(m_Handle, n, SEEK_SET);
	m_pos = n;
}

void __fastcall CWaveStrage::Read(short *sp, int size)
{
	if( m_pos != m_rpos ){
		Seek(m_rpos);
	}
	mmioRead(m_Handle, (char *)sp, size);
	m_rpos += size;
	m_pos = m_rpos;
}

void __fastcall CWaveStrage::Write(short *sp, int size)
{
	if( m_pos != m_wpos ){
		Seek(m_wpos);
	}
	mmioWrite(m_Handle, (char *)sp, size);
	m_wpos += size;
	m_pos = m_wpos;
}






