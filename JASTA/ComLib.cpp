//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "ComLib.h"
#include "LogFile.h"
#include "main.h"
//---------------------------------------------------------------------------
char	BgnDir[256];		// 起動ディレクトリ
char    MMLogDir[256];

LCID lcid;
int     MsgEng;
SYSSET  sys;

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
LPSTR StrDupe(LPCSTR s)
{
	LPSTR p = new char[strlen(s)+1];
	strcpy(p, s);
	return p;
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
LPSTR DelLastSpace(LPSTR t)
{
	int l = strlen(t);
	if( l ){
		LPSTR p;
		for( p = t + l - 1; p >= t; p-- ){
			if( (*p == ' ') || (*p == TAB) ){
				*p = 0;
			}
			else {
				break;
			}
		}
	}
	return t;
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
	flag |= (MB_OK | MB_ICONINFORMATION);
//	Application->NormalizeTopMosts();
	::MessageBox(hd, bf, "MMJASTA", flag);
//	Application->RestoreTopMosts();
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
	flag |= (MB_OK | MB_ICONEXCLAMATION);
//	Application->NormalizeTopMosts();
	::MessageBox(hd, bf, MsgEng ? "Error":"ｴﾗｰ", flag);
//	Application->RestoreTopMosts();
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
	flag |= (MB_OK | MB_ICONEXCLAMATION);
//	Application->NormalizeTopMosts();
	::MessageBox(hd, bf, MsgEng?"Warning":"警告", flag);
//	Application->RestoreTopMosts();
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
//	if( Screen->ActiveForm != NULL ) NormalWindow(Screen->ActiveForm);
	flag |= (MB_YESNO | MB_ICONQUESTION);
//	Application->NormalizeTopMosts();
	int r = ::MessageBox(hd, bf, "MMJASTA", flag);
//	Application->RestoreTopMosts();
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
	flag |= (MB_YESNOCANCEL | MB_ICONQUESTION);
//	Application->NormalizeTopMosts();
	int r = ::MessageBox(hd, bf, "MMJASTA", flag);
//	Application->RestoreTopMosts();
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
	flag |= (MB_OKCANCEL | MB_ICONQUESTION);
//	Application->NormalizeTopMosts();
	int r = ::MessageBox(hd, bf, "MMJASTA", flag);
//	Application->RestoreTopMosts();
	MsgBoxFlag--;
	return r;
}
///----------------------------------------------------------------
///
void ErrorFWrite(LPCSTR pName)
{
	if( GetFileAttributes(pName) & FILE_ATTRIBUTE_READONLY ){
		ErrorMB(MsgEng ? "'%s' is read-only (fail to update)." : "'%s'が 読み取り専用 に設定されているため更新に失敗しました.", pName);
	}
	else {
		ErrorMB( "Could not update '%s'", pName );
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
void NumCopy(LPSTR t, LPCSTR p)
{
	p = SkipSpace(p);
	for( ; *p; p++ ){
		if( isdigit(*p) ){
			*t++ = *p;
		}
		else {
			break;
		}
	}
	*t = 0;
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
int IsNumbAll(LPCSTR p)
{
	for( ; *p; p++ ){
		if( !isdigit(*p) ) return 0;
	}
	return 1;
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
		fp->Height = int(OFontHeight * Sx);
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
		fp->Height = int(OFontHeight * Sx);
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
		fp->Height = int(OFontHeight * (Sx < 1.0 ? Sx : 1.0));
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

//---------------------------------------------------------------------------
void __fastcall SetGroupEnabled(TGroupBox *gp)
{
	TControl *tp;
	int i;
	for( i = 0; i < gp->ControlCount; i++ ){
		tp = gp->Controls[i];
		if( tp != NULL ){
			tp->Enabled = gp->Enabled;
			if( tp->ClassNameIs("TComboBox") ){
				((TComboBox *)tp)->SelLength = 0;
			}
			else if( tp->ClassNameIs("TGroupBox") ){
				SetGroupEnabled((TGroupBox *)tp);
			}
		}
	}
	gp->Font->Color = gp->Enabled ? clBlack : clGrayText;
}
//---------------------------------------------------------------------------
void KeyEvent(const short *p)
{
	for( ; *p; p++ ){
		keybd_event(BYTE(*p), 0, *p&0x8000 ? KEYEVENTF_KEYUP : 0, 0);
	}
}
//---------------------------------------------------------------------------
// CMULTクラス
CMULT::CMULT()
{
	m_pBase = NULL;
	m_CNT = m_MAX = 0;
}
//---------------------------------------------------------------------------
void CMULT::Clear(void)
{
	for( int i = 0; i < m_CNT; i++ ){
		delete m_pBase[i].pStr;
	}
	delete m_pBase;
	m_pBase = NULL;
	m_CNT = m_MAX = 0;
}
//---------------------------------------------------------------------------
void CMULT::Alloc(void)
{
	int max = m_MAX ? (m_MAX * 2) : 256;
	MULTSET *pBase = new MULTSET[max];
	if( m_pBase != NULL ){
		memcpy(pBase, m_pBase, sizeof(MULTSET) * m_CNT);
		delete m_pBase;
	}
	m_pBase = pBase;
	m_MAX = max;
}
//---------------------------------------------------------------------------
int CMULT::Add(LPCSTR pKey)
{
	for( int i = 0; i < m_CNT; i++ ){
		if( !strcmp(m_pBase[i].pStr, pKey) ){
			m_pBase[i].Count++;
			return 0;
		}
	}
	if( m_CNT >= m_MAX ) Alloc();
	m_pBase[m_CNT].pStr = StrDupe(pKey);
	m_pBase[m_CNT].Count = 1;
	m_CNT++;
	return 1;
}
//---------------------------------------------------------------------------
int CMULT::Set(LPCSTR pKey, int n)
{
	for( int i = 0; i < m_CNT; i++ ){
		if( !strcmp(m_pBase[i].pStr, pKey) ){
			m_pBase[i].Count = n;
			return 0;
		}
	}
	if( m_CNT >= m_MAX ) Alloc();
	m_pBase[m_CNT].pStr = StrDupe(pKey);
	m_pBase[m_CNT].Count = n;
	m_CNT++;
	return 1;
}
//---------------------------------------------------------------------------
static int CMULTcmpCall(const void *s, const void *t)
{
	MULTSET *sp = (MULTSET *)s;
	MULTSET *tp = (MULTSET *)t;
	return strcmp(sp->pStr, tp->pStr);
}
static int CMULTcmpCount(const void *s, const void *t)
{
	MULTSET *sp = (MULTSET *)s;
	MULTSET *tp = (MULTSET *)t;
	if( sp->Count == tp->Count ){
		return strcmp(sp->pStr, tp->pStr);
	}
	else {
		return tp->Count - sp->Count;
	}
}
void CMULT::Sort(void)
{
	if( m_CNT < 2 ) return;
	qsort(m_pBase, m_CNT, sizeof(MULTSET), CMULTcmpCall);
}
void CMULT::SortCount(void)
{
	if( m_CNT < 2 ) return;
	qsort(m_pBase, m_CNT, sizeof(MULTSET), CMULTcmpCount);
}
//---------------------------------------------------------------------------
int CMULT::GetCount(LPCSTR pKey)
{
	for( int i = 0; i < m_CNT; i++ ){
		if( !strcmp(m_pBase[i].pStr, pKey) ) return m_pBase[i].Count;
	}
	return 0;
}
//---------------------------------------------------------------------------
int CMULT::GetTotal(void)
{
	int sum = 0;
	for( int i = 0; i < m_CNT; i++ ){
		sum += m_pBase[i].Count;
	}
	return sum;
}
//---------------------------------------------------------------------------

