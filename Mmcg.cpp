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



/************************************************************************
	MMCG	

	Copyright (C) JE3HHT 1993.
	Modifed on 2000 for MMTTY by JE3HHT
************************************************************************/
#include <vcl.h>	//ja7ude 0521
#pragma hdrstop

#include "ComLib.h"
#include "Mmcg.h"
#include "string.h"
#include "stdlib.h"
#include "Country.h"

CMMCG	mmcg;

static LPCSTR	ptbl[]={
	"",
	"北海道", "青森県","岩手県","秋田県","山形県","宮城県","福島県","新潟県",
	"長野県", "東京都","神奈川県","千葉県","埼玉県","茨城県","栃木県","群馬県",
	"山梨県","静岡県","岐阜県","愛知県","三重県","京都府","滋賀県","奈良県",
	"大阪府","和歌山県","兵庫県","富山県","福井県","石川県","岡山県","島根県",
	"山口県","鳥取県","広島県","香川県","徳島県","愛媛県","高知県","福岡県",
	"佐賀県","長崎県","熊本県","大分県","宮崎県","鹿児島県","沖縄県"
};

static LPCSTR	ptbleng[]={
	"",
	"Hokkaido", "Aomori","Iwate","Akita","Yamagata","Miyazaki","Fukushima","Nigata",
	"Nagano", "Tokyo","Kanagawa","Chiba","Saitama","Ibaragi","Tochigi","Gunma",
	"Yamanashi","Shizuoka","Gifu","Aichi","Mie","Kyoto","Shiga","Nara",
	"Osaka","Wakayama","Hyogo","Toyama","Fukui","Ishikawa","Okayama","Shimane",
	"Yamaguchi","Tottri","Hiroshima","Kagawa","Tokushima","Ehime","Kochi","Fukuoka",
	"Saga","Nagasaki","Kumamoto","Oita","Miyazaki","Kagoshima","Okinawa"
};

CMMCG::CMMCG()
{
	m_bp = NULL;
	m_Max = 0;
	m_Cnt = 0;
	m_sinc = 0;
	m_mask = -1;
}

CMMCG::~CMMCG()
{
	Free();
}

void CMMCG::Free(void)
{
	if( m_bp != NULL ){
		int i;
		MMCG	*mp;

		for( mp = m_bp, i = 0; i < m_Cnt; i++, mp++ ){
			if( mp->Code != NULL ) delete mp->Code;
			if( mp->QTH != NULL ) delete mp->QTH;
			if( mp->Key != NULL ) delete mp->Key;
		}
		m_bp = NULL;
	}
	if( m_fp != NULL ){
		delete m_fp;
		m_fp = NULL;
	}
	m_Max = m_Cnt = m_FindCnt = 0;
}

void CMMCG::Alloc(int n)
{
	if( n >= m_Max ){
		int max = m_Max ? m_Max * 2 : 256;
		MMCG *np = (MMCG *)new BYTE[sizeof(MMCG) * max];
		if( m_bp != NULL ){
			memcpy(np, m_bp, sizeof(MMCG) * m_Cnt);
			delete m_bp;
		}
		m_bp = np;
		m_Max = max;
	}
}

/*#$%
======================================================
	ローマ綴りを調整する
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
LPSTR CMMCG::adjspl(LPSTR p)
{
	static	char	bf[50];
	LPCSTR	tp[]={
		"TSU","TU",
		"CHA","CYA",
		"CHI","CI",
		"CHU","CYU",
		"CHO","CYO",
		"JYA","JA",
		"JYU","JU",
		"JYO","JO",
		"SHA","SYA",
		"SHI","SI",
		"SHU","SYU",
		"SHE","SYE",
		"SHO","SYO",
	};
	int		i, f;
	char	*t;

	for( t = bf; *p;){
		for( f = 0, i = 0; i < AN(tp); i+=2 ){
			if( !strncmp(p, tp[i], strlen(tp[i])) ){
				strcpy(t, tp[i+1]);
				t += strlen(t);
				p += strlen(tp[i]);
				f++;
				break;
			}
		}
		if( !f ){
			*t++ = *p++;
		}
	}
	*t = 0;
	return(bf);
}

/*#$%
======================================================
	最後のローマ綴りで検索起動を断念する
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
int	CMMCG::chkspl(LPCSTR p)
{
	static	char	*tp[]={
		"TS","CH","JY","SH",
	};
	int		i;

	i = strlen(p);
	if( i >= 2 ){
		p += (i-2);
		for( i = 0; i < AN(tp); i++ ){
			if( !strcmp(tp[i], p) ) return 0;
		}
	}
	return 1;
}

/*#$%
======================================================
	文字列の最後の一致を調べる
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
int CMMCG::LastMatch(LPSTR t, LPSTR s)
{
	t = lastp(t) - strlen(s) + 1;
	return !strcmp(t, s);
}

/*#$%
======================================================
	入力されたＱＴＨを正規化する
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
int CMMCG::NormQth(LPSTR p)
{
	int		i;
	LPCSTR	*cp;

	for( i = 1, cp = &ptbl[1]; i < AN(ptbl); i++, cp++ ){
		if( !strncmp(p, *cp, strlen(*cp)) ){
			if( *(p + strlen(*cp)) ){
				strcpy(p, p + strlen(*cp));
				return 1;
			}
		}
	}
	return 0;
}

/*#$%
======================================================
	エリアマスクを設定する
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void CMMCG::SetMask(void)
{
	LPCSTR	p;

	m_mask = -1;
	if( m_Call[0] ){	/* エリアマスク	*/
		p = m_Call;
		if( (m_Call[0] == '7') && (m_Call[1] != 'J') ){
			m_mask = 1;
			p += 3;
		}
		for( ; *p; p++ ){
			if( isdigit(*(LPUSTR)p) ) m_mask = ((*p) & 0x0f);
		}
	}
}

/*#$%
======================================================
	コードから検索対象にするかどうか調べる
------------------------------------------------------
	p : コードのポインタ
------------------------------------------------------
------------------------------------------------------
======================================================
*/
int CMMCG::isfind(LPCSTR p)
{
	const	char	_tt[]={
		-1,
		8,							/* 01		*/
		7, 7, 7, 7, 7, 7,			/* 02-07	*/
		0, 0,						/* 08-09	*/
		1, 1, 1, 1, 1, 1, 1, 1,		/* 10-17	*/
		2, 2, 2, 2,					/* 18-21	*/
		3, 3, 3, 3, 3, 3,			/* 22-27	*/
		9, 9, 9,					/* 28-30	*/
		4, 4, 4, 4, 4,				/* 31-35	*/
		5, 5, 5, 5,					/* 36-39	*/
		6, 6, 6, 6, 6, 6, 6, 6,		/* 40-47	*/
	};

	if( m_mask == -1 ){
		return 1;
	}
	else {
		return( (_tt[atoin(p, 2)] == m_mask) ? 1 : 0 );
	}
}


/*#$%
======================================================
	指定位置の文字の種類を得る
------------------------------------------------------
	p : バッファのポインタ
	x : 位置
------------------------------------------------------
------------------------------------------------------
======================================================
*/
int CMMCG::_ischrtyp(char *p, int x)
{
	int		xx, jis;

	for( jis = xx = 0; xx < x; xx++, p++ ){
		if( jis ){
			jis = 0;
		}
		else if( _mbsbtype((unsigned char *)p, 0) == _MBC_LEAD ){
			jis = 1;
		}
	}
	if( jis ){					/* KANJI2	*/
		return 2;
	}
	else if( _mbsbtype((unsigned char *)p, 0) == _MBC_LEAD ){		/* KANJI1	*/
		return 1;
	}
	else {						/* ANK		*/
		return 0;
	}
}

/*#$%
======================================================
	データファイルを読み込む
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void CMMCG::LoadDef(LPCSTR pFileName)
{
	FILE	*fp;
	char	hbf[512];
	LPSTR	t, p;

	Free();
	if( (fp = fopen(pFileName, "rt"))!=NULL ){
		while( !feof(fp) ){
			if( fgets(hbf, 512, fp)!=NULL ){
				if( hbf[0] == '$' ) break;
				ClipLF(hbf);
				if( hbf[0] != '!' ){
					p = StrDlm(t, hbf, TAB);
					clipsp(t);
					Alloc(m_Cnt);
					m_bp[m_Cnt].Code = StrDupe(t);
					if( p != NULL ){
						p = StrDlm(t, p, TAB);
						if( *t == '%' ) *t = 0;
						clipsp(t);
						m_bp[m_Cnt].QTH = StrDupe(SkipSpace(t));
					}
					else {
						m_bp[m_Cnt].QTH = StrDupe("");
					}
					if( p != NULL ){
						clipsp(p);
						m_bp[m_Cnt].Key = StrDupe(SkipSpace(p));
					}
					else {
						m_bp[m_Cnt].Key = StrDupe("");
					}
					m_Cnt++;
				}
			}
		}
		fclose(fp);
		m_fp = (MMCG **)new BYTE[sizeof(MMCG*)*m_Cnt];
	}
	else {
		WarningMB(MsgEng ? "'%s' was not found.\r\n\r\nYou cannot use a JCC/JCG list function.\r\nThis is not a problem if you do not need it":"'%s'が見つかりません.", pFileName);
	}
}

/*#$%
======================================================
	コードから県名を得る
------------------------------------------------------
	code : コードのポインタ
------------------------------------------------------
	県名のポインタ
------------------------------------------------------
======================================================
*/
LPCSTR CMMCG::getprf(LPCSTR pCode)
{
	int	n;

	n = atoin(pCode, 2);
	return(MsgEng ? ptbleng[n] : ptbl[n]);
}

/*#$%
======================================================
	郡名を得る
------------------------------------------------------
	p : 町村コードのポインタ
------------------------------------------------------
	QTHのポインタ
------------------------------------------------------
======================================================
*/
void CMMCG::GetGun(LPSTR t, LPCSTR pCode)
{
	strcpy(wbf, pCode);
	wbf[5] = 0;
	MMCG *mp = m_bp;
	int i;
	for( i = 0; i < m_Cnt; i++, mp++ ){
		if( !strcmp(mp->Code, wbf) ){
			strcpy(t, mp->QTH);
			return;
		}
	}
	*t = 0;
}

/*#$%
======================================================
	ＱＴＨの文字列を得る
------------------------------------------------------
	cp : データのポインタ
------------------------------------------------------
	QTHのポインタ
------------------------------------------------------
======================================================
*/
LPCSTR CMMCG::GetQTH(MMCG *mp)
{
	char bf[512];

	if( MsgEng ){
		char qth[128];
		strcpy(qth, mp->Key);
		LPSTR	t;
		LPSTR	p = qth;
		while(*p){
			p = StrDlm(t, p, ',');
		}
		if( strlen(mp->Code) == 6 ){
			if( isalpha((unsigned char)LastC(mp->Code)) ){		/* 町村コード	*/
				strcpy(bf, t);
				strcat(bf, ",");
				GetGun(qth, mp->Code);
				strcat(bf, qth);
			}
			else {								/* 政令指定都市		*/
				strcpy(bf, t);
			}
		}
		else {							/* 都道府県を追加	*/
			if( *t ){
				sprintf( bf, "%s,%s", t, getprf(mp->Code));
			}
			else {
				sprintf( bf, "%s", getprf(mp->Code));
			}
		}
	}
	else {
		if( strlen(mp->Code) == 6 ){
			if( isalpha((unsigned char)LastC(mp->Code)) ){		/* 町村コード	*/
				GetGun(bf, mp->Code);
				strcat(bf, mp->QTH);
			}
			else {								/* 政令指定都市		*/
				strcpy(bf, mp->QTH);
			}
		}
		else {							/* 都道府県を追加	*/
			sprintf( bf, "%s%s", getprf(mp->Code), mp->QTH);
		}
	}
	bf[MLQTH] = 0;
	m_QTH = bf;
	return m_QTH.c_str();


}


/*#$%
======================================================
	データの検索を行う（すべて）
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void CMMCG::FindAll(void)
{
	MMCG	*mp;
	int		i;

	m_FindCnt = 0;
	mp = m_bp;
	for( i = 0; i < m_Cnt; i++, mp++ ){
		if( isfind(mp->Code) ){
			m_fp[m_FindCnt] = mp;
			m_FindCnt++;
		}
	}
}

/*#$%
======================================================
	同一データが存在するか調べる
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
int CMMCG::isEnt(MMCG *mp)
{
	int		i;

	for( i = 0; i < m_FindCnt; i++ ){
		if( m_fp[i] == mp ) return 1;
	}
	return 0;
}

/*#$%
======================================================
	データの検索を行う
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void CMMCG::FindQTH(LPSTR pKey)
{
	MMCG	*mp;
	int		i;
	int		len, kj;
	LPSTR	t, p;

	kj = _mbsbtype((unsigned char *)pKey, 0) == _MBC_LEAD ? 1 : 0;
	if( kj ) NormQth(pKey);
	len = strlen(pKey);
	m_FindCnt = 0;
	mp = m_bp;
	char	bf[256];
	for( i = 0; i < m_Cnt; i++, mp++ ){
		if( isfind(mp->Code) ){
			if( kj ){
				if( strstr(mp->QTH, pKey) != NULL ){
					m_fp[m_FindCnt] = mp;
					m_FindCnt++;
					goto _nx;
				}
			}
			strcpy(bf, mp->Key);
			for( p = bf; *p; ){
				p = StrDlm(t, p, ',');
				if( (!strncmp(t, pKey, len)) || (m_sinc && (strstr(t, pKey)!=NULL)) ){
					m_fp[m_FindCnt] = mp;
					m_FindCnt++;
					break;
				}
			}
_nx:;
		}
	}
}

/*#$%
======================================================
	データの検索を行う
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void CMMCG::FindNumb(LPSTR pKey)
{
	MMCG	*mp;
	int		i;
	int		len;

	m_FindCnt = 0;
	len = strlen(pKey);
	mp = m_bp;
	for( i = 0; i < m_Cnt; i++, mp++ ){
		if( (!strncmp(mp->Code, pKey, len)) || (m_sinc && (strstr(mp->Code, pKey)!=NULL)) ){
			m_fp[m_FindCnt] = mp;
			m_FindCnt++;
		}
	}
}

/*#$%
======================================================
	データの検索を行う
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void CMMCG::Find(LPSTR p)
{
	if( *p ){
		if( isdigit(*(LPUSTR)p) ){
			FindNumb(p);
		}
		else {
			jstrupr(p);
			if( chkspl(p) ) FindQTH(adjspl(p));
		}
	}
	else {
		FindAll();
	}
}

#if 0
/*#$%
======================================================
	定義領域を確保する
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
int allocdef(int blk)
{
	if( cseg == 0 ){
		if( (cseg=__malloc(blk))!=0 ){		/* メモリの確保	*/
			return(NOERR);
		}
		return(ERR);
	}
	return(NOERR);
}

/*#$%
======================================================
	定義領域を開放する
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void freedef(void)
{
	if( cseg ){
		__free(cseg);
		cseg = 0;
	}
}

#define	XW		60
#define	YW		16
/*#$%
======================================================
	検索データのメニュー表示をオープンする
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
VEWP *opencc(void)
{
	cc.xw = XW-1;
	cc.yw = YW-1;
	cc.x = (crt.hmax-cc.xw+2)/2;
	cc.y = (crt.vmax-cc.yw+2)/2;
	cc.yt = cc.y;
	cc.ye = cc.y + cc.yw;
	cc.tb = cc.cy = 0;
	cc.xe = cc.x + cc.xw - 1;
	if( (cc.vp=openvew(cc.x, cc.y, cc.x+cc.xw, cc.ye, _menuwcol))!=NULL ){
		_dprintf( cc.x, cc.yt, " 検索>" );
		cc.x++;
		cc.y++;
/*		_dprintf( cc.x, cc.ye, "<ESC>中止, <CR>決定, <↓><↑>選択, <SPC>ｸﾘｱ, <HOME>ｴﾘｱ/ALL" );*/
		_dprintf( cc.x, cc.ye, "<ESC>中止,<CR>決定,<SPC>ｸﾘｱ,<F1>検索方法,<F2><HOME>ｴﾘｱ/ALL" );
	}
	return(cc.vp);
}

/*#$%
======================================================
	検索データのメニュー表示をクローズする
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void closecc(void)
{
	if( cc.vp ){
		closevew(cc.vp);
		cc.vp = NULL;
	}
}

/*#$%
======================================================
	１行の表示を行う
------------------------------------------------------
	y : 表示位置
	tp: テーブル番号
------------------------------------------------------
------------------------------------------------------
======================================================
*/
static void near dspone(int y, int tp)
{
	CD		cd;
	int		l;
	uchar	bf[60];

/*0123456789012345678901234567890123456789012345678901234567*/
/*250119__ 大阪府__ 大阪市阿倍野区______ ABENO______________*/
/*12345678 12345678 12345678901234567890 1234567890123456789*/

	_fills(cc.x, y, cc.xe, y);
	if( tp < cc.fn ){
		getdat(cc.ctbl[tp], &cd);
		l = strlen(cd.code);
		_dprintf(cc.x,    y, "%-.8s", cd.code );
		_dprintf(cc.x+9, y, "%-.8s", getprf(cd.code));
		if( l == 6 ){
			if( isalpha(lastc(cd.code)) ){		/* 町村	*/
				GetGun(bf, cd.code);
			}
			else {								/* 制令	*/
				bf[0] = 0;
			}
			strcat(bf, cd.qth);
			_dprintf(cc.x+18, y, "%-.20s", bf );
		}
		else {
			_dprintf(cc.x+18, y, "%-.20s", cd.qth );
		}
		_dprintf(cc.x+39, y, "%-.19s", cd.key );
	}
}

/*#$%
======================================================
	検索データの表示をする
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
static void near dspcc(void)
{
	int		y, cp;
	CD		cd;

	for( cp = cc.tb, y = cc.y; y < cc.ye; y++, cp++ ){
		dspone(y, cp);
	}
}

/*#$%
======================================================
	カーソルの表示
------------------------------------------------------
	sw : 0-OFF, 1-ON
------------------------------------------------------
------------------------------------------------------
======================================================
*/
static void near ecursor(int sw)
{
	int	at;

	at = sw ? _menuccol : CRT_ATTR_NORMAL;
	_afill(cc.x, cc.y + cc.cy, cc.xe, cc.y + cc.cy, at );
}

/*#$%
======================================================
	カーソルのアップ
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
static void near cup(void)
{
	if( cc.cy ){
		cc.cy--;
	}
	else if( cc.tb ){
		cc.tb--;
		_scrdwn(cc.x, cc.y, cc.xe, cc.ye-1);
		dspone(cc.y, cc.tb);
	}
}

/*#$%
======================================================
	カーソルのダウン
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
static void near cdown(void)
{
	if( (cc.tb + cc.cy) < (cc.fn - 1) ){
		if( cc.cy < (cc.yw - 2) ){
			cc.cy++;
		}
		else {
			cc.tb++;
			_scrup(cc.x, cc.y, cc.xe, cc.ye-1);
			dspone(cc.ye-1, cc.tb+cc.cy);
		}
	}
}

/*#$%
======================================================
	指定位置へのＳＥＥＫ
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
static void near cdseek(int tp)
{
	if( tp >= cc.fn ) tp = cc.fn - 1;
	if( tp < 0 ) tp = 0;
	cc.tb = tp;
	cc.cy = 0;
	dspcc();
}

/*#$%
======================================================
	ページアップ
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
static void near pup(void)
{
	CD		cd;
	int		tp, m;

	tp = cc.tb + cc.cy;
	getdat(cc.ctbl[tp], &cd);
	m = atoin(cd.code, 2);
	for( tp--; tp >= 0; tp-- ){
		getdat(cc.ctbl[tp], &cd);
		if( m != atoin(cd.code, 2) ){
			cdseek(tp);
			return;
		}
	}
	cdseek(0);
}

/*#$%
======================================================
	ページダウン
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
static void near pdown(void)
{
	CD		cd;
	int		tp, m;

	tp = cc.tb + cc.cy;
	getdat(cc.ctbl[tp], &cd);
	m = atoin(cd.code, 2);
	for( tp++; tp < cc.fn; tp++ ){
		getdat(cc.ctbl[tp], &cd);
		if( m != atoin(cd.code, 2) ){
			cdseek(tp);
			return;
		}
	}
	cdseek(32767);
}

/*#$%
======================================================
	バッファ内容の表示
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
static void near dspbf(void)
{
	_dprintf(cc.x+5, cc.yt, "%-40s", kbf );
	_dprintf(cc.xe - 15, cc.yt, sinc ? "含み":"先頭");
	_dprintf(cc.xe - 10, cc.yt, (m_mask != -1) ? "%uｴﾘｱ" : "ALL ", m_mask );
	_dprintf(cc.xe-4, cc.yt, "%4u", cc.fn );
}

/*#$%
======================================================
	ＭＭＣＧの処理を行う
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void mmcg(void)
{
	int	c;
	int		xx;

	loaddat();
	if( nosel == 1 ){
		strcpy(kbf, _dspopt(optnmb, &cc.sd));
		findnosel(jstrupr(kbf));
		setdat();
		return;
	}
	if( opencc() == NULL ) return;
	find1st();
	if( nosel == 2 ){
		if( cc.fn == 1 ) setdat();
		closecc();
		return;
	}
	dspcc();
	xx = strlen(kbf);
	_cursor(ON);
	while(1){
		dspbf();
		_locate(cc.x+5 + xx, cc.yt);
		ecursor(ON);
		c = _bgetch();
		ecursor(OFF);
		switch(c){
			case K_RU:
				pdown();
				break;
			case K_RD:
				pup();
				break;
			case K_UP:
				cup();
				break;
			case K_DOWN:
				cdown();
				break;
			case ESC:
				goto _ex;
			case K_CR:
				setdat();
				goto _ex;
			case K_BS:
				if( xx ){
					xx--;
					if( xx && (_ischrtyp(kbf, xx)==2) ) xx--;
					kbf[xx] = 0;
					find(kbf);
					dspcc();
				}
				break;
			case ' ':
				kbf[0] = 0;
				xx = 0;
				findall();
				dspcc();
				break;
			case K_F1:
				sinc = sinc ? 0 : 1;
				find(kbf);
				dspcc();
				break;
			case K_F2:
			case K_HOME:
				if( mask == -1 ){
					setmask();
				}
				else {
					mask = -1;
				}
				find(kbf);
				dspcc();
				break;
			default:
				if( (c > 0x0020) && (c <= 0x00ff) ){
					if( xx < 40 ){
						kbf[xx] = c;
						xx++;
						kbf[xx] = 0;
						if( _ischrtyp(kbf, xx-1) != 1 ){
							finds(kbf);
						}
						dspcc();
					}
				}
				break;
		}
	}
_ex:;
	closecc();
}

/*#$%
======================================================
	データの検索を行う（すべて）
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void findall(void)
{
	CD		cd;
	int	cp, np;

	cc.tb = cc.cy = cc.fn = 0;
	for( cp = 0; cp < cc.cwp; cp = np){
		np = getdat(cp, &cd);
		if( isfind(cd.code) ){
			cc.ctbl[cc.fn] = cp;
			cc.fn++;
			if( cc.fn >= CTMAX ) break;
		}
	}
}

/*#$%
======================================================
	同一データが存在するか調べる
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
int isEnt(int n)
{
	int		i;

	for( i = 0; i < cc.fn; i++ ){
		if( cc.ctbl[i] == n ){
			return(ON);
		}
	}
	return(OFF);
}

/*#$%
======================================================
	データの検索を行う
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void findqth(uchar *key)
{
	CD		cd;
	int	cp, np;
	int		len, kj;
	uchar	*t, *p;

	if( (kj = iskanji(*key))!=0 ) NormQth(key);
	len = strlen(key);
	cc.tb = cc.cy = cc.fn = 0;
	for( cp = 0; cp < cc.cwp; cp = np){
		np = getdat(cp, &cd);
		if( isfind(cd.code) ){
			if( kj ){
				if( jstrstr(cd.qth, key) != NULL ){
					cc.ctbl[cc.fn] = cp;
					cc.fn++;
					goto _nx;
				}
			}
			for( p = cd.key; *p; ){
				p = strdm(&t, p);
				if( (!strncmp(t, key, len)) || (sinc && (strstr(t, key)!=NULL)) ){
					cc.ctbl[cc.fn] = cp;
					cc.fn++;
					break;
				}
			}
_nx:;
			if( cc.fn >= CTMAX ) break;
		}
		if( _bkbhit() ) break;
	}
}

/*#$%
======================================================
	データの検索を行う
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void findqths(uchar *key)
{
	CD		cd;
	int	cp;
	int		len, fn, nn, f, kj;
	uchar	*t, *p;

	if( (kj = iskanji(*key))!=0 ){
		if( NormQth(key) ){
			findqth(key);
			return;
		}
	}
	len = strlen(key);
	fn = cc.fn;
	cc.tb = cc.cy = cc.fn = 0;
	f = OFF;
	if( lastc(key) == '\\' ){
		f = ON;
		*lastp(key) = 0;
	}
	for( nn = 0; nn < fn; nn++){
		cp = cc.ctbl[nn];
		getdat(cp, &cd);
		if( kj ){
			if( jstrstr(cd.qth, key) != NULL ){
				cc.ctbl[cc.fn] = cp;
				cc.fn++;
				goto _nx;
			}
		}
		for( p = cd.key; *p; ){
			p = strdm(&t, p);
			if( f ){
				if( (!strcmp(t, key)) || (sinc && (LastMatch(t,key))) ){
					cc.ctbl[cc.fn] = cp;
					cc.fn++;
					break;
				}
			}
			else {
				if( (!strncmp(t, key, len)) || (sinc && (strstr(t, key)!=NULL)) ){
					cc.ctbl[cc.fn] = cp;
					cc.fn++;
					break;
				}
			}
		}
_nx:;
		if( cc.fn >= CTMAX ) break;
	}
}

/*#$%
======================================================
	データの検索を行う
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void findnmb(uchar *key)
{
	CD		cd;
	int	cp, np;
	int		len;

	cc.tb = cc.cy = cc.fn = 0;
	len = strlen(key);
	for( cp = 0; cp < cc.cwp; cp = np){
		np = getdat(cp, &cd);
		if( (!strncmp(cd.code, key, len)) || (sinc && (strstr(cd.code, key)!=NULL)) ){
			cc.ctbl[cc.fn] = cp;
			cc.fn++;
			if( cc.fn >= CTMAX ) break;
		}
	}
}

/*#$%
======================================================
	データの検索を行う
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void findnmbs(uchar *key)
{
	CD		cd;
	int	cp;
	int		len, fn, nn, f;

	fn = cc.fn;
	cc.tb = cc.cy = cc.fn = 0;
	f = OFF;
	if( lastc(key) == '\\' ){
		f = ON;
		*lastp(key) = 0;
	}
	len = strlen(key);
	for( nn = 0; nn < fn; nn++){
		cp = cc.ctbl[nn];
		getdat(cp, &cd);
		if( f ){
			if( (!strcmp(cd.code, key)) || (sinc && LastMatch(cd.code, key)) ){
				cc.ctbl[cc.fn] = cp;
				cc.fn++;
			}
		}
		else {
			if( (!strncmp(cd.code, key, len)) || (sinc && (strstr(cd.code, key)!=NULL)) ){
				cc.ctbl[cc.fn] = cp;
				cc.fn++;
			}
		}
	}
	if( f ) strcat(key, "\\");
}

/*#$%
======================================================
	データの検索を行う
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void findnosel(uchar *key)
{
	CD		cd;
	int	cp, np;

	cc.tb = cc.cy = cc.fn = 0;
	for( cp = 0; cp < cc.cwp; cp = np){
		np = getdat(cp, &cd);
		if( !strcmp(cd.code, key) ){
			cc.ctbl[cc.fn] = cp;
			cc.fn++;
			break;
		}
	}
}

/*#$%
======================================================
	データの検索を行う
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void find(uchar *p)
{
	dspbf();
	if( *p ){
		if( isdigit(*p) ){
			findnmb(p);
		}
		else {
			jstrupr(p);
			if( chkspl(p) ) findqth(adjspl(p));
		}
	}
	else {
		findall();
	}
}

/*#$%
======================================================
	データの検索を行う
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void finds(uchar *p)
{
	dspbf();
	if( *p ){
		if( isdigit(*p) ){
			findnmbs(p);
			if( !cc.fn ) findnmb(p);
		}
		else {
			jstrupr(p);
			if( chkspl(p) ) findqths(adjspl(p));
		}
	}
	else {
		findall();
	}
}

/*#$%
======================================================
	最初の検索を行う
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void find1st(void)
{
	uchar	*p;
	uchar	far *s;

	setmask();
	kbf[0] = 0;
	if( setmac ){
		if( dtname[0] && (ishex(dtname)==NOERR) ){
			s = (uchar far *)htoln(dtname, -1);
			for( p = kbf; *s; s++, p++ ){
				*p = *s;
			}
			*p = 0;
		}
	}
	else {
		p = _dspopt(optnmb, &cc.sd);
		if( *p && isdigit(*p) ){
			strcpy(kbf, p);
		}
		else if( cc.sd.qth[0] ){
			strcpy(kbf, cc.sd.qth);
		}
	}
	find(jstrupr(kbf));
}

/*#$%
======================================================
	データをセットする
------------------------------------------------------
------------------------------------------------------
------------------------------------------------------
======================================================
*/
void setdat(void)
{
	CD		cd;
	uchar	far	*t;
	uchar	*p;

	if( cc.fn ){
		if( !dtname[0] ) return;
		getdat(cc.ctbl[cc.tb+cc.cy], &cd);
		if( setmac ){
			if( ishex(dtname)!=NOERR ) return;
			t = (uchar far *)htoln(dtname,-1);
			if( ocpy ){
				for( p = cd.code; *p; p++, t++ ){
					*t = *p;
				}
				*t = 0;
			}
			if( qcpy ){
				for( p = getqth(&cd); *p; p++, t++ ){
					*t = *p;
				}
				*t = 0;
			}
		}
		else {
			if( ocpy ) strcpy(_ptropt(optnmb, &cc.sd), cd.code);
			if( qcpy ) strcpy(cc.sd.qth, getqth(&cd));
			savedat();
		}
	}
}

/*#$%
================================================================
	１行のアスキーセーブの処理を行う
----------------------------------------------------------------
	fp : ファイルポインタ
	sp : データのポインタ
----------------------------------------------------------------
	ERR
----------------------------------------------------------------
================================================================
*/
void _sputdat(uchar *bf, SD *sp)
{
	sinit(bf);
	sputf("\042%s\042,\042%s\042,\042%s\042,\042%s\042,\042%s\042,\042%s\042,",
	_strydate(sp->year, sp->date), _strtimes(sp->btime), sp->call, sp->ur, sp->my, _strband(sp->band, sp->fq));
	sputf("\042%s\042,\042%s\042,\042%s\042,\042%s\042,\042%s\042,\042%s\042,",
	_strmode(sp->mode), sp->pow, sp->name, sp->qth, sp->rem, sp->qsl );
	sputf("\042%s\042,\042%.1s\042,\042%.1s\042,\042%.1s\042,\042%u\042,", _strtime(sp->etime), &sp->send, &sp->recv, &sp->cq, sp->env);
	sputf("\042%s\042,\042%s\042,", sp->opt1, sp->opt2 );
	sputf("\042%s\042,", _dspopt(2, sp));
	sputf("\042%s\042", _dspopt(3, sp));
}

/*#$%
================================================================
	１行のアスキーセーブの処理を行う
----------------------------------------------------------------
	fp : ファイルポインタ
	sp : データのポインタ
----------------------------------------------------------------
	ERR
----------------------------------------------------------------
================================================================
*/
int _fputdat(FILE *fp, SD *sp)
{
	_sputdat(hbf, sp);
	fputs(hbf, fp);
	fputs("\r\n", fp);
	return(ferror(fp));
}

/*#$%
================================================================
	アスキーデータを登録する
----------------------------------------------------------------
	p : バッファのポインタ
----------------------------------------------------------------
	ERR
----------------------------------------------------------------
================================================================
*/
int _aputdat(uchar *p, SD *sp)
{
	uchar	*t;
	int	y, m, d;

	_fillmem(sp, sizeof(SD), 0);
	p = strdm(&t, p);		/* DATE	*/
	if( sscanf(t, "%u.%u.%u", &y, &m, &d )!=3 ) return(ERR);
	sp->year = y;
	sp->date = (m * 100) + d;
	p = strdm(&t, p);		/* BGN	*/
	if( sscanf(t, "%u.%u", &d, &y) != 2 ){
		d = atoin(t, -1);
		y = 0;
	}
	m = d / 100;
	d = d % 100;
	y /= 2;
	sp->btime = (((m * 60) + d) * 30) + y;
	p = strdm(&t, p);		/* CALL	*/
	strcpy(sp->call, t);
	p = strdm(&t, p);		/* UR	*/
	strcpy(sp->ur, t);
	p = strdm(&t, p);		/* MY	*/
	strcpy(sp->my, t);
	p = strdm(&t, p);		/* BAND	*/
	sp->band = _bandno(&sp->fq, t);
	p = strdm(&t, p);		/* MODE	*/
	sp->mode = _modeno(t);
	p = strdm(&t, p);		/* POW	*/
	strcpy(sp->pow, t);
	p = strdm(&t, p);		/* NAME	*/
	strcpy(sp->name, t);
	p = strdm(&t, p);		/* QTH	*/
	strcpy(sp->qth, t);
	p = strdm(&t, p);		/* REM	*/
	strcpy(sp->rem, t);
	p = strdm(&t, p);		/* QSL	*/
	strcpy(sp->qsl, t);
	p = strdm(&t, p);		/* ETIME*/
	d = atoin(t, -1);
	m = d / 100;
	d = d % 100;
	sp->etime = ((m * 60) + d) * 30;
	p = strdm(&t, p);		/* S	*/
	sp->send = *t;
	p = strdm(&t, p);		/* R	*/
	sp->recv = *t;
	p = strdm(&t, p);		/* M	*/
	sp->cq = *t;
	p = strdm(&t, p);		/* ENV	*/
	sp->env = atoin(t, -1);
	p = strdm(&t, p);		/* OPT1	*/
	strcpy(sp->opt1, t);
	p = strdm(&t, p);		/* OPT2	*/
	strcpy(sp->opt2, t);
	p = strdm(&t, p);		/* USR1	*/
	if( *t ) strcpy(_ptropt(2,sp), t);
	p = strdm(&t, p);		/* USR2	*/
	if( *t ) strcpy(_ptropt(3,sp), t);
	return(NOERR);
}

/*#$%
================================================================
	引渡データを読み込む
----------------------------------------------------------------
----------------------------------------------------------------
----------------------------------------------------------------
================================================================
*/
int loaddat(void)
{
	int		r;
	FILE	*fp;

	r = ERR;
	_fillmem(&cc.sd, sizeof(SD), 0);
	if( !dtname[0] ) return(NOERR);
	if( setmac ) return(NOERR);
	if( (fp = fopen(dtname, "rt"))!=NULL ){
		if( (fgets(hbf, 256, fp))!=NULL ){
			cliplf(hbf);
			_aputdat(hbf, &cc.sd);
			r = NOERR;
		}
		fclose(fp);
	}
	return(r);
}

/*#$%
================================================================
	指定のデータを書き込む
----------------------------------------------------------------
----------------------------------------------------------------
----------------------------------------------------------------
================================================================
*/
void savedat(void)
{
	FILE	*fp;

	if( (fp = fopen(dtname, "wb"))!=NULL ){
		_fputdat(fp, &cc.sd);
		fclose(fp);
	}
}
#endif

