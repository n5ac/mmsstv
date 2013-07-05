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
#include <vcl.h>
#pragma hdrstop

#include <io.h>
#include "LogConv.h"
#include "country.h"
#include "main.h"
CLogText	LogText;
//***************************************************************************
// CLogConv 基本 クラス
CLogConv::CLogConv()
{
	m_Mode = 0;
	m_fp = NULL;
}

CLogConv::~CLogConv()
{
}
//***************************************************************************
// CLogText クラス
CLogText::CLogText()
{
	m_Type = 0;
	m_Double = 0;
	m_Delm = 0;
	m_err = 0;
	m_UTC = (lcid != LANG_JAPANESE) ? 1 : 0;
	int i;
	for( i = 0; i < TEXTCONVMAX; i++ ){
		m_tConv[i].Key = "";
		m_tConv[i].w = 0;
	}
	m_tConv[0].Key = "%YYYY-MM-DD";
	m_tConv[1].Key = "%HHMM";
	m_tConv[2].Key = "%EHHMM";
	m_tConv[3].Key = "%CALL";
	m_tConv[4].Key = "%HIS";
	m_tConv[5].Key = "%MY";
	m_tConv[6].Key = "%FREQ";
	m_tConv[7].Key = "%MODE";
	m_tConv[8].Key = "%POWER";
	m_tConv[9].Key = "%NAME";
	m_tConv[10].Key = "%QTH";
	m_tConv[11].Key = "%REM";
	m_tConv[12].Key = "%QSL";
	m_tConv[13].Key = "%S";
	m_tConv[14].Key = "%R";
	m_tConv[15].Key = "%EOD";
	for( i = 0; i < TEXTCONVMAX; i++ ){
		m_rConv[i] = m_tConv[i];
	}
}
//---------------------------------------------------------------------------
// テキストファイルのオープン
int CLogText::Open(LPCSTR pName)
{
	Close();
	m_Mode = 0;
	m_err = 0;

	m_fp = fopen(pName, "rt");

	if( m_fp != NULL ){
		setvbuf(m_fp, NULL, _IOFBF, 16384);
		m_FileName = pName;
		m_Mode = 0;
		return TRUE;
	}
	else {
		ErrorMB( MsgEng ? "Can't open '%s'":"'%s'がオープンできません.", pName);
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// テキストファイルの作成
int CLogText::Create(LPCSTR pName)
{
	Close();
	m_Mode = 1;

	m_fp = fopen(pName, "wt");
	if( m_fp != NULL ){
		setvbuf(m_fp, NULL, _IOFBF, 16384);
		m_FileName = pName;
		m_Mode = 0;
		return TRUE;
	}
	else {
		ErrorMB( MsgEng ? "Can't write to '%s'":"'%s'が作成できません.", pName);
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// テキストファイルのオープン
int CLogText::Close(void)
{
	int r = 0;
	if( m_fp != NULL ){
		r = fclose(m_fp);
		m_fp = NULL;
		if( r ){
			ErrorMB( MsgEng ? "Can't close '%s'":"'%s'が正しくクローズできませんでした.", m_FileName.c_str());
		}
		else if( m_Mode ){
			InfoMB(MsgEng ? "Done (write to '%s')":"'%s'への書きこみを終了しました.", m_FileName.c_str());
		}
	}
	return r ? FALSE : TRUE;
}
//---------------------------------------------------------------------------
// テキストファイルの読み出し
int CLogText::Read(SDMMLOG *sp)
{
	if( !IsOpen() ) return FALSE;

	if( !feof(m_fp) ){
		if( fgets(m_bf, sizeof(m_bf), m_fp) != NULL ){
			ClipLF(m_bf);
			return Text2MMLOG(sp, m_bf, m_err);
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// 出力変換
int CLogText::Write(SDMMLOG *sp)
{
	if( !IsOpen() ) return FALSE;

	MMLOG2Text(m_bf, sp);
	fputs(m_bf, m_fp);
	fputs("\n", m_fp);
	return ferror(m_fp) ? FALSE : TRUE;
}

const LPCSTR ConvTbl[]={
	"%NULL",
	"%YYYY-MM-DD", "%YY-MM-DD", "%YYYY/MM/DD", "%YY/MM/DD",
	"%DD MON, YYYY","%YYMMDD", "%YYYYMMDD",
	"%YY", "%YYYY", "%MM", "%MON", "%MON2", "%DD", "%HHMM", "%HH:MM","%HH:MM:SS",
	"%EHHMM", "%EHH:MM", "%EHH:MM:SS", "%CALL",
	"%M", "%MY", "%HIS", "%MYRST", "%HISRST", "%MYNR", "%HISNR", "%FREQ", "%KHZ",
	"%MBAND", "%MODE", "%POWER", "%NAME", "%QTH", "%REM", "%QSL",
	"%S", "%R", "%ENV", "%OPT1", "%OPT2", "%USR1", "%USR2",
	"%EOD",
	NULL,
};

const	char	*MONT1[]={"","JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
const	char	*MONT2[]={"","Jan.","Feb.","Mar.","Apr.","May","June","July","Aug.","Sept.","Oct.","Nov.","Dec."};

//---------------------------------------------------------------------------
void MMLOG2Text(LPSTR t, SDMMLOG *sp, AnsiString &Key)
{
	int		tim, l;

	if( Key == "%YYYY-MM-DD" ){
		sprintf(t, "%04u-%02u-%02u", YEAR(sp->year), sp->date/100, sp->date%100);
	}
	else if( Key == "%YY-MM-DD" ){
		sprintf(t, "%2u-%02u-%02u", sp->year, sp->date/100, sp->date%100);
	}
	else if( Key == "%YYYY/MM/DD" ){
		sprintf(t, "%04u/%02u/%02u", YEAR(sp->year), sp->date/100, sp->date%100);
	}
	else if( Key == "%YY/MM/DD" ){
		sprintf(t, "%2u/%02u/%02u", sp->year, sp->date/100, sp->date%100);
	}
	else if( Key == "%DD MON, YYYY" ){
		sprintf(t, "%02u %s, %04u", sp->date%100, MONT1[sp->date/100], YEAR(sp->year));
	}
	else if( Key == "%YYYYMMDD" ){
		sprintf(t, "%04u%02u%02u", YEAR(sp->year), sp->date/100, sp->date%100);
	}
	else if( Key == "%YYMMDD" ){
		sprintf(t, "%02u%02u%02u", sp->year, sp->date/100, sp->date%100);
	}
	else if( Key == "%YY" ){
		sprintf(t, "%02u", sp->year);
	}
	else if( Key == "%YYYY" ){
		sprintf(t, "%04u", YEAR(sp->year));
	}
	else if( Key == "%MM" ){
		sprintf(t, "%02u", sp->date / 100);
	}
	else if( Key == "%MON" ){
		strcpy(t, MONT1[sp->date/100]);
	}
	else if( Key == "%MON2" ){
		strcpy(t, MONT2[sp->date/100]);
	}
	else if( Key == "%DD" ){
		sprintf(t, "%02u", sp->date % 100);
	}
	else if( Key == "%HHMM" ){
		if( sp->btime ){
			tim = sp->btime / 30;
			sprintf(t, "%02u%02u", tim / 60, tim % 60);
		}
		else {
			*t = 0;
		}
	}
	else if( Key == "%HH:MM" ){
		if( sp->btime ){
			tim = sp->btime / 30;
			sprintf(t, "%02u:%02u", tim / 60, tim % 60);
		}
		else {
			*t = 0;
		}
	}
	else if( Key == "%HH:MM:SS" ){
		if( sp->btime ){
			tim = sp->btime / 30;
			sprintf(t, "%02u:%02u:%02u", tim / 60, tim % 60, (sp->btime * 2) % 60);
		}
		else {
			*t = 0;
		}
	}
	else if( Key == "%EHHMM" ){
		if( sp->etime ){
			tim = sp->etime / 30;
			sprintf(t, "%02u%02u", tim / 60, tim % 60);
		}
		else {
			*t = 0;
		}
	}
	else if( Key == "%EHH:MM" ){
		if( sp->etime ){
			tim = sp->btime / 30;
			sprintf(t, "%02u:%02u", tim / 60, tim % 60);
		}
		else {
			*t = 0;
		}
	}
	else if( Key == "%EHH:MM:SS" ){
		if( sp->etime ){
			tim = sp->btime / 30;
			sprintf(t, "%02u:%02u:%02u", tim / 60, tim % 60, (sp->etime * 2) % 60);
		}
		else {
			*t = 0;
		}
	}
	else if( Key == "%CALL" ){
		strcpy(t, sp->call);
	}
	else if( Key == "%M" ){
		*t = sp->send;
		*(t+1) = 0;
	}
	else if( Key == "%HIS" ){
		strcpy(t, sp->ur);
	}
	else if( Key == "%MY" ){
		strcpy(t, sp->my);
	}
	else if( Key == "%HISRST" ){
		StrCopy(t, sp->ur, GetLMode(sp->mode));
	}
	else if( Key == "%MYRST" ){
		StrCopy(t, sp->my, GetLMode(sp->mode));
	}
	else if( Key == "%HISNR" ){
		l = GetLMode(sp->mode);
		if( (int)strlen(sp->ur) >= l ){
			strcpy(t, &sp->ur[l]);
		}
		else {
			*t = 0;
		}
	}
	else if( Key == "%MYNR" ){
		l = GetLMode(sp->mode);
		if( (int)strlen(sp->my) >= l ){
			strcpy(t, &sp->my[l]);
		}
		else {
			*t = 0;
		}
	}
	else if( Key == "%FREQ" ){
		strcpy(t, Log.GetFreqString(sp->band, sp->fq));
	}
	else if( Key == "%MBAND" ){
		strcpy(t, FreqTomBand(sp));
	}
	else if( Key == "%KHZ" ){
		char bf[32];
		double dd;
		strcpy(bf, Log.GetFreqString(sp->band, sp->fq));
		sscanf(bf, "%lf", &dd);
		sprintf(t, "%.2lf", dd * 1000.0);
	}
	else if( Key == "%MODE" ){
		strcpy(t, Log.GetModeString(sp->mode));
	}
	else if( Key == "%POWER" ){
		strcpy(t, sp->pow);
	}
	else if( Key == "%NAME" ){
		strcpy(t, sp->name);
	}
	else if( Key == "%QTH" ){
		strcpy(t, sp->qth);
	}
	else if( Key == "%REM" ){
		strcpy(t, sp->rem);
	}
	else if( Key == "%QSL" ){
		strcpy(t, sp->qsl);
	}
	else if( Key == "%S" ){
		*t = sp->send;
		*(t+1) = 0;
	}
	else if( Key == "%R" ){
		*t = sp->recv;
		*(t+1) = 0;
	}
	else if( Key == "%ENV" ){
		sprintf(t, "%u", sp->env);
	}
	else if( Key == "%OPT1" ){
		strcpy(t, sp->opt1);
	}
	else if( Key == "%OPT2" ){
		strcpy(t, sp->opt2);
	}
	else if( Key == "%USR1" ){
		strcpy(t, Log.GetOptStr(2, sp));
	}
	else if( Key == "%USR2" ){
		strcpy(t, Log.GetOptStr(3, sp));
	}
	else if( (Key == "%EOD")||(Key == "%NULL") ){
		*t = 0;
	}
	else {
		strcpy(t, Key.c_str());
	}
}
//---------------------------------------------------------------------------
// 1行の出力変換
void CLogText::MMLOG2Text(LPSTR t, SDMMLOG *sp)
{
	char	bf[512];
	char	dlm = m_Delm ? TAB : ',';

	if( m_UTC ) JSTtoUTC(sp);
	int i;
	for( i = 0; i < TEXTCONVMAX; i++ ){
		if( (!m_tConv[i].Key.IsEmpty())&&(m_tConv[i].Key != "%EOD") ){
			::MMLOG2Text(bf, sp, m_tConv[i].Key);
			switch(m_Delm){
			case 0:				// CSV
			case 1:				// TAB
				if( i ) *t++ = dlm;
				if( m_Double || (strchr(bf, dlm) != NULL) ) *t++ = 0x22;
				if( m_tConv[i].w ){
					StrCopy(t, bf, m_tConv[i].w);
				}
				else {
					strcpy(t, bf);
				}
				t += strlen(t);
				if( m_Double || (strchr(bf, dlm) != NULL) ) *t++ = 0x22;
				break;
			default:			// Format Text
				if( m_tConv[i].w ){
					StrCopy(t, bf, m_tConv[i].w);
					FillSpace(t, m_tConv[i].w);
				}
				else {
					strcpy(t, bf);
				}
				t += strlen(t);
				break;
			}
		}
		else {
			break;
		}
	}
	*t = 0;
}
//---------------------------------------------------------------------------
int GetMonth(LPCSTR p)
{
	for( int i = 1; i <=12; i++ ){
		if( !strcmpi(MONT1[i], p) ) return i;
		if( !strcmpi(MONT2[i], p) ) return i;
	}
	return 0;
}
//---------------------------------------------------------------------------
int Text2MMLOG(SDMMLOG *sp, LPCSTR s, AnsiString &Key)
{
	int		y, m, d, h;
	int		tim;

	if( (Key == "%YYYY-MM-DD")||(Key == "%YY-MM-DD") ){
		if( sscanf(s, "%u-%u-%u", &y, &m, &d) != 3 ) return FALSE;
		sp->year = BYTE(y % 100);
		sp->date = WORD(m * 100 + d);
	}
	else if( (Key == "%YYYY/MM/DD")||(Key == "%YY/MM/DD") ){
		if( sscanf(s, "%u/%u/%u", &y, &m, &d) != 3 ) return FALSE;
		sp->year = BYTE(y % 100);
		sp->date = WORD(m * 100 + d);
	}
	else if( Key == "%DD MON, YYYY" ){
		d = atoin(s, 2);
		s += 3;
		char bf[4];
		StrCopy(bf, s, 3);
		m = GetMonth(bf);
		s = SkipSpace(s+1);
		y = atoin(s, -1);
		sp->year = BYTE(y % 100);
		sp->date = WORD(m * 100 + d);
	}
	else if( Key == "%YYYYMMDD" ){
		y = atoin(s, 4); s += 4;
		m = atoin(s, 2); s += 2;
		d = atoin(s, 2);
		sp->year = BYTE(y % 100);
		sp->date = WORD(m * 100 + d);
	}
	else if( Key == "%YYMMDD" ){
		y = atoin(s, 2); s += 2;
		m = atoin(s, 2); s += 2;
		d = atoin(s, 2);
		sp->year = BYTE(y % 100);
		sp->date = WORD(m * 100 + d);
	}
	else if( (Key == "%YY")||(Key == "%YYYY") ){
		if( sscanf(s, "%u", &y) != 1 ) return FALSE;
		sp->year = BYTE(y % 100);
	}
	else if( Key == "%MM" ){
		if( sscanf(s, "%u", &m) != 1 ) return FALSE;
		sp->date = WORD((sp->date % 100) + m * 100);
	}
	else if( (Key == "%MON")||(Key == "%MON2") ){
		m = GetMonth(s);
		sp->date = WORD((sp->date % 100) + m * 100);
	}
	else if( Key == "%DD" ){
		if( sscanf(s, "%u", &d) != 1 ) return FALSE;
		sp->date = WORD(((sp->date / 100) * 100) + d);
	}
	else if( Key == "%HHMM" ){
		if( *s ){
			if( sscanf(s, "%u", &d) != 1 ) return FALSE;
			h = d / 100;
			m = d % 100;
			tim = (h * 60 + m) * 30;
			if( !tim ) tim++;
			sp->btime = WORD(tim);
		}
	}
	else if( (Key == "%HH:MM")||(Key == "%HH:MM:SS") ){
		if( *s ){
			if( sscanf(s, "%u:%u:%u", &h, &m, &d ) != 3 ){
				d = 0;
				if( sscanf(s, "%u:%u", &h, &m) != 2 ) return FALSE;
			}
			tim = (h * 60 + m) * 30 + (d / 2);
			if( !tim ) tim++;
			sp->btime = WORD(tim);
		}
	}
	else if( Key == "%EHHMM" ){
		if( *s ){
			if( sscanf(s, "%u", &d) != 1 ) return FALSE;
			h = d / 100;
			m = d % 100;
			tim = (h * 60 + m) * 30;
			if( !tim ) tim++;
			sp->etime = WORD(tim);
		}
	}
	else if( (Key == "%EHH:MM")||(Key == "%EHH:MM:SS") ){
		if( *s ){
			if( sscanf(s, "%u:%u:%u", &h, &m, &d ) != 3 ){
				d = 0;
				if( sscanf(s, "%u:%u", &h, &m) != 2 ) return FALSE;
			}
			tim = (h * 60 + m) * 30 + (d / 2);
			if( !tim ) tim++;
			sp->etime = WORD(tim);
		}
	}
	else if( Key == "%CALL" ){
		if( sp->call[0] ){
			char bf[256];
			sprintf(bf, "%s/%s", sp->call, s);
			StrCopy(sp->call, bf, MLCALL);
		}
		else {
			StrCopy(sp->call, s, MLCALL);
		}
	}
	else if( Key == "%M" ){
		sp->send = *s;
	}
	else if( (Key == "%HIS")||(Key == "%HISRST") ){
		StrCopy(sp->ur, s, MLRST);
	}
	else if( (Key == "%MY")||(Key=="%MYRST") ){
		StrCopy(sp->my, s, MLRST);
	}
	else if( Key == "%FREQ" ){
		Log.SetFreq(sp, s);
	}
	else if( Key == "%MBAND" ){
		mBandToBand(sp, s);
	}
	else if( Key == "%KHZ" ){
		double dd;
		sscanf(s, "%lf", &dd);
		char bf[32];
		sprintf(bf, "%.03lf", dd / 1000.0);
		Log.SetFreq(sp, bf);
	}
	else if( Key == "%MODE" ){
		Log.SetMode(sp, s);
	}
	else if( Key == "%POWER" ){
		StrCopy(sp->pow, s, MLPOW);
	}
	else if( Key == "%NAME" ){
		StrCopy(sp->name, s, MLNAME);
	}
	else if( Key == "%QTH" ){
		StrCopy(sp->qth, s, MLQTH);
	}
	else if( Key == "%REM" ){
		StrCopy(sp->rem, s, MLREM);
	}
	else if( Key == "%QSL" ){
		StrCopy(sp->qsl, s, MLQSL);
	}
	else if( Key == "%S" ){
		sp->send = *s;
	}
	else if( Key == "%R" ){
		sp->recv = *s;
	}
	else if( Key == "%ENV" ){
		if( sscanf(s, "%u", &d) != 1 ) return FALSE;
		sp->env = WORD(d);
	}
	else if( Key == "%OPT1" ){
		StrCopy(sp->opt1, s, MLOPT);
	}
	else if( Key == "%OPT2" ){
		StrCopy(sp->opt2, s, MLOPT);
	}
	else if( Key == "%USR1" ){
		Log.SetOptStr(2, sp, s);
	}
	else if( Key == "%USR2" ){
		Log.SetOptStr(3, sp, s);
	}
	else if( (Key != "%EOD")&&(Key != "%NULL") ){
		if( Key != s ){
			return FALSE;
		}
	}
	return TRUE;
}
//---------------------------------------------------------------------------
// 1行の入力変換
int CLogText::Text2MMLOG(SDMMLOG *sp, LPSTR p, int &err)
{
	char	bf[512];
	char	dlm = m_Delm ? TAB : ',';
	LPSTR	t;

	memset(sp, 0, sizeof(SDMMLOG));
	int i;
	for( i = 0; i < TEXTCONVMAX; i++ ){
		if( (!m_rConv[i].Key.IsEmpty())&&(m_rConv[i].Key != "%EOD") ){
			if( m_Delm == 2 ){
				StrCopy(bf, p, m_rConv[i].w);
				p += strlen(bf);
				t = bf;
			}
			else {
				p = StrDlm(t, p, dlm);
			}
			clipsp(t);
			t = SkipSpace(t);
			if( ::Text2MMLOG(sp, t, m_rConv[i].Key ) == FALSE ){
				if( !err ){
					ErrorMB(MsgEng ? "An error occurred with conversion type [%s]." : "変換式[%s]でエラーが発生しました.", m_rConv[i].Key.c_str() );
				}
				err++;
			}
		}
		else {
			break;
		}
	}
	if( !sp->etime ) sp->etime = sp->btime;
	if( m_UTC ) UTCtoJST(sp);
	return err ? FALSE : TRUE;
}
//
//
//***************************************************************************
// CLog200 クラス
CLog200::CLog200()
{
	m_Type = 1;
	m_err = 0;
}
//---------------------------------------------------------------------------
// LOG200ファイルのオープン
int CLog200::Open(LPCSTR pName)
{
	Close();
	m_Index = 0;
	m_err = 0;

	m_fp = fopen(pName, "rb");
	if( m_fp != NULL ){
		m_FileName = pName;
		m_Mode = 0;
		return TRUE;
	}
	else {
		ErrorMB("'%s'がオープンできません.", pName);
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// LOG200ファイルの作成
int CLog200::Create(LPCSTR pName)
{
	Close();
	m_Mode = 1;
	m_err = 0;

	int add = 0;
	if( (m_fp = fopen(pName, "rb"))!=NULL ){
		fclose(m_fp);
		m_fp = NULL;
		add = 1;
		char BackName[256];
		strcpy(BackName, pName);
		SetEXT(BackName, ".BAK");
		int r = YesNoCancelMB( "'%s'は既に存在しています.\r\nＭＭＴＴＹはこのファイルにデータを追加します.\r\n\r\n"
							"この操作を実行する前に元のログファイルのバックアップファイル\r\n\r\n"
							"'%s' -> '%s'\r\n\r\nを作成しますか？\r\n\r\n"
							"[重要]\r\n念のためにバックアップを作成する事をお勧めします.", pName, pName, BackName);
		if( r == IDYES ){
			CWaitCursor w;
			if( ::CopyFile(pName, BackName, FALSE) == FALSE ){
				ErrorMB("バックアップの作成に失敗しました.\r\nこの処理は中断されます.元のログファイルは変更されません.");
				return FALSE;
			}
		}
		else if( r == IDCANCEL ){
			return FALSE;
		}
	}
	m_fp = fopen(pName, add ? "ab":"wb");
	if( m_fp != NULL ){
		m_FileName = pName;
		m_Mode = 0;
		return TRUE;
	}
	else {
		ErrorMB( "'%s'が作成できません.", pName);
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// LOG200ファイルのクローズ
int CLog200::Close(void)
{
	int r = 0;
	if( m_fp != NULL ){
		r = fclose(m_fp);
		m_fp = NULL;
		if( r ){
			ErrorMB("'%s'が正しくクローズできませんでした.", m_FileName.c_str());
		}
		else if( m_Mode ){
			InfoMB("'%s'への追加を終了しました.", m_FileName.c_str());
		}
	}
	return r ? FALSE : TRUE;
}
//---------------------------------------------------------------------------
// LOG200 -> MMLOG フォーマットの変換
void LOG200toMMLOG(SDMMLOG *sp, LPSTR s)
{
	LPSTR	t;
	int y, m, d, h, tim;
	char	bf[256];

	memset(sp, 0, sizeof(SDMMLOG));
	s = StrDlm(t, s, 0x1e);	/* DATE	*/
	y = atoin(t, 2); t += 2;
	m = atoin(t, 2); t += 2;
	d = atoin(t, 2);
	sp->year = BYTE(y % 100);
	sp->date = WORD(m * 100 + d);

	s = StrDlm(t, s, 0x1e);		/* BGN	*/
	d = atoin(t, 4);
	h = d / 100;
	m = d % 100;
	tim = (h * 60 + m) * 30;
	if( !tim ) tim++;
	sp->btime = WORD(tim);

	s = StrDlm(t, s, 0x1e);		/* END	*/
	d = atoin(t, 4);
	h = d / 100;
	m = d % 100;
	tim = (h * 60 + m) * 30;
	if( !tim ) tim++;
	sp->etime = WORD(tim);

	s = StrDlm(t, s, 0x1e);		/* CALL	*/
	clipsp(t);
	StrCopy(sp->call, t, MLCALL);

	s = StrDlm(t, s, 0x1e);		/* FREQ	*/
	if( LastC(t) == '.' ){
		*lastp(t) = 0;
	}
	Log.SetFreq(sp, t);

	s = StrDlm(t, s, 0x1e);		/* MODE	*/
	Log.SetMode(sp, t);

	s = StrDlm(t, s, 0x1e);		/* RST	*/
	clipsp(t);
	StrCopy(sp->ur, t, MLRST);

	s = StrDlm(t, s, 0x1e);		/* MyRST*/
	clipsp(t);
	StrCopy(sp->my, t, MLRST);

	s = StrDlm(t, s, 0x1e);		/* QTH	*/
	clipsp(t);
	StrCopy(sp->qth, t, MLQTH);

	s = StrDlm(t, s, 0x1e);		/* NAME	*/
	clipsp(t);
	StrCopy(sp->name, t, MLNAME);

	s = StrDlm(t, s, 0x1e);		/* RIG	*/
	clipsp(t);
	LPSTR pRIG = t;

	s = StrDlm(t, s, 0x1e);		/* CONTEST	*/
	clipsp(t);
	LPSTR pContest = t;

	s = StrDlm(t, s, 0x1e);		/* Rem	*/
	clipsp(t);

	if( (strlen(sp->call) == 15) && (RemoveL2(bf, t, "CALL", sizeof(bf)-1) == TRUE) ){
		StrCopy(sp->call, bf, MLCALL);
	}
	if( RemoveL2(bf, t, "POW", sizeof(bf)-1) == TRUE ){
		StrCopy(sp->pow, bf, MLPOW);
	}
	if( RemoveL2(bf, t, "ENV", sizeof(bf)-1) == TRUE ){
		sp->env = WORD(atoin(bf, -1));
	}
	if( RemoveL2(bf, t, "M", sizeof(bf)-1) == TRUE ){
		sp->cq = bf[0];
	}
	if( RemoveL2(bf, t, "OPT1", sizeof(bf)-1) == TRUE ){
		StrCopy(sp->opt1, bf, MLOPT);
	}
	if( RemoveL2(bf, t, "OPT2", sizeof(bf)-1) == TRUE ){
		StrCopy(sp->opt2, bf, MLOPT);
	}
	if( RemoveL2(bf, t, "USR1", sizeof(bf)-1) == TRUE ){
		Log.SetOptStr(2, sp, bf);
	}
	if( RemoveL2(bf, t, "USR2", sizeof(bf)-1) == TRUE ){
		Log.SetOptStr(3, sp, bf);
	}
	if( RemoveL2(bf, t, "MQSL", sizeof(bf)-1) == TRUE ){
		StrCopy(sp->qsl, bf, MLQSL);
	}
	clipsp(t);
	t = SkipSpace(t);
	StrCopy(sp->rem, t, MLREM);

	s = StrDlm(t, s, 0x1e);		/* MyQTH*/
	if( *t ) AddL2(sp->qsl, "MyQTH", t, '[', ']', MLQSL);

	s = StrDlm(t, s, 0x1e);		/* MyRig*/
	if( *t ) AddL2(sp->qsl, "MyRIG", t, '[', ']', MLQSL);

	s = StrDlm(t, s, 0x1e);		/* Send	*/
	sp->send = *t;
	StrDlm(t, s, 0x1e);			/* Recv	*/
	sp->recv = *t;

	if( *pRIG ) AddL2(sp->qsl, "RIG", pRIG, '[', ']', MLQSL);
	if( *pContest ) AddL2(sp->qsl, "TEST", pContest, '[', ']', MLQSL);
}
//---------------------------------------------------------------------------
// LOG200ファイルの読み出し
int CLog200::Read(SDMMLOG *sp)
{
	if( !IsOpen() ) return FALSE;

	if( fread(m_bf, 1, 200, m_fp) == 200 ){
		LOG200toMMLOG(sp, m_bf);
		m_Index++;
		return TRUE;
	}
	else {
		return FALSE;
	}
}

//---------------------------------------------------------------------------
// MMLOG -> LOG200フォーマット変換
void MMLOGtoLOG200(LPSTR t, SDMMLOG *sp, int &err)
{
	LPSTR s = t;
	int h, m;
	char	bf[256];

	memset(t, ' ', 200);
	// YYMMDD
	sprintf(t, "%02u%02u%02u\x1e", sp->year, sp->date / 100, sp->date % 100);
	t += strlen(t);
	// BGN(HHMM)
	h = sp->btime / 1800;
	m = (sp->btime % 1800) / 30;
	sprintf(t, "%02u%02u\x1e", h, m);
	t += strlen(t);
	// END
	h = sp->etime / 1800;
	m = (sp->etime % 1800) / 30;
	sprintf(t, "%02u%02u\x1e", h, m);
	t += strlen(t);
	// CALL
	StrCopy(t, sp->call, 15);	// 15文字に制限されている
	t += strlen(t);
	*t++ = 0x1e;
	// FREQ
	sprintf(bf, "%s", Log.GetFreqString(sp->band, sp->fq));
	if( bf[0] ){
		double dd;
		if( sscanf(bf, "%lf", &dd) == 1 ){
			sprintf(bf, "%.3lf", dd);
		}
	}
	strcpy(t, bf);
	t += strlen(t);
	*t++ = 0x1e;
	// MODE
	sprintf(t, "%s\x1e", Log.GetModeString(sp->mode));
	t += strlen(t);
	// HisRST
	sprintf(t, "%s\x1e", sp->ur);
	t += strlen(t);
	// MyRST
	sprintf(t, "%s\x1e", sp->my);
	t += strlen(t);
	// QTH
	sprintf(t, "%s\x1e", sp->qth);
	t += strlen(t);
	// NAME
	sprintf(t, "%s\x1e", sp->name);
	t += strlen(t);
	// RIG
	if( RemoveL2(bf, sp->qsl, "Rig", sizeof(bf)-1) == TRUE ){
		strcpy(t, bf);
	}
	*t++ = 0x1e;
	// CONTEST
	if( RemoveL2(bf, sp->qsl, "TEST", sizeof(bf)-1) == TRUE ){
		strcpy(t, bf);
	}
	*t++ = 0x1e;
	char MyRig[200];
	char MyQTH[200];
	MyRig[0] = MyQTH[0] = 0;
	if( RemoveL2(bf, sp->qsl, "MyRig", sizeof(bf)-1) == TRUE ){
		strcpy(MyRig, bf);
	}
	if( RemoveL2(bf, sp->qsl, "MyQTH", sizeof(bf)-1) == TRUE ){
		strcpy(MyQTH, bf);
	}
	clipsp(sp->qsl);
	LPSTR p = SkipSpace(sp->qsl);

	// REM
	strcpy(bf, sp->rem);
	char bbf[128];
	if( strlen(sp->call) > 11 ){
		AddL2(bf, "CALL", sp->call, '[', ']', MLQSL);
	}
	if( sp->cq ){
		bbf[0] = sp->cq; bbf[1] = 0;
		AddL2(bf, "M", bbf, '[', ']', MLQSL);
	}
	if( sp->pow[0] ) AddL2(bf, "POW", sp->pow, '[', ']', MLQSL);
	if( sp->env ){
		sprintf(bbf, "%u", sp->env);
		AddL2(bf, "ENV", bbf, '[', ']', MLQSL);
	}
	if( sp->opt1[0] ) AddL2(bf, "OPT1", sp->opt1, '[', ']', MLQSL);
	if( sp->opt2[0] ) AddL2(bf, "OPT2", sp->opt2, '[', ']', MLQSL);
	LPCSTR pp = Log.GetOptStr(2, sp);
	if( *pp ) AddL2(bf, "USR1", pp, '[', ']', MLQSL);
	pp = Log.GetOptStr(3, sp);
	if( *pp ) AddL2(bf, "USR2", pp, '[', ']', MLQSL);

	if( *p ) AddL2(bf, "MQSL", p, '[', ']', MLQSL);
	int	len = (t - s) + strlen(bf) + 1 + strlen(MyQTH) + 1 + strlen(MyRig) + 1 + 2;
	if( len >= 199 ){
		if( !err ){
			err++;
			WarningMB("変換した結果がLOG200レコード長を越えました.\r\n\r\nリマークフィールドの長さを調整します.");
		}
		len -= 199;
		bf[strlen(bf)-len] = 0;
	}

	sprintf(t, "%s\x1e", bf);
	t += strlen(t);

	// MyQTH
	sprintf(t, "%s\x1e", MyQTH);
	t += strlen(t);
	// MyRIG
	sprintf(t, "%s\x1e", MyRig);
	t += strlen(t);
	// S
	if( sp->send ) *t++ = sp->send;
	*t++ = 0x1e;
	// R
	if( sp->recv ) *t++ = sp->recv;
	*t = 0x1e;

	s[198] = '\r'; s[199] = '\n';
}
//---------------------------------------------------------------------------
// LOG200ファイルの書きこみ
int CLog200::Write(SDMMLOG *sp)
{
	if( !IsOpen() ) return FALSE;

	MMLOGtoLOG200(m_bf, sp, m_err);
	if( fwrite(m_bf, 1, 200, m_fp) != 200 ){
		ErrorMB("'%s'に書き込み中にエラーが発生しました.", m_FileName.c_str());
		return FALSE;
	}
	else {
		m_Index++;
		return TRUE;
	}
}
//
//
//***************************************************************************
// CHamLog クラス
//---------------------------------------------------------------------------
CHamLog::CHamLog()
{
	m_Type = 2;
	m_err = 0;

	m_DBRName = "";		// DBRファイルの名前
	m_dbrfp = NULL;		// DBRファイルのファイルポインタ
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// HamLogファイルのオープン
int CHamLog::Open(LPCSTR pName)
{
	Close();
	m_Index = 0;
	m_err = 0;
	m_Mode = 0;

	m_fp = fopen(pName, "rb");		// DBSのオープン

	if( m_fp == NULL ){
		ErrorMB( "'%s'がオープンできません.", pName);
		return FALSE;
	}
	m_FileName = pName;
	if( (fread(&m_hd, 1, sizeof(m_hd), m_fp)!=sizeof(m_hd)) ||
		(m_hd.Memo != 0x1a) ||
		(m_hd.HeadLen != 449) ||
		(m_hd.DataLen != sizeof(m_RecBuf))
	){
		fclose(m_fp);
		m_fp = NULL;
		ErrorMB("予期しないファイル形式です.");
		return FALSE;
	}
	char dbrName[256];
	strcpy(dbrName, pName);
	SetEXT(dbrName, ".DBR");
	m_DBRName = dbrName;
	m_dbrfp = fopen(dbrName, "rb");
	if( m_dbrfp == NULL ){
		WarningMB( "'%s'がオープンできません.", dbrName);
	}
	else if( fread(&m_dbrhd, 1, sizeof(m_dbrhd), m_dbrfp)!=sizeof(m_dbrhd) ){
		ErrorMB( "'%s'が正常に読みこめません. 処理を中断します.", dbrName);
		fclose(m_fp);
		m_fp = NULL;
		fclose(m_dbrfp);
		m_dbrfp = NULL;
		return FALSE;
	}

	if( Seek(0) == FALSE ){
		Close();
		ErrorMB("最初のレコードが見つかりません.");
		return FALSE;
	}
	return TRUE;
}

int CHamLog::Close(void)
{
	int r = 0;
	if( m_fp != NULL ){
		if( m_Mode ){		// 書きこみ
			fseek(m_fp, 0, SEEK_SET);
			if( fwrite(&m_hd, 1, sizeof(m_hd), m_fp) != sizeof(m_hd) ) r = 1;
		}
		if( fclose(m_fp) ) r = 1;;
		m_fp = NULL;
	}
	if( m_dbrfp != NULL ){
		if( m_Mode ){
			fseek(m_dbrfp, 0, SEEK_SET);
			if( fwrite(&m_dbrhd, 1, sizeof(m_dbrhd), m_dbrfp) != sizeof(m_dbrhd) ) r = 1;
		}
		if( fclose(m_dbrfp) ) r = 1;
		m_dbrfp = NULL;
	}
	if( r ){
		ErrorMB("ファイルのクローズに失敗しました.");
	}
	else if( m_Mode ){
		InfoMB("'%s'への追加を終了しました.", m_FileName.c_str());
	}
	return r ? FALSE : TRUE;
}

//---------------------------------------------------------------------------
// HamLogファイルの作成
int CHamLog::Create(LPCSTR pName)
{
	Close();
	m_Mode = 1;
	m_err = 0;

	char dbrName[256];
	strcpy(dbrName, pName);
	SetEXT(dbrName, ".DBR");

	int add = 0;
	if( (m_fp = fopen(pName, "rt"))!=NULL ){
		fclose(m_fp);
		add = 1;
		char BackNameDBS[256];
		strcpy(BackNameDBS, pName);
		SetEXT(BackNameDBS, ".$BS");
		char BackNameDBR[256];
		strcpy(BackNameDBR, dbrName);
		SetEXT(BackNameDBR, ".$BR");
		int r = YesNoCancelMB( "'%s'は既に存在しています. データはこのファイルに追加されます.\r\n\r\nこの操作を実行する前に元のログファイルのバックアップファイル\r\n\r\n'%s'->'%s'\r\n'%s'->'%s'\r\n\r\nを作成しますか？\r\n\r\n"
					"[重要]\r\n念のためにバックアップを作成する事をお勧めします.", pName, pName, BackNameDBS, dbrName, BackNameDBR);
		if( r == IDYES ){
			CWaitCursor w;
			if( ::CopyFile(pName, BackNameDBS, FALSE) == FALSE ){
				ErrorMB("バックアップの作成に失敗しました.\r\nこの処理は中断されます.元のログファイルは変更されません.");
				return FALSE;
			}
			if( ::CopyFile(dbrName, BackNameDBR, FALSE) == FALSE ){
				ErrorMB("バックアップの作成に失敗しました.\r\nこの処理は中断されます.元のログファイルは変更されません.");
				return FALSE;
			}
		}
		else if( r == IDCANCEL ){
			return FALSE;
		}
	}
	m_fp = fopen(pName, add ? "r+b":"wb");
	if( m_fp == NULL ){
		ErrorMB( "'%s'がオープンできません.", pName);
		return FALSE;
	}
	m_dbrfp = fopen(dbrName, add ? "r+b":"wb");
	if( m_dbrfp == NULL ){
		ErrorMB( "'%s'がオープンできません.", dbrName);
		return FALSE;
	}

	if( add ){		// 追加の時
		if( (fread(&m_hd, 1, sizeof(m_hd), m_fp)!=sizeof(m_hd)) ||
			(m_hd.Memo != 0x1a) ||
			(m_hd.HeadLen != 449) ||
			(m_hd.DataLen != sizeof(m_RecBuf))
		){
			fclose(m_fp);
			m_fp = NULL;
			fclose(m_dbrfp);
			m_dbrfp = NULL;
			ErrorMB("'%s'は予期しないファイル形式です.", pName);
			return FALSE;
		}
		if( (fread(&m_dbrhd, 1, sizeof(m_dbrhd), m_dbrfp)!=sizeof(m_dbrhd)) ||
			(m_dbrhd.Memo != 0x1a) ||
			(m_dbrhd.term != 0x1a)
		){
			fclose(m_fp);
			m_fp = NULL;
			fclose(m_dbrfp);
			m_dbrfp = NULL;
			ErrorMB("'%s'は予期しないファイル形式です.", dbrName);
			return FALSE;
		}
		fseek(m_dbrfp, 0, SEEK_END);	// DBRの最後に移動
	}
	else {			// 新規の時
		MakeHD();
	}
	m_FileName = pName;
	m_Mode = 1;
	m_DBRName = dbrName;
	m_Index = m_hd.Max;
	return TRUE;
}

//----------------------------------------------------------------------
int CHamLog::Seek(DWORD Index)
{
	long Pos = 449 + (Index * sizeof(m_RecBuf));
	if( fseek(m_fp, Pos, SEEK_SET) ) return FALSE;
	m_Index = Index;
	return TRUE;
}

//----------------------------------------------------------------------
//ＨＡＭＬＯＧのヘッダを作成
BOOL CHamLog::MakeHD(void)
{
	memset(&m_hd, 0, sizeof(m_hd));
	memset(&m_dbrhd, 0, sizeof(m_dbrhd));
	m_hd.Memo = 0x1a;
	SYSTEMTIME st;
	::GetLocal(&st);
	m_hd.YY = BYTE(st.wYear % 100);
	m_hd.MM = BYTE(st.wMonth);
	m_hd.DD = BYTE(st.wDay);
	m_hd.Max = 0;
	memcpy(&m_dbrhd, &m_hd, sizeof(m_hd));
	m_hd.HeadLen = 449;
	m_hd.DataLen = sizeof(m_RecBuf);
	if( fwrite(&m_hd, 1, sizeof(m_hd), m_fp) != sizeof(m_hd) ) return FALSE;

	LPCSTR _Name[]={"CALLS","IGN","POTBL","CODE","GL","FREQ","MODE","NAME","QSL","DATE","TIME","RST","OFS"};
	BYTE	_Len[]={6, 1, 3, 6, 6, 4, 3, 12, 3, 3, 2, 4, 4, 0};
	DBSLOT	slot;
	for( int i = 0; _Len[i]; i++ ){
		memset(&slot, 0, sizeof(slot));
		strcpy(slot.Name, _Name[i]);
		slot.Type = 'C';
		slot.Len = _Len[i];
		if( fwrite(&slot, 1, sizeof(slot), m_fp)!=sizeof(slot) ) return FALSE;
	}
	slot.Name[0] = 0x0d;
	if( fwrite(&slot, 1, 1, m_fp)!=1 ) return FALSE;

	m_dbrhd.m1 = 1;
	m_dbrhd.term = 0x1a;
	if( fwrite(&m_dbrhd, 1, sizeof(m_dbrhd), m_dbrfp)!=sizeof(m_dbrhd) ) return FALSE;
	return TRUE;
}

//---------------------------------------------------------------------------
// HamLogファイルの読み出し
int CHamLog::Read(SDMMLOG *sp)
{
	if( !IsOpen() ) return FALSE;

	Seek(m_Index);
	if( fread(&m_RecBuf, 1, sizeof(m_RecBuf), m_fp) == sizeof(m_RecBuf) ){
		HAMLOGtoMMLOG(sp, &m_RecBuf, m_dbrfp);
		m_Index++;
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// HamLogファイルの書きこみ
int CHamLog::Write(SDMMLOG *sp)
{
	if( !IsOpen() ) return FALSE;

	if( MMLOGtoHAMLOG(&m_RecBuf, sp, m_dbrfp) == FALSE ){
		ErrorMB("'%s'に書き込み中にエラーが発生しました.", m_DBRName.c_str());
		return FALSE;
	}
	Seek(m_hd.Max);
	if( fwrite(&m_RecBuf, 1, sizeof(m_RecBuf), m_fp) != sizeof(m_RecBuf) ){
		ErrorMB("'%s'に書き込み中にエラーが発生しました.", m_FileName.c_str());
		return FALSE;
	}
	else {
		m_hd.Max++;
		m_dbrhd.Max++;
		return TRUE;
	}
}
//----------------------------------------------------------------------
//ＨＡＭＬＯＧの周波数記録方式を通常の文字列に変換
void DecBand(LPSTR t, BYTE *pBand)
{
	if( pBand[3] & 0x80 ){	// ５～７文字の文字列
		int DotPos = pBand[3] & 0x7f;
		int Len = (DotPos >> 3) & 0x07;
		DotPos &= 0x07;
		LPSTR p = t;
		wsprintf(p, "%-7lu", (*((LONG *)pBand) & 0x00ffffffL));
		if( Len < 4 ){		// Ver3.19までの記録方式
			for(int i = 6; i > 3; i--){
				if(p[i]>'0'){
					break;
				}
				else if(p[i] == '0'){
					p[i] = ' ';
				}
			}
		}
		else {
			if( Len < 6 ) p[6] = ' ';
			if( Len == 4 ) p[5] = ' ';
		}
		p[DotPos] = '.';
		p[8] = 0;
	}
	else {					// そのまま
		StrCopy(t, (LPCSTR)pBand, 4);
	}
	clipsp(t);
	if( LastC(t) == '.' ){
		*lastp(t) = 0;
	}
}

void SetMMLOGKey(SDMMLOG *sp, LPSTR bf)
{
	char rbf[512];

	if( RemoveL2(rbf, bf, "ToRadio", sizeof(rbf)-1) == TRUE ){
		StrCopy(sp->call, rbf, MLCALL);
	}
	else if( RemoveL2(rbf, bf, "CALL", sizeof(rbf)-1) == TRUE ){
		StrCopy(sp->call, rbf, MLCALL);
	}

	if( RemoveL2(rbf, bf, "ENV", sizeof(rbf)-1) == TRUE ){
		sp->env = WORD(atoin(rbf, -1));
	}
	if( RemoveL2(rbf, bf, "END", sizeof(rbf)-1) == TRUE ){
		int tim = atoin(rbf, -1);
		int hh = tim / 100;
		int mm = tim % 100;
		sp->etime = WORD((hh * 60 + mm) * 30);
		if( !sp->etime ) sp->etime++;
	}
	if( RemoveL2(rbf, bf, "POW", sizeof(rbf)-1) == TRUE ){
		StrCopy(sp->pow, rbf, MLPOW);
	}
	if( RemoveL2(rbf, bf, "M", sizeof(rbf)-1) == TRUE ){
		sp->cq = rbf[0];
	}
	if( RemoveL2(rbf, bf, "SN", sizeof(rbf)-1) == TRUE ){
		int l = GetLMode(sp->mode);
		if( l ) sp->ur[l] = 0;
		strcat(sp->ur, rbf);
	}
	if( RemoveL2(rbf, bf, "RN", sizeof(rbf)-1) == TRUE ){
		int l = GetLMode(sp->mode);
		if( l ) sp->my[l] = 0;
		strcat(sp->my, rbf);
	}
}
//---------------------------------------------------------------------------
// HamLog -> MMLOG フォーマットの変換
void HAMLOGtoMMLOG(SDMMLOG *sp, SDHAMLOG *hp, FILE *dbrfp)
{
	int		CallOrder = 0;
	char	bf[512];

	memset(sp, 0, sizeof(SDMMLOG));
	DecBand(bf, (BYTE *)hp->freq);
	Log.SetFreq(sp, bf);

	StrCopy(bf, hp->mode, 3);
	clipsp(bf);
	if( !strcmp(bf, Log.m_LogSet.m_THRTTY.c_str()) || !strcmp(bf, "RTY") || !strcmp(bf, "TTY") || !strcmp(bf, "RTT") ){
		strcpy(bf, "RTTY");
	}
	else if( !strcmp(bf, Log.m_LogSet.m_THSSTV.c_str()) || !strcmp(bf, "STV")|| !strcmp(bf, "SST") ){
		strcpy(bf, "SSTV");
	}
	else if( !strcmp(bf, "FTV") ){
		strcpy(bf, "FSTV");
	}
	Log.SetMode(sp, bf);

	StrCopy(sp->name, hp->name, 12);
	clipsp(sp->name);

	sprintf(sp->ur, "%02u", BYTE(hp->hiss[0]));
	if( hp->hiss[1] != ' ' ) sp->ur[2] = hp->hiss[1];

	sprintf(sp->my, "%02u", BYTE(hp->myrs[0]));
	if( hp->myrs[1] != ' ' ) sp->my[2] = hp->myrs[1];

	int YY, MM, DD, HH, mm;
	YY = hp->date[0] & 0x007f;
	YY %= 100;
	MM = hp->date[1];
	DD = hp->date[2];
	HH = hp->time[0];
	mm = hp->time[1] & 0x7f;
	if( hp->time[1] & 0x80 ){
		UTCtoJST(YY, MM, DD, HH);
	}
	sp->year = BYTE(YY);
	sp->date = WORD(MM * 100 + DD);
	sp->btime = WORD(((HH * 60 + mm) * 30));
	if(!sp->btime) sp->btime++;

	if( hp->ofs && (dbrfp != NULL) ){
		FHDDBR	fhdbr;
		if( fseek(dbrfp, hp->ofs, SEEK_SET) ) goto _ex;
		if( fread(&fhdbr, 1, sizeof(fhdbr), dbrfp) != sizeof(fhdbr) ){
			goto _ex;
		}
		if( fhdbr.LenQTH ){
			if( fread(bf, 1, fhdbr.LenQTH, dbrfp)!=size_t(fhdbr.LenQTH) ){
				goto _ex;
			}
			bf[fhdbr.LenQTH] = 0;
			clipsp(bf);
			StrCopy(sp->qth, bf, MLQTH);
		}
		if( fhdbr.LenREM1 ){
			if( fread(bf, 1, fhdbr.LenREM1, dbrfp)!=size_t(fhdbr.LenREM1) ){
				goto _ex;
			}
			bf[fhdbr.LenREM1] = 0;
			SetMMLOGKey(sp, bf);
			clipsp(bf);
			if( strstr(bf, "$DX" ) != NULL ) CallOrder = 1;
			StrCopy(sp->rem, SkipSpace(bf), MLREM);
		}
		if( fhdbr.LenREM2 ){
			if( fread(bf, 1, fhdbr.LenREM2, dbrfp)!=size_t(fhdbr.LenREM2) ){
				goto _ex;
			}
			bf[fhdbr.LenREM2] = 0;
			SetMMLOGKey(sp, bf);
			clipsp(bf);
			if( strstr(bf, "$DX" ) != NULL ) CallOrder = 1;
			StrCopy(sp->qsl, SkipSpace(bf), MLQSL);
		}
	}
_ex:;
	sprintf(bf, "QSL[%.3s]", hp->qsl);
	Log.SetOptStr(2, sp, bf);
	if( hp->qsl[0] == 'N' ){
		sp->send = 'N';
	}
	else if( hp->send[0] == ' ' ){	// 未発送
		if( (hp->qsl[0] == 'J')||(hp->qsl[0] == ' ') ){
			sp->send = 0;
		}
		else {
			sp->send = BYTE(tolower(hp->qsl[0]));
		}
	}
	else {							// 発送済み
		sp->send = BYTE(toupper(hp->send[0]));
	}
	sp->recv = hp->rcv[0];
	if( sp->recv == ' ' ) sp->recv = 0;

	StrCopy(sp->opt1, hp->code, 6);
	StrCopy(sp->opt2, hp->glid, 6);

	if( !sp->call[0] ){
		if( CallOrder && (hp->potbl[0] != ' ') ){	/* KH6/JE3HHTの形式	*/
			StrCopy(bf, hp->potbl, 3);
			clipsp(bf);
			strcat(sp->call, bf);
			if( sp->call[0] ) strcat(sp->call, "/");
			StrCopy(bf, hp->calls, 7);
			clipsp(bf);
			strcat(sp->call, bf);
		}
		else {											/* JE3HHT/KH6の形式	*/
			StrCopy(bf, hp->calls, 7);
			clipsp(bf);
			strcat(sp->call, bf);
			if( hp->potbl[0] != ' ' ){
				if( sp->call[0] ) strcat(sp->call, "/");
				StrCopy(bf, hp->potbl, 3);
				clipsp(bf);
				strcat(sp->call, bf);
			}
		}
	}
	if( !sp->etime ) sp->etime = sp->btime;
}
//----------------------------------------------------------------------
//文字列のコピー（ヌルはセットされない）
void SpaceCopy(LPSTR t, LPCSTR s, int n)
{
	for( ; n && *s; s++, t++, n-- ){
		*t = *s;
	}
}
//----------------------------------------------------------------------
//ＲＳＴ文字列をＨＡＭＬＯＧ形式に変換して格納
void SetRST(char *pRST, LPCSTR p, BYTE mode)
{
	char	rs[3];
	int		rst;

	StrCopy(rs, p, 2);
	if( sscanf(rs, "%d", &rst) == 1 ){
		*pRST = BYTE(rst);
	}
	else {
		*pRST = 59;
	}
	if( strlen(p) < 3 ) return;
	if( GetLMode(mode) == 2 ) return;
	pRST[1] = *(p+2);
}
//----------------------------------------------------------------------
//周波数の文字列をＨＡＭＬＯＧの周波数記録方式に変換
void EncBand(BYTE *pBand, LPCSTR pStr)
{
	char	bf[32];
	StrCopy(bf, pStr, 7);

	int Len = strlen(bf);
	if( Len <= 4 ){
		memset(pBand, ' ', 4);
		SpaceCopy(LPSTR(pBand), bf, 4);
	}
	else {
		LPSTR	p = strchr(bf, '.');
		int	DotPos;
		if( p != NULL ){
			DotPos = p - bf;
			*p = '0';
		}
		else {
			DotPos = Len;
		}
		long Freq;
		sscanf(bf, "%lu", &Freq);
		*(ULONG *)pBand = Freq;
		pBand[3] = BYTE(0x0080 | (Len << 3) | DotPos);
	}
}

void AddMMLOGKey(AnsiString &REM1, AnsiString &REM2, LPCSTR s, LPCSTR pKey)
{
	if( !*s ) return;

	int len1 = strlen(REM1.c_str());
	int len2 = strlen(REM2.c_str());
	int len = strlen(s);
	if( pKey == NULL ){
		len++;
	}
	else {
		len += strlen(pKey) + 3;
	}
	AnsiString *ap;
	if( (len2 + len) < 54 ){
		ap = &REM2;
	}
	else if( (len1 + len) < 54 ){
		ap = &REM1;
	}
	else {
		return;
	}
	if( !ap->IsEmpty() ){
		*ap += " ";
	}
	if( pKey != NULL ){
		*ap += pKey;
		*ap += '[';
	}
	*ap += s;
	if( pKey != NULL ){
		*ap += ']';
	}
}
//---------------------------------------------------------------------------
// MMLOG -> HamLog フォーマットの変換
int MMLOGtoHAMLOG(SDHAMLOG *hp, SDMMLOG *sp, FILE *dbrfp)
{
	char bf[512];

	int CallOrder = FALSE;
	memset(hp, ' ', sizeof(SDHAMLOG));
	int YY = sp->year;
	int MM = sp->date / 100;
	int DD = sp->date % 100;
	int tim = sp->btime / 30;
	int HH = tim / 60;
	int mm = tim % 60;
	switch(Log.m_LogSet.m_THTZ){
		case 0:
			if( !IsJA(sp->call) ){
				JSTtoUTC(YY, MM, DD, HH);
				mm |= 0x80;
			}
			break;
		case 1:
			JSTtoUTC(YY, MM, DD, HH);
			mm |= 0x80;
			break;
		default:
			break;
	}
	hp->date[0] = BYTE(YY < 50 ? YY + 128 : YY);
	hp->date[1] = BYTE(MM);
	hp->date[2] = BYTE(DD);
	hp->time[0] = BYTE(HH);
	hp->time[1] = BYTE(mm);

	SpaceCopy(hp->name, sp->name, 12);
	SpaceCopy(hp->code, sp->opt1, 6);
	SpaceCopy(hp->glid, sp->opt2, 6);
	AnsiString	QTH = sp->qth;
	AnsiString	REM1 = sp->rem;
	AnsiString	REM2 = sp->qsl;

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
		SpaceCopy(hp->potbl, pp, 3);
	}
	else if( strlen(t) > 7 ){
		AddMMLOGKey(REM1, REM2, sp->call, "ToRadio");
	}
	SpaceCopy(hp->calls, t, 7);
	strcpy(bf, Log.GetModeString(sp->mode));
	if( !strcmp(bf, "RTTY") ){
		StrCopy(bf, Log.m_LogSet.m_THRTTY.c_str(), 3);
	}
	else if( !strcmp(bf, "SSTV") ){
		StrCopy(bf, Log.m_LogSet.m_THSSTV.c_str(), 3);
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
	SpaceCopy(hp->mode, bf, 3);
	SetRST(hp->hiss, sp->ur, sp->mode);
	SetRST(hp->myrs, sp->my, sp->mode);

	int l = GetLMode(sp->mode);
	if( sp->ur[l] ) AddMMLOGKey(REM1, REM2, &sp->ur[l], "SN");
	if( sp->my[l] ) AddMMLOGKey(REM1, REM2, &sp->my[l], "RN");

	strcpy(bf, Log.GetFreqString(sp->band, sp->fq));
	EncBand((BYTE *)hp->freq, bf);


	if( CallOrder && (!strstr(sp->rem, "$DX")) && (!strstr(sp->qsl, "$DX")) ){
		AddMMLOGKey(REM1, REM2, "$DX", NULL);
	}

	if( sp->etime ){
		tim = sp->etime / 30;
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
	strcpy(bf, Log.GetOptStr(2, sp));
	char rbf[32];
	if(RemoveL2(rbf, bf, "QSL", sizeof(rbf)-1) == TRUE){
		hp->qsl[0] = rbf[0];
		hp->send[0] = rbf[1];
		hp->rcv[0] = rbf[2];
	}
	else {
		hp->rcv[0] = sp->recv;
		if( !hp->rcv[0] ) hp->rcv[0] = ' ';
		if( sp->send == 'N' ){
			hp->qsl[0] = 'N';
			hp->send[0] = ' ';
		}
		else if( !sp->send ){
			hp->qsl[0] = 'J';
			hp->send[0] = ' ';
		}
		else if( (sp->send >= 0x60)||(sp->send == '?') ){
			hp->qsl[0] = BYTE(toupper(sp->send));
			hp->send[0] = ' ';
		}
		else if( isalpha(sp->send) ){
			hp->qsl[0] = sp->send;
			hp->send[0] = '*';
		}
		else {
			hp->qsl[0] = 'J';
			hp->send[0] = sp->send;
		}
	}

	if( !QTH.IsEmpty() || !REM1.IsEmpty() || !REM2.IsEmpty() ){
		FHDDBR	fhdbr;
		memset(&fhdbr, 0, sizeof(fhdbr));
		hp->ofs = ftell(dbrfp);
		fhdbr.LenQTH = BYTE(strlen(QTH.c_str()));
		fhdbr.LenREM1 = BYTE(strlen(REM1.c_str()));
		fhdbr.LenREM2 = BYTE(strlen(REM2.c_str()));
		if( fhdbr.LenQTH > 28 ){
			fhdbr.LenQTH = 28;
		}
		if( fhdbr.LenREM1 > 54 ){
			fhdbr.LenREM1 = 54;
		}
		if( fhdbr.LenREM2 > 54 ){
			fhdbr.LenREM2 = 54;
		}
		if( fwrite(&fhdbr, 1, sizeof(fhdbr), dbrfp) != sizeof(fhdbr) ) return FALSE;
		if( fhdbr.LenQTH ){
			if( fwrite(QTH.c_str(), 1, fhdbr.LenQTH, dbrfp) != size_t(fhdbr.LenQTH) ) return FALSE;
		}
		if( fhdbr.LenREM1 ){
			if( fwrite(REM1.c_str(), 1, fhdbr.LenREM1, dbrfp) != size_t(fhdbr.LenREM1) ) return FALSE;
		}
		if( fhdbr.LenREM2 ){
			if( fwrite(REM2.c_str(), 1, fhdbr.LenREM2, dbrfp) != size_t(fhdbr.LenREM2) ) return FALSE;
		}
	}
	else {
		hp->ofs = 0L;
	}
	return TRUE;
}

//***************************************************************************
// CLogADIF クラス
CLogADIF::CLogADIF()
{
	m_bf[sizeof(m_bf)-1] = 0;
}
//---------------------------------------------------------------------------
// ADIFファイルのオープン
int CLogADIF::Open(LPCSTR pName)
{
	Close();
	m_Mode = 0;

	m_p = NULL;
	m_conv = 1;
	m_fp = fopen(pName, "rt");
	if( m_fp != NULL ){
		setvbuf(m_fp, NULL, _IOFBF, 16384);
		m_FileName = pName;
		m_Mode = 0;
		return TRUE;
	}
	else {
		ErrorMB(MsgEng ? "Can't open '%s'": "'%s'がオープンできません.", pName);
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// ADIFファイルの作成
int CLogADIF::Create(LPCSTR pName)
{
	Close();
	m_Mode = 1;

	m_fp = fopen(pName, "wt");
	if( m_fp != NULL ){
		setvbuf(m_fp, NULL, _IOFBF, 16384);
		m_FileName = pName;
		m_Mode = 1;
		fprintf(m_fp, "ADIF Export from %s\n", VERTTL);
		fprintf(m_fp, "\n<EOH>\n");
		return TRUE;
	}
	else {
		ErrorMB( MsgEng ? "Can't write to '%s'":"'%s'が作成できません.", pName);
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// テキストファイルのオープン
int CLogADIF::Close(void)
{
	int r = 0;
	if( m_fp != NULL ){
		r = fclose(m_fp);
		m_fp = NULL;
		if( r ){
			ErrorMB(MsgEng ? "Can't write to '%s'":"'%s'が正しくクローズできませんでした.", m_FileName.c_str());
		}
		else if( m_Mode ){
			InfoMB(MsgEng ? "Done (write to '%s')":"'%s'への書きこみを終了しました.", m_FileName.c_str());
		}
	}
	return r ? FALSE : TRUE;
}
const char	*_BandText[]={
	"","160m","80m","80m","40m","30m","20m","17m","15m","12m","10m","6m",
	"2m","70cm","23cm","13cm","6cm","3cm","3cm","1.25cm","6mm",
	"4mm","2mm","1mm","65m","1.25m",NULL,
};
//---------------------------------------------------------------------------
// ADIFファイルの読み出し
void CLogADIF::SetData(SDMMLOG *sp, LPCSTR pKey, LPSTR pData)
{
	int l;

	if( !strcmpi(pKey, "CALL") ){
		StrCopy(sp->call, pData, MLCALL);
	}
	else if( !strcmpi(pKey, "QSO_DATE") ){
		sp->year = BYTE(atoin(pData, 4) % 100);
		sp->date = WORD(atoin(pData+4, 4));
	}
	else if( !strcmpi(pKey, "TIME_ON") ){
		int hh, mm, ss;
		hh = atoin(pData, 2);
		mm = atoin(pData+2, 2);
		ss = pData[4] ? atoin(pData+4, 2) : 0;
		sp->btime = WORD((hh * 60 + mm) * 30 + ss);
		if( !sp->btime ) sp->btime++;
	}
	else if( !strcmpi(pKey, "TIME_OFF") ){
		int hh, mm, ss;
		hh = atoin(pData, 2);
		mm = atoin(pData+2, 2);
		ss = pData[4] ? atoin(pData+4, 2) : 0;
		sp->btime = WORD((hh * 60 + mm) * 30 + ss);
		if( !sp->etime ) sp->btime++;
	}
	else if( !strcmpi(pKey, "FREQ") ){
		Log.SetFreq(sp, pData);
	}
	else if( !strcmpi(pKey, "BAND") ){
		if( !sp->band ){
			int i;
			for( i = 1; _BandText[i] != NULL; i++ ){
				if( !strcmpi(_BandText[i], pData) ) break;
			}
			if( _BandText[i] == NULL ) i = 0;
			sp->band = BYTE(i);
		}
	}
	else if( !strcmpi(pKey, "MODE") ){
		Log.SetMode(sp, pData);
	}
	else if( !strcmpi(pKey, "TX_PWR") ){
		StrCopy(sp->pow, pData, MLPOW);
	}
	else if( !strcmpi(pKey, "RST_SENT") ){
		if( sp->ur[0] ){
			char bf[MLRST+1];
			strcpy(bf, sp->ur);
			StrCopy(sp->ur, pData, MLRST);
			l = strlen(sp->ur);
			StrCopy(&sp->ur[l], bf, MLRST - l);
		}
		else {
			StrCopy(sp->ur, pData, MLRST);
		}
	}
	else if( !strcmpi(pKey, "RST_RCVD") ){
		if( sp->my[0] ){
			char bf[MLRST+1];
			strcpy(bf, sp->my);
			StrCopy(sp->my, pData, MLRST);
			l = strlen(sp->my);
			StrCopy(&sp->my[l], bf, MLRST - l);
		}
		else {
			StrCopy(sp->my, pData, MLRST);
		}
	}
	else if( !strcmpi(pKey, "STX") ){
		l = strlen(sp->ur);
		StrCopy(&sp->ur[l], pData, MLRST - l);
	}
	else if( !strcmpi(pKey, "SRX") ){
		l = strlen(sp->my);
		StrCopy(&sp->my[l], pData, MLRST - l);
	}
	else if( !strcmpi(pKey, "QSL_SENT") ){
		if( *pData == ' ' ) *pData = 0;
		sp->send = *pData;
	}
	else if( !strcmpi(pKey, "QSL_RCVD") ){
		if( *pData == ' ' ) *pData = 0;
		sp->recv = *pData;
	}
	else if( !strcmpi(pKey, "NAME") ){
		StrCopy(sp->name, pData, MLNAME);
	}
	else if( !strcmpi(pKey, "QTH") ){
		StrCopy(sp->qth, pData, MLQTH);
	}
	else if( !strcmpi(pKey, "COMMENT") ){
		StrCopy(sp->rem, pData, MLREM);
	}
	else if( !strcmpi(pKey, "QSLMSG") ){
		StrCopy(sp->qsl, pData, MLQSL);
	}
	else if( !strcmpi(pKey, "CONT") ){
		StrCopy(sp->opt2, pData, MLOPT);
	}
    else if( !strcmpi(pKey, "QSL_VIA") ){
		AddL2(sp->qsl, "QSL", pData, '[', ']', MLQSL);
    }
}
//---------------------------------------------------------------------------
void CLogADIF::AdjustData(SDMMLOG *sp)
{
	if( !sp->etime ) sp->etime = sp->btime;
	UTCtoJST(sp);
	if( sp->call[0] ){
		LPCSTR pCC = ClipCC(sp->call);
		Log.SetOptStr(0, sp, Cty.GetCountry(pCC));
		if( !sp->opt2[0] ) Log.SetOptStr(1, sp, Cty.GetCont(pCC));
	}
	if( !sp->ur[0] ) StrCopy(sp->ur, "599", GetLMode(sp->mode));
	if( !sp->my[0] ) StrCopy(sp->my, "599", GetLMode(sp->mode));
}
//---------------------------------------------------------------------------
// ADIFファイルの読み出し
int CLogADIF::Read(SDMMLOG *sp)
{
	if( !IsOpen() ) return FALSE;

	LPSTR	p, t, tt;
	char	bf[1024];

	while(1){
		if( (m_p == NULL) || !(*m_p) ){
			if( !feof(m_fp) ){
				if( fgets(m_bf, sizeof(m_bf), m_fp) != NULL ){
					ClipLF(m_bf);
					m_p = m_bf;
				}
				else if( sp->btime && sp->call[0] && m_conv ){
					AdjustData(sp);
					m_p = NULL;
					return TRUE;
				}
				else {
					return FALSE;
				}
			}
			else {
				return FALSE;
			}
		}
		m_p = SkipSpace(m_p);
		if( *m_p == '<' ){
			m_p = StrDlm(p, m_p+1, '>');
			p = StrDlm(t, p, ':');
			p = StrDlm(tt, p, ':');
			bf[0] = 0;
			int n = *tt ? atoin(tt, -1) : 0;
			if( (n >= 0) && (n < 1023) && (n <= (int)strlen(m_p)) ){
				if( n ) memcpy(bf, m_p, n);
				bf[n] = 0;
			}
			if( !strcmpi(t, "EOR") ){
				if( m_conv ){
					AdjustData(sp);
					return TRUE;
				}
			}
			if( !strcmpi(t, "EOH") ){
				m_conv = 1;
			}
			else if( m_conv ){
				SetData(sp, t, bf);
#if 0
				FILE	*fp = fopen("F:\\TEST.TXT", "at");
				fprintf(fp, "%s:%s\n", t, bf);
				fclose(fp);
#endif
				m_p += n;
			}
		}
		else if( *m_p ){
			m_p++;
		}
	}
}
//---------------------------------------------------------------------------
void CLogADIF::OutF(int &col, FILE *fp, LPCSTR fmt, ...)
{
	va_list	pp;
	char	bf[512];

	va_start(pp, fmt);
	vsprintf(bf, fmt, pp );
	va_end(pp);
	if( col ){
		fputs(" ", fp);
		col++;
	}
	int l = strlen(bf);
	if( (col + l) > 80 ){
		fputs("\n", fp);
		col = 0;
	}
	fputs(bf, fp);
	col += l;
}
//---------------------------------------------------------------------------
// 出力変換
//
//<CALL:6>KD4MUL <QSO_DATE:8:D>19930921 <TIME_ON:6>223558 <TIME_OFF:6>150000
//<FREQ:5>3.690 <BAND:3>80M <MODE:2>CW <TX_PWR:3>100 <RST_SENT:3>599
//<RST_RCVD:3>599 <QSL_SENT:1>Y <QSL_RCVD:1>N <NAME:3>ROY <QTH:6>STRUNK
//<STATE:2>KY <COMMENT:16>FIRST CW CONTACT <EOR> <CALL:6>WB4TXW
int CLogADIF::Write(SDMMLOG *sp)
{
	if( !IsOpen() ) return FALSE;

	int col = 0;
	JSTtoUTC(sp);
	OutF(col, m_fp, "<CALL:%u>%s", strlen(sp->call), sp->call);
	OutF(col, m_fp, "<QSO_DATE:8:D>%04u%02u%02u", YEAR(sp->year), sp->date/100, sp->date%100);
	int tim = sp->btime / 30;
	OutF(col, m_fp, "<TIME_ON:6>%02u%02u%02u", tim/60, tim%60, (sp->btime % 30)*2);
	tim = sp->etime / 30;
	OutF(col, m_fp, "<TIME_OFF:6>%02u%02u%02u", tim/60, tim%60, (sp->etime % 30)*2);
	LPCSTR p = Log.GetFreqString(sp->band, sp->fq);
	if( *p ) OutF(col, m_fp, "<FREQ:%u>%s", strlen(p), p);
	p = _BandText[sp->band];
	if( *p ) OutF(col, m_fp, "<BAND:%u>%s", strlen(p), p);
	p = Log.GetModeString(sp->mode);
	if( *p ) OutF(col, m_fp, "<MODE:%u>%s", strlen(p), p);
	if( sp->pow[0] ) OutF(col, m_fp, "<TX_PWR:%u>%s", strlen(sp->pow), sp->pow);
	int l = GetLMode(sp->mode);
	if( Log.m_LogSet.m_ClipRSTADIF ){
		if( sp->ur[l] ) OutF(col, m_fp, "<STX:%u>%s", strlen(&sp->ur[l]), &sp->ur[l]);
		if( sp->my[l] ) OutF(col, m_fp, "<SRX:%u>%s", strlen(&sp->my[l]), &sp->my[l]);
		sp->ur[GetLMode(sp->mode)] = 0;
		sp->my[GetLMode(sp->mode)] = 0;
	}
	if( sp->ur[0] ) OutF(col, m_fp, "<RST_SENT:%u>%s", strlen(sp->ur), sp->ur);
	if( sp->my[0] ) OutF(col, m_fp, "<RST_RCVD:%u>%s", strlen(sp->my), sp->my);
	if( sp->send ) OutF(col, m_fp, "<QSL_SENT:1>%c", sp->send);
	if( sp->recv ) OutF(col, m_fp, "<QSL_RCVD:1>%c", sp->recv);
	if( sp->name[0] ) OutF(col, m_fp, "<NAME:%u>%s", strlen(sp->name), sp->name);
	if( sp->qth[0] ) OutF(col, m_fp, "<QTH:%u>%s", strlen(sp->qth), sp->qth);
	if( sp->rem[0] ) OutF(col, m_fp, "<COMMENT:%u>%s", strlen(sp->rem), sp->rem);
	if( sp->opt2[0] ) OutF(col, m_fp, "<CONT:%u>%s", strlen(sp->opt2), sp->opt2);
	if( sp->qsl[0] ){
    	char via[MLQSL+1];
    	char qsl[MLQSL+1];
        StrCopy(qsl, sp->qsl, MLQSL);
		if( !RemoveL2(via, qsl, "QSL", sizeof(via)-1) ){
        	via[0] = 0;
		}
    	if( qsl[0] ) OutF(col, m_fp, "<QSLMSG:%u>%s", strlen(qsl), qsl);
        if( via[0] ) OutF(col, m_fp, "<QSL_VIA:%u>%s", strlen(via), via);
    }
	OutF(col, m_fp, "<EOR>\n");
	return ferror(m_fp) ? FALSE : TRUE;
}


//***************************************************************************
// CLogCabrillo クラス
CLogCabrillo::CLogCabrillo()
{
	m_bf[sizeof(m_bf)-1] = 0;
}
//---------------------------------------------------------------------------
// LogCabrilloファイルのオープン
int CLogCabrillo::Open(LPCSTR pName)
{
	Close();
	m_Mode = 0;

	m_p = NULL;
	m_SNR = "";
	m_fp = fopen(pName, "rt");
	if( m_fp != NULL ){
		setvbuf(m_fp, NULL, _IOFBF, 16384);
		m_FileName = pName;
		m_Mode = 0;
		return TRUE;
	}
	else {
		ErrorMB(MsgEng ? "Can't open '%s'": "'%s'がオープンできません.", pName);
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// LogCabrilloファイルの作成
int CLogCabrillo::Create(LPCSTR pName)
{
	Close();
	m_Mode = 1;

	m_fp = fopen(pName, "wt");
	if( m_fp != NULL ){
		setvbuf(m_fp, NULL, _IOFBF, 16384);
		m_FileName = pName;
		m_Mode = 1;
		fprintf( m_fp, "START-OF-LOG: 2.0\n");
		fprintf( m_fp, "ARRL-SECTION: \n" );
		fprintf( m_fp, "CONTEST: <== e.g. ARRL-RTTY, CQ-WW-RTTY, CQ-WPX-RTTY, BARTG-SPRINT, BARTG-RTTY\n");
		fprintf( m_fp, "CALLSIGN: %s\n", sys.m_Call.c_str() );
		fprintf( m_fp, "CATEGORY: <== e.g. SINGLE-OP ALL HIGH, SINGLE-OP-ASSISTED 20M LOW\n" );
		fprintf( m_fp, "CLAIMED-SCORE: \n");
		fprintf( m_fp, "OPERATORS:\n" );
		fprintf( m_fp, "CLUB:\n" );
		fprintf( m_fp, "NAME: <== your name\n" );
		fprintf( m_fp, "ADDRESS: <== your postal address\n" );
		fprintf( m_fp, "SOAPBOX: \n" );
		fprintf( m_fp, "CREATED-BY: %s\n", VERTTL2);
		return TRUE;
	}
	else {
		ErrorMB( MsgEng ? "Can't write to '%s'":"'%s'が作成できません.", pName);
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// テキストファイルのオープン
int CLogCabrillo::Close(void)
{
	int r = 0;
	if( m_fp != NULL ){
		fprintf(m_fp, "END-OF-LOG:\n");
		r = fclose(m_fp);
		m_fp = NULL;
		if( r ){
			ErrorMB(MsgEng ? "Can't write to '%s'":"'%s'が正しくクローズできませんでした.", m_FileName.c_str());
		}
		else if( m_Mode ){
			InfoMB(MsgEng ? "Done (write to '%s')\r\n\r\nMMSSTV did only make QSO section.\r\nEdit to the contest name, category, your name and address, etc... in the file.":"'%s'への書きこみを終了しました.\r\n\r\nMMSSTVはQSOセクションしか作成しません。\r\nコンテスト名、参加カテゴリ等を編集してください.", m_FileName.c_str());
			sprintf(m_bf, "NOTEPAD.EXE %s", m_FileName.c_str());
			WinExec(m_bf, SW_SHOWDEFAULT);
		}
	}
	return r ? FALSE : TRUE;
}
//---------------------------------------------------------------------------
// LogCabrilloファイルの読み出し
int CLogCabrillo::Read(SDMMLOG *sp)
{
	return FALSE;
}
//---------------------------------------------------------------------------
// 出力変換
//
int CLogCabrillo::Write(SDMMLOG *sp)
{
	if( !IsOpen() ) return FALSE;
//	"","1.9","3.5","3.8","7","10","14","18","21","24","28","50",
//	"144","430","1200","2400","5600","10.1G","10.4G","24G","47G",
//	"75G","142G","248G","4630","220",NULL,
	const char	*_bandc[]={
		"??","1800","3500","3500","7000","10000","14000","18000","21000","24000","28000",
		"A", "B","D","E","F","H","I","J","K","M",
		"N","P","R","?","C",NULL,
	};

//	"CW","SSB","AM","FM","RTTY","PAC","FAX","SSTV","ATV","TV","FSTV",
//	"A1","A2","A3","A3A","A3H","A3J","A4","A5","A5C","A5J",
//	"A9","A9C","F1","F2","F3","F4","F5","F9","P0","P1",
//	"P2D","P2E","P2F","P3D","P3E","P3F","P9",
//	"U1","U2","U3","U4",
//	"PSK","BPSK","QPSK","HELL","MFSK",
	const char	*_modec[]={
		"??",
		"CW","PH","PH","FM","RY","RY","??","TV","TV","TV","TV",
		"CW","CW","PH","PH","PH","PH","??","??","??","??",
		"??","??","RY","RY","FM","??","??","??",
		"??","??","??","??","??","??","??","??","??",
		"??","??","??","??",
		"RY","RY","RY","CW","RY",
		NULL,
	};

	JSTtoUTC(sp);
	int tim = sp->btime / 30;
	char SNO[MLRST+1];
	char RNO[MLRST+1];
	int mno = sp->mode >= MODEMAX ? 0 : sp->mode;
	int len = GetLMode(BYTE(mno));
	strcpy(SNO, &sp->ur[len]);
	strcpy(RNO, &sp->my[len]);
	sp->ur[len] = 0;
	sp->my[len] = 0;
	if( !SNO[0] ){
		if( m_SNR.IsEmpty() ){
			int r;
			if( MsgEng ){
				r = InputMB("Does not exist the Sent-NR information", "Enter contest number which you sent", m_SNR);
			}
			else {
				r = InputMB("Sent-NR情報が存在しません", "Sent-NRを入力して下さい", m_SNR);
			}
			if( r == FALSE ) return FALSE;
		}
		strcpy(SNO, m_SNR.c_str());
	}
	fprintf( m_fp, "QSO:%6s %2s %04u-%02u-%02u %02u%02u %-13s %-3s %-6s %-13s %-3s %-6s\n",
		_bandc[sp->band], _modec[mno],
		(sp->year <= 50) ? sp->year + 2000 : sp->year + 1900,
		sp->date / 100, sp->date % 100,
		tim/60, tim%60, sys.m_Call.c_str(),
		sp->ur, SNO,
		sp->call, sp->my, RNO
	);
	return ferror(m_fp) ? FALSE : TRUE;
}

