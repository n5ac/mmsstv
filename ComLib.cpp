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

#include "ComLib.h"
#include "PicRect.h"
#include "LogFile.h"
#include "sstv.h"
#include "jhandle.h"
#include "main.h"
//---------------------------------------------------------------------------
//#define	SYSMB	1			// MessageBox 0-Application, 1-System
//
char	BgnDir[256];		// 起動ディレクトリ

char    HistDir[256];
char    StockDir[256];

char	BitmapDir[256];     // ビットマップロード・セーブ
char	SBitmapDir[256];     // ビットマップロード・セーブ
char	MMLogDir[256];		// MMLOGファイルのディレクトリ
char	ExtLogDir[256];		// エクスポートファイルのディレクトリ
char	RecDir[256];		// 録音ファイルのディレクトリ
char	TemplateDir[256];     // ビットマップロード・セーブ

AnsiString	JanHelp;

double	SampFreq = 11025.0;	// サンプリング周波数
double	SampBase = 11025.0;
int		DemOver = 1;
int		SampType = 0;
int     SampSize = 2048;

double  FFTSamp = 11025.0;
int     FFTSampType = 0;
int		FFT_SIZE=2048;

SYSSET	sys;
LCID	lcid;				// ロケール情報
DWORD   ColorTable[128];
int		DisPaint;
int     MsgEng;
//HPALETTE UsrPal;
int     WinNT;
int     WinVista;

CExecPB	g_ExecPB;

const int SampTable[]={11025, 8000, 6000, 12000, 16000, 18000, 22050, 24000, 44100, 48000};

//---------------------------------------------------------------------------
void __fastcall SetDisPaint(void)
{
	DisPaint++;
}
void __fastcall ResDisPaint(void)
{
	if( DisPaint ) DisPaint--;
}
//---------------------------------------------------------------------------
void ShowHelp(int index)
{
	if( !JanHelp.IsEmpty() ){
		HWND hd;
		if( Screen->ActiveForm != NULL ){
			hd = Screen->ActiveForm->Handle;
		}
		else {
			hd = Application->MainForm->Handle;
		}
		char bf[256];
		sprintf(bf, "%s%s", BgnDir, JanHelp.c_str());
		if( !strcmpi(GetEXT(JanHelp.c_str()), "HLP") ){
			if( index != -1 ){
				::WinHelp(hd, bf, HELP_CONTEXT, index);
			}
			else {
				::WinHelp(hd, bf, HELP_FINDER, 0);
			}
		}
		else {
			::ShellExecute(hd, "open", bf, NULL, NULL, SW_SHOWDEFAULT);
		}
	}
}
//---------------------------------------------------------------------------
void InitSampType(void)
{

	if( SampFreq >= 46000.0 ){
		SampType = 9;
		SampBase = 48000.0;
		SampSize = (48000*2048)/11025;
		DemOver = 0;
		FFTSamp = SampFreq * 0.25;
		FFTSampType = 2;                // X4 over sampling
		FFT_SIZE = 2048;
    }
	else if( SampFreq >= 43000.0 ){
		SampType = 8;
		SampBase = 44100.0;
		SampSize = (44100*2048)/11025;
		DemOver = 0;
		FFTSamp = SampFreq * 0.25;
		FFTSampType = 2;                // X4 over sampling
		FFT_SIZE = 2048;
	}
	else if( SampFreq >= 23000.0 ){
		SampType = 7;
		SampBase = 24000.0;
		SampSize = (24000*2048)/11025;
		DemOver = 0;
		FFTSamp = SampFreq * 0.5;
		FFTSampType = 1;                // X2 over sampling
		FFT_SIZE = 2048;
	}
	else if( SampFreq >= 20000.0 ){
		SampType = 6;
		SampBase = 22050.0;
		SampSize = (22050*2048)/11025;
		DemOver = 0;
		FFTSamp = SampFreq * 0.5;
		FFTSampType = 1;                // X2 over sampling
		FFT_SIZE = 2048;
	}
	else if( SampFreq >= 17000.0 ){
		SampType = 5;
		SampBase = 18000.0;
		SampSize = (18000*2048)/11025;
		DemOver = 0;
		FFTSamp = SampFreq * 0.5;
		FFTSampType = 1;                // X2 over sampling
		FFT_SIZE = 2048;
	}
	else if( SampFreq >= 15000.0 ){
		SampType = 4;
		SampBase = 16000.0;
		SampSize = (16000*2048)/11025;
		DemOver = 0;
		FFTSamp = SampFreq;
		FFTSampType = 0;
		FFT_SIZE = 2048;
	}
	else if( SampFreq >= 11600.0 ){
		SampType = 3;
		SampBase = 12000.0;
		SampSize = (12000*2048)/11025;
		DemOver = 0;
		FFTSamp = SampFreq;
		FFTSampType = 0;
		FFT_SIZE = 2048;
	}
	else if( SampFreq >= 10000.0 ){
		SampType = 0;
		SampBase = 11025.0;
		SampSize = 2048;
		DemOver = 0;
		FFTSamp = SampFreq;
		FFTSampType = 0;
		FFT_SIZE = 2048;
	}
	else if( SampFreq >= 7000.0 ){
		SampType = 1;
		SampBase = 8000.0;
		SampSize = (8000*2048)/11025;
		DemOver = 0;
		FFTSamp = SampFreq;
		FFTSampType = 0;
		FFT_SIZE = 1024;
	}
	else if( SampFreq >= 5000.0 ){
		SampType = 2;
		SampBase = 6000.0;
		SampSize = (6000*2048)/11025;
		DemOver = 0;
		FFTSamp = SampFreq;
		FFTSampType = 0;
		FFT_SIZE = 1024;
	}
	if( SampSize & 1 ) SampSize++;
	if( SampSize > 8192 ) SampSize = 8192;
}
//---------------------------------------------------------------------------
double NormalSampFreq(double d, double m)
{
	d = double(int((d * m) + 0.5)/m);
	return d;
}
//---------------------------------------------------------------------------
int IsFile(LPCSTR pName)
{
	FILE *fp;
	fp = fopen(pName, "rb");
	if( fp != NULL ){
		fclose(fp);
		return 1;
	}
	else {
		return 0;
	}
}
//---------------------------------------------------------------------------
int SetTimeOffsetInfo(int &Hour, int &Min)
{
	TIME_ZONE_INFORMATION tz;

	int off;
	switch(GetTimeZoneInformation(&tz)){
		case TIME_ZONE_ID_STANDARD:
			off = tz.StandardBias;
			break;
		case TIME_ZONE_ID_DAYLIGHT:
			off = tz.DaylightBias;
			break;
		default:
			return FALSE;
	}
	if( off >= 0 ){
		Hour = -(off / 60);
		Min = (off % 60);
	}
	else {
		off = -off;
		Hour = off / 60;
		Min = off % 60;
	}
	return TRUE;
}
//---------------------------------------------------------------------------
void AddjustOffset(SYSTEMTIME *tp)
{
	if( sys.m_TimeOffset || sys.m_TimeOffsetMin ){
//typedef struct _SYSTEMTIME {  /* st */
//    WORD wYear;
//    WORD wMonth;
//    WORD wDayOfWeek;
//    WORD wDay;
//    WORD wHour;
//    WORD wMinute;
//    WORD wSecond;
//    WORD wMilliseconds;
//} SYSTEMTIME;


		LPCSTR	mt;
		int Hour = tp->wHour;
		int Min = tp->wMinute;
		Min += sys.m_TimeOffsetMin;
		if( Min >= 60 ){
			Hour++;
			Min -= 60;
		}
		else if( Min < 0 ){
			Hour--;
			Min += 60;
		}
		tp->wMinute = WORD(Min);
		Hour += sys.m_TimeOffset;
		if( Hour >= 24 ){
			tp->wHour = WORD(Hour - 24);
			tp->wDay++;
			if( tp->wYear % 4 ){
				mt = MONN;
			}
			else {
				mt = MONU;
			}
			if( tp->wDay > mt[tp->wMonth] ){
				tp->wDay = 1;
				tp->wMonth++;
				if( tp->wMonth > 12 ){
					tp->wMonth = 1;
					tp->wYear++;
				}
			}
		}
		else if( Hour < 0 ){
			tp->wHour = WORD(Hour + 24);
			tp->wDay--;
			if( tp->wDay < 1 ){
				tp->wMonth--;
				if( tp->wMonth < 1 ){
					tp->wMonth = 12;
					tp->wYear--;
				}
				if( tp->wYear % 4 ){
					tp->wDay = MONN[tp->wMonth];
				}
				else {
					tp->wDay = MONU[tp->wMonth];
				}
			}
		}
		else {
			tp->wHour = WORD(Hour);
		}
	}
}
//---------------------------------------------------------------------------
void GetUTC(SYSTEMTIME *tp)
{
	::GetSystemTime(tp);
	AddjustOffset(tp);
}
//---------------------------------------------------------------------------
void GetLocal(SYSTEMTIME *tp)
{
	::GetLocalTime(tp);
	AddjustOffset(tp);
}
//---------------------------------------------------------------------------
LPSTR StrDupe(LPCSTR s)
{
	LPSTR p = new char[strlen(s)+1];
	strcpy(p, s);
	return p;
}
//---------------------------------------------------------------------------
void InitColorTable(TColor cl, TColor ch)
{
	int		br = (ch & 0x000000ff);
	int		bg = (ch & 0x0000ff00) >> 8;
	int		bb = (ch & 0x00ff0000) >> 16;
	int		er = (cl & 0x000000ff);
	int		eg = (cl & 0x0000ff00) >> 8;
	int		eb = (cl & 0x00ff0000) >> 16;

	int		brw = er - br + 1;
	int		bgw = eg - bg + 1;
	int		bbw = eb - bb + 1;
	int		r, g, b;

	ColorTable[0] = DWORD( br | (bg<<8) | (bb<<16) );
	int i;
	for( i = 1; i < 127; i++ ){
		r = (br + (i * brw/128)) & 0x000000ff;
		g = (bg + (i * bgw/128)) & 0x000000ff;
		b = (bb + (i * bbw/128)) & 0x000000ff;
		ColorTable[i] = DWORD( r | (g<<8) | (b<<16) );
	}
	ColorTable[127] = DWORD( er | (eg<<8) | (eb<<16) );
}

#if 0
const DEFKEYTBL KEYTBL[]={
	{ VK_F1, "F1" },
	{ VK_F2, "F2" },
	{ VK_F3, "F3" },
	{ VK_F4, "F4" },
	{ VK_F5, "F5" },
	{ VK_F6, "F6" },
	{ VK_F7, "F7" },
	{ VK_F8, "F8" },
	{ VK_F9, "F9" },
	{ VK_F10, "F10" },
	{ VK_F11, "F11" },
	{ VK_F12, "F12" },

	{ VK_LEFT, "←"},
	{ VK_RIGHT, "→"},
	{ VK_UP, "↑"},
	{ VK_DOWN, "↓"},
	{ VK_PRIOR, "PageUp"},
	{ VK_NEXT, "PageDown"},
	{ VK_HOME, "Home" },
	{ VK_END, "End" },
	{ VK_INSERT, "Insert" },
	{ VK_DELETE, "Delete" },
	{ VK_HELP, "Help" },
	{ VK_ESCAPE, "ESC" },


	{ VK_F1 | 0x400, "Shift+F1" },
	{ VK_F2 | 0x400, "Shift+F2" },
	{ VK_F3 | 0x400, "Shift+F3" },
	{ VK_F4 | 0x400, "Shift+F4" },
	{ VK_F5 | 0x400, "Shift+F5" },
	{ VK_F6 | 0x400, "Shift+F6" },
	{ VK_F7 | 0x400, "Shift+F7" },
	{ VK_F8 | 0x400, "Shift+F8" },
	{ VK_F9 | 0x400, "Shift+F9" },
	{ VK_F10 | 0x400, "Shift+F10" },
	{ VK_F11 | 0x400, "Shift+F11" },
	{ VK_F12 | 0x400, "Shift+F12" },

	{ VK_LEFT | 0x400, "Shift+←"},
	{ VK_RIGHT | 0x400, "Shift+→"},
	{ VK_UP | 0x400, "Shift+↑"},
	{ VK_DOWN | 0x400, "Shift+↓"},
	{ VK_PRIOR | 0x400, "Shift+PageUp"},
	{ VK_NEXT | 0x400, "Shift+PageDown"},
	{ VK_HOME | 0x400, "Shift+Home" },
	{ VK_END | 0x400, "Shift+End" },
	{ VK_INSERT | 0x400, "Shift+Insert" },
	{ VK_DELETE | 0x400, "Shift+Delete" },
	{ VK_HELP | 0x400, "Shift+Help" },
	{ VK_ESCAPE | 0x400, "Shift+ESC" },

	{ VK_F1 | 0x100, "Ctrl+F1" },
	{ VK_F2 | 0x100, "Ctrl+F2" },
	{ VK_F3 | 0x100, "Ctrl+F3" },
	{ VK_F4 | 0x100, "Ctrl+F4" },
	{ VK_F5 | 0x100, "Ctrl+F5" },
	{ VK_F6 | 0x100, "Ctrl+F6" },
	{ VK_F7 | 0x100, "Ctrl+F7" },
	{ VK_F8 | 0x100, "Ctrl+F8" },
	{ VK_F9 | 0x100, "Ctrl+F9" },
	{ VK_F10 | 0x100, "Ctrl+F10" },
	{ VK_F11 | 0x100, "Ctrl+F11" },
	{ VK_F12 | 0x100, "Ctrl+F12" },
	{ '1' | 0x100, "Ctrl+1" },
	{ '2' | 0x100, "Ctrl+2" },
	{ '3' | 0x100, "Ctrl+3" },
	{ '4' | 0x100, "Ctrl+4" },
	{ '5' | 0x100, "Ctrl+5" },
	{ '6' | 0x100, "Ctrl+6" },
	{ '7' | 0x100, "Ctrl+7" },
	{ '8' | 0x100, "Ctrl+8" },
	{ '9' | 0x100, "Ctrl+9" },
	{ '0' | 0x100, "Ctrl+0" },
	{ 'A' | 0x100, "Ctrl+A" },
	{ 'B' | 0x100, "Ctrl+B" },
	{ 'C' | 0x100, "Ctrl+C" },
	{ 'D' | 0x100, "Ctrl+D" },
	{ 'E' | 0x100, "Ctrl+E" },
	{ 'F' | 0x100, "Ctrl+F" },
	{ 'G' | 0x100, "Ctrl+G" },
//    { 'H' | 0x100, "Ctrl+H" },
//    { 'I' | 0x100, "Ctrl+I" },
	{ 'K' | 0x100, "Ctrl+K" },
	{ 'L' | 0x100, "Ctrl+L" },
//    { 'M' | 0x100, "Ctrl+M" },
	{ 'N' | 0x100, "Ctrl+N" },
	{ 'O' | 0x100, "Ctrl+O" },
	{ 'P' | 0x100, "Ctrl+P" },
	{ 'Q' | 0x100, "Ctrl+Q" },
	{ 'R' | 0x100, "Ctrl+R" },
	{ 'S' | 0x100, "Ctrl+S" },
	{ 'T' | 0x100, "Ctrl+T" },
	{ 'U' | 0x100, "Ctrl+U" },
	{ 'V' | 0x100, "Ctrl+V" },
	{ 'W' | 0x100, "Ctrl+W" },
	{ 'X' | 0x100, "Ctrl+X" },
	{ 'Y' | 0x100, "Ctrl+Y" },
	{ 'Z' | 0x100, "Ctrl+Z" },

	{ VK_LEFT | 0x100, "Ctrl+←"},
	{ VK_RIGHT | 0x100, "Ctrl+→"},
	{ VK_UP | 0x100, "Ctrl+↑"},
	{ VK_DOWN | 0x100, "Ctrl+↓"},
	{ VK_PRIOR | 0x100, "Ctrl+PageUp"},
	{ VK_NEXT | 0x100, "Ctrl+PageDown"},
	{ VK_HOME | 0x100, "Ctrl+Home" },
	{ VK_END | 0x100, "Ctrl+End" },
	{ VK_INSERT | 0x100, "Ctrl+Insert" },
	{ VK_DELETE | 0x100, "Ctrl+Delete" },
	{ VK_HELP | 0x100, "Ctrl+Help" },
	{ VK_ESCAPE | 0x100, "Ctrl+ESC" },

	{ VK_F1 | 0x200, "Alt+F1" },
	{ VK_F2 | 0x200, "Alt+F2" },
	{ VK_F3 | 0x200, "Alt+F3" },
	{ VK_F4 | 0x200, "Alt+F4" },
	{ VK_F5 | 0x200, "Alt+F5" },
	{ VK_F6 | 0x200, "Alt+F6" },
	{ VK_F7 | 0x200, "Alt+F7" },
	{ VK_F8 | 0x200, "Alt+F8" },
	{ VK_F9 | 0x200, "Alt+F9" },
	{ VK_F10 | 0x200, "Alt+F10" },
	{ VK_F11 | 0x200, "Alt+F11" },
	{ VK_F12 | 0x200, "Alt+F12" },
	{ '1' | 0x200, "Alt+1" },
	{ '2' | 0x200, "Alt+2" },
	{ '3' | 0x200, "Alt+3" },
	{ '4' | 0x200, "Alt+4" },
	{ '5' | 0x200, "Alt+5" },
	{ '6' | 0x200, "Alt+6" },
	{ '7' | 0x200, "Alt+7" },
	{ '8' | 0x200, "Alt+8" },
	{ '9' | 0x200, "Alt+9" },
	{ '0' | 0x200, "Alt+0" },
	{ 'A' | 0x200, "Alt+A" },
	{ 'B' | 0x200, "Alt+B" },
	{ 'C' | 0x200, "Alt+C" },
	{ 'D' | 0x200, "Alt+D" },
	{ 'E' | 0x200, "Alt+E" },
	{ 'F' | 0x200, "Alt+F" },
	{ 'G' | 0x200, "Alt+G" },
	{ 'H' | 0x200, "Alt+H" },
	{ 'I' | 0x200, "Alt+I" },
	{ 'K' | 0x200, "Alt+K" },
	{ 'L' | 0x200, "Alt+L" },
	{ 'M' | 0x200, "Alt+M" },
	{ 'N' | 0x200, "Alt+N" },
	{ 'O' | 0x200, "Alt+O" },
	{ 'P' | 0x200, "Alt+P" },
	{ 'Q' | 0x200, "Alt+Q" },
	{ 'R' | 0x200, "Alt+R" },
	{ 'S' | 0x200, "Alt+S" },
	{ 'T' | 0x200, "Alt+T" },
	{ 'U' | 0x200, "Alt+U" },
	{ 'V' | 0x200, "Alt+V" },
	{ 'W' | 0x200, "Alt+W" },
	{ 'X' | 0x200, "Alt+X" },
	{ 'Y' | 0x200, "Alt+Y" },
	{ 'Z' | 0x200, "Alt+Z" },

	{ VK_LEFT | 0x200, "Alt+←"},
	{ VK_RIGHT | 0x200, "Alt+→"},
	{ VK_UP | 0x200, "Alt+↑"},
	{ VK_DOWN | 0x200, "Alt+↓"},
	{ VK_PRIOR | 0x200, "Alt+PageUp"},
	{ VK_NEXT | 0x200, "Alt+PageDown"},
	{ VK_HOME | 0x200, "Alt+Home" },
	{ VK_END | 0x200, "Alt+End" },
	{ VK_INSERT | 0x200, "Alt+Insert" },
	{ VK_DELETE | 0x200, "Alt+Delete" },
	{ VK_HELP | 0x200, "Alt+Help" },
	{ VK_ESCAPE | 0x200, "Alt+ESC" },
	{ 0, "" },
};
//---------------------------------------------------------------------------
LPCSTR ToDXKey(LPCSTR s)
{
	if( MsgEng && *s ){
		LPSTR		p;
		static char bf[32];
		strcpy(bf, s);
		if( (p = strstr(bf, "←"))!=NULL ){
			strcpy(p, "ArrowLeft");
			return bf;
		}
		else if( (p = strstr(bf, "→"))!=NULL ){
			strcpy(p, "ArrowRight");
			return bf;
		}
		else if( (p = strstr(bf, "↓"))!=NULL ){
			strcpy(p, "ArrowDown");
			return bf;
		}
		else if( (p = strstr(bf, "↑"))!=NULL ){
			strcpy(p, "ArrowUp");
			return bf;
		}
	}
	return s;
}
//---------------------------------------------------------------------------
LPCSTR ToJAKey(LPCSTR s)
{
	if( MsgEng && *s ){
		LPSTR		p;
		static char bf[32];
		strcpy(bf, s);
		if( (p = strstr(bf, "ArrowLeft"))!=NULL ){
			strcpy(p, "←");
			return bf;
		}
		else if( (p = strstr(bf, "ArrowRight"))!=NULL ){
			strcpy(p, "→");
			return bf;
		}
		else if( (p = strstr(bf, "ArrowDown"))!=NULL ){
			strcpy(p, "↓");
			return bf;
		}
		else if( (p = strstr(bf, "ArrowUp"))!=NULL ){
			strcpy(p, "↑");
			return bf;
		}
	}
	return s;
}
//---------------------------------------------------------------------------
LPCSTR GetKeyName(WORD Key)
{
	int i;
	for( i = 0; KEYTBL[i].Key; i++ ){
		if( Key == KEYTBL[i].Key ){
			break;
		}
	}
	return ToDXKey(KEYTBL[i].pName);
}
//---------------------------------------------------------------------------
WORD GetKeyCode(LPCSTR pName)
{
	LPCSTR p = ToJAKey(pName);
	int i;
	for( i = 0; KEYTBL[i].Key; i++ ){
		if( !strcmp(p, KEYTBL[i].pName) ) break;
	}
	return KEYTBL[i].Key;
}
#endif

//---------------------------------------------------------------------------
int SetTBValue(double d, double dmax, int imax)
{
	if( d ){
		int dd = int(sqrt(d * dmax * dmax / imax) + 0.5);
		if( dd > imax ) dd = imax;
		return dd;
	}
	else {
		return 0;
	}
}

double GetTBValue(int d, double dmax, int imax)
{
	return (d * d) * imax/ (dmax * dmax);
}

LPUSTR jstrupr(LPUSTR s)
{
	LPUSTR	p = s;
	int kf;

	for( kf = 0; *p; p++ ){
		if( kf ){
			kf = 0;
		}
		else if( _mbsbtype((unsigned char *)p, 0) == _MBC_LEAD ){
			kf = 1;
		}
		else {
			*p = (unsigned char)toupper(*p);
		}
	}
	return s;
}
/*#$%
===============================================================
	時刻を調整する
---------------------------------------------------------------
	t : 時刻(UTC)
	c : 時差コード
---------------------------------------------------------------
	ローカルタイム
---------------------------------------------------------------
	A-Z
	a-z 	+30min
===============================================================
*/
WORD AdjustRolTimeUTC(WORD tim, char c)
{
	const	char	tdf[]={
/*		A B C D E F G H I J K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z	*/
		1,2,3,4,5,6,7,8,9,9,10,11,12,23,22,21,20,19,18,17,16,15,14,13,12,0
	};
	int	cc;

	cc = toupper(c);
	if( (cc >= 'A')&&(cc <= 'Z') ){
		// JST to UTC
#if 0
		if( tim >= (9*60*30) ){
			tim -= WORD(9 * 60 * 30);
		}
		else {
			tim += WORD(15 * 60 * 30);
		}
#endif
		tim /= WORD(30);
		tim += WORD(tdf[cc-'A'] * 60);
		if( c >= 'a' ) tim += WORD(30);
		while( tim >= (24 * 60) ){
			tim -= WORD(24 * 60);
		}
		tim *= WORD(30);
		if( !tim ) tim++;
	}
	else {
		tim = 0;
	}
	return tim;
}
///----------------------------------------------------------------
///	ウィンドウをクライアントセンターにする
void FormCenter(TForm *tp, int XW, int YW)
{
	int top = (YW - tp->Height)/2;
	int left = (XW - tp->Width)/2;
	if( top < 0 ) top = 0;
	if( left < 0 ) left = 0;
	tp->Top = top;
	tp->Left = left;
}
/*#$%
========================================================
	最後の文字コードを返す
--------------------------------------------------------
	p : 文字列のポインタ
--------------------------------------------------------
	文字コード
--------------------------------------------------------
========================================================
*/
char *lastp(char *p)
{
	if( *p ){
		for( ; *p; p++ );
		p--;
		return(p);
	}
	else {
		return(p);
	}
}
/*#$%
========================================================
	末尾のスペースとＴＡＢを取る
--------------------------------------------------------
	s : 文字列のポインタ
--------------------------------------------------------
	文字列のポインタ
--------------------------------------------------------
========================================================
*/
char *clipsp(char *s)
{
	char	*p;

	if( *s ){
		for( p = lastp(s); p >= s; p-- ){
			if( (*p == ' ')||(*p == 0x09) ){
				*p = 0;
			}
			else {
				break;
			}
		}
	}
	return(s);
}
//---------------------------------------------------------------------------
LPCSTR _strdmcpy(LPSTR t, LPCSTR p, char c)
{
	for( ; *p && (*p != c); p++, t++ ) *t = *p;
	*t = 0;
	if( *p == c ) p++;
	return(p);
}
const char *StrDlmCpy(char *t, const char *p, char Dlm, int len)
{
	const char _tt1[]="[{(｢<";
	const char _tt2[]="]})｣>";
	const char	*pp;
	int			r = FALSE;

	char	Key;
	if( (pp = strchr(_tt2, Dlm))!=NULL ){
		Key = _tt1[pp - _tt2];
	}
	else {
		Key = 0;
	}
	int	f, k;
	for( f = k = 0; *p;  p++ ){
		if( k ){															// 漢字２バイト目
			k = 0;
		}
		else if(  _mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD ){	// 漢字１バイト目
			k = 1;
		}
		else if( *p == Key ){
			f++;
		}
		else if( *p == Dlm ){
			if( f ){
				f--;
			}
			else {
				r = TRUE;
				p++;
				break;
			}
		}
		if( len ){
			*t++ = *p;
			len--;
		}
	}
	*t = 0;
	return (r == TRUE) ? p : NULL;
}

const char *StrDlmCpyK(char *t, const char *p, char Dlm, int len)
{
	const char _tt1[]="[{(｢<";
	const char _tt2[]="]})｣>";
	const char	*pp;
	int			r = FALSE;

	char	Key;
	if( (pp = strchr(_tt2, Dlm))!=NULL ){
		Key = _tt1[pp - _tt2];
	}
	else {
		Key = 0;
	}
	int	f, k;
	for( f = k = 0; *p;  p++ ){
		if( k ){															// 漢字２バイト目
			k = 0;
		}
		else if(  _mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD ){	// 漢字１バイト目
			k = 1;
		}
		else if( (pp = strchr(_tt1, *p))!=NULL ){
			Key = _tt2[pp - _tt1];
			f++;
		}
		else if( f && (*p == Key) ){
			f--;
			Key = 0;
		}
		else if( *p == Dlm ){
			if( !f ){
				r = TRUE;
				p++;
				break;
			}
		}
		if( len ){
			*t++ = *p;
			len--;
		}
	}
	*t = 0;
	return (r == TRUE) ? p : NULL;
}
//---------------------------------------------------------------------------
void StrCopy(LPSTR t, LPCSTR s, int n)
{
	for( ; *s && n; n--, s++, t++){
		*t = *s;
	}
	*t = 0;
}
//---------------------------------------------------------------------------
char LastC(LPCSTR p)
{
	char c = 0;

	if( *p ){
		c = *(p + strlen(p) - 1);
	}
	return c;
}

/*#$%
========================================================
	拡張子を得る
--------------------------------------------------------
	p : 文字列のポインタ
--------------------------------------------------------
	文字コード
--------------------------------------------------------
========================================================
*/
LPCSTR GetEXT(LPCSTR Fname)
{
	if( *Fname ){
		LPCSTR p = Fname + (strlen(Fname) - 1);
		for( ; p > Fname; p-- ){
			if( *p == '.' ) return p+1;
		}
	}
	return "";
}

void SetEXT(LPSTR pName, LPSTR pExt)
{
	if( *pName ){
		LPSTR	p;

		for( p = &pName[strlen(pName)-1]; p >= pName; p-- ){
			if( *p == '.' ){
				strcpy(p, pExt);
				return;
			}
		}
		strcat(pName, pExt);
	}
}

BOOL __fastcall CheckEXT(LPCSTR pName, LPCSTR pExt)
{
	return !strcmpi(GetEXT(pName), pExt);
}

void SetCurDir(LPSTR t, int size)
{
	if( !::GetCurrentDirectory(size-1, t) ){
		*t = 0;
	}
	else {
		if( LastC(t) != '\\' ){
			strcat(t, "\\");
		}
	}
}

void SetDirName(LPSTR t, LPCSTR pName)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char name[_MAX_FNAME];
	char ext[_MAX_EXT];
	AnsiString	Dir;

	::_splitpath( pName, drive, dir, name, ext );
	Dir = drive;
	Dir += dir;
	strncpy(t, Dir.c_str(), 128);
}

LPCSTR StrDbl(double d)
{
	static	char	bf[64];
	LPSTR	p;

	sprintf(bf, "%lf", d);
	for( p = &bf[strlen(bf)-1]; p >= bf; p-- ){
		if( (*p == '0')&&(*(p-1)!='.') ){
			*p = 0;
		}
		else {
			break;
		}
	}
	return bf;
}

LPCSTR StrDblE(double d)
{
	static	char	bf[32];

	sprintf(bf, "%le", d);
	return bf;
}

double ReadDoubleIniFile(TMemIniFile *p, LPCSTR c1, LPCSTR c2, double d)
{
	AnsiString cs = StrDblE(d);
	AnsiString rs = p->ReadString(c1, c2, cs);
	if( sscanf(rs.c_str(), "%lf", &d) != 1 ) d = 0.0;
	return d;
}

void WriteDoubleIniFile(TMemIniFile *p, LPCSTR c1, LPCSTR c2, double d)
{
	AnsiString cs = StrDblE(d);
	p->WriteString(c1, c2, cs);
}

///----------------------------------------------------------------
///  CR/LFを削除
///
void ClipLF(LPSTR sp)
{
	for( ; *sp; sp++ ){
		if( (*sp == LF)||(*sp == CR) ){
			*sp = 0;
			break;
		}
	}
}
///----------------------------------------------------------------
///  コメントを削除（スペースおよびＴＡＢも削除）
///
void DeleteComment(LPSTR bf)
{
	LPSTR sp, tp;

	for( sp = tp = bf; *sp; sp++ ){
		if( (*sp == ';')||(*sp == CR)||(*sp == LF) ){
			break;
		}
		else if( (*sp!=' ')&&(*sp!=TAB) ){
			*tp++ = *sp;
		}
	}
	*tp = 0;
}

LPSTR FillSpace(LPSTR s, int n)
{
	LPSTR p = s;
	int i = 0;
	for( ; *p && (i < n); i++, p++ );
	for( ; i < n; i++, p++ ){
		*p = ' ';
	}
	*p = 0;
	return s;
}
///----------------------------------------------------------------
///  ホワイトスペースのスキップ
///
LPSTR SkipSpace(LPSTR sp)
{
	for( ; *sp; sp++ ){
		if( (*sp != ' ')&&(*sp != TAB) ) break;
	}
	return sp;
}

LPCSTR SkipSpace(LPCSTR sp)
{
	for( ; *sp; sp++ ){
		if( (*sp != ' ')&&(*sp != TAB) ) break;
	}
	return sp;
}

///----------------------------------------------------------------
///  デリミッタ分解を行う
///
LPSTR StrDlm(LPSTR &t, LPSTR p)
{
	return StrDlm(t, p, ',');
}

///----------------------------------------------------------------
///  デリミッタ分解を行う
///
LPSTR StrDlm(LPSTR &t, LPSTR p, char c)
{
	int		f, k;
	LPSTR	d1=NULL;
	LPSTR	d2=NULL;

	t = p;
	f = k = 0;
	while(*p){
		if( k ){															// 漢字２バイト目
			k = 0;
		}
		else if(  _mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD ){	// 漢字１バイト目
			k = 1;
		}
		else if( *p == 0x22 ){
			if( !f ){
				if( d1 == NULL ) d1 = p+1;
				f++;
			}
			else {
				d2 = p;
				f--;
			}
		}
		else if( !f && (*p == c) ){
			*p = 0;
			p++;
			break;
		}
		p++;
	}

	if( (d1!=NULL)&&(d2!=NULL) ){
		if( ((t+1)==d1) && ( ((p-2)==d2)||((p-1)==d2) ) ){
			t = d1;
			*d2 = 0;
		}
	}
	return(p);
}

///----------------------------------------------------------------
///
void ChgString(LPSTR t, char a, char b)
{
	for( ; *t; t++ ){
		if( *t == a ) *t = b;
	}
}

///----------------------------------------------------------------
///
void DelChar(LPSTR t, char a)
{
	for( ; *t; t++ ){
		if( *t == a ){
			strcpy(t, t+1);
			t--;
		}
	}
}

//---------------------------------------------------------------------------
BOOL __fastcall ATOI(int &d, LPCSTR s)
{
	LPCSTR p = SkipSpace(s);
    if( *p == '-' ) p++;
	while(*p){
		if( !isdigit(*p++) ) return FALSE;
    }
    if( sscanf(s, "%d", &d) != 1 ) return FALSE;
	return TRUE;
}

/*#$%
========================================================
	デシマルアスキーを数値に変換する
--------------------------------------------------------
	p : 文字列のポインタ
	n : 変換桁数
--------------------------------------------------------
	数値
--------------------------------------------------------
========================================================
*/
int atoin(const char *p, int n)
{
	int	d;

	for( d = 0; *p && n; p++, n-- ){
		d *= 10;
		d += (*p & 0x0f);
	}
	return(d);
}
/*#$%
========================================================
	１６進アスキーを数値に変換する
--------------------------------------------------------
	p : 文字列のポインタ
	n : 変換桁数
--------------------------------------------------------
	数値
--------------------------------------------------------
========================================================
*/
int htoin(const char *p, int n)
{
	if( *p == 'x' ) return 0;
	int	d;

	for( d = 0; *p && n; p++, n-- ){
		d = d << 4;
		d += (*p & 0x0f);
		if( *p >= 'A' ) d += 9;
	}
	return(d);
}
//---------------------------------------------------------------------------
int InvMenu(TMenuItem *pItem)
{
	pItem->Checked = pItem->Checked ? FALSE : TRUE;
	return pItem->Checked;
}
//---------------------------------------------------------------------------
void AdjustBitmapFormat(Graphics::TBitmap *pBitmap)
{
	if( sys.m_UseB24 ){
		pBitmap->PixelFormat = pf24bit;
	}
	else {
		if( sys.m_Palette ) pBitmap->PixelFormat = pf24bit;
	}
}
//---------------------------------------------------------------------------
Graphics::TBitmap *Bitmap24bit(Graphics::TBitmap *pBitmap)
{
	if( pBitmap->PixelFormat != pf24bit ){
		Graphics::TBitmap *pBmp = DupeBitmap(pBitmap, pf24bit);
		delete pBitmap;
		pBitmap = pBmp;
	}
	return pBitmap;
}
//---------------------------------------------------------------------------
int SaveBitmap24(Graphics::TBitmap *pBitmap, LPCSTR pName)
{
	int r;

	if( pBitmap->PixelFormat != pf24bit ){
		Graphics::TBitmap *pBmp = DupeBitmap(pBitmap, pf24bit);
		r = SaveBitmap(pBmp, pName);
		delete pBmp;
	}
	else {
		r = SaveBitmap(pBitmap, pName);
	}
	return r;
}
//---------------------------------------------------------------------------
int SaveBitmap(Graphics::TBitmap *pBitmap, LPCSTR pName)
{
	CWaitCursor wait;
	try {
		pBitmap->SaveToFile(pName);
	}
	catch(...){
		ErrorFWrite(pName);
		return FALSE;
	}
	return TRUE;
}
//---------------------------------------------------------------------------
int LoadBitmap(Graphics::TBitmap *pBitmap, LPCSTR pName)
{
	if( pBitmap == NULL ) return FALSE;

	CWaitCursor wait;
	try {
		pBitmap->LoadFromFile(pName);
	}
	catch(...){
		ErrorMB( "Could not load from '%s'", pName );
		return FALSE;
	}
	return TRUE;
}
//---------------------------------------------------------------------
void FillBitmap(Graphics::TBitmap *pBitmap, TColor col)
{
	TRect rc;
	rc.Top = 0;
	rc.Left = 0;
	rc.Right = pBitmap->Width;
	rc.Bottom = pBitmap->Height;
	pBitmap->Canvas->Brush->Style = bsSolid;
	pBitmap->Canvas->Brush->Color = col;
	pBitmap->Canvas->FillRect(rc);
}
//---------------------------------------------------------------------------
int SaveJPEG(Graphics::TBitmap *pBitmap, LPCSTR pName)
{
	CWaitCursor wait;
	int r = TRUE;
	int YW = pBitmap->Height;
	int XW = pBitmap->Width;

	image_buffer = new BYTE[YW * XW * 3];
	image_height = YW;
	image_width = XW;
	int f24 = ((pBitmap->PixelFormat == pf24bit) && (pBitmap->Palette==NULL)) ? 1 : 0;
	BYTE *bp = image_buffer;
	int x, y;
	if( f24 ){
		for( y = 0; y < YW; y++ ){
			BYTE *s = (BYTE *)pBitmap->ScanLine[y];
			for( x = 0; x < XW; x++ ){
				*bp++ = *(s + 2);
				*bp++ = *(s + 1);
				*bp++ = *s;
				s += 3;
			}
		}
	}
	else {
		COLD  c;
		for( y = 0; y < YW; y++ ){
			for( x = 0; x < XW; x++ ){
				c.c = pBitmap->Canvas->Pixels[x][y];
				*bp++ = c.b.r;
				*bp++ = c.b.g;
				*bp++ = c.b.b;
			}
		}
	}
	if( !write_JPEG_file(pName, sys.m_JPEGQuality) ){
		ErrorFWrite(pName);
		r = FALSE;
	}
	delete image_buffer;
	image_buffer = NULL;
	return r;
}
//---------------------------------------------------------------------
int SaveImage(Graphics::TBitmap *pBitmap, LPCSTR pName)
{
	LPCSTR pExt = GetEXT(pName);
	if( !strcmpi(pExt, "BMP") ){
		return SaveBitmap24(pBitmap, pName);
	}
	else if( !strcmpi(pExt, "JPG") ){
		return SaveJPEG(pBitmap, pName);
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------------
static Graphics::TBitmap *pBitmapLoad;
static int JpegPlane;
void SetJPEGSize(int height, int width, int plane)
{
	if( pBitmapLoad == NULL ) return;
	pBitmapLoad->Height = height;
	pBitmapLoad->Width = width;
	JpegPlane = plane;
}

void SetJPEGImage(BYTE *p, int line)
{
	if( pBitmapLoad == NULL ) return;
	if( !line ) return;
	if( !(line & 0x007f) ) MultProcA();

	int x;

	BYTE *t = (BYTE *)pBitmapLoad->ScanLine[line-1];
	if( JpegPlane == 1 ){
		for( x = 0; x < pBitmapLoad->Width; x++ ){
			*t++ = *p;
			*t++ = *p;
			*t++ = *p++;
		}
	}
	else if( JpegPlane >= 3 ){
		for( x = 0; x < pBitmapLoad->Width; x++ ){
			*t++ = *(p+2);
			*t++ = *(p+1);
			*t++ = *p;
			p+=3;
		}
	}
}
//---------------------------------------------------------------------------
int LoadJPEG(Graphics::TBitmap *pBitmap, LPCSTR pName)
{
	if( pBitmap == NULL ) return FALSE;

	CWaitCursor wait;
	pBitmapLoad = pBitmap;
	pBitmapLoad->PixelFormat = pf24bit;
	pBitmapLoad->Palette = NULL;
	return read_JPEG_file(pName) ? TRUE : FALSE;
}
//---------------------------------------------------------------------------
int LoadWMF(Graphics::TBitmap *pBitmap, LPCSTR pName)
{
	if( pBitmap == NULL ) return FALSE;

	CWaitCursor wait;

	int r = FALSE;
	TMetafile *pMeta = new TMetafile;
	try {
		pMeta->Enhanced = FALSE;
		pMeta->LoadFromFile(pName);
		pBitmap->PixelFormat = pf24bit;
		pBitmap->Width = pMeta->Width;
		pBitmap->Height = pMeta->Height;
		if( pMeta->Transparent ) FillBitmap(pBitmap, clBtnFace);
		pBitmap->Canvas->Draw(0, 0, pMeta);
		r = TRUE;
	}
	catch(...){
		ErrorMB( "Could not load from '%s'", pName );
	}
	delete pMeta;
	return r;
}
//---------------------------------------------------------------------
int IsPic(LPCSTR pExt)
{
	return (!strcmpi(pExt, "BMP") || !strcmpi(pExt, "JPG") || !strcmpi(pExt, "WMF"));
}

LPCSTR GetPicFilter(void)
{
	return MsgEng ? "Picture files(*.jpg;*.bmp;*.wmf)|*.jpg;*.bmp;*.wmf" : "画像ファイル(*.jpg;*.bmp;*.wmf)|*.jpg;*.bmp;*.wmf";
}
LPCSTR GetTempMFilter(void)
{
	return MsgEng ? "Templates(*.mtm)|*.mtm" : "テンプレート(*.mtm)|*.mtm";
}
LPCSTR GetTempIFilter(void)
{
	return MsgEng ? "Template items(*.mti)|*.mti" : "テンプレートアイテム(*.mti)|*.mti";
}
LPCSTR GetTempFilter(void)
{
	return MsgEng ? "Templates(*.mtm)|*.mtm|Template items(*.mti)|*.mti" : "テンプレート(*.mtm)|*.mtm|テンプレートアイテム(*.mti)|*.mti";
}
LPCSTR GetLibFilter(void)
{
	return MsgEng ? "Custom items(*.dll)|*.dll" : "カスタムアイテム(*.dll)|*.dll";
}
//---------------------------------------------------------------------
int LoadImage(Graphics::TBitmap *pBitmap, LPCSTR pName)
{
	LPCSTR pExt = GetEXT(pName);
	if( !strcmpi(pExt, "BMP") ){
		return LoadBitmap(pBitmap, pName);
	}
	else if( !strcmpi(pExt, "JPG") ){
		return LoadJPEG(pBitmap, pName);
	}
	else if( !strcmpi(pExt, "WMF") ){
		return LoadWMF(pBitmap, pName);
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------
void InitCustomColor(TColorDialog *tp)
{
	tp->CustomColors->Text = "";
}
//---------------------------------------------------------------------
void AddCustomColor(TColorDialog *tp, TColor col)
{
	char bf[256];

	sprintf(bf, "Color%c=%06lX", tp->CustomColors->Count + 'A', DWORD(col) & 0x00ffffff);
	tp->CustomColors->Add(bf);
}
///----------------------------------------------------------------
HWND GetMBHandle(int &flag)
{
	HWND hd;
	if( Screen->ActiveForm != NULL ){
		hd = Screen->ActiveForm->Handle;
	}
	else {
		hd = NULL;
	}
	flag = MB_SETFOREGROUND;
	flag |= MB_TOPMOST;
	flag |= (hd == NULL) ? MB_TASKMODAL : MB_APPLMODAL;
	return hd;
}
static int MsgBoxFlag = 0;
///----------------------------------------------------------------
///  メッセージの表示
///
///----------------------------------------------------------------
///  メッセージの表示
///
void InfoMB(LPCSTR fmt, ...)
{
	if( MsgBoxFlag ) return;
	MsgBoxFlag++;
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);

	int	flag;
	HWND hd = GetMBHandle(flag);
	SetDisPaint();
	flag |= (MB_OK | MB_ICONINFORMATION);
//	Application->NormalizeTopMosts();
	::MessageBox(hd, bf, "MMSSTV", flag);
//	Application->RestoreTopMosts();
	ResDisPaint();
	MsgBoxFlag--;
}
///----------------------------------------------------------------
///  エラーメッセージの表示
///
void ErrorMB(LPCSTR fmt, ...)
{
	if( MsgBoxFlag ) return;
	MsgBoxFlag++;
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);

	int	flag;
	HWND hd = GetMBHandle(flag);
	SetDisPaint();
	flag |= (MB_OK | MB_ICONEXCLAMATION);
//	Application->NormalizeTopMosts();
	::MessageBox(hd, bf, MsgEng ? "Error":"ｴﾗｰ", flag);
//	Application->RestoreTopMosts();
	ResDisPaint();
	MsgBoxFlag--;
}

///----------------------------------------------------------------
///  警告メッセージの表示
///
void WarningMB(LPCSTR fmt, ...)
{
	if( MsgBoxFlag ) return;
	MsgBoxFlag++;
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);

	int	flag;
	HWND hd = GetMBHandle(flag);
	SetDisPaint();
	flag |= (MB_OK | MB_ICONEXCLAMATION);
//	Application->NormalizeTopMosts();
	::MessageBox(hd, bf, MsgEng?"Warning":"警告", flag);
//	Application->RestoreTopMosts();
	ResDisPaint();
	MsgBoxFlag--;
}

///----------------------------------------------------------------
///  実行選択メッセージの表示
///
int YesNoMB(LPCSTR fmt, ...)
{
	if( MsgBoxFlag ) return IDNO;
	MsgBoxFlag++;
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);

	int	flag;
	HWND hd = GetMBHandle(flag);
	SetDisPaint();
//	if( Screen->ActiveForm != NULL ) NormalWindow(Screen->ActiveForm);
	flag |= (MB_YESNO | MB_ICONQUESTION);
//	Application->NormalizeTopMosts();
	int r = ::MessageBox(hd, bf, "MMSSTV", flag);
//	Application->RestoreTopMosts();
	ResDisPaint();
	MsgBoxFlag--;
	return r;
}

///----------------------------------------------------------------
///  実行選択メッセージの表示
///
int YesNoCancelMB(LPCSTR fmt, ...)
{
	if( MsgBoxFlag ) return IDCANCEL;
	MsgBoxFlag++;
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);

	int	flag;
	HWND hd = GetMBHandle(flag);
	SetDisPaint();
	flag |= (MB_YESNOCANCEL | MB_ICONQUESTION);
//	Application->NormalizeTopMosts();
	int r = ::MessageBox(hd, bf, "MMSSTV", flag);
//	Application->RestoreTopMosts();
	ResDisPaint();
	MsgBoxFlag--;
	return r;
}
///----------------------------------------------------------------
///  実行選択メッセージの表示
///
int OkCancelMB(LPCSTR fmt, ...)
{
	if( MsgBoxFlag ) return IDCANCEL;
	MsgBoxFlag++;
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);

	int	flag;
	HWND hd = GetMBHandle(flag);
	SetDisPaint();
	flag |= (MB_OKCANCEL | MB_ICONQUESTION);
//	Application->NormalizeTopMosts();
	int r = ::MessageBox(hd, bf, "MMSSTV", flag);
//	Application->RestoreTopMosts();
	ResDisPaint();
	MsgBoxFlag--;
	return r;
}
///----------------------------------------------------------------
///
void ErrorFWrite(LPCSTR pName)
{
	DWORD dw = ::GetFileAttributes(pName);
	if( (dw != 0xffffffff) && (dw & FILE_ATTRIBUTE_READONLY) ){
		ErrorMB(MsgEng ? "'%s' is read-only (fail to update)." : "'%s'が 読み取り専用 に設定されているため更新に失敗しました.", pName);
	}
	else {
		ErrorMB(MsgEng ? "Could not update '%s'" : "'%s' の更新に失敗しました.", pName );
	}
}
///----------------------------------------------------------------
///
int RemoveL2(LPSTR t, LPSTR ss, LPCSTR pKey, int size)
{
	char	c;
	int		k;
	LPCSTR	pp;
	LPSTR	s;
	const char	_tt1[]="[{(｢<";
	const char	_tt2[]="]})｣>";

	int		len = strlen(pKey);
	char ac = ';';
	for( k = 0, s = ss; *s; s++ ){
		if( k ){															// 漢字２バイト目
			k = 0;
			ac = 0x1e;
		}
		else if(  _mbsbtype((const unsigned char *)s, 0) == _MBC_LEAD ){	// 漢字１バイト目
			k = 1;
			if( (len >= 2) && (strchr(" ,./;:*\t[{(｢<]})｣>", ac)!=NULL) && (!strnicmp(s, pKey, len)) ){
				pp = s + len;
				if( (pp = strchr(_tt1, *pp))!=NULL ){
					c = _tt2[pp - _tt1];
					if( *(s+len+1) ){
						StrDlmCpy(t, s+len+1, c, size);
						if( *t ){
							strcpy((ac == ' ')?(s-1):s, s+len+2+strlen(t));
							return TRUE;
						}
					}
				}
			}
		}
		else if( (strchr(" ,./;:*\t\x1e[{(｢<]})｣>", ac)!=NULL) && (!strnicmp(s, pKey, len)) ){
			pp = s + len;
			if( (pp = strchr(_tt1, *pp))!=NULL ){
				c = _tt2[pp - _tt1];
				if( *(s+len+1) ){
					StrDlmCpy(t, s+len+1, c, size);
					if( *t ){
						strcpy((ac == ' ')?(s-1):s, s+len+2+strlen(t));
						return TRUE;
					}
				}
			}
			ac = *s;
		}
		else {
			ac = *s;
		}
	}
	return FALSE;
}

void AddL2(LPSTR t, LPCSTR pKey, LPCSTR s, UCHAR c1, UCHAR c2, int size)
{
	if( *s ){
		int len = strlen(t);
		int lenkey = strlen(pKey);
		int lenData = strlen(s);
		if( (len + lenkey + lenData + 2) < size ){
			t += len;
			if( len ) *t++ = ' ';
			if( *pKey ){
				strcpy(t, pKey);
				t += lenkey;
			}
			*t++ = c1;
			strcpy(t, s);
			t += lenData;
			*t++ = c2;
			*t = 0;
		}
	}
}
///----------------------------------------------------------------
///  数字が含まれるかどうか調べる
///
int IsNumbs(LPCSTR p)
{
	for( ; *p; p++ ){
		if( isdigit(*p) ) return 1;
	}
	return 0;
}
///----------------------------------------------------------------
///  数字が含まれるかどうか調べる
///
int IsAlphas(LPCSTR p)
{
	for( ; *p; p++ ){
		if( isalpha(*p) ) return 1;
	}
	return 0;
}
///----------------------------------------------------------------
///  RSTかどうか調べる
///
int IsRST(LPCSTR p)
{
	if( strlen(p) < 3 ) return 0;
	if( (*p < '1') || (*p > '5') ) return 0;
	p++;
	if( (*p < '1') || (*p > '9') ) return 0;
	p++;
	if( (*p < '1') || (*p > '9') ) return 0;
	return 1;
}
///----------------------------------------------------------------
int IsCallChar(char c)
{
	if( !isalpha(c) && !isdigit(c) && (c != '/') ) return 0;
	if( islower(c) ) return 0;
	return 1;
}
///----------------------------------------------------------------
///  コールサインかどうか調べる
///
int IsCall(LPCSTR p)
{
	int l = strlen(p);
	if( l > 16 ) return 0;
	if( l < 3 ) return 0;
	if( isdigit(*p) ){					// 先頭が数字
		if( l <= 3 ) return 0;				// 3文字以下の時NG
		if( isdigit(*(p+1)) ) return 0;		// 2文字目が数字の時NG
	}
	if( isdigit(LastC(p)) ){			// 最後が数字
		if( l <= 4 ) return 0;				// ４文字以下の時NG
//      if( !strchr(p, '/')==NULL ) return0;	// /が含まれていない時NG
//		if( p[l-2] != '/' ) return 0;		// 最後の１つ前が/以外の時NG
	}
	int i;
	for( i = 0; i < l-1; i++, p++ ){
		if( isdigit(*p) ) return 1;		// 数字が含まれていればOK
	}
	return 0;
}
/*#$%
======================================================
	ＪＡとＤＸの区別をする
------------------------------------------------------
	s : コールサイン文字列のポインタ
------------------------------------------------------
	1 : JA
	0 : DX (JD1 INCLUDEED)
------------------------------------------------------
======================================================
*/
int IsJA(const char *s)
{
	if( (!memcmp(s, "8J1R", 4) && (strlen(s) == 5))||(strstr(s, "/JD1")!=NULL) ){
		return(0);
	}
	else if( strchr(s, '/') ){
		char	bf[MLCALL+1];
		StrCopy(bf, s, MLCALL);
		char	*p;
		char	*t;
		for( p = bf; *p; ){
			p = StrDlm(t, p, '/');
			if( *t ){
				if( (strlen(t) >= 2) && isdigit((unsigned char)LastC(t)) ){
					if( *t == 'J' ){
						t++;
						if( *t == 'D' ) return(0);
						if( (*t >= 'A')&&(*t <= 'S' ) ) return(1);
					}
					else if( *t == '7' ){
						t++;
						if( (*t >= 'J')&&(*t <= 'N' ) ) return(1);
					}
					else if( *t == '8' ){
						t++;
						if( (*t >= 'J')&&(*t <= 'N' ) ) return(1);
					}
					else {
						return 0;
					}
				}
			}
		}
	}
	if( *s == 'J' ){
		s++;
		if( *s == 'D' ) return(0);
		if( (*s >= 'A')&&(*s <= 'S' ) ) return(1);
	}
	else if( *s == '7' ){
		s++;
		if( (*s >= 'J')&&(*s <= 'N' ) ) return(1);
	}
	else if( *s == '8' ){
		s++;
		if( (*s >= 'J')&&(*s <= 'N' ) ) return(1);
	}
	return(0);
}
/*#$%
======================================================
	コールサインをクリップする
------------------------------------------------------
	s : コールサイン文字列のポインタ
------------------------------------------------------
	クリップコールのポインタ
------------------------------------------------------
======================================================
*/
LPCSTR ClipCall(LPCSTR s)
{
	static	char	bf[MLCALL+1];
	LPCSTR	p1, p2;

	if( (p1=strchr(s, '/'))!=NULL ){
		if( (p2=strchr(p1+1, '/'))!=NULL ){	/* ３分割	*/
			if( (int(strlen(p2+1)) < int((p2 - p1)+1)) || (!IsCall(p2+1)) ){		/* 最後より途中が長い	*/
				if( ((p2-p1) < (p1-s))||(!IsCall(p1+1)) ){	/* 途中より最初が長い	*/
					StrCopy(bf, s, MLCALL);
					*strchr(bf, '/') = 0;
					return(bf);
				}
				else {
					strcpy(bf, p1+1);
					*strchr(bf, '/') = 0;
					return(bf);
				}
			}
			else if( int(strlen(p2+1)) < int((p1 - s)+1) ){	/* 最後より最初が長い	*/
				StrCopy(bf, s, MLCALL);
				*strchr(bf, '/') = 0;
				return(bf);
			}
			else {
				return(p2+1);
			}
		}
		else {								/* ２分割	*/
			if( (int(strlen(p1+1)) < int((p1 - s)+1)) || (!IsCall(p1+1)) ){
				StrCopy(bf, s, MLCALL);
				*strchr(bf, '/') = 0;
				return(bf);
			}
			else {
				return(p1+1);
			}
		}
	}
	else {
		return(s);
	}
}

/*#$%
======================================================
	ポータブル表記を調べる
------------------------------------------------------
	p : コールサイン文字列のポインタ
------------------------------------------------------
	数値のポインタ
------------------------------------------------------
======================================================
*/
static LPSTR chkptb(LPSTR p)
{
	if( *p ){
		p = lastp(p);
		if( isdigit(*p) && ((*(p-1))=='/') ){
			return(p);
		}
	}
	return(NULL);
}

/*#$%
======================================================
	ポータブル表記の入れ替え
------------------------------------------------------
	s : コールサイン文字列のポインタ
------------------------------------------------------
------------------------------------------------------
	元の文字列を破壊する
======================================================
*/
void chgptb(LPSTR s)
{
	LPSTR	p, t;

	if( (p = chkptb(s))!=NULL ){
		t = p;
		for( p--; p > s; p-- ){
			if( isdigit(*p) ){
				*p = *t;
				t--;
				*t = 0;
				break;
			}
		}
	}
}

/*#$%
======================================================
	コールサインからカントリの元をクリップする
------------------------------------------------------
	s : コールサイン文字列のポインタ
------------------------------------------------------
	カントリのポインタ
------------------------------------------------------
======================================================
*/
LPCSTR ClipCC(LPCSTR s)
{
	static	char	bf[MLCALL+1];
	LPSTR	p, t;

	StrCopy(bf, s, MLCALL);
	chgptb(bf);			/* ポータブル表記の入れ替え	*/
	for( p = bf; *p; ){
		if( *p ){
			p = StrDlm(t, p, '/');
			if( (strlen(t) > 1) && (isdigit(*t) || isdigit(LastC(t))) ) return(t);
		}
	}
	for( p = bf; *p; ){
		if( *p ){
			p = StrDlm(t, p, '/');
			if( (strlen(t) > 1) && (*t!='Q') && strcmp(t, "MM") ) return(t);
		}
	}
	return ClipCall(s);
}

///----------------------------------------------------------------
///  文字列変換
///
void Yen2CrLf(AnsiString &ws, AnsiString cs)
{
	ws = "";
	LPCSTR	p;
	int		f;
	int		dlm = 0;

	p = cs.c_str();
	if( *p == 0x22 ){
		p++;
		dlm++;
	}
	for( f = 0; *p; p++ ){
		if( f ){
			f = 0;
			ws += *p;
		}
		else if(  _mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD ){
			f = 1;
			ws += *p;
		}
		else if( *p == '\\' ){
			switch(*(p+1)){
			case 'r':
				ws += "\r";
				p++;
				break;
			case 'n':
				ws += "\n";
				p++;
				break;
			case 't':
				ws += "\t";
				p++;
				break;
			case '\\':
				ws += "\\";
				p++;
				break;
			default:
				p++;
				ws += *p;
				break;
			}
		}
		else if( !dlm || (*p!=0x22) || *(p+1) ){
			ws += *p;
		}
	}
}

void CrLf2Yen(AnsiString &ws, AnsiString cs)
{
	ws = "\x22";
	LPCSTR	p;
	int f = 0;

	for( p = cs.c_str(); *p; p++ ){
		if( f ){
			f = 0;
			ws += *p;
		}
		else if(  _mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD ){
			f = 1;
			ws += *p;
		}
		else if( *p == 0x0d ){
			ws += "\\r";
		}
		else if( *p == 0x0a ){
			ws += "\\n";
		}
		else if( *p == TAB ){
			ws += "\\t";
		}
		else if( *p == '\\' ){
			ws += "\\\\";
		}
		else {
			ws += *p;
		}
	}
	ws += "\x22";
}


//---------------------------------------------------------------------------
// コンボＢＯＸに文字列を設定する
void SetComboBox(TComboBox *pCombo, LPCSTR pList)
{
	pCombo->Clear();
	LPSTR s = strdup(pList);
	LPSTR p = s;
	LPSTR t;
	while(*p){
		p = StrDlm(t, p);
		clipsp(t);
		t = SkipSpace(t);
		if( *t ) pCombo->Items->Add(t);
	}
	free(s);
}

//---------------------------------------------------------------------------
// コンボＢＯＸに文字列を設定する
void GetComboBox(AnsiString &as, TComboBox *pCombo)
{
	as = "";
	int i;
	for( i = 0; i < pCombo->Items->Count; i++ ){
		if( i ) as += ',';
		as += pCombo->Items->Strings[i];
	}
}

//---------------------------------------------------------------------------
void ExecPB(LPCSTR pName)
{
	char bf[512];

	MultProc();
	strcpy(bf, "mspaint.exe");
	if( pName != NULL ){
		sprintf(lastp(bf)+1, " \x22%s\x22", pName);
	}
	if( ::WinExec(bf, SW_SHOWDEFAULT) <= 31 ){
		strcpy(bf, "PBRUSH.EXE");
		if( pName != NULL ){
			sprintf(lastp(bf)+1, " \x22%s\x22", pName);
		}
		::WinExec(bf, SW_SHOWDEFAULT);
	}
}
//---------------------------------------------------------------------------
int FontStyle2Code(TFontStyles style)
{
	int code = 0;

	TFontStyles fa;
	TFontStyles fb;

	fa << fsBold;
	fb << fsBold;
	fa *= style;
	if( fa == fb ) code |= FSBOLD;

	fa >> fsBold;
	fb >> fsBold;
	fa << fsItalic;
	fb << fsItalic;
	fa *= style;
	if( fa == fb ) code |= FSITALIC;

	fa >> fsItalic;
	fb >> fsItalic;
	fa << fsUnderline;
	fb << fsUnderline;
	fa *= style;
	if( fa == fb ) code |= FSUNDERLINE;

	fa >> fsUnderline;
	fb >> fsUnderline;
	fa << fsStrikeOut;
	fb << fsStrikeOut;
	fa *= style;
	if( fa == fb ) code |= FSSTRIKEOUT;

	return code;
}

TFontStyles Code2FontStyle(int code)
{
	TFontStyles fs;

	if( code & FSBOLD ) fs << fsBold;
	if( code & FSITALIC ) fs << fsItalic;
	if( code & FSUNDERLINE ) fs << fsUnderline;
	if( code & FSSTRIKEOUT ) fs << fsStrikeOut;
	return fs;
}

//---------------------------------------------------------------------------
void TopWindow(TForm *tp)
{
	if( tp->FormStyle == fsStayOnTop ){
		::SetWindowPos(tp->Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	}
}
void NormalWindow(TForm *tp)
{
	if( tp->FormStyle == fsStayOnTop ){
		::SetWindowPos(tp->Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	}
}
///---------------------------------------------------------
///  テキスト文字列ストリーマー
int __fastcall CTextString::LoadText(LPSTR tp, int len)
{
	char c;
	int n = 0;
	if( !(*rp) ) return FALSE;
	while(*rp){
		c = *rp++;
		if( c == LF ){
			*tp = 0;
			return TRUE;
		}
		else if( (c != CR)&&(c != 0x1a) ){
			if( n < (len-1) ){
				*tp++ = c;
				n++;
			}
		}
	}
	*tp = 0;
	return TRUE;
}
///----------------------------------------------------------------
///  CAlignクラス
void CAlign::InitControl(TControl *p, TControl *pB, TFont *pF /*= NULL*/)
{
	tp = p;
	OTop = p->Top;
	OLeft = p->Left;
	OWidth = p->Width;
	OHeight = p->Height;
	if( pF != NULL ){
		fp = pF;
		OFontHeight = pF->Height;
	}

	BTop = pB->Top;
	BLeft = pB->Left;
	BWidth = pB->ClientWidth;
	BHeight = pB->ClientHeight;
}

void CAlign::InitControl(TControl *p, RECT *rp, TFont *pF /*= NULL*/)
{
	tp = p;
	OTop = p->Top;
	OLeft = p->Left;
	OWidth = p->Width;
	OHeight = p->Height;
	if( pF != NULL ){
		fp = pF;
		OFontHeight = pF->Height;
	}

	BTop = rp->top;
	BLeft = rp->left;
	BWidth = rp->right - rp->left + 1;
	BHeight = rp->bottom - rp->top + 1;
}

void CAlign::NewAlign(TControl *pB)
{
	double Sx = double(pB->ClientWidth)/double(BWidth);
	double Sy = double(pB->ClientHeight)/double(BHeight);
	tp->SetBounds(int(OLeft * Sx), int(OTop * Sy), int(OWidth * Sx), int(OHeight * Sy));
	if( fp != NULL ){
		if( Sx > Sy ) Sx = Sy;
		fp->Height = int(OFontHeight * Sx) - sys.m_FontAdjSize;
		m_FontAdj = Sx;
	}
	if( tp->ClassNameIs("TComboBox") ){
		((TComboBox *)tp)->SelLength = 0;
	}
	else if( tp->ClassNameIs("TLabel") ){
		TLabel *lp = ((TLabel *)tp);
		if( lp->Alignment == taRightJustify ){
			lp->AutoSize = FALSE;
			lp->AutoSize = TRUE;
		}
	}
}

void CAlign::NewAlign(TControl *pB, double hx)
{
	double Sx = double(pB->ClientWidth)/double(BWidth);
	double Sy = double(pB->ClientHeight)/double(BHeight);
	Sy *= hx;
	tp->SetBounds(int(OLeft * Sx), int(OTop * Sy), int(OWidth * Sx), int(OHeight * Sy));
	if( fp != NULL ){
		if( Sx > Sy ) Sx = Sy;
		fp->Height = int(OFontHeight * Sx) - sys.m_FontAdjSize;
		m_FontAdj = Sx;
	}
	if( tp->ClassNameIs("TComboBox") ){
		((TComboBox *)tp)->SelLength = 0;
	}
}

void CAlign::NewFont(AnsiString &FontName, BYTE Charset, TFontStyles fs)
{
	if( fp != NULL ){
		fp->Name = FontName;
		fp->Charset = Charset;
		fp->Style = fs;
		if( tp->ClassNameIs("TComboBox") ){
			((TComboBox *)tp)->SelLength = 0;
		}
	}
}

void CAlign::NewFixAlign(TControl *pB, int XR)
{
	double Sx = double(pB->ClientWidth - XR) / double(BWidth - XR);

	tp->SetBounds(XR + (OLeft - XR) * Sx, OTop, OWidth * Sx, OHeight);
	if( fp != NULL ){
		fp->Height = int(OFontHeight * (Sx < 1.0 ? Sx : 1.0)) - sys.m_FontAdjSize;
		m_FontAdj = Sx;
	}
	if( tp->ClassNameIs("TComboBox") ){
		((TComboBox *)tp)->SelLength = 0;
	}
}

void CAlign::Resume(void)
{
	if( tp != NULL ){
		tp->Top = OTop;
		tp->Left = OLeft;
		tp->Width = OWidth;
		tp->Height = OHeight;
		if( fp != NULL ){
			fp->Height = OFontHeight;
		}
	}
}
///----------------------------------------------------------------
///  CAlignListクラス
CAlignList::CAlignList(void)
{
	Max = 0;
	Cnt = 0;
	AlignList = NULL;
}

CAlignList::~CAlignList(){
	if( AlignList != NULL ){
		for( int i = 0; i < Cnt; i++ ){
			delete AlignList[i];
		}
		delete AlignList;
		AlignList = NULL;
	}
}

void CAlignList::Alloc(void)
{
	if( Cnt >= Max ){
		Max = Max ? Max * 2 : 16;
		CAlign **np = (CAlign **)new BYTE[sizeof(CAlign*) * Max];
		if( AlignList != NULL ){
			for( int i = 0; i < Cnt; i++ ){
				np[i] = AlignList[i];
			}
			delete AlignList;
		}
		AlignList = np;
	}
	AlignList[Cnt] = new CAlign;
}
void CAlignList::EntryControl(TControl *tp, TControl *pB, TFont *pF /*= NULL*/)
{
	Alloc();
	AlignList[Cnt]->InitControl(tp, pB, pF);
	Cnt++;
}

void CAlignList::EntryControl(TControl *tp, RECT *rp, TFont *pF /*= NULL*/){
	Alloc();
	AlignList[Cnt]->InitControl(tp, rp, pF);
	Cnt++;
}

void CAlignList::EntryControl(TControl *tp, int XW, int YW, TFont *pF /*= NULL*/){
	RECT rc;
	rc.left = rc.top = 0;
	rc.right = XW; rc.bottom = YW;
	Alloc();
	AlignList[Cnt]->InitControl(tp, &rc, pF);
	Cnt++;
}

void CAlignList::NewAlign(TControl *pB)
{
	if( AlignList == NULL ) return;
	for( int i = 0; i < Cnt; i++ ){
		AlignList[i]->NewAlign(pB);
	}
}

double CAlignList::GetFontAdj(TControl *pB)
{
	for( int i = 0; i < Cnt; i++ ){
		if( AlignList[i]->GetControl() == pB ){
			return AlignList[i]->GetFontAdj();
		}
	}
	return 1.0;
}

void CAlignList::NewAlign(TControl *pB, TControl *pS, double hx)
{
	for( int i = 0; i < Cnt; i++ ){
		if( AlignList[i]->GetControl() == pS ){
			AlignList[i]->NewAlign(pB, hx);
			break;
		}
	}
}
void CAlignList::NewFont(AnsiString &FontName, BYTE Charset, TFontStyles fs)
{
	if( AlignList == NULL ) return;
	for( int i = 0; i < Cnt; i++ ){
		AlignList[i]->NewFont(FontName, Charset, fs);
	}
}

void CAlignList::NewFixAlign(TControl *pB, int XR)
{
	if( AlignList == NULL ) return;
	for( int i = 0; i < Cnt; i++ ){
		AlignList[i]->NewFixAlign(pB, XR);
	}
}

void CAlignList::Resume(TControl *pB)
{
	if( AlignList == NULL ) return;
	for( int i = 0; i < Cnt; i++ ){
		if( pB == NULL ){
			AlignList[i]->Resume();
		}
		else if( pB == AlignList[i]->GetControl() ){
			AlignList[i]->Resume();
			break;
		}
	}
}

///----------------------------------------------------------------
///  CAlignGridクラス
void CAlignGrid::InitGrid(TStringGrid *p)
{
	BWidth = p->Width;
	BHeight = p->Height;
	BRowHeight = p->DefaultRowHeight;
	Max = p->ColCount;
	bp = new int[Max];
	for( int i = 0; i < Max; i++ ){
		bp[i] = p->ColWidths[i];
	}
}

void CAlignGrid::NewAlign(TStringGrid *p)
{
	double Sx = double(p->Width)/double(BWidth);
	double Sy = double(p->Height)/double(BHeight);
	p->DefaultRowHeight = int(BRowHeight * Sy);
	for( int i = 0; (i < Max) && (i < p->ColCount); i++ ){
		p->ColWidths[i] = int(bp[i] * Sx);
	}
}


///------------------------------------------------------
///
///CWebRef クラス
///
void MakeCommand(LPSTR t, LPCSTR s, LPCSTR p)
{
	for( ;*s; s++ ){
		if( *s == '%' ){
			s++;
			if( *s == '%' ){
				*t++ = '%';
			}
			else if( (p != NULL) && (*s == '1') ){
				strcpy(t, p);
				t += strlen(t);
				p = NULL;
			}
		}
		else {
			*t++ = *s;
		}
	}
	*t = 0;
	if( p != NULL ){
		*t++ = ' ';
		strcpy(t, p);
	}
}

CWebRef::CWebRef()
{
	HTML = "";

	HKEY hkey=NULL;	/* オープン キーのハンドル	*/

	char    bf[512], name[512];
	ULONG   namelen, len;
	if( !RegOpenKeyEx(HKEY_CLASSES_ROOT, "http", 0, KEY_READ, &hkey) ){
		if( !RegOpenKeyEx(hkey, "shell", 0, KEY_READ, &hkey) ){
			if( !RegOpenKeyEx(hkey, "open", 0, KEY_READ, &hkey) ){
				if( !RegOpenKeyEx(hkey, "command", 0, KEY_READ, &hkey) ){
					namelen = sizeof(name);
					len = sizeof(bf);
					if( !::RegEnumValue(hkey, 0, name, &namelen, NULL,
					NULL, (LPBYTE)bf, &len) ){
						HTML = bf;
					}
				}
			}
		}
	}
	::RegCloseKey(hkey);
}

void CWebRef::ShowHTML(LPCSTR url)
{
	char    cmd[1024];
	MakeCommand(cmd, HTML.c_str(), url);
	::WinExec(cmd, SW_SHOWDEFAULT);
}


///------------------------------------------------------
///
///CWaitCursor クラス
///
CWaitCursor::CWaitCursor()
{
	sv = Screen->Cursor;
	Screen->Cursor = crHourGlass;
}

CWaitCursor::~CWaitCursor()
{
	Screen->Cursor = sv;
}

void CWaitCursor::Delete(void)
{
	Screen->Cursor = sv;
}

void CWaitCursor::Wait(void)
{
	Screen->Cursor = crHourGlass;
}

///-------------------------------------------------------
/// CRecentMenuクラス

CRecentMenu::CRecentMenu()
{
	InsPos = 0;
	Max = RECMENUMAX;
	int i;
	for( i = 0; i < RECMENUMAX; i++ ){
		Caption[i] = "";
		Items[i] = NULL;
	}
	Items[i] = NULL;
}
void CRecentMenu::Init(int pos, TMenuItem *pmenu, int max)
{
	InsPos = pos;
	pMenu = pmenu;
	Max = max;
}
void CRecentMenu::Init(TMenuItem *pi, TMenuItem *pmenu, int max)
{
	Init(pmenu->IndexOf(pi), pmenu, max);
}
int CRecentMenu::GetCount(void)
{
	int i, n;
	for( i = n = 0; i < RECMENUMAX; i++, n++ ){
		if( Caption[i].IsEmpty() ) break;
	}
	return n;
}
LPCSTR CRecentMenu::GetItemText(int n)
{
	if( (n >= 0) && (n < RECMENUMAX) ){
		return Caption[n].c_str();
	}
	else {
		return NULL;
	}
}
void CRecentMenu::SetItemText(int n, LPCSTR p)
{
	if( (n >= 0) && (n < RECMENUMAX) ){
		Caption[n] = p;
	}
}
int CRecentMenu::IsAdd(LPCSTR pNew)
{
	return strcmp(Caption[0].c_str(), pNew);
}
void CRecentMenu::Add(LPCSTR pNew)
{
	if( !IsAdd(pNew) ) return;
	int i;
	for( i = 1; i < RECMENUMAX; i++ ){
		if( !strcmp(pNew, Caption[i].c_str()) ){
			for( ; i < (RECMENUMAX - 1); i++ ){
				Caption[i] = Caption[i+1];
			}
			Caption[i] = "";
			break;
		}
	}
	for( i = RECMENUMAX - 1; i > 0; i-- ){
		Caption[i] = Caption[i-1];
	}
	Caption[0] = pNew;
}
void CRecentMenu::Delete(LPCSTR pName)
{
	int i;
	for( i = 0; i < RECMENUMAX; i++ ){
		if( !strcmp(pName, Caption[i].c_str()) ){
			for( ; i < (RECMENUMAX - 1); i++ ){
				Caption[i] = Caption[i+1];
			}
			Caption[i] = "";
			break;
		}
	}
}
void CRecentMenu::ExtFilter(LPCSTR pExt)
{
	int i, j;

	for( i = 0; i < RECMENUMAX; i++ ){
		if( (!Caption[i].IsEmpty()) && strcmpi(GetEXT(Caption[i].c_str()), pExt) ){
			for( j = i; j < (RECMENUMAX - 1); j++ ){
				Caption[j] = Caption[j+1];
			}
			Caption[j] = "";
			i--;
		}
	}
}
void CRecentMenu::Clear(void)
{
	Delete();
	for( int i = 0; i < RECMENUMAX; i++ ){
		Caption[i] = "";
	}
}
void CRecentMenu::Insert(TObject *op, Classes::TNotifyEvent pEvent)
{
	int n = GetCount();
	int i;
	int pos = InsPos;
	if( n > Max ) n = Max;
	char bf[256];
	for( i = 0; i < n; i++, pos++ ){
		Items[i] = new TMenuItem((TComponent *)op);
		sprintf(bf, "&%d %s", (i + 1) % 10, Caption[i].c_str());
		Items[i]->Caption = bf;
		Items[i]->OnClick = pEvent;
		pMenu->Insert(pos, Items[i]);
	}
	if( n ){
		Items[i] = new TMenuItem((TComponent *)op);
		Items[i]->Caption = "-";
		Items[i]->OnClick = NULL;
		pMenu->Insert(pos, Items[n]);
	}
}
void CRecentMenu::Delete(void)
{
	for( int i = 0; i < RECMENUMAX+1; i++ ){
		if( Items[i] != NULL ){
			pMenu->Delete(InsPos);
			Items[i] = NULL;
		}
	}
}
int CRecentMenu::FindItem(TMenuItem *mp)
{
	for( int i = 0; i < RECMENUMAX; i++ ){
		if( Items[i] == mp ){
			return i;
		}
	}
	return -1;
}
LPCSTR CRecentMenu::FindItemText(TMenuItem *mp)
{
	return GetItemText(FindItem(mp));
}

///-------------------------------------------------------
/// CBitmapHistクラス
int CBitmapHist::Open(void)
{
	Close();
	char bf[256];
	sprintf(bf, "%s"HISTNAME, HistDir);
	FILE *fp = fopen(bf, "rb");
	if( fp == NULL ){
		if( (fp = fopen(bf, "wb"))!=NULL ){
			m_Head.m_Max = sys.m_HistMax;
			fwrite(&m_Head, 1, sizeof(m_Head), fp);
			fwrite(&m_File, 1, sizeof(m_File), fp);
			fclose(fp);
			m_FileName = bf;
			return TRUE;
		}
		else {
			ErrorFWrite(bf);
			return FALSE;
		}
	}
	else {
		fread(&m_Head, 1, sizeof(m_Head), fp);
		fread(&m_File, 1, sizeof(m_File), fp);
		fclose(fp);
		m_FileName = bf;
		ChangeMax(sys.m_HistMax);
		return TRUE;
	}
}
void CBitmapHist::Close(void)
{
	if( !m_FileName.IsEmpty() ){
		if( !OverWrite() ){
			ErrorFWrite(m_FileName.c_str());
		}
		m_FileName = "";
	}
}

int CBitmapHist::OverWrite(void)
{
	int r = FALSE;
	if( !m_FileName.IsEmpty() ){
		FILE *fp;
		if( (fp = fopen(m_FileName.c_str(), "wb"))!=NULL ){
			fwrite(&m_Head, 1, sizeof(m_Head), fp);
			fwrite(&m_File, 1, sizeof(m_File), fp);
			fclose(fp);
			r = TRUE;
		}
	}
	return r;
}

void CBitmapHist::ChangeMax(int max)
{
	if( m_Head.m_Max == max ) return;

	CWaitCursor wait;
	int i, s, t;
	HEADFH      sFile[HISTMAX];
	memset(sFile, 0, sizeof(sFile));
	char sname[256];
	char tname[256];
	s = m_Head.m_wPnt - 1;
	t = max - 1;
	for( i = 0; i < m_Head.m_Cnt; i++, s--, t-- ){
		if( s < 0 ) s += m_Head.m_Max;
		if( t < 0 ) t += max;
		sprintf(sname, m_UseJPEG ? "%sHist%d.jpg" : "%sHist%d.bmp", HistDir, s + 1);
		sprintf(tname, m_UseJPEG ? "%sHist%d.$pg" : "%sHist%d.$mp", HistDir, t + 1);
		if( i >= max ){
			unlink(sname);
		}
		else {
			rename(sname, tname);
			sFile[t] = m_File[s];
		}
	}
	t = max - 1;
	for( i = 0; i < max; i++, t-- ){
		if( t < 0 ) t += max;
		sprintf(sname, m_UseJPEG ? "%sHist%d.$pg" : "%sHist%d.$mp", HistDir, t + 1);
		sprintf(tname, m_UseJPEG ? "%sHist%d.jpg" : "%sHist%d.bmp", HistDir, t + 1);
		rename(sname, tname);
	}
	memcpy(m_File, sFile, sizeof(m_File));
	if( m_Head.m_Cnt >= max ) m_Head.m_Cnt = max;
	m_Head.m_wPnt = 0;
	m_Head.m_Max = max;
}

int CBitmapHist::Read(Graphics::TBitmap *pBitmap, int slot)
{
	char bf[256];
	sprintf(bf, m_UseJPEG ? "%sHist%d.jpg" : "%sHist%d.bmp", HistDir, slot + 1);

	int r = FALSE;
	FILE *fp;
	if( (fp = fopen(bf, "rb"))!=NULL ){
		fclose(fp);
		m_CurRead = slot;
		if( m_UseJPEG ){
			LoadJPEG(pBitmap, bf);
		}
		else {
			LoadBitmap(pBitmap, bf);
		}
		r = TRUE;
	}
	else {
		FillBitmap(pBitmap, clWhite);
	}
	return r;
}

void CBitmapHist::Write(Graphics::TBitmap *pBitmap, int slot)
{
	char bf[256];
	sprintf(bf, m_UseJPEG ? "%sHist%d.jpg" : "%sHist%d.bmp", HistDir, slot + 1);

	try {
		if( m_UseJPEG ){
			SaveJPEG(pBitmap, bf);
		}
		else {
			pBitmap->SaveToFile(bf);
		}
		m_File[slot].m_Flag |= 1;
	}
	catch(...){
		ErrorFWrite(bf);
	}
}

void CBitmapHist::Add(Graphics::TBitmap *pBitmap, SYSTEMTIME *pTime, int mode)
{
	m_CurRead = m_Head.m_wPnt;
	memcpy(&m_File[m_Head.m_wPnt].m_UTC, pTime, sizeof(SYSTEMTIME));
	mode++;
	 m_File[m_Head.m_wPnt].m_Flag = mode << 16;
	Write(pBitmap, m_Head.m_wPnt);
	m_Head.m_wPnt++;
	if( m_Head.m_wPnt >= m_Head.m_Max ){
		m_Head.m_wPnt = 0;
	}
	if( m_Head.m_Cnt < m_Head.m_Max ){
		m_Head.m_Cnt++;
	}
	m_CurAdd = 1;
	OverWrite();
}

void CBitmapHist::Update(Graphics::TBitmap *pBitmap)
{
	int n = m_Head.m_wPnt - 1;
	if( n < 0 ) n += m_Head.m_Max;
	Write(pBitmap, n);
}

//---------------------------------------------------------------------------
void CBitmapHist::Delete(int n)
{
	if( !m_Head.m_Cnt ) return;

	CWaitCursor wait;
	char tname[256];
	char sname[256];
	int max = m_Head.m_Cnt - n - 1;     // Copy count
	int i, s, t;
	t = m_Head.m_wPnt - n - 1;
	if( t < 0 ) t += m_Head.m_Max;
	if( m_Head.m_Cnt == 1 ){
		sprintf(tname, "%sHist%d.bmp", HistDir, t + 1);
		unlink(tname);
		m_Head.m_Cnt--;
		memset(&m_File[t], 0, sizeof(HEADFH));
		return;
	}
	for( i = 0; i < max; i++ ){
		s = t - 1;
		if( s < 0 ) s += m_Head.m_Max;
		sprintf(sname, m_UseJPEG ? "%sHist%d.jpg" : "%sHist%d.bmp", HistDir, s + 1);
		sprintf(tname, m_UseJPEG ? "%sHist%d.jpg" : "%sHist%d.bmp", HistDir, t + 1);
		if( !i ){
			unlink(tname);
		}
		MoveFile(sname, tname);
		m_File[t] = m_File[s];
		t--;
		if( t < 0 ) t = m_Head.m_Max - 1;
	}
	memset(&m_File[t], 0, sizeof(HEADFH));
	m_Head.m_Cnt--;
}

//---------------------------------------------------------------------------
void CBitmapHist::DeleteAll(void)
{
	if( !m_Head.m_Cnt ) return;

	CWaitCursor wait;
	char tname[256];
	char sname[256];
	for( int i = 0; i < m_Head.m_Max; i++ ){
		sprintf(sname, "%sHist%d.bmp", HistDir, i + 1);
		sprintf(tname, "%sHist%d.jpg", HistDir, i + 1);
		unlink(sname);
		unlink(tname);
	}
	memset(&m_File, 0, sizeof(m_File));
	m_Head.m_Cnt = 0;
	m_Head.m_wPnt = 0;
	m_CurAdd = 0;
	m_CurRead = 0;
}

//---------------------------------------------------------------------------
void CBitmapHist::MoveDir(LPCSTR pNewDir)
{
	if( !strcmp(pNewDir, HistDir) ) return;

	CWaitCursor wait;
	char tname[256];
	char sname[256];
	sprintf(tname, "%s"HISTNAME, pNewDir);
	FILE *fp = fopen(tname, "wb");
	if( fp == NULL ){
		strcpy(sname, pNewDir);
		if( *lastp(sname) == '\\' ) *lastp(sname) = 0;
		mkdir(sname);
		fp = fopen(tname, "wb");
	}
	if( fp == NULL ){
		ErrorMB( MsgEng ? "'%s' does not exsit" : "フォルダ '%s' が存在しません", pNewDir );
		return;
	}
	else {
		fclose(fp);
	}
	Close();
	sprintf(sname, "%s"HISTNAME, HistDir);
	if( CopyFile(sname, tname, FALSE) == FALSE ){
		if( YesNoMB( MsgEng ? "Fail to copy the files. Memory to change to the folder?" : "ファイルのコピーに失敗しました.  このフォルダへの変更を記憶しますか?" ) == IDYES ){
			strcpy(HistDir, pNewDir);
		}
		Open();
		return;
	}
	unlink(sname);
	int i;
	for( i = 0; i < m_Head.m_Max; i++ ){
		sprintf(sname, m_UseJPEG ? "%sHist%d.jpg" : "%sHist%d.bmp", HistDir, i + 1);
		sprintf(tname, m_UseJPEG ? "%sHist%d.jpg" : "%sHist%d.bmp", pNewDir, i + 1);
		CopyFile(sname, tname, FALSE);
		unlink(sname);
	}
	if( *lastp(HistDir) == '\\' ) *lastp(HistDir) = 0;
	_rmdir(HistDir);
	strcpy(HistDir, pNewDir);
	Open();
}

//---------------------------------------------------------------------------
void CBitmapHist::ChangeFormat(int sw)
{
	if( m_UseJPEG == sw ) return;
	if( !m_Head.m_Cnt ){
		m_UseJPEG = sw;
		return;
	}
	CWaitCursor wait;
	Graphics::TBitmap *pBitmap = new Graphics::TBitmap();
	AdjustBitmapFormat(pBitmap);

	FILE *fp;
	char tname[256];
	char sname[256];
	int i;
	for( i = 0; i < m_Head.m_Max; i++ ){
		sprintf(sname, sw ? "%sHist%d.bmp" : "%sHist%d.jpg", HistDir, i + 1);
		sprintf(tname, sw ? "%sHist%d.jpg" : "%sHist%d.bmp", HistDir, i + 1);
		if( (fp = fopen(sname, "rb")) != NULL ){
			fclose(fp);
			if( sw ){
				LoadBitmap(pBitmap, sname);
				SaveJPEG(pBitmap, tname);
			}
			else {
				LoadJPEG(pBitmap, sname);
				SaveBitmap(pBitmap, tname);
			}
			unlink(sname);
		}
	}
	delete pBitmap;
	m_UseJPEG = sw;
}
///-------------------------------------------------------
/// CExecPBクラス
//---------------------------------------------------------------------------
HWND __fastcall IsPB(void)
{
	return FindWindow("MSPaintApp", NULL);
}
//---------------------------------------------------------------------------
void __fastcall QuitPB(BOOL fWait)
{
	HWND hWndA = NULL;
	while(1){
		HWND hWnd = IsPB();
		if( (hWnd != NULL) && (hWnd != hWndA) ){
			hWndA = hWnd;
			::PostMessage(hWnd, WM_QUIT, 0, 0);
			if( fWait ){
				for( int i = 0; i < 30; i++ ){
					::Sleep(100);
					MultProc();
                    if( IsPB() != hWnd ) break;
                }
        	}
            else {
				break;
            }
        }
        else {
			break;
        }
	}
}

//---------------------------------------------------------------------------
__fastcall CExecPB::CExecPB()
{
	m_Flag = FALSE;
    m_hWndMsg = NULL;
    m_dwCount = 0;
}

//---------------------------------------------------------------------------
__fastcall CExecPB::~CExecPB()
{
	Cancel();
}

//---------------------------------------------------------------------------
BOOL __fastcall CExecPB::Timer(void)
{
	if( !m_Flag ) return TRUE;

	if( (m_dwCount & 1) && !IsPB() ){
		m_Flag = FALSE;
		if( m_hWndMsg && m_uMsg ){
			::PostMessage(m_hWndMsg, m_uMsg, 0, m_dwID);
       	}
       	return TRUE;
    }
    m_dwCount++;
    return FALSE;
}
//---------------------------------------------------------------------------
BOOL __fastcall CExecPB::Exec(LPCSTR pName, HWND hWnd, UINT uMsg, DWORD dwID)
{
	CWaitCursor w;

	m_Flag = FALSE;
	QuitPB(TRUE);

	m_Name = pName;
	WIN32_FIND_DATA	FileData;
	HANDLE hFind = ::FindFirstFile(pName, &FileData);
	if( hFind ) ::FindClose(hFind);
    m_nFileSizeLow = FileData.nFileSizeLow;
    m_ftLastWriteTime = FileData.ftLastWriteTime;

	m_hWndMsg = hWnd;
    m_uMsg = uMsg;
    m_dwID = dwID;

    ExecPB(pName);
    for( int i = 0; i < 100; i++ ){
		::Sleep(100);
        if( IsPB() ){
			m_Flag = TRUE;
			return TRUE;
        }
    }
    return FALSE;
}
//---------------------------------------------------------------------------
BOOL __fastcall CExecPB::Exec(Graphics::TBitmap *pBitmap, HWND hWnd, UINT uMsg, DWORD dwID)
{
    char name[256];
    sprintf(name, "%sTemp$.bmp", BgnDir);
	try {
	    pBitmap->SaveToFile(name);
	    return Exec(name, hWnd, uMsg, dwID);
    }
    catch(...){
		return FALSE;
    }
}
//---------------------------------------------------------------------------
BOOL __fastcall CExecPB::IsChanged(void)
{
	WIN32_FIND_DATA	FileData;
	HANDLE hFind = ::FindFirstFile(m_Name.c_str(), &FileData);
	if( !hFind ) return FALSE;
   	::FindClose(hFind);

	if( FileData.nFileSizeLow != m_nFileSizeLow ) return TRUE;
	return memcmp(&FileData.ftLastWriteTime, &m_ftLastWriteTime, sizeof(FILETIME));
}
//---------------------------------------------------------------------------
BOOL __fastcall CExecPB::LoadBitmap(Graphics::TBitmap *pBitmap, BOOL sw)
{
	BOOL r = FALSE;

    if( IsChanged() ){
#if 1
		TPicRectDlg *pBox = new TPicRectDlg(Mmsstv);
		try
		{
			pBox->pBitmap->LoadFromFile(m_Name);
			if( sw ){
				::CopyBitmap(pBitmap, pBox->pBitmap);
                r = TRUE;
			}
			else if( pBox->Copy(pBitmap) == TRUE ){
				r = TRUE;
			}
		}
		catch (...)
		{
		}
		delete pBox;
#else
		Graphics::TBitmap *pBmp = new Graphics::TBitmap;
		try {
		    pBmp->LoadFromFile(m_Name);
			if( sw ){
				::CopyBitmap(pBitmap, pBmp);
			}
            else {
				::StretchCopyBitmapHW(pBitmap, pBmp);
            }
   	    	r = TRUE;
    	}
    	catch(...){
    	}
        delete pBmp;
#endif
    }
    ::DeleteFile(m_Name.c_str());
    return r;
}
//---------------------------------------------------------------------------
void __fastcall CExecPB::Cancel(void)
{
	if( m_Flag ){
		m_Flag = FALSE;
		QuitPB(FALSE);
    }
}
//---------------------------------------------------------------------------
Graphics::TBitmap* __fastcall CreateBitmap(int xw, int yw, int fmt)
{
	Graphics::TBitmap *pBitmap = new Graphics::TBitmap;
	if( fmt == -1 ){
		AdjustBitmapFormat(pBitmap);
	}
	else {
		pBitmap->PixelFormat = TPixelFormat(fmt);
	}
	pBitmap->Width = xw;
	pBitmap->Height = yw;
	return pBitmap;
}
//---------------------------------------------------------------------------
void __fastcall StretchCopyBitmap(Graphics::TBitmap *pTer, Graphics::TBitmap *pSrc)
{
	if( (pTer->Width != pSrc->Width)||(pTer->Height != pSrc->Height) ){
		Graphics::TBitmap *pbmp = pTer;
		if( pTer->PixelFormat != pf24bit ){
			pbmp = CreateBitmap(pTer->Width, pTer->Height, pf24bit);
		}
		pbmp->Palette = NULL;
		StretchCopy(pbmp, NULL, pSrc, NULL);
		if( pbmp != pTer ){
			pTer->Palette = NULL;
			pTer->Canvas->Draw(0, 0, pbmp);
			delete pbmp;
		}
	}
	else {
		pTer->Palette = NULL;
		pTer->Canvas->Draw(0, 0, pSrc);
	}
}
//---------------------------------------------------------------------------
//アスペクト比を保持する
void __fastcall KeepAspect(int &sx, int &sy, int bx, int by)
{
	double ass = double(sx) / double(sy);
	double asb = double(bx) / double(by);
	if( asb < ass ){    // ソースの方が横長
		sx = (double(sy) * asb) + 0.5;
	}
	else {
		sy = (double(sx) / asb) + 0.5;
	}
}
//---------------------------------------------------------------------------
//アスペクト比を保持する
void __fastcall KeepAspectDraw(TCanvas *tp, int sx, int sy, Graphics::TBitmap *pBitmap)
{
	KeepAspect(sx, sy, pBitmap->Width, pBitmap->Height);
	TRect tc;
	tc.Left = 0; tc.Top = 0;
	tc.Right = sx; tc.Bottom = sy;
	int Sop = ::SetStretchBltMode(tp->Handle, HALFTONE);
	tp->StretchDraw(tc, pBitmap);
	::SetStretchBltMode(tp->Handle, Sop);
}
//---------------------------------------------------------------------------
//アスペクト比を保持して最大範囲を伸縮コピーする
void __fastcall StretchCopyBitmapHW(Graphics::TBitmap *pTer, Graphics::TBitmap *pSrc)
{
	if( (pTer->Width != pSrc->Width)||(pTer->Height != pSrc->Height) ){
		Graphics::TBitmap *pbmp = pTer;
		if( pTer->PixelFormat != pf24bit ){
			pbmp = CreateBitmap(pTer->Width, pTer->Height, pf24bit);
		}
		TRect sc;
		sc.Left = 0; sc.Top = 0;
		double ass = double(pSrc->Width) / double(pSrc->Height);
		double ast = double(pTer->Width) / double(pTer->Height);
		if( ast < ass ){    // ソースの方が横長
			sc.Bottom = pSrc->Height;
			sc.Right = double(pSrc->Height) * ast;
		}
		else {
			sc.Right = pSrc->Width;
			sc.Bottom = double(pSrc->Width) / ast;
		}
		int w;
		if( sc.Right < pSrc->Width ){
			w = (pSrc->Width - sc.Right) / 2;
			sc.Left += w;
			sc.Right += w;
		}
		if( sc.Bottom < pSrc->Height ){
			w = (pSrc->Height - sc.Bottom) / 2;
			sc.Top += w;
			sc.Bottom += w;
		}
		if( sc.Right >= pSrc->Width + 1 ) sc.Right = pSrc->Width;
		if( sc.Bottom >= pSrc->Height + 1 ) sc.Bottom = pSrc->Height;
		pbmp->Palette = NULL;
		StretchCopy(pbmp, NULL, pSrc, &sc);
		if( pbmp != pTer ){
			pTer->Palette = NULL;
			pTer->Canvas->Draw(0, 0, pbmp);
			delete pbmp;
		}
	}
	else {
		pTer->Palette = NULL;
		pTer->Canvas->Draw(0, 0, pSrc);
	}
}
//---------------------------------------------------------------------------
Graphics::TBitmap *__fastcall DupeBitmap(Graphics::TBitmap *pSrc, int fmt)
{
	Graphics::TBitmap *pBitmap = new Graphics::TBitmap;
	if( fmt == -1 ){
		AdjustBitmapFormat(pBitmap);
	}
	else {
		pBitmap->PixelFormat = TPixelFormat(fmt);
	}
	pBitmap->Height = pSrc->Height;
	pBitmap->Width = pSrc->Width;
	pBitmap->Palette = NULL;
	pBitmap->Canvas->Draw(0, 0, pSrc);
	return pBitmap;
}
//---------------------------------------------------------------------------
Graphics::TBitmap *__fastcall RemakeBitmap(Graphics::TBitmap *pBitmap, int fmt)
{
	int w, h;
	if( pBitmap != NULL ){
		w = pBitmap->Width;
		h = pBitmap->Height;
		delete pBitmap;
	}
	else {
		w = 320;
		h = 256;
	}
	pBitmap = CreateBitmap(w, h, fmt);
	return pBitmap;
}
//---------------------------------------------------------------------------
void __fastcall CopyBitmap(Graphics::TBitmap *pTer, Graphics::TBitmap *pSrc)
{
	if( (pTer->Width != pSrc->Width)||(pTer->Height != pSrc->Height) ){
		pTer->Width = pSrc->Width;
		pTer->Height = pSrc->Height;
	}
	pTer->Palette = NULL;
	pTer->Canvas->Draw(0, 0, pSrc);
}
//---------------------------------------------------------------------------
// APIの伸縮コピー
void __fastcall StretchCopy(Graphics::TBitmap *pTer, Graphics::TBitmap *pSrc, int mode)
{
	TRect rc;
	rc.Left = 0; rc.Top = 0;
	rc.Right = pTer->Width;
	rc.Bottom = pTer->Height;
	int Sop = ::SetStretchBltMode(pTer->Canvas->Handle, mode);	//ja7ude 0521
	pTer->Palette = NULL;
	pTer->Canvas->StretchDraw(rc, pSrc);
	::SetStretchBltMode(pTer->Canvas->Handle, Sop);                    	//ja7ude 0521
}
//---------------------------------------------------------------------------
// 高度な伸縮コピー
void __fastcall StretchCopy(Graphics::TBitmap *pBitmap, TRect *tc, Graphics::TBitmap *pbmp, TRect *rp)
{
	if( pBitmap == NULL ) return;
	CWaitCursor wait;

	Graphics::TBitmap *psrc;

	if( pbmp->PixelFormat != pf24bit ){
		psrc = DupeBitmap(pbmp, pf24bit);
	}
	else {
		psrc = pbmp;
	}
//    DecBitmap(pBitmap, tc, psrc, rp);
	int top = 0;
	int left = 0;
	int w = pBitmap->Width;
	int h = pBitmap->Height;
	pBitmap->PixelFormat = pf24bit;
	pBitmap->Width = w;
	pBitmap->Height = h;
	if( tc != NULL ){
		w = tc->Right - tc->Left;
		h = tc->Bottom - tc->Top;
		left = tc->Left;
		top = tc->Top;
	}

	double zy;
	double zx;
	int sw, sh;
	int xss, yss, xoff, yoff;
	if( rp != NULL ){
		sh = rp->Bottom - rp->Top;
		sw = rp->Right - rp->Left;
		yoff = rp->Top;
		xoff = rp->Left;
	}
	else {
		sh = psrc->Height;
		sw = psrc->Width;
		yoff = xoff = 0;
	}
	xss = sw / 2;
	yss = sh / 2;
	zy = double(h)/double(sh);
	zx = double(w)/double(sw);

	if( (sw > w) && (sh > h) ){
		MultProc();
		TRect RC, TC;
		if( tc != NULL ){
			TC = *tc;
		}
		else {
			TC.Left = 0; TC.Right = pBitmap->Width;
			TC.Top = 0; TC.Bottom = pBitmap->Height;
		}
		if( rp != NULL ){
			RC = *rp;
		}
		else {
			RC.Left = 0; RC.Top = 0;
			RC.Right = pbmp->Width; RC.Bottom = pbmp->Height;
		}
		int Sop = ::SetStretchBltMode(pBitmap->Canvas->Handle, HALFTONE);
		pBitmap->Palette = NULL;
		pBitmap->Canvas->CopyRect(TC, pbmp->Canvas, RC);
		::SetStretchBltMode(pBitmap->Canvas->Handle, Sop);
		if( psrc != pbmp ) delete psrc;
		return;
	}

	MultProc();
	int xs = w / 2;
	int ys = h / 2;
	int i, j, m, n;
	int r, g, b;
	double x, y, p, q, pp, qq;
	BYTE *tp, *ssp1, *ssp2, *sp1, *sp2;
	for( i = -ys; i < ys; i++ ){
		tp = (BYTE *)pBitmap->ScanLine[i+ys+top];
		if( left ) tp += left * 3;
		y = double(i)/zy;
		if( y > 0 ){
			m = int(y);
		}
		else {
			m = int(y-1);
		}
		q = y - double(m);
		if( int(q) == 1 ){
			q = 0; m++;
		}
		qq = 1.0 - q;
		if( m < -yss ) m = -yss;
		if( (m + yss + 1) >= psrc->Height ) m = psrc->Height - yss - 1;
		ssp1 = (BYTE *)psrc->ScanLine[yoff + m + yss];
		ssp2 = (BYTE *)psrc->ScanLine[yoff + m + 1 + yss];
		if( (m + yss + 1) >= psrc->Height ) ssp2 = ssp1;
		if( xoff ) ssp1 += xoff * 3;
		if( xoff ) ssp2 += xoff * 3;
		for( j = -xs; j < xs; j++ ){
			x = double(j)/zx;
			if( x > 0 ){
				n = int(x);
			}
			else {
				n = int(x-1);
			}
			p = x - double(n);
			if( int(p) == 1 ){
				p = 0; n++;
			}
			if( n < -xss ) n = -xss;
			if( (n + xss + 1) >= psrc->Width ) n = psrc->Width - xss - 1;

			sp1 = ssp1 + (n + xss) * 3;
			sp2 = ssp2 + (n + xss) * 3;
			pp = 1.0 - p;
			if( (n + xss + 1) >= psrc->Width ){
				r = (int)(qq*(pp*(*sp1))
					+ q * (pp*(*sp2)) );
				sp1++; sp2++;
				g = (int)(qq*(pp*(*sp1))
					+ q * (pp*(*sp2)) );
				sp1++; sp2++;
				b = (int)(qq*(pp*(*sp1))
					+ q * (pp*(*sp2)) );
			}
			else {
				r = (int)(qq*(pp*(*sp1) + p*(*(sp1+3)))
					+ q * (pp*(*sp2) + p*(*(sp2+3))));
				sp1++; sp2++;
				g = (int)(qq*(pp*(*sp1) + p*(*(sp1+3)))
					+ q * (pp*(*sp2) + p*(*(sp2+3))));
				sp1++; sp2++;
				b = (int)(qq*(pp*(*sp1) + p*(*(sp1+3)))
					+ q * (pp*(*sp2) + p*(*(sp2+3))));
			}
			LimitRGB(r, g, b);
			*tp++ = BYTE(r);
			*tp++ = BYTE(g);
			*tp++ = BYTE(b);
		}
		MultProc();
	}
	if( psrc != pbmp ) delete psrc;
}
//---------------------------------------------------------------------------
void __fastcall SetGroupEnabled(TGroupBox *gp)
{
	int enb = gp->Enabled;
	TControl *tp;
	int i;
	for( i = 0; i < gp->ControlCount; i++ ){
		tp = gp->Controls[i];
		if( tp != NULL ){
			tp->Enabled = enb;
			if( tp->ClassNameIs("TGroupBox") ){
				SetGroupEnabled((TGroupBox *)tp);
			}
		}
	}
	gp->Font->Color = gp->Enabled ? clBlack : clGrayText;
	for( i = 0; i < gp->ControlCount; i++ ){
		tp = gp->Controls[i];
		if( tp != NULL ){
			if( tp->ClassNameIs("TComboBox") ){
				((TComboBox *)tp)->SelLength = 0;
			}
		}
	}
}
//---------------------------------------------------------------------------
int __fastcall Limit256(int d)
{
	if( d < 0 ) d = 0;
	if( d > 255 ) d = 255;
	return d;
}
//---------------------------------------------------------------------------
void __fastcall LimitRGB(int &R, int &G, int &B)
{
	R = Limit256(R);
	G = Limit256(G);
	B = Limit256(B);
}
//---------------------------------------------------------------------------
void __fastcall YCtoRGB(int &R, int &G, int &B, int Y, int RY, int BY)
{
	Y = Y - 16;
	R = 1.164457*Y + 1.596128*RY;
	G = 1.164457*Y - 0.813022*RY - 0.391786*BY;
	B = 1.164457*Y + 2.017364*BY;
	LimitRGB(R, G, B);
}
//---------------------------------------------------------------------------
TColor __fastcall YCtoTColor(int Y, int RY, int BY)
{
	int R, G, B;
	YCtoRGB(R, G, B, Y, RY, BY);
	return TColor((B << 16) + (G << 8) + R);
}
//---------------------------------------------------------------------------
int __fastcall ColorToFreq(int d)
{
	d = d * (2300 - 1500) / 256;
	return d + 1500;
}
//---------------------------------------------------------------------------
int __fastcall ColorToFreqNarrow(int d)
{
	d = d * NARROW_BW / 256;
	return d + NARROW_LOW;
}
//---------------------------------------------------------------------------
void __fastcall MultProc(void)
{
	Mmsstv->MultProc();
}
//---------------------------------------------------------------------------
void __fastcall MultProcA(void)
{
	if( Mmsstv->pSound->IsBusy() ) Mmsstv->MultProc();
}
//---------------------------------------------------------------------------
void __fastcall DrawBitmap(TPaintBox *pBox, Graphics::TBitmap *pBitmap)
{
	if( (pBox->Height >= 256) && ((pBitmap->Width != pBox->Width)||(pBitmap->Height != pBox->Height)) ){
		TRect rc;
		rc.Left = 0; rc.Right = pBox->Width;
		rc.Top = 0; rc.Bottom = pBox->Height;
		int Sop = ::SetStretchBltMode(pBox->Canvas->Handle, HALFTONE);
		pBox->Canvas->StretchDraw(rc, pBitmap);
		::SetStretchBltMode(pBox->Canvas->Handle, Sop);
	}
	else {
		pBox->Canvas->Draw(0, 0, pBitmap);
	}
}
//---------------------------------------------------------------------------
void __fastcall ReSizeBitmap(Graphics::TBitmap *pBitmap, int w, int h)
{
	if( (pBitmap->Width != w) || (pBitmap->Height != h) ){
		Graphics::TBitmap *pbmp = CreateBitmap(w, h, pf24bit);
		::StretchCopy(pbmp, NULL, pBitmap, NULL);
		pBitmap->Width = w;
		pBitmap->Height = h;
		pBitmap->Canvas->Draw(0, 0, pbmp);
		delete pbmp;
	}
}
//---------------------------------------------------------------------------
// メディアンフィルタ
static int _cmp(const void *s, const void *t)
{
	return *((BYTE *)s) - *((BYTE *)t);
}
Graphics::TBitmap *__fastcall MFilter(Graphics::TBitmap *pTer, Graphics::TBitmap *pSrc, int sw)
{
	CWaitCursor wait;
	Graphics::TBitmap *pBitmapS;
	if( pSrc->PixelFormat != pf24bit ){
		pBitmapS = DupeBitmap(pSrc, pf24bit);
	}
	else {
		pBitmapS = pSrc;
	}
	int XW = pBitmapS->Width;
	int YW = pBitmapS->Height;
	if( pTer == NULL ){
		pTer = new Graphics::TBitmap();
	}
	pTer->PixelFormat = pf24bit;
	pTer->Width = XW;
	pTer->Height = YW;


	MultProc();
	BYTE *bp = new BYTE[XW * YW * 3];
	int x, y;
	BYTE *tp = bp;
	BYTE *sp;
	for( y = 0; y < YW; y++ ){
		sp = (BYTE*)pBitmapS->ScanLine[y];
		for( x = 0; x < XW; x++ ){
			*tp++ = *sp++;
			*tp++ = *sp++;
			*tp++ = *sp++;
		}
	}
	int xw = XW * 3;          // 960
	int xwm = (XW - 1) * 3;
	BYTE List[9];
	if( sw ){
		for( y = 1; y < (YW - 1); y++ ){
			sp = &bp[y*xw+3];
			tp = (BYTE*)pTer->ScanLine[y] + 3;
			for( x = 1; x < xwm; x++, sp++, tp++ ){
				List[0] = *sp;
				List[1] = *(sp+3);
				List[2] = *(sp-3);
				List[3] = *(sp-xw);
				List[4] = *(sp-xw+3);
				List[5] = *(sp-xw-3);
				List[6] = *(sp+xw);
				List[7] = *(sp+xw+3);
				List[8] = *(sp+xw-3);
				qsort(List, 9, 1, _cmp);
//				*tp = List[5];
				*tp = List[4];
			}
			MultProcA();
		}
	}
	else {
		for( y = 1; y < (YW - 1); y++ ){
			sp = &bp[y*xw+3];
			tp = (BYTE*)pTer->ScanLine[y] + 3;
			for( x = 1; x < xwm; x++, sp++, tp++ ){
				List[0] = *sp;
				List[1] = *(sp+3);
				List[2] = *(sp-3);
				List[3] = *(sp-xw);
				List[4] = *(sp+xw);
				qsort(List, 5, 1, _cmp);
//				*tp = List[3];
				*tp = List[2];
			}
			MultProcA();
		}
	}
	MultProc();
	delete bp;
	if( pSrc != pBitmapS ) delete pBitmapS;
	return pTer;
}

void __fastcall CopyAutoSize(Graphics::TBitmap *pTer, Graphics::TBitmap *pSrc)
{
	if( (pSrc->Width == pTer->Width) && ((pSrc->Height == 240)||(pSrc->Height == 480)) ){
		switch(sys.m_Way240){
			case 0:
				StretchCopyBitmap(pTer, pSrc);
				break;
			case 1:
				StretchCopyBitmapHW(pTer, pSrc);
				break;
			case 2:
				FillBitmap(pTer, sys.m_PicClipColor);
				pTer->Canvas->Draw(0, 0, pSrc);
				break;
			default:
				FillBitmap(pTer, sys.m_PicClipColor);
				pTer->Canvas->Draw(0, 16, pSrc);
				break;
		}
	}
	else {
		StretchCopyBitmapHW(pTer, pSrc);
	}
}
//---------------------------------------------------------------------------
void __fastcall GetRY(int &Y, int &RY, int &BY, TColor col)
{
	BYTE *p = (BYTE *)&col;
	double R = *p++;
	double G = *p++;
	double B = *p;
#if 0
	Y = 16.0 + (.003906 * ((65.738 * R) + (129.057 * G) + (25.064 * B)));
	RY = 128.0 + (.003906 * ((112.439 * R) + (-94.154 * G) + (-18.285 * B)));
	BY = 128.0 + (.003906 * ((-37.945 * R) + (-74.494 * G) + (112.439 * B)));
#else
	Y = 16.0 + (0.256773*R + 0.504097*G + 0.097900*B);
	RY = 128.0 + (0.439187*R - 0.367766*G - 0.071421*B);
	BY = 128.0 + (-0.148213*R - 0.290974*G + 0.439187*B);
#endif
	LimitRGB(Y, RY, BY);
}
//---------------------------------------------------------------------------
void __fastcall DownColor(Graphics::TBitmap *pbmp, TColor bcol, int lm)
{
	CWaitCursor w;

	int lm2 = lm * 3;
	int SY, SR, SB;
	int TY, TR, TB;
	TColor tcol;
	GetRY(SY, SR, SB, bcol);
	int x, y;
	for( y = 0; y < pbmp->Height; y++ ){
		MultProc();
		for( x = 0; x < pbmp->Width; x++ ){
			tcol = pbmp->Canvas->Pixels[x][y];
			if( bcol != tcol ){
				GetRY(TY, TR, TB, tcol);
				if( (ABS(SY - TY) <= lm2)&&
					(ABS(SR - TR) <= lm)&&
					(ABS(SB - TB) <= lm)
				){
					pbmp->Canvas->Pixels[x][y] = bcol;
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
void KeyEvent(const short *p)
{
	for( ; *p; p++ ){
		keybd_event(BYTE(*p), 0, *p&0x8000 ? KEYEVENTF_KEYUP : 0, 0);
		MultProc();
	}
}
//---------------------------------------------------------------------------
void __fastcall SetThumbnailSize(TRect &rc, int sw)
{
	switch(sw){
		case 0:
			rc.Right = 50;
			rc.Bottom = 40;
			break;
		case 1:
			rc.Right = 64;
			rc.Bottom = 51;
			break;
		case 2:
			rc.Right = 80;
			rc.Bottom = 64;
			break;
		default:
			rc.Right = 100;
			rc.Bottom = 80;
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall NormalRect(long *x1, long *y1, long *x2, long *y2)	//ja7ude 0521
{
	if( *x1 > *x2 ){
		int d = *x1;
		*x1 = *x2;
		*x2 = d;
	}
	if( *y1 > *y2 ){
		int d = *y1;
		*y1 = *y2;
		*y2 = d;
	}
}
void __fastcall NormalRect(int &x1, int &y1, int &x2, int &y2)	//ja7ude 0521
{
	if( x1 > x2 ){
		int d = x1;
		x1 = x2;
		x2 = d;
	}
	if( y1 > y2 ){
		int d = y1;
		y1 = y2;
		y2 = d;
	}
}
//---------------------------------------------------------------------------
void __fastcall NormalRect(TRect *rc)
{
	NormalRect(&(rc->Left), &(rc->Top), &(rc->Right), &(rc->Bottom));	//ja7ude 0521
}
//---------------------------------------------------------------------------
int __fastcall GetActiveIndex(TPageControl *pp)
{
	int i;
	for( i = 0; i < pp->PageCount; i++ ){
		if( pp->ActivePage == pp->Pages[i] ) return i;
	}
	return 0;
}
//---------------------------------------------------------------------------
void __fastcall SetMBCP(void)
{
	_setmbcp(_MB_CP_ANSI);
}
//---------------------------------------------------------------------------
static int CodeErr = 0;
//---------------------------------------------------------------------------
void __fastcall ClearCodeErr(void)
{
	CodeErr = 0;
}
//---------------------------------------------------------------------------
void __fastcall SetMBCP(BYTE charset)
{
	UINT cp;
	switch(charset){
		case SHIFTJIS_CHARSET:
			cp = 932;
			break;
		case HANGEUL_CHARSET:
			cp = 949;
			break;
		case CHINESEBIG5_CHARSET:   //
			cp = 950;
			break;
		case 134:       // 簡略
			cp = 936;
			break;
		default:
			cp = _MB_CP_ANSI;
			break;
	}
	if( cp != _MB_CP_ANSI ){
		CPINFO info;
		if( GetCPInfo(cp, &info) != TRUE ){
			if( !CodeErr ){
				CodeErr++;
				InfoMB("CodePage %d is not supported on your PC.", cp);
			}
			cp = _MB_CP_ANSI;
		}
	}
	_setmbcp(cp);
}
//---------------------------------------------------------------------------
void __fastcall MoveParent(TWinControl *pNew, TWinControl *pOld, TWinControl *pCtr)
{
	pOld->RemoveControl(pCtr);
	pNew->InsertControl(pCtr);
	pCtr->Parent = pNew;
}
//---------------------------------------------------------------------------
BYTE __fastcall InvFontCharset(BYTE charset)
{
	BYTE cset;
	UINT cp = ::GetACP();
	switch(cp){
		case 932:
	       	cset = SHIFTJIS_CHARSET;
			break;
		case 949:
           	cset = HANGEUL_CHARSET;
			break;
		case 950:
           	cset = CHINESEBIG5_CHARSET;
			break;
		case 936:
           	cset = 134;       // 簡略
			break;
		default:
			cset = DEFAULT_CHARSET;
			break;
	}
	return BYTE((charset != ANSI_CHARSET) ? ANSI_CHARSET : cset);
}
//---------------------------------------------------------------------------
void __fastcall InvFontCharset(TFont *pFont)
{
	pFont->Charset = InvFontCharset(pFont->Charset);
}
//---------------------------------------------------------------------------


