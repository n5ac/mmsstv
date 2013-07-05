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

#include "ComLib.h"
#include "LogFile.h"
#include "Hamlog5.h"
#include "LogConv.h"
//--------------------------------------------
const LPCSTR g_Hamlog5Key[]={
	"CALLS", "IGN", "DATE", "TIME", "CODE", "GL", "QSL", "FLAG", "HIS", "MY",
	"FREQ", "MODE", "NAME", "QTH", "RMK1", "RMK2", NULL
};
const BYTE g_Hamlog5Len[]={
	8, 12, 4, 2, 6, 6, 3, 2, 3, 3,
    7, 4, 12, 28, 54, 54, 0
};
//--------------------------------------------
void __fastcall GetHamlog5FieldsLen(AnsiString &as)
{
	as = "";
	for( int i = 8; i < 15; i++ ){
		if( i > 8 ) as += ",";
		as += Log.m_LogSet.m_Hamlog5Len[i];
    }
}
//--------------------------------------------
void __fastcall SetHamlog5FieldsLen(AnsiString &as)
{
	LPSTR pBF = StrDupe(as.c_str());
    LPSTR p, t;
    p = pBF;
	for( int i = 8; i < 15; i++ ){
		p = StrDlm(t, p);
		Log.m_LogSet.m_Hamlog5Len[i] = BYTE(atoin(t, -1));
    }
    delete pBF;
}
//--------------------------------------------
//コンストラクタ
CHamlog5::CHamlog5()
{
	m_fCreate = FALSE;
	m_RecMax = 0;			// レコード数
	m_FilMax = 0;			// フィールド数
	m_RecWidth = 0;			// レコードの幅
	m_FilOff = 0;			// ヘッダオフセット
	m_bp = NULL;
	m_OpenFlag = FALSE;
	m_WriteFlag = FALSE;	// 書き込みフラグ
}

//--------------------------------------------
//デストラクタ
CHamlog5::~CHamlog5()
{
	Close();
}

//--------------------------------------------
//ヘッダデータセットアップ
BOOL CHamlog5::SetupHeader(void)
{
	if( fread(&m_Head, 1, sizeof(m_Head), m_fp)!=sizeof(m_Head) ){
		return FALSE;	// 異なるフォーマット
	}

	if( (m_Head.Type != 0x03)&&(m_Head.Type != 0x1a ) ) return FALSE;

	m_RecMax = m_Head.RecMax;
	m_FilOff = m_Head.FilOff;
	m_RecWidth = m_Head.RecWidth;
	if( !m_FilOff ) return FALSE;

	hamlog5DBRHD	slot;
	ULONG	fPos;
	for( m_FilMax = 0; m_FilMax < HamlogDBMAX; m_FilMax++ ){
		fPos = (32L*m_FilMax)+32L;
		if( fPos > ULONG(m_FilOff) ) break;
		if( fread(&slot, 1, sizeof(slot), m_fp)!=sizeof(slot) ) break;
		if( slot.Name[0] == 0x0d ) break;
		slot.Name[10] = 0;
		m_StrTable[m_FilMax] = slot.Name;
		m_TypeTable[m_FilMax] = slot.Type;
		m_LenTable[m_FilMax] = USHORT(slot.Len);
	}
	for( int i = 8; (i < 15) && (i < m_FilMax); i++ ){
		Log.m_LogSet.m_Hamlog5Len[i] = m_LenTable[i];
    }

//		if( (m_Head.Type == 0x1a) && !m_RecMax ){
//			m_RecMax = (m_fStatus.m_size - ULONG(m_FilOff)) / ULONG(m_RecWidth);
//		}

	USHORT	i;
	USHORT	Pos = 1;
	for( i = 0; i < m_FilMax; i++ ){
		m_PosTable[i] = Pos;
		Pos += m_LenTable[i];
	}
	return TRUE;
}

//--------------------------------------------
//ヘッダデータ作成
BOOL CHamlog5::MakeHeader(const LPCSTR _NT[], const BYTE _LT[])
{
	hamlog5DBRHD	slot;

	memset(&m_Head, 0, sizeof(m_Head));
	fseek(m_fp, 0, SEEK_SET);
	fwrite(&m_Head, 1, sizeof(m_Head), m_fp);
	m_FilOff = sizeof(m_Head);
	m_RecWidth = 1;
	for( m_FilMax = 0; _LT[m_FilMax]; m_FilMax++ ){
		memset(&slot, 0, sizeof(slot));
		strcpy(slot.Name, _NT[m_FilMax]);
		slot.Len = _LT[m_FilMax];
		slot.Type = 'C';
		fwrite(&slot, 1, sizeof(slot), m_fp);
		m_PosTable[m_FilMax] = m_RecWidth;
		m_RecWidth += USHORT(_LT[m_FilMax]);
		m_FilOff += USHORT(sizeof(slot));
		m_StrTable[m_FilMax] = _NT[m_FilMax];
		m_TypeTable[m_FilMax] = 'C';
		m_LenTable[m_FilMax] = USHORT(_LT[m_FilMax]);
	}
	char Term = 0x0d;
	fwrite(&Term, 1, 1, m_fp);
	m_FilOff++;
	m_Head.Type = 3;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	m_Head.Year = BYTE(st.wYear % 100);
	m_Head.Mon = BYTE(st.wMonth);
	m_Head.Day = BYTE(st.wDay);
	m_Head.RecMax = 0;
	m_Head.FilOff = m_FilOff;
	m_Head.RecWidth = m_RecWidth;
	fseek(m_fp, 0, SEEK_SET);
	fwrite(&m_Head, 1, sizeof(m_Head), m_fp);
	return TRUE;
}

//--------------------------------------------
//オープン
BOOL CHamlog5::Open(LPCSTR Name, BOOL fMsg)
{
	if( (m_OpenFlag == TRUE) ) Close();
	m_WriteFlag = FALSE;	// 書き込みフラグ
	if( (m_fp = fopen(Name, "rb")) == NULL ){
		if( fMsg ) ErrorMB("%sがオープンできません.", Name);
		return FALSE;	// ファイルがオープンできない
	}
	m_OpenFlag = TRUE;
    m_fCreate = FALSE;
	if( SetupHeader() == FALSE ){
		Close();
		if( fMsg ) ErrorMB("ファイルフォーマットが認識できません.");
		return FALSE;
	}
	if( m_bp ) delete m_bp;
	m_bp = new char[m_RecWidth];
	if( Seek(0) == FALSE ){
		Close();
		if( fMsg ) ErrorMB("データーレコードが存在しません.");
		return FALSE;
	}
	return TRUE;
}

//--------------------------------------------
//オープン
BOOL CHamlog5::Create(LPCSTR Name)
{
	if( (m_OpenFlag == TRUE) ) Close();
	m_WriteFlag = FALSE;	// 書き込みフラグ
	if( (m_fp = fopen(Name, "wb")) == NULL ){
		ErrorMB("%sを作成できません.", Name);
		return FALSE;	// ファイルがオープンできない
	}
	m_OpenFlag = TRUE;
    m_fCreate = TRUE;
	if( MakeHeader(g_Hamlog5Key, Log.m_LogSet.m_Hamlog5Len) == FALSE ){
		Close();
		ErrorMB("%sを作成できません.", Name);
		return FALSE;
	}
	if( m_bp ) delete m_bp;
	m_bp = new char[m_RecWidth];
	return TRUE;
}

//--------------------------------------------
//クローズ
void CHamlog5::Close(void)
{
	if( m_OpenFlag == TRUE ){
		Update();
		if( m_RecMax != m_Head.RecMax ){	// データが追加されている
			m_Head.RecMax = m_RecMax;
			fseek(m_fp, 0, SEEK_SET);
			fwrite(&m_Head, 1, sizeof(m_Head), m_fp);
		}
		int r = fclose(m_fp);
        m_fp = NULL;
        if( r && m_fCreate ){
			ErrorMB("ファイルの作成に失敗しました.");
        }
		m_OpenFlag = FALSE;
	}
	if( m_bp != NULL ){
		delete m_bp;
		m_bp = NULL;
	}
}

//--------------------------------------------
//レコードシーク
BOOL CHamlog5::Seek(ULONG Index)
{
	if( m_OpenFlag == FALSE ) return FALSE;
	if( Update() == FALSE ) return FALSE;
	m_Index = Index;
	m_Pos = (Index * ULONG(m_RecWidth)) + ULONG(m_FilOff);
	fseek(m_fp, m_Pos, SEEK_SET);
	if( Index < m_RecMax ){
		if( fread(m_bp, 1, m_RecWidth, m_fp) != ULONG(m_RecWidth) ){
			return FALSE;
		}
	}
	else {
		memset(m_bp, ' ', m_RecWidth);
	}
	return TRUE;
}

//--------------------------------------------
//マークのチェック
BOOL CHamlog5::IsData(void)
{
	return (*m_bp == ' ') ? TRUE : FALSE;
}

//--------------------------------------------
//データの読み込み
BOOL CHamlog5::GetData(USHORT SubIndex, AnsiString &cs)
{
	char	bf[1024];

	if( SubIndex >= m_FilMax ) return FALSE;

	int	len = m_LenTable[SubIndex];
	if( len >= 1023 ) len = 1023;
	StrCopy(bf, m_bp + m_PosTable[SubIndex], len);
	cs = bf;
	return TRUE;
}
//--------------------------------------------
//データの読み込み
BOOL CHamlog5::GetData(USHORT SubIndex, LPSTR pStore)
{
	if( SubIndex >= m_FilMax ) return FALSE;

	int	len = m_LenTable[SubIndex];
	if( len >= 1023 ) len = 1023;
	StrCopy(pStore, m_bp + m_PosTable[SubIndex], len);
	return TRUE;
}

//--------------------------------------------
//データの読み込み
int CHamlog5::GetData(USHORT SubIndex, LPBYTE pData, int len)
{
	if( SubIndex >= m_FilMax ) return 0;

	int	rlen = m_LenTable[SubIndex];
	if( rlen >= 1023 ) rlen = 1023;
	if( rlen > len ) rlen = len;
	memcpy(pData, m_bp + m_PosTable[SubIndex], rlen);
	return rlen;
}

//--------------------------------------------
//データの書き込み（バッファを更新するのみ）
BOOL CHamlog5::SetData(USHORT SubIndex, LPCSTR p)
{
	if( SubIndex >= m_FilMax ) return FALSE;

	int len = m_LenTable[SubIndex];
	LPSTR	t;
	int		i;
	for( i = 0, t = m_bp + m_PosTable[SubIndex]; i < len; i++, t++ ){
		if( *p ){
			*t = *p++;
		}
		else {
			*t = ' ';
		}
	}
	m_WriteFlag = TRUE;
	return TRUE;
}
//--------------------------------------------
//データの書き込み（バッファを更新するのみ）
BOOL CHamlog5::SetBinary(USHORT SubIndex, LPBYTE p)
{
	if( SubIndex >= m_FilMax ) return FALSE;

	int len = m_LenTable[SubIndex];
	LPSTR	t;
	int		i;
	for( i = 0, t = m_bp + m_PosTable[SubIndex]; i < len; i++, t++ ){
		*t = *p++;
	}
	m_WriteFlag = TRUE;
	return TRUE;
}

//--------------------------------------------
//データの書き込み（バッファの内容を反映させる）
BOOL CHamlog5::Update(void)
{
	if( m_OpenFlag == FALSE ) return FALSE;

	if( m_WriteFlag == TRUE ){
		m_WriteFlag = FALSE;
		fseek(m_fp, m_Pos, SEEK_SET);
		fwrite(m_bp, 1, m_RecWidth, m_fp);
		if( m_Index >= m_RecMax ){
			m_RecMax++;
		}
	}
	return TRUE;
}
//--------------------------------------------
//データのデコード
void CHamlog5::DecodeData(SDMMLOG *sp)
{
	memset(sp, 0, sizeof(SDMMLOG));
	char bf[1024];


	int YY, MM, DD, HH, mm;
	GetData(itemhamlog5DATE, LPBYTE(bf), 4);
    YY = bf[1];
    MM = bf[2];
    DD = bf[3];
	GetData(itemhamlog5TIME, LPBYTE(bf), 2);
    HH = bf[0];
    mm = bf[1] & 0x7f;
	if( bf[1] & 0x80 ){
		UTCtoJST(YY, MM, DD, HH);
	}
	sp->year = BYTE(YY);
	sp->date = WORD(MM * 100 + DD);
	sp->btime = WORD(((HH * 60 + mm) * 30));
	if(!sp->btime) sp->btime++;

	USHORT usFlag;		// コールサインの形式
	GetData(itemhamlog5FLAG, LPBYTE(&usFlag), sizeof(usFlag));

	LPSTR t, p;
	AnsiString call;
	if( (usFlag & bithamlog5DX) && (usFlag & bithamlog5RevOdr) ){	// KH6/JE3HHT
		AnsiString Add;
		GetData(itemhamlog5IGN, bf);
		p = bf;
		if( *p != ' ' ){
			p = StrDlm(t, p, ' ');
			Add = t;
		}
		p = SkipSpace(p);
		p = StrDlm(t, p, '/');
		call = t;
		GetData(itemhamlog5CALLS, bf); clipsp(bf);
		if( !call.IsEmpty() ) call += '/';
		call += SkipSpace(bf);
		call += Add;
		if( *p ){
			call += '/';
			call += p;
		}
	}
	else {															// JE3HHT/KH6
		GetData(itemhamlog5CALLS, bf); clipsp(bf);
		call = SkipSpace(bf);
		GetData(itemhamlog5IGN, bf);
		p = bf;
		if( *p != ' ' ){
			p = StrDlm(t, p, ' ');
			call += t;
		}
		p = SkipSpace(p);
		if( *p ){
			call += '/';
			call += p;
		}
	}

	StrCopy(sp->call, call.c_str(), MLCALL);
	GetData(itemhamlog5HIS, bf); clipsp(bf);
	StrCopy(sp->ur, bf, MLRST);
	GetData(itemhamlog5MY, bf); clipsp(bf);
	StrCopy(sp->my, bf, MLRST);
	GetData(itemhamlog5MODE, bf); clipsp(bf);
	Log.SetMode(sp, bf);
	GetData(itemhamlog5FREQ, bf); clipsp(bf);
	Log.SetFreq(sp, bf);
	GetData(itemhamlog5NAME, bf); clipsp(bf);
	StrCopy(sp->name, bf, MLNAME);
	GetData(itemhamlog5QTH, bf); clipsp(bf);
	StrCopy(sp->qth, bf, MLQTH);

	GetData(itemhamlog5RMK1, bf);
	SetMMLOGKey(sp, bf);
	clipsp(bf);
	StrCopy(sp->rem, SkipSpace(bf), MLREM);
	GetData(itemhamlog5RMK2, bf);
	SetMMLOGKey(sp, bf);
	clipsp(bf);
	StrCopy(sp->qsl, SkipSpace(bf), MLQSL);
	GetData(itemhamlog5QSL, LPBYTE(bf), 3);
	sprintf(&bf[32], "QSL[%.3s]", bf);
	Log.SetOptStr(2, sp, &bf[32]);
	if( bf[0] == 'N' ){
		sp->send = 'N';
	}
	else if( bf[1] == ' ' ){	// 未発送
		if( (bf[0] == 'J')||(bf[0] == ' ') ){
			sp->send = 0;
		}
		else {
			sp->send = BYTE(tolower(bf[0]));
		}
	}
	else {							// 発送済み
		sp->send = BYTE(toupper(bf[1]));
	}
	sp->recv = bf[2];
	if( sp->recv == ' ' ) sp->recv = 0;
	GetData(itemhamlog5CODE, LPBYTE(bf), 6); clipsp(bf);
	StrCopy(sp->opt1, bf, 6);
	GetData(itemhamlog5GL, LPBYTE(bf), 6); clipsp(bf);
	StrCopy(sp->opt2, bf, 6);
	if( !sp->etime ) sp->etime = sp->btime;
}
//--------------------------------------------
//データのエンコード
void CHamlog5::EncodeData(SDMMLOG *sp)
{
	char bf[1024];

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
	bf[0] = BYTE(YY > 50 ? 19 : 20);
    bf[1] = BYTE(YY);
    bf[2] = BYTE(MM);
    bf[3] = BYTE(DD);
    SetBinary(itemhamlog5DATE, LPBYTE(bf));
	bf[0] = BYTE(HH);
    bf[1] = BYTE(mm);
    SetBinary(itemhamlog5TIME, LPBYTE(bf));

	SetData(itemhamlog5NAME, sp->name);
	SetData(itemhamlog5CODE, sp->opt1);
	SetData(itemhamlog5GL, sp->opt2);
    SetData(itemhamlog5QTH, sp->qth);
	strcpy(bf, Log.GetModeString(sp->mode));
    SetData(itemhamlog5MODE, bf);
	strcpy(bf, Log.GetFreqString(sp->band, sp->fq));
	SetData(itemhamlog5FREQ, bf);

    SetData(itemhamlog5HIS, sp->ur);
    SetData(itemhamlog5MY, sp->my);

	AnsiString REM1, REM2;
    REM1 = sp->rem;
    REM2 = sp->qsl;
	int l = GetLMode(sp->mode);
	if( sp->ur[l] ) AddMMLOGKey(REM1, REM2, &sp->ur[l], "SN");
	if( sp->my[l] ) AddMMLOGKey(REM1, REM2, &sp->my[l], "RN");

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
	char qsl[3];
	if(RemoveL2(rbf, bf, "QSL", sizeof(rbf)-1) == TRUE){
		qsl[0] = rbf[0];
		qsl[1] = rbf[1];
		qsl[2] = rbf[2];
	}
	else {
		qsl[2] = sp->recv;
		if( !qsl[2] ) qsl[2] = ' ';
		if( sp->send == 'N' ){
			qsl[0] = 'N';
			qsl[1] = ' ';
		}
		else if( !sp->send ){
			qsl[0] = 'J';
			qsl[1] = ' ';
		}
		else if( (sp->send >= 0x60)||(sp->send == '?') ){
			qsl[0] = BYTE(toupper(sp->send));
			qsl[1] = ' ';
		}
		else if( isalpha(sp->send) ){
			qsl[0] = sp->send;
			qsl[1] = '*';
		}
		else {
			qsl[0] = 'J';
			qsl[1] = sp->send;
		}
	}
	SetData(itemhamlog5QSL, qsl);
	SetData(itemhamlog5RMK1, REM1.c_str());
    SetData(itemhamlog5RMK2, REM2.c_str());

	BOOL fJA = IsJA(sp->call);
	USHORT	usFlag = USHORT(fJA ? 0 : bithamlog5DX);
	AnsiString Calls, Ign;
	strcpy(bf, sp->call);
	LPSTR	pp, p2, t;
	t = bf;
	if( (pp = strchr(bf, '/')) != NULL ){		// KH6/JE3HHT or JE3HHT/KH6 の形式
		*pp = 0;
		pp++;
		int LenC = strlen(t);
		int LenP = strlen(pp);
		if( (p2 = strchr(pp, '/')) != NULL ){	// KH6/JE3HHT/P		JE3HHT/QRP/3
        	// t = KH6, pp = JE3HHT, p2 = P
			*p2 = 0; p2++;
            if( fJA ){
				Calls = t;
                Ign = pp;
                Ign += "/";
                Ign += p2;
            }
            else {
				Calls = pp;
                Ign = t;
                Ign += "/";
                Ign += p2;
                usFlag |= bithamlog5RevOdr;
            }
		}
        else {
			if( LenC < LenP ){
				Calls = pp;
                Ign = t;
                if( usFlag ) usFlag |= bithamlog5RevOdr;
            }
            else {
				Calls = t;
                Ign = pp;
                if( usFlag ) usFlag |= bithamlog5NorOdr;
            }
		}
	}
    else {
		Calls = sp->call;
    }
	SetBinary(itemhamlog5FLAG, LPBYTE(&usFlag));
	SetData(itemhamlog5CALLS, Calls.c_str());
    LPCSTR pCall = Calls.c_str();
	memset(bf, ' ', 12); bf[13] = 0;
    l = strlen(pCall);
    if( l > 8 ){
		pCall += 8;
        l -= 8;
		memcpy(bf, pCall, l);
    }
	l = strlen(Ign.c_str());
	if( l ){
	    if( l > 12 ) l = 12;
    	memcpy(&bf[12 - l], Ign.c_str(), l);
    }
	SetData(itemhamlog5IGN, bf);
}

