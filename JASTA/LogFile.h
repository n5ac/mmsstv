//---------------------------------------------------------------------------
#ifndef LogFileH
#define LogFileH

#include "ComLib.h"
//---------------------------------------------------------------------------
#define	MMLOGID	"MMLOG DATA Ver1.00\032"

extern const char	*_band[];

//---------------------------------------------------------------------------
// MMLOG Constant
//
enum BD {			/* バンドの番号						*/
	B_NULL,
	B_19,
	B_35,
	B_38,
	B_7,
	B_10,
	B_14,
	B_18,
	B_21,
	B_24,
	B_28,
	B_50,
	B_144,
	B_430,
	B_1200,
	B_2400,
	B_5600,
	B_101G,
	B_104G,
	B_24G,
	B_47G,
	B_75G,
	B_142G,
	B_248G,
	B_4630,
	B_220,
	B_SAT,
};

typedef struct {	// Logの動作オプション
	char	m_TimeZone;				// 表示用タイムゾーン

	int		m_UpperName;			// 大文字への変換
	int		m_UpperQTH;				// 大文字への変換
	int		m_UpperREM;				// 大文字への変換
	int		m_UpperQSL;				// 大文字への変換

	int		m_DefMyRST;				// 0-OFF, 1-ON

	int		m_CopyFreq;				// 0-Band, 1-Freq
	int		m_CopyHis;				// 0-599, 1-599001, 2-599UTC
	int		m_CopyName;				// 0-OFF, 1-ON
	int		m_CopyQTH;				// 0-OFF, 1-ON
	int		m_CopyREM;				// 0-OFF, 1-ON
	int		m_CopyQSL;				// 0-OFF, 1-ON

	int		m_AutoSave;				// 自動セーブ

	int		m_QSOMacroFlag;			// 0-OFF, 1-ON
	int		m_CheckBand;			// 同一バンドでの重複をチェック
	int		m_QSOMacro[5];			// 0-Run 1st, 1-Run 2nd, 2-Run Dupe, 3-S&P 1st, 4-S&P 2nd
	WORD	m_QSOMacroKey[5];
	AnsiString	m_QSOMacroStr[5];

	AnsiString	m_THRTTY;
	AnsiString	m_THSSTV;
	int     m_THTZ;
	int		m_ClipRSTADIF;
	int		m_DateType;
}LOGSET;

#define	MLCALL	16		/* ｺｰﾙｻｲﾝの長さ		*/
#define	MLRST	20		/* RSTナンバの長さ	*/
#define	MLREM	56		/* 記事の長さ		*/
#define	MLNAME	16		/* 名前の長さ		*/
#define	MLQTH	28		/* ＱＴＨの長さ		*/
#define	MLQSL	54		/* ＱＳＬの長さ		*/
#define	MLPOW	4		/* 電力				*/
#define	MLOPT	8		/* オプション		*/

#define MODEMAX	48

#define	YEAR(c)	(((c)<50)?(2000+(c)):(1900+(c)))

#pragma option -a-	// パックの指示
typedef struct {			/* ＭＭＬＯＧデータ形式	*/
	BYTE	year;		/* 年		*/
	WORD	date;			/* 日付		*/
	WORD	btime;			/* 開始時刻	*/
	WORD	etime;			/* 終了時刻	*/
	char	call[MLCALL+1];	/* ｺｰﾙｻｲﾝ	*/
	char	ur[MLRST+1];	/* T ﾘﾎﾟｰﾄ	*/
	char	my[MLRST+1];	/* R ﾘﾎﾟｰﾄ	*/
	BYTE	band;		/* ﾊﾞﾝﾄﾞ	*/
	SHORT	fq;				/* 周波数	*/
	BYTE	mode;		/* ﾓｰﾄﾞ		*/
	char	pow[MLPOW+1];	/* 電力		*/
	char	name[MLNAME+1];	/* 名前		*/
	char	qth[MLQTH+1];	/* QTH		*/
	char	qsl[MLQSL+1];	/* QSL		*/
	char	send;			/* QSL 送信	*/
	char	recv;			/* QSL 受信	*/
	char	cq;				/* cq/call	*/
	WORD	env;			/* 自局環境	*/
	char	rem[MLREM+1];	/* 記事		*/
	char	opt1[MLOPT+1];	/* ｵﾌﾟｼｮﾝ	*/
	char	opt2[MLOPT+1];	/* ｵﾌﾟｼｮﾝ2	*/
}SDMMLOG;

#define	FHDOFF	256			/* 先頭のヘッダのオフセット		*/
#define	UMODEMAX	32
typedef struct {
	char	id[20];			/* ファイルバージョン			*/
	char	dmy[4];			/* ダミー領域					*/
	USHORT	mlt;			/* マルチ情報のサイズ			*/
	char	td;				/* 時差コード					*/
	char	hash;			/* ハッシュデータ格納フラグ		*/
	long	size;			/* データサイズ					*/
	char	master;			/* マスターファイルフラグ		*/
	char	dm2[15];		/* ダミー２						*/
	char	mode[UMODEMAX][6];	/* ユーザ定義モード				*/
}FHD;
#pragma option -a.	// パック解除の指示

class CIndex
{
private:
	int		m_IndexMax;		// 確保中のインデックスの数
	int		m_IndexCnt;		// 現在のインデックスの数
	LPSTR	pIndex;			// インデックス領域のポインタ(16バイトインデックス)
	BYTE	*pMult;			// マルチ情報のポインタ
public:
	CIndex();
	~CIndex();
	void AllocIndex(int n);
	void ClearIndex(void);
	void WriteIndex(LPCSTR pCall, int n);
	void ReadIndex(LPSTR pCall, int n);
	LPSTR PointIndex(int n);
	void AddBlock(LPCSTR p, int len);
	int ReadIndex(int handle, FHD *hp);
	void MakeIndex(int handle, FHD *hp);
	int WriteIndex(int handle, FHD *hp);


};

#define	FINDMAX		32768
class CLogFind
{
public:
	int			m_FindCnt;
	int			m_FindCmp1Max;
	int			m_FindCmp2Max;
	int			m_FindStr1Max;
	int			m_FindStr2Max;

	int			*pFindTbl;
	AnsiString	m_FindStr;
public:
	CLogFind();
	~CLogFind();
	inline int GetCount(void){return m_FindCnt;};
	inline void Clear(void){
		m_FindCnt = m_FindCmp1Max = m_FindCmp2Max = m_FindStr1Max = m_FindStr2Max = 0;
	};
	int Add(int n);
	void Ins(int n);

	inline void Write(int n){
		pFindTbl[m_FindCnt] = n;
		m_FindCnt++;
	};
	inline void SetText(LPCSTR p){
		if( p != m_FindStr.c_str() ){
			m_FindStr = p;
		}
	};
	inline LPCSTR GetText(void){
		return m_FindStr.c_str();
	};
	inline void ClearText(void){
		m_FindStr = "";
	};
};

class CLogFile
{
private:
	int		m_Open;			// オープンフラグ
	int     m_ReadOnly;
	int		m_EditFlag;		// 編集フラグ
	int		m_Handle;		// ファイルハンドル
	CIndex	m_Index;		// 現在オープン中のインデックス
	FHD		m_fhd;			// 現在オープン中のファイルヘッダ
	AnsiString	m_Name;		// ログファイルの名前
	char	m_modebuf[8];
	char    LogDir[256];
public:
	AnsiString	m_FileName;	// ログファイルの名前（フルパス）

	int		m_CurNo;
	int		m_CurChg;
	SDMMLOG	m_sd;
	SDMMLOG	m_bak;
	SDMMLOG	m_asd;

	CLogFind	m_Find;		// カレント検索データ
	LOGSET		m_LogSet;
private:
	void SetHisRST(SDMMLOG *sp);

public:
	CLogFile();
	~CLogFile();

	int Open(LPCSTR pName, int enbwrt);
	int Close(void);

	void ReadIniFile(LPCSTR pKey, TIniFile *pIniFile);
	void WriteIniFile(LPCSTR pKey, TIniFile *pIniFile);

	void MakeIndex(void);

	inline IsEdit(void){return m_EditFlag;};
	void MakePathName(LPCSTR pName);
	void MakeName(LPCSTR pName);
	inline LPCSTR GetName(void){return m_Name.c_str();};

	inline int GetCount(void){return m_fhd.size;};
	inline int IsOpen(void){return m_Open;};
	int GetData(SDMMLOG *sp, int n);
	int PutData(SDMMLOG *sp, int n);

	LPCSTR GetDateString(SDMMLOG *sp, int sw);
	inline LPCSTR GetDateString(SDMMLOG *sp){ return GetDateString(sp, m_LogSet.m_DateType); };
	LPCSTR GetTimeString(WORD d);


	LPCSTR GetModeString(BYTE m);
	void SetMode(SDMMLOG *sp, LPCSTR s);

	LPCSTR GetFreqString(BYTE b, short fq);
	void SetFreq(SDMMLOG *sp, LPCSTR p);
	LPCSTR GetOptStr(int n, SDMMLOG *sp);
	void SetOptStr(int n, SDMMLOG *sp, LPCSTR pOpt);

	void CopyAF(void);

	void SortDate(int bb, int eb);
	int FindSameBand(void);
	int FindSameDate(SYSTEMTIME *pUTC);

	void FindStrSet(CLogFind *fp, LPCSTR pCall);
	int FindCmpSet(CLogFind *fp, LPCSTR pCall);
	int FindClipSet(CLogFind *fp, LPCSTR pCall);
	int FindSet(CLogFind *fp, LPCSTR pCall);
	int Find(LPCSTR pCall, int b, int dir);
	int IsAlready(LPCSTR pCall);
	void DeleteAll(void);
	void DeleteLast(void);
	void Delete(int top, int end);
	void Insert(int n, SDMMLOG *sp);

	void InitCur(void);
	void SetLastPos(void);

	int ReadAscii(SDMMLOG *sp, LPSTR p);
	CIndex *GetIndex(void){return &m_Index;};
};

void JSTtoUTC(int &Year, int &Mon, int &Day, int &Hour);
void JSTtoUTC(SDMMLOG *sp);
void UTCtoJST(int &Year, int &Mon, int &Day, int &Hour);
void UTCtoJST(SDMMLOG *sp);
void UTCtoJST(SYSTEMTIME *tp);
void mBandToBand(SDMMLOG *sp, LPCSTR p);
LPCSTR FreqTomBand(SDMMLOG *sp);
int GetLMode(BYTE m);

extern	CLogFile	Log;
extern	const char	MONN[];
extern	const char	MONU[];
#endif

