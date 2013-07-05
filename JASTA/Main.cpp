//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Main.h"
#include "OptDlg.h"
#include "TextEdit.h"
#include "QsoDlg.h"
#include "LogConv.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainWnd *MainWnd;
//---------------------------------------------------------------------------
#define TEMPLOGNAME "MMJASTA.MDT"
//---------------------------------------------------------------------------
__fastcall TMainWnd::TMainWnd(TComponent* Owner)
	: TForm(Owner)
{
	lcid = GetThreadLocale() & 0x00ff;
	if( lcid != LANG_JAPANESE ){                    // English
		MsgEng = 1;
		sys.m_Category = 1;
	}
	else {
		MsgEng = 0;
		sys.m_Category = 0;
	}

	if( ParamCount() >= 0 ){
		SetDirName(BgnDir, AnsiString(ParamStr(0)).c_str());
	}
	else {
		SetCurDir(BgnDir, sizeof(BgnDir));
	}
	strcpy(MMLogDir, "C:\\MMSSTV");
	::GetLocalTime(&m_now);
	sys.m_Year = m_now.wYear;
	if( m_now.wMonth < 8 ) sys.m_Year--;
	sys.m_Call = "";
	sys.m_CName = "JASTA SSTV activity contest";
	sys.m_TSize = 2;
	m_FIndex = 1;
	Caption = VERTTL;
	ReadRegister();
	SetLanguage();

	char bf[256];
	sprintf(bf, "%sARRL.DX", BgnDir);
	Cty.Load(bf);

	m_ReCalc = 0;
	m_pEditLog = NULL;
	m_pEditSum = NULL;
	m_pEditAna = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::SetLanguage(void)
{
	Memo->Font->Pitch = fpFixed;
	if( MsgEng ){
		Font->Name = "Arial";
		Font->Charset = ANSI_CHARSET;
		Memo->Font->Name = "Courier New";
		Memo->Font->Charset = ANSI_CHARSET;

		KF->Caption = "&File";
		KV->Caption = "&View";
		KH->Caption = "&Help";
		KFS->Caption = "&Setup...";
		KFL->Caption = "&Load log...";
		KFR->Caption = "&Re-totalize";
		KVL->Caption = "Log sheet...";
		KVS->Caption = "Summary sheet...";
		KVA->Caption = "Analysis file...";
		KHM->Caption = "Download latest version...";
		KHR->Caption = "Reflector...";
		KHW->Caption = "JASTA Home page...";
	}
	else {
		Font->Name = "ＭＳ Ｐゴシック";
		Font->Charset = SHIFTJIS_CHARSET;
		Memo->Font->Name = "ＭＳ ゴシック";
		Memo->Font->Charset = SHIFTJIS_CHARSET;

		KF->Caption = "ファイル(&F)";
		KV->Caption = "表示(&V)";
		KH->Caption = "ヘルプ(&H)";
		KFS->Caption = "セットアップ(&S)...";
		KFL->Caption = "ログファイルのロード(&L)...";
		KFR->Caption = "再集計(&R)";
		KVL->Caption = "ログシート...";
		KVS->Caption = "サマリシート...";
		KVA->Caption = "分析ファイル...";
		KHM->Caption = "最新版のダウンロード...";
		KHR->Caption = "メーリングリスト...";
		KHW->Caption = "JASTAのホームページ...";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	Log.Close();
	unlink(Log.m_FileName.c_str());
	WriteRegister();

	if( m_pEditLog != NULL ) delete m_pEditLog;
	if( m_pEditSum != NULL ) delete m_pEditSum;
	if( m_pEditAna != NULL ) delete m_pEditAna;
	CanClose = TRUE;
}
//---------------------------------------------------------------------------
// レジストリからの読み出し
void __fastcall TMainWnd::ReadRegister(void)
{
	char	bf[256];

	sprintf(bf, "%sMMJASTA.ini", BgnDir);
	TIniFile	*pIniFile = new TIniFile(bf);

	MsgEng = pIniFile->ReadInteger("Language", "English", MsgEng);


	AnsiString  as;
	as = pIniFile->ReadString("Folder", "Log", MMLogDir);
	StrCopy(MMLogDir, as.c_str(), sizeof(MMLogDir)-1);
	m_FIndex = pIniFile->ReadInteger("Folder", "Index", m_FIndex);

	sys.m_Call = pIniFile->ReadString("Define", "Call", sys.m_Call);
	sys.m_Name = pIniFile->ReadString("Define", "Name", sys.m_Name);
	sys.m_Zip = pIniFile->ReadString("Define", "Zip", sys.m_Zip);
	sys.m_Adr = pIniFile->ReadString("Define", "Adr", sys.m_Adr);
	sys.m_Mail = pIniFile->ReadString("Define", "Mail", sys.m_Mail);
	sys.m_License = pIniFile->ReadString("Define", "License", sys.m_License);
	sys.m_Pow = pIniFile->ReadString("Define", "Power", sys.m_Pow);
	sys.m_Sexual = pIniFile->ReadInteger("Define", "Sexual", sys.m_Sexual);
	sys.m_TSize = pIniFile->ReadInteger("Define", "TSize", sys.m_TSize);

	sys.m_Category = pIniFile->ReadInteger("Define", "Category", sys.m_Category);
	sys.m_First = pIniFile->ReadInteger("Define", "First", sys.m_First);
	delete pIniFile;
}
//---------------------------------------------------------------------------
// レジストリへの書き込み
void __fastcall TMainWnd::WriteRegister(void)
{
	CWaitCursor wait;
	char	bf[256];
	sprintf(bf, "%sMMJASTA.ini", BgnDir);

	try{
		TIniFile	*pIniFile = new TIniFile(bf);

		pIniFile->WriteInteger("Language", "English", MsgEng);
		pIniFile->WriteString("Folder", "Log", MMLogDir);
		pIniFile->WriteInteger("Folder", "Index", m_FIndex);

		pIniFile->WriteString("Define", "Call", sys.m_Call);
		pIniFile->WriteString("Define", "Call", sys.m_Call);
		pIniFile->WriteString("Define", "Name", sys.m_Name);
		pIniFile->WriteString("Define", "Zip", sys.m_Zip);
		pIniFile->WriteString("Define", "Adr", sys.m_Adr);
		pIniFile->WriteString("Define", "Mail", sys.m_Mail);
		pIniFile->WriteString("Define", "License", sys.m_License);
		pIniFile->WriteString("Define", "Power", sys.m_Pow);
		pIniFile->WriteInteger("Define", "Sexual", sys.m_Sexual);
		pIniFile->WriteInteger("Define", "TSize", sys.m_TSize);

		pIniFile->WriteInteger("Define", "Category", sys.m_Category);
		pIniFile->WriteInteger("Define", "First", sys.m_First);
		delete pIniFile;

	}
	catch(...){
		ErrorMB(MsgEng?"Cannot update MMJASTA.INI":"MMJASTA.INIを更新できません.");
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::SetTitle(void)
{
	if( m_SrcName.IsEmpty() ){
		Caption = VERTTL;
	}
	else {
		char bf[256];
		sprintf(bf, "%s [%s]", VERTTL, m_SrcName.c_str());
		Caption = bf;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::GridDrawCell(TObject *Sender, int Col,
	  int Row, TRect &Rect, TGridDrawState State)
{
	char	bf[256];

	Grid->Canvas->FillRect(Rect);
	int X = Rect.Left + 4;
	int Y = Rect.Top + 2;

	if( Row ){
		Row--;
		bf[0] = 0;
		SDMMLOG sd;
		int f = ( Row < Log.GetCount() ) ? TRUE : FALSE;
		if( f ) Log.GetData(&sd, Row);
		switch(Col){
			case 0:		// No
				wsprintf(bf, "%u", Row + 1);
				break;
			case 1:     // Date
				if( f ) wsprintf(bf, "%02u/%02u/%02u", sd.year, sd.date / 100, sd.date % 100);
				break;
			case 2:     // Time
				if( f ) wsprintf(bf, "%02u%02u", (sd.btime / 30) / 60, (sd.btime / 30) % 60);
				break;
			case 3:     // Band
				if( f ) wsprintf(bf, "%s", _band[sd.band]);
				break;
			case 4:     // Call
				if( f ) strcpy(bf, sd.call);
				break;
			case 5:
				if( f ) strcpy(bf, sd.ur);
				break;
			case 6:
				if( f ) strcpy(bf, sd.my);
				break;
			case 7:
				if( f ) strcpy(bf, sd.opt1);
				break;
			case 8:
				if( f ) strcpy(bf, sd.qsl);
				break;
		}
		Grid->Canvas->TextRect(Rect, X, Y, bf);
	}
	else {		// タイトル
		LPCSTR	_tt[]={
			"No","Date","UTC","Band","Call","Sent","Rcvd","Multi","Point"
		};
		Grid->Canvas->TextRect(Rect, X, Y, _tt[Col]);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::KFLClick(TObject *Sender)
{
	if( sys.m_Call.IsEmpty() ){
		KFSClick(NULL);
		if( sys.m_Call.IsEmpty() ) return;
	}
	OpenDialog->Filter = MsgEng ? "MMLOG data files(mdt)|*.mdt|ADIF files(adi)|*.adi|LOG200 files(l2)|*.l2|Turbo HAMLOG files(dbs)|*.dbs" :
								  "MMLOGデータファイル(mdt)|*.mdt|ADIF ファイル(adi)|*.adi|LOG200 ファイル(l2)|*.l2|Turbo HAMLOG ファイル(dbs)|*.dbs";
	OpenDialog->FilterIndex = m_FIndex;
	OpenDialog->InitialDir = MMLogDir;
	if( OpenDialog->Execute() == TRUE ){
		SetDirName(MMLogDir, AnsiString(OpenDialog->FileName).c_str());	//ja7ude 0525
		m_FIndex = OpenDialog->FilterIndex;
		m_SrcName = OpenDialog->FileName.c_str();
		Exec(m_SrcName.c_str(), TRUE);
		SetTitle();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::Exec(LPCSTR pName, int loadorg)
{
	m_ReCalc = 0;
	CWaitCursor w;

	LPCSTR pEXT = GetEXT(pName);
	if( !strcmpi(pEXT, "ADI") ){
		pName = ConvADIF(pName, loadorg);
		if( pName == NULL ) return;
	}
	else if( !strcmpi(pEXT, "L2") ){
		pName = ConvLog200(pName, loadorg);
		if( pName == NULL ) return;
	}
	else if( !strcmpi(pEXT, "DBS") ){
		pName = ConvHAMLOG(pName, loadorg);
		if( pName == NULL ) return;
	}

	CLogFile orgLog;
	orgLog.Open(pName, FALSE);
	if( !orgLog.IsOpen() ) return;

	char templog[256];
	sprintf(templog, "%sJASTA.$$$", BgnDir);
	Log.Open(templog, TRUE);
	if( !Log.IsOpen() ) return;
	Log.DeleteAll();

	m_errNOF = m_errDUP = m_errINV = m_errMLT = m_errBND = 0;

	SDMMLOG	sd;
	int i, n;
	n = 0;
	for( i = 0; i < orgLog.GetCount(); i++ ){
		orgLog.GetData(&sd, i);
		JSTtoUTC(&sd);
		int yy = sd.year + 2000;
		int mm = sd.date / 100;
		if( (sd.mode == 8) && (sd.band >= B_35) && (yy == sys.m_Year) && (mm == 8) && IsValidRST(sd.ur) ){
			AdjustData(&sd);
			Log.PutData(&sd, n);
			n++;
		}
	}
	orgLog.Close();
	Log.SortDate(0, Log.GetCount());
	Grid->RowCount = (n >= 8) ? (n + 1) : 9;
	Grid->Invalidate();
	Calc(ClipCall(sys.m_Call.c_str()));
}
//---------------------------------------------------------------------------
int __fastcall TMainWnd::IsValidRST(LPSTR pRST)
{
	int l = strlen(pRST);
	if( l <= 3 ) return 0;
	LPCSTR p = pRST;
	for( ; *p; p++ ){
		if( !isdigit(*p) ) return 0;
	}
	int n = atoin(&pRST[3], -1);
	if( (n < 1000) && (l != 6) ){
		sprintf(&pRST[3], "%03u", n);
	}
	return 1;
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::AdjustData(SDMMLOG *sp)
{
	if( sp->band <= B_28 ){
		strcpy(sp->qsl, "1");
	}
	else if( sp->band <= B_430 ){
		strcpy(sp->qsl, "2");
	}
	else {
		strcpy(sp->qsl, "3");
	}
	if( IsJA(sp->call) ){
		char c;
		char bf[MLCALL+1];
		StrCopy(bf, ClipCC(sp->call), MLCALL);
		if( (bf[0] == '7') && (bf[1] != 'J') ){
			c = '1';
		}
		else {
			c = bf[2];
		}
		sprintf(sp->opt1, "JA%c", c);
		strcpy(sp->opt2, "AS");
	}
	else if( Cty.IsData() ){
		int ref = Cty.GetRefP(sp->call);
        if( !ref ){
			LPCSTR pCC = ClipCC(sp->call);
			ref = Cty.GetRefP(pCC);
		}
		if( ref ){
			LPCSTR pDX = Cty.GetCountry(ref);
			if( (pDX != NULL) && *pDX && (strchr(pDX, '?') == NULL) ){
				LPCSTR p = lastp(sp->opt1);
                p+=2;
                if( *p != 0x01 ) StrCopy(sp->opt1, pDX, MLOPT);
			}
			pDX = Cty.GetCont(ref);
			if( (pDX != NULL) && *pDX && (strchr(pDX, '?') == NULL) ){
				LPCSTR p = lastp(sp->opt2);
                p+=2;
				if( *p != 0x01 ) StrCopy(sp->opt2, pDX, MLOPT);
			}
		}
	}
	DelLastSpace(sp->opt1);
	DelLastSpace(sp->opt2);
	if( !IsValidRST(sp->my) ){
		strcpy(sp->qsl, "0");
		strcpy(sp->opt1, "*INV*");
		m_errINV++;
	}
	else if( (strstr(sp->rem, "NOF") != NULL) || (strstr(sp->rem, "nof") != NULL) ){
		strcpy(sp->qsl, "0");
		strcpy(sp->opt1, "*NOF*");
		m_errNOF++;
	}
}
void __fastcall TMainWnd::ShowErr(StrText *tp, int n, SDMMLOG *sp, LPCSTR pKey)
{
	m_ShowErrCount++;
	if( m_ShowErrCount >= 100 ){
		if( m_ShowErrCount == 100 ){
			tp->Printf("Too many Warnings\r\n");
		}
	}
	else {
		tp->Printf("%-4u %u/%02u %02u%02uZ %5s %-13s %-7s %-7s : %s\r\n",
			n+1,
			sp->date/100, sp->date%100,
			(sp->btime/30)/60, (sp->btime/30)%60,
			_band[sp->band],
			sp->call, sp->ur, sp->my,
			pKey
		);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::Calc(LPCSTR pName)
{
	StrText str(32768);
	m_ShowErrCount = 0;

	m_Days = 0;
	if( pName == NULL ){
		pName = "OUTPUT";
	}
	sprintf(m_LogName, "%s%s.txt", BgnDir, pName);
	sprintf(m_SumName, "%s%s.sum", BgnDir, pName);
	sprintf(m_AnaName, "%s%s.ana", BgnDir, pName);
	FILE *pfLog = fopen(m_LogName, "wt");

	fprintf(pfLog, "JASTA Contest Log. Year:%04u  Contest Name:%s\n", sys.m_Year, sys.m_CName.c_str());
	fprintf(pfLog, "Call Sign:%s\n\n", sys.m_Call.c_str());
	fprintf(pfLog, "Date Time UTC     Band  Station WKD. Sent No. RCVD No. Multi. Point\n\n");

	int Score = 0;
	int i;
	int add = -1;
	m_CountHF = m_CountVU = m_CountS = 0;
	m_MJA.Clear();
	m_MDX.Clear();
	m_DUP.Clear();
	char mlt[32];
	int mf = 0;
	for( i = 0; i < Log.GetCount(); i++ ){
		SDMMLOG sd;
		Log.GetData(&sd, i);
		int wf = 0;
		mlt[0] = 0;
		int dd = sd.date % 100;
		if( add != dd ){     // 日付マルチ
			add = dd;
			m_Days++;
			m_DUP.Clear();
		}
		if( sd.opt1[0] != '*' ){
			if( !m_DUP.Add(clipsp(sd.call)) ){
				strcpy(sd.qsl, "0");
				strcpy(sd.opt1, "*DUP*");
				m_errDUP++;
				wf++;
			}
			else if( (!sd.opt1[0]) ||
					 (strchr(sd.opt1, '?') != NULL) ||
					 (!strcmp(sd.opt1, "JA")) ||
					 IsNumbAll(sd.opt1)
			){
				m_errMLT++;
				char bf[128];
				sprintf(bf, "DXCC Unknown [%s]", sd.opt1);
				ShowErr(&str, i, &sd, bf);
				if( !mf ){
					MoveGrid(i);
					mf++;
				}
			}
			else if( !strncmp(sd.opt1, "JA", 2) ){
				if( m_MJA.Add(sd.opt1) ) strcpy(mlt, sd.opt1);
			}
			else {
				if( sd.band >= B_144 ){
					m_errBND++;
					char bf[128];
					sprintf(bf, "DX on 2m & down");
					ShowErr(&str, i, &sd, bf);
					if( !mf ){
						MoveGrid(i);
						mf++;
					}
				}
				if( m_MDX.Add(sd.opt1) ) strcpy(mlt, sd.opt1);
			}
			int d = atoin(sd.qsl, -1);
			Score += d;
			switch(d){
				case 1:
					m_CountHF++;
					break;
				case 2:
					m_CountVU++;
					break;
				case 3:
					m_CountS++;
					break;
			}
		}
		if( wf ) Log.PutData(&sd, i);
		if( sd.opt1[0] == '*' ){
			strcpy(mlt, sd.opt1);
			ShowErr(&str, i, &sd, sd.opt1);
		}
//     1      123456789011231234567890123123456789012345678912345678
// 8/01 12:34U 　　　　14   JA1QRZ       595001    595010   JA1      1

		fprintf(pfLog, "%u/%02u %02u:%02uU%11s   %-13s%-10s%-9s%-8s%s\n",
			sd.date / 100, sd.date % 100,
			(sd.btime/30)/60, (sd.btime/30)%60,
			_band[sd.band],
			sd.call,
			sd.ur,
			sd.my,
			mlt,
			sd.qsl
		);
	}

	int MJA = m_MJA.GetCount();
	int MDX = m_MDX.GetCount();
	int MDAY = m_Days;
	if( MDAY > 10 ) MDAY = 10;
	int MALL = MJA+MDX+MDAY;
	int Total = MALL * Score;

	if( m_errMLT || m_errINV || m_errDUP || m_errNOF || m_errBND ){
		str.Printf("Warning\t" );
		if( m_errDUP ) str.Printf("DUP(%u)\t", m_errDUP);
		if( m_errNOF ) str.Printf("NOFACE(%u)\t", m_errNOF);
		if( m_errINV ) str.Printf("Invalid(%u)\t", m_errINV);
		if( m_errMLT ) str.Printf("DXCC Unknown(%u)\t", m_errMLT);
		if( m_errBND ) str.Printf("DX on 2m & down(%u)", m_errBND);
		str.Printf("\r\n" );
	}
	else {
		str.Printf("No Warning\r\n");
	}
	str.Printf("\r\nMulti\tJAs(%u) + DX(%u) + DAYs(%u/%u) = %u  \r\n", MJA, MDX, MDAY, m_Days, MALL);
	str.Printf("Points\tHF(%u)+VU(%u)+S(%u) = %u\r\n", m_CountHF, m_CountVU*2, m_CountS*3, Score);
	str.Printf("Total\t%u x %u = %u\r\n", Score, MALL, Total);
	str.Printf("Output\t%s   %s", m_LogName, m_SumName);
	Memo->Text = str.GetText();

	fprintf(pfLog, "-------------------------------------------------------------------\n");
	fprintf(pfLog, "Total                                            JA's area:%u\n", MJA);
	fprintf(pfLog, "%2u days                                              DXCC :%-3u %9u Points\n",m_Days, MDX, Score);
	fprintf(pfLog, "Total point %u*(%u+%u+%u)=%u\n", Score, MJA, MDX, MDAY, Total);
	if( m_errDUP || m_errNOF || m_errINV ){
		fprintf(pfLog, "\n[Note]\n");
		if( m_errDUP ) fprintf(pfLog, "*DUP* : Duplicate QSO\n");
		if( m_errNOF ) fprintf(pfLog, "*NOF* : Nothing his/her face in the received PIC\n");
		if( m_errINV ) fprintf(pfLog, "*INV* : Invalid QSO\n");
	}
	fclose(pfLog);


    const LPCSTR _tsize[]={"S","M","L","LL","?","?"};

	pfLog = fopen(m_SumName, "wt");
	if( MsgEng ){
		fprintf(pfLog, "\n%04u JASTA SSTV CONTEST SUMMARY SHEET\n\n", sys.m_Year);
		fprintf(pfLog, "1. Contest Name      : %s\n", sys.m_CName.c_str());
		fprintf(pfLog, "2. Entry Section     : %s\n", sys.m_Category ? "S" : "J");
		fprintf(pfLog, "3. First time entry  : %s\n", sys.m_First ? "Yes" : "No");
		fprintf(pfLog, "4. OM or YL(XYL)     : %s\n", sys.m_Sexual ? "YL/XYL":"OM");
		fprintf(pfLog, "5. Size of T-shirts  : %s (for an Activity Premium)\n", _tsize[sys.m_TSize]);
		fprintf(pfLog, "6. Callsign          : %s\n", sys.m_Call.c_str());
		fprintf(pfLog, "7. Address           : Postal Zip Code %s\n", sys.m_Zip.c_str());
		fprintf(pfLog, "                     : %s\n", sys.m_Adr.c_str());
		fprintf(pfLog, "    e-mail           : %s\n", sys.m_Mail.c_str());
		fprintf(pfLog, "8. Name              : %s\n", sys.m_Name.c_str());
		fprintf(pfLog, "9. Licensed Class    : %s\n", sys.m_License.c_str());
		fprintf(pfLog, "10.Output Power      : %s\n", sys.m_Pow.c_str());
		fprintf(pfLog, "11.Result\n");
		fprintf(pfLog, " (1)Confirmed QSO number of 3.5-28MHz       %5u x 1 = %u points\n", m_CountHF, m_CountHF);
		fprintf(pfLog, " (2)Confirmed QSO number of 50-430MHz       %5u x 2 = %u points\n", m_CountVU, m_CountVU*2);
		fprintf(pfLog, " (3)Confirmed QSO number of 1200MHz&UP      %5u x 3 = %u points\n", m_CountS, m_CountS*3);
		fprintf(pfLog, "\n\n");
		fprintf(pfLog, "            Total QSO points : (%u)+(%u)+(%u) = %u\n", m_CountHF, m_CountVU*2, m_CountS*3, Score);
		fprintf(pfLog, "\n");
		fprintf(pfLog, " (4)Total Areas of JA's                = %u\n", MJA);
		fprintf(pfLog, " (5)Total Entity of DXCC except JA     = %u\n", MDX);
		fprintf(pfLog, " (6)Total Worked Days(Maximum 10 days) = %u\n", MDAY );
		fprintf(pfLog, "\n");
		fprintf(pfLog, "            Total Multi      ： (%u)+(%u)+(%u) = %u\n", MJA, MDX, MDAY, MALL);
		fprintf(pfLog, "\n\n");
		fprintf(pfLog, "   Total Points : %u x %u = %u points\n", Score, MALL, Total);
		fprintf(pfLog, "\n");
		fprintf(pfLog, "I declare my honor that in this contest I have operated\n" );
		fprintf(pfLog, "my station within the limitations of my license and have\n" );
		fprintf(pfLog, "observed fully the rules and regulations of the contest.\n" );
		fprintf(pfLog, "\n" );
		fprintf(pfLog, "      %04u.%02u.%02u\n", m_now.wYear, m_now.wMonth, m_now.wDay);
		fprintf(pfLog, "      Singnature _________________\n" );
		fprintf(pfLog, "\n");
		fprintf(pfLog, "***************************************************************************\n");
		fprintf(pfLog, "(Remarks)\n\n\n");
		fprintf(pfLog, "---------------------------------------------------------------------------\n");
	}
	else {
		fprintf(pfLog, "\nＪＡＳＴＡ 主催ＳＳＴＶコンテストサマリーシート(様式ＡＳ）\n\n");
		fprintf(pfLog, "１．コンテスト名称   : %s\n", sys.m_CName.c_str());
		fprintf(pfLog, "２．参加部門         : %s\n", sys.m_Category ? "S" : "J");
		fprintf(pfLog, "３．初参加の区別     : %s\n", sys.m_First ? "初参加" : "過去にも参加した");
		fprintf(pfLog, "４．性別             : %s\n", sys.m_Sexual ? "YL/XYL":"OM");
		fprintf(pfLog, "５. Ｔシャツのサイズ : %s (アクティブ賞当選時用)\n", _tsize[sys.m_TSize]);
		fprintf(pfLog, "６．コールサイン     : %s\n", sys.m_Call.c_str());
		fprintf(pfLog, "７．住所             : 〒%s\n", sys.m_Zip.c_str());
		fprintf(pfLog, "                     : %s\n", sys.m_Adr.c_str());
		fprintf(pfLog, "    e-mail           : %s\n", sys.m_Mail.c_str());
		fprintf(pfLog, "８. 氏名             : %s\n", sys.m_Name.c_str());
		fprintf(pfLog, "９. 無線従事者の資格 : %s\n", sys.m_License.c_str());
		fprintf(pfLog, "10. 空中線電力       : %s\n", sys.m_Pow.c_str());
		fprintf(pfLog, "11. 結果\n");
		fprintf(pfLog, " (1) 3.5～28MHz帯での交信局数       %5u x 1 = %u 点\n", m_CountHF, m_CountHF);
		fprintf(pfLog, " (2) 50～430MHz帯での交信局数       %5u x 2 = %u 点\n", m_CountVU, m_CountVU*2);
		fprintf(pfLog, " (3) 1200MHz帯以上での交信局数      %5u x 3 = %u 点\n", m_CountS, m_CountS*3);
		fprintf(pfLog, "\n\n");
		fprintf(pfLog, "                        交信点数：(%u)+(%u)+(%u) = %u\n", m_CountHF, m_CountVU*2, m_CountS*3, Score);
		fprintf(pfLog, "\n");
		fprintf(pfLog, " (4) 交信したJAのエリア数                  = %u\n", MJA);
		fprintf(pfLog, " (5) 交信したDXCCエンティティ数（JAを除く）= %u\n", MDX);
		fprintf(pfLog, " (6) 運用日数（最大１０）                  = %u\n", MDAY );
		fprintf(pfLog, "\n");
		fprintf(pfLog, "                        マルチ：(%u)+(%u)+(%u) = %u\n", MJA, MDX, MDAY, MALL);
		fprintf(pfLog, "\n\n");
		fprintf(pfLog, "   総得点：交信点数×マルチ = %u × %u = %u 点\n", Score, MALL, Total);
		fprintf(pfLog, "\n");
		fprintf(pfLog, "  以上、ここに提出するサマリーは運用した事実と相違ない事を誓います。\n");
		fprintf(pfLog, "\n");
		fprintf(pfLog, "      %04u年 %02u月 %02u日\n", m_now.wYear, m_now.wMonth, m_now.wDay);
		fprintf(pfLog, "\n");
		fprintf(pfLog, "***************************************************************************\n");
		fprintf(pfLog, "（意見等あればこの欄に記入下さい。）\n\n\n");
		fprintf(pfLog, "---------------------------------------------------------------------------\n");
	}
	fclose(pfLog);

	MakeANA();
	if( m_errMLT ){
		WarningMB(MsgEng ? "There are unknown DXCC entities.\r\nDouble click on [DXCC Unknown] in the status window, and then edit the multi." : "未確定のDXCCエンティティが存在します.\r\n「DXCC Unknown」をダブルクリックし修正して下さい.");
		return;
	}

	KVAClick(NULL);
	KVLClick(NULL);
	KVSClick(NULL);
}
//---------------------------------------------------------------------------
int __fastcall TMainWnd::GetBNO(int b)
{
	// 3.5, 7, 14, 18, 21, 24, 28, 50, 144, 430, 1200, 2400+
	if( b >= B_2400 ){
		b = 9;
	}
	else {
        switch(b){
            case B_35:
            case B_38:
                b = 0;
                break;
            case B_7:
                b = 1;
                break;
            default:
                if( b >= B_14 ){
                    b -= B_14;
                    b += 2;
                }
                else {
                    b = 11;
                }
                break;
        }
	}
	return b;
}
//---------------------------------------------------------------------------
// 解析フィアルを作成する
void __fastcall TMainWnd::MakeANA(void)
{
	FILE *fp = fopen(m_AnaName, "wt");
	if( fp == NULL ) return;
	fprintf(fp, "\nAnalysis of %04u JASTA SSTV CONTEST\n\n", sys.m_Year);

	SDMMLOG sd;
	CMULT  MTMP;
	int i, n;
	for( i = 0; i < Log.GetCount(); i++ ){
		Log.GetData(&sd, i);
		n = atoin(sd.qsl, -1);
		if( n ){
			MTMP.Add(clipsp(sd.call));
		}
	}
	MTMP.SortCount();

	int b;
	int max = MTMP.GetCount();
	fprintf(fp, "[Number of QSOs according to a callsign (%u/%u)]\n", max, MTMP.GetTotal());
	int col, row;
	int base = 0;
	while(1){
		for( row = 0; row < 50; row++ ){
			fprintf(fp, "%-4u ", base + row + 1);
			for( col = 0; col < 4; col++ ){
				int n = base + (col * 50 + row);
				if( n < max ){
					fprintf(fp, "(%2u) %-13s", MTMP.GetCount(n), MTMP.GetText(n));
				}
				else {
					continue;
				}
			}
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n");
		base += 200;
		if( base >= max ) break;
	}

	int sum;
	CMULT   MDX[16];
	for( i = 0; i < Log.GetCount(); i++ ){
		Log.GetData(&sd, i);
		n = atoin(sd.qsl, -1);
		if( n && m_MJA.GetCount(sd.opt1) ){
			b = GetBNO(sd.band);
			MDX[b].Add(sd.opt1);
		}
	}
	for( b = 0; b < 11; b++ ){
		MDX[b].Set("all", MDX[b].GetTotal());
	}
	m_MJA.Sort();
	fprintf(fp, "[Number of QSOs according to a JA's area (%u/%u)]\n", m_MJA.GetCount(), m_MJA.GetTotal());
	fprintf(fp, "Area    3.5   7   14   18   21   24   28   50  144  430 1200 2400+ all\n");
	m_MJA.Add("all");
	for( row = 0; row < m_MJA.GetCount(); row++ ){
		LPCSTR pDX = m_MJA.GetText(row);
		fprintf(fp, "%-5s ", pDX);
		sum = 0;
		for( b = 0; b < 12; b++ ){
			n = MDX[b].GetCount(pDX);
			sum += n;
			fprintf(fp, n ? "%4u " : "   - ", n);
		}
		fprintf(fp, sum ? "%4u\n" : "   -\n", sum);
	}
	fprintf(fp, "\n");

	for( b = 0; b < 11; b++ ){
		MDX[b].Clear();
	}
	for( i = 0; i < Log.GetCount(); i++ ){
		Log.GetData(&sd, i);
		n = atoin(sd.qsl, -1);
		if( n && m_MDX.GetCount(sd.opt1) ){
			b = GetBNO(sd.band);
			MDX[b].Add(sd.opt1);
		}
	}
	for( b = 0; b < 11; b++ ){
		MDX[b].Set("all", MDX[b].GetTotal());
	}
	m_MDX.Sort();
	fprintf(fp, "[Number of QSOs according to a DX entity (%u/%u)]\n", m_MDX.GetCount(), m_MDX.GetTotal());
	fprintf(fp, "Entity  3.5   7   14   18   21   24   28   50  144  430 1200 2400+ all\n");
	m_MDX.Add("all");
	for( row = 0; row < m_MDX.GetCount(); row++ ){
		LPCSTR pDX = m_MDX.GetText(row);
		fprintf(fp, "%-5s ", pDX);
		sum = 0;
		for( b = 0; b < 12; b++ ){
			n = MDX[b].GetCount(pDX);
			sum += n;
			fprintf(fp, n ? "%4u " : "   - ", n);
		}
		fprintf(fp, sum ? "%4u\n" : "   -\n", sum);
	}
	fprintf(fp, "\n");

	for( b = 0; b < 11; b++ ) MDX[b].Clear();
	MTMP.Clear();
	for( i = 0; i < Log.GetCount(); i++ ){
		Log.GetData(&sd, i);
		n = atoin(sd.qsl, -1);
		if( n ){
			b = GetBNO(sd.band);
			MTMP.Add(sd.opt2);
			MDX[b].Add(sd.opt2);
		}

	}
	for( b = 0; b < 11; b++ ){
		MDX[b].Set("all", MDX[b].GetTotal());
	}
	MTMP.Sort();
	MTMP.Add("all");
	fprintf(fp, "[Number of QSOs according to a continent]\n");
	fprintf(fp, "Conti.  3.5   7   14   18   21   24   28   50  144  430 1200 2400+ all\n");
	for( row = 0; row < MTMP.GetCount(); row++ ){
		LPCSTR pDX = MTMP.GetText(row);
		fprintf(fp, "%-5s ", pDX);
		sum = 0;
		for( b = 0; b < 12; b++ ){
			n = MDX[b].GetCount(pDX);
			sum += n;
			fprintf(fp, n ? "%4u " : "   - ", n);
		}
		fprintf(fp, sum ? "%4u\n" : "   -\n", sum);
	}
	fprintf(fp, "\n");


	int QSODs[16][32];
	int QSOHs[16][25];
	memset(QSODs, 0, sizeof(QSODs));
	memset(QSOHs, 0, sizeof(QSOHs));
	for( i = 0; i < Log.GetCount(); i++ ){
		Log.GetData(&sd, i);
		n = atoin(sd.qsl, -1);
		if( n ){
			b = GetBNO(sd.band);
			n = sd.date % 100;
			if( (n >= 1) && (n <= 31) ){
				n--;
				QSODs[b][n]++;
			}
			n = (sd.btime / 30) / 60;
			if( (n >= 0) && (n <= 23) ){
				QSOHs[b][n]++;
			}
		}
	}
	fprintf(fp, "[Number of QSOs according to a day(UTC)]\n");
	fprintf(fp, "Day     3.5   7   14   18   21   24   28   50  144  430 1200 2400+ all\n");
	for( row = 0; row < 32; row++ ){
		fprintf(fp, row == 31 ? "all   " : "%02u    ", row + 1);
		int sum = 0;
		for( b = 0; b < 12; b++ ){
			n = QSODs[b][row];
			sum += n;
			fprintf(fp, n ? "%4u " : "   - ", n);
			QSODs[b][31] += n;
		}
		fprintf(fp, sum ? "%4u" : "   -", sum);
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fprintf(fp, "[Number of QSOs according to an hour(UTC)]\n");
	fprintf(fp, "Hour    3.5   7   14   18   21   24   28   50  144  430 1200 2400+ all\n");
	for( row = 0; row < 25; row++ ){
		fprintf(fp, row == 24 ? "all   " : "%02u    ", row);
		int sum = 0;
		for( b = 0; b < 12; b++ ){
			n = QSOHs[b][row];
			sum += n;
			fprintf(fp, n ? "%4u " : "   - ", n);
			QSOHs[b][24] += n;
		}
		fprintf(fp, sum ? "%4u" : "   -", sum);
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	memset(QSODs, 0, sizeof(QSODs));
	memset(QSOHs, 0, sizeof(QSOHs));
	for( i = 0; i < Log.GetCount(); i++ ){
		Log.GetData(&sd, i);
		n = atoin(sd.qsl, -1);
		if( n && !IsJA(sd.call) ){
			b = GetBNO(sd.band);
			n = sd.date % 100;
			if( (n >= 1) && (n <= 31) ){
				n--;
				QSODs[b][n]++;
			}
			n = (sd.btime / 30) / 60;
			if( (n >= 0) && (n <= 23) ){
				QSOHs[b][n]++;
			}
		}
	}
	fprintf(fp, "[Number of QSOs (DX) according to a day(UTC)]\n");
	fprintf(fp, "Day     3.5   7   14   18   21   24   28   50  144  430 1200 2400+ all\n");
	for( row = 0; row < 32; row++ ){
		fprintf(fp, row == 31 ? "all   " : "%02u    ", row + 1);
		int sum = 0;
		for( b = 0; b < 12; b++ ){
			n = QSODs[b][row];
			sum += n;
			fprintf(fp, n ? "%4u " : "   - ", n);
			QSODs[b][31] += n;
		}
		fprintf(fp, sum ? "%4u" : "   -", sum);
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");
	fprintf(fp, "[Number of QSOs (DX) according to an hour(UTC)]\n");
	fprintf(fp, "Hour    3.5   7   14   18   21   24   28   50  144  430 1200 2400+ all\n");
	for( row = 0; row < 25; row++ ){
		fprintf(fp, row == 24 ? "all   " : "%02u    ", row);
		int sum = 0;
		for( b = 0; b < 12; b++ ){
			n = QSOHs[b][row];
			sum += n;
			fprintf(fp, n ? "%4u " : "   - ", n);
			QSOHs[b][24] += n;
		}
		fprintf(fp, sum ? "%4u" : "   -", sum);
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	memset(QSODs, 0, sizeof(QSODs));
	memset(QSOHs, 0, sizeof(QSOHs));
	for( i = 0; i < Log.GetCount(); i++ ){
		Log.GetData(&sd, i);
		n = atoin(sd.qsl, -1);
		if( n && IsJA(sd.call) ){
			b = GetBNO(sd.band);
			n = sd.date % 100;
			if( (n >= 1) && (n <= 31) ){
				n--;
				QSODs[b][n]++;
			}
			n = (sd.btime / 30) / 60;
			if( (n >= 0) && (n <= 23) ){
				QSOHs[b][n]++;
			}
		}
	}
	fprintf(fp, "[Number of QSOs (JA) according to a day(UTC)]\n");
	fprintf(fp, "Day     3.5   7   14   18   21   24   28   50  144  430 1200 2400+ all\n");
	for( row = 0; row < 32; row++ ){
		fprintf(fp, row == 31 ? "all   " : "%02u    ", row + 1);
		int sum = 0;
		for( b = 0; b < 12; b++ ){
			n = QSODs[b][row];
			sum += n;
			fprintf(fp, n ? "%4u " : "   - ", n);
			QSODs[b][31] += n;
		}
		fprintf(fp, sum ? "%4u" : "   -", sum);
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fprintf(fp, "[Number of QSOs (JA) according to an hour(UTC)]\n");
	fprintf(fp, "Hour    3.5   7   14   18   21   24   28   50  144  430 1200 2400+ all\n");
	for( row = 0; row < 25; row++ ){
		fprintf(fp, row == 24 ? "all   " : "%02u    ", row);
		int sum = 0;
		for( b = 0; b < 12; b++ ){
			n = QSOHs[b][row];
			sum += n;
			fprintf(fp, n ? "%4u " : "   - ", n);
			QSOHs[b][24] += n;
		}
		fprintf(fp, sum ? "%4u" : "   -", sum);
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	memset(QSODs, 0, sizeof(QSODs));
	memset(QSOHs, 0, sizeof(QSOHs));
	CMULT MLTD, MLTH;
	for( i = 0; i < Log.GetCount(); i++ ){
		Log.GetData(&sd, i);
		n = atoin(sd.qsl, -1);
		if( n && sd.opt1[0] && !IsJA(sd.call) && (strchr(sd.opt1, '?')==NULL) ){
			b = GetBNO(sd.band);
			n = sd.date % 100;
			if( (n >= 1) && (n <= 31) ){
				n--;
				if( MLTD.Add(sd.opt1) ) QSODs[b][n]++;
			}
			n = (sd.btime / 30) / 60;
			if( (n >= 0) && (n <= 23) ){
				if( MLTH.Add(sd.opt1) ) QSOHs[b][n]++;
			}
		}
	}
	fprintf(fp, "[Number of NEW MULTIs (DX) according to a day(UTC)]\n");
	fprintf(fp, "Day     3.5   7   14   18   21   24   28   50  144  430 1200 2400+ all\n");
	for( row = 0; row < 32; row++ ){
		fprintf(fp, row == 31 ? "all   " : "%02u    ", row + 1);
		int sum = 0;
		for( b = 0; b < 12; b++ ){
			n = QSODs[b][row];
			sum += n;
			fprintf(fp, n ? "%4u " : "   - ", n);
			QSODs[b][31] += n;
		}
		fprintf(fp, sum ? "%4u" : "   -", sum);
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fprintf(fp, "[Number of NEW MULTIs (DX) according to an hour(UTC)]\n");
	fprintf(fp, "Hour    3.5   7   14   18   21   24   28   50  144  430 1200 2400+ all\n");
	for( row = 0; row < 25; row++ ){
		fprintf(fp, row == 24 ? "all   " : "%02u    ", row);
		int sum = 0;
		for( b = 0; b < 12; b++ ){
			n = QSOHs[b][row];
			sum += n;
			fprintf(fp, n ? "%4u " : "   - ", n);
			QSOHs[b][24] += n;
		}
		fprintf(fp, sum ? "%4u" : "   -", sum);
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	memset(QSODs, 0, sizeof(QSODs));
	memset(QSOHs, 0, sizeof(QSOHs));
	MLTD.Clear();
	MLTH.Clear();
	for( i = 0; i < Log.GetCount(); i++ ){
		Log.GetData(&sd, i);
		n = atoin(sd.qsl, -1);
		if( n && sd.opt1[0] && IsJA(sd.call) && (strchr(sd.opt1, '?')==NULL) ){
			b = GetBNO(sd.band);
			n = sd.date % 100;
			if( (n >= 1) && (n <= 31) ){
				n--;
				if( MLTD.Add(sd.opt1) ) QSODs[b][n]++;
			}
			n = (sd.btime / 30) / 60;
			if( (n >= 0) && (n <= 23) ){
				if( MLTH.Add(sd.opt1) ) QSOHs[b][n]++;
			}
		}
	}
	fprintf(fp, "[Number of NEW MULTIs (JA) according to a day(UTC)]\n");
	fprintf(fp, "Day     3.5   7   14   18   21   24   28   50  144  430 1200 2400+ all\n");
	for( row = 0; row < 32; row++ ){
		fprintf(fp, row == 31 ? "all   " : "%02u    ", row + 1);
		int sum = 0;
		for( b = 0; b < 12; b++ ){
			n = QSODs[b][row];
			sum += n;
			fprintf(fp, n ? "%4u " : "   - ", n);
			QSODs[b][31] += n;
		}
		fprintf(fp, sum ? "%4u" : "   -", sum);
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fprintf(fp, "[Number of NEW MULTIs (JA) according to an hour(UTC)]\n");
	fprintf(fp, "Hour    3.5   7   14   18   21   24   28   50  144  430 1200 2400+ all\n");
	for( row = 0; row < 25; row++ ){
		fprintf(fp, row == 24 ? "all   " : "%02u    ", row);
		int sum = 0;
		for( b = 0; b < 12; b++ ){
			n = QSOHs[b][row];
			sum += n;
			fprintf(fp, n ? "%4u " : "   - ", n);
			QSOHs[b][24] += n;
		}
		fprintf(fp, sum ? "%4u" : "   -", sum);
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fclose(fp);
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::KFSClick(TObject *Sender)
{
	int meng = MsgEng;
	TOptDlgBox *pBox = new TOptDlgBox(this);
	int r = pBox->Execute();
	delete pBox;
	if( meng != MsgEng ) SetLanguage();
	if( (r == TRUE) && Log.IsOpen() && !m_SrcName.IsEmpty() ){
		m_ReCalc = TRUE;
		if( YesNoMB(MsgEng ? "Does totalize once again?" : "集計しなおしますか ?") == IDYES ){
			Exec(m_SrcName.c_str(), FALSE);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::KVLClick(TObject *Sender)
{
	if( m_pEditLog != NULL ) delete m_pEditLog;
	if( m_LogName[0] ){
		m_pEditLog = new TFileEdit(this);
		m_pEditLog->Execute(m_LogName);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::KVSClick(TObject *Sender)
{
	if( m_pEditSum != NULL ) delete m_pEditSum;
	if( m_SumName[0] ){
		m_pEditSum = new TFileEdit(this);
		m_pEditSum->Execute(m_SumName);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::KVAClick(TObject *Sender)
{
	if( m_pEditAna != NULL ) delete m_pEditAna;
	if( m_AnaName[0] ){
		m_pEditAna = new TFileEdit(this);
		m_pEditAna->Execute(m_AnaName);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::KVClick(TObject *Sender)
{
	KVL->Enabled = m_LogName[0] && IsFile(m_LogName) ? TRUE : FALSE;
	KVS->Enabled = m_SumName[0] && IsFile(m_SumName) ? TRUE : FALSE;
	KVA->Enabled = m_AnaName[0] && IsFile(m_AnaName) ? TRUE : FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::MoveGrid(int n)
{
	Grid->Row = n + 1;
	if( (Grid->Row < Grid->TopRow) || (Grid->Row >= (Grid->TopRow+8)) ){
		Grid->TopRow = Grid->Row;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::MemoClick(TObject *Sender)
{
	m_GridMove = FALSE;
	if( Memo->Lines->Count ){
		int e = Memo->SelStart;
		LPCSTR p = AnsiString(Memo->Text).c_str();	//ja7ude 0525
		int n = 0;
		for( int i = 0; i < e; i++, p++ ){
			if( *p == LF ) n++;
		}
		if( n > Memo->Lines->Count ) n = Memo->Lines->Count - 1;
		char bf[256];
		StrCopy(bf, AnsiString(Memo->Lines->Strings[n]).c_str(), sizeof(bf)-1);
		if( isdigit(bf[0]) ){
			LPSTR t;
			StrDlm(t, bf, ' ');
			n = atoin(t, -1) - 1;
			if( (n >= 0) && (n < Log.GetCount()) ){
				MoveGrid(n);
				m_GridMove = TRUE;
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::MemoDblClick(TObject *Sender)
{
	if( m_GridMove ){
		GridDblClick(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::KHTClick(TObject *Sender)
{
	ShowHelp(this, MsgEng ? "EMMJASTA.TXT":"MMJASTA.TXT");
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::KHMClick(TObject *Sender)
{
	WebRef.ShowHTML(MsgEng ? "http://www.qsl.net/mmhamsoft/" : "http://plaza27.mbn.or.jp/~je3hht/mmsstv/index.html");
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::KHRClick(TObject *Sender)
{
	WebRef.ShowHTML(MsgEng ? "http://groups.yahoo.com/group/MM-SSTV/" : "http://www.egroups.co.jp/group/mmhamsoft/");
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::KHWClick(TObject *Sender)
{
	WebRef.ShowHTML("http://homepage3.nifty.com/jasta/");
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::GridDblClick(TObject *Sender)
{
	if( m_SrcName.IsEmpty() ) return;
	if( !Log.IsOpen() ) return;

	int n = Grid->Row - 1;
	if( (n >= 0) && (n < Log.GetCount()) ){
		TQSODlgBox *pBox = new TQSODlgBox(this);
		SDMMLOG	sd, rsd;
		Log.GetData(&sd, n);
		memcpy(&rsd, &sd, sizeof(sd));
		if( pBox->Execute(&sd) == TRUE ){
			DelLastSpace(sd.opt1);
			DelLastSpace(sd.opt2);
			if( strcmp(sd.opt1, rsd.opt1) || strcmp(sd.opt2, rsd.opt2) ){
				LPSTR p = lastp(sd.opt1);
                p+=2;
                *p = 0x01;
                p = lastp(sd.opt2);
                p+=2;
                *p = 0x01;

				CLogFile orgLog;
				orgLog.Open(m_SrcName.c_str(), TRUE);
				if( !orgLog.IsOpen() ) return;

				m_ReCalc = TRUE;
				int i;
				for( i = 0; i < Log.GetCount(); i++ ){
					Log.GetIndex()->ReadIndex(rsd.call, i);
					if( !strcmp(sd.call, rsd.call) ){
						Log.GetData(&rsd, i);
						memcpy(rsd.opt1, sd.opt1, MLOPT);
						memcpy(rsd.opt2, sd.opt2, MLOPT);
						Log.PutData(&rsd, i);
					}
				}

				for( i = 0; i < orgLog.GetCount(); i++ ){
					orgLog.GetIndex()->ReadIndex(rsd.call, i);
					if( !strcmp(sd.call, rsd.call) ){
						orgLog.GetData(&rsd, i);
						memcpy(rsd.opt1, sd.opt1, MLOPT);
						memcpy(rsd.opt2, sd.opt2, MLOPT);
						orgLog.PutData(&rsd, i);
					}
				}
				orgLog.Close();
			}
		}
		Grid->Invalidate();
		delete pBox;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::KFRClick(TObject *Sender)
{
	if( m_ReCalc ){
		Exec(m_SrcName.c_str(), FALSE);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::KFClick(TObject *Sender)
{
	KFR->Enabled = m_ReCalc && (!m_SrcName.IsEmpty());
}
//---------------------------------------------------------------------------
LPCSTR __fastcall TMainWnd::ConvADIF(LPCSTR pName, int loadorg)
{
	AnsiString  LogName = pName;

	char TmpName[256];
	sprintf(TmpName, "%s"TEMPLOGNAME, BgnDir);
	m_SrcName = TmpName;

	if( !loadorg ) return m_SrcName.c_str();

	CLogADIF adif;
	if( adif.Open(LogName.c_str()) == FALSE ) return NULL;

	CLogFile tmpLog;
	tmpLog.Open(TmpName, TRUE);
	if( !tmpLog.IsOpen() ) return NULL;
	tmpLog.DeleteAll();

	SDMMLOG	sd;
	memset(&sd, 0, sizeof(sd));
	CWaitCursor w;
	while(adif.Read(&sd)==TRUE){
		if( (sd.mode == 8) && (sd.band >= B_35) ){
			sd.opt1[0] = 0; sd.opt2[0] = 0;
			tmpLog.PutData(&sd, tmpLog.GetCount());
		}
		memset(&sd, 0, sizeof(sd));
	}
	adif.Close();
	tmpLog.Close();

	return m_SrcName.c_str();
}
//---------------------------------------------------------------------------
LPCSTR __fastcall TMainWnd::ConvLog200(LPCSTR pName, int loadorg)
{
	AnsiString  LogName = pName;

	char TmpName[256];
	sprintf(TmpName, "%s"TEMPLOGNAME, BgnDir);
	m_SrcName = TmpName;

	if( !loadorg ) return m_SrcName.c_str();

	CLog200 log;
	if( log.Open(LogName.c_str()) == FALSE ) return NULL;

	CLogFile tmpLog;
	tmpLog.Open(TmpName, TRUE);
	if( !tmpLog.IsOpen() ) return NULL;
	tmpLog.DeleteAll();

	SDMMLOG	sd;
	CWaitCursor w;
	while(log.Read(&sd)==TRUE){
		if( (sd.mode == 8) && (sd.band >= B_35) ){
			sd.opt1[0] = 0; sd.opt2[0] = 0;
			tmpLog.PutData(&sd, tmpLog.GetCount());
		}
	}
	log.Close();
	tmpLog.Close();

	return m_SrcName.c_str();
}
//---------------------------------------------------------------------------
LPCSTR __fastcall TMainWnd::ConvHAMLOG(LPCSTR pName, int loadorg)
{
	AnsiString  LogName = pName;

	char TmpName[256];
	sprintf(TmpName, "%s"TEMPLOGNAME, BgnDir);
	m_SrcName = TmpName;

	if( !loadorg ) return m_SrcName.c_str();

	CHamLog log;
	if( log.Open(LogName.c_str()) == FALSE ) return NULL;

	CLogFile tmpLog;
	tmpLog.Open(TmpName, TRUE);
	if( !tmpLog.IsOpen() ) return NULL;
	tmpLog.DeleteAll();

	SDMMLOG	sd;
	CWaitCursor w;
	while(log.Read(&sd)==TRUE){
		if( (sd.mode == 8) && (sd.band >= B_35) ){
			sd.opt1[0] = 0; sd.opt2[0] = 0;
			AdjustHamlogRSV(&sd);
			tmpLog.PutData(&sd, tmpLog.GetCount());
		}
	}
	log.Close();
	tmpLog.Close();

	return m_SrcName.c_str();
}
//---------------------------------------------------------------------------
void __fastcall TMainWnd::AdjustHamlogRSV(SDMMLOG *sp)
{
	char bf[MLREM+1];
	if( strlen(sp->ur) <= 3 ){
		NumCopy(bf, sp->rem);
		if( bf[0] ){
			int n = atoin(bf, -1);
			sprintf(bf, "%03u", n);
			strcat(sp->ur, bf);
		}
	}
	if( strlen(sp->my) <= 3 ){
		NumCopy(bf, sp->qsl);
		if( bf[0] ){
			int n = atoin(bf, -1);
			sprintf(bf, "%03u", n);
			strcat(sp->my, bf);
		}
	}
}
//---------------------------------------------------------------------------

