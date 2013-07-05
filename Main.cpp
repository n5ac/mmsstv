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

#include <SHELLAPI.H>
#include <io.h>
#include "Clipbrd.hpp"
#include "Main.h"
#include "Scope.h"
#include "PlayDlg.h"
#include "Option.h"
#include "LogFile.h"
#include "LogList.h"
#include "LogConv.h"
#include "LogLink.h"                       
#include "country.h"
#include "Qsodlg.h"
#include "Mmcg.h"
#include "VerDsp.h"
#include "LogSet.h"
#include "PicRect.h"
#include "ZoomView.h"
#include "TextEdit.h"
#include "ListText.h"
#include "PicFilte.h"
#include "ExtCmd.h"
#include "PerSpect.h"
#include "RepSet.h"
#include "MmcgDlg.h"
#include "radioset.h"
#include "RMenuDlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMmsstv *Mmsstv;
#define MEASCWSPEED	FALSE

#define FSKIDMAX    32
//ウインドウメッセージのハンドラ---------------------------------------------
void __fastcall TMmsstv::WndProc(TMessage &Message)
{
	switch(Message.Msg){
		case WM_COPYDATA:
			WndCopyData(Message);
			break;
		default:
			TForm::WndProc(Message);
			break;
	}
}
//---------------------------------------------------------------------------
// WM_COPYDATAの処理
void __fastcall TMmsstv::WndCopyData(TMessage &Message)
{
	COPYDATASTRUCT *cp = (COPYDATASTRUCT *)Message.LParam;
	if( LogLink.IsCopyData() ){
       	Message.Result = LogLink.m_pLink->OnCopyData(HWND(Message.WParam), cp);
		return;
	}
	switch(cp->dwData){
		case 0:
		case 1:             // Hamlogからの返信
			if( sys.m_LogLink != 1 ) return;
			switch(LogLink.AnaData(&Log.m_sd, cp)){
				case 115:
					UpdateTextData();
					break;
				case 106:
					LogFreq->Text = Log.GetFreqString(Log.m_sd.band, Log.m_sd.fq);
					break;
			}
			Message.Result = TRUE;
			break;
		case 0x80001212:    // 周波数データの指定
			if( cp->cbData && (cp->lpData != NULL) ){
				char bf[16];
				int len = cp->cbData;
				if( len > 15 ) len = 15;
				memcpy(bf, cp->lpData, len);
				bf[len] = 0;
				LogFreq->Text = bf;
			}
			Message.Result = TRUE;
			break;
		case 0x80001213:    // クリップボードからのロード
			KESClick(NULL);
			Message.Result = TRUE;
			break;
		case 0x80001214:    // 画像サイズの問い合わせ
			Message.Result = m_TXW + (m_TXH << 16);
			break;
		case 0x80001215:    // 履歴画像のコピー
			SBCopyClick(NULL);
			Message.Result = TRUE;
			break;
		case 0x80001216:    // 受信画像のコピー
			KERClick(NULL);
			Message.Result = TRUE;
			break;
		case 0x80001217:    // 送信/受信の切り替え
			if( cp->cbData && (cp->lpData != NULL) ){
				if( *(const BYTE *)(cp->lpData) ){
					if( !SBTX->Down ){
						AdjustPage(pgTX);
						ToTX();
					}
				}
				else if( SBTX->Down ){
					ToRX();
				}
			}
			Message.Result = TRUE;
			break;
		case 0x80001218:
			m_ChangeTemp = 1;
			SBULog->Enabled = TRUE;
			UpdateUI();
			Message.Result = TRUE;
			break;
		case 0x80001219:    // REMデータの指定
			if( cp->cbData && (cp->lpData != NULL) ){
				EditNote->Text = (LPCSTR)cp->lpData;
				EditNoteChange(NULL);
			}
			Message.Result = TRUE;
			break;
        case 0x8000121a:	// PTT
			if( cp->cbData && (cp->lpData != NULL) ){
				BOOL bPTT = *(const BYTE *)(cp->lpData);
				if( pRadio != NULL ) pRadio->SetPTT(bPTT);
				if( pComm != NULL ) pComm->SetPTT(bPTT);
            }
        	break;
        case 0x8000121b:	// status
			{
				DWORD dw = SBTX->Down ? 1 : 0;
                dw |= SBTune->Down ? 2 : 0;
				dw |= pDem->m_Sync ? 4 : 0;
				Message.Result = dw;
            }
        	break;
		default:
			Message.Result = FALSE;
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OnMSG(tagMSG &Msg, bool &Handled)
{
/*
Msg.hwnd := Handle;
Msg.message := WM_KEYDOWN;
Msg.wParam := Message.WParam;
Msg.lParam := Message.LParam;
*/
	if( Msg.message != WM_DROPFILES ) return;

	DropFile(Msg);
	Handled = true;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OnMini(TObject *Sender)
{
	if( pSound != NULL ){
		if( pSound->m_susp ){
			pSound->m_susp = 1;
		}
	}
}
//---------------------------------------------------------------------------
// アプリケーション例外
void __fastcall TMmsstv::AppException(TObject *Sender, Exception *E)
{
	if( m_AppErr < 3 ){
		m_AppErr++;
		ErrorMB(MsgEng?
			"The indistinct error was detected.":
			"原因不明のエラーが発生しました."
		);
	}
	ResDisPaint();
}
//---------------------------------------------------------------------------
__fastcall TMmsstv::TMmsstv(TComponent* Owner)
	: TForm(Owner)
{
	::VirtualLock(this, sizeof(TMmsstv));
	::VirtualLock(&sys, sizeof(sys));

	m_AppErr = 0;
	m_InitFirst = TRUE;
	m_DisEvent = 1;
	SampFreq = 11025.0;
	pDraw = NULL;
	pPaste = NULL;
	pRxView = NULL;
	pSyncView = NULL;
	pHistView = NULL;
	pPreView = NULL;
	pCalibration = NULL;
	pUndo = NULL;
	pCtrBtn = NULL;
	pStockView = NULL;
	pWaterView = NULL;
	pLogView = NULL;

	int i;
	for( i = 0; i < 8; i++ ){
		pFileView[i] = NULL;
	}
	m_TimerRXV = 0;
	m_TimerRXS = 0;
	m_FileViewClose = 0;
	m_ClientHeight[0] = m_ClientHeight[1] = 0;
	m_ClientWidth[0] = m_ClientWidth[1] = 0;
	m_SpecDown = 0;

	m_dwVersion = ::GetVersion();
	if( m_dwVersion < 0x80000000 ){
		WinNT = TRUE;			// NT,2000,XP,Vista

        OSVERSIONINFO osvi;

        ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        GetVersionEx(&osvi);
        if (osvi.dwMajorVersion == 5) {
            WinVista=FALSE;
        }
        else {
            WinVista=TRUE;
        }
	}
	else {		// win95/98/ME
		WinNT = FALSE;
        WinVista=FALSE;
	}

#if 0
// Get major and minor version numbers of Windows
dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
// Get build numbers for Windows NT or Win32s
if (dwVersion < 0x80000000)                // Windows NT
	dwBuild = (DWORD)(HIWORD(dwVersion));
else if (dwWindowsMajorVersion < 4)        // Win32s
	dwBuild = (DWORD)(HIWORD(dwVersion) & ~0x8000);
else         // Windows 95 -- No build numbers provided
	dwBuild =  0;
#endif

	InitFFT();

	EntryAlign();
	lcid = GetThreadLocale() & 0x00ff;
	if( lcid != LANG_JAPANESE ){                    // English
		sys.m_WinFontName = "Times New Roman";
		sys.m_WinFontCharset = ANSI_CHARSET;
		Log.m_LogSet.m_TimeZone = 'Z';
		sys.m_LogLink = 0;
		MsgEng = 1;
	}
	else {                                          // Japanese
		sys.m_WinFontName = "ＭＳ Ｐゴシック";
		sys.m_WinFontCharset = SHIFTJIS_CHARSET;
		sys.m_LogLink = 1;
		MsgEng = 0;
	}
	SetMBCP();
	sys.m_WinFontStyle = 0;
	LogLink.SetHandle(Handle, CM_CMML);

	sys.m_BitPixel = ::GetDeviceCaps(Canvas->Handle, BITSPIXEL);
	sys.m_Palette = sys.m_BitPixel < 16 ? 1 : 0;
	sys.m_DivMode = 0;
	sys.m_UseB24 = 0;
	sys.m_DisFontSmooth = WinNT;
	sys.m_TempDelay = 3;

	pBitmapNearest = NULL;
	if( sys.m_BitPixel < 24 ){
		pBitmapNearest = new Graphics::TBitmap;
        pBitmapNearest->Width = 1;
        pBitmapNearest->Height = 1;
    }

	pBitmapFFT = new Graphics::TBitmap();
	pBitmapFFT->Width = PBoxFFT->Width;
	pBitmapFFT->Height = PBoxFFT->Height;

	pBitmapWater = new Graphics::TBitmap();
	pBitmapWater->PixelFormat = pf24bit;
	pBitmapWater->Width = PBoxWater->Width;
	pBitmapWater->Height = PBoxWater->Height;

	PBoxRX->Width = 320;
	PBoxRX->Height = 256;
	PBoxHist->Width = 320;
	PBoxHist->Height = 256;
	PBoxTX->Width = 320;
	PBoxTX->Height = 256;
	PBoxTemp->Width = 320;
	PBoxTemp->Height = 256;

	pBitmapSS = NULL;
	for( i = 0; i < STOCKPAGE; i++ ){
		pBitmapST[i] = NULL;
	}

	m_TXW = m_RXW = 320; m_TXH = m_TXPH = m_RXH = m_RXPH = 256;
	m_SMax = 6;

	pBitmapD12 = CreateBitmap(PBoxD12->Width, PBoxD12->Height, pf24bit);

	pBitmapLvl = new Graphics::TBitmap();
	pBitmapLvl->Width = PBoxLvl->Width;
	pBitmapLvl->Height = PBoxLvl->Height;

	SBDPaste->Glyph->Assign(SBPaste->Glyph);
	SBRView->Glyph->Assign(SBHView->Glyph);
	SBTView->Glyph->Assign(SBHView->Glyph);
	SBTXFil->Glyph->Assign(SBHistFil->Glyph);
	SBRXFil->Glyph->Assign(SBHistFil->Glyph);
	m_Slant = 0;
//	m_Timer1S = 0;
	m_DSEL = 0;
	m_Dupe = 0;
	m_DrawTrans = 0;
//    m_DrawCmd = -1;
	m_DrawCmd = CM_SELECT;
	SBDSel->Down = TRUE;
	m_DragNo = 0;
	m_DragDataNo = 0;
	m_DragPMax = 50;
	m_TxSPage = -1;
	m_PSPage = m_APSPage = 0;
	for( i = 0; i < STOCKPAGE; i++ ){
		m_TSPage[i] = m_ATSPage[i] = 0;
	}
	m_ReqSampChg = 0;
	m_TuneTimer = 0;
	m_pRowBuf = NULL;
	m_ReqHistF = 0;
	m_SyncAccuracy = 1;

	m_TempTimer = 0;
	m_TempHold = 0;
	m_SuspMinimized = 0;

	InitProfile();

	for( i = 0; i < 9; i++ ){
		m_ModeAssignTX[i] = m_ModeAssignRX[i] = i;
	}
	for( i = 0; i < 8; i++ ){
		sys.m_ExtMode[i] = 0;
		sys.m_ExtName[i] = "";
		sys.m_ExtCmd[i] = "";
	}
	char bf[256];
	::GetWindowsDirectory(bf, sizeof(bf));
	char drv = bf[0];
	sprintf(bf, "%c:\\Program Files\\SSTV-PAL\\SSTV-Pal.exe", drv);
	sys.m_ExtName[0] = "SSTV-PAL";
	sys.m_ExtCmd[0] = bf;
	sys.m_ExtMode[0] = 1;

	sprintf(bf, "%c:\\Program Files\\MMTTY\\MMTTY.EXE", drv);
	if( !IsFile(bf) ){
		sprintf(bf, "%c:\\MMTTY\\MMTTY.EXE", drv);
	}
	sys.m_ExtName[7] = "MMTTY";
	sys.m_ExtCmd[7] = bf;
	sys.m_ExtMode[7] = 2;

	pComm = NULL;
	pRadio = NULL;

#if 1
	StartOption();
#else
	if( ParamCount() >= 0 ){
		SetDirName(BgnDir, ParamStr(0).c_str());
	}
	else {
		SetCurDir(BgnDir, sizeof(BgnDir));
	}
	strcpy(BitmapDir, BgnDir);
	strcpy(SBitmapDir, BgnDir);
	strcpy(TemplateDir, BgnDir);
	strcpy(RecDir, BgnDir);
	strcpy(ExtLogDir, BgnDir);
	sprintf(HistDir, "%sHistory\\", BgnDir);
	sprintf(StockDir, "%sStock\\", BgnDir);
#endif

	Caption = VERTTL2;
	InitSampType();
	InitCOMMPara();
	InitRADIOPara();
	sys.m_Priority = 0;
	sys.m_SoundFifoRX = 12;
	sys.m_SoundFifoTX = 8;
	sys.m_SoundPriority = 1;
	sys.m_SoundDevice = "-1";
	sys.m_SoundStereo = 0;
    sys.m_StereoTX = 0;
	sys.m_FFTType = 2;
	sys.m_FFTGain = 5;
	sys.m_FFTResp = 0;
	sys.m_FFTStg = 1;
	sys.m_FFTAGC = 1;
	sys.m_FFTWidth = 1;
	sys.m_FFTPriority = 0;
	sys.m_Call = "NOCALL";
	sys.m_TxRxName = "NONE";
	sys.m_TxRxLock = 1;
	sys.m_RTSonRX = 0;

	sys.m_ColorLow = clBlack;
	sys.m_ColorHigh = clWhite;
	sys.m_ColorFFTB = TColor(4227327);
	sys.m_ColorFFT = clYellow;
	sys.m_ColorFFTStg = clBlue;
	sys.m_ColorFFTSync = clLime;
	sys.m_ColorFFTFreq = clYellow;
	sys.m_LogName = "Temp.mdt";
	sys.m_WindowState = wsNormal;
	sys.m_TestDem = 0;
	sys.m_TuneTXTime = -1;
	sys.m_TuneSat = 0;
	sys.m_ColorRXB = clWhite;

	sys.m_HDDSize = 2;

	sys.m_VOX = 0;
//    sys.m_VOXSound = "1100,300,1500,100,1900,200,1500,200";
	sys.m_VOXSound = "1500,100,1700,100,2300,100,2100,100,1900,100,1500,100";

	sys.m_Sharp2D = 0;
	sys.m_Differentiator = 0;
	sys.m_DiffLevelP = 1.0;
	sys.m_DiffLevelM = sys.m_DiffLevelP / 3;

	sys.m_ColText[0] = TColor(255);
	sys.m_ColText[1] = TColor(16711935);
	sys.m_ColText[2] = TColor(65535);
	sys.m_ColText[3] = TColor(16776960);
	sys.m_ColText[4] = clBlack;
	sys.m_ColText[5] = clBlack;

	sys.m_TextGrade = 0;
	sys.m_TextShadow = 2;
	sys.m_TextZero = 0;
	sys.m_TextRot = 0;
	sys.m_TextVert = 0;
	sys.m_TextVertH = -6;
	sys.m_TextStack = 0x0000fc04;

	sys.m_PicShape = 0;
	sys.m_PicAdjust = 0;
	sys.m_PicLineStyle = 5;
	sys.m_PicLineColor = clWhite;

	sys.m_ColBar[0] = clBlack;
	sys.m_ColBar[1] = TColor(0x00f0f0f0);
	sys.m_ColBar[2] = clRed;
	sys.m_ColBar[3] = clGreen;

	sys.m_PicSelCurCol = clWhite;
	sys.m_PicSelSmooz = 0;
	sys.m_PicSelRTM = 1;

	const DWORD _tbl[]={
		16776960, 65535, 16711935, 255,
		255, 16711935, 65535, 16776960,
		12615935, 8454143, 16776960, 65280,
		16776960, 4259584, 65535, 16711935,
		255, 16711935, 65535, 65280,
		255, 4227327, 65535, 16777088,
		16776960, 65535, 4227327, 255,
		65280, 65535, 33023, 255,
	};

	for( i = 0; i < 32; i++ ){
		sys.m_ColorSet[i] = TColor(_tbl[i]);
	}

	sys.m_DemOff = 0;
	sys.m_DemWhite = 128.0/16384.0;
	sys.m_DemBlack = 128.0/16384.0;
	sys.m_DemCalibration = 0;

	for( i = 0; i < 17; i++ ){
		sys.m_Dem17[i] = m_DemPro[8].Dem17[i];
	}

	sys.m_ShowSizeRX = 0;
	sys.m_ShowSizeHist = 0;
	sys.m_ShowSizeTX = 0;
	sys.m_ShowSizeTemp = 0;
	sys.m_ShowSizeStock = 0;

	sys.m_PicClipType = 0;
	sys.m_PicClipRatio = 1;
	sys.m_PicClipMode = 0;
	sys.m_PicClipView = 1;
	sys.m_PicClipColor = clBlack;

	sys.m_Way240 = 1;

	sys.m_HistMax = 32;
	sys.m_UseRxBuff = 1;
	sys.m_AutoStop = 0;
	sys.m_AutoSync = 1;
	sys.m_CWID = 0;
	sys.m_TXFSKID = 0;
	sys.m_CWIDSpeed = 10;
    sys.m_CWIDWPM = 28;
	sys.m_CWIDText = "DE %m";
	sys.m_CWIDFreq = 1000;
	sys.m_MMVID = "";
	sys.m_UseJPEG = 0;
	sys.m_JPEGQuality = 80;
	sys.m_CWText = "%m";

	sys.m_nCWMenu = 7;
	sys.m_CWMenu[0] = "QSL 73 TU";
    sys.m_CWMenu[1] = "NR? AGN";
    sys.m_CWMenu[2] = "QRZ?";
    sys.m_CWMenu[3] = "UR %N %N";
    sys.m_CWMenu[4] = "%c de %m";
	sys.m_CWMenu[5] = "73 TU";
	sys.m_CWMenu[6] = "VVV";

	sys.m_FixedTxMode = 1;

	sys.m_TextList[0] = "CQ SSTV";
	sys.m_TextList[1] = "%c";
	sys.m_TextList[2] = "ur %r";
	sys.m_TextList[3] = "ur %R-%N";
	sys.m_TextList[4] = "73";
	sys.m_TextList[5] = "de %m";
	sys.m_TextList[6] = "%D %tZ";
	sys.m_TextList[7] = "%L %u Local";

	for( i = 0; i < 4; i++ ) sys.m_RegFont[i].m_Height = 0;

    sys.m_Temp24 = TRUE;

	sys.m_MaskUserPage = 0;
	sys.m_MaskCol1 = clYellow;
	sys.m_MaskCol2 = clBlack;

	m_nRadioMenu = 5;
	m_RadioMenu[0].strTTL = "7.033 LSB (FT847)";
    m_RadioMenu[0].strCMD = "\\$0070330001\\w10\\$0000000007\\w10";
	m_RadioMenu[1].strTTL = "14.230 USB (FT847)";
    m_RadioMenu[1].strCMD = "\\$0142300001\\w10\\$0100000007\\w10";
	m_RadioMenu[2].strTTL = "21.340 USB (FT847)";
    m_RadioMenu[2].strCMD = "\\$0213400001\\w10\\$0100000007\\w10";
	m_RadioMenu[3].strTTL = "28.680 USB (FT847)";
    m_RadioMenu[3].strCMD = "\\$0286800001\\w10\\$0100000007\\w10";
	m_RadioMenu[4].strTTL = "144.450 USB (FT847)";
    m_RadioMenu[4].strCMD = "\\$1444500001\\w10\\$0100000007\\w10";

	ReadSampFreq();
	pSound = NULL;
	pSound = new TSound(TRUE);
	pDem = &pSound->SSTVDEM;
	pMod = &pSound->SSTVMOD;

	pMod->m_vco.SetSampleFreq(sys.m_SampFreq + sys.m_TxSampOff);
	pSound->fftIN.m_Handle = Handle;
	ReadRegister();

	pBitmapRX = CreateBitmap(PBoxRX->Width, PBoxRX->Height, pf24bit);

	pBitmapHist = CreateBitmap(PBoxHist->Width, PBoxHist->Height, -1);
	pBitmapHistF = pBitmapHist;
	pBitmapTemp = CreateBitmap(PBoxTemp->Width, PBoxTemp->Height, -1);
	pBitmapTX = CreateBitmap(PBoxTX->Width, PBoxTX->Height, -1);
	pBitmapTXM = CreateBitmap(PBoxTX->Width, PBoxTX->Height, -1);

	PanelFFT->Color = sys.m_ColorFFTB;
	PanelWater->Color = sys.m_ColorLow;
	FillBitmap(pBitmapRX, sys.m_ColorRXB);
	FillBitmap(pBitmapHist, sys.m_ColorRXB);

	sprintf(bf, "%s"HISTNAME, HistDir);
	if( !IsFile(bf) ){
		strcpy(bf, HistDir);
		if( *lastp(bf) == '\\' ) *lastp(bf) = 0;
		mkdir(bf);
	}
	sprintf(bf, "%sCurrent.bmp", StockDir);
	if( !IsFile(bf) ){
		strcpy(bf, StockDir);
		if( *lastp(bf) == '\\' ) *lastp(bf) = 0;
		mkdir(bf);
	}

	UpdateSystemFont();

	InitColorTable(sys.m_ColorLow, sys.m_ColorHigh);
	DrawFFT(1);
	if( KVW->Checked ) DrawWater(1);
	UpdateRecentMenu();
	UpdateModeBtn();
	DispSyncStat();
	RxHist.Open();
	UpdateHist();
	UpdateTxClip();
	Application->OnMessage = OnMSG;
	Application->OnMinimize = OnMini;
#if !DEBUG
	Application->OnException = AppException;
#endif
	m_DisEvent = 0;
	if( int(WindowState) != sys.m_WindowState ) WindowState = TWindowState(sys.m_WindowState);
	pSound->fftIN.m_Handle = Handle;
	SBTX->Font->Color = clRed;

	sprintf(bf, "%sMmsstv.chm", BgnDir);
	if( IsFile(bf) ){
		JanHelp = "MMSSTV.CHM";
    }
    else {
		sprintf(bf, "%sMmsstv.hlp", BgnDir);
		if( IsFile(bf) ) JanHelp = "MMSSTV.HLP";
    }
    if( !JanHelp.IsEmpty() ){
		TMenuItem *pm = new TMenuItem (this);
        sprintf(bf, "MMSSTV &Help (%s)", JanHelp.c_str());
		pm->Caption = bf;
		pm->OnClick = KHlpMainClick;
		KHelp->Insert(0, pm);
	}
	UpdateProFileMenu();
	::DragAcceptFiles(PanelTX->Handle, TRUE);
	::DragAcceptFiles(PanelTemp->Handle, TRUE);
    m_hClipNext = ::SetClipboardViewer(Handle);
	MakeCalibrationTable();
	m_FFTWINDOW = (3010 * FFT_SIZE / FFTSamp);

	m_OrgTop = Top;
	m_OrgLeft = Left;

	DivControl(sys.m_DivMode);

	m_RepBeaconCount = 0;
	m_RepBeaconPos = m_RepTXPos = 0;
	if( sys.m_Repeater ) SetBeaconTick();
	if( sys.m_Priority ) UpdatePriority();

	LoadCurrentTemp();
	ChangeTxMode(SSTVSET.m_TxMode);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::StartOption(void)
{
	AnsiString as;
	if( ParamCount() >= 0 ){
		as = ParamStr(0).c_str();
		SetDirName(BgnDir, as.c_str());
	}
	else {
		SetCurDir(BgnDir, sizeof(BgnDir));
	}
	strcpy(BitmapDir, BgnDir);
	strcpy(SBitmapDir, BgnDir);
	strcpy(TemplateDir, BgnDir);
    strcpy(MMLogDir, BgnDir);
	strcpy(RecDir, BgnDir);
	strcpy(ExtLogDir, BgnDir);
	sprintf(HistDir, "%sHistory\\", BgnDir);
	sprintf(StockDir, "%sStock\\", BgnDir);

    sys.m_bCQ100 = FALSE;
	g_dblToneOffset = 0.0;

	sys.m_Repeater = 0;
	for( int i = 0; i <= ParamCount(); i++ ){
		as = ParamStr(i);
		if( as == "-r" ){
			sys.m_Repeater = 1;
		}
        else if( as == "-i" ){
			sys.m_bCQ100 = TRUE;
            g_dblToneOffset = -1000.0;
        }
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	Application->OnException = NULL;
	::DragAcceptFiles(PanelS->Handle, FALSE);	//ja7ude 0521
	::DragAcceptFiles(PanelTX->Handle, FALSE);	//ja7ude 0521
	::ChangeClipboardChain(Handle, m_hClipNext);

	if( SBTX->Down ) ToRX();
	Timer->Enabled = FALSE;
	Application->OnIdle = NULL;
	if( pRadio != NULL ){
		pRadio->ReqClose();
	}

	int DivMode = sys.m_DivMode;
	KVSDClick(NULL);
	DivControl(0);
	sys.m_DivMode = DivMode;

	KVR->Checked = pRxView != NULL;
	if( pRxView != NULL ){
		pRxView->GetViewPos(sys.m_RxViewLeft, sys.m_RxViewTop, sys.m_RxViewWidth, sys.m_RxViewHeight);
		delete pRxView;
		pRxView = NULL;
	}
	KVS->Checked = pSyncView != NULL;
	if( pSyncView != NULL ){
		pSyncView->GetViewPos(sys.m_SyncViewLeft, sys.m_SyncViewTop, sys.m_SyncViewWidth, sys.m_SyncViewHeight);
		delete pSyncView;
		pSyncView = NULL;
	}
	KVH->Checked = pHistView != NULL;
	if( pHistView != NULL ){
		CloseHistView();
	}
	KVC->Checked = pCtrBtn != NULL;
	if( pCtrBtn != NULL ){
		if( pCtrBtn->SBLock->Down ){
			pCtrBtn->SBLock->Down = FALSE;
			pCtrBtn->SBLockClick(NULL);
		}
		pCtrBtn->GetViewPos(sys.m_CtrBtnLeft, sys.m_CtrBtnTop, sys.m_CtrBtnWidth, sys.m_CtrBtnHeight);
		delete pCtrBtn;
		pCtrBtn = NULL;
	}
	int i;
	for( i = 0; i < 8; i++ ){
		if( pFileView[i] != NULL ){
			int Flag = (pFileView[i]->Visible || pFileView[i]->m_Suspend) ? pFileView[i]->m_TitleBar ? 1 : 2 : 0;
			CloseFileView(i);
			sys.m_FileViewFlag[i] = Flag;
		}
		else {
			sys.m_FileViewFlag[i] = 0;
		}
	}
	if( pSound != NULL ){
		pSound->ReqStop();
		Log.Close();
		WriteRegister();
		WaveStg.Close();
		Log.DoBackup();
	}
	DeleteUndo();
	DrawMain.FreeItem();
	DrawText.FreeItem();
	DrawPara.FreeItem();
	DrawTemp.FreeItem();

	delete pBitmapFFT;
	pBitmapFFT = NULL;

	if( pBitmapWater != NULL ){
		delete pBitmapWater;
		pBitmapWater = NULL;
	}

	if( pBitmapSS != NULL ){
		delete pBitmapSS;
		pBitmapSS = NULL;
	}
	for( i = 0; i < STOCKPAGE; i++ ){
		if( pBitmapST[i] != NULL ){
			delete pBitmapST[i];
			pBitmapST[i] = NULL;
		}
	}

	delete pBitmapRX;
	pBitmapRX = NULL;

	if( (pBitmapHistF != NULL) && (pBitmapHistF != pBitmapHist) ){
		delete pBitmapHistF;
		pBitmapHistF = NULL;
	}
	delete pBitmapHist;
	pBitmapHist = NULL;

	delete pBitmapTX;
	pBitmapTX = NULL;
	delete pBitmapTXM;
	pBitmapTXM = NULL;
	delete pBitmapTemp;
	pBitmapTemp = NULL;

	delete pBitmapD12;
	pBitmapD12 = NULL;

	delete pBitmapLvl;
	pBitmapLvl = NULL;

	if( pBitmapNearest != NULL ) delete pBitmapNearest;

	if( pPaste != NULL ){
		delete pPaste;
		pPaste = NULL;
	}
	if( pComm != NULL ){
		pComm->SetScan(0);
		pComm->Close();
		delete pComm;
		pComm = NULL;
	}
	if( pCalibration != NULL ){
		delete pCalibration;
		pCalibration = NULL;
	}
	::VirtualUnlock(&sys, sizeof(sys));
	::VirtualUnlock(this, sizeof(TMmsstv));
	CanClose = TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::FormDestroy(TObject *Sender)
{
	if( pSound != NULL ){
		pSound->WaitStop();
		delete pSound;
		pSound = NULL;
	}
	if( pRadio != NULL ){
		pRadio->WaitClose();
		delete pRadio;
		pRadio = NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateToneFreq(void)
{
	int fq = pMod->m_TuneFreq;
	char bf[128];
	sprintf(bf, MsgEng ? "Send %uHz tone" : "%uHzﾄｰﾝの送信", fq);
	SBTune->Hint = bf;
	SBTune->Caption = fq;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateSystemFont(void)
{
	TFontStyles fsw = Code2FontStyle(sys.m_WinFontStyle);
	if( (sys.m_WinFontName != Font->Name)||
		(sys.m_WinFontCharset != Font->Charset)||
		(fsw != Font->Style)
	){
		Font->Name = sys.m_WinFontName;
		Font->Charset = sys.m_WinFontCharset;
		Font->Style = fsw;
		GBMode->Font->Name = sys.m_WinFontName;
		GBMode->Font->Charset = sys.m_WinFontCharset;
		GBMode->Font->Style = fsw;
		GB1->Font->Name = sys.m_WinFontName;
		GB1->Font->Charset = sys.m_WinFontCharset;
		GB1->Font->Style = fsw;
		GBLog->Font->Name = sys.m_WinFontName;
		GBLog->Font->Charset = sys.m_WinFontCharset;
		GBLog->Font->Style = fsw;
		AlignPCS.NewFont(sys.m_WinFontName, sys.m_WinFontCharset, fsw);
		SBTX->Font->Name = sys.m_WinFontName;
		SBTX->Font->Charset = sys.m_WinFontCharset;
		SBTX->Font->Style = fsw;
	}
	if( Font->Charset != SHIFTJIS_CHARSET ){
		MsgEng = 1;
		KFile->Caption = "&File";
		KView->Caption = "&View";
		KOpt->Caption = "&Option";
//		KExtCmd->Caption = "Program(&P)";
		KHelp->Caption = "&Help";
// File Nenu for english
		KLogOpen->Caption = "Open &Log file...";
		KLogFlush->Caption = "Save data now(&F)";
		KFRecTim->Caption = "Record sound to the file with a time stamp(&A)";
		KFRec->Caption = "Record sound to the file(&W)...";
		KFPlay->Caption = "&Play sound from the file...";
		KFPlayPos->Caption = "Play positsion";
		KFRecRew->Caption = "Rewind";
		KFRecStop->Caption = "Close Record / Play";
		KExit->Caption = "Exit MMSSTV(&X)";

		KEdit->Caption = "&Edit";
		KEC->Caption = "&Copy from history window";
		KEE->Caption = "&Edit TX image...";
		KER->Caption = "Copy from &RX window";
		KEA->Caption = "&Auto copy";
		KEP->Caption = "&Paste to TX window...";
		KES->Caption = "&Stretch paste to TX window";
		KEX->Caption = "Paste via Clipper...";
		KEW->Caption = "Handling of 320x240";
		KEW1->Caption = "Vertical stretching";
		KEW2->Caption = "Auto stretching";
		KEW3->Caption = "No stretching";
		KEW4->Caption = "Shift picture for the header";

		KVR->Caption = "Always show RX viewer";
		KVS->Caption = "Always show Sync viewer";
		KVH->Caption = "Always show thumbnails of history";
		KVF->Caption = "Always show thumbnails of files folder";
		KVC->Caption = "Always show control buttons";
		KVSD->Caption = "Design";
		KVSD1->Caption = "Standard";
		KVSD2->Caption = "Separate stock view";
		KVSD3->Caption = "Separate all views";
		KRCD->Caption = "Child of a desktop window";
		KVL->Caption = "Level indicator";
		KVLSG->Caption = "Receipt signals";
		KVLSY->Caption = "Sync signals";
		KVSE1->Caption = "Stock view";
		KVSE2->Caption = "Tuning view";
		KVSE3->Caption = "Log view";

		KS->Caption = "Spectral display";
		KSFQ->Caption = "FM demodulator";
		KFFTW->Caption = "Spectral display range";
		KFFTS->Caption = "Spectral trail";
		KFSQ->Caption = "Quick";
		KFSS->Caption = "Short";
		KFSL->Caption = "Long";
		KF->Caption = "Spectral sensitivity";
		KR->Caption = "Spectral response";
		KRS->Caption = "Slow";
		KRM->Caption = "Medium";
		KRF->Caption = "Fast";
		KFTD->Caption = "Calculation priority";
		KFTD1->Caption = "Lower";
		KFTD2->Caption = "Normal";
		KFTD3->Caption = "A little higher";
		KFTD4->Caption = "Higher";
		KFTD5->Caption = "Highest";
		KOSC->Caption = "&Oscilloscope...";
		KVOut->Caption = "Soundcard output level(&V)...";
		KVIn->Caption = "Soundcard &Input level...";
		KORep->Caption = "Setup repeater...";
		KOL->Caption = "Setup &Logging...";
		KOM->Caption = "Setup MMSSTV(&O)...";

		KP->Caption = "P&Rofiles";
		KPDef->Caption = "MMSSTV Default";
		KPInit->Caption = "Initialize all menus as MMSSTV default";
		KPA->Caption = "Assign profile";
		KPD->Caption = "Delete profile";

		KX->Caption = "&Program";
		KXP->Caption = "MS &Paint";
		KXA->Caption = "Assign program";
		KXD->Caption = "Delete program";
		KXR->Caption = "Restore";
		KXS->Caption = "&Suspend";
		KXSM->Caption = "Suspend+&Minimize";

		KRadio->Caption = "Radio&Command";
		KRadioS->Caption = "Edit menu";
		KRLoad->Caption = "&Load...";
		KRSet->Caption = "&Setup...";
		KRadioAdd->Caption = "&Add menu...";

		KHlpUp->Caption = "Version-up information...";
		KHlpPad->Caption = "Use NotePad";
		KHJ->Caption = "JASTA Activity Contest (August)";
        KHN->Caption = "NVCG Contest (July)";
		KVer->Caption = "Version information(&A)...";

		TabSync->Caption = "Sync";
		TabRX->Caption = "RX";
		TabHist->Caption = "History";
		TabTX->Caption = "TX";
		TabTemp->Caption = "Template";
		SBPhase->Caption = "Phase";
		SBSlant->Caption = "Slant";
		SBSampSet->Caption = "Mem";
		TabS->Tabs->Strings[0] = "S.pix";
		TabS->Tabs->Strings[1] = "S.templates 1";

		CBSTemp->Caption = "Show with template";
		CBSDraft->Caption = "Draft";

		KRFS->Caption = "ReSync";
		KRC->Caption = "&Copy";
		KRCC->Caption = "Clear";
		KRCS->Caption = "High accuracy slant adjustment";
		KRCR->Caption = "High accuracy re-sync";
		KRSR->Caption = "Return to the settings sampling frequency";
		KRI->Caption = "Accuracy of initial sync";
		KRI1->Caption = "Normal";
		KRI2->Caption = "Higher";
		KRI3->Caption = "Highest";
		KRAS->Caption = "Auto stop";
		KRAR->Caption = "Auto restart";
		KRASY->Caption = "Auto resync";
		KRSW->Caption = "Auto switch to RX window";
		KRSA->Caption = "Auto slant adjustment";
		KRH->Caption = "Copy to the history";
		KRR->Caption = "Resize to RX size";
		KRRC->Caption = "Resize to 320x256";
		KRB->Caption = "Background color...";
		KRCan->Caption = "Cancel";
		KRD->Caption = "Auto clear";

		KSP->Caption = "&Paste";
		KSD->Caption = "&Delete";
		KSL->Caption = "&Load from file...";
		KSS->Caption = "&Save to file...";
        KSE->Caption = "&Edit image...";
		KSCD->Caption = "&Move folder...";
		KSFJ->Caption = "Use JPEG format";
		KSFB->Caption = "Use Windows bitmap format";
		KSMS->Caption = "Create a catalog image";
		KSTP->Caption = "Test pattern";
		KSAI->Caption = "Image adjustment...";
		KSAM->Caption = "Auto margin";
		KSIS->Caption = "Stretch TX image by different size";
		KSRR->Caption = "Auto switch to RX window";
		KSCan->Caption = KRCan->Caption;
		KSF->Caption = "High quality resizing with pasted thumbnail";
		KSA->Caption = "Add the items to the template";

		KTPA->Caption = KSAI->Caption;
		KTPI->Caption = "Paste image";
		KTPB->Caption = "Bitmap";
		KTPO->Caption = "Overlay";
		KTKA->Caption = "Keep aspect ratio";
		KTOS->Caption = "Resize to the original size";
		KTU->Caption = "&Undo";
		KTC->Caption = "&Cut";
		KTP->Caption = "&Paste";
		KTD->Caption = "&Delete";
		KTO->Caption = "&Object";
		KTOI->Caption = "&Insert...";
		KTCI->Caption = "Insert custom item";
		KTOCB->Caption = "Copy as bitmap";
		KTOB->Caption = "Stretch as bitmap";
		KTOR->Caption = "Properties";
		KTE->Caption = "Add text to the list...";
		KTB->Caption = "Go to &Back";
		KTF->Caption = "Go to &Front";
		KTTD->Caption = "Show transparent point";
		KTT->Caption = "&Transparent point";
		KTAP->Caption = "Auto transparent point";
		KTTC->Caption = "Back&Ground (transparent) color...";
		KTAC->Caption = "Find unique color as background";
		KTLI->Caption = "Items";
		KTSA->Caption = "All";
		KTSI->Caption = "Selected item";
		KTA->Caption = "&All delete";
		KTDF->Caption = "Samples";
		KTMD->Caption = "Draw dummy text of macro";
		KTBT->Caption = "Show TX picture";
		KTSZ->Caption = "Size";
		KTSZT->Caption = "Adapt with TX image";
		KTSD->Caption = "Show size";
		KTX->Caption = "Options";
		KTFS->Caption = "Disable font smoothing";
		KTH->Caption = "Use fast code";
		KTCan->Caption = KRCan->Caption;

		KHCD->Caption = "&Move folder...";
		KHAP->Caption = KTPA->Caption;
		KHS->Caption = "&Save to file...";
		KHT->Caption = "Save to file with &Time stamp...";
		KHD->Caption = "&Delete";
		KHDA->Caption = "All delete";
		KHFJ->Caption = KSFJ->Caption;
		KHFB->Caption = KSFB->Caption;
		KHAC->Caption = "Auto copy to the another folder";
		KHACD->Caption = "Assign another folder name...";
		KHACT->Caption = "Write time stamp in the copied picture";
		KHACJ->Caption = "Copy as JPEG format";
		KHDS->Caption = "Drag && Drop size";
		KHTB->Caption = "Show title bar";
		KHVS->Caption = "Thumbnail size";
		KHCan->Caption = KRCan->Caption;

		KHWeb->Caption = "MMSSTV Web site";
		KHRef->Caption = "MMSSTV Reflector";

		KCWM->Caption = "Edit menu";
		KCWCancel->Caption = "Cancel";
		KCWS->Caption = "Speed";
		KCWADD->Caption = "Add menu...";

		SBPL->Hint = "Shift left";
        SBPR->Hint = "Shift right";
		SBPhase->Hint = "Manual sync adjustment";
		SBSlant->Hint = "Manual slant adjustment";
		CBASave->Caption = "Auto history";
		CBASave->Hint = "Auto save to the history";
		SBSampSet->Hint = "Memory sampling freq.";
		SBTO->Hint = "Memory TX offset";
		SBLK->Caption = "Lock";
		SBLK->Hint = "Lock RX";
		SBWHist->Hint = "Save to the history";
		SBRView->Hint = "Picture viewer";
		SBCopy->Hint = "Copy";
		SBHView->Hint = "Picture viewer";

		SBCW->Hint = "Output CW (Right click for CW menu)";
		SBUseTemp->Hint = "Use template";
		SBUS240->Hint = "Shift picture and template for 240 line mode";
		SBDS240->Hint = "Shift picture for the header color bar";
		SBUA240->Hint = "Adjustment picture and template for 240 line mode";
		SBDA240->Hint = "Adjustment picture for the header color bar";
		SBPaste->Hint = "Paste";
		SBTView->Hint = "Picture viewer";
		SBDSel->Hint = "Select item";
		SBDline->Hint = "Draw line";
		SBDBox->Hint = "Draw box";
		SBDBoxS->Hint = "Draw filled box";
		SBDText->Hint = "Draw text";
		SBDPic->Hint = "Draw picture";
		SBDHead->Hint = "Draw Color bar";
		SBDList->Hint = "Draw text from defined list";
		SBDDel->Hint = "Cut";
		SBDPaste->Hint = "Paste";
		SBDCol->Hint = "Set color";
		SBDLineset->Hint = "Set line style";
		SBDFont->Hint = "Set font";

		SBQSO->Hint = "Start/End QSO";
		SBData->Hint = "Data window";
		SBFind->Hint = "Find callsign";
		SBInit->Hint = "Clear data";
		SBList->Hint = "Log list window";
		SBULog->Hint = "Update template";

		SBHistFil->Hint = "Image adjustment";
		SBRXID->Hint = "Decode FSKID";
		SBTXID->Hint = "Encode FSKID";
		SBAdj->Hint = "High accuracy slant adjustment (Right click for Sync adjustment)";
	}
	else {
		MsgEng = 0;
	}
	KHMT->Caption = KSMS->Caption;
	KHC->Caption = KRC->Caption;
    KHE->Caption = KSE->Caption;
	KSC->Caption = KRC->Caption;
	KTOC->Caption = KRC->Caption;
	KTOE->Caption = KEdit->Caption;
	KTOP->Caption = KTP->Caption;
	KTOT->Caption = KTPO->Caption;
	KSB->Caption = KRB->Caption;
	KSTS->Caption = CBSTemp->Caption;
	KSTD->Caption = CBSDraft->Caption;
	KSSZ->Caption = KTSZ->Caption;
	KSS1->Caption = KTSZ1->Caption;
	KSS2->Caption = KTSZ2->Caption;
	KSS3->Caption = KTSZ3->Caption;
	KSS4->Caption = KTSZ4->Caption;
	SBTXFil->Hint = SBHistFil->Hint;
	SBRXFil->Hint = SBHistFil->Hint;
	KSSD->Caption = KTSD->Caption;
	KHSD->Caption = KTSD->Caption;
	KRSD->Caption = KTSD->Caption;
	KTL->Caption = KSL->Caption;
	KTLA->Caption = KTSA->Caption;
	KTS->Caption = KSS->Caption;
	SBFS->Caption = KRFS->Caption;
	KORep->Enabled = sys.m_Repeater;
	UpdateToneFreq();
}
//---------------------------------------------------------------------------
// ファイルメニューの更新
void __fastcall TMmsstv::KFileClick(TObject *Sender)
{
	if( pSound != NULL ){
		KFRecTim->Enabled = pSound->m_playmode ? FALSE : TRUE;
		KFRec->Enabled = KFRecTim->Enabled;
		KFPlay->Enabled = KFRecTim->Enabled;
		KFRecRew->Enabled = !KFRecTim->Enabled;
		KFRecStop->Enabled = KFRecRew->Enabled;
		KFPlayPos->Enabled = (pSound->m_playmode == 1) ? TRUE : FALSE;
		if(pSound->WaveFile.m_mode != 2){
			KFRec->Checked = FALSE;
			KFRecTim->Checked = FALSE;
		}
		MultProc();
	}
	if( Log.IsOpen() ){
		KLogFlush->Enabled = Log.IsEdit();
	}
}
//---------------------------------------------------------------------------
//void __fastcall TMmsstv::OnIdle(TObject *Sender, bool &Done)
// アイドル処理
void __fastcall TMmsstv::UpdateUI(void)
{
	if( sys.m_AutoStop || pDem->m_SyncRestart || sys.m_AutoSync ){
		SBLK->Down = FALSE;
	}
	else {
		SBLK->Down = TRUE;
	}
	if( Log.IsOpen() ){
		if( Log.m_CurChg ){
			Log.SetLastPos();
			UpdateTextData();
			Log.m_CurChg = 0;
			SBQSO->Down = Log.m_sd.btime ? 1 : 0;
		}

		SBQSO->Enabled = !HisCall->Text.IsEmpty();
		SBData->Enabled = TRUE;
		SBFind->Enabled = SBQSO->Enabled;
		SBInit->Enabled = TRUE;
		SBList->Enabled = TRUE;
	}
	else {
		SBQSO->Enabled = FALSE;
		SBData->Enabled = FALSE;
		SBFind->Enabled = FALSE;
		SBInit->Enabled = FALSE;
		SBList->Enabled = FALSE;
	}
}
//---------------------------------------------------------------------------
// レジストリからの読み出し
void __fastcall TMmsstv::ReadSampFreq(void)
{
	char	bf[256];
	AnsiString	as, ws;

	sprintf(bf, "%sMmsstv.ini", BgnDir);
	TMemIniFile	*pIniFile = new TMemIniFile(bf);

	SampFreq = ReadDoubleIniFile(pIniFile, "SoundCard", "SampFreq", SampFreq);
	if( (SampFreq > CLOCKMAX) || (SampFreq < 5000.0) ) SampFreq = 11025.0;
	sys.m_TxSampOff = ReadDoubleIniFile(pIniFile, "SoundCard", "TxSampOffset", 0.0);
	InitSampType();
	sys.m_SampFreq = SampFreq;
	SSTVSET.m_SampFreq = SampFreq;
	SSTVSET.m_TxSampFreq = SampFreq + sys.m_TxSampOff;
	SSTVSET.InitIntervalPara();
	delete pIniFile;

	m_ASLmt[0] = 25.0 * SampFreq / 11025;
	m_ASLmt[1] = 10.0 * SampFreq / 11025;
	m_ASLmt[2] = 2.0 * SampFreq / 11025;
	m_ASLmt[3] = 0.5 * SampFreq / 11025;    // 64
	m_ASLmt[4] = 0.2 * SampFreq / 11025;    // 128
	m_ASLmt[5] = 0.2 * SampFreq / 11025;    // 160
	m_ASLmt[6] = 0.08 * SampFreq / 11025;    // 220
}
//---------------------------------------------------------------------------
// レジストリからの読み出し
void __fastcall TMmsstv::ReadCombList(TMemIniFile *tp, AnsiString &as, LPCSTR pKey, LPCSTR pDef)
{
	as = tp->ReadString("ComboList", pKey, pDef);
	if( as.IsEmpty() ) as = pDef;
}
#define	INIVER	2
#define	PROVER	1
#define LCVER   2
#define PLLVER  2
//---------------------------------------------------------------------------
// レジストリからの読み出し
void __fastcall TMmsstv::ReadRegister(void)
{
	char	bf[256];
	int i;
	AnsiString	as, ws;

	sprintf(bf, "%sMmsstv.ini", BgnDir);
	TMemIniFile	*pIniFile = new TMemIniFile(bf);

	int IniVer = pIniFile->ReadInteger("Version", "INI", 0);
	ReadCombList(pIniFile, as, "LogFreq", ( lcid != LANG_JAPANESE ) ?
		"1.8,3.5,3.8,7,10,14,14.230,18,21,21.340,24,28,28.680,50,144,220,430,1200,2400":
		"1.8,3.5,3.8,7,7.033,10,14,14.230,18,21,21.340,24,28,28.680,50,144,430,1200,2400"
	);
	SetComboBox(LogFreq, as.c_str());
	ReadCombList(pIniFile, as, "LogRST", "595,575,555,594,574,554,444,333");
	SetComboBox(HisRST, as.c_str());
	SetComboBox(MyRST, as.c_str());

	sys.m_WindowState = pIniFile->ReadInteger("Window", "WindowState", sys.m_WindowState);
	sys.m_MemWindow = pIniFile->ReadInteger("Window", "MemoryWindow", sys.m_MemWindow);
	sys.m_UseB24 = pIniFile->ReadInteger("Window", "UseB24", sys.m_UseB24);
	if( sys.m_MemWindow ){
		Top = pIniFile->ReadInteger("Window", "Top", Top);
		Left = pIniFile->ReadInteger("Window", "Left", Left);
		Width = pIniFile->ReadInteger("Window", "Width", Width);
		Height = pIniFile->ReadInteger("Window", "Height", Height);
	}
	sys.m_RxViewTop = pIniFile->ReadInteger("Window", "RxViewTop", 0);
	sys.m_RxViewLeft = pIniFile->ReadInteger("Window", "RxViewLeft", 0);
	sys.m_RxViewWidth = pIniFile->ReadInteger("Window", "RxViewWidth", PBoxRX->Width);
	sys.m_RxViewHeight = pIniFile->ReadInteger("Window", "RxViewHeight", PBoxRX->Height);
	KVR->Checked = pIniFile->ReadInteger("Window", "RxView", 0);
	KRCD->Checked = pIniFile->ReadInteger("Window", "RxViewInd", 0);

	sys.m_SyncViewTop = pIniFile->ReadInteger("Window", "SyncViewTop", 0);
	sys.m_SyncViewLeft = pIniFile->ReadInteger("Window", "SyncViewLeft", 0);
	sys.m_SyncViewWidth = pIniFile->ReadInteger("Window", "SyncViewWidth", PBoxD12->Width);
	sys.m_SyncViewHeight = pIniFile->ReadInteger("Window", "SyncViewHeight", PBoxD12->Height);
	KVS->Checked = pIniFile->ReadInteger("Window", "SyncView", 0);
	sys.m_HistViewTB = pIniFile->ReadInteger("Window", "HistViewTB", 1);
	sys.m_HistViewTop = pIniFile->ReadInteger("Window", "HistViewTop", 0);
	sys.m_HistViewLeft = pIniFile->ReadInteger("Window", "HistViewLeft", 0);
	sys.m_HistViewWidth = pIniFile->ReadInteger("Window", "HistViewWidth", 516);
	sys.m_HistViewHeight = pIniFile->ReadInteger("Window", "HistViewHeight", 70);
	sys.m_HistViewSize = pIniFile->ReadInteger("Window", "HistViewSize", 2);
	KVH->Checked = pIniFile->ReadInteger("Window", "HistView", 0);

	sys.m_StockViewTop = pIniFile->ReadInteger("Window", "StockViewTop", Top+TabS->Top);
	sys.m_StockViewLeft = pIniFile->ReadInteger("Window", "StockViewLeft", Left);
	sys.m_StockViewWidth = pIniFile->ReadInteger("Window", "StockViewWidth", 470);
	sys.m_StockViewHeight = pIniFile->ReadInteger("Window", "StockViewHeight", 156);

	sys.m_WaterViewTop = pIniFile->ReadInteger("Window", "WaterViewTop", 0);
	sys.m_WaterViewLeft = pIniFile->ReadInteger("Window", "WaterViewLeft", GBLog->Left);
	sys.m_WaterViewWidth = pIniFile->ReadInteger("Window", "WaterViewWidth", GBLog->Width);
	sys.m_WaterViewHeight = pIniFile->ReadInteger("Window", "WaterViewHeight", PanelLvl->Height);

	sys.m_LogViewTop = pIniFile->ReadInteger("Window", "LogViewTop", GBLog->Top);
	sys.m_LogViewLeft = pIniFile->ReadInteger("Window", "LogViewLeft", GBLog->Left);
	sys.m_LogViewWidth = pIniFile->ReadInteger("Window", "LogViewWidth", GBLog->Width);
	sys.m_LogViewHeight = pIniFile->ReadInteger("Window", "LogViewHeight", GBLog->Height);

	sys.m_DivMode = pIniFile->ReadInteger("Window", "DivMode", sys.m_DivMode);
	i = pIniFile->ReadInteger("Window", "DivSW", 7);
	KVSE1->Checked = i & 1;
	KVSE2->Checked = (i >> 1) & 1;
	KVSE3->Checked = (i >> 2) & 1;

	sys.m_CtrBtnTop = pIniFile->ReadInteger("Window", "CtrBtnTop", 0);
	sys.m_CtrBtnLeft = pIniFile->ReadInteger("Window", "CtrBtnLeft", 0);
	sys.m_CtrBtnWidth = pIniFile->ReadInteger("Window", "CtrBtnWidth", 128);
	sys.m_CtrBtnHeight = pIniFile->ReadInteger("Window", "CtrBtnHeight", 198);
	KVC->Checked = pIniFile->ReadInteger("Window", "CtrBtn", 0);

	sys.m_FileViewShowSize = pIniFile->ReadInteger("FileView", "ShowSize", 0);
	sys.m_FileViewKeep = pIniFile->ReadInteger("FileView", "KeepA", 0);
	for( i = 0; i < 8; i++ ){
		sprintf(bf, "FileView%d", i+1);
		sys.m_FileViewFlag[i] = pIniFile->ReadInteger(bf, "Flag", 0);
		sys.m_FileViewSize[i] = pIniFile->ReadInteger(bf, "Size", 2);
		sys.m_FileViewMode[i] = pIniFile->ReadInteger(bf, "Mode", 1);
		sys.m_FileViewTop[i] = pIniFile->ReadInteger(bf, "Top", 0);
		sys.m_FileViewLeft[i] = pIniFile->ReadInteger(bf, "Left", 0);
		sys.m_FileViewWidth[i] = pIniFile->ReadInteger(bf, "Width", 516);
		sys.m_FileViewHeight[i] = pIniFile->ReadInteger(bf, "Height", 70);
		sys.m_FileViewMaxPage[i] = pIniFile->ReadInteger(bf, "MaxPage", 1);
		sys.m_FileViewCurPage[i] = pIniFile->ReadInteger(bf, "CurPage", 0);
		if( sys.m_FileViewMaxPage[i] > FPAGEMAX ) sys.m_FileViewMaxPage[i] = 10;
		if( sys.m_FileViewMaxPage[i] < 1 ) sys.m_FileViewMaxPage[i] = 1;
		for( int j = 0; j < sys.m_FileViewMaxPage[i]; j++ ){
			char bff[32];
			sprintf(bff, "FMPage%u", j+1);
			sys.m_FileViewFMPage[i][j] = pIniFile->ReadInteger(bf, bff, 0x80000000);
			sprintf(bff, j ? "Folder%u" : "Folder", j+1);
			sys.m_FileViewFolder[i][j] = pIniFile->ReadString(bf, bff, "");
			sprintf(bff, "Name%u", j+1);
			sys.m_FileViewName[i][j] = pIniFile->ReadString(bf, bff, "");
		}
	}
	sys.m_PreViewTop = pIniFile->ReadInteger("Window", "PreViewTop", 0);
	sys.m_PreViewLeft = pIniFile->ReadInteger("Window", "PreViewLeft", 0);
	sys.m_PreViewWidth = pIniFile->ReadInteger("Window", "PreViewWidth", 160);
	sys.m_PreViewHeight = pIniFile->ReadInteger("Window", "PreViewHeight", 128);

	m_MainPage = pIniFile->ReadInteger("Window", "MainPage", pgRX);
	Page->ActivePage = Page->Pages[m_MainPage];

	TabS->TabIndex = pIniFile->ReadInteger("Window", "StockPage", 0);
	m_PSPage = pIniFile->ReadInteger("Window", "StockPicPosition", m_PSPage);
	for( i = 0; i < STOCKPAGE; i++ ){
		sprintf(bf, "StockTempPos%u", i + 1);
		m_TSPage[i] = pIniFile->ReadInteger("Window", bf, m_TSPage[i]);
	}
	UDStock->Position = short(TabS->TabIndex ? m_TSPage[TabS->TabIndex-1] : m_PSPage);
	KSF->Checked = pIniFile->ReadInteger("Window", "HQResizing", KSF->Checked);
	SBUseTemp->Down = pIniFile->ReadInteger("Window", "UseTemplate", SBUseTemp->Down);
	CBSTemp->Checked = pIniFile->ReadInteger("Window", "ShowSuperimpos", 0);
	CBSDraft->Checked = pIniFile->ReadInteger("Window", "DraftSuperimpos", CBSDraft->Checked);
	sys.m_ShowSizeRX = pIniFile->ReadInteger("Window", "ShowSizeRX", sys.m_ShowSizeRX);
	sys.m_ShowSizeHist = pIniFile->ReadInteger("Window", "ShowSizeHist", sys.m_ShowSizeHist);
	sys.m_ShowSizeTX = pIniFile->ReadInteger("Window", "ShowSizeTX", sys.m_ShowSizeTX);
	sys.m_ShowSizeTemp = pIniFile->ReadInteger("Window", "ShowSizeTemp", sys.m_ShowSizeTemp);
	sys.m_ShowSizeStock = pIniFile->ReadInteger("Window", "ShowSizeStock", sys.m_ShowSizeStock);
	KSIS->Checked = pIniFile->ReadInteger("Window", "StretchTXimage", KSIS->Checked);
	KSRR->Checked = pIniFile->ReadInteger("Window", "SwitchRX", KSRR->Checked);

	sys.m_PicClipType = pIniFile->ReadInteger("Window", "PicClipType", sys.m_PicClipType);
	sys.m_PicClipRatio = pIniFile->ReadInteger("Window", "PicClipRatio", sys.m_PicClipRatio);
	sys.m_PicClipMode = pIniFile->ReadInteger("Window", "PicClipMode", sys.m_PicClipMode);
	sys.m_PicClipView = pIniFile->ReadInteger("Window", "PicClipView", sys.m_PicClipView);
	sys.m_PicClipColor = (TColor)pIniFile->ReadInteger("Window", "PicClipColor", sys.m_PicClipColor);
	sys.m_Way240 = pIniFile->ReadInteger("Window", "Way240", sys.m_Way240);

	sys.m_Priority = pIniFile->ReadInteger("Window", "Priority", sys.m_Priority);

	KSAM->Checked = pIniFile->ReadInteger("Window", "AutoMargin", TRUE);
	m_TxClipType = pIniFile->ReadInteger("Window", "TxClipType", m_TxClipType);
	sys.m_HistMax = pIniFile->ReadInteger("Window", "HistMax", sys.m_HistMax);
	KRD->Checked = pIniFile->ReadInteger("Window", "AutoRxClear", 1);
	CBASave->Checked = pIniFile->ReadInteger("Window", "AutoRxSave", CBASave->Checked);
	KEA->Checked = pIniFile->ReadInteger("Window", "AutoCopy", KEA->Checked);
	KTBT->Checked = pIniFile->ReadInteger("Window", "TempSuperimpos", KTBT->Checked);
	KTMD->Checked = pIniFile->ReadInteger("Window", "DummyMacro", KTMD->Checked);
	KTTD->Checked = pIniFile->ReadInteger("Window", "ShowTransPoint", KTTD->Checked);
	KTAP->Checked = pIniFile->ReadInteger("Window", "AutoTransPoint", KTAP->Checked);
	sys.m_HDDSize = pIniFile->ReadInteger("Window", "HistDDSize", sys.m_HDDSize);

	sys.m_WinFontName = pIniFile->ReadString("WindowFont", "Name", sys.m_WinFontName);
	sys.m_WinFontCharset = (BYTE)pIniFile->ReadInteger("WindowFont", "Charset", sys.m_WinFontCharset);
	sys.m_WinFontStyle = pIniFile->ReadInteger("WindowFont", "Style", sys.m_WinFontStyle);
	sys.m_FontAdjSize = pIniFile->ReadInteger("WindowFont", "Adjust", sys.m_FontAdjSize);
	sys.m_FFTType = pIniFile->ReadInteger("Define", "FFTType", sys.m_FFTType);
	sys.m_FFTGain = pIniFile->ReadInteger("Define", "FFTGain", sys.m_FFTGain);
	sys.m_FFTResp = pIniFile->ReadInteger("Define", "FFTResp", sys.m_FFTResp);
	sys.m_FFTStg = pIniFile->ReadInteger("Define", "FFTStgv", sys.m_FFTStg);
	sys.m_FFTWidth = pIniFile->ReadInteger("Define", "FFTWidth", sys.m_FFTWidth);
	sys.m_FFTAGC = pIniFile->ReadInteger("Define", "FFTAGC", sys.m_FFTAGC);
	sys.m_FFTPriority = pIniFile->ReadInteger("Define", "FFTPriority", sys.m_FFTPriority);
	KVW->Checked = pIniFile->ReadInteger("Define", "WaterSW", KVW->Checked);
	pDem->m_LevelType = pIniFile->ReadInteger("Define", "LevelType", pDem->m_LevelType);

	sys.m_HelpNotePad = pIniFile->ReadInteger("Help", "UseNotePad", sys.m_HelpNotePad);
	KHlpPad->Checked = sys.m_HelpNotePad;

	sys.m_Sharp2D = pIniFile->ReadInteger("ImageAdj", "Sharp2D", sys.m_Sharp2D);

	sys.m_ColorLow = (TColor)pIniFile->ReadInteger("Color", "WaterLow", sys.m_ColorLow);
	sys.m_ColorHigh = (TColor)pIniFile->ReadInteger("Color", "WaterHigh", sys.m_ColorHigh);
	sys.m_ColorFFTB = (TColor)pIniFile->ReadInteger("Color", "FFTB", sys.m_ColorFFTB);
	sys.m_ColorFFT = (TColor)pIniFile->ReadInteger("Color", "FFT", sys.m_ColorFFT);
	sys.m_ColorFFTStg = (TColor)pIniFile->ReadInteger("Color", "FFTStg", sys.m_ColorFFTStg);
	sys.m_ColorFFTSync = (TColor)pIniFile->ReadInteger("Color", "FFTSync", sys.m_ColorFFTSync);
	sys.m_ColorFFTFreq = (TColor)pIniFile->ReadInteger("Color", "FFTFreq", sys.m_ColorFFTFreq);

	sys.m_ColorRXB = (TColor)pIniFile->ReadInteger("Color", "RxWindow", sys.m_ColorRXB);
	PanelRX->Color = sys.m_ColorRXB;
	PanelHist->Color = sys.m_ColorRXB;

	sys.m_echo = pIniFile->ReadInteger("Define", "TXLoopBack", sys.m_echo);

	sys.m_UseRxBuff = pIniFile->ReadInteger("Define", "UseRxBuff", sys.m_UseRxBuff);
	pDem->m_bpf = pIniFile->ReadInteger("Define", "DEMBPF", pDem->m_bpf);
	pDem->m_MSync = pIniFile->ReadInteger("Define", "SyncStart", pDem->m_MSync);
	pDem->m_SyncRestart = pIniFile->ReadInteger("Define", "SyncReStart", pDem->m_SyncRestart);
	pDem->CalcBPF();
	sys.m_AutoStop = pIniFile->ReadInteger("Define", "AutoStop", sys.m_AutoStop);
	sys.m_AutoSync = pIniFile->ReadInteger("Define", "AutoSync", sys.m_AutoSync);
	m_SyncAccuracy = pIniFile->ReadInteger("Define", "SyncAccuracy", m_SyncAccuracy);
	KRSW->Checked = pIniFile->ReadInteger("Define", "AutoSwirchRX", KRSW->Checked);
	KRSA->Checked = pIniFile->ReadInteger("Define", "AutoSlant", KRSA->Checked);

	pDem->m_SenseLvl = pIniFile->ReadInteger("Define", "DEMSLVL", pDem->m_SenseLvl);
	pDem->SetSenseLvl();
	sys.m_Differentiator = pIniFile->ReadInteger("Define", "Differentiator", sys.m_Differentiator);
	sys.m_DiffLevelP = ReadDoubleIniFile(pIniFile, "Define", "DiffLevel", sys.m_DiffLevelP);
	sys.m_DiffLevelM = sys.m_DiffLevelP / 3.0;

	sys.m_FixedTxMode = pIniFile->ReadInteger("Define", "FixedTxMode", sys.m_FixedTxMode);
	SSTVSET.m_TxMode = pIniFile->ReadInteger("Define", "SSTVMode", SSTVSET.m_TxMode);
	SSTVSET.SetMode(SSTVSET.m_TxMode);
	SSTVSET.SetTxMode(SSTVSET.m_TxMode);
	for( i = 0; i < 9; i++ ){
		sprintf(bf, "Button%d", i);
		m_ModeAssignTX[i] = pIniFile->ReadInteger("Mode", bf, m_ModeAssignTX[i]);
		m_ModeAssignRX[i] = m_ModeAssignTX[i];
	}
	pDem->m_fskdecode = pIniFile->ReadInteger("Define", "RXFSKID", pDem->m_fskdecode);
	SBRXID->Down = pDem->m_fskdecode;
	sys.m_TXFSKID = pIniFile->ReadInteger("Define", "TXFSKID", sys.m_TXFSKID);
	SBTXID->Down = sys.m_TXFSKID;
	sys.m_CWID = pIniFile->ReadInteger("Define", "CWID", sys.m_CWID);
	sys.m_CWIDWPM = pIniFile->ReadInteger("Define", "CWIDWPM", 0);
	if( sys.m_CWIDWPM <= 0 ){
		sys.m_CWIDSpeed = pIniFile->ReadInteger("Define", "CWIDSpeed", sys.m_CWIDSpeed);
		sys.m_CWIDWPM = (1110.0 / (sys.m_CWIDSpeed + 30)) + 0.5;
    }
	sys.m_CWIDFreq = pIniFile->ReadInteger("Define", "CWIDFreq", sys.m_CWIDFreq);
	sys.m_CWIDText = pIniFile->ReadString("Define", "CWIDText", sys.m_CWIDText);
	sys.m_MMVID = pIniFile->ReadString("Define", "MMVID", sys.m_MMVID);
	pMod->m_TuneFreq = pIniFile->ReadInteger("Define", "TuneFreq", pMod->m_TuneFreq);
	sys.m_TuneTXTime = pIniFile->ReadInteger("Define", "TuneTime", sys.m_TuneTXTime);
	sys.m_TuneSat = pIniFile->ReadInteger("Define", "TuneSat", sys.m_TuneSat);
	sys.m_CWText = pIniFile->ReadString("Define", "CWText", sys.m_CWText);
	sys.m_VOX = pIniFile->ReadInteger("Define", "VOX", sys.m_VOX);
	as = pIniFile->ReadString("Define", "VOXTone", sys.m_VOXSound);
	Yen2CrLf(sys.m_VOXSound, as);

	pMod->m_VariOut = pIniFile->ReadInteger("Vari", "Enabled", 0);
	pMod->m_VariOut = 0;
	pMod->m_VariR = pIniFile->ReadInteger("Vari", "R", 298);
	pMod->m_VariG = pIniFile->ReadInteger("Vari", "G", 588);
	pMod->m_VariB = pIniFile->ReadInteger("Vari", "B", 110);
	if( pMod->m_VariB == 11 ) pMod->m_VariB = 110;

	pMod->m_outgain = pIniFile->ReadInteger("Define", "TXGAIN", pMod->m_outgain);
	pMod->InitGain();
	pMod->m_bpf = pIniFile->ReadInteger("Define", "TXBPF", pMod->m_bpf);
	pMod->m_lpf = pIniFile->ReadInteger("Define", "TXLPF", pMod->m_lpf);
	pMod->m_bpftap = pIniFile->ReadInteger("Define", "TXBPFTAP", pMod->m_bpftap);
	pMod->m_lpffq = pIniFile->ReadInteger("Define", "TXLPFFQ", pMod->m_lpffq);
	pMod->CalcFilter();

	pDem->m_afc = pIniFile->ReadInteger("Define", "RXAFC", pDem->m_afc);
	SBAFC->Down = pDem->m_afc;
	pSound->m_lms = pIniFile->ReadInteger("Define", "RXLMS", pSound->m_lms);
	pSound->m_lms_AN = pIniFile->ReadInteger("Define", "RXLMSAN", pSound->m_lms_AN);
	pSound->lms.SetAN(pSound->m_lms_AN);
	UpdateLMS();

	sys.m_SoundFifoRX = pIniFile->ReadInteger("Define", "SoundFifoRX", sys.m_SoundFifoRX);
	sys.m_SoundFifoTX = pIniFile->ReadInteger("Define", "SoundFifoTX", sys.m_SoundFifoTX);
	if( sys.m_SoundFifoRX > WAVE_FIFO_MAX ) sys.m_SoundFifoRX = WAVE_FIFO_MAX;
	if( sys.m_SoundFifoTX > WAVE_FIFO_MAX ) sys.m_SoundFifoTX = WAVE_FIFO_MAX;
	sys.m_SoundPriority = pIniFile->ReadInteger("Define", "SoundPriority", sys.m_SoundPriority);
	sys.m_SoundDevice = pIniFile->ReadString("Define", "SoundDevice", sys.m_SoundDevice);
	sys.m_SoundMMW = pIniFile->ReadString("Define", "SoundMMW", sys.m_SoundMMW);
	pSound->m_FifoSizeRX = sys.m_SoundFifoRX;
	pSound->m_FifoSizeTX = sys.m_SoundFifoTX;
	pSound->SetDeviceID();
	sys.m_SoundStereo = pIniFile->ReadInteger("Define", "SoundStereo", sys.m_SoundStereo);
	sys.m_StereoTX = pIniFile->ReadInteger("Define", "StereoTX", sys.m_StereoTX);
	pSound->InitWFX();

	pDem->m_Type = pIniFile->ReadInteger("Define", "DemType", pDem->m_Type);
	pDem->m_fqc.m_outOrder = pIniFile->ReadInteger("Define", "fqcOutOrder", pDem->m_fqc.m_outOrder);
	pDem->m_fqc.m_outFC = ReadDoubleIniFile(pIniFile, "Define", "fqcOutFC", pDem->m_fqc.m_outFC);
	pDem->m_fqc.m_Type = pIniFile->ReadInteger("Define", "fqcType", pDem->m_fqc.m_Type);
	pDem->m_fqc.m_SmoozFq = ReadDoubleIniFile(pIniFile, "Define", "fqcSmooth", pDem->m_fqc.m_SmoozFq);
	pDem->m_fqc.CalcLPF();

	int LCVer = pIniFile->ReadInteger("Version", "LinearCalibration", 0);
	int PLLVer = pIniFile->ReadInteger("Version", "PLL", 0);
	sys.m_DemOff = pIniFile->ReadInteger("Define", "ColorOffset", sys.m_DemOff);
	sys.m_DemWhite = ReadDoubleIniFile(pIniFile, "Define", "ColorWhite", sys.m_DemWhite);
	sys.m_DemBlack = ReadDoubleIniFile(pIniFile, "Define", "ColorBlack", sys.m_DemBlack);
	sys.m_DemCalibration = pIniFile->ReadInteger("Define", "ColorCalibration", sys.m_DemCalibration);
	for( i = 0; i < 17; i++ ){
		sprintf(bf, "Value%d", i);
		sys.m_Dem17[i] = ReadDoubleIniFile(pIniFile, "Polynomial", bf, sys.m_Dem17[i]);
	}

	pDem->m_pll.SetVcoGain(ReadDoubleIniFile(pIniFile, "Define", "pllVcoGain", pDem->m_pll.m_vcogain));
	pDem->m_pll.m_loopOrder = pIniFile->ReadInteger("Define", "pllLoopOrder", pDem->m_pll.m_loopOrder);
	pDem->m_pll.m_loopFC = ReadDoubleIniFile(pIniFile, "Define", "pllLoopFC", pDem->m_pll.m_loopFC);
	pDem->m_pll.m_outOrder = pIniFile->ReadInteger("Define", "pllOutOrder", pDem->m_pll.m_outOrder);
	pDem->m_pll.m_outFC = ReadDoubleIniFile(pIniFile, "Define", "pllOutFC", pDem->m_pll.m_outFC);
	pDem->m_pll.MakeLoopLPF();
	pDem->m_pll.MakeOutLPF();

	sys.m_TxRxName = pIniFile->ReadString("Define", "PTT", sys.m_TxRxName);
	sys.m_TxRxLock = pIniFile->ReadInteger("Define", "PTTLock", sys.m_TxRxLock);
	sys.m_RTSonRX = pIniFile->ReadInteger("Define", "RTSonRX", sys.m_RTSonRX);

	sys.m_Call = pIniFile->ReadString("Define", "Call", sys.m_Call);

	sys.m_TempDelay = pIniFile->ReadInteger("Log", "TempDelay", sys.m_TempDelay);
	sys.m_AutoTimeOffset = pIniFile->ReadInteger("Log", "AutoUTCOffset", sys.m_AutoTimeOffset);
	sys.m_TimeOffset = pIniFile->ReadInteger("Log", "UTCOffset", sys.m_TimeOffset);
	sys.m_TimeOffsetMin = pIniFile->ReadInteger("Log", "UTCOffsetMin", sys.m_TimeOffsetMin);
	sys.m_LogLink = pIniFile->ReadInteger("Log", "LogLink", sys.m_LogLink);
	LogLink.LoadMMLink(pIniFile);
	LogLink.SetPolling(pIniFile->ReadInteger("Log", "LinkPoll", 0));
	LogLink.SetPTTEnabled(pIniFile->ReadInteger("Log", "LinkPTT", FALSE));
	Log.m_FileName = pIniFile->ReadString("LogFile", "Name", Log.m_FileName);
	Log.MakeName(Log.m_FileName.c_str());

	sys.m_JPEGQuality = pIniFile->ReadInteger("Window", "JPEG", sys.m_JPEGQuality);

	RxHist.m_UseJPEG = pIniFile->ReadInteger("Dir", "HistoryFormat", RxHist.m_UseJPEG);
	as = pIniFile->ReadString("Dir", "History", HistDir);
	strcpy(HistDir, as.c_str());
	sys.m_UseJPEG = pIniFile->ReadInteger("Dir", "StockFormat", sys.m_UseJPEG);
	as = pIniFile->ReadString("Dir", "Stock", StockDir);
	strcpy(StockDir, as.c_str());
	as = pIniFile->ReadString("Dir", "RecSound", RecDir);
	strcpy(RecDir, as.c_str());
	as = pIniFile->ReadString("Dir", "Bitmap", BitmapDir);
	strcpy(BitmapDir, as.c_str());
	as = pIniFile->ReadString("Dir", "SaveBitmap", SBitmapDir);
	strcpy(SBitmapDir, as.c_str());
	as = pIniFile->ReadString("Dir", "Template", TemplateDir);
	strcpy(TemplateDir, as.c_str());
	sys.m_AutoSaveDir = pIniFile->ReadString("Dir", "AutoSaveDir", sys.m_AutoSaveDir);
//    PBoxHist->DragMode = sys.m_AutoSaveDir.IsEmpty() ? dmManual : dmAutomatic;
	KHAC->Checked = pIniFile->ReadInteger("Dir", "AutoSave", KHAC->Checked);
	KHACT->Checked = pIniFile->ReadInteger("Dir", "AutoSaveTime", KHACT->Checked);
	KHACJ->Checked = pIniFile->ReadInteger("Dir", "AutoSaveFormat", KHACJ->Checked);

	as = pIniFile->ReadString("Dir", "ExtLog", ExtLogDir);
	strcpy(ExtLogDir, as.c_str());
	for( i = 0; i < TEXTCONVMAX; i++ ){
		sprintf(bf, "WDEF%d", i+1);
		LogText.m_tConv[i].Key = pIniFile->ReadString("ExtConv", bf, LogText.m_tConv[i].Key);
		sprintf(bf, "WLEN%d", i+1);
		LogText.m_tConv[i].w = pIniFile->ReadInteger("ExtConv", bf, LogText.m_tConv[i].w);
	}
	for( i = 0; i < TEXTCONVMAX; i++ ){
		sprintf(bf, "RDEF%d", i+1);
		LogText.m_rConv[i].Key = pIniFile->ReadString("ExtConv", bf, LogText.m_rConv[i].Key);
		sprintf(bf, "RLEN%d", i+1);
		LogText.m_rConv[i].w = pIniFile->ReadInteger("ExtConv", bf, LogText.m_rConv[i].w);
	}
	for( i = 0; i < 8; i++ ){
		sprintf(bf, "Program%d", i+1);
		sys.m_ExtMode[i] = pIniFile->ReadInteger(bf, "Flag", sys.m_ExtMode[i]);
		sys.m_ExtName[i] = pIniFile->ReadString(bf, "Name", sys.m_ExtName[i]);
		sys.m_ExtCmd[i] = pIniFile->ReadString(bf, "File", sys.m_ExtCmd[i]);
	}

	for( i = 0; i < RECMENUMAX; i++ ){
		sprintf(bf, "File%d", i + 1);
		as = pIniFile->ReadString("Recent File", bf, "");
		RecentMenu.SetItemText(i, as.c_str());
	}
	Log.ReadIniFile("LogSet", pIniFile);

	LoadRADIOSetup(pIniFile);

	for( i = 0; i < 6; i++ ){
		sprintf(bf, "Color%d", i+1);
		sys.m_ColText[i] = (TColor)pIniFile->ReadInteger("DrawText", bf, sys.m_ColText[i]);
	}
	for( i = 0; i < 4; i++ ){
		sprintf(bf, "Color%d", i+1);
		sys.m_ColBar[i] = (TColor)pIniFile->ReadInteger("DrawBar", bf, sys.m_ColBar[i]);
	}
	for( i = 0; i < 32; i++ ){
		sprintf(bf, "Color%d", i+1);
		sys.m_ColorSet[i] = (TColor)pIniFile->ReadInteger("ColorSet", bf, sys.m_ColorSet[i]);
	}

	for( i = 0; i < 16; i++ ){
		sprintf(bf, "Text%d", i+1);
		sys.m_TextList[i] = pIniFile->ReadString("TextList", bf, sys.m_TextList[i]);
	}

	sys.m_TextGrade = pIniFile->ReadInteger("Text", "Grade", sys.m_TextGrade);
	sys.m_TextShadow = pIniFile->ReadInteger("Text", "Shadow", sys.m_TextShadow);
	sys.m_TextZero = pIniFile->ReadInteger("Text", "Zero", sys.m_TextZero);
	sys.m_TextRot = pIniFile->ReadInteger("Text", "Rot", sys.m_TextRot);
	sys.m_TextVert = pIniFile->ReadInteger("Text", "Vert", sys.m_TextVert);
	sys.m_TextVertH = pIniFile->ReadInteger("Text", "VertH", sys.m_TextVertH);
	sys.m_TextStack = pIniFile->ReadInteger("Text", "3D", sys.m_TextStack);
	as = pIniFile->ReadString("Text", "MSG", sys.m_Msg);
	Yen2CrLf(sys.m_Msg, as);
	sys.m_DisFontSmooth = pIniFile->ReadInteger("Text", "DisFontSmooth", sys.m_DisFontSmooth);
	i = pIniFile->ReadInteger("Text", "Temp24", sys.m_Temp24);
    if( (i >> 16) != sys.m_BitPixel ){
        sys.m_Temp24 = (sys.m_BitPixel >= 24);
    }
    else {
		sys.m_Temp24 = i & 1;
    }

	sys.m_PicShape = pIniFile->ReadInteger("Pic", "Shape", sys.m_PicShape);
	sys.m_PicAdjust = pIniFile->ReadInteger("Pic", "Adjust", sys.m_PicAdjust);
	sys.m_PicLineStyle = pIniFile->ReadInteger("Pic", "LineStyle", sys.m_PicLineStyle);
	sys.m_PicLineColor = (TColor)pIniFile->ReadInteger("Pic", "LineColor", sys.m_PicLineColor);

	sys.m_PicSelRTM = pIniFile->ReadInteger("PicSel", "RTM", sys.m_PicSelRTM);
	sys.m_PicSelSmooz = pIniFile->ReadInteger("PicSel", "Smooz", sys.m_PicSelSmooz);
	sys.m_PicSelCurCol = (TColor)pIniFile->ReadInteger("PicSel", "Color", sys.m_PicSelCurCol);

	int ProVER = pIniFile->ReadInteger("ProFile", "Version", 0);
	PRODEM *pPD = m_DemPro;
	for( i = 0; i < 8; i++, pPD++ ){
		sprintf(bf, "ProFile%d", i+1);
		as = pIniFile->ReadString(bf, "Name", pPD->Name);
		if( as.IsEmpty() ) continue;

		pPD->Name = as.c_str();
		pPD->VCOGain = ReadDoubleIniFile(pIniFile, bf, "VcoGain", pPD->VCOGain);
		pPD->loopOrder = pIniFile->ReadInteger(bf, "loopOrder", pPD->loopOrder);
		pPD->loopFC = ReadDoubleIniFile(pIniFile, bf, "loopFC", pPD->loopFC);
		pPD->OutOrder = pIniFile->ReadInteger(bf, "OutOrder", pPD->OutOrder);
		pPD->OutFC = ReadDoubleIniFile(pIniFile, bf, "OutFC", pPD->OutFC);

		pPD->Type = pIniFile->ReadInteger(bf, "DemType", pPD->Type);
		pPD->crossOutOrder = pIniFile->ReadInteger(bf, "fqcOutOrder", pPD->crossOutOrder);
		pPD->crossOutFC = ReadDoubleIniFile(pIniFile, bf, "fqcOutFC", pPD->crossOutFC);

		pPD->crossType = pIniFile->ReadInteger(bf, "fqcType", pPD->crossType);
		pPD->crossSmooz = ReadDoubleIniFile(pIniFile, bf, "fqcSmooth", pPD->crossSmooz);

		pPD->DemOff = ReadDoubleIniFile(pIniFile, bf, "DemOff", pPD->DemOff);
		pPD->DemWhite = ReadDoubleIniFile(pIniFile, bf, "DemWhite", pPD->DemWhite);
		pPD->DemBlack = ReadDoubleIniFile(pIniFile, bf, "DemBlack", pPD->DemBlack);
		pPD->DemCalibration = pIniFile->ReadInteger(bf, "DemCalibration", pPD->DemCalibration);
		char bff[64];
        double *dp = pPD->Dem17;
		for( int j = 0; j < 17; j++, dp++ ){
			sprintf(bff, "DemPolynomial%d", j);
			*dp = ReadDoubleIniFile(pIniFile, bf, bff, *dp);
		}
		pPD->Differentiator = pIniFile->ReadInteger(bf, "Differentiator", pPD->Differentiator);
		pPD->DiffLevel = ReadDoubleIniFile(pIniFile, bf, "DiffLevel", pPD->DiffLevel);
	}
	if( (ProVER < PROVER) || (LCVer < LCVER) || (PLLVer < PLLVER) ){
		InitProfile();
		SetProFile(8);
	}
	for( i = 0; i < FSKIDMAX; i++ ){
		sprintf(bf, "Item%d", i+1);
		as = pIniFile->ReadString("Calls", bf, "");
		if( as.IsEmpty() ) break;
		LPSTR p;
		for( p = as.c_str(); *p; p++ ){
			if( *p == '_' ) *p = '\t';
		}
		TMenuItem *pm = new TMenuItem(this);
		pm->Caption = as.c_str();
		pm->RadioItem = FALSE;
		pm->OnClick = KCClick;
		pm->Checked = FALSE;
		pm->Enabled = TRUE;
		PopupC->Items->Add(pm);
	}
	for( i = 0; i < 4; i++ ){
		sprintf(bf, "Font.%d", i+1);
        sys.m_RegFont[i].m_Name = pIniFile->ReadString(bf, "Name", sys.m_RegFont[i].m_Name);
        sys.m_RegFont[i].m_Charset = (BYTE)pIniFile->ReadInteger(bf, "Charset", sys.m_RegFont[i].m_Charset);
        sys.m_RegFont[i].m_Height = pIniFile->ReadInteger(bf, "Height", sys.m_RegFont[i].m_Height);
        sys.m_RegFont[i].m_Style = pIniFile->ReadInteger(bf, "Style", sys.m_RegFont[i].m_Style);
    }
//CWMenu
	sys.m_nCWMenu = pIniFile->ReadInteger("CWMenu", "Menus", sys.m_nCWMenu);
	for( i = 0; i < sys.m_nCWMenu; i++ ){
		sprintf(bf, "M%d", i+1);
		sys.m_CWMenu[i] = pIniFile->ReadString("CWMenu", bf, sys.m_CWMenu[i]);
    }
//RadioMenu
    m_nRadioMenu = pIniFile->ReadInteger("RadioMenu", "Menus", m_nRadioMenu);
	for( i = 0; i < m_nRadioMenu; i++ ){
		sprintf(bf, "Cap%d", i+1);
        m_RadioMenu[i].strTTL = pIniFile->ReadString("RadioMenu", bf, m_RadioMenu[i].strTTL);
		sprintf(bf, "Cmd%d", i+1);
        m_RadioMenu[i].strCMD = pIniFile->ReadString("RadioMenu", bf, m_RadioMenu[i].strCMD);
    }

// リピータ
	if( sys.m_Repeater ){
		pDem->m_Repeater = pIniFile->ReadInteger("Repeater", "Enabled", 0);

		pDem->m_RepTone = pIniFile->ReadInteger("Repeater", "Tone", 1750);
		pDem->InitRepeater();
		pDem->m_RepSQ = pIniFile->ReadInteger("Repeater", "SQLVL", 6000);

		sys.m_RepSenseLvl = pIniFile->ReadInteger("Repeater", "Sense", 2);
		pDem->SetRepSenseLvl();
		sys.m_RepTimeA = pIniFile->ReadInteger("Repeater", "TimeA", 1500);
		sys.m_RepTimeB = pIniFile->ReadInteger("Repeater", "TimeB", 500);
		sys.m_RepTimeC = pIniFile->ReadInteger("Repeater", "TimeC", 10000);
		sys.m_RepTimeD = pIniFile->ReadInteger("Repeater", "TimeD", 2000);
		sys.m_RepAnsCW = pIniFile->ReadString("Repeater", "AnsCW", "%m K");

		sys.m_RepBeacon = pIniFile->ReadInteger("Repeater", "Beacon", 0);
		sys.m_RepBeaconMode = pIniFile->ReadInteger("Repeater", "BeaconMode", smSCT1);
		sys.m_RepBeaconFilter = pIniFile->ReadInteger("Repeater", "BeaconFilter", 1);
		sys.m_RepTempTX = pIniFile->ReadString("Repeater", "TempReplay", "0");
		sys.m_RepTempBeacon = pIniFile->ReadString("Repeater", "TempBeacon", "0");
		sys.m_RepBottomAdj = pIniFile->ReadInteger("Repeater", "BottomAdj", 1);
		sys.m_RepQuietnessTime = pIniFile->ReadInteger("Repeater", "QuietnessTime", 300);
		sys.m_RepFolder = pIniFile->ReadString("Repeater", "Folder", "");
	}
	else {
		pDem->m_Repeater = 0;
	}
	delete pIniFile;
	if( IniVer < INIVER ){
    	sys.m_PicClipView = 1;
		sys.m_PicClipColor = clBlack;
    }
}
//---------------------------------------------------------------------------
// レジストリへの書き込み
void __fastcall TMmsstv::WriteRegister(void)
{
	CWaitCursor wait;
	int     i;
	char	bf[256];
	sprintf(bf, "%sMmsstv.ini", BgnDir);

	try{
		TMemIniFile	*pIniFile = new TMemIniFile(bf);

		WriteDoubleIniFile(pIniFile, "SoundCard", "SampFreq", sys.m_SampFreq);
		WriteDoubleIniFile(pIniFile, "SoundCard", "TxSampOffset", sys.m_TxSampOff);

		AnsiString as;
		GetComboBox(as, LogFreq);
		if( !as.IsEmpty() ) pIniFile->WriteString("ComboList", "LogFreq", as);
		GetComboBox(as, HisRST);
		if( !as.IsEmpty() ) pIniFile->WriteString("ComboList", "LogRST", as);

		pIniFile->WriteInteger("Version", "INI", INIVER);
		pIniFile->WriteInteger("Version", "LinearCalibration", LCVER);
		pIniFile->WriteInteger("Version", "PLL", PLLVER);

		pIniFile->WriteInteger("Window", "Priority", sys.m_Priority);
		pIniFile->WriteInteger("Window", "WindowState", WindowState);
		pIniFile->WriteInteger("Window", "MemoryWindow", sys.m_MemWindow);
		pIniFile->WriteInteger("Window", "UseB24", sys.m_UseB24);
		if( (WindowState == wsNormal) && sys.m_MemWindow ){
			pIniFile->WriteInteger("Window", "Top", Top);
			pIniFile->WriteInteger("Window", "Left", Left);
			if( sys.m_DivMode != 2 ){
				pIniFile->WriteInteger("Window", "Width", Width);
			}
			if( !sys.m_DivMode ){
				pIniFile->WriteInteger("Window", "Height", Height);
			}
		}
		pIniFile->WriteInteger("Window", "RxViewInd", KRCD->Checked);
		pIniFile->WriteInteger("Window", "RxView", KVR->Checked);
		pIniFile->WriteInteger("Window", "RxViewTop", sys.m_RxViewTop);
		pIniFile->WriteInteger("Window", "RxViewLeft", sys.m_RxViewLeft);
		pIniFile->WriteInteger("Window", "RxViewWidth", sys.m_RxViewWidth);
		pIniFile->WriteInteger("Window", "RxViewHeight", sys.m_RxViewHeight);
		pIniFile->WriteInteger("Window", "SyncView", KVS->Checked);
		pIniFile->WriteInteger("Window", "SyncViewTop", sys.m_SyncViewTop);
		pIniFile->WriteInteger("Window", "SyncViewLeft", sys.m_SyncViewLeft);
		pIniFile->WriteInteger("Window", "SyncViewWidth", sys.m_SyncViewWidth);
		pIniFile->WriteInteger("Window", "SyncViewHeight", sys.m_SyncViewHeight);
		pIniFile->WriteInteger("Window", "HistView", KVH->Checked);
		pIniFile->WriteInteger("Window", "HistViewTB", sys.m_HistViewTB);
		pIniFile->WriteInteger("Window", "HistViewTop", sys.m_HistViewTop);
		pIniFile->WriteInteger("Window", "HistViewLeft", sys.m_HistViewLeft);
		pIniFile->WriteInteger("Window", "HistViewWidth", sys.m_HistViewWidth);
		pIniFile->WriteInteger("Window", "HistViewHeight", sys.m_HistViewHeight);
		pIniFile->WriteInteger("Window", "HistViewSize", sys.m_HistViewSize);

		pIniFile->WriteInteger("Window", "CtrBtn", KVC->Checked);
		pIniFile->WriteInteger("Window", "CtrBtnTop", sys.m_CtrBtnTop);
		pIniFile->WriteInteger("Window", "CtrBtnLeft", sys.m_CtrBtnLeft);
		pIniFile->WriteInteger("Window", "CtrBtnWidth", sys.m_CtrBtnWidth);
		pIniFile->WriteInteger("Window", "CtrBtnHeight", sys.m_CtrBtnHeight);

		pIniFile->WriteInteger("Window", "StockViewTop", sys.m_StockViewTop);
		pIniFile->WriteInteger("Window", "StockViewLeft", sys.m_StockViewLeft);
		pIniFile->WriteInteger("Window", "StockViewWidth", sys.m_StockViewWidth);
		pIniFile->WriteInteger("Window", "StockViewHeight", sys.m_StockViewHeight);

		pIniFile->WriteInteger("Window", "WaterViewTop", sys.m_WaterViewTop);
		pIniFile->WriteInteger("Window", "WaterViewLeft", sys.m_WaterViewLeft);
		pIniFile->WriteInteger("Window", "WaterViewWidth", sys.m_WaterViewWidth);
		pIniFile->WriteInteger("Window", "WaterViewHeight", sys.m_WaterViewHeight);

		pIniFile->WriteInteger("Window", "LogViewTop", sys.m_LogViewTop);
		pIniFile->WriteInteger("Window", "LogViewLeft", sys.m_LogViewLeft);
		pIniFile->WriteInteger("Window", "LogViewWidth", sys.m_LogViewWidth);
		pIniFile->WriteInteger("Window", "LogViewHeight", sys.m_LogViewHeight);

		pIniFile->WriteInteger("Window", "DivMode", sys.m_DivMode);
		pIniFile->WriteInteger("Window", "DivSW", KVSE1->Checked | (KVSE2->Checked << 1) | (KVSE3->Checked << 2) );

		pIniFile->WriteInteger("FileView", "ShowSize", sys.m_FileViewShowSize);
		pIniFile->WriteInteger("FileView", "KeepA", sys.m_FileViewKeep);
		for( i = 0; i < 8; i++ ){
			sprintf(bf, "FileView%d", i+1);
			pIniFile->WriteInteger(bf, "Flag", sys.m_FileViewFlag[i]);
			pIniFile->WriteInteger(bf, "Mode", sys.m_FileViewMode[i]);
			pIniFile->WriteInteger(bf, "Size", sys.m_FileViewSize[i]);
			pIniFile->WriteInteger(bf, "Top", sys.m_FileViewTop[i]);
			pIniFile->WriteInteger(bf, "Left", sys.m_FileViewLeft[i]);
			pIniFile->WriteInteger(bf, "Width", sys.m_FileViewWidth[i]);
			pIniFile->WriteInteger(bf, "Height", sys.m_FileViewHeight[i]);
			pIniFile->WriteInteger(bf, "MaxPage", sys.m_FileViewMaxPage[i]);
			pIniFile->WriteInteger(bf, "CurPage", sys.m_FileViewCurPage[i]);
			for( int j = 0; j < sys.m_FileViewMaxPage[i]; j++ ){
				char bff[32];
				sprintf(bff, "FMPage%u", j+1);
				pIniFile->WriteInteger(bf, bff, sys.m_FileViewFMPage[i][j]);
				sprintf(bff, j ? "Folder%u" : "Folder", j+1);
				pIniFile->WriteString(bf, bff, sys.m_FileViewFolder[i][j]);
				sprintf(bff, "Name%u", j+1);
				pIniFile->WriteString(bf, bff, sys.m_FileViewName[i][j]);
			}
		}

		pIniFile->WriteInteger("Window", "PreViewTop", sys.m_PreViewTop);
		pIniFile->WriteInteger("Window", "PreViewLeft", sys.m_PreViewLeft);
		pIniFile->WriteInteger("Window", "PreViewWidth", sys.m_PreViewWidth);
		pIniFile->WriteInteger("Window", "PreViewHeight", sys.m_PreViewHeight);

		pIniFile->WriteInteger("Window", "StayOnTop", sys.m_StayOnTop);
		pIniFile->WriteInteger("Window", "MainPage", m_MainPage);
		pIniFile->WriteInteger("Window", "StockPage", TabS->TabIndex);
		pIniFile->WriteInteger("Window", "StockPicPosition", m_PSPage);
		for( i = 0; i < STOCKPAGE; i++ ){
			sprintf(bf, "StockTempPos%u", i + 1);
			pIniFile->WriteInteger("Window", bf, m_TSPage[i]);
		}
		pIniFile->WriteInteger("Window", "HQResizing", KSF->Checked);
		pIniFile->WriteInteger("Window", "UseTemplate", SBUseTemp->Down);
		pIniFile->WriteInteger("Window", "ShowSuperimpos", CBSTemp->Checked);
		pIniFile->WriteInteger("Window", "DraftSuperimpos", CBSDraft->Checked);
		pIniFile->WriteInteger("Window", "PicClipType", sys.m_PicClipType);
		pIniFile->WriteInteger("Window", "PicClipRatio", sys.m_PicClipRatio);
		pIniFile->WriteInteger("Window", "PicClipMode", sys.m_PicClipMode);
		pIniFile->WriteInteger("Window", "PicClipView", sys.m_PicClipView);
		pIniFile->WriteInteger("Window", "PicClipColor", sys.m_PicClipColor);
		pIniFile->WriteInteger("Window", "Way240", sys.m_Way240);
		pIniFile->WriteInteger("Window", "AutoMargin", KSAM->Checked);
		pIniFile->WriteInteger("Window", "TxClipType", m_TxClipType);
		pIniFile->WriteInteger("Window", "HistMax", sys.m_HistMax);
		pIniFile->WriteInteger("Window", "AutoRxClear", KRD->Checked);
		pIniFile->WriteInteger("Window", "AutoRxSave", CBASave->Checked);
		pIniFile->WriteInteger("Window", "AutoCopy", KEA->Checked);
		pIniFile->WriteInteger("Window", "JPEG", sys.m_JPEGQuality);
		pIniFile->WriteInteger("Window", "TempSuperimpos", KTBT->Checked);
		pIniFile->WriteInteger("Window", "DummyMacro", KTMD->Checked);
		pIniFile->WriteInteger("Window", "ShowTransPoint", KTTD->Checked);
		pIniFile->WriteInteger("Window", "AutoTransPoint", KTAP->Checked);
		pIniFile->WriteInteger("Window", "HistDDSize", sys.m_HDDSize);
		pIniFile->WriteInteger("Window", "ShowSizeRX", sys.m_ShowSizeRX);
		pIniFile->WriteInteger("Window", "ShowSizeHist", sys.m_ShowSizeHist);
		pIniFile->WriteInteger("Window", "ShowSizeTX", sys.m_ShowSizeTX);
		pIniFile->WriteInteger("Window", "ShowSizeTemp", sys.m_ShowSizeTemp);
		pIniFile->WriteInteger("Window", "ShowSizeStock", sys.m_ShowSizeStock);
		pIniFile->WriteInteger("Window", "StretchTXimage", KSIS->Checked);
		pIniFile->WriteInteger("Window", "SwitchRX", KSRR->Checked);

		pIniFile->WriteString("WindowFont", "Name", sys.m_WinFontName);
		pIniFile->WriteInteger("WindowFont", "Charset", sys.m_WinFontCharset);
		pIniFile->WriteInteger("WindowFont", "Style", sys.m_WinFontStyle);
		pIniFile->WriteInteger("WindowFont", "Adjust", sys.m_FontAdjSize);

		pIniFile->WriteInteger("Help", "UseNotePad", sys.m_HelpNotePad);
		pIniFile->WriteInteger("ImageAdj", "Sharp2D", sys.m_Sharp2D);

		pIniFile->WriteInteger("Define", "FFTType", sys.m_FFTType);
		pIniFile->WriteInteger("Define", "FFTGain", sys.m_FFTGain);
		pIniFile->WriteInteger("Define", "FFTResp", sys.m_FFTResp);
		pIniFile->WriteInteger("Define", "FFTStgv", sys.m_FFTStg);
		pIniFile->WriteInteger("Define", "FFTWidth", sys.m_FFTWidth);
		pIniFile->WriteInteger("Define", "FFTAGC", sys.m_FFTAGC);
		pIniFile->WriteInteger("Define", "FFTPriority", sys.m_FFTPriority);
		pIniFile->WriteInteger("Define", "WaterSW", KVW->Checked);
		pIniFile->WriteInteger("Define", "LevelType", pDem->m_LevelType);

		pIniFile->WriteInteger("Color", "WaterLow", sys.m_ColorLow);
		pIniFile->WriteInteger("Color", "WaterHigh", sys.m_ColorHigh);
		pIniFile->WriteInteger("Color", "FFTB", sys.m_ColorFFTB);
		pIniFile->WriteInteger("Color", "FFT", sys.m_ColorFFT);
		pIniFile->WriteInteger("Color", "FFTStg", sys.m_ColorFFTStg);
		pIniFile->WriteInteger("Color", "FFTSync", sys.m_ColorFFTSync);
		pIniFile->WriteInteger("Color", "FFTFreq", sys.m_ColorFFTFreq);
		pIniFile->WriteInteger("Color", "RxWindow", sys.m_ColorRXB);

		pIniFile->WriteInteger("Define", "UseRxBuff", sys.m_UseRxBuff);

		pIniFile->WriteInteger("Define", "SoundFifoRX", sys.m_SoundFifoRX);
		pIniFile->WriteInteger("Define", "SoundFifoTX", sys.m_SoundFifoTX);
		pIniFile->WriteInteger("Define", "SoundPriority", sys.m_SoundPriority);
		pIniFile->WriteString("Define", "SoundDevice", sys.m_SoundDevice);
		pIniFile->WriteInteger("Define", "SoundStereo", sys.m_SoundStereo);
		pIniFile->WriteInteger("Define", "StereoTX", sys.m_StereoTX);
		pIniFile->WriteString("Define", "SoundMMW", sys.m_SoundMMW);

		pIniFile->WriteString("Define", "PTT", sys.m_TxRxName);
		pIniFile->WriteInteger("Define", "PTTLock", sys.m_TxRxLock);
		pIniFile->WriteInteger("Define", "RTSonRX", sys.m_RTSonRX);

		pIniFile->WriteInteger("Define", "TXLoopBack", sys.m_echo);

		pIniFile->WriteInteger("Define", "ColorOffset", sys.m_DemOff);
		WriteDoubleIniFile(pIniFile, "Define", "ColorWhite", sys.m_DemWhite);
		WriteDoubleIniFile(pIniFile, "Define", "ColorBlack", sys.m_DemBlack);
		pIniFile->WriteInteger("Define", "ColorCalibration", sys.m_DemCalibration);
		for( i = 0; i < 17; i++ ){
			sprintf(bf, "Value%d", i);
			WriteDoubleIniFile(pIniFile, "Polynomial", bf, sys.m_Dem17[i]);
		}

		pIniFile->WriteInteger("Define", "FixedTxMode", sys.m_FixedTxMode);
		pIniFile->WriteInteger("Define", "SSTVMode", SSTVSET.m_TxMode);
		for( i = 0; i < 9; i++ ){
			sprintf(bf, "Button%d", i);
			pIniFile->WriteInteger("Mode", bf, m_ModeAssignTX[i]);
		}
		pIniFile->WriteInteger("Define", "RXFSKID", pDem->m_fskdecode);
		pIniFile->WriteInteger("Define", "TXFSKID", sys.m_TXFSKID);
		pIniFile->WriteInteger("Define", "CWID", sys.m_CWID);
		pIniFile->WriteInteger("Define", "CWIDWPM", sys.m_CWIDWPM);
		pIniFile->WriteInteger("Define", "CWIDFreq", sys.m_CWIDFreq);
		pIniFile->WriteString("Define", "CWIDText", sys.m_CWIDText);
		pIniFile->WriteString("Define", "MMVID", sys.m_MMVID);
		pIniFile->WriteInteger("Define", "TuneFreq", pMod->m_TuneFreq);
		pIniFile->WriteInteger("Define", "TuneTime", sys.m_TuneTXTime);
		pIniFile->WriteInteger("Define", "TuneSat", sys.m_TuneSat);
		pIniFile->WriteString("Define", "CWText", sys.m_CWText);

		pIniFile->WriteInteger("Define", "VOX", sys.m_VOX);
		CrLf2Yen(as, sys.m_VOXSound);
		pIniFile->WriteString("Define", "VOXTone", as);

		pIniFile->WriteInteger("Vari", "Enabled", pMod->m_VariOut);
		pIniFile->WriteInteger("Vari", "R", pMod->m_VariR);
		pIniFile->WriteInteger("Vari", "G", pMod->m_VariG);
		pIniFile->WriteInteger("Vari", "B", pMod->m_VariB);

		pIniFile->WriteInteger("Define", "TXGAIN", pMod->m_outgain);
		pIniFile->WriteInteger("Define", "TXBPF", pMod->m_bpf);
		pIniFile->WriteInteger("Define", "TXLPF", pMod->m_lpf);
		pIniFile->WriteInteger("Define", "TXBPFTAP", pMod->m_bpftap);
		pIniFile->WriteInteger("Define", "TXLPFFQ", pMod->m_lpffq);
		pIniFile->WriteInteger("Define", "DEMSLVL", pDem->m_SenseLvl);
		pIniFile->WriteInteger("Define", "Differentiator", sys.m_Differentiator);
		WriteDoubleIniFile(pIniFile, "Define", "DiffLevel", sys.m_DiffLevelP);

		pIniFile->WriteInteger("Define", "RXAFC", pDem->m_afc);
		pIniFile->WriteInteger("Define", "RXLMS", pSound->m_lms);
		pIniFile->WriteInteger("Define", "RXLMSAN", pSound->m_lms_AN);
		pIniFile->WriteInteger("Define", "DEMBPF", pDem->m_bpf);
		pIniFile->WriteInteger("Define", "SyncStart", pDem->m_MSync);
		pIniFile->WriteInteger("Define", "SyncReStart", pDem->m_SyncRestart);
		pIniFile->WriteInteger("Define", "AutoStop", sys.m_AutoStop);
		pIniFile->WriteInteger("Define", "AutoSync", sys.m_AutoSync);
		pIniFile->WriteInteger("Define", "SyncAccuracy", m_SyncAccuracy);
		pIniFile->WriteInteger("Define", "AutoSwirchRX", KRSW->Checked);
		pIniFile->WriteInteger("Define", "AutoSlant", KRSA->Checked);

		pIniFile->WriteInteger("Define", "DemType", pDem->m_Type);
		pIniFile->WriteInteger("Define", "fqcOutOrder", pDem->m_fqc.m_outOrder);
		WriteDoubleIniFile(pIniFile, "Define", "fqcOutFC", pDem->m_fqc.m_outFC);
		pIniFile->WriteInteger("Define", "fqcType", pDem->m_fqc.m_Type);
		WriteDoubleIniFile(pIniFile, "Define", "fqcSmooth", pDem->m_fqc.m_SmoozFq);

		WriteDoubleIniFile(pIniFile, "Define", "pllVcoGain", pDem->m_pll.m_vcogain);
		pIniFile->WriteInteger("Define", "pllLoopOrder", pDem->m_pll.m_loopOrder);
		WriteDoubleIniFile(pIniFile, "Define", "pllLoopFC", pDem->m_pll.m_loopFC);
		pIniFile->WriteInteger("Define", "pllOutOrder", pDem->m_pll.m_outOrder);
		WriteDoubleIniFile(pIniFile, "Define", "pllOutFC", pDem->m_pll.m_outFC);

		pIniFile->WriteString("Define", "Call", sys.m_Call);

		pIniFile->WriteInteger("Log", "TempDelay", sys.m_TempDelay);
		pIniFile->WriteInteger("Log", "AutoUTCOffset", sys.m_AutoTimeOffset);
		pIniFile->WriteInteger("Log", "UTCOffset", sys.m_TimeOffset);
		pIniFile->WriteInteger("Log", "UTCOffsetMin", sys.m_TimeOffsetMin);
		pIniFile->WriteInteger("Log", "LogLink", sys.m_LogLink);
		pIniFile->WriteInteger("Log", "LinkPoll", LogLink.IsPolling());
		pIniFile->WriteInteger("Log", "LinkPTT", LogLink.GetPTTEnabled());
		LogLink.SaveMMLink(pIniFile);

		pIniFile->WriteString("LogFile", "Name", Log.m_FileName);

		for( i = 0; i < 8; i++ ){
			sprintf(bf, "Program%d", i+1);
			pIniFile->WriteInteger(bf, "Flag", sys.m_ExtMode[i]);
			pIniFile->WriteString(bf, "Name", sys.m_ExtName[i]);
			pIniFile->WriteString(bf, "File", sys.m_ExtCmd[i]);
		}

		pIniFile->WriteInteger("Dir", "HistoryFormat", RxHist.m_UseJPEG);
		pIniFile->WriteString("Dir", "History", HistDir);
		pIniFile->WriteInteger("Dir", "StockFormat", sys.m_UseJPEG);
		pIniFile->WriteString("Dir", "Stock", StockDir);
		pIniFile->WriteString("Dir", "RecSound", RecDir);
		pIniFile->WriteString("Dir", "Bitmap", BitmapDir);
		pIniFile->WriteString("Dir", "SaveBitmap", SBitmapDir);
		pIniFile->WriteString("Dir", "Template", TemplateDir);
		pIniFile->WriteString("Dir", "AutoSaveDir", sys.m_AutoSaveDir);
		pIniFile->WriteInteger("Dir", "AutoSave", KHAC->Checked);
		pIniFile->WriteInteger("Dir", "AutoSaveTime", KHACT->Checked);
		pIniFile->WriteInteger("Dir", "AutoSaveFormat", KHACJ->Checked);

		pIniFile->WriteString("Dir", "ExtLog", ExtLogDir);
		for( i = 0; i < TEXTCONVMAX; i++ ){
			sprintf(bf, "WDEF%d", i+1);
			pIniFile->WriteString("ExtConv", bf, LogText.m_tConv[i].Key);
			sprintf(bf, "WLEN%d", i+1);
			pIniFile->WriteInteger("ExtConv", bf, LogText.m_tConv[i].w);
		}
		for( i = 0; i < TEXTCONVMAX; i++ ){
			sprintf(bf, "RDEF%d", i+1);
			pIniFile->WriteString("ExtConv", bf, LogText.m_rConv[i].Key);
			sprintf(bf, "RLEN%d", i+1);
			pIniFile->WriteInteger("ExtConv", bf, LogText.m_rConv[i].w);
		}

		for( i = 0; i < RecentMenu.GetCount(); i++ ){
			sprintf(bf, "File%d", i + 1);
			pIniFile->WriteString("Recent File", bf, RecentMenu.GetItemText(i));
		}
		Log.WriteIniFile("LogSet", pIniFile);

		SaveRADIOSetup(pIniFile);

		for( i = 0; i < 6; i++ ){
			sprintf(bf, "Color%d", i+1);
			pIniFile->WriteInteger("DrawText", bf, sys.m_ColText[i]);
		}
		for( i = 0; i < 4; i++ ){
			sprintf(bf, "Color%d", i+1);
			pIniFile->WriteInteger("DrawBar", bf, sys.m_ColBar[i]);
		}
		for( i = 0; i < 32; i++ ){
			sprintf(bf, "Color%d", i+1);
			pIniFile->WriteInteger("ColorSet", bf, sys.m_ColorSet[i]);
		}

		for( i = 0; i < 16; i++ ){
			sprintf(bf, "Text%d", i+1);
			pIniFile->WriteString("TextList", bf, sys.m_TextList[i]);
		}

		pIniFile->WriteInteger("Text", "Grade", sys.m_TextGrade);
		pIniFile->WriteInteger("Text", "Shadow", sys.m_TextShadow);
		pIniFile->WriteInteger("Text", "Zero", sys.m_TextZero);
		pIniFile->WriteInteger("Text", "Rot", sys.m_TextRot);
		pIniFile->WriteInteger("Text", "Vert", sys.m_TextVert);
		pIniFile->WriteInteger("Text", "VertH", sys.m_TextVertH);
		pIniFile->WriteInteger("Text", "3D", sys.m_TextStack);
		CrLf2Yen(as, sys.m_Msg);
		pIniFile->WriteString("Text", "MSG", as);
		pIniFile->WriteInteger("Text", "DisFontSmooth", sys.m_DisFontSmooth);
		pIniFile->WriteInteger("Text", "Temp24", sys.m_Temp24 | (sys.m_BitPixel << 16));

		pIniFile->WriteInteger("Pic", "Shape", sys.m_PicShape);
		pIniFile->WriteInteger("Pic", "Adjust", sys.m_PicAdjust);
		pIniFile->WriteInteger("Pic", "LineStyle", sys.m_PicLineStyle);
		pIniFile->WriteInteger("Pic", "LineColor", sys.m_PicLineColor);

		pIniFile->WriteInteger("PicSel", "RTM", sys.m_PicSelRTM);
		pIniFile->WriteInteger("PicSel", "Smooz", sys.m_PicSelSmooz);
		pIniFile->WriteInteger("PicSel", "Color", sys.m_PicSelCurCol);

		pIniFile->WriteInteger("ProFile", "Version", PROVER);
		PRODEM *pPD = m_DemPro;
		for( i = 0; i < 8; i++, pPD++ ){
			sprintf(bf, "ProFile%d", i+1);
			pIniFile->WriteString(bf, "Name", pPD->Name);
			WriteDoubleIniFile(pIniFile, bf, "VcoGain", pPD->VCOGain);
			pIniFile->WriteInteger(bf, "loopOrder", pPD->loopOrder);
			WriteDoubleIniFile(pIniFile, bf, "loopFC", pPD->loopFC);
			pIniFile->WriteInteger(bf, "OutOrder", pPD->OutOrder);
			WriteDoubleIniFile(pIniFile, bf, "OutFC", pPD->OutFC);
			pIniFile->WriteInteger(bf, "DemType", pPD->Type);
			pIniFile->WriteInteger(bf, "fqcOutOrder", pPD->crossOutOrder);
			WriteDoubleIniFile(pIniFile, bf, "fqcOutFC", pPD->crossOutFC);
			pIniFile->WriteInteger(bf, "fqcType", pPD->crossType);
			WriteDoubleIniFile(pIniFile, bf, "fqcSmooth", pPD->crossSmooz);

			WriteDoubleIniFile(pIniFile, bf, "DemOff", pPD->DemOff);
			WriteDoubleIniFile(pIniFile, bf, "DemWhite", pPD->DemWhite);
			WriteDoubleIniFile(pIniFile, bf, "DemBlack", pPD->DemBlack);

			pIniFile->WriteInteger(bf, "DemCalibration", pPD->DemCalibration);
			char bff[64];
			double *dp = pPD->Dem17;
			for( int j = 0; j < 17; j++ ){
				sprintf(bff, "DemPolynomial%d", j);
				WriteDoubleIniFile(pIniFile, bf, bff, *dp++);
			}

			pIniFile->WriteInteger(bf, "Differentiator", pPD->Differentiator);
			WriteDoubleIniFile(pIniFile, bf, "DiffLevel", pPD->DiffLevel);
		}
		for( i = 0; i < PopupC->Items->Count; i++ ){
			sprintf(bf, "Item%d", i+1);
			char bbf[64];
			StrCopy(bbf, AnsiString(PopupC->Items->Items[i]->Caption).c_str(), 63);	//ja7ude 0428
			LPSTR p;
			for( p = bbf; *p; p++ ){
				if( *p == '\t' ) *p = '_';
			}
			pIniFile->WriteString("Calls", bf, bbf);
		}
// Fonts
		for( i = 0; i < 4; i++ ){
			sprintf(bf, "Font.%d", i+1);
            if( sys.m_RegFont[i].m_Height ){
		        pIniFile->WriteString(bf, "Name", sys.m_RegFont[i].m_Name);
		        pIniFile->WriteInteger(bf, "Charset", sys.m_RegFont[i].m_Charset);
		        pIniFile->WriteInteger(bf, "Height", sys.m_RegFont[i].m_Height);
		        pIniFile->WriteInteger(bf, "Style", sys.m_RegFont[i].m_Style);
            }
	    }
//CWMenu
		try{
			pIniFile->EraseSection("CWMenu");
        }
        catch(...){
        }
		pIniFile->WriteInteger("CWMenu", "Menus", sys.m_nCWMenu);
		for( i = 0; i < sys.m_nCWMenu; i++ ){
			sprintf(bf, "M%d", i+1);
			pIniFile->WriteString("CWMenu", bf, sys.m_CWMenu[i]);
	    }

//RadioMenu
		try{
			pIniFile->EraseSection("RadioMenu");
        }
        catch(...){
        }
	    pIniFile->WriteInteger("RadioMenu", "Menus", m_nRadioMenu);
		for( i = 0; i < m_nRadioMenu; i++ ){
			sprintf(bf, "Cap%d", i+1);
	        pIniFile->WriteString("RadioMenu", bf, m_RadioMenu[i].strTTL);
			sprintf(bf, "Cmd%d", i+1);
	        pIniFile->WriteString("RadioMenu", bf, m_RadioMenu[i].strCMD);
	    }

// リピータ
		if( sys.m_Repeater ){
			pIniFile->WriteInteger("Repeater", "Enabled", pDem->m_Repeater);
			pIniFile->WriteInteger("Repeater", "Tone", pDem->m_RepTone);
			pIniFile->WriteInteger("Repeater", "SQLVL", pDem->m_RepSQ);
			pIniFile->WriteInteger("Repeater", "Sense", sys.m_RepSenseLvl);
			pIniFile->WriteInteger("Repeater", "TimeA", sys.m_RepTimeA);
			pIniFile->WriteInteger("Repeater", "TimeB", sys.m_RepTimeB);
			pIniFile->WriteInteger("Repeater", "TimeC", sys.m_RepTimeC);
			pIniFile->WriteInteger("Repeater", "TimeD", sys.m_RepTimeD);
			pIniFile->WriteString("Repeater", "AnsCW", sys.m_RepAnsCW);
			pIniFile->WriteInteger("Repeater", "Beacon", sys.m_RepBeacon);
			pIniFile->WriteInteger("Repeater", "BeaconMode", sys.m_RepBeaconMode);
			pIniFile->WriteInteger("Repeater", "BeaconFilter", sys.m_RepBeaconFilter);
			pIniFile->WriteString("Repeater", "TempReplay", sys.m_RepTempTX);
			pIniFile->WriteString("Repeater", "TempBeacon", sys.m_RepTempBeacon);
			pIniFile->WriteInteger("Repeater", "BottomAdj", sys.m_RepBottomAdj);
			pIniFile->WriteInteger("Repeater", "QuietnessTime", sys.m_RepQuietnessTime);
			pIniFile->WriteString("Repeater", "Folder", sys.m_RepFolder);
		}
		pIniFile->UpdateFile();
		delete pIniFile;

	}
	catch(...){
		ErrorMB(MsgEng?"Cannot update MMSSTV.INI":"MMSSTV.INIを更新できません.");
	}
	sprintf(bf, "%s"TEMPV106C, StockDir);
	SaveTemplate(&DrawMain, bf);
	sprintf(bf, "%s"TEMPV106L, StockDir);
	SaveTemplate(&DrawText, bf);
	sprintf(bf, "%sCurrent.bmp", StockDir);
	SaveBitmap(pBitmapTXM, bf);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateTitle(void)
{
	char bbf[64];
	char bf[512];
	bf[0] = 0;
	if( IsCall(sys.m_Call.c_str()) ){
		sprintf(bf, "%s (%s) - ", sys.m_Call.c_str(), Log.GetName() );
	}
	strcat(bf, "MMSSTV ");
	if( sys.m_Repeater && (pSound != NULL) && pDem->m_Repeater ){
		strcat(bf, "Repeater ");
	}
	strcat(bf, VERID VERBETA);
	if( SampType ){
		sprintf(bbf, " [based on %.lfHz]", SampBase);
		strcat(bf, bbf);
	}
	if( sys.m_BitPixel < 24 ){
		sprintf(bbf, " [%u colors]", int(pow(2, sys.m_BitPixel)));
		strcat(bf, bbf);
	}
	if( sys.m_bCQ100 ){
		strcat(bf, " for Internet");
    }
	Caption = bf;
	UpdateCallsign();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateLMS(void)
{
	SBLMS->Down = pSound->m_lms;
	int an = pSound->m_lms_AN;
	if( an ){
		SBLMS->Caption = (an == 1) ? "ANF" : "ANS";
	}
	else {
		SBLMS->Caption = "LMS";
	}
	SBLMS->Font->Color = an ? clRed : clBlack;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AdjustFocus(void)
{
	Page->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::FormPaint(TObject *Sender)
{
	if( m_InitFirst ){
		m_InitFirst = 0;
		int f1stInst = FALSE;
		if( (sys.m_Call == "NOCALL") || sys.m_Call.IsEmpty() ){
            f1stInst = TRUE;
			AnsiString as = "";
			if( sys.m_Call != "NOCALL" ){
				as = sys.m_Call;
			}
			if( InputMB("MMSSTV", MsgEng ? "Please enter your callsign":"あなたのコールサインを入力して下さい.", as) == TRUE ){
				if( !as.IsEmpty() ){
					char bf[MLCALL+1];
					StrCopy(bf, as.c_str(), MLCALL);
					jstrupr(bf);
					sys.m_Call = bf;
					Log.MakePathName(ClipCall(bf));
				}
			}
		}
		char bf[256];
		sprintf(bf, "%sARRL.DX", BgnDir);
		Cty.Load(bf);
		sprintf(bf, "%sMMCG.DEF", BgnDir);
		mmcg.LoadDef(bf);

		if( !f1stInst ) Log.DoBackup();
		Log.Open(NULL, !f1stInst);
		LogLink.UpdateLink(sys.m_LogLink);
		UpdateTextData();
		if( Log.m_sd.btime ) SBQSO->Down = TRUE;
		UpdateTitle();
		OpenCloseCom();
		if( sys.m_UseRxBuff == 2 ){
			WaveStg.Open();
		}
		if( pSound != NULL ){
			pDem->OpenCloseRxBuff();
			pSound->Resume();
		}
		sprintf(bf, "%sCurrent.bmp", StockDir);
		if( IsFile(bf) ){
			LoadBitmap(pBitmapTXM, bf);
			pBitmapTX->Width = pBitmapTXM->Width;
			pBitmapTX->Height = pBitmapTXM->Height;
		}
		else {
			FillBitmap(pBitmapTXM, sys.m_PicClipColor);
		}
		ChangeTxMode(SSTVSET.m_TxMode);
		if( m_TxSPage == -1 ) TabSChange(NULL);
		m_ChangeTemp = 1;
		SBULog->Enabled = TRUE;
		UpdatePic();
		UpdateUI();
		if( KVS->Checked ) KVSClick(NULL);
		for( int i = 0; i < 8; i++ ){
			if( sys.m_FileViewFlag[i] ){
				ShowFileView(i, 1);
			}
		}
		if( KVH->Checked ) KVHClick(NULL);
		if( KVR->Checked ) KVRClick(NULL);
		if( KVC->Checked ) KVCClick(NULL);
		if( KSF->Checked ) m_ReqHistF = 10;
		DisPaint = 0;
		AdjustFocus();
	}
	else if( pSound != NULL ){
		if( (pSound->m_susp == 1) && m_SuspMinimized ) KXRClick(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::EntryAlign(void)
{
	RECT	rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = 792-1;
	rc.bottom = 548-1;

	AlignFix.EntryControl(GBMode, &rc, GBMode->Font);
	AlignFix.EntryControl(GB1, &rc, GB1->Font);
	AlignFix.EntryControl(PanelLvl, &rc, NULL);
	AlignFix.EntryControl(PanelFFT, &rc, NULL);
	AlignFix.EntryControl(PanelWater, &rc, NULL);
	AlignFix.EntryControl(GBLog, &rc, GBLog->Font);

	AlignMode.EntryControl(SBAuto, GBMode, NULL);
	AlignMode.EntryControl(SBM1, GBMode, NULL);
	AlignMode.EntryControl(SBM2, GBMode, NULL);
	AlignMode.EntryControl(SBM3, GBMode, NULL);
	AlignMode.EntryControl(SBM4, GBMode, NULL);
	AlignMode.EntryControl(SBM5, GBMode, NULL);
	AlignMode.EntryControl(SBM6, GBMode, NULL);
	AlignMode.EntryControl(SBM7, GBMode, NULL);
	AlignMode.EntryControl(SBM8, GBMode, NULL);
	AlignMode.EntryControl(SBM9, GBMode, NULL);

	AlignDsp.EntryControl(SBAFC, GB1, NULL);
	AlignDsp.EntryControl(SBLMS, GB1, NULL);

	AlignLog.EntryControl(L1, GBLog, NULL);
	AlignLog.EntryControl(L2, GBLog, NULL);
	AlignLog.EntryControl(L3, GBLog, NULL);
	AlignLog.EntryControl(L4, GBLog, NULL);
	AlignLog.EntryControl(L5, GBLog, NULL);
	AlignLog.EntryControl(L6, GBLog, NULL);
	AlignLog.EntryControl(L7, GBLog, NULL);
	AlignLog.EntryControl(HisCall, GBLog, NULL);
	AlignLog.EntryControl(HisRST, GBLog, NULL);
	AlignLog.EntryControl(MyRST, GBLog, NULL);
	AlignLog.EntryControl(HisName, GBLog, NULL);
	AlignLog.EntryControl(HisQTH, GBLog, NULL);
	AlignLog.EntryControl(EditNote, GBLog, NULL);
	AlignLog.EntryControl(EditQSL, GBLog, NULL);
	AlignLog.EntryControl(SBULog, GBLog, NULL);
	AlignLog.EntryControl(SBRXID, GBLog, NULL);
	AlignLog.EntryControl(SBTXID, GBLog, NULL);
	AlignLog.EntryControl(SBQSO, GBLog, NULL);
	AlignLog.EntryControl(SBData, GBLog, NULL);
	AlignLog.EntryControl(SBFind, GBLog, NULL);
	AlignLog.EntryControl(SBInit, GBLog, NULL);
	AlignLog.EntryControl(SBList, GBLog, NULL);
	AlignLog.EntryControl(LogFreq, GBLog, NULL);

	AlignPCS.EntryControl(CBSTemp, PCS, CBSTemp->Font);
	AlignPCS.EntryControl(CBSDraft, PCS, CBSDraft->Font);
	AlignPCS.EntryControl(LS, PCS, LS->Font);
	AlignPCS.EntryControl(UDStock, PCS, NULL);

	int CX = ::GetSystemMetrics(SM_CXFULLSCREEN);
	int CY = ::GetSystemMetrics(SM_CYFULLSCREEN);
//	int CX = ::GetSystemMetrics(SM_CXSCREEN);
//	int CY = ::GetSystemMetrics(SM_CYSCREEN);
	if( (CX < Width)||(CY < Height) ){
		Top = 0;
		Left = 0;
		Width = CX;
		Height = CY;
	}
	if( CY < 500 ){
		TabS->TabHeight = 20;
	}
	FormCenter(this, CX, CY);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AdjustStockView(void)
{
	if( pStockView == NULL ){
		TabS->Height = ClientHeight - Page->Height;
		PCS->Left = GBMode->Left;
		PCS->Top = TabS->Top;
		PCS->Height = TabS->TabHeight;
		PCS->Width = ClientWidth - PCS->Left;
		AlignPCS.NewAlign(PCS);
	}
	else {
		if( !pStockView->Visible ) return;
		PCS->Left = pStockView->ClientWidth - PCS->Width;
//        AlignPCS.NewAlign(PCS);
	}
	MultProc();
	double h = PBoxS->ClientHeight;
	double w = PBoxS->ClientWidth;
	double k = w / h;
	int smax = m_SMax;
	if( k <  3.2 ){
		m_SMax = 18;
	}
	else if( k <  6.0 ){
		m_SMax = 12;
	}
	else {
		m_SMax = 6;
	}
	MultProc();
	if( pBitmapSS != NULL ){
		delete pBitmapSS;
		pBitmapSS = NULL;
	}
	for( int i = 0; i < STOCKPAGE; i++ ){
		if( pBitmapST[i] != NULL ){
			delete pBitmapST[i];
			pBitmapST[i] = NULL;
		}
	}
	if( smax != m_SMax ){
		m_DragPMax = STOCKMAX / m_SMax;
		if( STOCKMAX % m_SMax ) m_DragPMax++;
		if( UDStock->Position >= m_DragPMax ){
			UDStock->Position = 0;
		}
		UDStock->Max = SHORT(m_DragPMax - 1);
		if( !m_InitFirst ){
			m_TxSPage = -1;
			TabSChange(NULL);
		}
	}
	char bf[64];
	sprintf(bf, "%u/%u", UDStock->Position + 1, m_DragPMax);
	LS->Caption = bf;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::FormResize(TObject *Sender)
{
	if( Timer->Enabled == FALSE ) return;

	if( WindowState != wsMinimized ){
		int n = (WindowState == wsNormal) ? 0 : 1;
		if( pStockView != NULL ){
			if( !m_ClientHeight[n] ){
				m_ClientHeight[n] =  ClientHeight;
			}
			if( ClientHeight != Page->Height ){
				ClientHeight = Page->Height;
			}
		}
		else if( m_ClientHeight[n] ){
			ClientHeight = m_ClientHeight[n];
			m_ClientHeight[n] = 0;
		}

		if( pWaterView != NULL ){
			if( !m_ClientWidth[n] ){
				m_ClientWidth[n] =  ClientWidth;
			}
			if( ClientWidth != GBMode->Left + GBMode->Width ){
				ClientWidth = GBMode->Left + GBMode->Width;
			}
		}
		else if( m_ClientWidth[n] ){
			ClientWidth = m_ClientWidth[n];
			m_ClientWidth[n] = 0;
		}
	}

	CWaitCursor wait;
	if( pWaterView == NULL ){
		AlignFix.NewFixAlign(this, GBMode->Left); MultProc();
	}
	else {
		PanelLvl->Left = 0;
		int w = 30;
		PanelLvl->Width = w;
		PanelFFT->Left = w;
		PanelWater->Left = w;
		w = pWaterView->ClientWidth - PanelLvl->Width;
		PanelFFT->Width = w;
		PanelWater->Width = w;

		w = pWaterView->ClientHeight - PBoxG->Height;
		PanelLvl->Height = w;
		PanelFFT->Height = w * 24 / 35;
		PanelWater->Top = PanelFFT->Top + PanelFFT->Height;
		PanelWater->Height = pWaterView->ClientHeight - PanelFFT->Height - PanelFFT->Top;
		HisCall->ParentFont = TRUE;
	}
	AlignMode.NewAlign(GBMode); MultProc();
	AlignDsp.NewAlign(GB1); MultProc();
	AlignLog.NewAlign(GBLog); MultProc();
//    UDStock->Left = PCS->Width - UDStock->Width;
//    LS->Left = UDStock->Left - LS->Width - 5;
	PanelFFT->Height = KVW->Checked ? PanelLvl->Height - PanelWater->Height : PanelLvl->Height;
	if( pBitmapLvl != NULL ){
		delete pBitmapLvl;
		pBitmapLvl = new Graphics::TBitmap();
		pBitmapLvl->Width = PBoxLvl->Width;
		pBitmapLvl->Height = PBoxLvl->Height;
		MultProc();
		delete pBitmapFFT;
		pBitmapFFT = new Graphics::TBitmap();
		pBitmapFFT->Width = PBoxFFT->Width > DFFTSIZE ? DFFTSIZE : PBoxFFT->Width;
		pBitmapFFT->Height = PBoxFFT->Height;
		delete pBitmapWater;
		MultProc();
		pBitmapWater = NULL;
		if( KVW->Checked ){
			pBitmapWater = CreateBitmap(PBoxWater->Width > DFFTSIZE ? DFFTSIZE : PBoxWater->Width, PBoxWater->Height, pf24bit);
			PanelWater->Visible = TRUE;
		}
		else {
			PanelWater->Visible = FALSE;
		}
		if( pSound != NULL ){
			MultProc();
			InitDrawFFT();
			DrawFFT(1);
			if( KVW->Checked ) DrawWater(1);
		}
	}
	PBoxG->Width = PBoxFFT->Width;
	PBoxG->Left = PanelFFT->Left + (PanelFFT->Width - PBoxFFT->Width)/2;
	PBoxG->Top = 0;
	PBoxG->Height = PanelFFT->Top - 1;

	if( pStockView != NULL ) pStockView->Visible = KVSE1->Checked;
	if( pWaterView != NULL ) pWaterView->Visible = KVSE2->Checked;
	if( pLogView != NULL ) pLogView->Visible = KVSE3->Checked;

	AdjustStockView();
	m_MW = Height - ClientHeight;
}
//---------------------------------------------------------------------------
// 現在の論理パレットを返す（TControl::GetPaletteのオーバライド関数）
HPALETTE __fastcall TMmsstv::GetPalette(void)
{
	sys.m_Palette = 1;
	switch(m_MainPage){
		case pgRX:
			return pBitmapRX->Palette;
		case pgHist:
			return pBitmapHist->Palette;
		case pgTX:
			return pBitmapTX->Palette;
		case pgTemp:
			return pBitmapTemp->Palette;
		default:
			return NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::CloseFileView(int i)
{
	TFileViewDlg *fp = pFileView[i];
	sys.m_FileViewMaxPage[i] = fp->m_MaxPage;
	sys.m_FileViewCurPage[i] = fp->m_CurPage;
	for( int j = 0; j < fp->m_MaxPage; j++ ){
		CFILEV *cp = fp->pFileV[j];
		sys.m_FileViewFolder[i][j] = cp->m_Folder.c_str();
		sys.m_FileViewName[i][j] = cp->m_Name.c_str();
		sys.m_FileViewFMPage[i][j] = cp->m_CurPage | (cp->m_UseIndex ? 0x80000000 : 0) | (cp->m_Type << 16);
	}
	sys.m_FileViewFlag[i] = 0;
	sys.m_FileViewSize[i] = fp->m_SSize | (fp->KBP->Checked ? 0x00010000 : 0);
	sys.m_FileViewMode[i] = fp->SBMode->Down;
	fp->GetViewPos(sys.m_FileViewLeft[i], sys.m_FileViewTop[i], sys.m_FileViewWidth[i], sys.m_FileViewHeight[i]);
	delete fp;
	pFileView[i] = NULL;
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AdjustFileView(void)
{
	int i;
	int f = FALSE;
	for( i = 0; i < 8; i++ ){
		if( (pFileView[i] != NULL) && (!pFileView[i]->Visible) && (!pFileView[i]->m_Suspend) ){
			CloseFileView(i);
		}
		if( pFileView[i] == NULL ) f = TRUE;
	}
	KVF->Enabled = f;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LoadAllFileView(void)
{
	for( int i = 0; i < 8; i++ ){
		if( pFileView[i] != NULL ){
			pFileView[i]->Reset();
			pFileView[i]->LoadImage();
			MultProc();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::ReOpenFileView(TFileViewDlg *pView)
{
	for( int i = 0; i < 8; i++ ){
		if( pFileView[i] == pView ){
			CWaitCursor w;
			CloseFileView(i);
			ShowFileView(i, 0);
			break;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::InitDrawFFT(void)
{
	GetFFTRect(m_FM, m_low);

	m_XW = pBitmapFFT->Width;
	if( m_XW > DFFTSIZE ) m_XW = DFFTSIZE;
	m_FX[0] = int(((1900-m_low)*FFT_SIZE*double(m_XW)/double(FFTSamp*m_FM)) + 0.5);
	m_FX[1] = int(((1500-m_low)*FFT_SIZE*double(m_XW)/double(FFTSamp*m_FM)) + 0.5);
	m_FX[2] = int(((2300-m_low)*FFT_SIZE*double(m_XW)/double(FFTSamp*m_FM)) + 0.5);
	m_FX[3] = int(((1201-m_low)*FFT_SIZE*double(m_XW)/double(FFTSamp*m_FM)) + 0.5);
	m_FX[6] = int(((2044-m_low)*FFT_SIZE*double(m_XW)/double(FFTSamp*m_FM)) + 0.5);
	if( pSound != NULL ){
		int fq;
		if( sys.m_Repeater ){
			fq = pDem->m_RepTone;
			m_FX[4] = int(((fq-m_low)*FFT_SIZE*double(m_XW)/double(FFTSamp*m_FM)) + 0.5);
		}
		fq = pSound->notch.m_freq;
		m_FX[5] = int(((fq-m_low)*FFT_SIZE*double(m_XW)/double(FFTSamp*m_FM)) + 0.5);
	}
	m_of = m_low * FFT_SIZE / FFTSamp;
	InitFFT();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::InitFFT(void)
{
	memset(m_FFTMAX, 0, sizeof(m_FFTMAX));
	memset(m_fft, 0, sizeof(m_fft));

	m_FFTMax = 512;
	m_FFTSumMax = m_FFTMax*4;

	m_WaterMax = 128;
	m_WaterSumMax = m_WaterMax*8;
	m_WaterMin = 0;

	m_FFTFlag = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::CalcFFT(void)
{
	CFFT *fp = &pSound->fftIN;
	if( fp->IsData() ){
		switch(sys.m_FFTGain){
			case 0:
				fp->CalcFFT(m_FFTWINDOW, 30.0, sys.m_FFTResp);
				break;
			case 1:
				fp->CalcFFT(m_FFTWINDOW, 34.0, sys.m_FFTResp);
				break;
			case 2:
				fp->CalcFFT(m_FFTWINDOW, 42.0, sys.m_FFTResp);
				break;
			case 3:
				fp->CalcFFT(m_FFTWINDOW, 54.0, sys.m_FFTResp);
				break;
			default:
				if( sys.m_FFTType == 2 ){
					fp->CalcFFT(m_FFTWINDOW, 10.0, sys.m_FFTResp);
				}
				else {
					fp->CalcFFT(m_FFTWINDOW, (SBTX->Down && (sys.m_echo != 2)) ? 0.5 : 10.0, sys.m_FFTResp);
				}
				break;
		}
		int *ip = m_fft;
		int i, d;
		if( (sys.m_FFTType == 1) && (g_dblToneOffset < -10.0) ){
			int of = (m_low + g_dblToneOffset) * FFT_SIZE / FFTSamp;
            int n;
			for( i = 0; i < m_XW; i++, ip++ ){
				n = (i * m_FM)/m_XW + of;
				if( n >= 0 ){
					d = fp->m_fft[n];
                }
                else {
					d = 0;
                }
				if( !sys.m_FFTPriority || (d > *ip) ){
					*ip = d;
				}
			}
		}
		else {
			if( sys.m_FFTPriority ){
				for( i = 0; i < m_XW; i++, ip++ ){
					d = fp->m_fft[(i * m_FM)/m_XW + m_of];
					if( d > *ip ) *ip = d;
				}
			}
			else {
				for( i = 0; i < m_XW; i++, ip++ ){
					*ip = fp->m_fft[(i * m_FM)/m_XW + m_of];
				}
			}
		}
		m_FFTFlag++;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DrawFFT(int sw)
{
	TCanvas *tp = pBitmapFFT->Canvas;
	TRect rc;
	int XR = pBitmapFFT->Width - 1;
	int YB = pBitmapFFT->Height - 1;
	rc.Left = 0;
  rc.Top = 0;
	rc.Right = XR;
	rc.Bottom = pBitmapFFT->Height;
	tp->Brush->Color = sys.m_ColorFFTB;
	tp->FillRect(rc);

	int i, x;
	char bf[64];
	if( pSound->m_Err || pDem->m_OverFlow ){
		if( pSound->m_Err || !SBTX->Down ){
			tp->Font->Color = clWhite;
			tp->Font->Height = -16;
			strcpy(bf, pSound->m_Err ? "Lost sound" : "Overflow");
			x = tp->TextWidth(bf);
			tp->TextOut(XR - x - 3, 0, bf);
		}
		pDem->m_OverFlow = 0;
		if( pSound->m_Err ) pSound->m_Err--;
	}
	else if( pSound->WaveFile.m_mode ){
		tp->Font->Color = clWhite;
		tp->Font->Height = -16;
		LPCSTR p = "";
		if( pSound->WaveFile.m_pause ){
			p = "Pause";
		}
		else {
			switch(pSound->WaveFile.m_mode){
				case 1:
					p = "Play";
					break;
				case 2:
					p = "Rec.";
					break;
			}
		}
		if( *p ){
			x = tp->TextWidth(p);
			tp->TextOut(XR - x - 3, 0, p);
		}
	}
	if( pDem->m_Tick ){
		tp->Pen->Color = clLime;
		x = int(((pDem->m_TickFreq-m_low)*FFT_SIZE*double(m_XW)/double(FFTSamp*m_FM)) + 0.5);
		tp->MoveTo(x, 0);
		tp->LineTo(x, YB);
	}
	else {
		int fNarrow = FALSE;
        if( SBTX->Down ){
			fNarrow = SSTVSET.m_fTxNarrow;
        }
        else if( pDem->m_Sync ){
			fNarrow = pDem->m_fNarrow;
        }
		if( fNarrow ){
			tp->Pen->Color = sys.m_ColorFFTSync;
			tp->Pen->Style = psSolid;
			tp->MoveTo(m_FX[0], 0);		// 1900
			tp->LineTo(m_FX[0], YB);
			tp->Pen->Color = sys.m_ColorFFTFreq;
			tp->Pen->Style = psDot;
			tp->MoveTo(m_FX[3], 0);		// 1200
			tp->LineTo(m_FX[3], YB);
			tp->MoveTo(m_FX[1], 0);		// 1500
			tp->LineTo(m_FX[1], YB);
			tp->MoveTo(m_FX[6], 0);		// 2044
			tp->LineTo(m_FX[6], YB);
        }
        else {
			tp->Pen->Color = sys.m_ColorFFTSync;
			tp->Pen->Style = psSolid;
			tp->MoveTo(m_FX[3], 0);		// 1200
			tp->LineTo(m_FX[3], YB);
			tp->Pen->Color = sys.m_ColorFFTFreq;
			tp->Pen->Style = psDot;
			tp->MoveTo(m_FX[0], 0);		// 1900
			tp->LineTo(m_FX[0], YB);
			tp->Pen->Style = psSolid;
			tp->MoveTo(m_FX[1], 0);		// 1500
			tp->LineTo(m_FX[1], YB);
        }
		tp->Pen->Style = psSolid;
		tp->MoveTo(m_FX[2], 0);		// 2300
		tp->LineTo(m_FX[2], YB);
		if( sys.m_Repeater ){
			tp->Pen->Style = psDot;
			tp->MoveTo(m_FX[4], 0);
			tp->LineTo(m_FX[4], YB);
			tp->Pen->Style = psSolid;
		}
	}
	if( pDem->m_Sync && pDem->m_afc ){
		tp->Pen->Color = clWhite;
		tp->Pen->Style = psSolid;
#if NARROW_SYNC == 1200
		int fq = 1200 + pDem->m_AFCFQ;
#else
		int fq = (pDem->m_fNarrow ? NARROW_SYNC : 1200) + pDem->m_AFCFQ;
#endif
		TPoint	ary[3];
		x = int(((fq-m_low)*FFT_SIZE*double(m_XW)/double(FFTSamp*m_FM)) + 0.5);
		ary[0].x = x, ary[0].y = 0;
		ary[1].x = x-3, ary[1].y = 7;
		ary[2].x = x+3, ary[2].y = 7;
		tp->Brush->Color = pDem->m_AFCFlag ? clBlue : clRed;
		tp->Polygon(ary, 2);
		if( pDem->m_AFCFlag ) pDem->m_AFCFlag--;
	}
	if( sw || (!sys.m_FFTType) ) return;


	tp->Pen->Color = sys.m_ColorFFT;

	int d;
	double k = double(YB)/256.0;
	if( sys.m_FFTAGC ){
		if( sys.m_FFTType == 1 ){
			switch(sys.m_FFTGain){
				case 0:
				case 1:
				case 2:
				case 3:
					k = k * 64 / m_FFTMax;
					break;
				case 4:
					k = k * 80 / m_FFTMax;
					break;
				case 5:
					k = k * 128 / m_FFTMax;
					break;
				case 6:
					k = k * 384 / m_FFTMax;
					break;
				case 7:
					k = k * 768 / m_FFTMax;
					break;
			}
		}
		else {
			k = k * 128 / m_FFTMax;
		}
	}
	else if( sys.m_FFTGain >= 4 ){
		k = k * 0.25;
	}
	int max = 0;
	int *mp = &m_fft[1];
	int *wp = &m_FFTMAX[1];
	for( i = 1; i < m_XW; i++, mp++, wp++ ){
		d = *mp * k;
		if( d > max ) max = d;
		if( d >= YB ) d = YB - 1;
		if( *wp < d ) *wp = d;
		if( i > 1 ){
			tp->LineTo(i, YB-d);
		}
		else {
			tp->MoveTo(i, YB-d);
		}
	}
	if( sys.m_FFTAGC ){
		m_FFTSumMax -= m_FFTMax;
		m_FFTSumMax += max;
		switch(sys.m_FFTGain){
			case 4:
				if( m_FFTSumMax < 2048 ) m_FFTSumMax = 2048;
				break;
			case 5:
				if( m_FFTSumMax < 1024 ) m_FFTSumMax = 1024;
				break;
			case 6:
				if( m_FFTSumMax < 256 ) m_FFTSumMax = 256;
				break;
			case 7:
				if( m_FFTSumMax < 128 ) m_FFTSumMax = 128;
				break;
			default:
				if( m_FFTSumMax < 160 ) m_FFTSumMax = 160;
				break;
		}
		m_FFTMax = m_FFTSumMax / 4;
	}

	if( sys.m_FFTStg ){
		tp->Pen->Color = sys.m_ColorFFTStg;

		mp = &m_FFTMAX[1];
		for( i = 1; i < XR; i++, mp++ ){
			d = *mp;
			if( i > 1 ){
				tp->LineTo(i, YB-d);
			}
			else {
				tp->MoveTo(i, YB-d);
			}

			switch(sys.m_FFTStg){
				case 1:
					(*mp) -= 8;
					if( *mp < 0 ) *mp = 0;
					break;
				case 2:
					(*mp) -= 4;
					if( *mp < 0 ) *mp = 0;
					break;
				case 3:
					(*mp) -= 2;
					if( *mp < 0 ) *mp = 0;
					break;
			}
		}
	}
	if( pSound->m_notch ){
//    	tp->Brush->Color = sys.m_ColorFFTB;
		tp->Font->Color = clWhite;
		tp->Font->Height = -16;
		int tm = ::SetBkMode(tp->Handle, TRANSPARENT);
		tp->TextOut(0, 0, "Notch");
		::SetBkMode(tp->Handle, tm);
		tp->Pen->Color = clWhite;
		tp->Pen->Style = psSolid;
		TPoint	ary[3];
		x = m_FX[5];
		ary[0].x = x, ary[0].y = YB;
		ary[1].x = x-5, ary[1].y = YB-9;
		ary[2].x = x+5, ary[2].y = YB-9;
		tp->Brush->Color = clRed;
		tp->Polygon(ary, 2);
	}
#if SHOWAFCFQ
    else if( pDem->m_Sync && pDem->m_AFCFlag ){
		tp->Font->Color = clWhite;
		tp->Font->Height = -16;
		int tm = ::SetBkMode(tp->Handle, TRANSPARENT);
		int fq = (pDem->m_fNarrow ? NARROW_SYNC : 1200) + pDem->m_AFCFQ;
        sprintf(bf, "%u", fq);
        tp->TextOut(0, 0, bf);
		::SetBkMode(tp->Handle, tm);
    }
#endif
}
//---------------------------------------------------------------------------
// スレッド内でコールしてはいけない
void __fastcall TMmsstv::DrawWater(int sw)
{
	TCanvas *tp = pBitmapWater->Canvas;
	TRect rc;
	int XR = pBitmapWater->Width - 1;
	int YB = pBitmapWater->Height - 1;
	rc.Left = 0;
  rc.Top = 0;
	rc.Right = XR;
	rc.Bottom = YB+1;
	if( sw ){
		tp->Brush->Color = sys.m_ColorLow;
		tp->FillRect(rc);
		return;
	}
	int i;

	TRect src(rc);
	src.Bottom--;
	rc.Top++;
	tp->CopyRect(rc, tp, src);

	int d;
	int mx = -MAXINT;
	int mi = 0;
	int micnt = 0;
	double k1 = 128.0/double(m_WaterMax - m_WaterMin);
	BYTE *bp = (BYTE *)pBitmapWater->ScanLine[0];
	BYTE *cp;
	for( i = 0; i < m_XW; i++ ){
		if( sys.m_FFTStg ){
			d = m_FFTMAX[i] * 2;
		}
		else {
			d = m_fft[i] / 2;
		}
		if( mx < d ) mx = d;
		mi += d;
		micnt++;

		d -= m_WaterMin;
		d = double(d) * k1;

		if( d >= 128 ) d = 127;
		if( d < 0 ) d = 0;

		cp = ((BYTE *)&ColorTable[127-d]) + 2;
		*bp++ = *cp--;
		*bp++ = *cp--;
		*bp++ = *cp;
	}
	if( sys.m_FFTGain < 4 ){
		m_WaterSumMax -= m_WaterMax;
		m_WaterSumMax += mx;
		if( m_WaterSumMax < 64 ) m_WaterSumMax = 64;
		m_WaterMax = m_WaterSumMax / 4;
		if( micnt ){
			m_WaterMin = mi/micnt;
		}
		if( m_WaterMin >= (m_WaterMax - 16) ) m_WaterMax = m_WaterMin + 16;
	}
	else if( sys.m_FFTStg ){
		m_WaterMin = 0;
		m_WaterMax = 48;
	}
	else {
		m_WaterMin = 0;
		m_WaterMax = 64;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::CMfft(TMessage *Message)
{
	if( pSound != NULL ){
		CalcFFT();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::TimerTimer(TObject *Sender)
{
	if( pSound != NULL ){
		if( !pSound->IsBusy() && sys.m_FFTType && !DisPaint ){
			if( !sys.m_FFTPriority ){
				CalcFFT();
			}
			if( m_FFTFlag ){
				DrawFFT(0);
				PBoxFFT->Canvas->Draw(0, 0, pBitmapFFT);
				if( KVW->Checked ){
					DrawWater(0);
					PBoxWater->Canvas->Draw(0, 0, pBitmapWater);
				}
				if( sys.m_FFTPriority ) memset(m_fft, 0, sizeof(m_fft));
				m_FFTFlag = 0;
			}
		}
		SendSSTV();
		DrawSSTV();
		DrawLvl();

		if( pSound->IsBusy() ) return;

		if( pDem->m_Sync && m_SyncAccuracy && !m_ReqSampChg && (SSTVSET.m_Mode != smAVT) ){
			if( (m_SyncAccuracy == 2) && !(m_SyncAccuracyN & 2) && (m_AY >= 32) ){
				m_SyncAccuracyN = 3;
				RedrawSampFreq(FALSE);
			}
			else if( !(m_SyncAccuracyN & 1) && (m_AY >= 16) ){
				m_SyncAccuracyN = 1;
				RedrawSampFreq(FALSE);
			}
		}
#if 0
		if( (m_MainPage == pgTX) && (!SBTX->Down) ){
			if( !m_Timer1S ){
				m_Timer1S = 20;
				UpdateUI();
			}
			m_Timer1S--;
		}
#endif
		if( pRxView != NULL ){
			if( pRxView->Visible ){
				if( !SBTX->Down || sys.m_echo ){
					if( !m_TimerRXV ){
						m_TimerRXV = 5;
						pRxView->PBoxPaint(NULL);
					}
					m_TimerRXV--;
				}
//                ::SetWindowPos(pRxView->Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE	);
			}
			else {
				pRxView->GetViewPos(sys.m_RxViewLeft, sys.m_RxViewTop, sys.m_RxViewWidth, sys.m_RxViewHeight);
				delete pRxView;
				pRxView = NULL;
			}
		}
		if( pSyncView != NULL ){
			if( pSyncView->Visible ){
				if( !m_TimerRXS ){
					m_TimerRXS = 5;
					pSyncView->PBoxPaint(NULL);
				}
				m_TimerRXS--;
			}
			else {
				pSyncView->GetViewPos(sys.m_SyncViewLeft, sys.m_SyncViewTop, sys.m_SyncViewWidth, sys.m_SyncViewHeight);
				delete pSyncView;
				pSyncView = NULL;
			}
		}
		if( m_FileViewClose ){
			m_FileViewClose = 0;
			AdjustFileView();
			if( pHistView != NULL ){
				if( !pHistView->Visible ){
					CloseHistView();
				}
			}
		}
		if( (pCtrBtn != NULL) && !pCtrBtn->Visible ){
			pCtrBtn->GetViewPos(sys.m_CtrBtnLeft, sys.m_CtrBtnTop, sys.m_CtrBtnWidth, sys.m_CtrBtnHeight);
			delete pCtrBtn;
			pCtrBtn = NULL;
		}

		if( pDem->m_fskrec ){
			pDem->m_fskrec = 0;
			if( pDem->m_Sync && ((pCtrBtn == NULL) || (!pCtrBtn->SBLock->Down)) ){     // 強制停止
				if( pDem->m_LoopBack != 1 ){
					pDem->m_SyncMode = 0;
                    RxAutoPush(TRUE);
//					SBAuto->Down = TRUE;
//					SBAutoClick(NULL);
				}
			}
			if( strcmp(sys.m_Call.c_str(), pDem->m_fskcall) ){
				AddCall(pDem->m_fskcall);
				if( !SBTX->Down && (!SBQSO->Down || HisCall->Text.IsEmpty()) ){
					if( strcmp(AnsiString(HisCall->Text).c_str(), pDem->m_fskcall) ){	//ja7ude 0428
						HisCall->Text = pDem->m_fskcall;
						FindCall();
	                    HisCallChange(NULL);
					}
				}
			}
		}
		if( pDem->m_fskNRrec ){
			pDem->m_fskNRrec = 0;
#if DEBUGFSKID
	        if( (!SBQSO->Down || (strlen(MyRST->Text.c_str()) <= 3)) ){
#else
			if( !SBTX->Down && (!SBQSO->Down || (strlen(AnsiString(MyRST->Text).c_str()) <= 3) || !strcmp(AnsiString(HisCall->Text).c_str(), pDem->m_fskcall)) ){	//ja7ude 0428
#endif
				char bf[MLRST+1];
        	    sprintf(bf, "595%s", pDem->m_fskNRS);
				if( strcmp(AnsiString(MyRST->Text).c_str(), bf) ){	//ja7ude 0428
	            	MyRST->Text = bf;
                    HisCallChange(NULL);
                }
            }
        }
	}
	if( pRadio != NULL ){
		pRadio->Timer(SBTX->Down, Timer->Interval);
		if( pRadio->IsFreqChange(AnsiString(LogFreq->Text).c_str()) ){	//ja7ude 0428
			LogFreq->Text = pRadio->GetFreq();
			LogFreqChange(NULL);
		}
	}
	if( m_ChangeTemp && !SBTX->Down && (SBUseTemp->Down || (KTBT->Checked && (m_MainPage == pgTemp)) ) ){
		m_ChangeTemp--;
		if( !m_ChangeTemp ){
			m_ChangeTemp = 1;
			UpdatePic();
		}
	}
	if( m_ReqSampChg ){
		if( m_ReqSampChg < 0 ){
			m_ReqSampChg = 0;
			RedrawSampFreq(FALSE);
		}
		else {
			m_ReqSampChg--;
			if( !m_ReqSampChg ){
				RedrawSampFreq(TRUE);
			}
		}
	}
	if( m_ReqHistF && !SBTX->Down && !m_TempHold && ((m_MainPage == pgTX) || (m_MainPage == pgTemp)) ){
		m_ReqHistF--;
		if( !m_ReqHistF ){
			AdjustHistF();
			UpdatePic();
		}
	}
	if( m_TuneTimer && SBTune->Down ){
		if( ::GetTickCount() >= m_TuneTimer ){
			m_TuneTimer = 0;
			if( sys.m_TuneSat && (sys.m_TuneTXTime >= 0) ){
				ToTX();
			}
			else {
				ToRX();
			}
		}
	}
	if( m_TempTimer ) m_TempTimer--;

	if( LogLink.IsEnabled() ){
		if( LogLink.TimerLogLink() ){
			UpdateLogLink();
		}
		if( LogLink.IsPolling() && LogLink.IsLink() && !SBTX->Down && !pDem->m_Sync ){
			LogLink.EventGetFreq();
		}
	}

	if( sys.m_Repeater && !SBTX->Down && pDem->m_Repeater ){
		Repeater();
	}
    g_ExecPB.Timer();
#if 0
	CSSTVDEM *dp = &pSound->SSTVDEM;
	CSYNCINT *si = &dp->m_sint2;
	char bf[128];
	sprintf(bf, "%d, %d, %d, %d", si->m_MSyncList[5], si->m_MSyncList[4], si->m_MSyncList[3], si->m_MSyncList[2]);
//    sprintf(bf, "%d, %d, %s, %s", pDem->m_fskmode, pDem->m_fskcnt, pDem->m_fskdata, pDem->m_fskcall);
//    sprintf(bf, "%d", pDem->m_VisTrig);
//    sprintf(bf, "%d", dp->m_repsig);
	EditNote->Text = bf;
#endif
#if 0
	char bf[128];
	sprintf(bf, "%d,%d,%d", m_SyncMax, m_SyncRPos, int(SSTVSET.m_OFP));
	EditNote->Text = bf;
#endif
#if 0
	char bf[128];
	sprintf(bf, "%d", _getmbcp());
	EditNote->Text = bf;
//    _setmbcp(932);
#endif
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateLogLink(void)
{
	LogLink.UpdateLink(sys.m_LogLink);
	char bf[256];
	if( LogLink.IsLink() ){
		sprintf(bf, "Log [Link to %s]", LogLink.GetSessionName());
		GBLog->Caption = bf;
	}
	else {
		GBLog->Caption = "Log";
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SyncSSTV(void)
{
	CSSTVDEM *dp = pDem;
	if( SSTVSET.m_Mode == smAVT ){
		SSTVSET.m_IOFS = SSTVSET.m_OFS = 0;
		dp->m_wBgn = 0;
		return;
	}
	int e = 4;
	if( m_SyncAccuracy && sys.m_UseRxBuff && (SSTVSET.m_TW >= SSTVSET.m_SampFreq) ) e = 3;
	if( dp->m_wLine >= e ){
		int i, pg, x;
		int n = 0;
		int wd = int(SSTVSET.m_TW) + 2;
		int *bp = new int[wd];
		memset(bp, 0, sizeof(int)*(wd));
		for( pg = 0; pg < e; pg++ ){
			short *sp = &dp->m_B12[pg * dp->m_BWidth];
			for( i = 0; i < SSTVSET.m_WD; i++, sp++ ){
				x = fmod(n, SSTVSET.m_TW);
				bp[x] += *sp;
				n++;
			}
		}
		n = 0;
		int max = 0;
		for( i = 0; i < wd; i++ ){
			if( max < bp[i] ){
				max = bp[i];
				n = i;
			}
		}
		n -= SSTVSET.m_OFP;
		n = -n;
		switch(SSTVSET.m_Mode){
			case smSCT1:
			case smSCT2:
			case smSCTDX:
				if( n < 0 ) n += SSTVSET.m_WD;
				break;
			default:
				break;
		}
		if( dp->m_Type == 2 ) n -= dp->m_hill.m_htap/4;
		SSTVSET.m_IOFS = SSTVSET.m_OFS = dp->m_rBase = n;
		delete bp;
		dp->m_wBgn = 0;
	}
}

void __fastcall TMmsstv::InitAutoStop(void)
{
	memset(m_AutoStopAPos, 0, sizeof(m_AutoStopAPos));
	m_AutoStopCnt = 0;
	m_AutoStopACnt = 0;
	m_AutoStopPos = 0;
	m_ASBgnPos = 0x7fffffff;
	m_ASDis = 0;
	m_ASBitMask = 0;
	m_ASAvg.SetCount(16);
	m_Z[0] = m_Z[1] = m_Z[2] = 0;
	m_ASPos[0] = 64;
	m_ASPos[1] = 128;
	m_ASPos[2] = 160;
	m_ASPos[3] = (SSTVSET.m_L - 36);
	switch(SSTVSET.m_Mode){
		case smPD50:
		case smPD90:            // Max 128
		case smMP73:
		case smMP115:
		case smMP140:
		case smMP175:
		case smR24:
		case smRM8:
		case smRM12:
        case smMN73:
        case smMN110:
        case smMN140:
			m_ASPos[0] = 48;
			m_ASPos[1] = 64;
			m_ASPos[2] = 72;
			m_ASPos[3] = 110;
			break;
		case smPD160:           // Max 200
			m_ASPos[0] = 48;
			m_ASPos[1] = 80;
			m_ASPos[2] = 126;
			m_ASPos[3] = 160;
			break;
		case smPD290:           // Max 308 Limit 288
			m_ASPos[3] = 240;
			break;
		case smP3:              // Max496
			m_ASPos[1] = 200;
			m_ASPos[2] = 360;
			m_ASPos[3] = 496-48;
			break;
		case smP5:              // Max496 Limit 439
			m_ASPos[1] = 200;
			m_ASPos[2] = 300;
			m_ASPos[3] = 380;
			break;
		case smP7:              // Max496 Limit 330
			m_ASPos[1] = 128;
			m_ASPos[2] = 220;
			m_ASPos[3] = 280;
			break;
	}
	m_AutoSyncPos = 0x7fffffff;
	m_Mult = int(SSTVSET.m_TW / 320.0);
	m_AutoSyncDiff = m_Mult * 3;
	if( m_AutoSyncDiff > (45 * SampBase / 11025) ) m_AutoSyncDiff = (45 * SampBase / 11025);
}

//---------------------------------------------------------------------------
// 最小2乗近似
int __fastcall TMmsstv::GetSqerrPos(int n)
{
	double  T, L, TT, TL;
	T = L = TT = TL = 0;
	for( int i = 0; i < n; i++ ){
		T += i;
		int l = m_AutoStopAPos[15-i];
		L += l;
		TT += i * i;
		TL += i * l;
	}
	double l0 = (L * TT - T * TL) / (n * TT - T*T);
//    double k0 = (5 * TL - L * T) / (5 * TT - T*T);
	return l0;
}

//---------------------------------------------------------------------------
int __fastcall TMmsstv::AutoStopJob(void)
{
	if( (sys.m_AutoStop || sys.m_AutoSync || KRSA->Checked) && (SSTVSET.m_Mode != smAVT) ){   // 自動停止のチェック
		m_AutoStopPos = m_SyncPos - int(SSTVSET.m_OFP);
		int h = SSTVSET.m_TW/2;
		m_AutoStopPos = m_AutoStopPos > h ? m_AutoStopPos - SSTVSET.m_TW : m_AutoStopPos;
		if( m_AutoStopACnt >= 8 ){
			int i, n;
			for( n = i = 0; i < 16; i++ ){
				int df = ABS(m_AutoStopPos - m_AutoStopAPos[i]);
				if( m_AutoStopACnt >= 16 ){
					if( df <= (14*m_Mult) ) n++;
				}
				else {
					if( df <= (10*m_Mult) ) n++;
				}
			}
#if 0
		char bf[128];
		sprintf(bf, "%d, %d", m_AutoStopPos, m_SyncMax - m_SyncMin);
		EditNote->Text = bf;
#endif
			if( n < 4 ){
				if( sys.m_AutoSync && !m_ASDis && (n >= 2) && (m_AutoSyncPos != 0x7fffffff) && !m_AutoSyncCount && sys.m_UseRxBuff ){
					if( (m_SyncMax - m_SyncMin) > 5000 ){
						int df = m_AutoStopPos - m_AutoStopAPos[15];
						if( ABS(df) <= ((KRSA->Checked ? 5 : 2)*m_Mult) ){
							if( m_AutoSyncCount || !sys.m_UseRxBuff ){
								df = m_AutoStopPos;
							}
							else {
								df = m_AutoStopPos - m_AutoSyncPos;
							}
							if( ABS(df) >= ((KRSA->Checked ? 5 : 2)*m_Mult) ){
								df = m_AutoStopPos;
								if( df < 0 ) df += SSTVSET.m_TW;
								pDem->m_Skip = df;
								m_SyncPos = m_SyncRPos = -1;
								m_AutoSyncPos = 0x7fffffff;
								m_AutoSyncDis = 6;
								m_AutoSyncCount++;
								if( m_AutoStopCnt ) m_AutoStopCnt--;
							}
						}
					}
				}
				if( (n < 2) || ((m_SyncMax - m_SyncMin) < 8192) ){
					m_AutoStopCnt++;
				}
				if( sys.m_AutoStop && !m_ASDis && (m_AutoStopCnt >= 8) ){
					RxAutoPush(TRUE);
//					SBAuto->Down = TRUE;
//					SBAutoClick(NULL);
					return TRUE;
				}
			}
			else {
				m_AutoSyncPos = m_AutoStopPos;
				m_AutoStopCnt -= 2;
				if( m_AutoStopCnt < 0 ) m_AutoStopCnt = 0;
			}
			if( sys.m_AutoSync && !m_ASDis && (!m_AutoSyncDis) && (m_AutoSyncPos != 0x7fffffff) && (m_AutoSyncCount || !sys.m_UseRxBuff) ){
				if( (m_SyncMax - m_SyncMin) > 5000 ){
					int df = m_AutoStopPos - m_AutoStopAPos[15];
					if( ABS(df) <= m_AutoSyncDiff ){
						df = m_AutoStopPos;
						if( ABS(df) >= m_AutoSyncDiff ){
							if( df < 0 ) df += SSTVSET.m_TW;
							pDem->m_Skip = df;
							m_SyncPos = m_SyncRPos = -1;
							m_AutoSyncPos = 0x7fffffff;
							m_AutoSyncDis = 6;
							m_AutoSyncCount++;
							if( m_AutoStopCnt ) m_AutoStopCnt--;
						}
					}
				}
			}
		}
		if( m_AutoSyncDis ) m_AutoSyncDis--;
		m_AutoStopACnt++;
		memcpy(m_AutoStopAPos, &m_AutoStopAPos[1], sizeof(m_AutoStopAPos) - sizeof(int));
		m_AutoStopAPos[15] = m_AutoStopPos;

		if( KRSA->Checked && !m_AutoSyncCount && (!SBTX->Down || (sys.m_echo == 2) || sys.m_TxSampOff) ){
			if( m_AutoStopACnt >= 5 ){
				int df, cf;
				df = ABS(m_AutoStopAPos[15] - m_AutoStopAPos[14]);
				cf = ABS(m_AutoStopAPos[14] - m_AutoStopAPos[13]);
				if( df < cf ) df = cf;
				cf = ABS(m_AutoStopAPos[13] - m_AutoStopAPos[12]);
				if( df < cf ) df = cf;
				cf = ABS(m_AutoStopAPos[12] - m_AutoStopAPos[11]);
				if( df < cf ) df = cf;
				cf = ABS(m_AutoStopAPos[11] - m_AutoStopAPos[10]);
				if( df < cf ) df = cf;
				int pos;
				if( df < (8*m_Mult) ){
//                      pos = m_AutoStopAPos[15];
					pos = GetSqerrPos(5);
#if 0
					char bf[64];
					sprintf(bf, "%d, %d", pos, m_AutoStopAPos[15]);
					EditNote->Text = bf;
#endif
					if( m_ASBgnPos == 0x7fffffff ){
						m_ASBgnPos = pos;
						m_ASCurY = 0;
					}
					else if( m_ASCurY >= 3){
						double d = (m_ASBgnPos - pos) * SSTVSET.m_SampFreq / SSTVSET.m_TW;
						d /= m_ASCurY;
						double smp = m_ASAvg.Avg(SSTVSET.m_SampFreq - d);
						d = SSTVSET.m_SampFreq - smp;
						if( (!(m_ASBitMask & 1) && (ABS(d) >= m_ASLmt[0]) )||
							(!(m_ASBitMask & 1) && (m_ASCurY >= 16) && (ABS(d) >= m_ASLmt[1]))||
							(!(m_ASBitMask & 1) && (m_ASCurY >= 32) && (ABS(d) >= m_ASLmt[2]))||
							(!(m_ASBitMask & 2) && (m_ASCurY >= m_ASPos[0]) && (ABS(d) >= m_ASLmt[3]))||
							(!(m_ASBitMask & 4) && (m_ASCurY >= m_ASPos[1]) && (ABS(d) >= m_ASLmt[4]))||
							(!(m_ASBitMask & 8) && (m_ASCurY >= m_ASPos[2]) && (ABS(d) >= m_ASLmt[5]))||
							(!(m_ASBitMask & 16) && (m_ASCurY >= m_ASPos[3]) && (ABS(d) >= m_ASLmt[6]))
						){
							if( m_ASCurY >= 32 ) m_ASBitMask |= 1;
							if( m_ASCurY >= m_ASPos[0] ) m_ASBitMask |= 2;
							if( m_ASCurY >= m_ASPos[1] ) m_ASBitMask |= 4;
							if( m_ASCurY >= m_ASPos[2] ) m_ASBitMask |= 8;
							if( m_ASCurY >= m_ASPos[3] ) m_ASBitMask |= 16;
							if( !m_ASDis ){
								if( smp > (SampFreq * 1100/1060) ){
									smp = SampFreq * 1100/1060;
								}
								SSTVSET.m_SampFreq = NormalSampFreq(smp, 50);
								m_ReqSampChg = 1;
							}
						}
#if 0
						if( !m_ASDis && (m_AutoStopACnt >= 16) ){
							char bf[32];
							sprintf(bf, "%8.2lf", NormalSampFreq(smp, 50));
							pBitmapD12->Canvas->Brush->Color = clBlack;
							pBitmapD12->Canvas->Font->Color = clWhite;
							pBitmapD12->Canvas->TextOut(250, 3, bf);
						}
#endif
					}
				}
			}
		}
		m_ASCurY++;
	}
	return FALSE;
}

//---------------------------------------------------------------------------
int __fastcall TMmsstv::GetPixelLevel(short *ip)
{
	if( sys.m_DemCalibration && (pCalibration != NULL) ){
		int d = (*ip / 8) + 2048;
		if( d < 0 ) d = 0;
		if( d > 4096 ) d = 4096;
		return pCalibration[d];
	}
	else {
		int d = (*ip - sys.m_DemOff);
		if( d >= 0 ){
			d *= sys.m_DemWhite;
		}
		else {
			d *= sys.m_DemBlack;
		}
		return d;
	}
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::GetPictureLevel(short *ip)
{
	int d;
	if( sys.m_UseRxBuff != 2 ){
		if( *ip < *(ip+SSTVSET.m_KSB) ){
			d = GetPixelLevel(ip+SSTVSET.m_KSB);
		}
		else {
			d = GetPixelLevel(ip);
		}
	}
	else {
		d = GetPixelLevel(ip);
	}
	return d;
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::GetPictureLevelDiff(short *ip)
{
	int d;     
	if( sys.m_UseRxBuff != 2 ){
		if( *ip < *(ip+SSTVSET.m_KSB) ){
			d = GetPixelLevel(ip+SSTVSET.m_KSB);
		}
		else {
			d = GetPixelLevel(ip);
		}
	}
	else {
		d = GetPixelLevel(ip);
	}
	double o = (d * -0.5 + m_Z[0] + m_Z[1] * -0.5);
	if( o > 0.0 ){
		o *= sys.m_DiffLevelP;
	}
	else {
		o *= sys.m_DiffLevelM;
	}
	o += m_Z[2];
	m_Z[1] = m_Z[0];
	m_Z[2] = m_Z[0] = d;
	return o;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::MultProc(void)
{
	if( pSound == NULL ) return;

	if( pDem->m_Sync || pSound->m_Tx ){
		if( pSound->m_Tx && IsSR() ) SendSSTV();
		::Sleep(pSound->IsBusy() ? 100 : 1);
		DrawSSTV();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DrawSSTV(short *ip, short *sp)
{
	if( sys.m_Differentiator && (SSTVSET.m_Mode != smSCTDX) ){
		DrawSSTVDiff(ip, sp);
	}
	else {
		DrawSSTVNormal(ip, sp);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DrawSSTVNormal(short *ip, short *sp)
{
	CSSTVDEM *dp = pDem;
	BYTE    *gp = NULL;
	BYTE    *bp = NULL;
	BYTE    *wp;
	BYTE    *gp2 = NULL;
	int R,G,B;
	int i, n;
	int bx = -1;
	n = dp->m_rBase;

	int ay = -5;
	for( i = 0; i < SSTVSET.m_WD; i++, ip++, sp++, n++ ){
		if( n < 0 ) continue;
		double ps = fmod(double(n), SSTVSET.m_TW);
		int y = int(double(n)/SSTVSET.m_TW);
		if( ay != y ){
			m_AY = ay = y;
			if( (SSTVSET.m_Mode == smSCT1)||(SSTVSET.m_Mode == smSCT2)||(SSTVSET.m_Mode == smSCTDX) ){
				if( (y > 0) && (y <= 256) ){
					gp = (BYTE *)pBitmapRX->ScanLine[y-1];
				}
				else {
					gp = NULL;
				}
			}
			else if( ((SSTVSET.m_Mode >= smPD50)&&(SSTVSET.m_Mode <= smPD290)) ||
					 ((SSTVSET.m_Mode >= smMP73)&&(SSTVSET.m_Mode <= smMP175)) ||
                     ((SSTVSET.m_Mode >= smMN73)&&(SSTVSET.m_Mode <= smMN140)) ||
					 ((SSTVSET.m_Mode >= smR24)&&(SSTVSET.m_Mode <= smRM12))
			){
				if( (y >= 0) && (y < SSTVSET.m_L) ){
					R = y * 2;
					gp = (BYTE *)pBitmapRX->ScanLine[R];
					gp2 = (BYTE *)pBitmapRX->ScanLine[R+1];
				}
				else {
					gp = NULL;
					gp2 = NULL;
				}
			}
			else if( (y >= 0) && (y < pBitmapRX->Height) ){
				gp = (BYTE *)pBitmapRX->ScanLine[y];
			}
			else {
				gp = NULL;
			}
			if( (y >= 0) && (y < 256) ){
				bp = (BYTE *)pBitmapD12->ScanLine[y];
			}
			else {
				bp = NULL;
			}
		}
		if( !int(ps) ){
			if( (sys.m_AutoStop || sys.m_AutoSync || KRSA->Checked) && dp->m_Sync && (m_SyncPos != -1) ){
				AutoStopJob();
			}
			m_SyncMin = m_SyncMax = *sp;
			m_SyncRPos = m_SyncPos;
		}
		else if( m_SyncMax < *sp ){
			m_SyncMax = *sp;
			m_SyncPos = ps;
		}
		else if( m_SyncMin > *sp ){
			m_SyncMin = *sp;
		}
		int     d, x;
		x = ps * pBitmapD12->Width / SSTVSET.m_TW;
		if( (x != bx) && (x < pBitmapD12->Width) && (x >= 0) ){
			if( bp != NULL ){
				d = (*sp * 256) / 4096;
				d = Limit256(d);
				wp = bp + (x * 3);
				*wp++ = BYTE(d);
				*wp++ = BYTE(d);
				*wp = BYTE(d);
			}
			bx = x;
		}
		if( ps >= SSTVSET.m_OF ){
			ps -= SSTVSET.m_OF;
			switch(SSTVSET.m_Mode){
			case smSCT1:
			case smSCT2:
			case smSCTDX:
				if( ps < SSTVSET.m_KS ){               // R
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (x != m_AX) && (x >= 0) && (x < 320) ){
						m_AX = x;
						if( SSTVSET.m_Mode == smSCTDX ){
							d = GetPixelLevel(ip);
						}
						else {
							d = GetPictureLevel(ip);
						}
						d += 128;
						d = Limit256(d);
						if( gp != NULL ){
							wp = gp + (x * 3);
							*wp++ = BYTE(m_D36[1][x]);  // B
							*wp++ = BYTE(m_D36[0][x]);  // G
							*wp = BYTE(d);              // R
						}
					}
				}
				else if( ps < SSTVSET.m_CG ){          // G
					ps -= SSTVSET.m_SG;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (x != m_AX) && (x >= 0) && (x < 320) ){
						m_AX = x;
						if( SSTVSET.m_Mode == smSCTDX ){
							d = GetPixelLevel(ip);
						}
						else {
							d = GetPictureLevel(ip);
						}
						d += 128;
						d = Limit256(d);
						m_D36[0][x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CB ){          // B
					ps -= SSTVSET.m_SB;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (x != m_AX) && (x >= 0) && (x < 320) ){
						m_AX = x;
						if( SSTVSET.m_Mode == smSCTDX ){
							d = GetPixelLevel(ip);
						}
						else {
							d = GetPictureLevel(ip);
						}
						d += 128;
						d = Limit256(d);
						m_D36[1][x] = short(d);
					}
				}
				break;
			case smR36:
				if( ps < SSTVSET.m_KS ){               // 輝度
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x < 320) ){
						m_AX = x;
						d = GetPictureLevel(ip);
						d += 128;
						d = Limit256(d);
						m_Y36[x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CG ){          // TCS
					ps -= SSTVSET.m_SG;
					if( ps >= 0 ){
						d = GetPixelLevel(ip);
						if( (d >= 64) || (d < -64) ){
							m_DSEL = (d >= 0) ? 1 : 0;  // RY=1500 m_D36[0], BY=2300 m_D36[1]
						}
						else {
							m_DSEL = m_DSEL ? 0 : 1;
						}
					}
				}
				else if( ps < SSTVSET.m_CB ){          // 色差
					ps -= SSTVSET.m_SB;
					x = ps * pBitmapRX->Width / SSTVSET.m_KS2S;
					if( (m_AX != x) && (x >= 0) && (x < 320) && (y < 240) ){
						m_AX = x;
						d = GetPixelLevel(ip);
						m_D36[m_DSEL][x] = short(d);
						YCtoRGB(R, G, B, m_Y36[x], m_D36[0][x], m_D36[1][x]);
						if( gp != NULL ){
							wp = gp + (x * 3);
							*wp++ = BYTE(B);  // B
							*wp++ = BYTE(G);  // G
							*wp = BYTE(R);    // R
						}
					}
				}
				break;
			case smR24:
			case smR72:
			case smMR73:
			case smMR90:
			case smMR115:
			case smMR140:
			case smMR175:
			case smML180:
			case smML240:
			case smML280:
			case smML320:
				if( ps < SSTVSET.m_KS ){               // 輝度
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x < pBitmapRX->Width) ){
						m_AX = x;
						d = GetPictureLevel(ip);
						d += 128;
						d = Limit256(d);
						m_Y36[x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CG ){          // R-Y
					ps -= SSTVSET.m_SG;
					x = ps * pBitmapRX->Width / SSTVSET.m_KS2S;
					if( (m_AX != x) && (x >= 0) && (x < pBitmapRX->Width) ){
						m_AX = x;
						d = GetPixelLevel(ip);
						m_D36[1][x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CB ){          // B-Y
					ps -= SSTVSET.m_SB;
					x = ps * pBitmapRX->Width / SSTVSET.m_KS2S;
					if( (m_AX != x) && (x >= 0) && (x < pBitmapRX->Width) && (y < SSTVSET.m_L) ){
						m_AX = x;
						d = GetPixelLevel(ip);
						YCtoRGB(R, G, B, m_Y36[x], m_D36[1][x], d);
						if( gp != NULL ){
							wp = gp + (x * 3);
							*wp++ = BYTE(B);  // B
							*wp++ = BYTE(G);  // G
							*wp = BYTE(R);    // R
							if( SSTVSET.m_Mode == smR24 ){
								wp = gp2 + (x * 3);
								*wp++ = BYTE(B);  // B
								*wp++ = BYTE(G);  // G
								*wp = BYTE(R);    // R
							}
						}
					}
				}
				break;
			case smPD50:
			case smPD90:
			case smPD120:
			case smPD160:
			case smPD180:
			case smPD240:
			case smPD290:
			case smMP73:
			case smMP115:
			case smMP140:
			case smMP175:
			case smMN73:
            case smMN110:
            case smMN140:
				if( ps < SSTVSET.m_KS ){               // 輝度
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x < pBitmapRX->Width) ){
						m_AX = x;
						d = GetPictureLevel(ip);
						d += 128;
						d = Limit256(d);
						m_Y36[x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CG ){          // R-Y
					ps -= SSTVSET.m_SG;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x < pBitmapRX->Width) ){
						m_AX = x;
						d = GetPixelLevel(ip);
						m_D36[1][x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CB ){          // B-Y
					ps -= SSTVSET.m_SB;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x < pBitmapRX->Width) ){
						m_AX = x;
						d = GetPixelLevel(ip);
						m_D36[0][x] = short(d);
					}
				}
				else if( ps < (SSTVSET.m_CB + SSTVSET.m_KS) ){          // Y(even)
					ps -= SSTVSET.m_CB;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x < pBitmapRX->Width) && (y < SSTVSET.m_L) ){
						m_AX = x;
						if( gp != NULL ){
							YCtoRGB(R, G, B, m_Y36[x], m_D36[1][x], m_D36[0][x]);
							wp = gp + (x * 3);
							*wp++ = BYTE(B);    // B
							*wp++ = BYTE(G);    // G
							*wp = BYTE(R);      // R
							d = GetPictureLevel(ip);
							d += 128;
							YCtoRGB(R, G, B, d, m_D36[1][x], m_D36[0][x]);
							wp = gp2 + (x * 3);
							*wp++ = BYTE(B);    // B
							*wp++ = BYTE(G);    // G
							*wp = BYTE(R);      // R
						}
					}
				}
				break;
			case smRM8:
			case smRM12:
				if( ps < SSTVSET.m_KS ){               // 輝度
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x < pBitmapRX->Width) && (y < SSTVSET.m_L) ){
						m_AX = x;
						d = GetPictureLevel(ip);
						d *= (256.0 / (256.0 - 32.0));
						d += 128;
						d = Limit256(d);
						if( gp != NULL ){
							wp = gp + (x * 3);
							*wp++ = BYTE(d);    // B
							*wp++ = BYTE(d);    // G
							*wp = BYTE(d);      // R
							wp = gp2 + (x * 3);
							*wp++ = BYTE(d);    // B
							*wp++ = BYTE(d);    // G
							*wp = BYTE(d);      // R
						}
					}
				}
				break;
			default:
				if( ps < SSTVSET.m_KS ){               // R or G(MRT)
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (x != m_AX) && (x >= 0) && (x < pBitmapRX->Width) ){
						m_AX = x;
						d = GetPictureLevel(ip);
						d += 128;
						d = Limit256(d);
						m_D36[0][x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CG ){          // G or B(MRT)
					ps -= SSTVSET.m_SG;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (x != m_AX) && (x >= 0) && (x < pBitmapRX->Width) ){
						m_AX = x;
						d = GetPictureLevel(ip);
						d += 128;
						d = Limit256(d);
						m_D36[1][x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CB ){          // B or R(MRT)
					ps -= SSTVSET.m_SB;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (x != m_AX) && (x >= 0) && (x < pBitmapRX->Width) ){
						m_AX = x;
						d = GetPictureLevel(ip);
						d += 128;
						d = Limit256(d);
						if( (SSTVSET.m_Mode == smMRT1)||(SSTVSET.m_Mode == smMRT2) ){
							if( gp != NULL ){
								wp = gp + (x * 3);
								*wp++ = BYTE(m_D36[1][x]);  // B
								*wp++ = BYTE(m_D36[0][x]);  // G
								*wp = BYTE(d);              // R
							}
						}
						else {
							if( gp != NULL ){
								wp = gp + (x * 3);
								*wp++ = BYTE(d);            // B
								*wp++ = BYTE(m_D36[1][x]);  // G
								*wp = BYTE(m_D36[0][x]);    // R
							}
						}
					}
				}
				break;
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DrawSSTVDiff(short *ip, short *sp)
{
	CSSTVDEM *dp = pDem;
	BYTE    *gp = NULL;
	BYTE    *bp = NULL;
	BYTE    *wp;
	BYTE    *gp2 = NULL;
	int R,G,B;
	int i, n;
	int bx = -1;
	n = dp->m_rBase;

	int ay = -5;
	for( i = 0; i < SSTVSET.m_WD; i++, ip++, sp++, n++ ){
		if( n < 0 ) continue;
		double ps = fmod(double(n), SSTVSET.m_TW);
		int y = int(double(n)/SSTVSET.m_TW);
		if( ay != y ){
			m_AY = ay = y;
			if( (SSTVSET.m_Mode == smSCT1)||(SSTVSET.m_Mode == smSCT2)||(SSTVSET.m_Mode == smSCTDX) ){
				if( (y > 0) && (y <= 256) ){
					gp = (BYTE *)pBitmapRX->ScanLine[y-1];
				}
				else {
					gp = NULL;
				}
			}
			else if( ((SSTVSET.m_Mode >= smPD50)&&(SSTVSET.m_Mode <= smPD290)) ||
					 ((SSTVSET.m_Mode >= smMP73)&&(SSTVSET.m_Mode <= smMP175)) ||
					 ((SSTVSET.m_Mode >= smMN73)&&(SSTVSET.m_Mode <= smMN140)) ||
					 ((SSTVSET.m_Mode >= smR24)&&(SSTVSET.m_Mode <= smRM12))
			){
				if( (y >= 0) && (y < SSTVSET.m_L) ){
					R = y * 2;
					gp = (BYTE *)pBitmapRX->ScanLine[R];
					gp2 = (BYTE *)pBitmapRX->ScanLine[R+1];
				}
				else {
					gp = NULL;
					gp2 = NULL;
				}
			}
			else if( (y >= 0) && (y < pBitmapRX->Height) ){
				gp = (BYTE *)pBitmapRX->ScanLine[y];
			}
			else {
				gp = NULL;
			}
			if( (y >= 0) && (y < 256) ){
				bp = (BYTE *)pBitmapD12->ScanLine[y];
			}
			else {
				bp = NULL;
			}
		}
		if( !int(ps) ){
			if( (sys.m_AutoStop || sys.m_AutoSync || KRSA->Checked) && dp->m_Sync && (m_SyncPos != -1) ){
				AutoStopJob();
			}
			m_SyncMin = m_SyncMax = *sp;
			m_SyncRPos = m_SyncPos;
		}
		else if( m_SyncMax < *sp ){
			m_SyncMax = *sp;
			m_SyncPos = ps;
		}
		else if( m_SyncMin > *sp ){
			m_SyncMin = *sp;
		}
		int     d, x;
		x = ps * pBitmapD12->Width / SSTVSET.m_TW;
		if( (x != bx) && (x < pBitmapD12->Width) && (x >= 0) ){
			if( bp != NULL ){
				d = (*sp * 256) / 4096;
				d = Limit256(d);
				wp = bp + (x * 3);
				*wp++ = BYTE(d);
				*wp++ = BYTE(d);
				*wp = BYTE(d);
			}
			bx = x;
		}
		if( ps >= SSTVSET.m_OF ){
			ps -= SSTVSET.m_OF;
			switch(SSTVSET.m_Mode){
			case smSCT1:
			case smSCT2:
				if( ps < SSTVSET.m_KS ){               // R
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (x != m_AX) && (x >= 0) && (x < 321) ){
						m_AX = x;
						x = x ? x - 1 : 0;
						d = GetPictureLevelDiff(ip);
						if( m_AX ){
							d += 128;
							d = Limit256(d);
							if( gp != NULL ){
								wp = gp + (x * 3);
								*wp++ = BYTE(m_D36[1][x]);  // B
								*wp++ = BYTE(m_D36[0][x]);  // G
								*wp = BYTE(d);              // R
							}
						}
					}
				}
				else if( ps < SSTVSET.m_CG ){          // G
					ps -= SSTVSET.m_SG;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (x != m_AX) && (x >= 0) && (x < 321) ){
						m_AX = x;
						x = x ? x - 1 : 0;
						d = GetPictureLevelDiff(ip);
						d += 128;
						d = Limit256(d);
						m_D36[0][x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CB ){          // B
					ps -= SSTVSET.m_SB;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (x != m_AX) && (x >= 0) && (x < 321) ){
						m_AX = x;
						x = x ? x - 1 : 0;
						d = GetPictureLevelDiff(ip);
						d += 128;
						d = Limit256(d);
						m_D36[1][x] = short(d);
					}
				}
				break;
			case smR36:
				if( ps < SSTVSET.m_KS ){               // 輝度
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x < 321) ){
						m_AX = x;
						x = x ? x - 1 : 0;
						d = GetPictureLevelDiff(ip);
						d += 128;
						d = Limit256(d);
						m_Y36[x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CG ){          // TCS
					ps -= SSTVSET.m_SG;
					if( ps >= 0 ){
						d = GetPixelLevel(ip);
						if( (d >= 64) || (d < -64) ){
							m_DSEL = (d >= 0) ? 1 : 0;  // RY=1500 m_D36[0], BY=2300 m_D36[1]
						}
						else {
							m_DSEL = m_DSEL ? 0 : 1;
						}
					}
				}
				else if( ps < SSTVSET.m_CB ){          // 色差
					ps -= SSTVSET.m_SB;
					x = ps * pBitmapRX->Width / SSTVSET.m_KS2S;
					if( (m_AX != x) && (x >= 0) && (x < 320) && (y < 240) ){
						m_AX = x;
						d = GetPixelLevel(ip);
						m_D36[m_DSEL][x] = short(d);
						YCtoRGB(R, G, B, m_Y36[x], m_D36[0][x], m_D36[1][x]);
						if( gp != NULL ){
							wp = gp + (x * 3);
							*wp++ = BYTE(B);  // B
							*wp++ = BYTE(G);  // G
							*wp = BYTE(R);    // R
						}
					}
				}
				break;
			case smR24:
			case smR72:
			case smMR73:
			case smMR90:
			case smMR115:
			case smMR140:
			case smMR175:
			case smML180:
			case smML240:
			case smML280:
			case smML320:
				if( ps < SSTVSET.m_KS ){               // 輝度
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x <= pBitmapRX->Width) ){
						m_AX = x;
						x = x ? x - 1 : 0;
						d = GetPictureLevelDiff(ip);
						d += 128;
						d = Limit256(d);
						m_Y36[x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CG ){          // R-Y
					ps -= SSTVSET.m_SG;
					x = ps * pBitmapRX->Width / SSTVSET.m_KS2S;
					if( (m_AX != x) && (x >= 0) && (x < pBitmapRX->Width) ){
						m_AX = x;
						d = GetPixelLevel(ip);
						m_D36[1][x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CB ){          // B-Y
					ps -= SSTVSET.m_SB;
					x = ps * pBitmapRX->Width / SSTVSET.m_KS2S;
					if( (m_AX != x) && (x >= 0) && (x < pBitmapRX->Width) && (y < SSTVSET.m_L) ){
						m_AX = x;
						d = GetPixelLevel(ip);
						YCtoRGB(R, G, B, m_Y36[x], m_D36[1][x], d);
						if( gp != NULL ){
							wp = gp + (x * 3);
							*wp++ = BYTE(B);  // B
							*wp++ = BYTE(G);  // G
							*wp = BYTE(R);    // R
							if( SSTVSET.m_Mode == smR24 ){
								wp = gp2 + (x * 3);
								*wp++ = BYTE(B);  // B
								*wp++ = BYTE(G);  // G
								*wp = BYTE(R);    // R
							}
						}
					}
				}
				break;
			case smPD50:
			case smPD90:
			case smPD120:
			case smPD180:
			case smPD240:
			case smPD290:
			case smMP73:
			case smMP115:
			case smMP140:
			case smMP175:
			case smMN73:
            case smMN110:
            case smMN140:
				if( ps < SSTVSET.m_KS ){               // 輝度
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x < (pBitmapRX->Width + 1)) ){
						m_AX = x;
						x = x ? x - 1 : 0;
						d = GetPictureLevelDiff(ip);
						d += 128;
						d = Limit256(d);
						m_Y36[x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CG ){          // R-Y
					ps -= SSTVSET.m_SG;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x < pBitmapRX->Width) ){
						m_AX = x;
						d = GetPixelLevel(ip);
						m_D36[1][x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CB ){          // B-Y
					ps -= SSTVSET.m_SB;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x < pBitmapRX->Width) ){
						m_AX = x;
						d = GetPixelLevel(ip);
						m_D36[0][x] = short(d);
					}
				}
				else if( ps < (SSTVSET.m_CB + SSTVSET.m_KS) ){          // Y(even)
					ps -= SSTVSET.m_CB;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x < (pBitmapRX->Width + 1)) && (y < SSTVSET.m_L) ){
						m_AX = x;
						x = x ? x - 1 : 0;
						if( gp != NULL ){
							YCtoRGB(R, G, B, m_Y36[x], m_D36[1][x], m_D36[0][x]);
							wp = gp + (x * 3);
							*wp++ = BYTE(B);    // B
							*wp++ = BYTE(G);    // G
							*wp = BYTE(R);      // R
							d = GetPictureLevelDiff(ip);
							d += 128;
							YCtoRGB(R, G, B, d, m_D36[1][x], m_D36[0][x]);
							wp = gp2 + (x * 3);
							*wp++ = BYTE(B);    // B
							*wp++ = BYTE(G);    // G
							*wp = BYTE(R);      // R
						}
					}
				}
				break;
			case smRM8:
			case smRM12:
				if( ps < SSTVSET.m_KS ){               // 輝度
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (m_AX != x) && (x >= 0) && (x < (pBitmapRX->Width + 1)) && (y < SSTVSET.m_L) ){
						m_AX = x;
						x = x ? x - 1 : 0;
						d = GetPictureLevelDiff(ip);
						d *= (256.0 / (256.0 - 32.0));
						d += 128;
						d = Limit256(d);
						if( gp != NULL ){
							wp = gp + (x * 3);
							*wp++ = BYTE(d);    // B
							*wp++ = BYTE(d);    // G
							*wp = BYTE(d);      // R
							wp = gp2 + (x * 3);
							*wp++ = BYTE(d);    // B
							*wp++ = BYTE(d);    // G
							*wp = BYTE(d);      // R
						}
					}
				}
				break;
			default:
				if( ps < SSTVSET.m_KS ){               // R or G(MRT)
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (x != m_AX) && (x >= 0) && (x < (pBitmapRX->Width + 1)) ){
						m_AX = x;
						x = x ? x - 1 : 0;
						d = GetPictureLevelDiff(ip);
						d += 128;
						d = Limit256(d);
						m_D36[0][x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CG ){          // G or B(MRT)
					ps -= SSTVSET.m_SG;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (x != m_AX) && (x >= 0) && (x < (pBitmapRX->Width + 1)) ){
						m_AX = x;
						x = x ? x - 1 : 0;
						d = GetPictureLevelDiff(ip);
						d += 128;
						d = Limit256(d);
						m_D36[1][x] = short(d);
					}
				}
				else if( ps < SSTVSET.m_CB ){          // B or R(MRT)
					ps -= SSTVSET.m_SB;
					x = ps * pBitmapRX->Width / SSTVSET.m_KSS;
					if( (x != m_AX) && (x >= 0) && (x < (pBitmapRX->Width + 1)) ){
						m_AX = x;
						x = x ? x - 1 : 0;
						d = GetPictureLevelDiff(ip);
						if( m_AX ){
							d += 128;
							d = Limit256(d);
							if( (SSTVSET.m_Mode == smMRT1)||(SSTVSET.m_Mode == smMRT2) ){
								if( gp != NULL ){
									wp = gp + (x * 3);
									*wp++ = BYTE(m_D36[1][x]);  // B
									*wp++ = BYTE(m_D36[0][x]);  // G
									*wp = BYTE(d);              // R
								}
							}
							else {
								if( gp != NULL ){
									wp = gp + (x * 3);
									*wp++ = BYTE(d);            // B
									*wp++ = BYTE(m_D36[1][x]);  // G
									*wp = BYTE(m_D36[0][x]);    // R
								}
							}
						}
					}
				}
				break;
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::WriteHistory(int sw)
{
	if( !pDem->m_Sync ){
		PBoxRXPaint(NULL);
		DrawSync();
	}
	if( pComm != NULL ) pComm->SetScan(0);
	int f = sw;
	if( m_AY > (SSTVSET.m_L * 65/100) ){
		f = 1;
		TrackTxMode(0);
	}
	if( f && KEA->Checked ){
		CopyBitmap(pBitmapRX);
	}
	if( CBASave->Checked && f ){
		SBWHistClick(NULL);         // Write history
	}
	else {
		SBWHist->Enabled = TRUE;
	}
	if( sys.m_Repeater ){
		SetBeaconTick();
		if( f ) MakeReplay();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::TrackTxMode(int sw)
{
	if( (!sys.m_FixedTxMode) && (!SBTX->Down) && (SSTVSET.m_Mode != SSTVSET.m_TxMode) ){
		if( (m_RXW == pBitmapTX->Width) || sw ){
			ChangeTxMode(SSTVSET.m_Mode);
            UpdateModeBtn();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DrawSSTV(void)
{
	CSSTVDEM *dp = pDem;
	if( dp->m_Sync && (dp->m_wPage != dp->m_rPage) ){
		while( dp->m_Sync && (dp->m_wPage != dp->m_rPage) ){
			if( dp->m_wBgn ){
				if( dp->m_wBgn != 1 ){
					m_SyncRPos = m_SyncPos = -1;
					m_SyncAccuracyN = 0;
					SBTO->Enabled = FALSE;
					dp->m_fskcall[0] = 0;
					m_DSEL = 0;
					m_AX = -1;
					m_AY = -5;
					if( dp->m_ReqSave ){
						dp->m_ReqSave = 0;
						WriteHistory(1);
					}
					SSTVSET.GetPictureSize(m_RXW, m_RXH, m_RXPH, SSTVSET.m_Mode);
					if( KRSW->Checked ){
						if( pRxView != NULL ){
							if( pRxView->Handle != ::GetForegroundWindow() ){
								::SetForegroundWindow(pRxView->Handle);
							}
						}
						else if( m_MainPage != pgRX ){
							AdjustPage(pgRX);
						}
					}
					if( pBitmapRX->Width != m_RXW ){
						pBitmapRX->Width = m_RXW;
						pBitmapRX->Height = m_RXH;
						PBoxRX->Invalidate();
					}
					UpdateModeBtn();
					dp->m_LoopBack = SBTX->Down ? sys.m_echo : 0;
					::GetUTC(&m_StartTime);
					dp->m_SyncAVT = 0;
					WaveStg.WInit();
					RxHist.ClearAddFlag();
					dp->m_wStgLine = 0;
					DispSyncStat();
					SBWHist->Enabled = FALSE;
					KRH->Enabled = FALSE;
					int rf = 0;
					if( sys.m_Repeater && dp->m_Repeater ){
						dp->m_repRX++;
						SetBeaconTick();
						if( dp->m_repmode == 5 ){
							dp->m_repmode++;
							dp->m_repRLY++;
							rf = 1;
						}
						else if( dp->m_repmode <= 2 ){
							dp->m_repmode = 0;
						}
					}
					if( pRxView != NULL ){
						pRxView->UpdateTitle(SSTVSET.m_Mode, rf);
					}
					if( pComm != NULL ) pComm->SetScan(pDem->m_Sync);
				}
				dp->m_wBgn = 1;
				SyncSSTV();
				if( dp->m_wBgn ) return;
				m_HistM = SSTVSET.m_Mode;
				if( KRD->Checked ){
					ClearDraw(pBitmapRX, PBoxRX, sys.m_ColorRXB);
				}
				ClearDraw(pBitmapD12, PBoxD12, clWhite);
				InitAutoStop();
				m_AutoSyncCount = m_AutoSyncDis = 0;
			}
			short *ip = &dp->m_Buf[dp->m_rPage * dp->m_BWidth];
			short *sp = &dp->m_B12[dp->m_rPage * dp->m_BWidth];
			if( dp->m_StgBuf ){
				if( ((dp->m_wStgLine + 1) * SSTVSET.m_WD) < dp->m_RxBufAllocSize ){
					memcpy(&dp->m_StgBuf[dp->m_wStgLine * SSTVSET.m_WD], ip, SSTVSET.m_WD*sizeof(short));
					memcpy(&dp->m_StgB12[dp->m_wStgLine * SSTVSET.m_WD], sp, SSTVSET.m_WD*sizeof(short));
					dp->m_wStgLine++;
					if( dp->m_wStgLine == 16 ) UpdateSBTO();
				}
			}
			else if( WaveStg.IsOpen() ){
				WaveStg.Write(ip, SSTVSET.m_WD*sizeof(short));
				WaveStg.Write(sp, SSTVSET.m_WD*sizeof(short));
				dp->m_wStgLine++;
				if( dp->m_wStgLine == 16 ) UpdateSBTO();
			}
			DrawSSTV(ip, sp);
			dp->m_rBase += SSTVSET.m_WD;
			if( m_AY > SSTVSET.m_L ){
				if( dp->m_Sync ){
					int lost = dp->m_Lost;
					dp->Stop();
					if( dp->m_LoopBack != 1 ){
						WriteHistory(1);
					}
					else {
						SBWHist->Enabled = TRUE;
					}
					TrackTxMode(0);
					UpdateModeBtn();
					if( lost ) InfoRxLost();
	                UpdateUI();
				}
				break;
			}
			dp->m_rPage++;
			if( dp->m_rPage >= SSTVDEMBUFMAX ){
				dp->m_rPage = 0;
			}
		}
		if( !pSound->IsBusy() ){
			PBoxRXPaint(NULL);
			DrawSync();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::InfoRxLost(void)
{
	InfoMB( MsgEng ?
		"MMSSTV lost some sound during the receiving by the CPU overload.\r\n\r\n"
		"Do increase FIFO on sound in the Misc page of Setup MMSSTV." :
		"受信中にCPU過負荷によりサウンドを一部欠落しました.\r\n\r\n"
		"設定画面の「その他」ページのサウンドカードのFIFOを増やして下さい"
	);
}
void __fastcall TMmsstv::InfoTxLost(void)
{
	InfoMB( MsgEng ?
		"MMSSTV lost some sound during the transmission by the CPU overload.\r\n\r\n"
		"Do increase FIFO on sound in the Misc page of Setup MMSSTV." :
		"送信中にCPU過負荷によりサウンドを一部欠落しました.\r\n\r\n"
		"設定画面の「その他」ページのサウンドカードのFIFOを増やして下さい"
	);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxFFTPaint(TObject *Sender)
{
	PBoxFFT->Canvas->Draw(0, 0, pBitmapFFT);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxWaterPaint(TObject *Sender)
{
	if( KVW->Checked ) PBoxWater->Canvas->Draw(0, 0, pBitmapWater);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxRXPaint(TObject *Sender)
{
	if( m_MainPage != pgRX ) return;

	DrawBitmap(PBoxRX, pBitmapRX);
	if( sys.m_ShowSizeRX ){
		ShowSize(PBoxRX, m_RXW, m_RXPH, 12, clGreen);
	}
#if 0
	if( sys.m_Repeater ){
		CSSTVDEM *dp = pDem;
		if( dp->m_Repeater && (dp->m_repmode == 6) ){
			PBoxRX->Canvas->Brush->Color = clBlue;
			PBoxRX->Canvas->Font->Color = clWhite;
			PBoxRX->Canvas->Font->Height = -12;
			LPCSTR dp = "Repeater";
			int fw = PBoxRX->Canvas->TextWidth(dp);
			int fh = PBoxRX->Canvas->TextHeight(dp);
			PBoxRX->Canvas->TextOut(PBoxRX->Width - fw, PBoxRX->Height - fh, dp);
		}
	}
#endif
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFRecTimClick(TObject *Sender)
{
	if( pSound == NULL ) return;
	SYSTEMTIME	now;
	GetLocal(&now);

	char bf[256];
	sprintf(bf, "%s%02u%02u%02u%02u.mmv", RecDir, now.wMonth, now.wDay, now.wHour, now.wMinute);
	pSound->WaveFile.Rec(bf);
	if( pSound->WaveFile.m_mode == 2 ){
		KFRecTim->Checked = TRUE;
		RecentAdd(pSound->WaveFile.m_FileName.c_str(), TRUE);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFRecClick(TObject *Sender)
{
	if( pSound == NULL ) return;
	if( MsgEng ){
		SaveDialog->Title = "Record MMSSTV Sound file";
		SaveDialog->Filter = "MMSSTV Sound Files(*.mmv)|*.mmv|";
	}
	else {
		SaveDialog->Title = "MMSSTVサウンドファイルの作成";
		SaveDialog->Filter = "MMSSTVサウンドファイル(*.mmv)|*.mmv|";
	}
	SaveDialog->FileName = "Recv.mmv";
	SaveDialog->DefaultExt = "mmv";
	SaveDialog->InitialDir = RecDir;
	SetDisPaint();
	NormalWindow(this);
	if( SaveDialog->Execute() == TRUE ){
		pSound->WaveFile.Rec(AnsiString(SaveDialog->FileName).c_str());	//ja7ude 0428
		if( pSound->WaveFile.m_mode == 2 ){
			KFRec->Checked = TRUE;
			RecentAdd(pSound->WaveFile.m_FileName.c_str(), TRUE);
		}
		SetDirName(RecDir, AnsiString(SaveDialog->FileName).c_str());	//ja7ude 0428
	}
	TopWindow(this);
	ResDisPaint();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFPlayClick(TObject *Sender)
{
	if( pSound == NULL ) return;
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options << ofFileMustExist;
	if( MsgEng ){
		OpenDialog->Title = "Play MMSSTV Sound file";
		OpenDialog->Filter = "MMSSTV Sound Files(*.mmv)|*.mmv|";
	}
	else {
		OpenDialog->Title = "MMSSTVサウンドファイルの再生";
		OpenDialog->Filter = "MMSSTVサウンドファイル(*.mmv)|*.mmv|";
	}
	OpenDialog->FileName = "";
	OpenDialog->DefaultExt = "mmv";
	OpenDialog->InitialDir = RecDir;
	SetDisPaint();
	NormalWindow(this);
	if( OpenDialogExecute(FALSE) == TRUE ){
		pSound->WaveFile.Play(AnsiString(OpenDialog->FileName).c_str());	//ja7ude 0428
		SetDirName(RecDir, AnsiString(OpenDialog->FileName).c_str());	//ja7ude 0428
	}
	ResDisPaint();
	if( pSound->WaveFile.m_mode == 1 ){
		KFPlayPosClick(NULL);
		RecentAdd(pSound->WaveFile.m_FileName.c_str(), TRUE);
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
// Recent Menu Click
void __fastcall TMmsstv::OnRecentClick(TObject *Sender)
{
	if( pSound == NULL ) return;
	LPCSTR pName = RecentMenu.FindItemText((TMenuItem *)Sender);
	if( pName != NULL ){
		char bf[256];
		strcpy(bf, pName);
		RecentAdd(bf, pSound->WaveFile.Play(bf));
		if( pSound->WaveFile.m_mode == 1 ){
			KFPlayPosClick(NULL);
		}
	}
}
//---------------------------------------------------------------------------
// Recent Menu Click
void __fastcall TMmsstv::RecentAdd(LPCSTR pNew, int f)
{
	if( RecentMenu.IsAdd(pNew) ){
		RecentMenu.Delete();
		if( f != FALSE ){
			RecentMenu.Add(pNew);
		}
		else {
			RecentMenu.Delete(pNew);
		}
		RecentMenu.Insert(NULL, OnRecentClick);
	}
}
//---------------------------------------------------------------------------
// Recent Menu Update
void __fastcall TMmsstv::UpdateRecentMenu(void)
{
	RecentMenu.Delete();
	RecentMenu.Init(KExit, KFile, RECMENUMAX);
	RecentMenu.ExtFilter("MMV");
	RecentMenu.Insert(NULL, OnRecentClick);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFPlayPosClick(TObject *Sender)
{
	if( pSound == NULL ) return;
	TPlayDlgBox *pBox = new TPlayDlgBox(this);

	pBox->Execute(&pSound->WaveFile);
	delete pBox;
	TopWindow(this);
}
//---------------------------------------------------------------------------

void __fastcall TMmsstv::KFRecRewClick(TObject *Sender)
{
	if( pSound == NULL ) return;
	pSound->WaveFile.Rewind();
}
//---------------------------------------------------------------------------

void __fastcall TMmsstv::KFRecStopClick(TObject *Sender)
{
	if( pSound == NULL ) return;
	pSound->WaveFile.FileClose();
}

//---------------------------------------------------------------------------
void __fastcall TMmsstv::CopyStgBuf(void)
{
	CSSTVDEM *dp = pDem;

	while( dp->m_rPage != dp->m_wPage ){
		short *ip = &dp->m_Buf[dp->m_rPage * dp->m_BWidth];
		short *sp = &dp->m_B12[dp->m_rPage * dp->m_BWidth];
		if( dp->m_StgBuf != NULL ){
			if( ((dp->m_wStgLine + 1) * SSTVSET.m_WD) < dp->m_RxBufAllocSize ){
				memcpy(&dp->m_StgBuf[dp->m_wStgLine * SSTVSET.m_WD], ip, SSTVSET.m_WD*sizeof(short));
				memcpy(&dp->m_StgB12[dp->m_wStgLine * SSTVSET.m_WD], sp, SSTVSET.m_WD*sizeof(short));
				dp->m_wStgLine++;
			}
			else {
				break;
			}
		}
		else if( WaveStg.IsOpen() ){
			WaveStg.Write(ip, SSTVSET.m_WD*sizeof(short));
			WaveStg.Write(sp, SSTVSET.m_WD*sizeof(short));
			dp->m_wStgLine++;
		}
		if( dp->m_wStgLine == 16 ) UpdateSBTO();
		dp->m_rPage++;
		if( dp->m_rPage >= SSTVDEMBUFMAX ){
			dp->m_rPage = 0;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::CorrectSlant(void)
{
	CSSTVDEM *dp = pDem;
	if( ((dp->m_StgBuf != NULL) || WaveStg.IsOpen()) && (dp->m_wStgLine >= 16) && (SSTVSET.m_Mode != smAVT) ){
		if( (dp->m_StgBuf != NULL) && (((dp->m_wStgLine + 1) * SSTVSET.m_WD) >= dp->m_RxBufAllocSize) ){
			return;
		}
		CWaitCursor w;
		double StartSamp = SSTVSET.m_SampFreq;
		int LW = SSTVSET.m_TW * 0.1;       // 10%の揺れを許容
		int z;
		for( z = 0; z < 5; z++ ){
			// 基準位置を探す
			WaveStg.RInit();
			int i, j;
			int wd = int(SSTVSET.m_TW);
			int *bp = new int[wd];
			memset(bp, 0, sizeof(int) * wd);
			short *sp;
			int n = 0;
			for( i = 0; i < dp->m_wStgLine && (i < 32); i++ ){
				if( dp->m_StgBuf != NULL ){
					sp = &dp->m_StgB12[i * SSTVSET.m_WD];
				}
				else {
					WaveStg.Read(WaveStg.pData, SSTVSET.m_WD*sizeof(short));
					WaveStg.Read(WaveStg.pSync, SSTVSET.m_WD*sizeof(short));
					sp = WaveStg.pSync;
				}
				for( j = 0; j < SSTVSET.m_WD; j++, sp++ ){
					bp[n] += *sp;
					n++;
					if( n >= wd ) n = 0;
				}
				if( dp->m_Sync ) ::Sleep(pSound->IsBusy() ? 100 : 1);
				if( SBTX->Down && IsSR() ) SendSSTV();
			}
			double bpos = 0;
			int max = 0;
			for( i = 0; i < wd; i++ ){
				if( max < bp[i] ){
					max = bp[i];
					bpos = i;
				}
			}
			delete bp;

			// 傾き調整
			WaveStg.RInit();
			int base = 0;
			int yy;
			int y = 0;
			max = 0;
			int min = 16384;
			n = 0;
			int m = 0;
			double xx;
			double ps = 0;
			double  T, L, TT, TL;
			T = L = TT = TL = 0;
			for( i = 0; i < dp->m_wStgLine; i++ ){
				if( dp->m_StgBuf != NULL ){
					sp = &dp->m_StgB12[i * SSTVSET.m_WD];
				}
				else {
					WaveStg.Read(WaveStg.pData, SSTVSET.m_WD*sizeof(short));
					WaveStg.Read(WaveStg.pSync, SSTVSET.m_WD*sizeof(short));
					sp = WaveStg.pSync;
				}
				for( j = 0; j < SSTVSET.m_WD; j++, sp++, base++ ){
					yy = base / SSTVSET.m_TW;
					xx = fmod(base, SSTVSET.m_TW);
					if( yy != y ){
						if( bpos < 0 ){
							if( ps >= (SSTVSET.m_TW/4) ){         // 左方向への周りこみ
								ps -= SSTVSET.m_TW;
							}
							else if( ps >= (SSTVSET.m_TW/8) ){
								goto _nx;
							}
						}
						else if( bpos >= SSTVSET.m_TW ){        // 右方向への周りこみ
							if( ps < (SSTVSET.m_TW*3/4) ){
								ps += SSTVSET.m_TW;
							}
							else if( ps < (SSTVSET.m_TW*7/8) ){
								goto _nx;
							}
						}
						else if( bpos >= (SSTVSET.m_TW*3/4) ){  // 右側
							if( (ps < SSTVSET.m_TW/4) ){
								ps += SSTVSET.m_TW;
							}
						}
						else if( bpos <= (SSTVSET.m_TW/4) ){    // 左側
							if( ps >= (SSTVSET.m_TW*3/4) ){
								ps -= SSTVSET.m_TW;
							}
						}
						if( (y >= 0) && ((max - min) >= 4800) && (ABS(ps - bpos) <= LW) ){
							bpos = ps;
							if( n >= 2 ){
								T += y;
								L += ps;
								TT += y * y;
								TL += y * ps;
								m++;
							}
							n++;
							if( n >= dp->m_wStgLine ) goto _nx;
						}
						y = yy;
						max = 0;
						min = 16384;
						ps = 0;
					}
					if( max < *sp ){
						max = *sp;
						ps = xx;
					}
					if( min > *sp ){
						min = *sp;
					}
				}
				if( dp->m_Sync ) ::Sleep(pSound->IsBusy() ? 100 : 1);
				if( SBTX->Down && IsSR() ) SendSSTV();
			}
_nx:;
			double fq = SSTVSET.m_SampFreq;
			if( m >= 6 ){
				double k0 = (m * TL - L * T) / (m * TT - T*T);
				fq = SSTVSET.m_SampFreq + (k0 * SSTVSET.m_SampFreq / SSTVSET.m_TW);
#if 0
				char bf[64];
				sprintf(bf, "%lf, %lf", fq, k0 );
				EditNote->Text = bf;
#endif
				fq = NormalSampFreq(fq, 100);
			}
			if( fabs(fq-SSTVSET.m_SampFreq) < (0.1/11025.0 * SSTVSET.m_SampFreq) ){
				SSTVSET.m_SampFreq = fq;
				SSTVSET.SetSampFreq();
				break;
			}
			SSTVSET.m_SampFreq = StartSamp;
			SSTVSET.SetSampFreq();
			MultProc();
			SSTVSET.m_SampFreq = fq;
			SSTVSET.SetSampFreq();
			LW *= 0.5;
		}
		if( (SSTVSET.m_SampFreq != StartSamp) &&
			( (dp->m_StgBuf == NULL)||(((dp->m_wStgLine + 32) * SSTVSET.m_WD) < dp->m_RxBufAllocSize) )
		){
			RedrawSampFreq(FALSE);
		}
		else if( SSTVSET.m_SampFreq != StartSamp ){
			SSTVSET.m_SampFreq = StartSamp;
			SSTVSET.SetSampFreq();
		}
		DrawSlantInfo();
	}
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::AdjustSyncPos(int n)
{
		n -= SSTVSET.m_OFP;
		n = -n;
		switch(SSTVSET.m_Mode){
			case smSCT1:
			case smSCT2:
			case smSCTDX:
				if( n < 0 ) n += int(SSTVSET.m_TW);
				break;
			case smMRT1:        // 原因不明のタイミングズレを補正
			case smSC2_180:
			case smSC2_120:
			case smP3:
			case smP5:
			case smP7:
            case smMC110:
            case smMC140:
            case smMC180:
				n += (0.45/1000.0) * SSTVSET.m_SampFreq;
				break;
			case smMRT2:
			case smSC2_60:
				if( pDem->m_wStgLine < 20 ){
					n += (0.30/1000.0) * SSTVSET.m_SampFreq;
				}
				else {
					n += (0.40/1000.0) * SSTVSET.m_SampFreq;
				}
				break;
			case smR36:
			case smR72:
				n += (0.16/1000.0) * SSTVSET.m_SampFreq;
				break;
			case smMP73:
			case smMP115:
			case smMP140:
			case smMP175:
            case smMN73:
            case smMN110:
            case smMN140:
			case smMR73:
			case smMR90:
			case smMR115:
			case smMR140:
			case smMR175:
			case smML180:
			case smML240:
			case smML280:
			case smML320:
				n += (0.2/1000.0) * SSTVSET.m_SampFreq;
				break;
			case smR24:
			case smRM8:
			case smRM12:
				n += (0.5/1000.0) * SSTVSET.m_SampFreq;
				break;
			default:
				break;
		}
	return n;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::ReSyncSSTV(void)
{
	CSSTVDEM *dp = pDem;
	SSTVSET.SetSampFreq();
	if( (dp->m_StgBuf != NULL) || WaveStg.IsOpen() ){
//        int ZS = SSTVSET.m_OFS;
		WaveStg.RInit();
		int i;
		int wd = int(SSTVSET.m_TW);
		int *bp = new int[wd+2];
		memset(bp, 0, sizeof(int) * (wd+2));
		short *sp;
		int n = 0;
		for( i = 0; i < dp->m_wStgLine && (i < 32); i++ ){
			if( dp->m_StgBuf != NULL ){
				sp = &dp->m_StgB12[i * SSTVSET.m_WD];
			}
			else {
				WaveStg.Read(WaveStg.pData, SSTVSET.m_WD*sizeof(short));
				WaveStg.Read(WaveStg.pSync, SSTVSET.m_WD*sizeof(short));
				sp = WaveStg.pSync;
			}
			for( int j = 0; j < SSTVSET.m_WD; j++, sp++ ){
				int x = fmod(n, SSTVSET.m_TW);
				bp[x] += *sp;
				n++;
			}
		}
		n = 0;
		int max = 0;
		for( i = 0; i < wd; i++ ){
			if( max < bp[i] ){
				max = bp[i];
				n = i;
			}
		}
        n = AdjustSyncPos(n);
		if( dp->m_Type == 2 ) n -= dp->m_hill.m_htap/4;
		SSTVSET.m_IOFS = SSTVSET.m_OFS = dp->m_rBase = n;
		delete bp;
#if 0
		char bf[64];
		sprintf(bf, "%d, %d", ZS, SSTVSET.m_OFS);
		EditNote->Text = bf;
#endif
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DrawSlantInfo(void)
{
	double df = SSTVSET.m_SampFreq - sys.m_SampFreq;
	double m = (SSTVSET.m_SampFreq - sys.m_SampFreq) * 1000000.0 / sys.m_SampFreq;
	char bf[64];
	sprintf(bf, "%+.2lfHz (%+.0lfppm)", df, m);
	pBitmapD12->Canvas->Font->Height = -16;
	int fw = pBitmapD12->Canvas->TextWidth(bf) + 1;
	int fh = pBitmapD12->Canvas->TextHeight(bf) + 4;
	int xw = 200;
	TRect rc;
	int xl = pBitmapD12->Width - xw;
	int xr = pBitmapD12->Width - 1;
	int yt = 1;
	int yb = fh + 1;
	if( (m_AY >= 0) && (m_AY < yb) ) return;
	pBitmapD12->Canvas->Pen->Color = clBlack;
	pBitmapD12->Canvas->RoundRect(xl, yt, xr, yb, 10, 5);
	pBitmapD12->Canvas->TextOut(xl + (xw - fw)/2, yt + 2, bf);
	if( m_MainPage == pgSync) PBoxD12->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DrawShiftInfo(void)
{
	int td = SSTVSET.m_IOFS - SSTVSET.m_OFS;
    char bf[32];
    sprintf(bf, "%.2lfms", td * 1000.0 / SampFreq);
	pBitmapD12->Canvas->Font->Height = -16;
	int fw = pBitmapD12->Canvas->TextWidth(bf) + 1;
	int fh = pBitmapD12->Canvas->TextHeight(bf) + 4;
	int xw = fw + 16;
	int xl = pBitmapD12->Width - xw;
	int xr = pBitmapD12->Width - 1;
	int yt = 1;
	int yb = fh + 1;
	if( (m_AY >= 0) && (m_AY < yb) ) return;
	pBitmapD12->Canvas->Pen->Color = clBlack;
	pBitmapD12->Canvas->RoundRect(xl, yt, xr, yb, 10, 5);
	pBitmapD12->Canvas->TextOut(xl + (xw - fw)/2, yt + 2, bf);
	if( m_MainPage == pgSync) PBoxD12->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::RedrawSampFreq(int sw)
{
	CWaitCursor w;

	UpdateSampFreq();
	SSTVSET.SetSampFreq();
	DispSyncStat();
	DrawSSTV();
	if( !pDem->m_Sync ) PBoxRXPaint(NULL);
	if( sw ) DrawSlantInfo();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateSampFreq(void)
{
	CSSTVDEM *dp = pDem;
	SSTVSET.SetSampFreq();
	if( (dp->m_StgBuf != NULL) || WaveStg.IsOpen() ){
		ReSyncSSTV();
		WaveStg.RInit();
		InitAutoStop();
		m_ASDis = 1;
		dp->m_rBase = SSTVSET.m_OFS;
		for( int i = 0; i < dp->m_wStgLine; i++ ){
			if( dp->m_StgBuf != NULL ){
				DrawSSTV(&dp->m_StgBuf[i * SSTVSET.m_WD], &dp->m_StgB12[i * SSTVSET.m_WD]);
			}
			else {
				WaveStg.Read(WaveStg.pData, SSTVSET.m_WD*sizeof(short));
				WaveStg.Read(WaveStg.pSync, SSTVSET.m_WD*sizeof(short));
				DrawSSTV(WaveStg.pData, WaveStg.pSync);
			}
			dp->m_rBase += SSTVSET.m_WD;
			if( dp->m_Sync ){
				::Sleep(pSound->IsBusy() ? 100 : 1);
				CopyStgBuf();
			}
			if( SBTX->Down && IsSR() ) SendSSTV();
		}
		if( (!dp->m_Sync) && RxHist.IsAddFlag() ){
			RxHist.Update(pBitmapRX);
			::CopyBitmap(pBitmapHist, pBitmapRX);
			TrigHistF();
			UpdatePic();
		}
		DrawSync();
		pSound->fftIN.TrigFFT();
		m_ASDis = 0;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBAdjMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		RedrawAdjustSync();
    }
}
//---------------------------------------------------------------------------
#define	MAKEPOSLOG	FALSE
//---------------------------------------------------------------------------
void __fastcall TMmsstv::RedrawAdjustSync(void)
{
	CWaitCursor wc;

	CSSTVDEM *dp = pDem;
	if( (dp->m_StgBuf != NULL) || WaveStg.IsOpen() ){
		int pos, w, n, j;
		SSTVSET.GetBitmapSize(w, n, SSTVSET.m_Mode);
#if MAKEPOSLOG
		FILE *fp = fopen("\\mmsstv\\zzz.txt", "wt");
        fprintf(fp, "PixelDots=%.1lf\n", SSTVSET.m_KS / double(w) + 0.5);
#endif
		double k = 0.5;
		switch(SSTVSET.m_Mode){
			case smPD50:
            case smPD120:
            	k = 3.0;
            	break;
            case smPD160:
				k = 2.0;
            	break;
            case smRM8:
            	k = 1.5;
                break;
            case smRM12:
            case smR24:
            case smPD90:
            case smSCT2:
            case smMRT2:
				k = 1.0;
            	break;
            default:
                break;
		}
		w = SSTVSET.m_KS * k / double(w) + 0.5;
		short *pData, *pSync, *sp;
        short max;
		ReSyncSSTV();
		InitAutoStop();
		m_ASDis = 1;
		WaveStg.RInit();
		dp->m_rBase = SSTVSET.m_OFS;
		for( int i = 0; i < dp->m_wStgLine; i++ ){
			if( dp->m_StgBuf != NULL ){
				pData = &dp->m_StgBuf[i * SSTVSET.m_WD];
                pSync = &dp->m_StgB12[i * SSTVSET.m_WD];
            }
            else {
				WaveStg.Read(WaveStg.pData, SSTVSET.m_WD*sizeof(short));
				WaveStg.Read(WaveStg.pSync, SSTVSET.m_WD*sizeof(short));
                pData = WaveStg.pData;
                pSync = WaveStg.pSync;
            }
			pos = 0;
			max = 0;
			n = dp->m_rBase;
			if( n >= 0 ){
	            sp = pSync;
	            for( j = 0; j < SSTVSET.m_WD; j++, n++, sp++ ){
					if( *sp > max ){
            	        max = *sp;
						pos = fmod(n, SSTVSET.m_TW);
                	}
    	        }
	    	    pos = AdjustSyncPos(pos);
    	    	if( pos >= (int(SSTVSET.m_TW)/2) ) pos -= int(SSTVSET.m_TW);
#if MAKEPOSLOG
				fprintf(fp, "%d\n", pos);
#endif
            }
			DrawSSTV(pData, pSync);
			dp->m_rBase += SSTVSET.m_WD;
            if( (m_AY >= 2) && (ABS(pos) > w) ) dp->m_rBase += pos;
			if( dp->m_Sync ){
				::Sleep(pSound->IsBusy() ? 100 : 1);
				CopyStgBuf();
			}
        }
#if MAKEPOSLOG
		fclose(fp);
#endif
		m_ASDis = 0;
		DrawSync();
		if( (!dp->m_Sync) && RxHist.IsAddFlag() ){
			RxHist.Update(pBitmapRX);
			::CopyBitmap(pBitmapHist, pBitmapRX);
			TrigHistF();
			UpdatePic();
		}
		DrawSSTV();
		if( SSTVSET.m_SampFreq != sys.m_SampFreq ) DrawSlantInfo();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::RedrawSSTV(void)
{
	CWaitCursor w;

	CSSTVDEM *dp = pDem;
	if( (dp->m_StgBuf != NULL) || WaveStg.IsOpen() ){
			InitAutoStop();
			m_ASDis = 1;
			WaveStg.RInit();
			dp->m_rBase = SSTVSET.m_OFS;
			for( int i = 0; i < dp->m_wStgLine; i++ ){
				if( dp->m_StgBuf != NULL ){
					DrawSSTV(&dp->m_StgBuf[i * SSTVSET.m_WD], &dp->m_StgB12[i * SSTVSET.m_WD]);
				}
				else {
					WaveStg.Read(WaveStg.pData, SSTVSET.m_WD*sizeof(short));
					WaveStg.Read(WaveStg.pSync, SSTVSET.m_WD*sizeof(short));
					DrawSSTV(WaveStg.pData, WaveStg.pSync);
				}
				dp->m_rBase += SSTVSET.m_WD;
				if( dp->m_Sync ){
					::Sleep(pSound->IsBusy() ? 100 : 1);
					CopyStgBuf();
				}
			}
			m_ASDis = 0;
			DrawSync();
			if( (!dp->m_Sync) && RxHist.IsAddFlag() ){
				RxHist.Update(pBitmapRX);
				::CopyBitmap(pBitmapHist, pBitmapRX);
				TrigHistF();
				UpdatePic();
			}
			DrawSSTV();
			if( SSTVSET.m_OFS != SSTVSET.m_IOFS ) DrawShiftInfo();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::ShiftSSTV(TObject *Sender, int step)
{
	if( !step ) step++;

	if( Sender == SBPL ){
		SSTVSET.m_OFS -= step;
    }
    else {
		SSTVSET.m_OFS += step;
    }
	int e = 0;
	switch(SSTVSET.m_Mode){
		case smSCT1:
		case smSCT2:
		case smSCTDX:
			e = SSTVSET.m_WD;
			break;
		default:
			break;
	}
	while( (SSTVSET.m_OFS + SSTVSET.m_WD) < e ){
		SSTVSET.m_OFS += SSTVSET.m_WD;
	}
	RedrawSSTV();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBPLClick(TObject *Sender)
{
	int h, w;
	SSTVSET.GetBitmapSize(w, h, SSTVSET.m_Mode);
	ShiftSSTV(Sender, (SSTVSET.m_KS / double(w)) + 0.5);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBPLMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		ShiftSSTV(Sender, 1);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxD12MouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( pSound == NULL ) return;
	if( m_ReqSampChg ){             // サンプリング周波数変更要求中
		m_ReqSampChg = 0;
		RedrawSampFreq(FALSE);
		return;
	}
	CSSTVDEM *dp = pDem;
	if( SBPhase->Down ){
		if( Button != mbLeft ) return;
		CWaitCursor wait;
		SSTVSET.m_OFS -= (X * SSTVSET.m_TW / PBoxD12->Width);
		int e = 0;
		switch(SSTVSET.m_Mode){
			case smSCT1:
			case smSCT2:
			case smSCTDX:
				e = SSTVSET.m_WD;
				break;
			default:
				break;
		}
		while( (SSTVSET.m_OFS + SSTVSET.m_WD) < e ){
			SSTVSET.m_OFS += SSTVSET.m_WD;
		}
		if( (dp->m_StgBuf != NULL) || WaveStg.IsOpen() ){
#if 1
			RedrawSSTV();
#else
			InitAutoStop();
			m_ASDis = 1;
			WaveStg.RInit();
			dp->m_rBase = SSTVSET.m_OFS;
			for( int i = 0; i < dp->m_wStgLine; i++ ){
				if( dp->m_StgBuf != NULL ){
					DrawSSTV(&dp->m_StgBuf[i * SSTVSET.m_WD], &dp->m_StgB12[i * SSTVSET.m_WD]);
				}
				else {
					WaveStg.Read(WaveStg.pData, SSTVSET.m_WD*sizeof(short));
					WaveStg.Read(WaveStg.pSync, SSTVSET.m_WD*sizeof(short));
					DrawSSTV(WaveStg.pData, WaveStg.pSync);
				}
				dp->m_rBase += SSTVSET.m_WD;
				if( dp->m_Sync ){
					::Sleep(pSound->IsBusy() ? 100 : 1);
					CopyStgBuf();
				}
			}
			m_ASDis = 0;
			DrawSync();
			if( (!dp->m_Sync) && RxHist.IsAddFlag() ){
				RxHist.Update(pBitmapRX);
				::CopyBitmap(pBitmapHist, pBitmapRX);
				TrigHistF();
				UpdatePic();
			}
			DrawSSTV();
			if( SSTVSET.m_SampFreq != sys.m_SampFreq ) DrawSlantInfo();
#endif
		}
		else {
			int n = (X * SSTVSET.m_TW / PBoxD12->Width);
			while( n >= SSTVSET.m_WD ){
				n -= SSTVSET.m_WD;
			}
			while( n < 0 ){
				n += SSTVSET.m_WD;
			}
			dp->m_Skip = n;
		}
	}
	else if( SBSlant->Down ){
		if( Button == mbRight ){
			if( m_Slant ){
				m_Slant = 0;
				DrawSync();
				DispSyncStat();
			}
		}
		else if( m_Slant ){
			CWaitCursor wait;
			m_Slant = 0;
			double samp = GetSyncSamp(X, Y);
			if( samp != SSTVSET.m_SampFreq ){
				SSTVSET.m_SampFreq = samp;
				UpdateSampFreq();
			}
			SSTVSET.SetSampFreq();
			DispSyncStat();
			DrawSSTV();
			DrawSlantInfo();
		}
		else {
			m_SlantX = X;
			m_SlantY = Y;
			m_Slant++;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TMmsstv::PBoxD12Paint(TObject *Sender)
{
	DrawSync();
}
//---------------------------------------------------------------------------
TSpeedButton *__fastcall TMmsstv::GetModeBtn(int n)
{
	TSpeedButton *tp[]={
		SBM1, SBM2, SBM3, SBM4, SBM5, SBM6, SBM7, SBM8, SBM9,
	};
	return tp[n];
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::GetButtonNo(TSpeedButton *tp)
{
	for( int i = 0; i < 9; i++ ){
		if( GetModeBtn(i) == tp ){
			return i;
		}
	}
	return 8;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateModeCaption(void)
{
	int *ip = ((m_MainPage == pgTX)||(m_MainPage == pgTemp)) ? m_ModeAssignTX : m_ModeAssignRX;
	int i;
	for( i = 0; i < 9; i++, ip++ ){
		TSpeedButton *tp = GetModeBtn(i);
		if( tp->Caption != SSTVModeList[*ip] ){
			tp->Caption = SSTVModeList[*ip];
		}
	}
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::FindModeAssign(int *ip, int m)
{
	int f = -1;
	for( int i = 0; i < 9; i++, ip++ ){
	   if( *ip == m ){
			f = i;
			break;
	   }
	}
	return f;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateModeBtn(void)
{
	if( pSound == NULL ) return;

	int i, f;
	if( (m_MainPage == pgTX)||(m_MainPage == pgTemp) ){
		GBMode->Caption = "TX Mode";
		GBMode->Font->Color = clBlue;
//		SBAuto->Enabled = FALSE;
        SBAuto->GroupIndex = 1;
		SBAuto->Down = !sys.m_FixedTxMode;
		f = FindModeAssign(m_ModeAssignTX, SSTVSET.m_TxMode);
		if( f < 0 ){
			m_ModeAssignTX[8] = SSTVSET.m_TxMode;
			f = 8;
		}
		GetModeBtn(f)->Down = TRUE;
		AdjustPBoxSize((m_TXPH == 240) ? 240 : 256);
	}
	else {
		GBMode->Caption = "RX Mode";
		GBMode->Font->Color = clBlack;
//		SBAuto->Enabled = TRUE;
        SBAuto->GroupIndex = 2;
		SBAuto->Down = (pDem->m_SyncMode >= 0);
		if( pDem->m_Sync ){
			f = FindModeAssign(m_ModeAssignRX, SSTVSET.m_Mode);
			if( f < 0 ){
				m_ModeAssignRX[8] = SSTVSET.m_Mode;
				f = 8;
			}
			GetModeBtn(f)->Down = TRUE;
		}
		else if( pDem->m_SyncMode < 0 ){
			for( i = 0; i < 9; i++ ){
				GetModeBtn(i)->Down = FALSE;
			}
		}
		else {
			SBAuto->Down = TRUE;
		}
	}
	UpdateModeCaption();
	if( pCtrBtn != NULL ) pCtrBtn->UpdateBtn();
	SBFS->Enabled = pDem->m_Sync && (SSTVSET.m_Mode != smAVT) ? TRUE : FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBAFCClick(TObject *Sender)
{
	pDem->m_afc = SBAFC->Down;
	if( !pDem->m_afc ){
		pDem->InitAFC();
	}
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::GetLMSDelay(void)
{
    if( !pSound->m_lms || pSound->m_lms_AN ) return 0;
    return pSound->lms.m_Tap / 2;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBLMSClick(TObject *Sender)
{
#if 1
	// Tap数を増やした場合の処理
    // 2003/11/28に実験し、結果Tap数は従来通りの４に戻した
    // のでこの処理をキャンセルした
    int d = GetLMSDelay();
	pSound->m_lms = SBLMS->Down;
	if( pSound->m_lms && pSound->m_lms_AN ) pSound->lms.SetAN(pSound->m_lms_AN);
    if( pDem->m_Sync ) pDem->m_Skip = GetLMSDelay() - d;
#else
	pSound->m_lms = SBLMS->Down;
	if( pSound->m_lms && pSound->m_lms_AN ) pSound->lms.SetAN(pSound->m_lms_AN);
#endif
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::RxAutoPush(int sw)
{
	if( pSound == NULL ) return;

	int sc = pDem->m_Sync;
	pDem->Stop();
	if( sc ) WriteHistory(0);
	if( !sw ){
		pDem->m_SyncMode = -1;
	}
	else {
		pDem->m_SyncMode = 0;
	}
	if( (m_MainPage != pgTX) && (m_MainPage != pgTemp) ){
		SBAuto->Down = sw;
    }
	UpdateModeBtn();
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBAutoClick(TObject *Sender)
{
	if( pSound == NULL ) return;

	if( (m_MainPage == pgTX)||(m_MainPage == pgTemp) ){
		sys.m_FixedTxMode = !SBAuto->Down;
       	TrackTxMode(0);
	}
    else {
    	RxAutoPush(SBAuto->Down);
    }
#if 0
	int sc = pDem->m_Sync;
	pDem->Stop();
	if( sc ) WriteHistory(0);
	if( !SBAuto->Down ){
		pDem->m_SyncMode = -1;
	}
	else {
		pDem->m_SyncMode = 0;
	}
	UpdateModeBtn();
	UpdateUI();
#endif
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AdjustPBoxSize(int l)
{
	if( (l <= 256) && (l != PBoxTX->Height) ){
		PBoxTX->Height = l;
		PBoxTX->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBMClick(TObject *Sender)
{
	if( pSound == NULL ) return;

	TSpeedButton *tp = (TSpeedButton *)Sender;
	if( !SBTX->Down && !tp->Down && ((m_MainPage == pgTX)||(m_MainPage == pgTemp)) ){
		tp->Down = TRUE;
		m_ExtMode = GetButtonNo(tp);
		PopupM->Popup(Left + GBMode->Left + tp->Left + tp->Width/2, Top + GBMode->Top + tp->Top + tp->Height);
		return;
	}
	if( !SBTX->Down || (sys.m_echo == 2) ){
		m_ExtMode = GetButtonNo(tp);
		if( ((m_MainPage == pgTX)||(m_MainPage == pgTemp)) ){
			if( !SBTX->Down ){
				ChangeTxMode(m_ModeAssignTX[m_ExtMode]);
			}
		}
		else if( !pDem->m_Sync || (SSTVSET.m_Mode != m_ModeAssignRX[m_ExtMode] ) ){
			if( pDem->m_Sync ) WriteHistory(0);
            int mode = m_ModeAssignRX[m_ExtMode];
			pDem->Start(mode, tp->Down);
		}
	}
	UpdateModeBtn();
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::ClearDraw(Graphics::TBitmap *pBitmap, TPaintBox *pBox, TColor col)
{
	FillBitmap(pBitmap, col);
	if( pBox == NULL ) return;

	if( pBox == PBoxRX ){
		if( m_MainPage != pgRX ) return;
	}
	else if( pBox == PBoxD12 ){
		if( m_MainPage != pgSync ) return;
	}
	else if( pBox == PBoxHist ){
		if( m_MainPage != pgHist ) return;
	}
	pBox->Canvas->Draw(0, 0, pBitmap);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DrawLvl(void)
{
	if( pSound == NULL ) return;
	if( DisPaint ) return;

	TCanvas *pCanvas = pBitmapLvl->Canvas;

	TRect rc;
	int XL = 0;
	int XR = pBitmapLvl->Width - 1;
	int YT = 0;
	int YB = pBitmapLvl->Height - 1;
	rc.Left = XL;
	rc.Right = XR;
	rc.Top = YT;
	rc.Bottom = YB+1;
	pCanvas->Brush->Color = clBlack;
	pCanvas->FillRect(rc);

	double k;
	pDem->m_lvl.Fix();
	if( pDem->m_LevelType ){
		pDem->m_SyncLvl.Fix();
		k = double(YB) / 16384.0;
		rc.Top = YB - (pDem->m_SyncLvl.m_Lvl * k);
	}
	else {
		k = double(YB) / 24578.0;
		rc.Top = YB - (pDem->m_lvl.m_CurMax * k);
	}
	if( pSound->m_Tx ){
		pCanvas->Brush->Color = pDem->m_Sync ? TColor(0x00ffff00) : clYellow;
	}
	else if( pDem->m_lvl.m_CurMax >= 24578 ){
		pCanvas->Brush->Color = clRed;
	}
	else if( pDem->m_Sync ){
		pCanvas->Brush->Color = clLime;
	}
	else {
		pCanvas->Brush->Color = clGray;
	}
	if( rc.Top < 0 ) rc.Top = 0;
	pCanvas->FillRect(rc);
	if( !pDem->m_LevelType ){
		rc.Top = YB - (pDem->m_lvl.m_PeakMax * k);
		if( rc.Top < 0 ) rc.Top = 0;
		rc.Bottom = rc.Top + 1;
		if( pSound->m_Tx ){
			pCanvas->Brush->Color = clWhite;
		}
		else if( pDem->m_lvl.m_PeakMax < 24578 ){
			pCanvas->Brush->Color = clWhite;
		}
		else {
			pCanvas->Brush->Color = clRed;
		}
		pCanvas->FillRect(rc);
	}
	PBoxLvl->Canvas->Draw(0, 0, pBitmapLvl);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DispHistStat(void)
{
	if( UDHist->Position < RxHist.m_Head.m_Cnt ){
		char bf1[128], bf2[128];
		char bbf[64];
		char tm[2];
		tm[0] = 'Z';
		tm[1] = 0;
		SYSTEMTIME tim = RxHist.m_File[RxHist.m_CurRead].m_UTC;
		if( Log.m_LogSet.m_TimeZone == 'I' ){
			UTCtoJST(&tim);
			tm[0] = 0;
		}
		int mm = RxHist.m_File[RxHist.m_CurRead].m_Flag >> 16;
		if( mm ){
			mm--;
			int dm, w, h;
			SSTVSET.GetPictureSize(w, dm, h, mm);
			if( ((mm >= smR24) && (mm <= smRM12)) ){
				w /= 2;
				h /= 2;
			}
			sprintf(bbf, "%s (%ux%u)", SSTVModeList[mm], w, h);
			mm++;
		}
		else {
			strcpy(bbf, "???");
		}

		sprintf(bf1, "%s\r\n%u %04u/%02u/%02u %02u%02u%s",
            bbf,
			UDHist->Position + 1,
			tim.wYear,
			tim.wMonth,
			tim.wDay,
			tim.wHour,
			tim.wMinute,
			tm
        );
		sprintf(bf2, "%u  %04u/%02u/%02u %02u%02u%s",
			UDHist->Position + 1,
			tim.wYear,
			tim.wMonth,
			tim.wDay,
			tim.wHour,
			tim.wMinute,
			tm
		);
		HistStat->Caption = bf1;
		if( pHistView != NULL ){
			if( mm ) sprintf(&bf2[strlen(bf2)], " %s", bbf);
			pHistView->UpdateTitle(pHistView->Active ? bf2 : NULL);
		}
	}
	else {
		HistStat->Caption = "No data";
		if( pHistView != NULL ) pHistView->UpdateTitle(NULL);
	}
	SBLatest->Enabled = UDHist->Position ? TRUE : FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DispSyncStat(void)
{
	char bf[128];
	sprintf(bf, "%.2lfHz", SSTVSET.m_SampFreq);
	SyncStat->Caption = bf;
	SBSampSet->Enabled = (sys.m_SampFreq != SSTVSET.m_SampFreq) ? TRUE : FALSE;
	UpdateSBTO();
	if( pSyncView != NULL ) pSyncView->UpdateSyncTitle(SSTVSET.m_SampFreq);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateHist(void)
{
	if( UDHist->Position < RxHist.m_Head.m_Cnt ){
		int n = (RxHist.m_Head.m_wPnt - 1) - UDHist->Position;
		if( n < 0 ) n += RxHist.m_Head.m_Max;
		if( n >= RxHist.m_Head.m_Max ) n -= RxHist.m_Head.m_Max;
		RxHist.Read(pBitmapHist, n);
	}
	else {
		ClearDraw(pBitmapHist, PBoxHist, sys.m_ColorRXB);
	}
	TrigHistF();
	UpdatePic();
	DispHistStat();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UDHistClick(TObject *Sender, TUDBtnType Button)
{
	if( m_DisEvent ) return;

	if( UDHist->Position < RxHist.m_Head.m_Cnt ){
		UpdateHist();
	}
	else {
		UDHist->Position--;
	}
}
//---------------------------------------------------------------------------
// ヒストリの削除
void __fastcall TMmsstv::KHDClick(TObject *Sender)
{
	if( UDHist->Position < RxHist.m_Head.m_Cnt ){
		if( YesNoMB( MsgEng ? "Delete the picture, are you sure?" : "ほんまに消しますか？" ) == IDYES ){
			RxHist.Delete(UDHist->Position);
			if( UDHist->Position >= RxHist.m_Head.m_Cnt ) UDHist->Position--;
			UpdateHist();
			if( pHistView != NULL ){
				pHistView->LoadHist(0);
			}
		}
	}
}
//---------------------------------------------------------------------------
// 全ヒストリの削除
void __fastcall TMmsstv::KHDAClick(TObject *Sender)
{
	if( RxHist.m_Head.m_Cnt ){
		if( YesNoMB( MsgEng ? "Delete all pictures, are you sure?" : "全ての履歴を削除します." ) == IDYES ){
			RxHist.DeleteAll();
			UDHist->Position = 0;
			UpdateHist();
			if( pHistView != NULL ) pHistView->LoadHist(0);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DateBitmap(Graphics::TBitmap *pBitmap, SYSTEMTIME &tim)
{
	pBitmap->Canvas->Font->Name = Font->Name;
	pBitmap->Canvas->Font->Charset = Font->Charset;
	pBitmap->Canvas->Font->Size = 8;
	pBitmap->Canvas->Font->Color = clWhite;
	pBitmap->Canvas->Brush->Color = clBlack;
	char bf[64];
	switch(Log.m_LogSet.m_DateType){
		case 2:
		case 3:
			sprintf(bf, "%02u-%s-%04u %02u%02u", tim.wDay, MONT1[tim.wMonth], tim.wYear, tim.wHour, tim.wMinute);
			break;
		case 4:
		case 5:
			sprintf(bf, "%s-%02u-%04u %02u%02u", MONT1[tim.wMonth], tim.wDay, tim.wYear, tim.wHour, tim.wMinute);
			break;
		default:
			sprintf(bf, "%04u-%s-%02u %02u%02u", tim.wYear, MONT1[tim.wMonth], tim.wDay, tim.wHour, tim.wMinute);
			break;
	}
	int y = pBitmap->Height - pBitmap->Canvas->TextHeight(bf) - 1;
	pBitmap->Canvas->TextOut(0, y, bf);
}
//---------------------------------------------------------------------------
// ヒストリのファイルへのセーブ
void __fastcall TMmsstv::KHSClick(TObject *Sender)
{
	char bf[128];
	bf[0] = 0;
	if( UDHist->Position < RxHist.m_Head.m_Cnt ){
		SYSTEMTIME tim = RxHist.m_File[RxHist.m_CurRead].m_UTC;
		if( Log.m_LogSet.m_TimeZone == 'I' ){
			UTCtoJST(&tim);
		}
		sprintf(bf, "%04u%02u%02u%02u%02u",
			tim.wYear,
			tim.wMonth,
			tim.wDay,
			tim.wHour,
			tim.wMinute
		);
		if( Sender == KHT ){
			Graphics::TBitmap *pBitmap = DupeBitmap(pBitmapHist, pf24bit);
			DateBitmap(pBitmap, tim);
			SaveBitmapMenu(pBitmap, bf, NULL);
			delete pBitmap;
		}
		else {
			SaveBitmapMenu(pBitmapHist, bf, NULL);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxHistPaint(TObject *Sender)
{
	DrawBitmap(PBoxHist, pBitmapHist);
	if( sys.m_ShowSizeHist ){
		ShowSize(PBoxHist, pBitmapHist->Width, pBitmapHist->Height, 12, clGreen);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBLatestClick(TObject *Sender)
{
	if( UDHist->Position ){
		UDHist->Position = 0;
		UpdateHist();
	}
}
//---------------------------------------------------------------------------
double __fastcall TMmsstv::GetSyncSamp(int X, int Y)
{
	if( m_SlantY == Y ) return SSTVSET.m_SampFreq;
	double d = (X - m_SlantX);
	d = d * SSTVSET.m_SampFreq / pBitmapD12->Width;
	d /= (m_SlantY - Y);
	d = SSTVSET.m_SampFreq - d;
	d = NormalSampFreq(d, 10);
	if( d > (SampFreq * 1100/1060) ){
		d = SampFreq * 1100/1060;
	}
	return d;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DrawSync(void)
{
	if( m_MainPage != pgSync ) return;

	PBoxD12->Canvas->Draw(0, 0, pBitmapD12);
	if( m_Slant ){
		PBoxD12->Canvas->Pen->Color = clBlue;
		PBoxD12->Canvas->Pen->Style = psSolid;
		PBoxD12->Canvas->MoveTo(m_SlantX, m_SlantY);
		int Sop = ::SetROP2(PBoxD12->Canvas->Handle, R2_MERGEPENNOT);
		PBoxD12->Canvas->LineTo(m_SlantX2, m_SlantY2);
		::SetROP2(PBoxD12->Canvas->Handle, Sop);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxD12MouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if( m_Slant ){
		m_SlantX2 = X;
		m_SlantY2 = Y;
		DrawSync();
		char bf[32];
		sprintf(bf, "%.2lfHz", GetSyncSamp(X, Y));
		SyncStat->Caption = bf;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBSampSetClick(TObject *Sender)
{
	sys.m_SampFreq = SSTVSET.m_SampFreq;
	SBSampSet->Enabled = FALSE;
	InfoSampFreq();
	SSTVSET.InitIntervalPara();
	UpdateSBTO();
	DrawSlantInfo();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBPasteClick(TObject *Sender)
{
	if( PasteBitmap(pBitmapTXM, (Sender == KEX) ? 2 : 0) == TRUE ){
		AdjustPage(pgTX);
		MakeTxBitmap(0);
		PBoxTXPaint(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::ShowSize(TPaintBox *pBox, int w, int h, int fs, TColor col)
{
	pBox->Canvas->Brush->Color = col;
	pBox->Canvas->Font->Color = clWhite;
	pBox->Canvas->Font->Height = -fs;
	char bf[64];
	sprintf(bf, "%ux%u", w, h);
	int fh = pBox->Canvas->TextHeight(bf);
	pBox->Canvas->TextOut(0, pBox->Height - fh, bf);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::ShowSize(Graphics::TBitmap *pBitmap, int x, int y, int w, int h, int fs, TColor col)
{
	pBitmap->Canvas->Brush->Color = col;
	pBitmap->Canvas->Font->Color = clWhite;
	pBitmap->Canvas->Font->Height = -fs;
	char bf[64];
	sprintf(bf, "%ux%u", w, h);
	int fh = pBitmap->Canvas->TextHeight(bf);
	pBitmap->Canvas->TextOut(x, y - fh, bf);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::ShowNo(Graphics::TBitmap *pBitmap, int x, int y, int n, int fs, TColor col)
{
	pBitmap->Canvas->Brush->Color = col;
	pBitmap->Canvas->Font->Color = clBlack;
	pBitmap->Canvas->Font->Height = -fs;
	char bf[64];
	sprintf(bf, "%u", n);
	pBitmap->Canvas->TextOut(x+1, y+1, bf);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxTXPaint(TObject *Sender)
{
	if( m_MainPage != pgTX ) return;

	DrawBitmap(PBoxTX, pBitmapTX);
	m_SendY = -1;
	if( sys.m_ShowSizeTX ){
		ShowSize(PBoxTX, m_TXW, m_TXPH, 12, clGreen);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBCopyClick(TObject *Sender)
{
	CopyBitmap(pBitmapHist);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRCCClick(TObject *Sender)
{
	ClearDraw(pBitmapRX, PBoxRX, sys.m_ColorRXB);
	ClearDraw(pBitmapD12, PBoxD12, clWhite);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LineR24(CSSTVMOD *mp)
{
	int x;
	int Y, RY[320], BY[320];
	mp->Write(1200, 6);
	mp->Write(1500, 2);
	for( x = 0; x < 320; x++ ){     // Y
		GetRY(Y, RY[x], BY[x], pBitmapTX->Canvas->Pixels[x][mp->m_wLine]);
		mp->Write(short(ColorToFreq(Y)), 92.0/320.0);
	}
	mp->Write(1500, 3.0);
	mp->Write(1900, 1.0);
	for( x = 0; x < 320; x++ ){     // R-Y
		mp->Write(short(ColorToFreq(RY[x])), 46.0/320.0);
	}
	mp->Write(2300, 3.0);
	mp->Write(1900, 1.0);
	for( x = 0; x < 320; x++ ){     // B-Y
		mp->Write(short(ColorToFreq(BY[x])), 46.0/320.0);
	}
	mp->m_wLine++;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LineR36(CSSTVMOD *mp)
{
	int x;
	int Y, RY[320], BY[320];
	mp->Write(1200, 9);
	mp->Write(1500, 3);
	for( x = 0; x < 320; x++ ){     // Y
		GetRY(Y, RY[x], BY[x], pBitmapTX->Canvas->Pixels[x][mp->m_wLine]);
		mp->Write(short(ColorToFreq(Y)), 88.0/320.0);
	}
	mp->Write(short(mp->m_wLine & 1 ? 2300 : 1500), 4.5);   // RY=1500, BY=2300
	mp->Write(1900, 1.5);
	for( x = 0; x < 320; x++ ){     // DeffColor
		if( !(mp->m_wLine & 1) ){      // R-Y
			Y = RY[x];
		}
		else {                      // B-Y
			Y = BY[x];
		}
		mp->Write(short(ColorToFreq(Y)), 44.0/320.0);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LineR72(CSSTVMOD *mp)
{
	int x;
	int Y, RY[320], BY[320];
	mp->Write(1200, 9);
	mp->Write(1500, 3);
	for( x = 0; x < 320; x++ ){     // Y
		GetRY(Y, RY[x], BY[x], pBitmapTX->Canvas->Pixels[x][mp->m_wLine]);
		mp->Write(short(ColorToFreq(Y)), 138.0/320.0);
	}
	mp->Write(1500, 4.5);
	mp->Write(1900, 1.5);
	for( x = 0; x < 320; x++ ){     // R-Y
		mp->Write(short(ColorToFreq(RY[x])), 69.0/320.0);
	}
	mp->Write(2300, 4.5);
	mp->Write(1900, 1.5);
	for( x = 0; x < 320; x++ ){     // B-Y
		mp->Write(short(ColorToFreq(BY[x])), 69.0/320.0);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LineAVT(CSSTVMOD *mp)
{
	int x;
	COLD    col[320];
	COLD    *cp;
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // R
		cp->c = pBitmapTX->Canvas->Pixels[x][mp->m_wLine];
		mp->Write(short(ColorToFreq(cp->b.r)+0x1000), 125.0/320.0);
	}
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // G
		mp->Write(short(ColorToFreq(cp->b.g)+0x2000), 125.0/320.0);
	}
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // B
		mp->Write(short(ColorToFreq(cp->b.b)+0x3000), 125.0/320.0);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LineSCT(CSSTVMOD *mp, double tw)
{
	int x;
	COLD    col[320];
	COLD    *cp;
	mp->Write(1500+0x2000, 1.5);
	tw /= 320.0;
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // G
		cp->c = pBitmapTX->Canvas->Pixels[x][mp->m_wLine];
		mp->Write(short(ColorToFreq(cp->b.g)+0x2000), tw);
	}
	mp->Write(1500+0x3000, 1.5);
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // B
		mp->Write(short(ColorToFreq(cp->b.b)+0x3000), tw);
	}
	mp->Write(1200, 9);
	mp->Write(1500+0x1000, 1.5);
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // R
		mp->Write(short(ColorToFreq(cp->b.r)+0x1000), tw);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LineMRT(CSSTVMOD *mp, double tw)
{
	int x;
	COLD    col[320];
	COLD    *cp;
	mp->Write(1200, 4.862);
	mp->Write(1500+0x2000, 0.572);
	tw /= 320.0;
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // G
		cp->c = pBitmapTX->Canvas->Pixels[x][mp->m_wLine];
		mp->Write(short(ColorToFreq(cp->b.g)+0x2000), tw);
	}
	mp->Write(1500+0x3000, 0.572);
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // B
		mp->Write(short(ColorToFreq(cp->b.b)+0x3000), tw);
	}
	mp->Write(1500+0x1000, 0.572);
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // R
		mp->Write(short(ColorToFreq(cp->b.r)+0x1000), tw);
	}
	mp->Write(1500, 0.572);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LineSC2180(CSSTVMOD *mp, double S, double tw)
{
// 0.711043693634973869144258914710309
	int x;
	COLD    col[320];
	COLD    *cp;
	mp->Write(1200, S);
	mp->Write(1500+0x1000, 0.5);
	tw /= 320.0;
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // R
		cp->c = pBitmapTX->Canvas->Pixels[x][mp->m_wLine];
		mp->Write(short(ColorToFreq(cp->b.r)+0x1000), tw);
	}
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // G
		mp->Write(short(ColorToFreq(cp->b.g)+0x2000), tw);
	}
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // B
		mp->Write(short(ColorToFreq(cp->b.b)+0x3000), tw);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LinePD(CSSTVMOD *mp, double tw)
{
	int x;
	int Y, RY[800], BY[800];
	mp->Write(1200, 20.000);
	mp->Write(1500, 2.080);
	tw /= double(pBitmapTX->Width);
	for( x = 0; x < pBitmapTX->Width; x++ ){     // Y(odd)
		GetRY(Y, RY[x], BY[x], pBitmapTX->Canvas->Pixels[x][mp->m_wLine]);
		mp->Write(short(ColorToFreq(Y)), tw);
	}
	for( x = 0; x < pBitmapTX->Width; x++ ){     // R-Y
		mp->Write(short(ColorToFreq(RY[x])), tw);
	}
	for( x = 0; x < pBitmapTX->Width; x++ ){     // B-Y
		mp->Write(short(ColorToFreq(BY[x])), tw);
	}
	mp->m_wLine++;
	for( x = 0; x < pBitmapTX->Width; x++ ){     // Y(even)
		GetRY(Y, RY[0], BY[0], pBitmapTX->Canvas->Pixels[x][mp->m_wLine]);
		mp->Write(short(ColorToFreq(Y)), tw);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LineP(CSSTVMOD *mp, double S, double P, double C)
{
	int x;
	COLD    col[640];
	COLD    *cp;
	mp->Write(1200, S);
	mp->Write(1500+0x1000, P);
	double tw = C / 640.0;
	for( cp = col, x = 0; x < 640; x++, cp++ ){     // R
		cp->c = pBitmapTX->Canvas->Pixels[x][mp->m_wLine];
		mp->Write(short(ColorToFreq(cp->b.r)+0x1000), tw);
	}
	mp->Write(1500+0x2000, P);
	for( cp = col, x = 0; x < 640; x++, cp++ ){     // G
		mp->Write(short(ColorToFreq(cp->b.g)+0x2000), tw);
	}
	mp->Write(1500+0x3000, P);
	for( cp = col, x = 0; x < 640; x++, cp++ ){     // B
		mp->Write(short(ColorToFreq(cp->b.b)+0x3000), tw);
	}
	mp->Write(1500, P);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LineMP(CSSTVMOD *mp, double tw)
{
	int x;
	int Y, RY[320], BY[320];
	mp->Write(1200, 9.000);
	mp->Write(1500, 1.000);
	tw /= double(pBitmapTX->Width);
	for( x = 0; x < pBitmapTX->Width; x++ ){     // Y(odd)
		GetRY(Y, RY[x], BY[x], pBitmapTX->Canvas->Pixels[x][mp->m_wLine]);
		mp->Write(short(ColorToFreq(Y)), tw);
	}
	for( x = 0; x < pBitmapTX->Width; x++ ){     // R-Y
		mp->Write(short(ColorToFreq(RY[x])), tw);
	}
	for( x = 0; x < pBitmapTX->Width; x++ ){     // B-Y
		mp->Write(short(ColorToFreq(BY[x])), tw);
	}
	mp->m_wLine++;
	for( x = 0; x < pBitmapTX->Width; x++ ){     // Y(even)
		GetRY(Y, RY[0], BY[0], pBitmapTX->Canvas->Pixels[x][mp->m_wLine]);
		mp->Write(short(ColorToFreq(Y)), tw);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LineMR(CSSTVMOD *mp, double tw)
{
	int x;
	int Y, RY[640], BY[640];
	int TXW = pBitmapTX->Width;
	mp->Write(1200, 9);
	mp->Write(1500, 1);
	double ty = tw / double(TXW);
	double tc = ty / 2.0;
	short d;
	for( x = 0; x < TXW; x++ ){     // Y
		GetRY(Y, RY[x], BY[x], pBitmapTX->Canvas->Pixels[x][mp->m_wLine]);
		d = short(ColorToFreq(Y));
		mp->Write(d, ty);
	}
	mp->Write(d, 0.1);
	for( x = 0; x < TXW; x++ ){     // R-Y
		d = short(ColorToFreq(RY[x]));
		mp->Write(d, tc);
	}
	mp->Write(d, 0.1);
	for( x = 0; x < TXW; x++ ){     // B-Y
		d = short(ColorToFreq(BY[x]));
		mp->Write(d, tc);
	}
	mp->Write(d, 0.1);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LineRM(CSSTVMOD *mp, double ts, double tw)
{
	int x;
	int Y[320], YY, RY, BY;
	mp->Write(1200, ts);
	mp->Write(1500, ts/3.0);
	tw /= 320.0;
	for( x = 0; x < 320; x++ ){     // Y
		GetRY(Y[x], RY, BY, pBitmapTX->Canvas->Pixels[x][mp->m_wLine]);
	}
	mp->m_wLine++;
	for( x = 0; x < 320; x++ ){     // Y
		GetRY(YY, RY, BY, pBitmapTX->Canvas->Pixels[x][mp->m_wLine]);
		YY = (YY + Y[x]) / 2;
		mp->Write(short(ColorToFreq(YY)), tw);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LineMN(CSSTVMOD *mp, double tw)
{
	int x;
	int Y, RY[320], BY[320];
	mp->Write(NARROW_SYNC, 9.000);
	mp->Write(NARROW_LOW, 1.000);
	tw /= double(pBitmapTX->Width);
	for( x = 0; x < pBitmapTX->Width; x++ ){     // Y(odd)
		GetRY(Y, RY[x], BY[x], pBitmapTX->Canvas->Pixels[x][mp->m_wLine]);
		mp->Write(short(ColorToFreqNarrow(Y)), tw);
	}
	for( x = 0; x < pBitmapTX->Width; x++ ){     // R-Y
		mp->Write(short(ColorToFreqNarrow(RY[x])), tw);
	}
	for( x = 0; x < pBitmapTX->Width; x++ ){     // B-Y
		mp->Write(short(ColorToFreqNarrow(BY[x])), tw);
	}
	mp->m_wLine++;
	for( x = 0; x < pBitmapTX->Width; x++ ){     // Y(even)
		GetRY(Y, RY[0], BY[0], pBitmapTX->Canvas->Pixels[x][mp->m_wLine]);
		mp->Write(short(ColorToFreqNarrow(Y)), tw);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LineMC(CSSTVMOD *mp, double tw)
{
	int x;
	COLD    col[320];
	COLD    *cp;
	mp->Write(NARROW_SYNC, 8.0);
	mp->Write(NARROW_LOW, 0.5);
	tw /= 320.0;
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // R
		cp->c = pBitmapTX->Canvas->Pixels[x][mp->m_wLine];
		mp->Write(short(ColorToFreqNarrow(cp->b.r)), tw);
	}
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // G
		mp->Write(short(ColorToFreqNarrow(cp->b.g)), tw);
	}
	for( cp = col, x = 0; x < 320; x++, cp++ ){     // B
		mp->Write(short(ColorToFreqNarrow(cp->b.b)), tw);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OutputMMV(void)
{
	CSSTVMOD *mp = pMod;
	if( mp->GetRowCnt() ) return;
	CWaitCursor wait;
	BYTE    head[4];

	FILE *fp = fopen(sys.m_MMVID.c_str(), "rb");
	if( fp != NULL ){
		int len = filelength(fileno(fp));
		if( (len >= 4) && (fread(head, 1, 4, fp) == 4) ){
			int Samp;
			if( (head[0] == 0x55) && (head[1] == 0xaa) ){
				len -= 4;
				Samp = head[2];
				if( Samp > 8 ) Samp = 0;
			}
			else {
				fseek(fp, 0L, SEEK_SET);
				Samp = head[0];
			}
			if( m_pRowBuf != NULL ) delete m_pRowBuf;
			BYTE *bp = new BYTE[len+2];
			int pos = 0;
			while(len > 0){
				int rlen = fread(&bp[pos], 1, 16384, fp);
				len -= rlen;
				pos += rlen;
				if( rlen < 16384 ) break;
			}
			if( Samp != SampType ){
				double sfq = SampTable[Samp];
				len = int(double(pos) * SampBase / sfq);
				len &= 0xfffffffe;
				m_pRowBuf = new BYTE[len+2];
				short *tp = (short *)m_pRowBuf;
				short *sp = (short *)bp;
				len /= 2;
				int i, r;
				CIIR    iir;
				iir.MakeIIR(2700, SampBase, 4, 0, 0);
				for( i = 0; i < len; i++ ){
					r = int(double(i) * sfq / SampBase);
					*tp++ = iir.Do(sp[r]);
				}
				pos = len * 2;
				delete bp;
			}
			else {
				m_pRowBuf = bp;
			}
			mp->SetRow((short *)m_pRowBuf, pos/2);
		}
		fclose(fp);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OutputFSKID(void)
{
	CSSTVMOD *mp = pMod;
	CWaitCursor wait;

	char bf[128];
	LPSTR p;
   	strcpy(bf, sys.m_Call.c_str()); p = bf;
	mp->Write(FSKSPACE, FSKGARD);
	mp->Write(1900, FSKINTVAL);
	mp->WriteFSK(0x2a);     // STX
	BYTE s = 0;
    BYTE c;
	for(; *p; p++ ){
		while(mp->GetBufCnt() >= ((mp->m_TXBufLen-2) * SampFreq)) ::Sleep(1000);
		c = BYTE((*p) - 0x20);
		s = BYTE(c ^ s);
		mp->WriteFSK(c);
	}
	mp->WriteFSK(0x01);     // EOT
	mp->WriteFSK(s);        // check sum

	if( Log.m_LogSet.m_FSKNR ){
		char ubf[256];
		strcpy(ubf, AnsiString(HisRST->Text).c_str());	//ja7ude 0428
        LPCSTR sp = ubf;
        for(p = bf; *sp; *sp++ ){
			if( *sp >= '0' ) *p++ = *sp;
        }
        *p = 0;
        p = bf;
   		while(mp->GetBufCnt() >= ((mp->m_TXBufLen-2) * SampFreq)) ::Sleep(1000);
		if( strlen(p) > 3 ){
			p += 3;
	        int l = strlen(p);
	        int d;
			if( !IsAlphas(p) && (l >= 3) && (sscanf(p, "%u", &d) == 1) && (d < 4096) && ((l < 4)||(d >= 1000)) ){
				s = 0x02;
				mp->WriteFSK(s);
	            c = BYTE((d >> 6) & 0x3f);
	            s = s ^ c;
	            mp->WriteFSK(c);
	            c = BYTE(d & 0x3f);
	            s = s ^ c;
	            mp->WriteFSK(c);
	        }
			else {
		    	jstrupr(p);
				s = 0;
				for(; *p; p++ ){
					while(mp->GetBufCnt() >= ((mp->m_TXBufLen-2) * SampFreq)) ::Sleep(1000);
					c = BYTE((*p) - 0x20);
					s = BYTE(c ^ s);
					mp->WriteFSK(c);
				}
				mp->WriteFSK(0x01);     // EOT
    	    }
			mp->WriteFSK(s);        // check sum
        }
    }
	mp->Write(FSKSPACE, FSKGARD);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OutputCWID(void)
{
	if( !sys.m_CWIDText.IsEmpty() ){
		CWaitCursor wait;
		CSSTVMOD *mp = pMod;
		char bf[80];
		bf[0] = '@';
		MacroText(&bf[1], sys.m_CWIDText.c_str(), sizeof(bf)-2);
		LPCSTR p = bf;
		for(; *p; p++ ){
			while(mp->GetBufCnt() >= ((mp->m_TXBufLen-2) * SampFreq)) ::Sleep(1000);
			mp->WriteCWID(*p);
		}
    }
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::IsSR(void)
{
	return pMod->GetBufCnt() < ((pMod->m_TXBufLen-2) * SampFreq);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SendSSTV(void)
{
	if( !SBTX->Down ) return;

	CSSTVMOD *mp = pMod;
	if( IsSR() ){
		if( mp->m_wLine >= SSTVSET.m_TL ){
			if( mp->m_wLine == SSTVSET.m_TL ){
				// MMSSTV フッター
				if( !sys.m_TXFSKID ){			// FSKIDなし
					double tw = SampFreq/2;
					if( !sys.m_VOX && !SSTVSET.m_fTxNarrow ){
						mp->WriteC(1500, SSTVSET.m_TW > tw ? tw : SSTVSET.m_TW);
						mp->Write(1900, 100);
						mp->Write(1500, 100);
						mp->Write(1900, 100);
						mp->Write(1500, 100);
                    }
                    else {
						mp->WriteC(1900, SSTVSET.m_TW > tw ? tw : SSTVSET.m_TW);
                    }
				}
				else {							// FSKIDあり
					mp->Write(WORD(SSTVSET.m_fTxNarrow ? 1900 : 1500), 300);
				}
				mp->m_wLine++;
			}
			else if( mp->m_wLine == (SSTVSET.m_TL + 1) ){
				if( mp->GetBufCnt() < (2 * SampFreq) ){
					if( sys.m_TXFSKID && !sys.m_Call.IsEmpty() ){
						OutputFSKID();
					}
					if( sys.m_CWID == 1 ){
						OutputCWID();
					}
					else if( (sys.m_CWID == 2) && !sys.m_MMVID.IsEmpty() ){
						OutputMMV();
					}
					mp->m_wLine++;
				}
			}
			else if( mp->m_wLine == (SSTVSET.m_TL + 2) ){
				if( !mp->m_Cnt && !mp->m_RowCnt ){
					pSound->TrigBCC();
					mp->m_wLine++;
				}
			}
			else if( pSound->GetBCC() < 0 ){
				int lost = mp->m_Lost;
				ToRX();
				if( lost ) InfoTxLost();
				if( KSRR->Checked && (m_MainPage == pgTX) ){
					AdjustPage(pgRX);
				}
				return;
			}
#if 0
			else if( !mp->m_Cnt && !mp->m_RowCnt ){
				int lost = mp->m_Lost;
//                ::Sleep( (pSound->m_BuffSize * pSound->m_FifoSize * 1000) / SSTVSET.m_TxSampFreq ); // バッファ送信完了待ち
				ToRX();
				if( lost ) InfoTxLost();
				return;
			}
#endif
		}
		else {
			while( (mp->GetBufCnt() < ((mp->m_TXBufLen-2) * SampFreq)) && (mp->m_wLine < SSTVSET.m_TL) ){
				switch(SSTVSET.m_TxMode){
					case smR36:
						LineR36(mp);
						break;
					case smR72:
						LineR72(mp);
						break;
					case smAVT:
						LineAVT(mp);
						break;
					case smSCT1:
						LineSCT(mp, 138.24);
						break;
					case smSCT2:
						LineSCT(mp, 88.064);
						break;
					case smSCTDX:
						LineSCT(mp, 345.6);
						break;
					case smMRT1:
						LineMRT(mp, 146.432);
						break;
					case smMRT2:
						LineMRT(mp, 73.216);
						break;
					case smSC2_180:
						LineSC2180(mp, 5.5437, 235.0);
						break;
					case smSC2_120:
						LineSC2180(mp, 5.52248, 156.5);
						break;
					case smSC2_60:
						LineSC2180(mp, 5.5006, 78.128); // 6.0006
						break;
					case smPD50:
						LinePD(mp, 91.520);
						break;
					case smPD90:
						LinePD(mp, 170.240);
						break;
					case smPD120:
						LinePD(mp, 121.600);
						break;
					case smPD160:
						LinePD(mp, 195.584);
						break;
					case smPD180:
						LinePD(mp, 183.040);
						break;
					case smPD240:
						LinePD(mp, 244.480);
						break;
					case smPD290:
						LinePD(mp, 228.800);
						break;
					case smP3:
						LineP(mp, 5.208, 1.042, 133.333);
						break;
					case smP5:
						LineP(mp, 7.813, 1.562375, 200.000);
						break;
					case smP7:
						LineP(mp, 10.417, 2.083, 266.667);
						break;
					case smMR73:
						LineMR(mp, 138.0);
						break;
					case smMR90:
						LineMR(mp, 171.0);
						break;
					case smMR115:
						LineMR(mp, 220.0);
						break;
					case smMR140:
						LineMR(mp, 269.0);
						break;
					case smMR175:
						LineMR(mp, 337.0);
						break;
					case smMP73:
						LineMP(mp, 140.0);
						break;
					case smMP115:
						LineMP(mp, 223.0);
						break;
					case smMP140:
						LineMP(mp, 270.0);
						break;
					case smMP175:
						LineMP(mp, 340.0);
						break;
					case smML180:
						LineMR(mp, 176.5);
						break;
					case smML240:
						LineMR(mp, 236.5);
						break;
					case smML280:
						LineMR(mp, 277.5);
						break;
					case smML320:
						LineMR(mp, 317.5);
						break;
					case smR24:
						LineR24(mp);
						break;
					case smRM8:
						LineRM(mp, 6.0, 58.89709);
						break;
					case smRM12:
						LineRM(mp, 6.0, 92.0);
						break;
					case smMN73:
						LineMN(mp, 140.0);
						break;
					case smMN110:
						LineMN(mp, 212.0);
						break;
					case smMN140:
						LineMN(mp, 270.0);
						break;
					case smMC110:
						LineMC(mp, 140.0);
						break;
					case smMC140:
						LineMC(mp, 180.0);
						break;
					case smMC180:
						LineMC(mp, 232.0);
						break;
				}
				mp->m_wLine++;
			}
		}
	}
	// 送信位置マーカー
	if( mp->m_sCnt < mp->m_rCnt ){
		int l = mp->m_rCnt - mp->m_sCnt;
		if( mp->m_wLine >= (SSTVSET.m_TL + 3) ){
			l -= pSound->GetOutCount(1);
		}
		else {
			l -= pSound->GetOutCount(0);
		}
		l /= SSTVSET.m_TWD;
//        int l = (mp->m_rCnt - mp->m_sCnt - ) / SSTVSET.m_TWD;
		switch(SSTVSET.m_TxMode){
			case smR24:
			case smRM8:
			case smRM12:
			case smPD50:
			case smPD90:
			case smMP73:
			case smMP115:
			case smMP140:
			case smMP175:
            case smMN73:
            case smMN110:
            case smMN140:
				l *= 2;
				break;
			case smPD120:
			case smPD180:
			case smPD240:
				l = l * 512 / 496;
				if( l > 256 ) l = 256;
				break;
			case smPD160:
				l = l * 512 / 400;
				if( l > 256 ) l = 256;
				break;
			case smPD290:
				l = l * 512 / 616;
				if( l > 256 ) l = 256;
				break;
			case smP3:
			case smP5:
			case smP7:
			case smML180:
			case smML240:
			case smML280:
			case smML320:
				l = l * 256 / 496;
				if( l > 256 ) l = 256;
				break;
		}
		if( l > SSTVSET.m_TL ) l = SSTVSET.m_TL;
		if( l < 0 ) l = 0;
		DrawTXCursor(l);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateTXUI(void)
{
	int f = SBTX->Down ? FALSE : TRUE;
	SBTune->Enabled = f;
    SBCW->Enabled = f;
//    SBUseTemp->Enabled = f;
	SBUA240->Enabled = (m_TXW == 320) ? f : FALSE;
	SBUS240->Enabled = (m_TXW == 320) ? f : FALSE;
	SBDA240->Enabled = f;
	SBDS240->Enabled = f;
	SBTXFil->Enabled = f;
	SBPaste->Enabled = f && ::IsClipboardFormatAvailable(CF_BITMAP);
	if( pCtrBtn != NULL ) pCtrBtn->UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OutHEAD(void)
{
	CSSTVMOD *mp = pMod;

	switch(sys.m_VOX){
		case 0:
			if( SSTVSET.m_fTxNarrow ){
				mp->Write(1900, 100);
				mp->Write(2300, 100);
				mp->Write(1900, 100);
				mp->Write(2300, 100);
            }
            else {
				mp->Write(1900, 100);
				mp->Write(1500, 100);
				mp->Write(1900, 100);
				mp->Write(1500, 100);
				mp->Write(2300, 100);
				mp->Write(1500, 100);
				mp->Write(2300, 100);
				mp->Write(1500, 100);
            }
        	break;
		case 1:
			if( !sys.m_VOXSound.IsEmpty() ){
				CTextString cs(sys.m_VOXSound.c_str());
				char bf[1024];
				if( cs.LoadText(bf, sizeof(bf)-1) ){
					DeleteComment(bf);
					if( bf[0] == '#' ){
						if( !strcmp(&bf[1], "id") ){
							mp->Write(WORD(SSTVSET.m_fTxNarrow ? 1900 : 1500), 300);
							OutputFSKID();
							break;
						}
						else if( !strcmp(&bf[1], "cw") ){
							OutputCWID();
                            break;
                        }
						else {
							int n = atoin(&bf[1], -1);
							while(n--){
								if( !cs.LoadText(bf, sizeof(bf)-1) ) break;
							}
							DeleteComment(bf);
						}
					}
					LPSTR t, p;
					p = bf;
					int ttm = 0;
					int lmt = (SSTVSET.m_TxMode == smAVT) ? 1800 : 8000;
					while(*p){
						p = StrDlm(t, p);
						if( *t ){
							int fq = atoin(t, -1);
							if( fq < 0 ) fq = 0;
							if( fq > 2800 ) fq = 2800;
							p = StrDlm(t, p);
							int tm = atoin(t, -1);
							if( tm <= 0 ) tm = 100;
							ttm += tm;
							if( ttm > lmt ){
								ttm -= tm;
								tm = lmt - ttm;
								if( tm > 0 ) mp->Write(short(fq), tm);
								break;
							}
							else {
								mp->Write(short(fq), tm);
							}
						}
						else {
							break;
						}
					}
				}
			}
			break;
		default:
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::ToTX(void)
{
	CWaitCursor wait;

	m_TuneTimer = 0;
	if( !sys.m_echo ){
		int f = pDem->m_Sync;
		pDem->Stop();
		pDem->FreeRxBuff();
		if( f ) WriteHistory(0);
	}
	if( m_ReqHistF || m_ChangeTemp || DrawMain.IsTimeMacro() ){
		if( m_ReqHistF ) AdjustHistF();
		UpdatePic();
	}
	int rep = (sys.m_Repeater && pDem->m_Repeater) ? 1 : 0;
	if( SBUseTemp->Down && DrawMain.m_Cnt && (pBitmapTXM->Width != DrawMain.m_SX) && (!rep) ){
		SBTX->Down = FALSE;
		if(
			YesNoMB( MsgEng ?
			"The template image quality is no good, because the size of template is different from the TX image.\r\n\r\nContinue processing?" :
			"テンプレートのサイズが送信画像と異なるためテンプレートの画質は良くありません.\r\n\r\nこのまま送信しますか？" ) == IDNO
		){
			return;
		}
		SBTX->Down = TRUE;
	}

	m_SendY = -1;
	SBTX->Down = 1;
	UpdateTXUI();
	CSSTVMOD *mp = pMod;
	SSTVSET.SetTxMode(SSTVSET.m_TxMode);

	mp->OpenTXBuf(10);
	mp->InitTXBuf();

	// MMSSTV ヘッダー
	OutHEAD();
	// VIS
	if( SSTVSET.m_fTxNarrow ){			// 狭帯域モードの送信
		mp->Write(1900, 300);
		mp->Write(FSKSPACE, FSKGARD);
		mp->Write(1900, FSKINTVAL);
		mp->WriteFSK(0x2d);     // STX
		mp->WriteFSK(0x15);     // STX
		BYTE d = 0;
		switch(SSTVSET.m_TxMode){
			case smMN73:
				d = 0x02;
				break;
			case smMN110:
				d = 0x04;
				break;
			case smMN140:
				d = 0x05;
				break;
			case smMC110:
				d = 0x14;
				break;
			case smMC140:
				d = 0x15;
				break;
			case smMC180:
				d = 0x16;
				break;
		}
		mp->WriteFSK(d);
		mp->WriteFSK(BYTE(d^0x15));
//		mp->Write(FSKSPACE, FSKGARD);
	}
	else {
		int i;
		int n;
		int d;
		int e = (SSTVSET.m_TxMode == smAVT) ? 3 : 1;
		for( n = 0; n < e; n++ ){
			mp->Write(1900, 300);
			mp->Write(1200, 10);
			mp->Write(1900, 300);
			mp->Write(1200, 30);
			switch(SSTVSET.m_TxMode){
				case smR36:
					d = 0x88;
					break;
				case smR72:
					d = 0x0c;
					break;
				case smAVT:
					d = 0x44;
					break;
				case smSCT1:
					d = 0x3c;
					break;
				case smSCT2:
					d = 0xb8;
					break;
				case smSCTDX:
					d = 0xcc;
					break;
				case smMRT1:
					d = 0xac;
					break;
				case smMRT2:
					d = 0x28;
					break;
				case smSC2_180:
					d = 0xb7;
					break;
				case smSC2_120:
					d = 0x3f;
					break;
				case smSC2_60:
					d = 0xbb;
					break;
				case smPD50:
					d = 0xdd;
					break;
				case smPD90:
					d = 0x63;
					break;
				case smPD120:
					d = 0x5f;
					break;
				case smPD160:
					d = 0xe2;
					break;
				case smPD180:
					d = 0x60;
					break;
				case smPD240:
					d = 0xe1;
					break;
				case smPD290:
					d = 0xde;
					break;
				case smP3:
					d = 0x71;
					break;
				case smP5:
					d = 0x72;
					break;
				case smP7:
					d = 0xf3;
					break;
				case smMR73:
					d = 0x4523;
					break;
				case smMR90:
					d = 0x4623;
					break;
				case smMR115:
					d = 0x4923;
					break;
				case smMR140:
					d = 0x4a23;
					break;
				case smMR175:
					d = 0x4c23;
					break;
				case smMP73:
					d = 0x2523;
					break;
				case smMP115:
					d = 0x2923;
					break;
				case smMP140:
					d = 0x2a23;
					break;
				case smMP175:
					d = 0x2c23;
					break;
				case smML180:
					d = 0x8523;
					break;
				case smML240:
					d = 0x8623;
					break;
				case smML280:
					d = 0x8923;
					break;
				case smML320:
					d = 0x8a23;
					break;
				case smR24:
					d = 0x84;
					break;
				case smRM8:
					d = 0x82;
					break;
				case smRM12:
					d = 0x86;
					break;
			}
			if( d >= 0x100 ){
				for( i = 0; i < 16; i++ ){
					mp->Write(short(d & 0x0001 ? 1100 : 1300), 30);
					d = d >> 1;
				}
			}
			else {
				for( i = 0; i < 8; i++ ){
					mp->Write(short(d & 0x0001 ? 1100 : 1300), 30);
					d = d >> 1;
				}
			}
			mp->Write(1200, 30);
		}
		if( SSTVSET.m_TxMode == smAVT ){       // 同期信号
			int sd = 0x5fa0;
			for( i = 0; i < 32; i++ ){
				mp->Write(1900, 9.7646);
				d = sd;
				for( n = 0; n < 16; n++ ){
					mp->Write(short(d & 0x8000 ? 1600 : 2200), 9.7646);
					d = d << 1;
				}
				sd = ((sd & 0xff00) - 0x0100) | ((sd & 0x00ff) + 0x0001);
			}
			mp->Write(0, 0.30514375);
		}
		else if( (SSTVSET.m_TxMode == smSCT1) || (SSTVSET.m_TxMode == smSCT2) || (SSTVSET.m_TxMode == smSCTDX) ){
			mp->Write(1200, 9.0);
		}
	}
	mp->m_sCnt = mp->m_Cnt;
	SendSSTV();
	if( pRadio != NULL ) pRadio->SetPTT(1);
	if( pComm != NULL ){
		pComm->SetPTT(1);
		pComm->SetScan(pDem->m_Sync);
	}
	LogLink.SetPTT(1);
	::Sleep(10);    // 10msのウエイト
	pSound->m_ReqTx = 1;
	pMod->m_tune = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::ToRX(void)
{
	CWaitCursor w;

	m_TuneTimer = 0;
	CSSTVDEM *dp = pDem;
	if( sys.m_Repeater ){
		if( dp->m_repmode >= 6 ){
			dp->m_repmode = 0;
		}
		SetBeaconTick();
	}
	if( (dp->m_LoopBack == 1 ) && dp->m_Sync &&
//        (pDem->m_rBase < (SSTVSET.m_LM * 90/100)) ){
		(m_AY < (SSTVSET.m_L * 90/100)) ){
		dp->Stop();
		dp->m_SyncMode = 0;
	}
	pSound->m_ReqTx = 0;
	int tout = (pSound->m_BuffSize * pSound->m_FifoSizeTX * 10) / SSTVSET.m_TxSampFreq;
	tout += 10;
	int lost = pDem->m_Lost;
	for( int i = 0; i < tout; i++ ){
		if( !pSound->m_Tx ) break;
		DrawSSTV();
		::Sleep(100);
	}
	if( !lost ) pDem->m_Lost = 0;
	pMod->SetRow(NULL, 0);
	if( m_pRowBuf != NULL ){
		delete m_pRowBuf;
		m_pRowBuf = NULL;
	}
	if( !sys.m_echo ){
		pDem->OpenCloseRxBuff();
	}
	pMod->m_tune = 0;
	if( pRadio != NULL ) pRadio->SetPTT(0);
	if( pComm != NULL ){
		pComm->SetScan(dp->m_Sync);
		pComm->SetPTT(0);
	}
	LogLink.SetPTT(0);
	SBTX->Down = 0;
	SBTune->Down = 0;
	UpdateTXUI();
	DeleteTXCursor();
	pMod->CloseTXBuf();
	UpdateSBTO();
	UpdateModeBtn();
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBTXClick(TObject *Sender)
{
	if( SBTX->Down ){
		ToTX();
	}
	else {
		ToRX();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBTuneClick(TObject *Sender)
{
	if( SBTune->Down ){
		SBCW->Enabled = FALSE;
		if( pRadio != NULL ) pRadio->SetPTT(1);
		if( pComm != NULL ) pComm->SetPTT(1);
		LogLink.SetPTT(1);
		pMod->m_tune = 1;
		pSound->m_ReqTx = 1;
		for( int i = 0; i < 20; i++ ){
			if( pSound->m_Tx ) break;
			::Sleep(100);
		}
		m_TuneTimer = ::GetTickCount();
		if( sys.m_TuneTXTime >= 0 ){
			m_TuneTimer += sys.m_TuneTXTime * 1000;
		}
		else {
			m_TuneTimer += 30 * 1000;
		}
	}
	else {
		ToRX();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBCWClick(TObject *Sender)
{
	if( SBCW->Down ){
		SendCWID(sys.m_CWText.c_str());
        Application->ProcessMessages();
		SBCW->Down = FALSE;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DrawTXCursor_(int y)
{
	if( m_MainPage != pgTX ) return;

	PBoxTX->Canvas->MoveTo(0, y);
	int Sop = ::SetROP2(PBoxTX->Canvas->Handle, R2_NOT);
	PBoxTX->Canvas->LineTo(320, y);
	::SetROP2(PBoxTX->Canvas->Handle, Sop);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DrawTXCursor(int y)
{
	if( m_SendY == -1 ){
		PBoxTXPaint(NULL);
	}
	if( y != m_SendY ){
		DeleteTXCursor();
		m_SendY = y;
		DrawTXCursor_(m_SendY);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DeleteTXCursor(void)
{
	if( m_SendY >= 0 ){
		DrawTXCursor_(m_SendY);
		m_SendY = -1;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PageChange(TObject *Sender)
{
	m_MainPage = GetActiveIndex(Page);
	MultProc();
	m_TempHold = 0;
	UpdateModeBtn();
	if( (m_MainPage == pgTX) && (!SBTX->Down) ){
		MultProc();
		UpdateTemp();
		MakeTxBitmap(0);
	}
	if( m_MainPage == pgTemp ) UpdateUITemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateTxClip(void)
{
	switch(m_TxClipType){
		case 0:
			SBDS240->Down = FALSE;
			SBUS240->Down = FALSE;
			SBDA240->Down = FALSE;
			SBUA240->Down = FALSE;
			break;
		case 1: // DS
			SBDS240->Down = TRUE;
			break;
		case 2: // US
			SBUS240->Down = TRUE;
			break;
		case 3: // DA
			SBDA240->Down = TRUE;
			break;
		case 4: // UA
			SBUA240->Down = TRUE;
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDS240Click(TObject *Sender)
{
	m_TxClipType = SBDS240->Down ? 1 : 0;
	UpdatePic();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBUS240Click(TObject *Sender)
{
	m_TxClipType = SBUS240->Down ? 2 : 0;
	UpdatePic();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDA240Click(TObject *Sender)
{
	m_TxClipType = SBDA240->Down ? 3 : 0;
	UpdatePic();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBUA240Click(TObject *Sender)
{
	m_TxClipType = SBUA240->Down ? 4 : 0;
	UpdatePic();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::MakeTxBitmap(int sw)
{
	if( !sw && pSound->m_Tx && !SBTune->Down ) return;
	CWaitCursor wait;

	int sf = pBitmapTX->Width != pBitmapTemp->Width ? 1 : 0;
	int Sop;
	FillBitmap(pBitmapTX, sys.m_PicClipColor);
	TRect rc;
	int rep = 0;
	if( sys.m_Repeater && pDem->m_Repeater ){
		rep = 1;
	}
	switch(m_TxClipType){
		case 0:
_n1:;
			pBitmapTX->Canvas->Draw(0, 0, pBitmapTXM);
			break;
		case 1: // DS
			if( rep ) goto _n1;
			pBitmapTX->Canvas->Draw(0, 16, pBitmapTXM);
			break;
		case 2: // US
			if( sf || rep ) goto _n1;
			pBitmapTX->Canvas->Draw(0, -16, pBitmapTXM);
			break;
		case 3: // DA
			if( rep ) goto _n1;
			rc.Left = 0; rc.Right = pBitmapTX->Width;
			rc.Top = 16; rc.Bottom = pBitmapTX->Height;
			Sop = ::SetStretchBltMode(pBitmapTX->Canvas->Handle, HALFTONE);
			pBitmapTX->Canvas->StretchDraw(rc, pBitmapTXM);
			::SetStretchBltMode(pBitmapTX->Canvas->Handle, Sop);
			break;
		case 4: // UA
			if( sf || rep ) goto _n1;
			rc.Left = 0; rc.Right = pBitmapTX->Width;
			rc.Top = 0; rc.Bottom = pBitmapTX->Height - 16;
			Sop = ::SetStretchBltMode(pBitmapTX->Canvas->Handle, HALFTONE);
			pBitmapTX->Canvas->StretchDraw(rc, pBitmapTXM);
			::SetStretchBltMode(pBitmapTX->Canvas->Handle, Sop);
			break;
	}
	if( SBUseTemp->Down || (m_MainPage == pgTemp) ){
		pBitmapTemp->Transparent = TRUE;
		pBitmapTemp->TransparentMode = tmFixed;
		pBitmapTemp->TransparentColor = pBitmapTemp->Canvas->Pixels[DrawMain.m_TransX][DrawMain.m_TransY];
		if( rep && sys.m_RepBottomAdj ){    // 伸縮なし
			DrawMain.FillPic(pBitmapTX->Canvas, pBitmapTemp->TransparentColor, -1, 1.0, 1.0, m_TXPH - pBitmapTemp->Height);
			pBitmapTX->Canvas->Draw(0, m_TXPH - pBitmapTemp->Height, pBitmapTemp);
		}
		else {
			DrawMain.FillPic(pBitmapTX->Canvas, pBitmapTemp->TransparentColor, m_TxClipType, double(m_TXW)/double(DrawMain.m_SX), double(m_TXH)/double(DrawMain.m_SY), 0);
			switch(m_TxClipType){
				case 2:
					if( sf ) goto _n2;
					pBitmapTX->Canvas->Draw(0, -16, pBitmapTemp);
					break;
				case 4:
					if( sf ) goto _n2;
					rc.Left = 0; rc.Right = pBitmapTX->Width;
					rc.Top = 0; rc.Bottom = pBitmapTX->Height - 16;
					Sop = ::SetStretchBltMode(pBitmapTX->Canvas->Handle, HALFTONE);
					pBitmapTX->Canvas->StretchDraw(rc, pBitmapTemp);
					::SetStretchBltMode(pBitmapTX->Canvas->Handle, Sop);
					break;
				default:
_n2:;
					if( sf ){
						StretchCopy(pBitmapTX, pBitmapTemp, HALFTONE);
					}
					else {
						pBitmapTX->Canvas->Draw(0, 0, pBitmapTemp);
					}
					break;
			}
		}
		pBitmapTemp->Transparent = FALSE;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBUseTempClick(TObject *Sender)
{
	if( pSound->m_Tx && !SBTune->Down && !SBUseTemp->Down ){
		SBUseTemp->Down = TRUE;
	}
	if( m_ReqHistF ) AdjustHistF();
	UpdateTemp();
	MakeTxBitmap(1);
	PBoxTXPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KOSCClick(TObject *Sender)
{
	if( pSound == NULL ) return;
	TTScope *pBox = new TTScope(this);
	pBox->Execute(pDem);
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KOLClick(TObject *Sender)
{
	TLogSetDlg *pBox = new TLogSetDlg(this);
	pBox->Execute();
	delete pBox;
	UpdateLogLink();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::InfoSampFreq(void)
{
	if( fabs(sys.m_SampFreq - SampFreq) >= 50.0 ){
		if( (pSound != NULL) && (sys.m_SampFreq > (SampFreq * 1100/1060)) ){
			pDem->Start(smSCT1, 0);
			UpdateModeBtn();
			GBMode->Enabled = FALSE;
		}
		InfoMB(
			MsgEng ?
			"Restart MMSSTV for the new sampling rate.\r\n(Does not have to restart your PC)\r\n\r\n"
			"Now MMSSTV is running on %.2lfHz, and was requested to change to %.2lfHz."
			"  This difference are too large for the DSP of MMSSTV." :
			"新しいサンプリングレートを適用します。MMSSTVを再起動して下さい\r\n"
			"(パソコンを再起動する必要はありません)\r\n\r\n"
			"現在MMSSTVは%.2lfHzで動作しており、%.2lfHzへの変更を要請されています。"
			"  再起動しないと正常な動作を維持することができません。"
			, SampFreq, sys.m_SampFreq
		);
	}
	else {
		GBMode->Enabled = TRUE;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KOMClick(TObject *Sender)
{
	TOptionDlg *pBox = new TOptionDlg(this);
	int stereo = sys.m_SoundStereo;
    int stereoTX = sys.m_StereoTX;
	int fontadj = sys.m_FontAdjSize;
	int cal = sys.m_DemCalibration;
	int AppPriority = sys.m_Priority;
	AnsiString DevID = sys.m_SoundDevice;
	AnsiString MMW = sys.m_SoundMMW;
	if( Sender == NULL ) pBox->SetPageIndex(1);
	if( pBox->Execute(pDem, pMod) == TRUE ){
		if( cal != sys.m_DemCalibration ) MakeCalibrationTable();
		if( (sys.m_UseRxBuff == 2) && !WaveStg.IsOpen() ) WaveStg.Open();
		if( (sys.m_UseRxBuff != 2) && WaveStg.IsOpen() ) WaveStg.Close();

		if( !SBTX->Down ){
			pMod->m_vco.SetSampleFreq(sys.m_SampFreq + sys.m_TxSampOff);
		}
		UpdateSystemFont();
		InitColorTable(sys.m_ColorLow, sys.m_ColorHigh);
		if( (pSound->m_FifoSizeTX != sys.m_SoundFifoTX) ||
			(pSound->m_FifoSizeRX != sys.m_SoundFifoRX) ||
			(DevID != sys.m_SoundDevice) ||
            (MMW != sys.m_SoundMMW) ||
			(stereo != sys.m_SoundStereo) ||
            (stereoTX != sys.m_StereoTX) ){
//			pSound->m_IDDevice = sys.m_SoundDevice;
			pSound->InitSound();
		}
		DispSyncStat();
		InfoSampFreq();
		UpdateTitle();
		if( sys.m_HistMax != RxHist.m_Head.m_Max ){
			RxHist.ChangeMax(sys.m_HistMax);
			m_DisEvent++;
			UDHist->Position = 0;
			UpdateHist();
			m_DisEvent--;
		}
		if( fontadj != sys.m_FontAdjSize ){
			FormResize(NULL);
		}
		SBRXID->Down = pDem->m_fskdecode;
		SBTXID->Down = sys.m_TXFSKID;
		if( AppPriority != sys.m_Priority ) UpdatePriority();
	}
	if( COMM.change || RADIO.change ){
		OpenCloseCom();
	}
	delete pBox;
	UpdateModeBtn();
	UpdateUI();
	if( pCtrBtn != NULL ) pCtrBtn->UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OpenCloseCom(void)
{
	if( pComm != NULL ){
		delete pComm;
		pComm = NULL;
	}
	if( pRadio != NULL ){
		delete pRadio;
		pRadio = NULL;
	}
	if( strcmpi(sys.m_TxRxName.c_str(), "NONE") ){
		pComm = new CComm();
		pComm->Open(sys.m_TxRxName.c_str());
		if( !pComm->IsOpen() ){
			if( sys.m_TxRxLock ){
				delete pComm;
				pComm = NULL;
				ErrorMB( "Could not open '%s' for the PTT controls", sys.m_TxRxName.c_str());
			}
		}
		if( pComm != NULL ){
			pComm->SetPTT(SBTX->Down);
			pComm->SetScan(pDem->m_Sync);
		}
//        if( !sys.m_TxRxLock ) pComm->Close();
	}
	RADIO.change = 0;
	if( strcmp(RADIO.StrPort, "NONE" ) ){
		pRadio = new CCradio(TRUE);
		pRadio->Open(&RADIO, Handle, CM_CMMR, CM_CRADIO);
		if( pRadio->m_CreateON != TRUE ){
			delete pRadio;
			pRadio = NULL;
			ErrorMB( MsgEng ? "Could not open '%s' for Radio commands":"Radioコマンドポート'%s'がオープンできません.", RADIO.StrPort);
		}
		else {
			pRadio->SendCommand(RADIO.CmdInit.c_str());
			if( RADIO.PollScan ){
				m_DisEvent++;
				LogFreq->Text = "???";
				m_DisEvent--;
				RADIO.Cmdxx = 0x00;
				pRadio->m_ScanAddr = 1;
			}
		}
	}
	RADIO.change = COMM.change = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KExitClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBListClick(TObject *Sender)
{
	TLogListDlg *pBox = new TLogListDlg(this);
	MultProc();
	pBox->Execute();
//    Application->OnIdle = NULL;
	delete pBox;
	AutoLogSave();
	UpdateLogLink();
	UpdateTitle();
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateCallsign(void)
{
	if( HisCall->Text.IsEmpty() ) m_Dupe = 0;
	HisCall->Font->Color = m_Dupe ? clRed : clBlack;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateLogData(void)
{
	StrCopy(Log.m_sd.call, AnsiString(HisCall->Text).c_str(), MLCALL);	//ja7ude 0428
	clipsp(Log.m_sd.call);
	jstrupr(Log.m_sd.call);
	StrCopy(Log.m_sd.name, AnsiString(HisName->Text).c_str(), MLNAME);	//ja7ude 0428
	StrCopy(Log.m_sd.qth, AnsiString(HisQTH->Text).c_str(), MLQTH);	//ja7ude 0428
	StrCopy(Log.m_sd.my, AnsiString(MyRST->Text).c_str(), MLRST);	//ja7ude 0428
	jstrupr(Log.m_sd.my);
	StrCopy(Log.m_sd.ur, AnsiString(HisRST->Text).c_str(), MLRST);	//ja7ude 0428
	jstrupr(Log.m_sd.ur);
	StrCopy(Log.m_sd.rem, AnsiString(EditNote->Text).c_str(), MLREM);	//ja7ude 0428
	StrCopy(Log.m_sd.qsl, AnsiString(EditQSL->Text).c_str(), MLQSL);	//ja7ude 0428
	Log.SetFreq(&Log.m_sd, AnsiString(LogFreq->Text).c_str());	//ja7ude 0428
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateTextData(void)
{
	HisCall->Text = Log.m_sd.call;
	HisName->Text = Log.m_sd.name;
	HisQTH->Text = Log.m_sd.qth;
	HisRST->Text = Log.m_sd.ur;
	MyRST->Text = Log.m_sd.my;
	EditNote->Text = Log.m_sd.rem;
	EditQSL->Text = Log.m_sd.qsl;
	LogFreq->Text = Log.GetFreqString(Log.m_sd.band, Log.m_sd.fq);
	Log.m_CurChg = 0;
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::FindCall(void)
{
	CWaitCursor w;
	m_Dupe = 0;
	StrCopy(Log.m_sd.call, AnsiString(HisCall->Text).c_str(), MLCALL);	//ja7ude 0428
    clipsp(Log.m_sd.call);

	if( IsCall(Log.m_sd.call) ) AddCall(Log.m_sd.call);

	AnsiString  His = HisRST->Text;
	if( Log.FindSet(&Log.m_Find, Log.m_sd.call) ){  // 見つかった時
		MultProc();
		Log.SetFreq(&Log.m_sd, AnsiString(LogFreq->Text).c_str());	//ja7ude 0428
		SDMMLOG	sd;
		Log.GetData(&sd, Log.m_Find.pFindTbl[0]);	// 一番最新のデータ
		if( Log.m_Find.m_FindCmp1Max ){
			switch(Log.m_LogSet.m_CheckBand){
				case 1:
					if( Log.FindSameBand() ) m_Dupe = 1;
					break;
				case 2:
					if( Log.FindSameDate() ) m_Dupe = 1;
					break;
				default:
					m_Dupe = 1;
					break;
			}
		}
#if 0
		if( Log.m_Find.m_FindCmp1Max && ((!Log.m_LogSet.m_CheckBand) || Log.FindSameBand()) ){
			m_Dupe = 1;
		}
		else {
			m_Dupe = 0;
		}
#endif
		if( (!SBQSO->Down || !Log.m_sd.name[0] ) && Log.m_LogSet.m_CopyName ) strcpy(Log.m_sd.name, sd.name);
		if( (!SBQSO->Down || !Log.m_sd.qth[0] ) && Log.m_LogSet.m_CopyQTH ) strcpy(Log.m_sd.qth, sd.qth);
		if( (!SBQSO->Down || !Log.m_sd.rem[0] ) && Log.m_LogSet.m_CopyREM ) strcpy(Log.m_sd.rem, sd.rem);
		if( (!SBQSO->Down || !Log.m_sd.qsl[0] ) && Log.m_LogSet.m_CopyQSL ) strcpy(Log.m_sd.qsl, sd.qsl);
		UpdateTextData();
		if( SBQSO->Down ){
			Log.m_Find.Ins(Log.m_CurNo);
			UpdateTextData();
			Log.PutData(&Log.m_sd, Log.m_CurNo);
		}
	}
	else {                                          // 見つからなかった時
		if( !SBQSO->Down && Log.m_LogSet.m_CopyName ) Log.m_sd.name[0] = 0;
		if( !SBQSO->Down && Log.m_LogSet.m_CopyQTH ) Log.m_sd.qth[0] = 0;
		if( !SBQSO->Down && Log.m_LogSet.m_CopyREM ) Log.m_sd.rem[0] = 0;
		if( !SBQSO->Down && Log.m_LogSet.m_CopyQSL ) Log.m_sd.qsl[0] = 0;
		UpdateTextData();
	}
	LPCSTR pCC = ClipCC(Log.m_sd.call);
	Log.SetOptStr(0, &Log.m_sd, Cty.GetCountry(pCC));
	Log.SetOptStr(1, &Log.m_sd, Cty.GetCont(pCC));
	if( !His.IsEmpty() ) HisRST->Text = His;
	UpdateCallsign();
	UpdateUI();
	if( !SBQSO->Down ){
		LogLink.SetMode("SSTV");
		LogLink.SetFreq(AnsiString(LogFreq->Text).c_str());	//ja7ude 0428
		LogLink.FindCall(Log.m_sd.call);
	}
	else {
		LogLink.Write(&Log.m_sd, 1);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBFindClick(TObject *Sender)
{
	if( !Log.IsOpen() ){
		if( Log.Open(NULL, TRUE) == FALSE ){
			UpdateUI();
			return;
		}
	}
	FindCall();
	TQSODlgBox *pBox = new TQSODlgBox(this);
	pBox->ShowFind(&Log.m_Find);
	delete pBox;
	AutoLogSave();
	TopWindow(this);
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBFindMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		AnsiString as;
		if( InputMB("MMSSTV", "Callsign", as) == TRUE ){
			jstrupr(as.c_str());
			CLogFind find;
			Log.FindSet(&find, as.c_str() );
			TQSODlgBox *pBox = new TQSODlgBox(this);
			pBox->ShowFind(&find);
			delete pBox;
			TopWindow(this);
        }
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDataClick(TObject *Sender)
{
	SDMMLOG sd;

	UpdateLogData();
	if( strcmp(Log.m_Find.GetText(), Log.m_sd.call) ){
		CWaitCursor w;
		Log.FindSet(&Log.m_Find, Log.m_sd.call);
	}
	TQSODlgBox *pBox = new TQSODlgBox(this);
	MultProc();
	memcpy(&sd, &Log.m_sd, sizeof(sd));
	if( pBox->Execute(&Log.m_Find, &Log.m_sd, Log.m_CurNo) == TRUE ){
		if( memcmp(&sd, &Log.m_sd, sizeof(sd)) ){
			AdjustFocus();
			UpdateTextData();
			HisCallChange(NULL);
			LogLink.Write(&Log.m_sd, 0);
		}
	}
	delete pBox;
	TopWindow(this);
	AdjustFocus();
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDataMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		int n = Log.m_CurNo - 1;
		if( n < 0 ){
			SBDataClick(NULL);
			return;
		}

		TQSODlgBox *pBox = new TQSODlgBox(this);
		SDMMLOG	sd;
		Log.GetData(&sd, n);
		CLogFind	Find;

		Log.FindSet(&Find, sd.call);

		pBox->Execute(&Find, &sd, n);
		delete pBox;
		TopWindow(this);
		if( !SBQSO->Down && Log.m_CurNo && Log.IsEdit() ){
			Log.SetLastPos();
			UpdateTextData();
		}
		AutoLogSave();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBInitClick(TObject *Sender)
{
	AdjustFocus();
	AnsiString	as = LogFreq->Text;
	if( SBQSO->Down ){
		Log.DeleteLast();
		Log.m_Find.Clear();
		Log.InitCur();
		SBQSO->Down = FALSE;
		UpdateTextData();
	}
	else {
		Log.InitCur();
		Log.m_sd.call[0] = 0;
		Log.m_sd.name[0] = 0;
		Log.m_sd.qth[0] = 0;
		UpdateTextData();
		UpdateCallsign();
	}
	if( !as.IsEmpty() ){
		LogFreq->Text = as;
		Log.SetFreq(&Log.m_sd, as.c_str());
	}
	HisCallChange(NULL);
	LogLink.Clear();
	if( HisCall->CanFocus() ) HisCall->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::TempDelay(void)
{
	int delay;
    switch(sys.m_TempDelay){
		case 0:
        	delay = 500;
        	break;
        case 1:
        	delay = 1000;
            break;
        case 2:
        	delay = 2000;
            break;
        default:
        	delay = 3000;
            break;
    }
	m_ChangeTemp = delay / Timer->Interval;
}
//---------------------------------------------------------------------------
// バンドの変更
void __fastcall TMmsstv::LogFreqChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	Log.SetFreq(&Log.m_sd, AnsiString(LogFreq->Text).c_str());	//ja7ude 0428
	LogLink.SetFreq(AnsiString(LogFreq->Text).c_str());	//ja7ude 0428
#if 0
	if( !HisCall->Text.IsEmpty() ){
		FindCall();
	}
#endif
	if( DrawMain.IsMacro("%b") || DrawMain.IsMacro("%B") ){
		TempDelay();
		SBULog->Enabled = TRUE;
		UpdateUI();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::HisCallKeyPress(TObject *Sender, char &Key)
{
	if( Key == VK_RETURN ){
		Key = 0;
	}
	else {
		Key = char(toupper(Key));
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::HisNameKeyPress(TObject *Sender, char &Key)
{
	if( Key == VK_RETURN ){
		Key = 0;
		if( (Sender == HisQTH) && IsJA(AnsiString(HisCall->Text).c_str()) ){
			TMmcgDlgBox *pBox = new TMmcgDlgBox(this);

			AnsiString	qth = HisQTH->Text;
			AnsiString	opt = Log.m_sd.opt1;
			AnsiString  hc = HisCall->Text;
			if( pBox->Execute(hc, qth, opt) == TRUE ){	//ja7ude 0521
				HisQTH->Text = qth;
				StrCopy(Log.m_sd.opt1, opt.c_str(), MLOPT);
			}
			delete pBox;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBQSOClick(TObject *Sender)
{
	UpdateLogData();
	SYSTEMTIME	now;
	GetUTC(&now);
	int Year = now.wYear % 100;
	int Month = now.wMonth;
	int Day = now.wDay;
	int Hour = now.wHour;
	UTCtoJST(Year, Month, Day, Hour);

//	Log.m_sd.cq = m_Running ? 'A' : 'C';
	if( SBQSO->Down ){		// Start QSO
		LogLink.SetTime(&now, 0);
		Log.m_sd.year = char(Year % 100);
		Log.m_sd.date = WORD(Month * 100 + Day);
		Log.m_sd.btime = WORD((Hour * 60 + now.wMinute) * 30 + now.wSecond/2);
		if( !Log.m_sd.btime ) Log.m_sd.btime++;
		CWaitCursor w;
		if( Log.FindSet(&Log.m_Find, Log.m_sd.call) ){
			MultProc();
			SDMMLOG	sd;
			Log.GetData(&sd, Log.m_Find.pFindTbl[0]);	// 一番最新のデータ
			if( !Log.m_sd.name[0] && Log.m_LogSet.m_CopyName ) strcpy(Log.m_sd.name, sd.name);
			if( !Log.m_sd.qth[0] && Log.m_LogSet.m_CopyQTH ) strcpy(Log.m_sd.qth, sd.qth);
			if( !Log.m_sd.rem[0] && Log.m_LogSet.m_CopyREM ) strcpy(Log.m_sd.rem, sd.rem);
			if( !Log.m_sd.qsl[0] && Log.m_LogSet.m_CopyQSL ) strcpy(Log.m_sd.qsl, sd.qsl);
		}
		Log.CopyAF();
		if( Log.m_sd.call[0] ){
			LPCSTR pCC = ClipCC(Log.m_sd.call);
			Log.SetOptStr(0, &Log.m_sd, Cty.GetCountry(pCC));
			Log.SetOptStr(1, &Log.m_sd, Cty.GetCont(pCC));
		}
		UpdateTextData();
		if( Log.PutData(&Log.m_sd, Log.m_CurNo) == FALSE ){
			SBQSO->Down = FALSE;
		}

		if( Log.m_Find.m_FindCmp1Max && ((!Log.m_LogSet.m_CheckBand) || Log.FindSameBand()) ){
			m_Dupe = 1;
		}
		else {
			m_Dupe = 0;
		}
		Log.m_Find.Ins(Log.m_CurNo);
		Log.m_CurChg = 0;
		LogLink.Write(&Log.m_sd, 1);
		if( HisName->CanFocus() ) HisName->SetFocus();
	}
	else {					// Finish QSO
		LogLink.SetTime(&now, 1);
		Log.m_sd.etime = WORD((Hour * 60 + now.wMinute) * 30 + now.wSecond/2);
		if( !Log.m_sd.etime ) Log.m_sd.etime++;
		if( !Log.m_sd.ur[0] ){
			strcpy(Log.m_sd.ur, "595");
		}
		if( !Log.m_sd.my[0] ){
			strcpy(Log.m_sd.my, "595");
		}
		Log.PutData(&Log.m_sd, Log.m_CurNo);
		LogLink.Write(&Log.m_sd, 2);

		memcpy(&Log.m_asd, &Log.m_sd, sizeof(Log.m_asd));
		Log.m_CurNo++;
		Log.m_CurChg = 0;
		Log.m_Find.Clear();
		Log.InitCur();
		UpdateTextData();
		AutoLogSave();
		if( HisCall->CanFocus() ) HisCall->SetFocus();
	}
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AutoLogSave(void)
{
	if( Log.IsOpen() && Log.m_LogSet.m_AutoSave && Log.IsEdit() ) KLogFlushClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KLogFlushClick(TObject *Sender)
{
	Log.Close();
	Log.Open(NULL, TRUE);
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KLogOpenClick(TObject *Sender)
{
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options >> ofFileMustExist;
	if( MsgEng ){
		OpenDialog->Title = "Open LogData File";
		OpenDialog->Filter = "MMLOG Data Files(*.mdt)|*.mdt|";
	}
	else {
		OpenDialog->Title = "ログファイルのオープン";
		OpenDialog->Filter = "MMLOGデータファイル(*.mdt)|*.mdt|";
	}
	OpenDialog->FileName = "";
	OpenDialog->DefaultExt = "mdt";
	OpenDialog->InitialDir = MMLogDir;
	NormalWindow(this);
	if( OpenDialogExecute(FALSE) == TRUE ){
		Log.Close();
        Log.DoBackup();
		Log.MakeName(AnsiString(OpenDialog->FileName).c_str());
		Log.Open(NULL, TRUE);
		UpdateTextData();
		UpdateTitle();
	}
	TopWindow(this);
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KVerClick(TObject *Sender)
{
	TVerDspDlg *pBox = new TVerDspDlg(this);
	pBox->ShowModal();
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxTempCursor(int X, int Y)
{
	int cr = crDefault;
	if( m_DrawTrans ){
		cr = crCross;
	}
	else if( m_DrawCmd == CM_SELECT ){
		if( DrawMain.pSel != NULL ){
			switch(DrawMain.pSel->HitTest(X, Y)){
				case HT_LT:
				case HT_RB:
					cr = crSizeNWSE;
					break;
				case HT_T:
				case HT_B:
					cr = crSizeNS;
					break;
				case HT_RT:
				case HT_LB:
					cr = crSizeNESW;
					break;
				case HT_R:
				case HT_L:
					cr = crSizeWE;
					break;
				case HT_NONE:
					break;
				default:
					cr = crHandPoint;
					break;
			}
		}
	}
	else if( m_DrawCmd != -1 ) {
		cr = crCross;
	}
	if( PBoxTemp->Cursor != cr ) PBoxTemp->Cursor = (TCursor)cr;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AdjustTempSC(int &X, int &Y)
{
	if( ((DrawMain.m_SX != 320) || (DrawMain.m_SY != 320)) ){
		X = X * DrawMain.m_SX / 320;
		Y = Y * DrawMain.m_SY / 256;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxTempMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
//    AdjustFocus();
	if( Button == mbRight ){
		m_TempX = X;
		m_TempY = Y;
		return;
	}

	m_TempHold = 1;

	SetDrawSize(DrawMain.m_SX, DrawMain.m_SY);
	AdjustTempSC(X, Y);
	if( m_DrawTrans ){
		SaveUndo();
		KTAP->Checked = FALSE;
		m_DrawTrans = 0;
		DrawMain.m_TransX = X;
		DrawMain.m_TransY = Y;
		UpdateTemp();
	}
	else if( m_DrawCmd == CM_SELECT ){
		m_TempTimer = 2;        // 0.4[s]
		m_TempX = X;
		m_TempY = Y;
		CDraw *pSel = DrawMain.pSel;
		if( (DrawMain.pSel == NULL) || ((DrawMain.m_Hit = DrawMain.pSel->HitTest(X, Y)) == HT_NONE) ){
			DrawMain.pSel = DrawMain.SelectItem(X, Y);
		}
		if( pSel != DrawMain.pSel ) UpdateTemp();
		if( DrawMain.pSel != NULL ){
			AdjustTempView(1);
			DrawMain.pSel->StartMove(PBoxTemp->Canvas, X, Y, DrawMain.m_Hit);
			AdjustTempView(0);
			PBoxTempCursor(X, Y);
		}
#if 0
		else {
			PBoxTemp->BeginDrag(TRUE);
		}
#endif
	}
	else if( m_DrawCmd != CM_TLIST ){
		pDraw = DrawMain.MakeItem(m_DrawCmd);
		if( pDraw != NULL ){
			AdjustTempView(1);
			pDraw->Start(PBoxTemp->Canvas, X, Y);
			AdjustTempView(0);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxTempMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	AdjustTempSC(X, Y);

	TShiftState sc1, sc2;
	sc1 << ssLeft;
	sc2 << ssLeft;
	sc1 *= Shift;
	if( sc1 == sc2 ){			// Left button
		AdjustTempView(1);
		if( pDraw != NULL ){                // 新規作成中
			pDraw->Making(X, Y);
		}
		else if( DrawMain.pSel != NULL ){   // 移動/サイズ変更中
			if( !m_TempTimer || (ABS(X-m_TempX)+ABS(Y-m_TempY)) > 5 ){
				m_TempTimer =0;
				DrawMain.pSel->Moving(X, Y);
			}
		}
		else if( !PBoxTemp->Dragging() ){   // ドラッグ
			//PBoxTemp->BeginDrag(TRUE);
      PBoxTemp->BeginDrag(true,0);
		}
		AdjustTempView(0);
	}
	else {
		PBoxTempCursor(X, Y);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxTempMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ) return;

	m_TempHold = 0;
	AdjustTempSC(X, Y);
	if( pDraw != NULL ){        // アイテム作成中
		if( pDraw->Finish(X, Y) ){
			TMemoryStream *ps = SaveUndo(NULL);
			DrawMain.AddItem(pDraw);
			DrawMain.pSel = pDraw;
			m_DrawCmd = CM_SELECT;
			SBDSel->Down = TRUE;
			if( (DrawMain.pSel->m_Command == CM_PIC) || (DrawMain.pSel->m_Command == CM_TITLE) ){
				KTBClick(NULL);
			}
			if( KTAP->Checked ) DrawMain.AdjustTransPoint();
			UpdateUndo(ps);
		}
		else {
			delete pDraw;
		}
		pDraw = NULL;
		UpdateTemp();
	}
	else if( m_DrawCmd == CM_TLIST ){
		TListTextDlg *pBox = new TListTextDlg(this);
		pDraw = pBox->Execute(&DrawText, 0);
		delete pBox;
		if( pDraw != NULL ){
			SaveUndo();
			DrawMain.pSel = DrawMain.AddItemCopy(pDraw);
			pDraw = DrawMain.pSel;
			int xw = pDraw->m_X2 - pDraw->m_X1;
			int yw = pDraw->m_Y2 - pDraw->m_Y1;
			pDraw->m_X1 = X;
			pDraw->m_Y1 = Y;
			pDraw->m_X2 = X + xw;
			pDraw->m_Y2 = Y + yw;
//			TempGrid.Align(pDraw);
			pDraw->UpdateText();
			pDraw = NULL;
			m_DrawCmd = CM_SELECT;
			SBDSel->Down = TRUE;
			if( KTAP->Checked ) DrawMain.AdjustTransPoint();
			UpdateTemp();
		}
	}
	else if( DrawMain.pSel != NULL ){   // 移動/サイズ変更中
		if( DrawMain.pSel->IsMove() && !m_TempTimer ){
			SaveUndo();
			DrawMain.pSel->Move(X, Y);
//			TempGrid.Align(DrawMain.pSel);
			if( (DrawMain.pSel->m_Command == CM_PIC) && (DrawMain.m_Hit != HT_I) ){
//                DrawMain.pSel->UpdatePic(pBitmapHistF);
				TShiftState sc1, sc2;
				sc1 << ssShift;
				sc2 << ssShift;
				sc1 *= Shift;
				if( sc1 == sc2 ){   // +Shift key
					((CDrawPic *)DrawMain.pSel)->KeepAspect();
				}
			}
			if( KTAP->Checked ) DrawMain.AdjustTransPoint();
			UpdateTemp();
		}
	}
	UpdateUITemp();
	PBoxTempCursor(X, Y);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DeleteUndo(void)
{
	if( pUndo != NULL ){
		delete pUndo;
		pUndo = NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateUndo(TMemoryStream *ps)
{
	if( pUndo != ps ){
		if( pUndo != NULL ) delete pUndo;
		pUndo = ps;
	}
}
//---------------------------------------------------------------------------
TMemoryStream *__fastcall TMmsstv::SaveUndo(TMemoryStream *ps)
{
	if( ps == NULL ){
		ps = new TMemoryStream;
	}
	ps->Seek(0, soFromBeginning);
	DrawMain.SaveToStream(ps);
	return ps;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SaveUndo(void)
{
	pUndo = SaveUndo(pUndo);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxTempPaint(TObject *Sender)
{
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDSelClick(TObject *Sender)
{
	m_DrawCmd = SBDSel->Down ? CM_SELECT : -1;
	DrawMain.pSel = NULL;
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDlineClick(TObject *Sender)
{
	m_DrawCmd = SBDline->Down ? CM_LINE : -1;
	DrawMain.pSel = NULL;
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDBoxClick(TObject *Sender)
{
	m_DrawCmd = SBDBox->Down ? CM_BOX : -1;
	DrawMain.pSel = NULL;
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDBoxSClick(TObject *Sender)
{
	m_DrawCmd = SBDBoxS->Down ? CM_BOXS : -1;
	DrawMain.pSel = NULL;
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDTextClick(TObject *Sender)
{
	m_DrawCmd = SBDText->Down ? CM_TEXT : -1;
	DrawMain.pSel = NULL;
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDPicClick(TObject *Sender)
{
	m_DrawCmd = SBDPic->Down ? CM_PIC : -1;
	DrawMain.pSel = NULL;
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDHeadClick(TObject *Sender)
{
	m_DrawCmd = SBDHead->Down ? CM_TITLE : -1;
	DrawMain.pSel = NULL;
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDListClick(TObject *Sender)
{
	m_DrawCmd = SBDList->Down ? CM_TLIST : -1;
	DrawMain.pSel = NULL;
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDDelClick(TObject *Sender)
{
	if( DrawMain.pSel != NULL ){
		SaveUndo();
		if( pPaste != NULL ){
			delete pPaste;
		}
		pPaste = DrawMain.pSel;
		DrawMain.ReleaseItem(DrawMain.pSel);
		DrawMain.pSel = DrawMain.GetLastItem();
		UpdateTemp();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDPasteClick(TObject *Sender)
{
	if( pPaste != NULL ){
		SaveUndo();
		DrawMain.pSel = DrawMain.AddItemCopy(pPaste);
		UpdateTemp();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTFXClick(TObject *Sender)
{
#if 1
	SaveUndo();
	DrawMain.ConvBitmap(DrawMain.pSel);
	UpdateTemp();
#else
	if( (DrawMain.pSel != NULL) && (DrawMain.pSel->m_Command == CM_PIC) ){
		SaveUndo();
		CDrawPic *pI = (CDrawPic *)DrawMain.pSel;
		pI->m_Type = 1;
		UpdateTemp();
	}
#endif
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTDClick(TObject *Sender)
{
	if( DrawMain.pSel != NULL ){
		SaveUndo();
		DrawMain.ReleaseItem(DrawMain.pSel);
		delete DrawMain.pSel;
		DrawMain.pSel = NULL;
		UpdateTemp();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTEClick(TObject *Sender)
{
	if( DrawMain.pSel == NULL ) return;

	DrawText.AddItemCopy(DrawMain.pSel);
	CDraw *pItem = DrawText.GetLastItem();
	TListTextDlg *pBox = new TListTextDlg(this);
	if( pBox->Execute(&DrawText, -1) == NULL ){
		DrawText.DeleteItem(pItem);
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTBClick(TObject *Sender)
{
	if( DrawMain.pSel != NULL ){
		SaveUndo();
		DrawMain.BackItem(DrawMain.pSel);
		UpdateTemp();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTFClick(TObject *Sender)
{
	if( DrawMain.pSel != NULL ){
		SaveUndo();
		DrawMain.FrontItem(DrawMain.pSel);
		UpdateTemp();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTTDClick(TObject *Sender)
{
	InvMenu(KTTD);
	UpdateTemp();
}
//---------------------------------------------------------------------------
// テンプレート 透過ポイントの指定
void __fastcall TMmsstv::KTTClick(TObject *Sender)
{
	m_DrawTrans = 1;
	UpdateUITemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTTCClick(TObject *Sender)
{
	InitCustomColor(ColorDialog);
	AddCustomColor(ColorDialog, DrawMain.m_TransCol);
	ColorDialog->Color = DrawMain.m_TransCol;
	SetDisPaint();
	if( ColorDialog->Execute() == TRUE ){
		SaveUndo();
		DrawMain.m_TransCol = ColorDialog->Color;
#if 0
		FillBitmap(pBitmapTemp, DrawMain.m_TransCol);
		DrawMain.m_TransCol = pBitmapTemp->Canvas->Pixels[0][0];
#else
		DrawMain.m_TransCol = GetNearestColor(DrawMain.m_TransCol);
#endif
		DrawMain.UpdateText();
		DrawMain.UpdatePic(pBitmapHistF);
		UpdateTemp();
	}
	ResDisPaint();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTMDClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	InvMenu(KTMD);
	DrawMain.UpdateText();
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTBTClick(TObject *Sender)
{
	InvMenu(KTBT);
	UpdateTemp();
}
//---------------------------------------------------------------------------
// テンプレート のロード
void __fastcall TMmsstv::KTLClick(TObject *Sender)
{
	if( LoadTemplateMenu(&DrawMain, 0) == TRUE ){
		DrawMain.pSel = NULL;
		if( !DrawMain.m_Cnt ){
			DrawMain.m_SX = m_TXW;
			DrawMain.m_SY = m_TXH;
		}
		UpdateTemp();
	}
}
//---------------------------------------------------------------------------
// テンプレート アイテムのロード
void __fastcall TMmsstv::AddTemplate(CDrawGroup *pGroup)
{
	SaveUndo();
	CDraw *pItem = NULL;
	while(pGroup->m_Cnt){
		pItem = pGroup->pBase[0];
		DrawMain.AddItem(pItem);
		pGroup->ReleaseItem(pItem);
		MultProc();
	}
	if( pItem != NULL ){
		DrawMain.pSel = pItem;
	}
	pGroup->FreeItem();
}
//---------------------------------------------------------------------------
// テンプレート アイテムのロード
void __fastcall TMmsstv::KTLIClick(TObject *Sender)
{
	if( LoadTemplateMenu(&DrawTemp, 1) == TRUE ){
		AddTemplate(&DrawTemp);
		UpdateTemp();
	}
	MultProc();
	DrawTemp.FreeItem();
}
//---------------------------------------------------------------------------
// テンプレート 全部のセーブ
void __fastcall TMmsstv::KTSAClick(TObject *Sender)
{
	SaveTemplateMenu(&DrawMain, 0);
}
//---------------------------------------------------------------------------
// テンプレート アイテムのセーブ
void __fastcall TMmsstv::KTSIClick(TObject *Sender)
{
	if( DrawMain.pSel != NULL ){
		DrawTemp.m_TransCol = DrawMain.m_TransCol;
		DrawTemp.AddItemCopy(DrawMain.pSel);
		SaveTemplateMenu(&DrawTemp, 1);
		DrawTemp.FreeItem();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTAClick(TObject *Sender)
{
	DrawMain.FreeItem();
	DrawMain.m_SX = m_TXW;
	DrawMain.m_SY = m_TXH;
	UpdateTemp();
}
//---------------------------------------------------------------------------
// テンプレート画面のUI更新処理
void __fastcall TMmsstv::UpdateUITemp(void)
{
	int cmd = DrawMain.pSel != NULL ? DrawMain.pSel->m_Command : 0;
	SBDDel->Enabled = cmd;
	SBDPaste->Enabled = pPaste != NULL;
	SBDFont->Enabled = (cmd == CM_TEXT) || ((cmd == CM_LIB)&&((((CDrawLib *)DrawMain.pSel)->m_Type & 0x0003)==2) );
	SBDCol->Enabled = cmd;
	if( cmd && (cmd != CM_TITLE) ){
		if( ( (cmd == CM_TEXT) && (((CDrawText *)DrawMain.pSel)->m_Shadow != 1)&&(((CDrawText *)DrawMain.pSel)->m_Shadow != 6) )||
			((cmd == CM_PIC) && (((CDrawPic *)DrawMain.pSel)->m_Type == 2))
		){
			SBDLineset->Enabled = FALSE;
		}
		else {
			SBDLineset->Enabled = TRUE;
		}
	}
	else {
		SBDLineset->Enabled = FALSE;
	}
	TDragMode dm;
	if( ((m_DrawCmd != -1) || m_DrawTrans) ){
		dm = dmManual;
	}
	else {
		dm = dmAutomatic;
	}
	if( PBoxTemp->DragMode != dm ) PBoxTemp->DragMode = dm;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AdjustTempView(int sw)
{
	if( pBitmapTemp->Width == 320 ) return;
	if( sw ){
		::SetMapMode(PBoxTemp->Canvas->Handle, MM_ANISOTROPIC);
		::SetWindowExtEx(PBoxTemp->Canvas->Handle, pBitmapTemp->Width, pBitmapTemp->Height, NULL);
		::SetViewportExtEx(PBoxTemp->Canvas->Handle, 320, 256, NULL);
	}
	else {
		::SetMapMode(PBoxTemp->Canvas->Handle, MM_TEXT);
		::SetWindowExtEx(PBoxTemp->Canvas->Handle, 320, 256, NULL);
		::SetViewportExtEx(PBoxTemp->Canvas->Handle, 320, 256, NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateTemp(void)
{
	if( (pBitmapTemp->Width != DrawMain.m_SX)||(pBitmapTemp->Height != DrawMain.m_SY) ){
		pBitmapTemp->Width = DrawMain.m_SX;
		pBitmapTemp->Height = DrawMain.m_SY;
	}
	TRect rc;
	rc.Left = 0;
	rc.Top = 0;
	rc.Right = pBitmapTemp->Width;
	rc.Bottom = pBitmapTemp->Height;
	pBitmapTemp->Canvas->Brush->Style = bsSolid;
	pBitmapTemp->Canvas->Brush->Color = DrawMain.m_TransCol;
	pBitmapTemp->Canvas->Pen->Color = DrawMain.m_TransCol;
	pBitmapTemp->Canvas->FillRect(rc);

	MultProc();
	if( m_ChangeTemp ){
		DrawMain.UpdateText();
		m_ChangeTemp = 0;
		SBULog->Enabled = FALSE;
	}
	else {
		DrawMain.UpdateTimeText();
	}
	MultProc();
//    DrawMain.UpdatePic(pBitmapHistF);
	if( KTAP->Checked ) DrawMain.AdjustTransPoint();
	if( m_MainPage == pgTemp ){
		DrawMain.DrawNotSel(pBitmapTemp->Canvas);
		if( DrawMain.pSel != NULL ) DrawMain.pSel->DrawSel(pBitmapTemp->Canvas, 0);
	}
	else {
		DrawMain.Draw(pBitmapTemp->Canvas);
	}
	MultProc();
	if( KTBT->Checked && !SBTX->Down ){
		int sv = m_TxClipType;
		if( (m_TxClipType == 4) || (m_TxClipType == 2) ) m_TxClipType = 0;
		MakeTxBitmap(0);
		m_TxClipType = sv;
		DrawBitmap(PBoxTemp, pBitmapTX);
		if( DrawMain.pSel != NULL ){
			AdjustTempView(1);
			DrawMain.pSel->DrawSel(PBoxTemp->Canvas, KTBT->Checked);
			AdjustTempView(0);
		}
	}
	else {
		DrawBitmap(PBoxTemp, pBitmapTemp);
//        PBoxTemp->Canvas->Draw(0, 0, pBitmapTemp);
	}

	MultProc();
	rc.Right = PBoxTemp->Width;
	rc.Bottom = PBoxTemp->Height;
	PBoxTemp->Canvas->Brush->Color = clGreen;
	PBoxTemp->Canvas->FrameRect(rc);
	if( KTTD->Checked ){
		AdjustTempView(1);
		DrawMain.DrawTrans(PBoxTemp->Canvas);
		AdjustTempView(0);
	}
#if 0
	if( m_MainPage == pgTemp ){
		TempGrid.Draw(PBoxTemp->Canvas, DrawMain.m_SX, DrawMain.m_SY);
	}
#endif
	if( sys.m_ShowSizeTemp ){
		TColor col = (DrawMain.m_SX != pBitmapTXM->Width) ? clRed : clGreen;
		ShowSize(PBoxTemp, DrawMain.m_SX, DrawMain.m_SY, 12, col);
	}
	UpdateUITemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBULogClick(TObject *Sender)
{
	if( m_ReqHistF ) AdjustHistF();
	UpdatePic(1);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdatePic(void)
{
	UpdatePic(0);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdatePic(int sw)
{
	if( m_MainPage == pgHist ){
		PBoxHist->Invalidate();
	}
	UpdateTemp();
	MakeTxBitmap(sw);
	PBoxTXPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxTempDblClick(TObject *Sender)
{
	if( DrawMain.pSel != NULL ){
		TMemoryStream *sp = SaveUndo(NULL);
		if( DrawMain.pSel->Edit() == TRUE ){
			if( KTAP->Checked ) DrawMain.AdjustTransPoint();
			UpdateTemp();
			UpdateUndo(sp);
		}
		else {
			delete sp;
		}
	}
	if( PBoxTemp->Dragging() ) PBoxTemp->EndDrag(FALSE);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDFontClick(TObject *Sender)
{
	if( DrawMain.pSel != NULL ){
		if( DrawMain.pSel->m_Command == CM_TEXT ){
			CDrawText *pText = (CDrawText *)(DrawMain.pSel);
			FontDialog->Font->Assign(pText->pFont);
			FontDialog->Font->Color = pText->m_Col1;
			SetDisPaint();
			TMemoryStream *sp = SaveUndo(NULL);
			if( FontDialog->Execute() == TRUE ){
				pText->m_Col1 = FontDialog->Font->Color;
				pText->UpdateFont(FontDialog->Font);
				if( KTAP->Checked ) DrawMain.AdjustTransPoint();
				UpdateTemp();
				UpdateUndo(sp);
			}
			else {
				delete sp;
			}
			ResDisPaint();
		}
		else if( DrawMain.pSel->m_Command == CM_LIB ){
			CDrawLib *pLib = (CDrawLib *)DrawMain.pSel;
			TMemoryStream *sp = SaveUndo(NULL);
			if( pLib->Font() ){
				UpdateTemp();
				UpdateUndo(sp);
			}
			else {
				delete sp;
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDLinesetClick(TObject *Sender)
{
	if( DrawMain.pSel != NULL ){
		TMemoryStream *sp = SaveUndo(NULL);
		if( DrawMain.pSel->CDraw::Edit() == TRUE ){
			if( DrawMain.pSel->m_Command == CM_TEXT ){
				DrawMain.pSel->UpdateText();
			}
			else if( DrawMain.pSel->m_Command == CM_PIC ){
				CDrawPic *pPic = (CDrawPic *)DrawMain.pSel;
				if( pPic->m_Shape != 5 ){
					sys.m_PicLineStyle = pPic->m_LineStyle;
					sys.m_PicLineColor = pPic->m_LineColor;
				}
			}
			if( KTAP->Checked ) DrawMain.AdjustTransPoint();
			UpdateTemp();
			UpdateUndo(sp);
		}
		else {
			delete sp;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDColClick(TObject *Sender)
{
	if( DrawMain.pSel != NULL ){
		TMemoryStream *sp = SaveUndo(NULL);
		if( DrawMain.pSel->Color() == TRUE ){
			if( KTAP->Checked ) DrawMain.AdjustTransPoint();
			UpdateTemp();
			UpdateUndo(sp);
		}
		else {
			delete sp;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxTXDragOver(TObject *Sender, TObject *Source,
	  int X, int Y, TDragState State, bool &Accept)
{
	Accept = FALSE;
	if( Source == PBoxS ){
		if( SBTX->Down ) return;
		if( (m_MainPage != pgTemp ) && TabS->TabIndex ){
			AdjustPage(pgTemp);
		}
		else if( (m_MainPage != pgTX) && !TabS->TabIndex ){
			AdjustPage(pgTX);
		}
		Accept = TRUE;
	}
	else if( (pHistView != NULL) && (pHistView->IsPBox(Source) >= 0) ){
		AdjustPage(pgTemp);
		Accept = TRUE;
	}
	else {
		if( SBTX->Down ) return;
		for( int i = 0; i < 8; i++ ){
			TFileViewDlg *fp = pFileView[i];
			if( (fp != NULL) && (fp->IsPBox(Source) >= 0) ){
				AdjustPage(fp->pCurPage->m_Type ? pgTemp : -1);
				Accept = TRUE;
				break;
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::GetTempHistSize(int &x, int &y)
{
	switch(sys.m_HDDSize){
		case 0:     // 1/16
			x += (80-1);
			y += (64-1);
			break;
		case 1:     // 1/12
			x += (92-1);
			y += (74-1);
			break;
		case 2:     // 1/9
			x += (107-1);
			y += (85-1);
			break;
		case 3:     // 1/6
			x += (131-1);
			y += (105-1);
			break;
		case 4:    // 1/4
			x += (160-1);
			y += (128-1);
			break;
		default:    // 1/3
			x += (185-1);
			y += (148-1);
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxTXDragDrop(TObject *Sender, TObject *Source,
	  int X, int Y)
{
	CWaitCursor w;

	if( Source == PBoxS ){
		if( m_DragNo >= 0 ){
			if( CBSTemp->Checked || TabS->TabIndex ){
				SaveUndo();
				LoadStockTemp(&DrawMain, m_DragDataNo, NULL);
				if( !KSIS->Checked || !DrawMain.m_Cnt ){
					if( !sys.m_Repeater || !pDem->m_Repeater){
						DrawMain.m_SX = m_TXW;
						DrawMain.m_SY = m_TXH;
					}
				}
			}
			if( !TabS->TabIndex ){
				Graphics::TBitmap *pBitmap = LoadBitmapS(m_DragNo);
				pBitmapTXM = RemakeBitmap(pBitmapTXM, -1);
				if( KSIS->Checked ){
					StretchCopyBitmapHW(pBitmapTXM, pBitmap);
				}
				else {
					FillBitmap(pBitmapTXM, sys.m_PicClipColor);
					pBitmapTXM->Canvas->Draw(0, 0, pBitmap);
				}
				delete pBitmap;
			}
			UpdatePic();
		}
	}
	else if( (pHistView != NULL) && (pHistView->IsPBox(Source) >= 0) ){
		CDrawPic *pDraw = (CDrawPic *)DrawMain.MakeItem(CM_PIC);
		if( pDraw != NULL ){
			AdjustTempSC(X, Y);
			AdjustTempView(1);
			pDraw->Start(PBoxTemp->Canvas, X, Y);
			GetTempHistSize(X, Y);
			pDraw->Making(X, Y);
			if( pDraw->FinishR(X, Y) ){
#if 0
				SaveUndo();
				DrawMain.AddItem(pDraw);
				DrawMain.pSel = pDraw;
				m_DrawCmd = CM_SELECT;
				SBDSel->Down = TRUE;
				KTBClick(NULL);
				if( KTAP->Checked ) DrawMain.AdjustTransPoint();
#else
				AddItem(pDraw, 0);
#endif
			}
			else {
				delete pDraw;
			}
			AdjustTempView(0);
			UpdateTemp();
		}
	}
	else {
		for( int i = 0; i < 8; i++ ){
			TFileViewDlg *fp = pFileView[i];
			if( (fp != NULL) && (fp->IsPBox(Source) >= 0) ){
				if( fp->pCurPage->m_Type ){
					AnsiString as;
					fp->GetCurFileName(as);
					switch(fp->pCurPage->m_Type){
						case 2:
							LoadTemplate(&DrawTemp, as.c_str(), NULL);
							AddTemplate(&DrawTemp);
							break;
						case 3:
							DropLib(X, Y, as.c_str());
							break;
						case 4:
							DropOle(X, Y, as.c_str(), 1);
							break;
						default:
							SaveUndo();
							LoadTemplate(&DrawMain, as.c_str(), NULL);
							break;
					}
					UpdateTemp();
				}
				else if( m_MainPage == pgTX ){
					pBitmapTXM = RemakeBitmap(pBitmapTXM, -1);
					if( pFileView[i]->CopyBitmap(pBitmapTXM) == TRUE ){
						UpdatePic();
					}
				}
				else {
					int w, h;
					pFileView[i]->GetSize(w, h);
					Graphics::TBitmap *pBitmap = CreateBitmap(w, h, -1);
					if( pFileView[i]->CopyBitmap(pBitmap) == TRUE ){
						DropPic(X, Y, pBitmap, pFileView[i]->GetType());
					}
					delete pBitmap;
				}
				break;
			}
		}
	}
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxSDragDrop(TObject *Sender,
	  TObject *Source, int X, int Y)
{
	if( (Source == PBoxTemp) || (Source == PBoxTX) ){
		int n = GetStockNo(X, Y);
		int nn = n + UDStock->Position * m_SMax;
		if( (n >= 0) && (nn < STOCKMAX) ){
			int Off = m_StockOff;
			if( !TabS->TabIndex ) Off = 0;
			if( CBSTemp->Checked || (Source == PBoxTemp) ){
				SaveStockTemp(&DrawMain, nn + Off);
			}
			if( !TabS->TabIndex ){
				SaveBitmapS(pBitmapTXM, n);
			}
			UpdateTabS();
		}
	}
	else {
		for( int i = 0; i < 8; i++ ){
			TFileViewDlg *fp = pFileView[i];
			if( (fp != NULL) && (fp->IsPBox(Source) >= 0) ){
				if( !TabS->TabIndex ){
					int n = GetStockNo(X, Y);
					int nn = n + UDStock->Position * m_SMax;
					if( (n >= 0) && (nn < STOCKMAX) ){
						Graphics::TBitmap *pBitmap = CreateBitmap(m_PSSize[n] & 0x0000ffff, m_PSSize[n]>>16, -1);
						if( fp->CopyBitmap(pBitmap) == TRUE ){
							SaveBitmapS(pBitmap, n);
							UpdateTabS();
						}
						delete pBitmap;
					}
				}
				break;
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxSDragOver(TObject *Sender,
	  TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
	Accept = FALSE;
	if( GetStockNo(X, Y) >= 0 ){
		if( Source == PBoxTemp ){
			if( !TabS->TabIndex ){
				TabS->TabIndex = 1;
				TabSChange(NULL);
				PBoxS->Invalidate();
			}
			Accept = TRUE;
		}
		else if( Source == PBoxTX ){
			if( TabS->TabIndex ){
				TabS->TabIndex = 0;
				TabSChange(NULL);
				PBoxS->Invalidate();
			}
			Accept = TRUE;
		}
		else {
			for( int i = 0; i < 8; i++ ){
				TFileViewDlg *fp = pFileView[i];
				if( (fp != NULL) && (fp->IsPBox(Source) >= 0) && !fp->pCurPage->m_Type ){
					if( TabS->TabIndex ){
						TabS->TabIndex = 0;
						TabSChange(NULL);
						PBoxS->Invalidate();
					}
					Accept = TRUE;
					break;
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxSMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_DragNo = GetStockNo(X, Y);
	m_DragDataNo = UDStock->Position * m_SMax + m_DragNo;
	if( m_DragDataNo >= STOCKMAX ){
		m_DragNo = -1;
		m_DragDataNo = 0;
	}
	else if( TabS->TabIndex ){
		m_DragDataNo += m_StockOff;
	}
	if( Button == mbLeft ){
		m_TabSXX = X;
		m_TabSYY = Y;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxSMouseMove(TObject *Sender, TShiftState Shift,
	  int X, int Y)
{
	TShiftState sc1, sc2;
	sc1 << ssLeft;
	sc2 << ssLeft;
	sc1 *= Shift;
	if( sc1 == sc2 ){			// Left button
		if( (ABS(m_TabSXX-X) + ABS(m_TabSYY-Y)) >= 10 ){
			if( m_DragNo >= 0 ) PBoxS->BeginDrag(TRUE,0);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxSDblClick(TObject *Sender)
{
	if( m_DragNo >= 0 ){
		if( TabS->TabIndex || !SBTX->Down ){
			AdjustPage( TabS->TabIndex ? pgTemp : pgTX);
			PBoxTXDragDrop(Sender, PBoxS, 0, 0);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::GetStockXY(int &X1, int &Y1, int &X2, int &Y2, int n)
{
	int mm = m_SMax / 6;
	if( !mm ) mm++;
	int xw = (PBoxS->Width / 6) - 4;
	int yw = (PBoxS->Height / mm) - 4;

	X1 = (((n % 6) * PBoxS->Width) / 6) + 2;
	X2 = X1 + xw;
	Y1 = (((n / 6) * PBoxS->Height) / mm) + 2;
	Y2 = Y1 + yw;
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::GetStockNo(int X, int Y)
{
	int i, x1, y1, x2, y2;
	for( i = 0; i < m_SMax; i++ ){
		GetStockXY(x1, y1, x2, y2, i);
		if( (X > x1)&&(X < x2)&&(Y > y1)&&(Y < y2) ) return i;
	}
	return -1;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateTabS(void)
{
	if( (pStockView != NULL) && !pStockView->Visible ) return;

	CWaitCursor wait;

	m_DisEvent++;

	Graphics::TBitmap *pBitmap = new Graphics::TBitmap();
	AdjustBitmapFormat(pBitmap);
	MultProc();

	int ktmd = KTMD->Checked;
	KTMD->Checked = TRUE;

	TRect rc;
	TRect sc;
	int x1, x2, y1, y2;
	int n;
	if( TabS->TabIndex ){      // テンプレート
		m_StockPage = TabS->TabIndex - 1;
		m_StockOff = m_StockPage * STOCKMAX;
		n = m_TSPage[m_StockPage] * m_SMax;
		Graphics::TBitmap *pBmpStock = pBitmapST[m_StockPage];
		if( pBmpStock == NULL ){
			pBmpStock = new Graphics::TBitmap();
			AdjustBitmapFormat(pBmpStock);
			pBitmapST[m_StockPage] = pBmpStock;
		}
		pBmpStock->Width = PBoxS->Width;
		pBmpStock->Height = PBoxS->Height;
		FillBitmap(pBmpStock, clWhite);
		pBmpStock->Canvas->Brush->Color = clBlack;
		pBmpStock->Canvas->Pen->Color = clBlack;
		MultProc();
		for( int i = 0; i < m_SMax; i++, n++ ){
			if( n >= STOCKMAX ) break;

			GetStockXY(x1, y1, x2, y2, i);
			rc.Top = y1; rc.Left = x1; rc.Right = x2; rc.Bottom = y2;
			LoadStockTemp(&DrawTemp, n + m_StockOff, pBitmap->Canvas);
			if( DrawTemp.m_Cnt ){
				pBitmap->Width = DrawTemp.m_SX;
				pBitmap->Height = DrawTemp.m_SY;
				FillBitmap(pBitmap, DrawTemp.m_TransCol);
//                DrawTemp.UpdatePic(pBitmapHistF);
				MultProc();
				if( CBSDraft->Checked ){
					DrawTemp.DrawFast(pBitmap->Canvas);
				}
				else {
					DrawTemp.Draw(pBitmap->Canvas);
				}
				MultProc();
				pBmpStock->Canvas->StretchDraw(rc, (TGraphic*)pBitmap);
				if( sys.m_ShowSizeStock ){
					TColor col = (DrawTemp.m_SX != pBitmapTXM->Width) ? clRed : clGreen;
					ShowSize(pBmpStock, x1, y2, DrawTemp.m_SX, DrawTemp.m_SY, 11, col);
					pBmpStock->Canvas->Brush->Color = clBlack;
				}
				if( sys.m_Repeater ){
					ShowNo(pBmpStock, x1, y1, n+m_StockOff+1, 11, clYellow);
					pBmpStock->Canvas->Brush->Color = clBlack;
				}
			}
			pBmpStock->Canvas->FrameRect(rc);
			MultProc();
		}
		m_ATSPage[m_StockPage] = m_TSPage[m_StockPage];
	}
	else {
		n = m_PSPage * m_SMax;
		if( pBitmapSS == NULL ){
			pBitmapSS = new Graphics::TBitmap();
			AdjustBitmapFormat(pBitmapSS);
		}
		pBitmapSS->Width = PBoxS->Width;
		pBitmapSS->Height = PBoxS->Height;
		FillBitmap(pBitmapSS, clWhite);
		pBitmapSS->Canvas->Brush->Color = clBlack;
		pBitmapSS->Canvas->Pen->Color = clBlack;
		MultProc();

		pBitmap->Transparent = TRUE;
		pBitmap->TransparentMode = tmFixed;

		Graphics::TBitmap *pBitmapSV = CreateBitmap(PBoxTX->Width, PBoxTX->Height, -1);
		MultProc();

		for( int i = 0; i < m_SMax; i++, n++ ){
			if( n >= STOCKMAX ) break;
			Graphics::TBitmap *pBitmapPic = LoadBitmapS(i);
			m_PSSize[i] = pBitmapPic->Width + (pBitmapPic->Height << 16);

			GetStockXY(x1, y1, x2, y2, i);
			rc.Top = y1; rc.Left = x1; rc.Right = x2; rc.Bottom = y2;

			if( CBSTemp->Checked ){
				LoadStockTemp(&DrawTemp, n, pBitmap->Canvas);
			}
			if( !pBitmapPic->Modified ){
				if( CBSTemp->Checked && DrawTemp.m_Cnt ){
					FillBitmap(pBitmapSV, clWhite);
				}
				else {
					pBitmapSS->Canvas->FrameRect(rc);
					delete pBitmapPic;
					continue;
				}
			}
			else if( pBitmapPic->Width != pBitmapSV->Width ){
				StretchCopy(pBitmapSV, pBitmapPic, HALFTONE);
			}
			else {
				pBitmapSV->Canvas->Draw(0, 0, pBitmapPic);
			}
			if( CBSTemp->Checked && DrawTemp.m_Cnt ){
				MultProc();
				pBitmap->Width = DrawTemp.m_SX;
				pBitmap->Height = DrawTemp.m_SY;
				FillBitmap(pBitmap, DrawTemp.m_TransCol);
				DrawTemp.UpdatePic(pBitmapHistF);
				MultProc();
				if( CBSDraft->Checked ){
					DrawTemp.DrawFast(pBitmap->Canvas);
				}
				else {
					DrawTemp.Draw(pBitmap->Canvas);
				}
				MultProc();
				pBitmap->TransparentColor = pBitmap->Canvas->Pixels[DrawTemp.m_TransX][DrawTemp.m_TransY];
				if( pBitmapSV->Width != pBitmap->Width ){
					StretchCopy(pBitmapSV, pBitmap, HALFTONE);
				}
				else {
					pBitmapSV->Canvas->Draw(0, 0, pBitmap);
				}
			}
			pBitmapSS->Canvas->StretchDraw(rc, pBitmapSV);
			pBitmapSS->Canvas->FrameRect(rc);
			if( sys.m_ShowSizeStock && pBitmapPic->Modified ){
				TColor col = (pBitmapPic->Width != pBitmapTXM->Width) ? clRed : clGreen;
				ShowSize(pBitmapSS, x1, y2, pBitmapPic->Width, pBitmapPic->Height, 11, col);
				pBitmapSS->Canvas->Brush->Color = clBlack;
			}
			if( sys.m_Repeater ){
				ShowNo(pBitmapSS, x1, y1, n+1, 11, clYellow);
				pBitmapSS->Canvas->Brush->Color = clBlack;
			}
			MultProc();
			delete pBitmapPic;
		}
		delete pBitmapSV;
		MultProc();
		m_APSPage = m_PSPage;
	}
	DrawTemp.FreeItem();
	delete pBitmap;
	KTMD->Checked = ktmd;
	m_DisEvent--;
	PBoxSPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxSPaint(TObject *Sender)
{
	if( TabS->TabIndex ){       // テンプレート
		m_StockPage = TabS->TabIndex - 1;
		m_StockOff = m_StockPage * STOCKMAX;
		if( pBitmapST[m_StockPage] == NULL ){
			UpdateTabS();
		}
		else {
			PBoxS->Canvas->Draw(0, 0, pBitmapST[m_StockPage]);
		}
	}
	else {                      // ストック画像
		if( pBitmapSS == NULL ){
			UpdateTabS();
		}
		else {
			PBoxS->Canvas->Draw(0, 0, pBitmapSS);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSTPClick(TObject *Sender)
{
	CWaitCursor wait;
	int x, y, n, l;

	const TColor  bc[]={
		clBlack, clWhite,
		clGreen, clBlue,
		clBlue, clRed,
		clRed, clYellow,
		clYellow, clLime,
		clLime, clPurple,
		clPurple, clOlive,
		clOlive, clBlack,
	};
	pBitmapTXM = RemakeBitmap(pBitmapTXM, -1);
	TColor col;
	int XW = pBitmapTXM->Width / 16;
	int YW = pBitmapTXM->Height / 8;
	for( n = l = 0; n < 16; n += 2, l += YW ){
		for( y = l; y < (l + YW); y++ ){
			for( x = 0; x < (pBitmapTXM->Width / 2); x++ ){
				col = GetCol(bc[n], bc[n+1], x, (pBitmapTXM->Width / 2));
				pBitmapTXM->Canvas->Pixels[x][y] = col;
			}
		}
	}
	l = pBitmapTXM->Width / 2;
	for( n = 0; n < 16; n += 2, l += XW ){
		for( x = l; x < (l + XW); x++ ){
			for( y = 0; y < pBitmapTXM->Height; y++ ){
				col = GetCol(bc[n], bc[n+1], y, pBitmapTXM->Height);
				pBitmapTXM->Canvas->Pixels[x][y] = col;
			}
		}
	}
	UpdatePic();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSAIClick(TObject *Sender)
{
	if( PopupS->PopupComponent == PBoxTX ){
		SBTXFilClick(NULL);
		return;
	}
	if( m_DragNo < 0 ) return;
	if( !TabS->TabIndex ){  // ビットマップ
		TPicFilterDlg *pBox = new TPicFilterDlg(this);
		Graphics::TBitmap *pBitmap = LoadBitmapS(m_DragNo);
		if( pBox->Execute(pBitmap) == TRUE ){
			SaveBitmapS(pBitmap, m_DragNo);
			UpdateTabS();
		}
		delete pBitmap;
		delete pBox;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::TabSChange(TObject *Sender)
{
	if( m_DisEvent ) return;
	if( (pStockView != NULL) && !pStockView->Visible ) return;

	int i;
	if( TabS->TabIndex == 0 ){
		if( !CBSTemp->Checked ) UDStock->Position = short(m_PSPage);
		if( UDStock->Position != m_TxSPage ){
			CWaitCursor wait;
			m_TxSPage = UDStock->Position;
		}
		::DragAcceptFiles(PanelS->Handle, TRUE);
		CBSTemp->Enabled = TRUE;
		CBSDraft->Enabled = CBSTemp->Checked;
		int f = 0;
		for( i = 0; i < 8; i++ ){
			if( (pFileView[i] != NULL) && pFileView[i]->m_Overlap ){
				pFileView[i]->Visible = TRUE;
				pFileView[i]->m_Suspend = 0;
				f++;
				MultProc();
			}
		}
		if( f && (pRxView != NULL) && (pRxView->Handle != ::GetForegroundWindow()) ){
			::SetForegroundWindow(pRxView->Handle);
		}
		if( m_PSPage != m_APSPage ) UpdateTabS();
	}
	else {
		m_StockPage = TabS->TabIndex - 1;
		if( !CBSTemp->Checked ) UDStock->Position = short(m_TSPage[m_StockPage]);
		::DragAcceptFiles(PanelS->Handle, FALSE);
		CBSTemp->Enabled = FALSE;
		CBSDraft->Enabled = TRUE;
		for( i = 0; i < 8; i++ ){
			if( (pFileView[i] != NULL) && pFileView[i]->m_Overlap ){
				pFileView[i]->Visible = FALSE;
				pFileView[i]->m_Suspend = 1;
				MultProc();
			}
		}
		if( m_TSPage[m_StockPage] != m_ATSPage[m_StockPage] ) UpdateTabS();
	}
	char bf[64];
	sprintf(bf, "%u/%u", UDStock->Position + 1, m_DragPMax);
	LS->Caption = bf;
	PBoxS->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UDStockClick(TObject *Sender, TUDBtnType Button)
{
	if( TabS->TabIndex ){
		m_TSPage[m_StockPage] = UDStock->Position;
		if( CBSTemp->Checked && (TabS->TabIndex == 1) ) m_PSPage = m_TSPage[0];
	}
	else {
		m_PSPage = UDStock->Position;
		if( CBSTemp->Checked ) m_TSPage[0] = m_PSPage;
	}
	TabSChange(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::CopyBitmap(Graphics::TBitmap *pBitmap)
{
	//int DataHandle;	//ja7ude 0521
	NativeUInt DataHandle;
	HPALETTE APalette;
	//unsigned short MyFormat;	//ja7ude 0521
	System::Word MyFormat;	//ja7ude 0521
	pBitmap->SaveToClipboardFormat(MyFormat, DataHandle, APalette);
	Clipboard()->SetAsHandle(MyFormat,DataHandle);
//	SBPaste->Enabled = TRUE;
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::PasteBitmap(Graphics::TBitmap *pBitmap, int rs)
{
	int r = FALSE;
	TClipboard *pCB = Clipboard();
	if (pCB->HasFormat(CF_BITMAP)){
		TPicRectDlg *pBox = new TPicRectDlg(this);
		try
		{
			pBox->pBitmap->LoadFromClipboardFormat(CF_BITMAP, pCB->GetAsHandle(CF_BITMAP), 0);
			if( rs == 2 ){
				r = pBox->Execute(pBitmap);
            }
			else if( rs ){
				::CopyBitmap(pBitmap, pBox->pBitmap);
				r = TRUE;
			}
			else if( pBox->Copy(pBitmap) == TRUE ){
				r = TRUE;
			}
		}
		catch (...)
		{
			ErrorMB( "Clipboard error." );
		}
		delete pBox;
	}
	return r;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SaveBitmapS(Graphics::TBitmap *pBitmap, int n)
{
	SaveBitmapSN(pBitmap, m_PSPage * m_SMax + n + 1);
}
//---------------------------------------------------------------------------
Graphics::TBitmap* __fastcall TMmsstv::LoadBitmapS(int n)
{
	Graphics::TBitmap *pBitmap = new Graphics::TBitmap;
	LoadBitmapSN(pBitmap, m_PSPage * m_SMax + n + 1);
	return pBitmap;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SaveBitmapSN(Graphics::TBitmap *pBitmap, int n)
{
	CWaitCursor wait;
	char bf[256];
	sprintf(bf, sys.m_UseJPEG ? "%sTxStock%u.jpg":"%sTxStock%u.bmp", StockDir, n);
	if( sys.m_UseJPEG ){
		SaveJPEG(pBitmap, bf);
	}
	else {
		SaveBitmap(pBitmap, bf);
	}
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LoadBitmapSN(Graphics::TBitmap *pBitmap, int n)
{
	CWaitCursor wait;
	MultProc();
	char bf[256];
	sprintf(bf, sys.m_UseJPEG ? "%sTxStock%u.jpg" : "%sTxStock%u.bmp", StockDir, n);
	if( IsFile(bf) ){
		MultProc();
		if( sys.m_UseJPEG ){
			LoadJPEG(pBitmap, bf);
		}
		else {
			LoadBitmap(pBitmap, bf);
		}
		pBitmap->Modified = TRUE;
	}
	else {
		pBitmap->Width = 320;
		pBitmap->Height = 256;
		FillBitmap(pBitmap, clWhite);
		pBitmap->Modified = FALSE;
	}
}
//---------------------------------------------------------------------------
// ファイルからロード
int __fastcall TMmsstv::LoadBitmapMenu(Graphics::TBitmap *pBitmap, int rs)
{
	int r = FALSE;

	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options << ofFileMustExist;
	if( MsgEng ){
		OpenDialog->Title = "Load picture from file";
	}
	else {
		OpenDialog->Title = "画像のロード";
	}
	OpenDialog->Filter = GetPicFilter();
	OpenDialog->FileName = "";
	OpenDialog->DefaultExt = "jpg";
	OpenDialog->InitialDir = BitmapDir;
	SetDisPaint();
	NormalWindow(this);
	if( OpenDialogExecute(TRUE) == TRUE ){
		SetDirName(BitmapDir, AnsiString(OpenDialog->FileName).c_str());
		TPicRectDlg *pBox = new TPicRectDlg(this);
		if( LoadImage(pBox->pBitmap, AnsiString(OpenDialog->FileName).c_str()) == TRUE ){
			if( rs ){
				::CopyBitmap(pBitmap, pBox->pBitmap);
				r = TRUE;
			}
			else if( pBox->Execute(pBitmap) == TRUE ){
				r = TRUE;
			}
		}
		delete pBox;
	}
	ResDisPaint();
	TopWindow(this);
	return r;
}

int __fastcall TMmsstv::SaveBitmapMenu(Graphics::TBitmap *pBitmap, LPCSTR pName, LPCSTR pFolder)
{
	int r = FALSE;

	if( MsgEng ){
		SaveDialog->Title = "Save picture";
	}
	else {
		SaveDialog->Title = "画像をセーブ";
	}
	SaveDialog->Filter = "JPEG files(*.jpg)|*.jpg|Bitmap files(*.bmp)|*.bmp|";
	SaveDialog->FileName = pName != NULL ? pName : "Picture";
	SaveDialog->DefaultExt = "jpg";
	SaveDialog->InitialDir = pFolder != NULL ? pFolder : SBitmapDir;
	SetDisPaint();
	NormalWindow(this);
	if( SaveDialog->Execute() == TRUE ){
		if( pFolder == NULL ){
			SetDirName(SBitmapDir, AnsiString(SaveDialog->FileName).c_str());
		}
		if( SaveImage(pBitmap, AnsiString(SaveDialog->FileName).c_str()) == TRUE ){
			r = TRUE;
		}
	}
	TopWindow(this);
	ResDisPaint();
	return r;
}
//---------------------------------------------------------------------------
// ファイルからロード
int __fastcall TMmsstv::LoadTemplateMenu(CDrawGroup *pItem, int isw)
{
	MultProc();
	int r = FALSE;
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options << ofFileMustExist;
	if( MsgEng ){
		OpenDialog->Title = "Load template from file";
	}
	else {
		OpenDialog->Title = "テンプレートファイルのロード";
	}
	if( isw ){
		char bf[256];
		sprintf(bf, "%s|%s", GetTempIFilter(), GetTempMFilter());
		OpenDialog->Filter = bf;
		OpenDialog->DefaultExt = "mti";
	}
	else {
		OpenDialog->Filter = GetTempMFilter();
		OpenDialog->DefaultExt = "mtm";
	}
	OpenDialog->FileName = "";
	OpenDialog->InitialDir = TemplateDir;
	SetDisPaint();
	MultProc();
	NormalWindow(this);
	if( OpenDialogExecute(TRUE) == TRUE ){
		MultProc();
		SetDirName(TemplateDir, AnsiString(OpenDialog->FileName).c_str());     //ja7ude 0521
		SaveUndo();
		r = LoadTemplate(pItem, AnsiString(OpenDialog->FileName).c_str(), NULL);     //ja7ude 0521
	}
	ResDisPaint();
	TopWindow(this);
	MultProc();
	return r;
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::SaveTemplateMenu(CDrawGroup *pItem, int isw)
{
	return SaveTemplateMenu(pItem, NULL, NULL, isw);
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::SaveTemplateMenu(CDrawGroup *pItem, LPCSTR pName, LPCSTR pFolder, int isw)
{
	MultProc();
	int r = FALSE;
	if( MsgEng ){
		SaveDialog->Title = "Save template";
	}
	else {
		SaveDialog->Title = "テンプレートをセーブ";
	}
	if( isw ){
		SaveDialog->Filter = GetTempIFilter();
		SaveDialog->FileName = pName != NULL ? pName : "Item";
		SaveDialog->DefaultExt = "mti";
	}
	else {
		SaveDialog->Filter = GetTempFilter();
		SaveDialog->FileName = pName != NULL ? pName : "Template";
		SaveDialog->DefaultExt = "mtm";
		SaveDialog->FilterIndex = 1;
	}
	SaveDialog->InitialDir = pFolder != NULL ? pFolder : TemplateDir;
	SetDisPaint();
	MultProc();
	NormalWindow(this);
	if( SaveDialog->Execute() == TRUE ){
		MultProc();
		if( pFolder == NULL ){
			SetDirName(TemplateDir, AnsiString(SaveDialog->FileName).c_str());	//ja7ude 0521
		}
		r = SaveTemplate(pItem, AnsiString(SaveDialog->FileName).c_str());	//ja7ude 0521
	}
	TopWindow(this);
	ResDisPaint();
	MultProc();
	return r;
}
//---------------------------------------------------------------------------
// ファイルからロード
void __fastcall TMmsstv::KSLClick(TObject *Sender)
{
	if( PopupS->PopupComponent == PBoxTX ){
		if( LoadBitmapMenu(pBitmapTXM, 0) == TRUE ){
			SBUseTempClick(NULL);
		}
		return;
	}
	if( m_DragNo < 0 ) return;
	if( TabS->TabIndex ){   // テンプレート
		if( LoadTemplateMenu(&DrawTemp, 0) == TRUE ){
			SaveStockTemp(&DrawTemp, m_DragDataNo);
			UpdateTabS();
		}
		DrawTemp.FreeItem();
	}
	else {                  // ビットマップ
		Graphics::TBitmap *pBitmap = CreateBitmap(m_PSSize[m_DragNo] & 0x0000ffff, m_PSSize[m_DragNo]>>16, -1);
//        Graphics::TBitmap *pBitmap = LoadBitmapS(m_DragNo);
		if( LoadBitmapMenu(pBitmap, 0) == TRUE ){
			SaveBitmapS(pBitmap, m_DragNo);
			UpdateTabS();
		}
		delete pBitmap;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSSClick(TObject *Sender)
{
	if( PopupS->PopupComponent == PBoxTX ){
		Graphics::TBitmap *pBitmap = pBitmapTX;
		if( m_TXPH == 240 ){
			pBitmap = CreateBitmap(m_TXW, m_TXPH, pf24bit);
			pBitmap->Canvas->Draw(0, 0, pBitmapTX);
        }
		SaveBitmapMenu(pBitmap, NULL, NULL);
        if( pBitmap != pBitmapTX ) delete pBitmap;
		return;
	}
	if( m_DragNo < 0 ) return;
	if( TabS->TabIndex ){   // テンプレート
		LoadStockTemp(&DrawTemp, m_DragDataNo, NULL);
		SaveTemplateMenu(&DrawTemp, 0);
		DrawTemp.FreeItem();
	}
	else {                  // ビットマップ
		Graphics::TBitmap *pBitmap = LoadBitmapS(m_DragNo);
		SaveBitmapMenu(pBitmap, NULL, NULL);
		delete pBitmap;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSCClick(TObject *Sender)
{
	if( PopupS->PopupComponent == PBoxTX ){
		Graphics::TBitmap *pBitmap = pBitmapTX;
		if( m_TXPH == 240 ){
			pBitmap = CreateBitmap(m_TXW, m_TXPH, pf24bit);
			pBitmap->Canvas->Draw(0, 0, pBitmapTX);
    	}
		CopyBitmap(pBitmap);
	    if( pBitmap != pBitmapTX ) delete pBitmap;
		return;
	}
	if( m_DragNo < 0 ) return;
	if( !TabS->TabIndex ){  // ビットマップ
		Graphics::TBitmap *pBitmap = LoadBitmapS(m_DragNo);
		CopyBitmap(pBitmap);
		delete pBitmap;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSPClick(TObject *Sender)
{
	if( PopupS->PopupComponent == PBoxTX ){
		if( PasteBitmap(pBitmapTXM, 0) == TRUE ){
			SBUseTempClick(NULL);
		}
		return;
	}
	if( m_DragNo < 0 ) return;
	if( !TabS->TabIndex ){       // ビットマップ
		Graphics::TBitmap *pBitmap = CreateBitmap(m_PSSize[m_DragNo] & 0x0000ffff, m_PSSize[m_DragNo]>>16, -1);
//        Graphics::TBitmap *pBitmap = LoadBitmapS(m_DragNo);
		if( PasteBitmap(pBitmap, 0) == TRUE ){
			SaveBitmapS(pBitmap, m_DragNo);
			UpdateTabS();
		}
		delete pBitmap;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSDClick(TObject *Sender)
{
	if( PopupS->PopupComponent == PBoxTX ){
		pBitmapTXM = RemakeBitmap(pBitmapTXM, -1);
		FillBitmap(pBitmapTXM, sys.m_PicClipColor);
		SBUseTempClick(NULL);
		return;
	}
	if( m_DragNo < 0 ) return;
	if( TabS->TabIndex ){   // テンプレート
		DeleteStockTemp(m_DragDataNo);
		UpdateTabS();
	}
	else {                  // ビットマップ
		CWaitCursor wait;
		char bf[256];
		sprintf(bf, sys.m_UseJPEG ? "%sTxStock%u.jpg" : "%sTxStock%u.bmp", StockDir, m_DragDataNo + 1);
		unlink(bf);
		UpdateTabS();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSSZClick(TObject *Sender)
{
	if( PopupS->PopupComponent == PBoxTX ) return;
	if( m_DragNo < 0 ) return;
	if( !TabS->TabIndex ){   // テンプレート
		int w, h;
		if( Sender == KSS1 ){
			w = 320; h = 256;
		}
		else if( Sender == KSS2 ){
			w = 512; h = 400;
		}
		else if( Sender == KSS3 ){
			w = 640; h = 496;
		}
		else {
			w = 800; h = 616;
		}
		Graphics::TBitmap *pBitmap = LoadBitmapS(m_DragNo);
		ReSizeBitmap(pBitmap, w, h);
		SaveBitmapS(pBitmap, m_DragNo);
		delete pBitmap;
		UpdateTabS();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PopupSPopup(TObject *Sender)
{
	MultProc();
	SBPaste->Enabled = ::IsClipboardFormatAvailable(CF_BITMAP);
	KSTD->Checked = CBSDraft->Checked;
	if( PopupS->PopupComponent == PBoxTX ){ // Tx Window
		KSF->Enabled = TRUE;
		KSB->Enabled = TRUE;
		KSCD->Enabled = FALSE;
		KSTP->Enabled = TRUE;
		KSAM->Enabled = TRUE;
		KSIS->Enabled = TRUE;
		KSRR->Enabled = TRUE;
		KSSD->Checked = sys.m_ShowSizeTX;
		KSTS->Enabled = SBUseTemp->Enabled;
		KSTD->Enabled = FALSE;
		KSTS->Checked = SBUseTemp->Down;
	}
	else {
		KSF->Enabled = FALSE;
		KSB->Enabled = FALSE;
		KSCD->Enabled = TRUE;
		KSTP->Enabled = FALSE;
        KSAM->Enabled = FALSE;
		KSIS->Enabled = FALSE;
		KSRR->Enabled = FALSE;
		KSSD->Checked = sys.m_ShowSizeStock;
		KSTS->Enabled = CBSTemp->Enabled;
		KSTD->Enabled = CBSDraft->Enabled;
		KSTS->Checked = CBSTemp->Checked;
	}
	MultProc();
	if( PopupS->PopupComponent == PBoxTX ){ // Tx Window
		KSC->Enabled = TRUE;
		KSP->Enabled = SBPaste->Enabled;
		KSFJ->Enabled = FALSE;
		KSFB->Enabled = FALSE;
		KSAI->Enabled = TRUE;
		KSSZ->Enabled = FALSE;
		KSMS->Enabled = FALSE;
		KSA->Enabled = FALSE;
		KSE->Enabled = TRUE;
	}
	else if( TabS->TabIndex ){   // テンプレート
		KSC->Enabled = FALSE;
		KSP->Enabled = FALSE;
		KSFJ->Enabled = FALSE;
		KSFB->Enabled = FALSE;
		KSAI->Enabled = FALSE;
		KSSZ->Enabled = FALSE;
		KSMS->Enabled = FALSE;
		KSA->Enabled = TRUE;
		KSE->Enabled = FALSE;
	}
	else {                  // ビットマップ
		KSC->Enabled = TRUE;
		KSP->Enabled = SBPaste->Enabled;
		KSFJ->Enabled = !sys.m_UseJPEG;
		KSFB->Enabled = sys.m_UseJPEG;
		KSAI->Enabled = TRUE;
		KSMS->Enabled = TRUE;
		KSA->Enabled = FALSE;
		if( m_DragNo >= 0 ){
			KSSZ->Enabled = TRUE;
			switch(m_PSSize[m_DragNo] & 0x0000ffff){
				case 512:
					KSS2->Checked = TRUE;
					break;
				case 640:
					KSS3->Checked = TRUE;
					break;
				case 800:
					KSS4->Checked = TRUE;
					break;
				default:
					KSS1->Checked = TRUE;
					break;
			}
		}
		else {
			KSSZ->Enabled = FALSE;
		}
		KSE->Enabled = TRUE;
	}
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PopupHPopup(TObject *Sender)
{
	KHD->Enabled = UDHist->Position < RxHist.m_Head.m_Cnt;
	KHDA->Enabled = RxHist.m_Head.m_Cnt;
	KHFB->Enabled = RxHist.m_UseJPEG;
	KHFJ->Enabled = !RxHist.m_UseJPEG;
	switch(sys.m_HDDSize){
		case 0:
			KHDS1->Checked = TRUE;
			break;
		case 1:
			KHDS2->Checked = TRUE;
			break;
		case 2:
			KHDS3->Checked = TRUE;
			break;
		case 3:
			KHDS4->Checked = TRUE;
			break;
		case 4:
			KHDS5->Checked = TRUE;
			break;
		default:
			KHDS6->Checked = TRUE;
			break;
	}
	KHTB->Checked = sys.m_HistViewTB;
	if( PopupH->PopupComponent != PBoxHist ){
		KHDS->Enabled = TRUE;
		KHTB->Enabled = TRUE;
		KHSD->Enabled = FALSE;
		KHVS->Enabled = TRUE;
	}
	else {
		KHDS->Enabled = FALSE;
		KHTB->Enabled = FALSE;
		KHSD->Enabled = TRUE;
		KHVS->Enabled = FALSE;
	}
	KHSD->Checked = sys.m_ShowSizeHist;
	switch(sys.m_HistViewSize){
		case 0:
			KHVS1->Checked = TRUE;
			break;
		case 1:
			KHVS2->Checked = TRUE;
			break;
		case 2:
			KHVS3->Checked = TRUE;
			break;
		default:
			KHVS4->Checked = TRUE;
			break;
	}
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PopupTPopup(TObject *Sender)
{
	MultProc();
	int cmd = DrawMain.pSel ? DrawMain.pSel->m_Command : 0;
	KTU->Enabled = pUndo != NULL ? TRUE : FALSE;
	KTC->Enabled = cmd;
	KTPA->Enabled = (cmd == CM_PIC);
	if( cmd != CM_PIC ){
		KTFX->Caption = MsgEng ? "Convert to image item" : "画像アイテムに変換";
	}
	else {
		KTFX->Caption = MsgEng ? "Convert to bitmap" : "ビットマップに変換";
	}
	KTFX->Enabled = DrawMain.IsConvBitmap(DrawMain.pSel);
	KTD->Enabled = SBDDel->Enabled;
	KTE->Enabled = (cmd == CM_TEXT)||((cmd == CM_LIB)&&((((CDrawLib *)DrawMain.pSel)->m_Type & 3)==2));
	KTB->Enabled = SBDDel->Enabled;
	KTF->Enabled = SBDDel->Enabled;
	KTP->Enabled = pPaste != NULL;
	KTS->Enabled = DrawMain.m_Cnt ? TRUE : FALSE;
	KTSI->Enabled = cmd ? TRUE : FALSE;
	KTA->Enabled = KTS->Enabled;
	KTBT->Enabled = !SBTX->Down;
	KTPI->Enabled = Clipboard()->HasFormat(CF_BITMAP);
	switch(DrawMain.m_SX){
		case 512:
			KTSZ2->Checked = TRUE;
			break;
		case 640:
			KTSZ3->Checked = TRUE;
			break;
		case 800:
			KTSZ4->Checked = TRUE;
			break;
		default:
			KTSZ1->Checked = TRUE;
			break;
	}
	KTSZT->Enabled = pBitmapTemp->Width != pBitmapTXM->Width;
	KTSD->Checked = sys.m_ShowSizeTemp;
	KTKA->Enabled = cmd && (!DrawMain.pSel->IsOrgSize());
	KTOS->Enabled = KTKA->Enabled;
	KTFS->Checked = sys.m_DisFontSmooth;
    KTH->Checked = sys.m_Temp24;
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTOClick(TObject *Sender)
{
	int cmd = DrawMain.pSel ? DrawMain.pSel->m_Command : 0;
	int fole = (cmd == CM_OLE);
	int flib = (cmd == CM_LIB);
	CDrawOle *pOle = (CDrawOle *)DrawMain.pSel;
	if( fole ) fole = (pOle->pContainer != NULL);
	KTOT->Enabled = fole;
	KTOC->Enabled = fole;
	KTOCB->Enabled = fole || flib;
	KTOE->Enabled = fole;
	KTOB->Enabled = fole;
	KTOR->Enabled = fole || flib;
	if( fole ){
		KTOT->Checked = pOle->m_Trans;
		KTOB->Checked = pOle->m_Stretch;
	}
	TOleContainer *pO = new TOleContainer(this);
	KTOP->Enabled = pO->CanPaste || ::IsClipboardFormatAvailable(CF_BITMAP);
	delete pO;
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTOIClick(TObject *Sender)
{
	AdjustTempSC(m_TempX, m_TempY);
	MultProc();
	CDrawOle *pDraw = (CDrawOle *)DrawMain.MakeItem(CM_OLE);
	pDraw->Start(PBoxTemp->Canvas, m_TempX, m_TempY);
	pDraw->Making(m_TempX, m_TempY);
	if( pDraw->Finish(m_TempX+160, m_TempY + 128) ){
		AddItem(pDraw, 0);
	}
	else {
		delete pDraw;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTCIClick(TObject *Sender)
{
	AdjustTempSC(m_TempX, m_TempY);
	MultProc();
	CDrawLib *pDraw = (CDrawLib *)DrawMain.MakeItem(CM_LIB);
	pDraw->Start(PBoxTemp->Canvas, m_TempX, m_TempY);
	pDraw->Making(m_TempX, m_TempY);
	if( pDraw->Finish(m_TempX+160, m_TempY + 128) ){
		AddItem(pDraw, 0);
	}
	else {
		delete pDraw;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTOCClick(TObject *Sender)
{
	if( DrawMain.pSel == NULL ) return;
//    if( DrawMain.pSel->m_Command != CM_OLE ) return;
	CWaitCursor w;
	CDrawOle *pDraw = (CDrawOle *)DrawMain.pSel;
	if( Sender == KTOC ){
		pDraw->pContainer->Copy();
	}
	else if( Sender == KTOE ){
		pDraw->DoPrimary();
		UpdateTemp();
	}
	else if( Sender == KTOB ){
		pDraw->m_Stretch = pDraw->m_Stretch ? 0 : 1;
		UpdateTemp();
	}
	else if( Sender == KTOT ){
		pDraw->m_Trans = pDraw->m_Trans ? 0 : 1;
		UpdateTemp();
	}
	else if( Sender == KTOCB ){
		if( pDraw->m_Command == CM_OLE ){
			Graphics::TBitmap *pBitmap = pDraw->MakeBitmap();
			CopyBitmap(pBitmap);
			delete pBitmap;
		}
		else {
			CDrawLib *pLib = (CDrawLib *)DrawMain.pSel;
			CopyBitmap(pLib->pBitmap);
		}
	}
	else if( Sender == KTOR ){
		if( pDraw->m_Command == CM_OLE ){
			if( pDraw->ObjectProperties() ){
				UpdateTemp();
			}
		}
		else {
			CDrawLib *pLib = (CDrawLib *)DrawMain.pSel;
			pLib->ObjectProperties();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AddItem(CDraw *pItem, int b)
{
	MultProc();
	SaveUndo();
	DrawMain.AddItem(pItem);
	DrawMain.pSel = pItem;
	m_DrawCmd = CM_SELECT;
	SBDSel->Down = TRUE;
	if( b ) KTBClick(NULL);
	if( KTAP->Checked ) DrawMain.AdjustTransPoint();
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTOPClick(TObject *Sender)
{
	CDrawOle *pDraw = (CDrawOle *)DrawMain.MakeItem(CM_OLE);
	TOleContainer *pO = new TOleContainer(this);
	int f = pO->CanPaste;
	delete pO;
	pDraw->m_X1 = m_TempX; pDraw->m_Y1 = m_TempY;
	if( f ){
		if( !pDraw->Paste() ){
			delete pDraw;
			pDraw = NULL;
		}
	}
	else {
		Graphics::TBitmap *pBitmap = new Graphics::TBitmap;
		TClipboard *pCB = Clipboard();
		try{
			pBitmap->LoadFromClipboardFormat(CF_BITMAP, pCB->GetAsHandle(CF_BITMAP), 0);
			pDraw->LoadFromBitmap(m_TempX, m_TempY, pBitmap);
		}
		catch(...){
			delete pDraw;
			pDraw = NULL;
		}
		delete pBitmap;
	}
	if( pDraw != NULL ) AddItem(pDraw, 0);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTUClick(TObject *Sender)
{
	if( pUndo != NULL ){
		pUndo->Seek(0, soFromBeginning);
		DrawMain.LoadFromStream(PBoxTemp->Canvas, pUndo);
		UpdateTemp();
		DeleteUndo();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SetGreetingString(LPSTR t, LPCSTR pCall, int type)
{
	strcpy(t, type ? "Hi" : "HELLO");
	if( !Cty.IsData() ) return;
	LPCSTR	p = ClipCC(pCall);
	if( *p ){
		int		n;
		if( (n = Cty.GetNoP(p))!=0 ){
			CTL *cp = Cty.GetCTL(n-1);
			if( cp->TD != NULL ){
				SYSTEMTIME	now;
				GetUTC(&now);

				WORD tim = WORD((now.wHour * 60 + now.wMinute) * 30 + now.wSecond/2);
				tim = AdjustRolTimeUTC(tim, *cp->TD);
				if( tim ){
					tim /= WORD(30);
					if( tim < 12*60 ){
						strcpy(t, type ? "GM" : "Good morning");
					}
					else if( tim < 18*60 ){
						strcpy(t, type ? "GA" : "Good afternoon");
					}
					else {
						strcpy(t, type ? "GE" : "Good evening");
					}
				}
			}
		}

	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::MacroDate(LPSTR t, SYSTEMTIME &now)
{
	switch(Log.m_LogSet.m_DateType){
		case 2:
		case 3:
			sprintf(t, "%02u-%s-%04u", now.wDay, MONT1[now.wMonth], now.wYear);
			break;
		case 4:
			case 5:
			sprintf(t, "%s-%02u-%04u", MONT1[now.wMonth], now.wDay, now.wYear);
			break;
		default:
			sprintf(t, "%04u-%s-%02u", now.wYear, MONT1[now.wMonth], now.wDay);
			break;
	}
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::MacroText(LPSTR t, LPCSTR p, int size)
{
	int r = 0;
	int n, l;
	char	bf[256];
	SYSTEMTIME	now;
	int f = KTMD->Checked;

	for( n = 0;*p; p++ ){
		if( *p == '%' ){
			p++;
			switch(*p){
				case 'm':
					strcpy(t, sys.m_Call.c_str());
					break;
				case 'C':
				case 'c':
					strcpy(t, HisCall->Text.IsEmpty() ? "ToCall" : AnsiString(HisCall->Text).c_str());	//ja7ude 0521
					if( *p == 'C' ) strlwr(t);
					break;
				case 'n':
					strcpy(t, (f && HisName->Text.IsEmpty()) ? "OM" : AnsiString(HisName->Text).c_str());	//ja7ude 0521
					break;
				case 'J':
					if( !HisName->Text.IsEmpty() ){
						strcpy(bf, AnsiString(HisName->Text).c_str());	//ja7ude 0521
						LPSTR tt;
						StrDlm(tt, bf);
						if( IsJA(AnsiString(HisCall->Text).c_str()) || (*tt < 0) ){ 	//ja7ude 0521
							sprintf(t, "%sさん", tt);
						}
						else {
							strcpy(t, tt);
						}
					}
					else {
						*t = 0;
					}
					break;
				case 'q':
					strcpy(t, (f && HisQTH->Text.IsEmpty()) ? "His/Her QTH" : AnsiString(HisQTH->Text).c_str());	//ja7ude 0521
					break;
				case 'r':
					strcpy(t, (f && HisRST->Text.IsEmpty()) ? "RSV" : AnsiString(HisRST->Text).c_str());	//ja7ude 0521
					break;
				case 's':
					strcpy(t, (f && MyRST->Text.IsEmpty()) ? "RSV" : AnsiString(MyRST->Text).c_str());	//ja7ude 0521
					break;
				case 'R':
					strcpy(bf, (f && HisRST->Text.IsEmpty()) ? "RST" : AnsiString(HisRST->Text).c_str());	//ja7ude 0521
					if( strlen(bf) >= 3 ){
						StrCopy(t, bf, 3);
					}
					else {
						strcpy(t, "595");
					}
					break;
				case 'N':
					strcpy(bf, (f && HisRST->Text.IsEmpty()) ? "RSVNR" : AnsiString(HisRST->Text).c_str());	//ja7ude 0521
					if( strlen(bf) > 3 ){
						strcpy(t, &bf[3]);
					}
					else {
						*t = 0;
					}
					break;
				case 'M':
					strcpy(bf, (f && MyRST->Text.IsEmpty()) ? "RSV" : AnsiString(MyRST->Text).c_str());
					if( strlen(bf) > 3 ){
						strcpy(t, &bf[3]);
					}
					else {
						*t = 0;
					}
					break;
				case 'g':
					SetGreetingString(t, AnsiString(HisCall->Text).c_str(), 0);	//ja7ude 0521
					r = 1;
					break;
				case 'f':
					SetGreetingString(t, AnsiString(HisCall->Text).c_str(), 1);	//ja7ude 0521
					r = 1;
					break;
				case 'D':
					GetUTC(&now);
					MacroDate(t, now);
					r = 1;
					break;
				case 'L':
					::GetLocalTime(&now);
					MacroDate(t, now);
					r = 1;
					break;
				case 'T':
					GetUTC(&now);
					sprintf(t, "%02u:%02u", now.wHour, now.wMinute);
					r = 1;
					break;
				case 't':
					GetUTC(&now);
					sprintf(t, "%02u%02u", now.wHour, now.wMinute);
					r = 1;
					break;
				case 'U':
					::GetLocalTime(&now);
					sprintf(t, "%02u:%02u", now.wHour, now.wMinute);
					r = 1;
					break;
				case 'u':
					::GetLocalTime(&now);
					sprintf(t, "%02u%02u", now.wHour, now.wMinute);
					r = 1;
					break;
				case 'v':
					strcpy(t, VERID);
					break;
				case 'V':
					strcpy(t, VERBETA);
					break;
				case 'B':
					strcpy(t, (f && LogFreq->Text.IsEmpty()) ? "Freq" : AnsiString(LogFreq->Text).c_str());	//ja7ude 0521
					break;
				case 'b':
					strcpy(t, (f && LogFreq->Text.IsEmpty()) ? "Band" : _BandText[Log.m_sd.band]);
					break;
				case 'o':
					{
						AnsiString ws;
						Yen2CrLf(ws, EditNote->Text);
						strcpy(t, (f && EditNote->Text.IsEmpty()) ? "Note" : ws.c_str());
					}
					break;
				case 'X':
					now = RxHist.m_File[RxHist.m_CurRead].m_UTC;
					MacroDate(t, now);
					sprintf(t+strlen(t), " %02u%02u", now.wHour, now.wMinute);
					r = 1;
					break;
				default:
					strcpy(t, "%%");
					break;
			}
			l = strlen(t);
			t += l;
			n += l;
		}
		else {
			*t++ = *p;
			n++;
		}
		if( n >= (size - 1) ) break;
	}
	*t = 0;
	return r;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::HisCallChange(TObject *Sender)
{
	TempDelay();
	SBULog->Enabled = TRUE;
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::CBSTempClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	if( Sender == KSTS ){
		if( PopupS->PopupComponent == PBoxTX ){ // Tx Window
			SBUseTemp->Down = SBUseTemp->Down ? FALSE : TRUE;
			SBUseTempClick(NULL);
			return;
		}
		else {
			m_DisEvent++;
			CBSTemp->Checked = CBSTemp->Checked ? FALSE : TRUE;
			m_DisEvent--;
		}
	}
	else if( Sender == KSTD ){
		m_DisEvent++;
		CBSDraft->Checked = CBSDraft->Checked ? FALSE : TRUE;
		m_DisEvent--;
	}
	if( CBSTemp->Checked ){
		m_PSPage = m_TSPage[0] = UDStock->Position;
	}
	UpdateTabS();
	if( TabS->TabIndex ){
		CBSTemp->Enabled = FALSE;
		CBSDraft->Enabled = TRUE;
	}
	else {
		CBSTemp->Enabled = TRUE;
		CBSDraft->Enabled = CBSTemp->Checked;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBHViewClick(TObject *Sender)
{
	TZoomViewDlg *pBox = new TZoomViewDlg(this);
	pBox->Execute(pBitmapHist, FALSE);
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBRViewClick(TObject *Sender)
{
	TZoomViewDlg *pBox = new TZoomViewDlg(this);
	pBox->Execute(pBitmapRX, TRUE);
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBTViewClick(TObject *Sender)
{
	TZoomViewDlg *pBox = new TZoomViewDlg(this);
	pBox->Execute(pBitmapTX, FALSE);
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBLKClick(TObject *Sender)
{
	sys.m_AutoStop = !SBLK->Down;
	if( SBLK->Down ){
		sys.m_AutoSync = 0;
	}
	else {
		sys.m_AutoSync = TRUE;
	}
	pDem->m_SyncRestart = !SBLK->Down;
	pDem->CalcBPF();
	if( pCtrBtn != NULL ) pCtrBtn->UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AutoCopyJob(SYSTEMTIME tim, Graphics::TBitmap *pSource)
{
	if( Log.m_LogSet.m_TimeZone == 'I' ){
		UTCtoJST(&tim);
	}
	char fname[256];
	char abf[2];
	abf[0] = abf[1] = 0;
	if( LastC(sys.m_AutoSaveDir.c_str()) != '\\' ) abf[0] = '\\';
	sprintf(fname, "%s%s%04u%02u%02u%02u%02u", sys.m_AutoSaveDir.c_str(), abf,
		tim.wYear,
		tim.wMonth,
		tim.wDay,
		tim.wHour,
		tim.wMinute
	);
	if( KHACJ->Checked ){
		strcat(fname, ".jpg");
	}
	else {
		strcat(fname, ".bmp");
	}
	int r;
	if( KHACT->Checked ){
		Graphics::TBitmap *pBitmap = DupeBitmap(pSource, pf24bit);
		DateBitmap(pBitmap, tim);
		if( KHACJ->Checked ){
			r = SaveJPEG(pBitmap, fname);
		}
		else {
			r = SaveBitmap24(pBitmap, fname);
		}
		delete pBitmap;
	}
	else {
		if( KHACJ->Checked ){
			r = SaveJPEG(pSource, fname);
		}
		else {
			r = SaveBitmap24(pSource, fname);
		}
	}
	if( r ){
		char bf[256];
		sprintf(bf, "%02u%02u%02u%02u%02u",
			tim.wYear % 100,
			tim.wMonth,
			tim.wDay,
			tim.wHour,
			tim.wMinute
		);
		AnsiString as = EditQSL->Text;
		if( SBQSO->Down ){
			if( !strstr(as.c_str(), bf) ){
				if( !as.IsEmpty() ) as += ",";
				as += bf;
			}
		}
		else {
			as = bf;
		}
		EditQSL->Text = as;
		StrCopy(Log.m_sd.qsl, AnsiString(EditQSL->Text).c_str(), MLQSL);	//ja7ude 0521
	}
}
//---------------------------------------------------------------------------
// ヒストリに書きこみ
void __fastcall TMmsstv::SBWHistClick(TObject *Sender)
{
	m_DisEvent++;
	RxHist.Add(pBitmapRX, &m_StartTime, m_HistM);
	if( pHistView != NULL ) pHistView->Add(pBitmapRX);
	UDHist->Position = 0;
	AdjustBitmapFormat(pBitmapHist);
	::CopyBitmap(pBitmapHist, pBitmapRX);
	TrigHistF();
	UpdatePic(1);
	DispHistStat();
	SBWHist->Enabled = FALSE;
	if( KHAC->Checked ){
		AutoCopyJob(m_StartTime, pBitmapHist);
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxTXMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbLeft ){
		PBoxTX->BeginDrag(TRUE,0);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHlpMainClick(TObject *Sender)
{
	if( Sender == KHlpMain ){
		ShowHelp(this, MsgEng ? "EMMSSTV.TXT":"MMSSTV.TXT");
	}
	else {
		ShowHelp(-1);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHlpUpClick(TObject *Sender)
{
	ShowHelp(this, MsgEng ? "EUPDATE.TXT":"UPDATE.TXT");
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHlpPadClick(TObject *Sender)
{
	sys.m_HelpNotePad = InvMenu(KHlpPad);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DropFile(tagMSG &Msg)
{
	HDROP hDrop = (HDROP)Msg.wParam;
	POINT N;
	char fname[256];
	if( Msg.hwnd == PanelS->Handle ){
		if( ::DragQueryPoint(hDrop, &N) ){
			m_DragNo = GetStockNo(N.x, N.y);
			if( m_DragNo >= 0 ){
				m_DragDataNo = UDStock->Position * m_SMax + m_DragNo;
				if( ::DragQueryFile(hDrop, 0, fname, sizeof(fname)) ){
					::SetForegroundWindow(Handle);
					LPCSTR pExt = GetEXT(fname);
					if( IsPic(pExt) ){
						int r;
						TPicRectDlg *pBox = new TPicRectDlg(this);
						r = ::LoadImage(pBox->pBitmap, fname);
						if( r == TRUE ){
							Graphics::TBitmap *pBitmap = CreateBitmap(m_PSSize[m_DragNo] & 0x0000ffff, m_PSSize[m_DragNo]>>16, -1);
//                            Graphics::TBitmap *pBitmap = LoadBitmapS(m_DragNo);
							if( pBox->Copy(pBitmap) == TRUE ){
								SaveBitmapS(pBitmap, m_DragNo);
								UpdateTabS();
							}
							delete pBitmap;
						}
						delete pBox;
					}
					else {
						ErrorMB("'%s' is not supported.", fname);
					}
				}
			}
		}
		::DragFinish(hDrop);
	}
	else if( Msg.hwnd == PanelTX->Handle ){     // 送信画面へのドロップ
		if( ::DragQueryFile(hDrop, 0, fname, sizeof(fname)) ){
			::SetForegroundWindow(Handle);
			LPCSTR pExt = GetEXT(fname);
			if( IsPic(pExt) ){
				int r;
				TPicRectDlg *pBox = new TPicRectDlg(this);
				r = ::LoadImage(pBox->pBitmap, fname);
				if( r == TRUE ){
					if( pBox->Copy(pBitmapTXM) == TRUE ){
						MakeTxBitmap(0);
						PBoxTXPaint(NULL);
					}
				}
				delete pBox;
			}
			else {
				AdjustPage(pgTemp);
				goto _tdrop;
//                ErrorMB("'%s' is not supported.", fname);
			}
		}
		::DragFinish(hDrop);
	}
	else if( Msg.hwnd == PanelTemp->Handle ){       // テンプレートへのドロップ
_tdrop:;
		if( ::DragQueryPoint(hDrop, &N) ){
			if( ::DragQueryFile(hDrop, 0, fname, sizeof(fname)) ){
				::SetForegroundWindow(Handle);
				LPCSTR pExt = GetEXT(fname);
				int b;
				if( ((b = !strcmpi(pExt, "BMP")) != 0) || IsPic(pExt) ){
					Graphics::TBitmap *pBitmap = new Graphics::TBitmap;
					if( ::LoadImage(pBitmap, fname) == TRUE ){
						DropPic(N.x, N.y, pBitmap, b ? 2 : 1);
					}
					delete pBitmap;
				}
				else if( !strcmpi(pExt, "MTM") ){
					SaveUndo();
					LoadTemplate(&DrawMain, fname, NULL);
					UpdateTemp();
				}
				else if( !strcmpi(pExt, "MTI") ){
					LoadTemplate(&DrawTemp, fname, NULL);
					AddTemplate(&DrawTemp);
					UpdateTemp();
				}
				else if( !strcmpi(pExt, "DLL") ){
					DropLib(N.x, N.y, fname);
				}
				else {
					DropOle(N.x, N.y, fname, 1);
				}
			}
		}
		::DragFinish(hDrop);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LoadDefTemp(void)
{
	char fname[256];
	int i;
	for( i = 0; i < 5; i++ ){
		sprintf(fname, "%sdef%d.mtm", BgnDir, i+1);
		if( LoadTemplate(&DrawTemp, fname, NULL) != TRUE ) break;
		SaveStockTemp(&DrawTemp, i);
		if( i == 0 ) DrawMain.Copy(&DrawTemp);
	}
	DrawTemp.FreeItem();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFL1Click(TObject *Sender)
{
	sys.m_FFTGain = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFM1Click(TObject *Sender)
{
	sys.m_FFTGain = 1;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFH1Click(TObject *Sender)
{
	sys.m_FFTGain = 2;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFT1Click(TObject *Sender)
{
	sys.m_FFTGain = 3;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFL2Click(TObject *Sender)
{
	sys.m_FFTGain = 4;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFM2Click(TObject *Sender)
{
	sys.m_FFTGain = 5;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFH2Click(TObject *Sender)
{
	sys.m_FFTGain = 6;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFT2Click(TObject *Sender)
{
	sys.m_FFTGain = 7;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRSClick(TObject *Sender)
{
	sys.m_FFTResp = 2;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRMClick(TObject *Sender)
{
	sys.m_FFTResp = 1;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRFClick(TObject *Sender)
{
	sys.m_FFTResp = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KViewClick(TObject *Sender)
{
	switch(sys.m_FFTType){
		case 0:
			KSOFF->Checked = TRUE;
			break;
		case 1:
			KSFFT->Checked = TRUE;
			break;
		default:
			KSFQ->Checked = TRUE;
			break;
	}
	switch(sys.m_FFTWidth){
		case 0:
			KFW30->Checked = TRUE;
			break;
		case 1:
			KFW20->Checked = TRUE;
			break;
		default:
			KFW15->Checked = TRUE;
			break;
	}
	switch(sys.m_FFTResp){
		case 2:
			KRS->Checked = TRUE;
			break;
		case 1:
			KRM->Checked = TRUE;
			break;
		case 0:
			KRF->Checked = TRUE;
			break;
	}
	switch(sys.m_FFTGain){
		case 0:
			KFL1->Checked = TRUE;
			break;
		case 1:
			KFM1->Checked = TRUE;
			break;
		case 2:
			KFH1->Checked = TRUE;
			break;
		case 3:
			KFT1->Checked = TRUE;
			break;
		case 4:
			KFL2->Checked = TRUE;
			break;
		case 5:
			KFM2->Checked = TRUE;
			break;
		case 6:
			KFH2->Checked = TRUE;
			break;
		case 7:
			KFT2->Checked = TRUE;
			break;
	}
	KFTA->Checked = sys.m_FFTAGC;
	switch(sys.m_FFTStg){
		case 0:
			KFSOF->Checked = TRUE;
			break;
		case 1:
			KFSQ->Checked = TRUE;
			break;
		case 2:
			KFSS->Checked = TRUE;
			break;
		case 3:
			KFSL->Checked = TRUE;
			break;
	}
	switch(sys.m_FFTPriority){
		case 0:
			KFTD1->Checked = TRUE;
			break;
		case 1:
			KFTD2->Checked = TRUE;
			break;
		case 2:
			KFTD3->Checked = TRUE;
			break;
		case 3:
			KFTD4->Checked = TRUE;
			break;
		default:
			KFTD5->Checked = TRUE;
			break;
	}
	if(pDem->m_LevelType){
		KVLSY->Checked = TRUE;
	}
	else {
		KVLSG->Checked = TRUE;
	}
	KVR->Checked = pRxView != NULL ? TRUE : FALSE;
	KVS->Checked = pSyncView != NULL ? TRUE : FALSE;
	KVH->Checked = pHistView != NULL ? TRUE : FALSE;
	KVC->Checked = pCtrBtn != NULL ? TRUE : FALSE;
	switch(sys.m_DivMode){
		case 1:
			KVSD2->Checked = TRUE;
			break;
		case 2:
			KVSD3->Checked = TRUE;
			break;
		default:
			KVSD1->Checked = TRUE;
			break;
	}
	AdjustFileView();
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::FormKeyDown(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	if( m_MainPage == pgTemp ){
		if( DrawMain.pSel != NULL ){
			TShiftState sf1, sf2;
			sf1 << ssShift;
			sf2 << ssShift;
			sf1 *= Shift;
			int sft = (sf1 == sf2) ? TRUE : FALSE;
			CDrawText *pText = NULL;
			if( sft && (DrawMain.pSel->m_Command == CM_TEXT) ) pText = (CDrawText *)DrawMain.pSel;
			switch(Key){
				case VK_UP:
					AdjustTempView(1);
					if( pText != NULL ){
						pText->pFont->Height++;
						pText->UpdateText();
					}
					else {
						DrawMain.pSel->StartMove(PBoxTemp->Canvas, 0, 0, sft ? HT_B : HT_I);
						DrawMain.pSel->Moving(0, -1);
						DrawMain.pSel->Move(0, -1);
					}
					AdjustTempView(0);
					if( KTAP->Checked ) DrawMain.AdjustTransPoint();
					UpdateTemp();
					Key = 0;
					break;
				case VK_DOWN:
					AdjustTempView(1);
					if( pText != NULL ){
						pText->pFont->Height--;
						pText->UpdateText();
					}
					else {
						DrawMain.pSel->StartMove(PBoxTemp->Canvas, 0, 0, sft ? HT_B : HT_I);
						DrawMain.pSel->Moving(0, 1);
						DrawMain.pSel->Move(0, 1);
					}
					AdjustTempView(0);
					if( KTAP->Checked ) DrawMain.AdjustTransPoint();
					UpdateTemp();
					Key = 0;
					break;
				case VK_LEFT:
					AdjustTempView(1);
					if( pText != NULL ){
						pText->pFont->Height++;
						pText->UpdateText();
					}
					else {
						DrawMain.pSel->StartMove(PBoxTemp->Canvas, 0, 0, sft ? HT_R : HT_I);
						DrawMain.pSel->Moving(-1, 0);
						DrawMain.pSel->Move(-1, 0);
					}
					AdjustTempView(0);
					if( KTAP->Checked ) DrawMain.AdjustTransPoint();
					UpdateTemp();
					Key = 0;
					break;
				case VK_RIGHT:
					AdjustTempView(1);
					if( pText != NULL ){
						pText->pFont->Height--;
						pText->UpdateText();
					}
					else {
						DrawMain.pSel->StartMove(PBoxTemp->Canvas, 0, 0, sft ? HT_R : HT_I);
						DrawMain.pSel->Moving(1, 0);
						DrawMain.pSel->Move(1, 0);
					}
					AdjustTempView(0);
					if( KTAP->Checked ) DrawMain.AdjustTransPoint();
					UpdateTemp();
					Key = 0;
					break;
			}
		}
	}
	else if( m_MainPage == pgTX ){
		TShiftState sa1, sa2;

		sa1 << ssAlt;
		sa2 << ssAlt;
		sa1 *= Shift;
		if( sa1 == sa2  ){      // ALT+Any
			switch(Key){
				case 'T':
					if( SBTX->Enabled ){
						SBTX->Down = SBTX->Down ? 0 : 1;
						SBTXClick(NULL);
						Key = 0;
					}
					break;
				case 'U':
					if( SBTune->Enabled ){
						SBTune->Down = SBTune->Down ? 0 : 1;
						SBTuneClick(NULL);
						Key = 0;
					}
					break;
			}
		}
	}
	if( pCtrBtn != NULL ){
		if( Key == VK_F9 ){
			pCtrBtn->SBTXClick(NULL);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::FormKeyUp(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	TShiftState sa1, sa2;

	sa1 << ssAlt;
	sa2 << ssAlt;
	sa1 *= Shift;
	if( sa1 == sa2  ){      // ALT+Any
		switch(Key){
			case 'C':
				if( HisCall->CanFocus() ) HisCall->SetFocus();
				Key = 0;
				break;
			case 'N':
				if( HisName->CanFocus() ) HisName->SetFocus();
				Key = 0;
				break;
			case 'Q':
				if( HisQTH->CanFocus() ) HisQTH->SetFocus();
				Key = 0;
				break;
			case 'L':
				SBListClick(NULL);
				Key = 0;
				break;
		}
	}
	else if( Key == VK_RETURN ){
		if( ActiveControl == HisCall ){
			if( !HisCall->Text.IsEmpty() ){
				FindCall();
				MultProc();
				SBULogClick(NULL);
			}
			Key = 0;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KVOutClick(TObject *Sender)
{
	HWND hWnd = ::FindWindow("Volume Control", NULL);
	if( hWnd != NULL ){
		::PostMessage(hWnd, WM_CLOSE, 0, 0);
		::Sleep(200);
		MultProc();
	}
	char cmd[128];

    if( WinVista ){
		if( Sender != KVOut ){
			strcpy(cmd, "control.exe mmsys.cpl,,1");
		}
        else {
			strcpy(cmd, "sndvol.exe");
		}
    }
    else {
		strcpy(cmd, "SNDVOL32.EXE");
		if( WinNT && (Sender != KVOut) ) strcat(cmd, " /R");
    }

	WinExec(cmd, SW_SHOW);
	MultProc();
	if( !WinNT && (Sender != KVOut) ){
		CWaitCursor w;
		int i;
		for( i = 0; i < 30; i++ ){
			::Sleep(100);
			hWnd = ::FindWindow("Volume Control", NULL);
			if( hWnd != NULL ) break;
			MultProc();
		}
		if( i < 30 ){
			::SetForegroundWindow(hWnd);
			::Sleep(100);
			const short _tt[]={
				VK_MENU, 'P', 'P'|0x8000, VK_MENU|0x8000,
				'R', 'R'|0x8000, VK_TAB, VK_TAB|0x8000,
				VK_DOWN, VK_DOWN|0x8000, VK_RETURN, VK_RETURN|0x8000,
				0
			};
			KeyEvent(_tt);
		}
	}
}
//---------------------------------------------------------------------------
// 周波数範囲情報の取得
void __fastcall TMmsstv::GetFFTRect(int &FM, int &low)
{
	switch(sys.m_FFTWidth){
		case 0:     // 3K
			FM = 3000 * FFT_SIZE / FFTSamp;
			low = 0;
			break;
		case 1:     // 2K
			FM = 2000 * FFT_SIZE / FFTSamp;
			low = 700;
			break;
		case 2:     // 1.5K
			FM = 1500 * FFT_SIZE / FFTSamp;
			low = 1000;
			break;
	}
}
//---------------------------------------------------------------------------
// 周波数ゲージの表示
void __fastcall TMmsstv::PBoxGPaint(TObject *Sender)
{
	if( pSound == NULL ) return;

	TCanvas *tp = PBoxG->Canvas;
	if( Width < 760 ){
		tp->Font->Size = 7;
	}
	else {
		tp->Font->Size = 8;
	}
	tp->Font->Color = clBlack;

	int FM;
	int low;
	GetFFTRect(FM, low);
	int fq = pDem->m_Tick ? pDem->m_TickFreq : 1200;
	char bf[32];
	int x = int(((fq - low)*FFT_SIZE*double(PBoxG->Width)/double(FFTSamp*FM)) + 0.5);
	sprintf(bf, "%u", int(fq + g_dblToneOffset));
	int FH = tp->TextHeight(bf);
	int y = PBoxG->Height - FH;
	int FW = tp->TextWidth(bf);
	x -= FW/2;
	tp->TextOut(x, y, bf);
	if( pDem->m_Tick ) return;

	x = m_FX[1];
	sprintf(bf, "%u", int(1500 + g_dblToneOffset));
	FW = tp->TextWidth(bf);
	x -= FW/2;
	tp->TextOut(x, y, bf);
	x = m_FX[0];
	sprintf(bf, "%u", int(1900 + g_dblToneOffset));
	x -= FW/2;
	tp->TextOut(x, y, bf);
	x = m_FX[2];
	sprintf(bf, "%u", int(2300 + g_dblToneOffset));
	x -= FW/2;
	tp->TextOut(x, y, bf);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFSOFClick(TObject *Sender)
{
	sys.m_FFTStg = 0;
	InitFFT();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFSQClick(TObject *Sender)
{
	sys.m_FFTStg = 1;
	InitFFT();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFSSClick(TObject *Sender)
{
	sys.m_FFTStg = 2;
	InitFFT();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFSLClick(TObject *Sender)
{
	sys.m_FFTStg = 3;
	InitFFT();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UDSampClick(TObject *Sender, TUDBtnType Button)
{
	if( Button == btNext ){
		SSTVSET.m_SampFreq += 0.02;
	}
	else {
		SSTVSET.m_SampFreq -= 0.02;
	}
	SSTVSET.m_SampFreq = NormalSampFreq(SSTVSET.m_SampFreq, 50);
	if( (pDem->m_StgBuf != NULL) || WaveStg.IsOpen() ){
		m_ReqSampChg = 5;
	}
	else {          // No buffer
		UpdateSampFreq();
	}
	DispSyncStat();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBPhaseClick(TObject *Sender)
{
	if( SBPhase->Down || SBSlant->Down ){
		PBoxD12->Cursor = crCross;
	}
	else {
		PBoxD12->Cursor = crDefault;
	}
}
//---------------------------------------------------------------------------
// フォルダの変更
void __fastcall TMmsstv::KHCDClick(TObject *Sender)
{
	if( MsgEng ){
		SaveDialog->Title = "Move history files folder";
	}
	else {
		SaveDialog->Title = "履歴ファイルのフォルダを変更";
	}
	SaveDialog->Filter = "";
	SaveDialog->FileName = "History.bin";
	SaveDialog->DefaultExt = "bin";
	SaveDialog->InitialDir = HistDir;
	SetDisPaint();
	NormalWindow(this);
	if( SaveDialog->Execute() == TRUE ){
		char bf[256];
		SetDirName(bf, AnsiString(SaveDialog->FileName).c_str());	//ja7ude 0521
		if( strcmp(bf, HistDir) ){
			if( YesNoMB( "%s >>> %s, are you sure?", HistDir, bf ) == IDYES ){
				RxHist.MoveDir(bf);
			}
		}
	}
	TopWindow(this);
	ResDisPaint();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSCDClick(TObject *Sender)
{
	if( MsgEng ){
		SaveDialog->Title = "Move stock files folder";
	}
	else {
		SaveDialog->Title = "ストックファイルのフォルダを変更";
	}
	SaveDialog->Filter = "";
	SaveDialog->FileName = TEMPV106C;
	SaveDialog->DefaultExt = "mtm";
	SaveDialog->InitialDir = StockDir;
	SetDisPaint();
	NormalWindow(this);
	if( SaveDialog->Execute() == TRUE ){
		char bf[256];
		SetDirName(bf, AnsiString(SaveDialog->FileName).c_str());
		if( strcmp(bf, StockDir) ){
			if( YesNoMB( "%s >>> %s, are you sure?", StockDir, bf ) == IDYES ){
				MoveStockDir(bf);
			}
		}
	}
	TopWindow(this);
	ResDisPaint();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::MoveStockDir(LPCSTR pNewDir)
{
	if( !strcmp(pNewDir, StockDir) ) return;

	CWaitCursor wait;
	char tname[256];
	char sname[256];

	sprintf(sname, "%s"TEMPV106C, StockDir);
	sprintf(tname, "%s"TEMPV106C, pNewDir);
	if( CopyFile(sname, tname, FALSE) == FALSE ){
		ErrorMB( "Copy fail..." );
		return;
	}
	int i;
	for( i = 0; i < (STOCKMAX*STOCKPAGE); i++ ){
		sprintf(sname, "%st%d.mtm", StockDir, i + 1);
		sprintf(tname, "%st%d.mtm", pNewDir, i + 1);
		CopyFile(sname, tname, FALSE);
		unlink(sname);

		sprintf(sname, sys.m_UseJPEG ? "%sTxStock%u.jpg" : "%sTxStock%u.bmp", StockDir, i + 1);
		sprintf(tname, sys.m_UseJPEG ? "%sTxStock%u.jpg" : "%sTxStock%u.bmp", pNewDir, i + 1);
		CopyFile(sname, tname, FALSE);
		unlink(sname);
	}
	sprintf(sname, "%sCurrent.bmp", StockDir);
	unlink(sname);
	sprintf(sname, "%s"TEMPV106C, StockDir);
	unlink(sname);
	sprintf(sname, "%s"TEMPV106L, StockDir);
	unlink(sname);

	if( *lastp(StockDir) == '\\' ) *lastp(StockDir) = 0;
	_rmdir(StockDir);
	strcpy(StockDir, pNewDir);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::ChangeStockFormat(int sw)
{
	if( sw == sys.m_UseJPEG ) return;

	CWaitCursor wait;
	Graphics::TBitmap *pBitmap = new Graphics::TBitmap();
	AdjustBitmapFormat(pBitmap);

	char tname[256];
	char sname[256];
	int i;
	for( i = 0; i < STOCKMAX; i++ ){
		sprintf(sname, sw ? "%sTxStock%u.bmp" : "%sTxStock%u.jpg", StockDir, i + 1);
		sprintf(tname, sw ? "%sTxStock%u.jpg" : "%sTxStock%u.bmp", StockDir, i + 1);
		if( IsFile(sname) ){
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
	sys.m_UseJPEG = sw;
}

void __fastcall TMmsstv::SBTuneMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		AnsiString as = pMod->m_TuneFreq;
		if( InputMB("MMSSTV", MsgEng?"Enter tone frequency (Hz)":"トーン周波数(Hz)を入力", as) == TRUE ){
			int d;
			sscanf(as.c_str(), "%u", &d);
			if( (d >= 100) && (d <= 3000) ){
				pMod->m_TuneFreq = d;
				UpdateToneFreq();
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KEditClick(TObject *Sender)
{
	BOOL f = ::IsClipboardFormatAvailable(CF_BITMAP);
	SBPaste->Enabled = f;
	KEP->Enabled = f;
	KES->Enabled = f;
	KEX->Enabled = f;
	switch(sys.m_Way240){
		case 0:
			KEW1->Checked = TRUE;
			break;
		case 1:
			KEW2->Checked = TRUE;
			break;
		case 2:
			KEW3->Checked = TRUE;
			break;
		default:
			KEW4->Checked = TRUE;
			break;
	}
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KERClick(TObject *Sender)
{
	CopyBitmap(pBitmapRX);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KESClick(TObject *Sender)
{
//
	TClipboard *pCB = Clipboard();
	if (pCB->HasFormat(CF_BITMAP)){
		Graphics::TBitmap *pBitmap = new Graphics::TBitmap();
		AdjustBitmapFormat(pBitmap);
		try
		{
			pBitmap->LoadFromClipboardFormat(CF_BITMAP, pCB->GetAsHandle(CF_BITMAP), 0);

			MultProc();
			pBitmapTXM = RemakeBitmap(pBitmapTXM, -1);
			MultProc();
			CopyAutoSize(pBitmapTXM, pBitmap);
			MultProc();
		}
		catch (...)
		{
			ErrorMB( "Clipboard error." );
		}
		delete pBitmap;
		AdjustPage(pgTX);
		MakeTxBitmap(0);
		PBoxTXPaint(NULL);
	}
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHFJClick(TObject *Sender)
{
	if( RxHist.m_UseJPEG ) return;

	int r = IDYES;
	if( RxHist.m_Head.m_Cnt ){
		r = YesNoMB( MsgEng ? "MMSSTV will convert the format in all the files which were stored already.\r\n\rThis processing may take a long time, are you sure?" : "既存のファイルのフォーマットを変更します.\r\n\r\nこの処理は時間がかかる可能性があります. ほんまに変換しますか？" );
	}
	if( r == IDYES ){
		RxHist.ChangeFormat(1);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHFBClick(TObject *Sender)
{
	if( !RxHist.m_UseJPEG ) return;

	int r = IDYES;
	if( RxHist.m_Head.m_Cnt ){
		r = YesNoMB( MsgEng ? "MMSSTV will convert the format in all the files which were stored already.\r\n\rThis processing may take a long time, are you sure?" : "既存のファイルのフォーマットを変更します.\r\n\r\nこの処理は時間がかかる可能性があります. ほんまに変換しますか？" );
	}
	if( r == IDYES ){
		RxHist.ChangeFormat(0);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSFJClick(TObject *Sender)
{
	if( sys.m_UseJPEG ) return;

	int r = YesNoMB( MsgEng ? "MMSSTV will convert the format in all the files which were stored already.\r\n\rThis processing may take a long time, are you sure?" : "既存のファイルのフォーマットを変更します.\r\n\r\nこの処理は時間がかかる可能性があります. ほんまに変換しますか？" );
	if( r == IDYES ){
		ChangeStockFormat(1);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSFBClick(TObject *Sender)
{
	if( !sys.m_UseJPEG ) return;

	int r = YesNoMB( MsgEng ? "MMSSTV will convert the format in all the files which were stored already.\r\n\rThis processing may take a long time, are you sure?" : "既存のファイルのフォーマットを変更します.\r\n\r\nこの処理は時間がかかる可能性があります. ほんまに変換しますか？" );
	if( r == IDYES ){
		ChangeStockFormat(0);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRASClick(TObject *Sender)
{
	sys.m_AutoStop = sys.m_AutoStop ? 0 : 1;
    UpdateUI();
	if( pCtrBtn != NULL ) pCtrBtn->UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRASYClick(TObject *Sender)
{
	sys.m_AutoSync = sys.m_AutoSync ? 0 : 1;
    UpdateUI();
	if( pCtrBtn != NULL ) pCtrBtn->UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRARClick(TObject *Sender)
{
	pDem->m_SyncRestart = pDem->m_SyncRestart ? 0 : 1;
	pDem->CalcBPF();
    UpdateUI();
	if( pCtrBtn != NULL ) pCtrBtn->UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PopupRPopup(TObject *Sender)
{
	CSSTVDEM *dp = pDem;
	int f1 = (pRxView != NULL) && (PopupR->PopupComponent == pRxView->PBox);
	int f2 = (pSyncView != NULL) && (PopupR->PopupComponent == pSyncView->PBox);
	int f3 = (PopupR->PopupComponent == PBoxD12 );
	KRFS->Enabled = dp->m_Sync && (SSTVSET.m_Mode != smAVT);
	KRAS->Checked = sys.m_AutoStop;
	KRAR->Checked = dp->m_SyncRestart;
	KRH->Enabled = SBWHist->Enabled;
	KRSA->Enabled = sys.m_UseRxBuff ? TRUE : FALSE;
	KRASY->Checked = sys.m_AutoSync;
	KRCS->Enabled = ( ((dp->m_StgBuf != NULL) || WaveStg.IsOpen()) && (dp->m_wStgLine >= 16) && (SSTVSET.m_Mode != smAVT) ) ? TRUE : FALSE;
	KRCR->Enabled = KRCS->Enabled;
	KRSR->Enabled = (sys.m_SampFreq != SSTVSET.m_SampFreq);

	if( f1 ){       // 常時受信画面
		if( (pRxView->ClientWidth != pBitmapRX->Width) ||
			(pRxView->ClientHeight != pBitmapRX->Height)
		){
			KRR->Enabled = TRUE;
		}
		else {
			KRR->Enabled = FALSE;
		}
		if( (pRxView->ClientWidth != 320) ||
			(pRxView->ClientHeight != 256)
		){
			KRRC->Enabled = TRUE;
		}
		else {
			KRRC->Enabled = FALSE;
		}
		KRC->Enabled = TRUE;
		KRSW->Enabled = TRUE;
		KRSD->Enabled = FALSE;
		KRB->Enabled = TRUE;
		KRCD->Enabled = TRUE;
	}
	else if( f2 ){  // 常時Sync画面
		if( (pSyncView->ClientWidth != pBitmapD12->Width) ||
			(pSyncView->ClientHeight != pBitmapD12->Height)
		){
			KRRC->Enabled = TRUE;
		}
		else {
			KRRC->Enabled = FALSE;
		}
		KRR->Enabled = FALSE;
		KRC->Enabled = FALSE;
		KRH->Enabled = FALSE;
		KRSW->Enabled = FALSE;
		KRSD->Enabled = FALSE;
		KRB->Enabled = FALSE;
		KRCD->Enabled = FALSE;
	}
	else if( f3 ){  // 同期画面
		KRR->Enabled = FALSE;
		KRRC->Enabled = FALSE;
		KRC->Enabled = FALSE;
		KRH->Enabled = FALSE;
		KRSW->Enabled = FALSE;
		KRSD->Enabled = FALSE;
		KRB->Enabled = FALSE;
		KRCD->Enabled = FALSE;
	}
	else {          // 受信画面
		KRR->Enabled = FALSE;
		KRRC->Enabled = FALSE;
		KRC->Enabled = TRUE;
		KRSW->Enabled = TRUE;
		KRSD->Enabled = TRUE;
		KRB->Enabled = TRUE;
		KRCD->Enabled = FALSE;
	}
	KRSD->Checked = sys.m_ShowSizeRX;
	switch(m_SyncAccuracy){
		case 0:
			KRI1->Checked = TRUE;
			break;
		case 1:
			KRI2->Checked = TRUE;
			break;
		default:
			KRI3->Checked = TRUE;
			break;
	}
	KRI->Enabled = KRSA->Enabled;
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRRClick(TObject *Sender)
{
	if( pRxView != NULL ){
		if( PopupR->PopupComponent == pRxView->PBox ){
			pRxView->ClientWidth = pBitmapRX->Width;
			pRxView->ClientHeight = pBitmapRX->Height;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRRCClick(TObject *Sender)
{
	if( pRxView != NULL ){
		if( PopupR->PopupComponent == pRxView->PBox ){
			pRxView->ClientWidth = 320;
			pRxView->ClientHeight = 256;
		}
	}
	if( pSyncView != NULL ){
		if( PopupR->PopupComponent == pSyncView->PBox ){
			pSyncView->ClientWidth = pBitmapD12->Width;
			pSyncView->ClientHeight = pBitmapD12->Height;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KVRClick(TObject *Sender)
{
	if( pRxView == NULL ){
		if( KRCD->Checked ){
			pRxView = new TRxViewDlg(NULL);
			pRxView->ParentWindow = GetDesktopWindow();
		}
		else {
			pRxView = new TRxViewDlg(this);
		}
		pRxView->PBox->PopupMenu = PopupR;
		pRxView->SetViewPos(sys.m_RxViewLeft, sys.m_RxViewTop, sys.m_RxViewWidth, sys.m_RxViewHeight, pBitmapRX);
		if( (pSound != NULL) && pDem->m_Sync ){
			pRxView->UpdateTitle(SSTVSET.m_Mode, 0);
		}
		else {
			pRxView->UpdateTitle(-1, 0);
		}
		pRxView->Visible = TRUE;
		m_TimerRXV = 0;
	}
	else {
		pRxView->GetViewPos(sys.m_RxViewLeft, sys.m_RxViewTop, sys.m_RxViewWidth, sys.m_RxViewHeight);
		delete pRxView;
		pRxView = NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KVSClick(TObject *Sender)
{
	if( pSyncView == NULL ){
		pSyncView = new TRxViewDlg(this);
		pSyncView->UpdateSyncTitle(SSTVSET.m_SampFreq);
		pSyncView->PBox->PopupMenu = PopupR;
		pSyncView->SetViewPos(sys.m_SyncViewLeft, sys.m_SyncViewTop, sys.m_SyncViewWidth, sys.m_SyncViewHeight, pBitmapD12);
		pSyncView->Visible = TRUE;
		m_TimerRXS = 0;
	}
	else {
		pSyncView->GetViewPos(sys.m_SyncViewLeft, sys.m_SyncViewTop, sys.m_SyncViewWidth, sys.m_SyncViewHeight);
		delete pSyncView;
		pSyncView = NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KVHClick(TObject *Sender)
{
	if( pHistView == NULL ){
		OpenHistView();
	}
	else {
		sys.m_HistViewTB = pHistView->m_TitleBar;
		CloseHistView();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KVCClick(TObject *Sender)
{
	if( pCtrBtn == NULL ){
		pCtrBtn = new TCtrBtnWnd(this);
		pCtrBtn->SetViewPos(sys.m_CtrBtnLeft, sys.m_CtrBtnTop, sys.m_CtrBtnWidth, sys.m_CtrBtnHeight);
		pCtrBtn->UpdateBtn();
		pCtrBtn->Visible = TRUE;
	}
	else {
		pCtrBtn->GetViewPos(sys.m_CtrBtnLeft, sys.m_CtrBtnTop, sys.m_CtrBtnWidth, sys.m_CtrBtnHeight);
		delete pCtrBtn;
		pCtrBtn = NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::ShowFileView(int i, int ttl)
{
	CWaitCursor w;

	MultProc();
	TFileViewDlg *fp = pFileView[i] = new TFileViewDlg(this);
	fp->m_MyIndex = i;
	int MaxPage = sys.m_FileViewMaxPage[i];
	fp->m_MaxPage = MaxPage;
	fp->MakeFileV();
	MultProc();
	fp->m_SSize = sys.m_FileViewSize[i] & 0x00000003;
	fp->KBP->Checked = sys.m_FileViewSize[i] >> 16;
	fp->UpdateSize(0);
	MultProc();
	fp->m_CurPage = sys.m_FileViewCurPage[i];
	fp->SetCurPage();
	for( int j = 0; j < MaxPage; j++ ){
		CFILEV *cp = fp->pFileV[j];
		cp->m_Folder = sys.m_FileViewFolder[i][j].c_str();
		cp->m_Name = sys.m_FileViewName[i][j].c_str();
		cp->m_CurPage = sys.m_FileViewFMPage[i][j];
		cp->m_UseIndex = cp->m_CurPage & 0x80000000 ? 1 : 0;
		cp->m_Type = (cp->m_CurPage >> 16) & 7;
		cp->m_CurPage &= 0x0000ffff;
	}
	if( ttl ){
		fp->m_TitleBar = (sys.m_FileViewFlag[i] == 1) ? 1 : 0;
	}
	fp->SBMode->Down = sys.m_FileViewMode[i];
	fp->UD->Max = SHORT(pFileView[i]->pCurPage->m_CurPage + 1);
	fp->UD->Position = SHORT(pFileView[i]->pCurPage->m_CurPage);
	fp->SetViewPos(sys.m_FileViewLeft[i], sys.m_FileViewTop[i], sys.m_FileViewWidth[i], sys.m_FileViewHeight[i]);
	if( ttl ){
		if( !fp->m_TitleBar ) fp->UpdateTitlebar();
	}
	MultProc();
	pFileView[i] = fp;
	if( ttl && TabS->TabIndex && fp->m_Overlap &&
		((pStockView == NULL) || pStockView->Visible)
	){
		fp->Visible = FALSE;
		fp->m_Suspend = 1;
	}
	else {
		fp->Visible = TRUE;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KVFClick(TObject *Sender)
{
	int i;
	for( i = 0; i < 8; i++ ){
		if( pFileView[i] == NULL ){
			ShowFileView(i, 0);
			break;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHACDClick(TObject *Sender)
{
	AnsiString as;

	as = sys.m_AutoSaveDir;
	if( InputMB("MMSSTV", MsgEng ?
		"Please enter a copy folder name":
		"コピー先のフォルダ名を入力して下さい.", as) == TRUE
	){
		jstrupr(as.c_str());
		sys.m_AutoSaveDir = as;
		PBoxHist->DragMode = sys.m_AutoSaveDir.IsEmpty() ? dmManual : dmAutomatic;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHWebClick(TObject *Sender)
{
	WebRef.ShowHTML("http://mmhamsoft.amateur-radio.ca/");
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHRefClick(TObject *Sender)
{
	WebRef.ShowHTML(MsgEng ? "http://groups.yahoo.com/group/MM-SSTV/" : "http://www.egroups.co.jp/group/mmhamsoft/");
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::EditQSLDragOver(TObject *Sender, TObject *Source,
	  int X, int Y, TDragState State, bool &Accept)
{
	Accept = FALSE;
	if( sys.m_AutoSaveDir.IsEmpty() ) return;

	if( Source == PBoxHist ){
		if( UDHist->Position < RxHist.m_Head.m_Cnt ){
			Accept = TRUE;
		}
	}
	else if( Source == PBoxTX ){
		Accept = TRUE;
	}
	else if( (pHistView != NULL) && (pHistView->IsPBox(Source) >= 0) ){
		if( UDHist->Position < RxHist.m_Head.m_Cnt ){
			Accept = TRUE;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::EditQSLDragDrop(TObject *Sender, TObject *Source,
	  int X, int Y)
{
	if( (Source == PBoxHist) ||
		((pHistView != NULL) && (pHistView->IsPBox(Source) >= 0))
	){
		if( UDHist->Position < RxHist.m_Head.m_Cnt ){
			SYSTEMTIME tim = RxHist.m_File[RxHist.m_CurRead].m_UTC;
			AutoCopyJob(tim, pBitmapHist);
		}
	}
	else if( Source == PBoxTX ){
		SYSTEMTIME tim;
		GetUTC(&tim);
		AutoCopyJob(tim, pBitmapTX);
	}
}

void __fastcall TMmsstv::InitProfile(void)
{
	int i;
	PRODEM *mp = m_DemPro;
	for( i = 0; i <= 8; i++, mp++ ){
		mp->Name = "";
		mp->VCOGain = 1.0;
		mp->loopOrder = 1;
		mp->loopFC = 1500;
		mp->OutOrder = 3;
		mp->OutFC = 900;
		mp->Type = 2;
		mp->crossOutOrder = 3;
		mp->crossOutFC = 900;
		mp->crossType = 0;
		mp->crossSmooz = 2200;
		mp->DemOff = 0;
		mp->DemWhite = 128.0/16384.0;
		mp->DemBlack = 128.0/16384.0;
		mp->Differentiator = 0;
		mp->DiffLevel = 0.8;
		mp->DemCalibration = 0;
		double *dp = mp->Dem17;
		*dp++ = -1.636467e+04;
		*dp++ = -1.428800e+04;
		*dp++ = -1.223433e+04;
		*dp++ = -1.019400e+04;
		*dp++ = -8.179333e+03;
		*dp++ = -6.145000e+03;
		*dp++ = -4.088667e+03;
		*dp++ = -2.063000e+03;
		*dp++ = -1.233333e+01;
		*dp++ = 2.050000e+03;
		*dp++ = 4.105000e+03;
		*dp++ = 6.209000e+03;
		*dp++ = 8.332667e+03;
		*dp++ = 1.045133e+04;
		*dp++ = 1.256500e+04;
		*dp++ = 1.463267e+04;
		*dp = 1.663133e+04;
	}

    mp = m_DemPro;
	mp->Name = MsgEng ? "Hilbert transform" : "ヒルベルト変換";
	mp->Type = 2;

    mp++;
	mp->Name = MsgEng ? "Zero crossing" : "ゼロクロス検波";
	mp->Type = 1;
	mp->crossOutOrder = 3;
	mp->crossOutFC = 900;
	mp->DemCalibration = 1;

    mp++;
	mp->Name = MsgEng ? "Zero crossing with Differentiator" : "ゼロクロス検波+微分処理";
	mp->Type = 1;
	mp->crossOutOrder = 3;
	mp->crossOutFC = 900;
	mp->Differentiator = 1;
	mp->DemCalibration = 1;

	m_DemPro[8].Type = 2;
	m_DemPro[8].DemCalibration = 0;
}

//---------------------------------------------------------------------------
TMenuItem *__fastcall TMmsstv::GetKP(int n)
{
	TMenuItem *tbl[]={
		KP1, KP2, KP3, KP4, KP5, KP6, KP7, KP8,
	};
	return tbl[n];
}
//---------------------------------------------------------------------------
TMenuItem *__fastcall TMmsstv::GetKPA(int n)
{
	TMenuItem *tbl[]={
		KPA1, KPA2, KPA3, KPA4, KPA5, KPA6, KPA7, KPA8,
	};
	return tbl[n];
}
//---------------------------------------------------------------------------
TMenuItem *__fastcall TMmsstv::GetKPD(int n)
{
	TMenuItem *tbl[]={
		KPD1, KPD2, KPD3, KPD4, KPD5, KPD6, KPD7, KPD8,
	};
	return tbl[n];
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateProFileMenu(void)
{
	int i;
	for( i = 0; i < 8; i++ ){
		TMenuItem *tp = GetKP(i);
		TMenuItem *dp = GetKPD(i);
		if( m_DemPro[i].Name.IsEmpty() ){
			char bf[32];
			sprintf(bf, "Profile%d", i + 1);
			tp->Caption = bf;
			tp->Enabled = FALSE;
			dp->Caption = bf;
			dp->Enabled = FALSE;
		}
		else {
			tp->Caption = m_DemPro[i].Name;
			tp->Enabled = TRUE;
			dp->Caption = tp->Caption;
			dp->Enabled = TRUE;
		}
		GetKPA(i)->Caption = tp->Caption;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KPAClick(TObject *Sender)
{
	if( pSound == NULL ) return;
	int i;
	for( i = 0; i < 8; i++ ){
		TMenuItem *tp = GetKPA(i);
		if( Sender == tp ){
			AnsiString as = tp->Caption;
			if( InputMB(MsgEng ? "Assign profile" : "プロファイルの登録", (Font->Charset != SHIFTJIS_CHARSET) ? "Enter name" : "名前を入力", as) == TRUE ){
				m_DemPro[i].Name = as;
				m_DemPro[i].VCOGain = pDem->m_pll.m_vcogain;
				m_DemPro[i].loopOrder = pDem->m_pll.m_loopOrder;
				m_DemPro[i].loopFC = pDem->m_pll.m_loopFC;
				m_DemPro[i].OutOrder = pDem->m_pll.m_outOrder;
				m_DemPro[i].OutFC = pDem->m_pll.m_outFC;
				m_DemPro[i].Type = pDem->m_Type;
				m_DemPro[i].crossOutOrder = pDem->m_fqc.m_outOrder;
				m_DemPro[i].crossOutFC = pDem->m_fqc.m_outFC;

				m_DemPro[i].crossType = pDem->m_fqc.m_Type;
				m_DemPro[i].crossSmooz = pDem->m_fqc.m_SmoozFq;

				m_DemPro[i].DemOff = sys.m_DemOff;
				m_DemPro[i].DemWhite = sys.m_DemWhite;
				m_DemPro[i].DemBlack = sys.m_DemBlack;
				m_DemPro[i].DemCalibration = sys.m_DemCalibration;
				for( int j = 0; j < 17; j++ ){
					m_DemPro[i].Dem17[j] = sys.m_Dem17[j];
				}
				m_DemPro[i].Differentiator = sys.m_Differentiator;
				m_DemPro[i].DiffLevel = sys.m_DiffLevelP;
				UpdateProFileMenu();
			}
			break;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KPDClick(TObject *Sender)
{
	if( pSound == NULL ) return;
	int i;
	for( i = 0; i < 8; i++ ){
		if( Sender == GetKPD(i) ){
			m_DemPro[i].Name = "";
			UpdateProFileMenu();
			break;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SetProFile(int n)
{
	pDem->m_pll.SetVcoGain(m_DemPro[n].VCOGain);
	pDem->m_pll.m_loopOrder = m_DemPro[n].loopOrder;
	pDem->m_pll.m_loopFC = m_DemPro[n].loopFC;
	pDem->m_pll.m_outOrder = m_DemPro[n].OutOrder;
	pDem->m_pll.m_outFC = m_DemPro[n].OutFC;
	pDem->m_pll.MakeLoopLPF();
	pDem->m_pll.MakeOutLPF();
	pDem->m_Type = m_DemPro[n].Type;
	pDem->m_fqc.m_outOrder = m_DemPro[n].crossOutOrder;
	pDem->m_fqc.m_outFC = m_DemPro[n].crossOutFC;
	pDem->m_fqc.m_Type = m_DemPro[n].crossType;
	pDem->m_fqc.m_SmoozFq = m_DemPro[n].crossSmooz;
	pDem->m_fqc.CalcLPF();

	sys.m_DemOff = m_DemPro[n].DemOff;
	sys.m_DemWhite = m_DemPro[n].DemWhite;
	sys.m_DemBlack = m_DemPro[n].DemBlack;
	sys.m_DemCalibration = m_DemPro[n].DemCalibration;
	for( int j = 0; j < 17; j++ ){
		sys.m_Dem17[j] = m_DemPro[n].Dem17[j];
	}
	MakeCalibrationTable();

	sys.m_Differentiator = m_DemPro[n].Differentiator;
	sys.m_DiffLevelP = m_DemPro[n].DiffLevel;
	sys.m_DiffLevelM = sys.m_DiffLevelP / 3.0;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KP1Click(TObject *Sender)
{
	if( pSound == NULL ) return;
	int i;
	for( i = 0; i < 8; i++ ){
		if( Sender == GetKP(i) ){
			SetProFile(i);
			break;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KPDefClick(TObject *Sender)
{
	SetProFile(8);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KPInitClick(TObject *Sender)
{
	InitProfile();
	SetProFile(8);
	UpdateProFileMenu();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KPClick(TObject *Sender)
{
	int i;
	for( i = 0; i < 8; i++ ){
		int f = 0;
		if(GetKP(i)->Enabled ){
			if( m_DemPro[i].VCOGain != pDem->m_pll.m_vcogain ) f++;
			if( m_DemPro[i].loopOrder != pDem->m_pll.m_loopOrder ) f++;
			if( m_DemPro[i].loopFC != pDem->m_pll.m_loopFC) f++;
			if( m_DemPro[i].OutOrder != pDem->m_pll.m_outOrder) f++;
			if( m_DemPro[i].OutFC != pDem->m_pll.m_outFC ) f++;
			if( m_DemPro[i].Type != pDem->m_Type ) f++;
			if( m_DemPro[i].crossOutOrder != pDem->m_fqc.m_outOrder ) f++;
			if( m_DemPro[i].crossOutFC != pDem->m_fqc.m_outFC ) f++;

			if( m_DemPro[i].crossType != pDem->m_fqc.m_Type ) f++;
			if( m_DemPro[i].crossSmooz != pDem->m_fqc.m_SmoozFq ) f++;

			if( m_DemPro[i].DemOff != sys.m_DemOff ) f++;
			if( m_DemPro[i].DemWhite != sys.m_DemWhite ) f++;
			if( m_DemPro[i].DemBlack != sys.m_DemBlack ) f++;
			if( m_DemPro[i].DemCalibration != sys.m_DemCalibration ) f++;

			if( m_DemPro[i].Differentiator != sys.m_Differentiator ) f++;
			if( m_DemPro[i].DiffLevel != sys.m_DiffLevelP ) f++;

			GetKP(i)->Checked = f ? FALSE : TRUE;
		}
		else {
			GetKP(i)->Checked = FALSE;
		}
	}
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::HisNameChange(TObject *Sender)
{
	if( DrawMain.IsMacro("%n")||DrawMain.IsMacro("%J") ){
		TempDelay();
		SBULog->Enabled = TRUE;
		UpdateUI();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::HisQTHChange(TObject *Sender)
{
	if( DrawMain.IsMacro("%q") ){
		TempDelay();
		SBULog->Enabled = TRUE;
		UpdateUI();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::EditNoteChange(TObject *Sender)
{
	if( DrawMain.IsMacro("%o") ){
		TempDelay();
		SBULog->Enabled = TRUE;
		UpdateUI();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBTXFilClick(TObject *Sender)
{
	TPicFilterDlg *pBox = new TPicFilterDlg(this);
	if( pBox->Execute(pBitmapTXM) == TRUE ){
		MakeTxBitmap(0);
		PBoxTXPaint(NULL);
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBHistFilClick(TObject *Sender)
{
	TPicFilterDlg *pBox = new TPicFilterDlg(this);
	if( pBox->Execute(pBitmapHist) == TRUE ){
		PBoxHistPaint(NULL);
		DrawMain.UpdatePic(pBitmapHistF);
		UpdatePic();
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFW30Click(TObject *Sender)
{
	sys.m_FFTWidth = 0;
	InitDrawFFT();
	PBoxG->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFW20Click(TObject *Sender)
{
	sys.m_FFTWidth = 1;
	InitDrawFFT();
	PBoxG->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFW15Click(TObject *Sender)
{
	sys.m_FFTWidth = 2;
	InitDrawFFT();
	PBoxG->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRBClick(TObject *Sender)
{
	InitCustomColor(ColorDialog);
	AddCustomColor(ColorDialog, sys.m_ColorRXB);
	ColorDialog->Color = sys.m_ColorRXB;
	SetDisPaint();
	if( ColorDialog->Execute() == TRUE ){
		sys.m_ColorRXB = ColorDialog->Color;
		PanelRX->Color = sys.m_ColorRXB;
		PanelHist->Color = sys.m_ColorRXB;
	}
	ResDisPaint();
}
#if 0
///----------------------------------------------------------------
///  多項式計算を行う
double Teira(double x, const double *p, int n)
{
	double y = *p++;
	int i;
	for( i = 1; i < n; i++, p++ ){
		y += pow(x, i)*(*p);
	}
	return y;
}
#endif
//---------------------------------------------------------------------------
// 多項式変換テーブルを作成する
void __fastcall TMmsstv::MakeCalibrationTable(void)
{
	if( sys.m_DemCalibration ){
		if( pCalibration == NULL ){
			pCalibration = new short[4097];
		}
		short *wp = pCalibration;
		int i;
//        FILE *fp = fopen("F:\\Color.TXT", "wt");
		for( i = 0; i <= 4096; i++, wp++ ){
			int d = (i - 2048) * 8;
			int f = 0;
			int c = -128;
			int j;
			for( j = 1; j <= 16; j++, c += 16 ){
				if( d < sys.m_Dem17[j] ){
					*wp = c + (d - sys.m_Dem17[j-1]) * (16.0 / (sys.m_Dem17[j] - sys.m_Dem17[j-1]));
					f++;
					break;
				}
			}
			if( !f ){
				*wp = 128;
			}
//            fprintf(fp, "%4u:%d\n", i, *wp);
		}
//        fclose(fp);
	}
	else {
		if( pCalibration != NULL ){
			delete pCalibration;
			pCalibration = NULL;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHDS1Click(TObject *Sender)
{
	sys.m_HDDSize = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHDS2Click(TObject *Sender)
{
	sys.m_HDDSize = 1;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHDS3Click(TObject *Sender)
{
	sys.m_HDDSize = 2;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHDS4Click(TObject *Sender)
{
	sys.m_HDDSize = 3;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHDS5Click(TObject *Sender)
{
	sys.m_HDDSize = 4;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHDS6Click(TObject *Sender)
{
	sys.m_HDDSize = 5;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KVWClick(TObject *Sender)
{
	InvMenu(KVW);
	FormResize(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFTAClick(TObject *Sender)
{
	sys.m_FFTAGC = sys.m_FFTAGC ? 0 : 1;    
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFTD1Click(TObject *Sender)
{
	sys.m_FFTPriority = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFTD2Click(TObject *Sender)
{
	sys.m_FFTPriority = 1;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFTD3Click(TObject *Sender)
{
	sys.m_FFTPriority = 2;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFTD4Click(TObject *Sender)
{
	sys.m_FFTPriority = 3;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KFTD5Click(TObject *Sender)
{
	sys.m_FFTPriority = 4;
}
//---------------------------------------------------------------------------
TMenuItem *__fastcall TMmsstv::GetKX(int n)
{
	TMenuItem *tbl[]={
		KX1, KX2, KX3, KX4, KX5, KX6, KX7, KX8,
	};
	return tbl[n];
}
//---------------------------------------------------------------------------
TMenuItem *__fastcall TMmsstv::GetKXA(int n)
{
	TMenuItem *tbl[]={
		KXA1, KXA2, KXA3, KXA4, KXA5, KXA6, KXA7, KXA8,
	};
	return tbl[n];
}
//---------------------------------------------------------------------------
TMenuItem *__fastcall TMmsstv::GetKXD(int n)
{
	TMenuItem *tbl[]={
		KXD1, KXD2, KXD3, KXD4, KXD5, KXD6, KXD7, KXD8,
	};
	return tbl[n];
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateProgramMenu(void)
{
	int i;
	for( i = 0; i < 8; i++ ){
		TMenuItem *tp = GetKX(i);
		TMenuItem *dp = GetKXD(i);
		if( sys.m_ExtName[i].IsEmpty() ){
			char bf[32];
			sprintf(bf, "Program%d", i + 1);
			tp->Caption = bf;
			dp->Caption = bf;
		}
		else {
			tp->Caption = sys.m_ExtName[i];
			dp->Caption = sys.m_ExtName[i];
		}
		if( sys.m_ExtMode[i] ){
			tp->Enabled = TRUE;
			dp->Enabled = TRUE;
		}
		else {
			tp->Enabled = FALSE;
			dp->Enabled = FALSE;
		}
		GetKXA(i)->Caption = tp->Caption;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::ExecExtCmd(int n)
{
	if( sys.m_ExtCmd[n].IsEmpty() || !IsFile(sys.m_ExtCmd[n].c_str()) ){
		if( !AssignExtCmd(n) ) return;
	}
	if( sys.m_ExtMode[n] == 2 ){
		KXSClick(NULL);
	}
	char bf[256];
	SetDirName(bf, sys.m_ExtCmd[n].c_str());
	::SetCurrentDirectory(bf);
	::WinExec(sys.m_ExtCmd[n].c_str(), SW_SHOWDEFAULT);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KXRClick(TObject *Sender)
{
	CWaitCursor wait;
	pDem->OpenCloseRxBuff();
	pSound->fftIN.TrigFFT();
	pSound->m_suspack = 0;
	pSound->m_susp = 0;
	for( int i = 0; (i < 20) && !pSound->m_suspack; i++ ) ::Sleep(100);
	if( pSound->m_susp ){		// 解除に失敗
		m_SuspMinimized = FALSE;
    	return;
    }
	COMM.change = 1;
	RADIO.change = 1;
	OpenCloseCom();
	::SetCurrentDirectory(BgnDir);
	Log.Close();
	Log.Open(NULL, TRUE);
	RxHist.Open();
	UpdateHist();
	UpdateModeBtn();
	DisPaint = FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KXSClick(TObject *Sender)
{
	CWaitCursor wait;
	if( SBTX->Down ){
		ToRX();
	}
	if( pDem->m_Sync ){
		pDem->Stop();
	}
	pSound->m_suspack = 0;
	pSound->m_susp = 1;
	if( pComm != NULL ){
		delete pComm;
		pComm = NULL;
	}
	if( pRadio != NULL ){
		delete pRadio;
		pRadio = NULL;
	}
	Log.Close();
	Log.Open(NULL, TRUE);
	RxHist.Close();
	int i;
	for( i = 0; (i < 20) && !pSound->m_suspack; i++ ) ::Sleep(100);
	pSound->m_suspack = 0;
	pDem->FreeRxBuff();
	DisPaint = TRUE;
	m_SuspMinimized = (Sender != KXS);
	if( m_SuspMinimized ) Application->Minimize();
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::AssignExtCmd(int n)
{
	int r = FALSE;
	int Susp = sys.m_ExtMode[n] == 2 ? 1 : 0;
	TExtCmdDlg *pBox = new TExtCmdDlg(this);
	if( pBox->Execute(sys.m_ExtName[n], sys.m_ExtCmd[n], Susp) == TRUE ){
		sys.m_ExtMode[n] = Susp + 1;
		r = TRUE;
	}
	delete pBox;
	return r;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KX1Click(TObject *Sender)
{
	int i;
	for( i = 0; i < 8; i++ ){
		if( Sender == GetKX(i) ){
			ExecExtCmd(i);
			break;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KXA1Click(TObject *Sender)
{
	int i;
	for( i = 0; i < 8; i++ ){
		if( Sender == GetKXA(i) ){
			AssignExtCmd(i);
			break;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KXD1Click(TObject *Sender)
{
	int i;
	for( i = 0; i < 8; i++ ){
		if( Sender == GetKXD(i) ){
			sys.m_ExtName[i] = "";
			sys.m_ExtMode[i] = 0;
			break;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KXClick(TObject *Sender)
{
	KXR->Enabled = pSound->m_susp ? TRUE : FALSE;
	KXS->Enabled = pSound->m_susp ? FALSE : TRUE;
	KXSM->Enabled = KXS->Enabled;
	UpdateProgramMenu();
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTAPClick(TObject *Sender)
{
	InvMenu(KTAP);
	if( KTAP->Checked ){
		DrawMain.AdjustTransPoint();
		UpdateTemp();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTACClick(TObject *Sender)
{
	SaveUndo();
	DrawMain.AdjustTransColor(pBitmapTemp);
	DrawMain.UpdatePic(pBitmapHistF);
	UpdateTemp();
}
//---------------------------------------------------------------------------

void __fastcall TMmsstv::KVLSGClick(TObject *Sender)
{
	pDem->m_LevelType = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KVLSYClick(TObject *Sender)
{
	pDem->m_LevelType = 1;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHTBClick(TObject *Sender)
{
	if( pHistView != NULL ){
		pHistView->m_TitleBar = pHistView->m_TitleBar ? 0 : 1;
		sys.m_HistViewTB = pHistView->m_TitleBar;
		pHistView->UpdateTitlebar();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OnMove(TMessage *Message)
{
	if( (pHistView != NULL) && !pHistView->m_TitleBar ){
		pHistView->Top += (Top - m_OrgTop);
		pHistView->Left += (Left - m_OrgLeft);
	}
	for( int i = 0; i < 8; i++ ){
		if( pFileView[i] != NULL && !pFileView[i]->m_TitleBar ){
			pFileView[i]->Top += (Top - m_OrgTop);
			pFileView[i]->Left += (Left - m_OrgLeft);
		}
	}
	m_OrgTop = Top;
	m_OrgLeft = Left;
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::OpenDialogExecute(int sw)
{
	MultProc();
	m_PreViewFlag = sw;
	int r = OpenDialog->Execute();
	MultProc();
	return r;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OpenDialogSelectionChange(TObject *Sender)
{
	if( !m_PreViewFlag ) return;

	MultProc();
	char fname[256];
	StrCopy(fname, AnsiString(OpenDialog->FileName).c_str(), 255);	//ja7ude 0521
	LPCSTR pExt = GetEXT(fname);
	int f = 0;
	if( IsPic(pExt) ){
		if( IsFile(fname) ){
			f = 1;
		}
	}
	else if( !strcmpi(pExt, "MTM") || !strcmpi(pExt, "MTI") ){
		if( IsFile(fname) ){
			f = 2;
		}
	}
	MultProc();
	if( f ){
		CWaitCursor w;
		if( pPreView == NULL ){
			pPreView = new TPrevViewBox(NULL);
			pPreView->SetViewPos(sys.m_PreViewLeft, sys.m_PreViewTop, sys.m_PreViewWidth, sys.m_PreViewHeight);
		}
		if( f == 2 ){
			CDrawGroup *pGroup = new CDrawGroup;
			if( LoadTemplate(pGroup, fname, pPreView->pBitmap->Canvas) == TRUE ){
				pPreView->pBitmap->Width = pGroup->m_SX;
				pPreView->pBitmap->Height = pGroup->m_SY;
				FillBitmap(pPreView->pBitmap, pGroup->m_TransCol);
				MultProc();
				pGroup->Draw(pPreView->pBitmap->Canvas);
			}
			pGroup->FreeItem();
			delete pGroup;
		}
		else {
			::LoadImage(pPreView->pBitmap, fname);
		}
		MultProc();
		pPreView->UpdateTitle(fname);
		pPreView->Show();
		pPreView->Invalidate();
		MultProc();
	}
	else {
		OpenDialogClose(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OpenDialogClose(TObject *Sender)
{
	if( pPreView != NULL ){
		pPreView->GetViewPos(sys.m_PreViewLeft, sys.m_PreViewTop, sys.m_PreViewWidth, sys.m_PreViewHeight);
		delete pPreView;
		pPreView = NULL;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMmsstv::KTPAClick(TObject *Sender)
{
	if(DrawMain.pSel->m_Command == CM_PIC){
		CDrawPic *pPic = (CDrawPic *)DrawMain.pSel;
		if( !pPic->m_Type ){
			SBHistFilClick(NULL);
		}
		else {
			TPicFilterDlg *pBox = new TPicFilterDlg(this);
			if( pBox->Execute(pPic->pBitmap) == TRUE ){
				UpdatePic();
				DeleteUndo();
			}
			delete pBox;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PopupMPopup(TObject *Sender)
{
	int *ip = ((m_MainPage == pgTX)||(m_MainPage == pgTemp)) ? m_ModeAssignTX : m_ModeAssignRX;
	int n = 0;
	int m = ip[m_ExtMode];
	int i;
	int w, h;
	char bf[128];
	TMenuItem *pm;
	for( i = 0; i < smEND; i++ ){
		if( n < PopupM->Items->Count ){
			pm = PopupM->Items->Items[n];
		}
		else {
			pm = new TMenuItem (this);
		}
		int mm = SSTVModeOdr[i];
		int dm;
		SSTVSET.GetPictureSize(w, dm, h, mm);
		double tw = SSTVSET.GetTiming(mm);
		if( ((mm >= smPD50) && (mm <= smPD290)) || ((mm >= smMP73) && (mm <= smMP175)) || ((mm >= smMN73) && (mm <= smMN140)) || ((mm >= smR24) && (mm <= smRM12))) tw *= 0.5;
		tw = ((tw * h) / 1000.0) + 0.5;
		if( ((mm >= smR24) && (mm <= smRM12)) ){
			w /= 2;
			h /= 2;
		}
		sprintf(bf, "%s\t(%dx%d %us)", SSTVModeList[mm], w, h, int(tw));
//        pm->Break = mm == smMP73 ? mbBarBreak : mbNone;
		pm->Caption = bf;
		pm->RadioItem = TRUE;
		pm->OnClick = KMClick;
		pm->Checked = (m == mm) ? TRUE : FALSE;
		pm->Enabled = ((m == mm) || (FindModeAssign(ip, mm) < 0)) ? TRUE : FALSE;
		if( n >= PopupM->Items->Count ){
			PopupM->Items->Insert(n, pm);
		}
		n++;
	}
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KMClick(TObject *Sender)
{
	int i;
	for( i = 0; i < PopupM->Items->Count; i++ ){
		if( PopupM->Items->Items[i] == Sender ){
			int mode = SSTVModeOdr[i];
			if( ((m_MainPage == pgTX)||(m_MainPage == pgTemp)) ){
				m_ModeAssignTX[m_ExtMode] = mode;
				if( GetModeBtn(m_ExtMode)->Down ){
					ChangeTxMode(mode);
				}
				if( !pDem->m_Sync || (SSTVSET.m_Mode != m_ModeAssignRX[m_ExtMode]) ){
					m_ModeAssignRX[m_ExtMode] = mode;
				}
			}
			else {
				m_ModeAssignRX[m_ExtMode] = mode;
				if( !SBTX->Down || (SSTVSET.m_TxMode != m_ModeAssignTX[m_ExtMode]) ){
					if( SSTVSET.m_TxMode == m_ModeAssignTX[m_ExtMode] ){
						m_ModeAssignTX[m_ExtMode] = mode;
						ChangeTxMode(mode);
					}
					else {
						m_ModeAssignTX[m_ExtMode] = mode;
					}
				}
			}
			break;
		}
	}
	UpdateModeBtn();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBMMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		TSpeedButton *tp = (TSpeedButton *)Sender;
		m_ExtMode = GetButtonNo(tp);
		if( tp->Down ){
			if( SBTX->Down ) return;
		}
		PopupM->Popup(Left + GBMode->Left + tp->Left + tp->Width/2, Top + GBMode->Top + tp->Top + tp->Height);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::ChangeTxMode(int mode)
{
	SSTVSET.GetPictureSize(m_TXW, m_TXH, m_TXPH, mode);
	UpdateTXUI();
	int rep = (sys.m_Repeater && pDem->m_Repeater) ? 1 : 0;
	if( (pBitmapTXM->Width != m_TXW) || (pBitmapTXM->Height != m_TXH) ){
		if( KSIS->Checked || rep ){
			ReSizeBitmap(pBitmapTXM, m_TXW, m_TXH);
		}
		else {
			pBitmapTXM->Width = m_TXW;
			pBitmapTXM->Height = m_TXH;
		}
		pBitmapTX->Width = m_TXW;
		pBitmapTX->Height = m_TXH;
		if( !KSIS->Checked || !DrawMain.m_Cnt ){
			if( !rep ){
				DrawMain.m_SX = m_TXW;
				DrawMain.m_SY = m_TXH;
			}
		}
		UpdatePic();
		if( sys.m_ShowSizeStock ){
			UpdateTabS();
		}
	}
	else if( sys.m_Repeater && pDem->m_Repeater ){
		UpdatePic();
	}
    if( (SSTVSET.m_TxMode != mode) && KSAM->Checked && !rep ){
		m_TxClipType = (m_TXPH == 240) ? 2 : 0;
        UpdateTxClip();
        UpdatePic();
    }
	SSTVSET.m_TxMode = mode;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSSDClick(TObject *Sender)
{
	if( PopupS->PopupComponent == PBoxTX ){
		sys.m_ShowSizeTX = sys.m_ShowSizeTX ? FALSE : TRUE;
		PBoxTX->Invalidate();
	}
	else {
		sys.m_ShowSizeStock = sys.m_ShowSizeStock ? FALSE : TRUE;
		UpdateTabS();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTSZClick(TObject *Sender)
{
	int w, h;

	if( Sender == KTSZ1 ){
		w = 320; h = 256;
	}
	else if( Sender == KTSZ2 ){
		w = 512; h = 400;
	}
	else if( Sender == KTSZ3 ){
		w = 640; h = 496;
	}
	else if( Sender == KTSZ4 ){
		w = 800; h = 616;
	}
	else {
		w = m_TXW; h = m_TXH;
	}
	SaveUndo();
	DrawMain.m_SX = w;
	DrawMain.m_SY = h;
	UpdatePic();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTSDClick(TObject *Sender)
{
	sys.m_ShowSizeTemp = sys.m_ShowSizeTemp ? FALSE : TRUE;
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHSDClick(TObject *Sender)
{
	sys.m_ShowSizeHist = sys.m_ShowSizeHist ? FALSE : TRUE;
	PBoxHist->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRSDClick(TObject *Sender)
{
	sys.m_ShowSizeRX = sys.m_ShowSizeRX ? FALSE : TRUE;
	PBoxRX->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdateSBTO(void)
{
	SBTO->Enabled = (!SBTX->Down && (pDem->m_LoopBack == 2) && (SSTVSET.m_SampFreq != sys.m_SampFreq)) ? TRUE : FALSE;
	CSSTVDEM *dp = pDem;
    BOOL f = ( ((dp->m_StgBuf != NULL) || WaveStg.IsOpen()) && (dp->m_wStgLine >= 16) && (SSTVSET.m_Mode != smAVT) ) ? TRUE : FALSE;
	SBAdj->Enabled = f;
    SBPL->Enabled = f;
    SBPR->Enabled = f;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBTOClick(TObject *Sender)
{
	double  r = (sys.m_SampFreq * SSTVSET.m_TxSampFreq)/ SSTVSET.m_SampFreq;
	r -= sys.m_SampFreq;
	r = NormalSampFreq(r, 100);
	sys.m_TxSampOff = r;
	pMod->m_vco.SetSampleFreq(sys.m_SampFreq + sys.m_TxSampOff);
	SBTO->Enabled = FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRCSClick(TObject *Sender)
{
	CorrectSlant();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRCRClick(TObject *Sender)
{
	RedrawAdjustSync();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRSRClick(TObject *Sender)
{
	if( sys.m_SampFreq != SSTVSET.m_SampFreq ){
		CWaitCursor w;

		SSTVSET.m_SampFreq = sys.m_SampFreq;
		RedrawSampFreq(FALSE);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBRXIDClick(TObject *Sender)
{
	pDem->m_fskdecode = SBRXID->Down;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSOFFClick(TObject *Sender)
{
	int type = sys.m_FFTType;
	pSound->fftIN.TrigFFT();
	if( Sender == KSOFF ){
		sys.m_FFTType = 0;
	}
	else if( Sender == KSFFT ){
		sys.m_FFTType = 1;
	}
	else {
		sys.m_FFTType = 2;
	}
	if( !sys.m_FFTType ){
		DrawFFT(1);
		PBoxFFT->Invalidate();
		if( KVW->Checked ){
			DrawWater(1);
			PBoxWater->Invalidate();
		}
	}
	else if( type != sys.m_FFTType ){
		switch(sys.m_FFTType){
			case 1:
				if( sys.m_FFTGain >= 4 ) sys.m_FFTGain = 1;
				break;
			case 2:
				if( sys.m_FFTGain < 4 ) sys.m_FFTGain = 5;
				break;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBRXIDMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		if( PopupC->Items->Count ){
			TControl *tp = (TControl *)Sender;
			PopupC->Popup(Left + GBLog->Left + tp->Left + SBRXID->Width/2, m_MW + Top + GBLog->Top + tp->Top);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KCClick(TObject *Sender)
{
	TMenuItem *pm = (TMenuItem *)Sender;
	if( strcmp(AnsiString(HisCall->Text).c_str(), (AnsiString(pm->Caption).c_str()+6)) ){
		HisCall->Text = pm->Caption.c_str() + 6;
		FindCall();
		m_ChangeTemp = 2;
		SBULog->Enabled = TRUE;
		UpdateUI();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AddCall(LPCSTR p)
{
	SYSTEMTIME	now;
	GetLocal(&now);
	char bf[128];
	sprintf(bf, "%02u:%02u\t%s", now.wHour, now.wMinute, p);

	int i;
	TMenuItem *pm;
	for( i = 0; i < PopupC->Items->Count; i++ ){
		pm = PopupC->Items->Items[i];
		if( !strcmp(AnsiString(pm->Caption).c_str() + 6, p) ){
			if( !i ){
				pm->Caption = bf;
				return;
			}
			else {
				PopupC->Items->Delete(i);
				break;
			}
		}
	}
	pm = new TMenuItem (this);
	pm->Caption = bf;
	pm->RadioItem = FALSE;
	pm->OnClick = KCClick;
	pm->Checked = FALSE;
	pm->Enabled = TRUE;
	if( (i >= PopupC->Items->Count) && (PopupC->Items->Count == FSKIDMAX) ){
		PopupC->Items->Delete(FSKIDMAX-1);
	}
	PopupC->Items->Insert(0, pm);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DeleteHistF(void)
{
	if( (pBitmapHistF != NULL) && (pBitmapHistF != pBitmapHist) ){
		delete pBitmapHistF;
	}
	pBitmapHistF = pBitmapHist;
	DrawMain.UpdatePic(pBitmapHistF);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AdjustHistF(void)
{
	m_ReqHistF = 0;
	if( KSF->Checked ){
		if( pBitmapHistF == pBitmapHist ) pBitmapHistF = NULL;
		pBitmapHistF = MFilter(pBitmapHistF, pBitmapHist, 1);
		DrawMain.UpdatePic(pBitmapHistF);
	}
	else {
		DeleteHistF();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::TrigHistF(void)
{
	if( KSF->Checked ){
		DeleteHistF();
		m_ReqHistF = 10;
	}
	else {
		DrawMain.UpdatePic(pBitmapHistF);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSFClick(TObject *Sender)
{
	InvMenu(KSF);
	AdjustHistF();
	UpdatePic(1);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTF1Click(TObject *Sender)
{
	CWaitCursor w;

	SaveUndo();
	char fname[256];
	int n;
	if( Sender == KTF1 ){
		n = 1;
	}
	else if( Sender == KTF2 ){
		n = 2;
	}
	else if( Sender == KTF3 ){
		n = 3;
	}
	else if( Sender == KTF4 ){
		n = 4;
	}
	else {
		n = 5;
	}
	sprintf(fname, "%sdef%d.mtm", BgnDir, n);
	LoadTemplate(&DrawMain, fname, NULL);
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTKAClick(TObject *Sender)
{
	if( DrawMain.pSel != NULL ){
		SaveUndo();
		DrawMain.pSel->KeepAspect();
		UpdateTemp();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTOSClick(TObject *Sender)
{
	if( DrawMain.pSel != NULL ){
		SaveUndo();
		DrawMain.pSel->SetOrgSize();
		UpdateTemp();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UDStockMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		int n = UDStock->Position;
		int w = UDStock->Width/2;
		int m = n % 5;
		if( X >= w ){
			n += (5 - m);
		}
		else if( m ){
			n -= m;
		}
		else {
			n -= 5;
		}
		int max = UDStock->Max + 1;
		if( n < 0 ) n += max;
		if( n >= max ) n -= max;
		n = n - (n % 5);
		UDStock->Position = SHORT(n);
		UDStockClick(NULL, btNext);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRI1Click(TObject *Sender)
{
	if( Sender == KRI1 ){
		m_SyncAccuracy = 0;
	}
	else if( Sender == KRI2 ){
		m_SyncAccuracy = 1;
	}
	else {
		m_SyncAccuracy = 2;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRCDClick(TObject *Sender)
{
	InvMenu(KRCD);
	if( pRxView != NULL ){
		KVRClick(NULL);
		if( !KRCD->Checked ) Application->Restore();
		KVRClick(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KORepClick(TObject *Sender)
{
	TRepSetDlg *pBox = new TRepSetDlg(this);
	int r = pBox->Execute();
	if( r != mrCancel ){
		InitDrawFFT();
		PBoxG->Invalidate();
		UpdateTitle();
		if( (r == 1024) && pDem->m_Repeater ){
			SendBeacon();
		}
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KEWClick(TObject *Sender)
{
	if( Sender == KEW1 ){
		sys.m_Way240 = 0;
	}
	else if( Sender == KEW2 ){
		sys.m_Way240 = 1;
	}
	else if( Sender == KEW3 ){
		sys.m_Way240 = 2;
	}
	else {
		sys.m_Way240 = 3;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AdjustPage(int sw)
{
	switch(sw){
		case pgRX:
			if( m_MainPage != pgRX ){
				m_MainPage = pgRX;
				Page->ActivePage = TabRX;
				PageChange(NULL);
			}
			break;
		case pgHist:
			if( m_MainPage != pgHist ){
				m_MainPage = pgHist;
				Page->ActivePage = TabHist;
				PageChange(NULL);
			}
			break;
		case pgTX:
			if( m_MainPage != pgTX ){
				m_MainPage = pgTX;
				Page->ActivePage = TabTX;
				PageChange(NULL);
			}
			break;
		case pgTemp:
			if( m_MainPage != pgTemp ){
				m_MainPage = pgTemp;
				Page->ActivePage = TabTemp;
				PageChange(NULL);
			}
			break;
		default:
			if( (m_MainPage != pgTX) && (m_MainPage != pgTemp) ){
				m_MainPage = pgTX;
				Page->ActivePage = TabTX;
				PageChange(NULL);
			}
			break;
	}
}
//
//---------------------------------------------------------------------------
void __fastcall TMmsstv::Repeater(void)
{
	CSSTVDEM *dp = pDem;
	switch(dp->m_repmode){
		case 3:
			SetBeaconDelay();
			dp->m_repANS++;
			SendCWID(sys.m_RepAnsCW.c_str());
			if( dp->m_SyncMode < 0 /* !SBAuto->Down*/ ){
				dp->Stop();
				dp->m_SyncMode = 0;
				UpdateModeBtn();
				UpdateUI();
			}
			dp->m_repmode++;
			break;
		case 9:
			SetBeaconTick();
			if( !sys.m_RepFolder.IsEmpty() ){
				SYSTEMTIME tim;
				::GetUTC(&tim);
				if( Log.m_LogSet.m_TimeZone == 'I' ){
					UTCtoJST(&tim);
				}
				char fname[256];
				char abf[2];
				abf[0] = abf[1] = 0;
				if( LastC(sys.m_RepFolder.c_str()) != '\\' ) abf[0] = '\\';
				sprintf(fname, "%s%s%04u%02u%02u%02u%02u.jpg", sys.m_RepFolder.c_str(), abf,
					tim.wYear,
					tim.wMonth,
					tim.wDay,
					tim.wHour,
					tim.wMinute
				);
				SaveJPEG(pBitmapTX, fname);
			}
			AdjustPage(pgTX);
			ToTX();
			dp->m_repTX++;
			dp->m_repmode++;
			m_RepTXPos++;
			break;
	}
	if( sys.m_RepBeacon ){
		DWORD NTick = ::GetTickCount();
		DWORD CTick = m_RepTick + (sys.m_RepBeacon * 1000);
		if( dp->m_RepSQ && (dp->m_repsig > dp->m_RepSQ) ){    // 信号検出中
			if( CTick < (NTick + sys.m_RepQuietnessTime*1000) ){ // 300s前の時は延長
				m_RepTick = NTick - ((sys.m_RepBeacon - sys.m_RepQuietnessTime) * 1000);
			}
		}
		else if( (CTick < NTick) && !dp->m_repmode ){
			m_RepTick = NTick;
			SendBeacon();
		}
	}
}
//---------------------------------------------------------------------------
int __fastcall TMmsstv::LoadRepTemplate(LPCSTR pList, int &Pos)
{
	char bf[256];
	StrCopy(bf, pList, sizeof(bf)-1);
	LPSTR t, p;
	int max, n;
	for( max = 0, p = bf; *p; max++ ){
		p = StrDlm(t, p);
		if( !*SkipSpace(t) ) break;
	}
	if( Pos >= max ) Pos = 0;
	StrCopy(bf, pList, sizeof(bf)-1);
	for( n = 0, p = bf; n < Pos; n++ ){
		p = StrDlm(t, p);
	}
	StrDlm(t, p);
	t = SkipSpace(t);
	clipsp(t);
	int gf = 0;
	if( *t ){
		if( *t == '#' ){
			t++;
			gf = 1;
		}
		n = atoin(t, -1);
		if( (n >= 1) && (n <= STOCKMAX) ){
			if( gf ) gf = n;

			LoadStockTemp(&DrawMain, n-1, NULL);
			if( !DrawMain.m_Cnt ){
				DrawMain.m_SX = m_TXW;
				DrawMain.m_SY = m_TXH;
			}
			if( !SBUseTemp->Down ){
				SBUseTemp->Down = TRUE;
				SBUseTempClick(NULL);
			}
		}
		else {
			gf = 0;
		}
	}
	return gf;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SendBeacon(void)
{
	if( pDem->m_repmode >= 2 ) return;
	if( SBTX->Down ) ToRX();
	ChangeTxMode(sys.m_RepBeaconMode);
	UpdateModeBtn();
	int gf = LoadRepTemplate(sys.m_RepTempBeacon.c_str(), m_RepBeaconPos);
	if( gf ){       // 画像のロード
		Graphics::TBitmap *pBitmap = new Graphics::TBitmap();
		LoadBitmapSN(pBitmap, gf);
		pBitmapTXM = RemakeBitmap(pBitmapTXM, -1);
		StretchCopyBitmapHW(pBitmapTXM, pBitmap);
		UpdatePic();
		delete pBitmap;
	}
	else {
		MakeBeacon(0, sys.m_RepBeaconFilter, 0);
	}
	AdjustPage(pgTX);
	SetBeaconTick();
	if( pDem->m_repmode >= 2 ) return;
	m_RepBeaconCount++;
	ToTX();
	m_RepBeaconPos++;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SetBeaconTick(void)
{
	m_RepTick = ::GetTickCount();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SetBeaconDelay(void)
{
	DWORD NTick = ::GetTickCount();
	DWORD CTick = m_RepTick + (sys.m_RepBeacon * 1000);
	int qtime = sys.m_RepQuietnessTime;
	if( qtime < 300 ) qtime = 300;
	if( CTick < (NTick + qtime*1000) ){ // 300s前の時は延長
		m_RepTick = NTick - ((sys.m_RepBeacon - qtime) * 1000);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::GetBeaconBitmap(Graphics::TBitmap *pBitmap, int n)
{
	n = (RxHist.m_Head.m_wPnt - 1) - n;
	if( n < 0 ) n += RxHist.m_Head.m_Max;
	if( n >= RxHist.m_Head.m_Max ) n -= RxHist.m_Head.m_Max;
	RxHist.Read(pBitmap, n);
}
//---------------------------------------------------------------------------
// 0 : 4 pix -> TX
// 1 : 3 pix -> TX
void __fastcall TMmsstv::MakeBeacon(int sw, int fsw, int nn)
{
	if( nn < RxHist.m_Head.m_Cnt ){
		Graphics::TBitmap *pBTX = NULL;
		if( sw ) pBTX = DupeBitmap(pBitmapTXM, -1);
		pBitmapTXM = RemakeBitmap(pBitmapTXM, -1);
		FillBitmap(pBitmapTXM, sys.m_PicClipColor);
		Graphics::TBitmap *pBitmap = new Graphics::TBitmap();
		Graphics::TBitmap *pBitmapQ = new Graphics::TBitmap();

		int Y1 = 0;
//        int Y2 = pBitmapTXM->Height;
		int Y2 = m_TXPH;
		if( SBUseTemp->Down && DrawMain.m_Cnt ){
			DrawMain.GetColorBarPos(Y1, Y2);
		}
		int XW = pBitmapTXM->Width / 2;
		int YW = (Y2-Y1) / 2;
		pBitmapQ->Width = XW;
		pBitmapQ->Height = YW;
		int x, y, i;
		for( i = 3; i >= 0; i-- ){
			x = (i & 1) ? XW : 0;
			y = Y1 + ((i / 2) ? YW : 0);
			if( !sw || (i < 3) ){
				if( (i + nn) < RxHist.m_Head.m_Cnt ){
					GetBeaconBitmap(pBitmap, i + nn);
				}
				else {
					GetBeaconBitmap(pBitmap, RxHist.m_Head.m_Cnt - 1);
				}
				if( fsw && (XW <= (pBitmap->Width / 2)) ){
					Graphics::TBitmap *pBitmapF = MFilter(NULL, pBitmap, 1);
					delete pBitmap;
					pBitmap = pBitmapF;
				}
			}
			else {
				::CopyBitmap(pBitmap, pBTX);
			}
			MultProc();
			StretchCopyBitmap(pBitmapQ, pBitmap);
			pBitmapTXM->Canvas->Draw(x, y, pBitmapQ);
			MultProc();
		}
		delete pBitmapQ;
		delete pBitmap;
		UpdatePic();
		if( pBTX != NULL ) delete pBTX;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::MakeReplay(void)
{
	CSSTVDEM *dp = pDem;
	if( dp->m_repmode == 6 ){
		if( !CBASave->Checked ) SBWHistClick(NULL);
		TrackTxMode(1);
		StretchCopyBitmap(pBitmapTXM, pBitmapRX);
		int gf = LoadRepTemplate(sys.m_RepTempTX.c_str(), m_RepTXPos);
		if( gf ){       // 画像のロード
			Graphics::TBitmap *pBitmap = new Graphics::TBitmap();
			LoadBitmapSN(pBitmap, gf);
			pBitmapTXM = RemakeBitmap(pBitmapTXM, -1);
			StretchCopyBitmapHW(pBitmapTXM, pBitmap);
			delete pBitmap;
		}
		UpdatePic();
		dp->m_repmode++;
	}
	else {
		dp->m_repmode = 0;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SendCWID(LPCSTR p)
{
	if( !p || !*p ) return;
	CWaitCursor w;

	sys.m_CWIDSpeed = (1110.0 / sys.m_CWIDWPM) - 30.0 + 0.5;
	if( pRadio != NULL ) pRadio->SetPTT(1);
	if( pComm != NULL ) pComm->SetPTT(1);
	LogLink.SetPTT(1);
	CSSTVMOD *mp = pMod;
	SBTX->Down = TRUE;
	SBTX->Update();
	if( pCtrBtn != NULL ) pCtrBtn->UpdateBtn();

	char bf[256];
	MacroText(bf, p, sizeof(bf)-1);

#if MEASCWSPEED
DWORD dwBgn = ::GetTickCount();
#endif

	mp->OpenTXBuf(10);
	mp->InitTXBuf();
	for( p = bf; *p; p++ ){
		if( mp->GetBufCnt() >= (9*SampFreq) ){
			break;
		}
		else {
			mp->WriteCWID(*p);
		}
	}
	pSound->m_ReqTx = 1;
	for( int i = 0; i < 200; i++ ){
		if( pSound->m_Tx ) break;
		::Sleep(10);
	}
	while(*p){      // 残留テキストの送出
		if( mp->GetBufCnt() < (3*SampFreq) ){
			mp->WriteCWID(*p);
			p++;
		}
		DrawLvl(); PBoxLvl->Update();
		::Sleep(200);
		if( !pSound->Wave.IsOutOpen() ) break;
	}
	while( mp->GetBufCnt() ){
		DrawLvl(); PBoxLvl->Update();
		::Sleep(200);
		if( !pSound->Wave.IsOutOpen() ) break;
	}
	pSound->TrigBCC();
	while( pSound->GetBCC() >= 0 ){
		DrawLvl(); PBoxLvl->Update();
		if( !pSound->Wave.IsOutOpen() ) break;
		::Sleep(200);
	}
#if MEASCWSPEED
char bbf[128];
dwBgn = ::GetTickCount() - dwBgn;
p = bf;
int l;
for(l = 0; *p; p++ ){
	if( *p != ' ' ) l++;
}
double cp = double(l)*60.0*1000.0/double(dwBgn);
double wp = 10.0*60.0*1000.0/double(dwBgn);
sprintf(bbf, "%d, %.3lfcpm, %.3lfwpm, %.3lf", sys.m_CWIDSpeed, cp, wp, wp * (sys.m_CWIDSpeed+30.0));
Caption = bbf;
#endif
	pSound->m_ReqTx = 0;
	int tout = (pSound->m_BuffSize * pSound->m_FifoSizeTX * 10) / SSTVSET.m_TxSampFreq;
	tout += 10;
	for( int i = 0; i < tout; i++ ){
		if( !pSound->m_Tx ) break;
		DrawLvl(); PBoxLvl->Update();
		::Sleep(100);
	}
	if( pRadio != NULL ) pRadio->SetPTT(0);
	if( pComm != NULL ) pComm->SetPTT(0);
	LogLink.SetPTT(0);
	pMod->CloseTXBuf();
	SBTX->Down = 0;
	if( pCtrBtn != NULL ) pCtrBtn->UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::HisCallDblClick(TObject *Sender)
{
	TShiftState Shift;
	SBRXIDMouseUp(Sender, mbRight, Shift, 0, 0);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::HisQTHDblClick(TObject *Sender)
{
	char Key = VK_RETURN;
	HisNameKeyPress(HisQTH, Key);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::MoveParent(TWinControl *pNew, TWinControl *pOld, TWinControl *pCtr)
{
	pOld->RemoveControl(pCtr);
	pNew->InsertControl(pCtr);
	pCtr->Parent = pNew;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DivStockView(void)
{
	if( pStockView == NULL ){
		pStockView = new TStockView(this);
		pStockView->SetViewPos(sys.m_StockViewLeft, sys.m_StockViewTop, sys.m_StockViewWidth, sys.m_StockViewHeight);
		pStockView->Caption = KVSE1->Caption;
		MultProc();
		MoveParent(pStockView, this, TabS);
		TabS->Align = alClient;
		MultProc();
		MoveParent(pStockView, this, PCS);
		MultProc();
		PCS->Top = 0;
		CBSTemp->Visible = FALSE;
		CBSDraft->Visible = FALSE;
		PCS->Width = 100;
		UDStock->Left = PCS->Width - UDStock->Width;
		LS->Left = UDStock->Left - 4 - LS->Width;
		pStockView->Visible = KVSE1->Checked;
	}
	else {
		pStockView->GetViewPos(sys.m_StockViewLeft, sys.m_StockViewTop, sys.m_StockViewWidth, sys.m_StockViewHeight);
		MoveParent(this, pStockView, TabS);
		TabS->Align = alBottom;
		MoveParent(this, pStockView, PCS);
		MultProc();
		CBSTemp->Visible = TRUE;
		CBSDraft->Visible = TRUE;

		MultProc();
		delete pStockView;
		pStockView = NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DivWaterView(void)
{
	if( pWaterView == NULL ){
		pWaterView = new TStockView(this);
		pWaterView->SetViewPos(sys.m_WaterViewLeft, sys.m_WaterViewTop, sys.m_WaterViewWidth, sys.m_WaterViewHeight);
		pWaterView->Caption = KVSE2->Caption;
		MultProc();
		PBoxG->Parent = pWaterView;
		MoveParent(pWaterView, this, PanelLvl);
		MoveParent(pWaterView, this, PanelFFT);
		MoveParent(pWaterView, this, PanelWater);

		pLogView = new TStockView(this);
		pLogView->SetViewPos(sys.m_LogViewLeft, sys.m_LogViewTop, sys.m_LogViewWidth, sys.m_LogViewHeight);
		pLogView->Caption = KVSE3->Caption;
		MultProc();
		MoveParent(pLogView, this, GBLog);
		GBLog->Align = alClient;

		AlignFix.Resume(GBLog);
		pLogView->AlignList.EntryControl(GBLog, 367, 164, GBLog->Font);
		AlignFix.Resume(GBMode);
		AlignFix.Resume(GB1);
		pWaterView->Visible = KVSE2->Checked;
		pLogView->Visible = KVSE3->Checked;
	}
	else {
		pWaterView->GetViewPos(sys.m_WaterViewLeft, sys.m_WaterViewTop, sys.m_WaterViewWidth, sys.m_WaterViewHeight);
		pLogView->GetViewPos(sys.m_LogViewLeft, sys.m_LogViewTop, sys.m_LogViewWidth, sys.m_LogViewHeight);

		PBoxG->Parent = this;
		MoveParent(this, pWaterView, PanelLvl);
		MoveParent(this, pWaterView, PanelFFT);
		MoveParent(this, pWaterView, PanelWater);

		MoveParent(this, pLogView, GBLog);
		GBLog->Align = alNone;

		delete pWaterView;
		pWaterView = NULL;
		delete pLogView;
		pLogView = NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DivControl(int sw)
{
	switch(sw){
		case 1:     // ストック画面のみ分離
			if( pWaterView != NULL ) DivWaterView();
			if( pStockView == NULL ) DivStockView();
			break;
		case 2:     // すべて分離
			if( pStockView == NULL ) DivStockView();
			if( pWaterView == NULL ) DivWaterView();
			break;
		default:    // すべて合体
			if( pWaterView != NULL ) DivWaterView();
			if( pStockView != NULL ) DivStockView();
			break;
	}
	sys.m_DivMode = sw;
	FormResize(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KVSD1Click(TObject *Sender)
{
	if( Sender == KVSD2 ){
		sys.m_DivMode = 1;
	}
	else if( Sender == KVSD3 ){
		sys.m_DivMode = 2;
	}
	else {
		sys.m_DivMode = 0;
	}
	DivControl(sys.m_DivMode);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OpenHistView(void)
{
	MultProc();
	pHistView = new THistViewDlg(this);
	MultProc();
	pHistView->m_TitleBar = sys.m_HistViewTB;
	pHistView->UpdateTitle(NULL);
	pHistView->SetViewPos(sys.m_HistViewLeft, sys.m_HistViewTop, sys.m_HistViewWidth, sys.m_HistViewHeight);
	if( !pHistView->m_TitleBar ) pHistView->UpdateTitlebar();
	pHistView->LoadHist(0);
	MultProc();
	pHistView->SetPopup(PopupH);
	MultProc();
	pHistView->Visible = TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::CloseHistView(void)
{
	if( pHistView != NULL ){
		pHistView->GetViewPos(sys.m_HistViewLeft, sys.m_HistViewTop, sys.m_HistViewWidth, sys.m_HistViewHeight);
		delete pHistView;
		pHistView = NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHVSClick(TObject *Sender)
{
	if( Sender == KHVS1 ){
		sys.m_HistViewSize = 0;
	}
	else if( Sender == KHVS2 ){
		sys.m_HistViewSize = 1;
	}
	else if( Sender == KHVS3 ){
		sys.m_HistViewSize = 2;
	}
	else {
		sys.m_HistViewSize = 3;
	}
	if( pHistView != NULL ){
		CloseHistView();
		OpenHistView();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRFSClick(TObject *Sender)
{
	CSSTVDEM *dp = pDem;
	if( dp->m_Sync && (SSTVSET.m_Mode != smAVT) && (m_SyncRPos >= 0) ){
		if( ABS(m_SyncPos - int(SSTVSET.m_OFP)) >= 5 ){
			int Skip = m_SyncRPos - int(SSTVSET.m_OFP);
			if( Skip < 0 ) Skip += int(SSTVSET.m_TW);
			dp->m_Skip = Skip;
			m_SyncPos = m_SyncRPos = -1;
			m_AutoSyncPos = 0x7fffffff;
			m_AutoStopCnt = 0;
			m_AutoStopACnt = 0;
			m_AutoSyncDis = 6;
			m_AutoSyncCount++;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSBClick(TObject *Sender)
{
	InitCustomColor(ColorDialog);
	AddCustomColor(ColorDialog, sys.m_PicClipColor);
	ColorDialog->Color = sys.m_PicClipColor;
	SetDisPaint();
	if( ColorDialog->Execute() == TRUE ){
		sys.m_PicClipColor = ColorDialog->Color;
	}
	if( Sender == KSB ){
		MultProc();
		MakeTxBitmap(1);
		PBoxTX->Invalidate();
	}
	ResDisPaint();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::UpdatePriority(void)
{
	switch(sys.m_Priority){
		case 1:
			::SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
			break;
		case 2:
			::SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
			break;
		default:
			::SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHMT3Click(TObject *Sender)
{
	MakeBeacon(Sender == KHMT3 ? 1 : 0, 1, UDHist->Position);
	AdjustPage(pgTX);
}

//---------------------------------------------------------------------------
void __fastcall TMmsstv::MakeThImage(int mm)
{
	if( m_DragNo < 0 ) return;

	pBitmapTXM = RemakeBitmap(pBitmapTXM, -1);
	FillBitmap(pBitmapTXM, sys.m_PicClipColor);
	Graphics::TBitmap *pBitmap = new Graphics::TBitmap();
	Graphics::TBitmap *pBitmapQ = new Graphics::TBitmap();

	int Y1 = 0;
//    int Y2 = pBitmapTXM->Height;
	int Y2 = m_TXPH;
	int X2 = pBitmapTXM->Width;
	if( SBUseTemp->Down && DrawMain.m_Cnt ){
		DrawMain.GetColorBarPos(Y1, Y2);
	}
	int XW = pBitmapTXM->Width / mm;
	int YW = (Y2-Y1) / mm;
	int x, y, i;
	int n = m_DragDataNo;
	int m = 0;
	for( i = 0; i < STOCKMAX; i++, n++ ){
		MultProc();
		if( n >= STOCKMAX ) n = 0;
		LoadBitmapSN(pBitmap, n+1);
		if( pBitmap->Modified != FALSE ){
			x = (m % mm) * XW;
			y = Y1 + ((m / mm) * YW);
			if( ((m % mm) == (mm - 1)) && (x + XW) != X2 ){
				pBitmapQ->Width = X2 - x;
			}
			else {
				pBitmapQ->Width = XW;
			}
			if( ((m / mm) == (mm - 1)) && (y + YW) != Y2 ){
				pBitmapQ->Height = Y2 - y;
			}
			else {
				pBitmapQ->Height = YW;
			}
			MultProc();
			StretchCopyBitmapHW(pBitmapQ, pBitmap);
			pBitmapTXM->Canvas->Draw(x, y, pBitmapQ);
			MultProc();
			m++;
			if( m >= (mm*mm) ) break;
		}
	}
	delete pBitmapQ;
	delete pBitmap;
	UpdatePic();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSMS4Click(TObject *Sender)
{
	int n;
	if( Sender == KSMS4 ){
		n = 2;
	}
	else if( Sender == KSMS9 ){
		n = 3;
	}
	else {
		n = 4;
	}
	MakeThImage(n);
	AdjustPage(pgTX);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHelpClick(TObject *Sender)
{
	MultProc();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DropOle(int X, int Y, LPCSTR pName, int trans)
{
	CDrawOle *pDraw = (CDrawOle *)DrawMain.MakeItem(CM_OLE);
	if( pDraw != NULL ){
		if( X >= 0 ) AdjustTempSC(X, Y);
		pDraw->m_Trans = trans;
		if( pDraw->LoadFromFile(X, Y, pName) ){
			AddItem(pDraw, 0);
		}
		else {
			delete pDraw;
		}
		UpdateTemp();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DropLib(int X, int Y, LPCSTR pName)
{
	AdjustPage(pgTemp);
	MultProc();
	CDrawLib *pDraw = (CDrawLib *)DrawMain.MakeItem(CM_LIB);
	if( pDraw != NULL ){
		AdjustTempSC(X, Y);
		if( X < 0 ){
			X = (DrawMain.m_SX - 160) / 2;
			Y = (DrawMain.m_SY - 128) / 2;
		}
		pDraw->m_X1 = X; pDraw->m_Y1 = Y;
		pDraw->m_X2 = X + 160; pDraw->m_Y2 = Y + 128;
		if( pDraw->MakeR(pName) ){
			AddItem(pDraw, 0);
		}
		else {
			delete pDraw;
		}
		UpdateTemp();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DropPic(int X, int Y, Graphics::TBitmap *pSrc, int type)
{
	AdjustPage(pgTemp);
	MultProc();
	CDrawPic *pDraw = (CDrawPic *)DrawMain.MakeItem(CM_PIC);
	MultProc();
	if( pDraw != NULL ){
		AdjustTempSC(X, Y);
		int lx = DrawMain.m_SX;
		int ly = DrawMain.m_SY;
		int w = pSrc->Width;
		int h = pSrc->Height;
		if( (w > lx) || (h > ly) ){
			KeepAspect(lx, ly, w, h);
			w = lx; h = ly;
		}
		if( X < 0 ){
			X = (DrawMain.m_SX - w) / 2;
			Y = (DrawMain.m_SY - h) / 2;
		}
		w++; h++;
		AdjustTempView(1);
		pDraw->Start(PBoxTemp->Canvas, X, Y);
		AdjustTempView(0);
		X += w;
		Y += h;
		pDraw->Making(X, Y);
		if( pDraw->FinishR(X, Y) ){
			pDraw->m_Type = type;
			pDraw->m_Shape = 0;
			pDraw->m_LineStyle = TPenStyle(-1);
			::StretchCopy(pDraw->pBitmap, pSrc, type == 2 ? COLORONCOLOR : HALFTONE);
			AddItem(pDraw, 1);
		}
		else {
			delete pDraw;
		}
		UpdateTemp();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTPBClick(TObject *Sender)
{
	TClipboard *pCB = Clipboard();
	if(pCB->HasFormat(CF_BITMAP)){
		Graphics::TBitmap *pBitmap = new Graphics::TBitmap;
		try
		{
			pBitmap->LoadFromClipboardFormat(CF_BITMAP, pCB->GetAsHandle(CF_BITMAP), 0);
			DropPic(-1, 0, pBitmap, (Sender == KTPB) ? 1 : 2);
		}
		catch (...)
		{
		}
		delete pBitmap;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LoadStockTemp(CDrawGroup *pItem, int n, TCanvas *tp)
{
	char tname[256];
	sprintf(tname, "%st%d.mtm", StockDir, n+1);
	if( IsFile(tname) ){
		LoadTemplate(pItem, tname, tp);
	}
	else {
		pItem->FreeItem();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SaveStockTemp(CDrawGroup *pItem, int n)
{
	char tname[256];
	sprintf(tname, "%st%d.mtm", StockDir, n+1);
	SaveTemplate(pItem, tname);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::DeleteStockTemp(int n)
{
	char tname[256];
	sprintf(tname, "%st%d.mtm", StockDir, n+1);
	unlink(tname);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LoadCurrentTemp(void)
{
	char bf[256];
	sprintf(bf, "%s"TEMPV106C, StockDir);   // Current.mtm
	if( IsFile(bf) ){       // Ver1.06
		LoadTemplate(&DrawMain, bf, NULL);
		sprintf(bf, "%s"TEMPV106L, StockDir);   // StrList.mtl
		LoadTemplate(&DrawText, bf, NULL);
		return;
	}
	sprintf(bf, "%s"TEMPV105, StockDir);    // CurTemp.bin
	if( IsFile(bf) ){       // Ver1.05
		try {
			char fname[256];
			TFileStream *tp = new TFileStream(bf, fmOpenRead);
			if( DrawMain.LoadFromStream(PBoxTemp->Canvas, tp) == TRUE ){
				int i;
				for( i = 0; i < STOCKMAX; i++ ){
					if( DrawTemp.LoadFromStream(PBoxTemp->Canvas, tp) == FALSE ) break;
					if( DrawTemp.m_Cnt ){
						sprintf(fname, "%st%d.mtm", StockDir, i+1);
						if( SaveTemplate(&DrawTemp, fname) == FALSE ) break;
					}
				}
				if( i >= STOCKMAX ){
					DrawText.LoadFromStream(PBoxTemp->Canvas, tp);
				}
			}
			delete tp;
			sprintf(fname, "%s"TEMPV105B, StockDir);    // CurTemp.105
			chmod(bf, S_IREAD|S_IWRITE);
			rename(bf, fname);  // CurTemp.bin -> CurTemp.bak
			unlink(bf);
			DrawTemp.FreeItem();
		}
		catch(...){
		}
		return;
	}
	else {                  // 初期起動
		LoadDefTemp();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBRXFilClick(TObject *Sender)
{
	Graphics::TBitmap *pBitmap = DupeBitmap(pBitmapRX, pf24bit);
	TPicFilterDlg *pBox = new TPicFilterDlg(this);
	pBox->OKBtn->Enabled = FALSE;
	pBox->Execute(pBitmap);
	delete pBox;
	delete pBitmap;
}
//---------------------------------------------------------------------------

void __fastcall TMmsstv::UDHistMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		if( !RxHist.m_Head.m_Cnt ) return;
		int n = UDHist->Position;
		int w = UDHist->Width/2;
		if( X < w ){
			n += 5;
		}
		else {
			n -= 5;
		}
		int max = RxHist.m_Head.m_Cnt - 1;
		if( n < 0 ) n = 0;
		if( n >= max ) n = max;
		UDHist->Position = SHORT(n);
		UDHistClick(NULL, btNext);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::IMenuClick(TObject *Sender)
{
	InvMenu((TMenuItem *)Sender);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KVSDClick(TObject *Sender)
{
	KVSE1->Enabled = (pStockView != NULL) ? TRUE : FALSE;
	KVSE1->Checked = (pStockView == NULL) || pStockView->Visible ? TRUE : FALSE;
	KVSE2->Enabled = (pWaterView != NULL) ? TRUE : FALSE;
	KVSE2->Checked = (pWaterView == NULL) || pWaterView->Visible ? TRUE : FALSE;
	KVSE3->Enabled = (pLogView != NULL) ? TRUE : FALSE;
	KVSE3->Checked = (pLogView == NULL) || pLogView->Visible ? TRUE : FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KVSE1Click(TObject *Sender)
{
	IMenuClick(Sender);
	FormResize(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxFFTMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	int notch = pSound->m_notch;
	if( Button == mbLeft ){
//        m_FX[4] = int(((fq-m_low)*FFT_SIZE*double(m_XW)/double(FFTSamp*m_FM)) + 0.5);
		m_FX[5] = X;
		double f = X * double(FFTSamp*m_FM) / (FFT_SIZE*double(m_XW));
		pSound->notch.SetNotchFreq(f + m_low);
		pSound->m_notch = 1;
		m_SpecDown = 1;
	}
	else {
		pSound->m_notch = pSound->m_notch ? 0 : 1;
	}
	if( (notch != pSound->m_notch) && pDem->m_Sync ){
		int delay = pSound->notch.m_tap / 2;
		if( notch ) delay = -delay;
		pDem->m_Skip = delay;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxFFTMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
	if( m_SpecDown && pSound->m_notch ){
		m_FX[5] = X;
		double f = X * double(FFTSamp*m_FM) / (FFT_SIZE*double(m_XW));
		pSound->notch.SetNotchFreq(f + m_low);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PBoxFFTMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	m_SpecDown = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBTXMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		KOMClick(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBLMSMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		switch(pSound->m_lms_AN){
			case 1:
				KLM2->Checked = TRUE;
				break;
			case 2:
				KLM3->Checked = TRUE;
				break;
			default:
				KLM1->Checked = TRUE;
				break;
		}
		PopupL->Popup(Left + GB1->Left + SBLMS->Left + X + 4, m_MW + Top + GB1->Top + SBLMS->Top);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KLM1Click(TObject *Sender)
{
	int d = GetLMSDelay();
	int n = 0;
	if( Sender == KLM2 ){
		n = 1;
	}
	else if( Sender == KLM3 ){
		n = 2;
	}
	pSound->lms.SetAN(n);
	pSound->m_lms_AN = n;
    if( pDem->m_Sync ) pDem->m_Skip = GetLMSDelay() - d;
	UpdateLMS();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSAClick(TObject *Sender)
{
	if( m_DragNo >= 0 ){
		AdjustPage(pgTemp);
		LoadStockTemp(&DrawTemp, m_DragDataNo, NULL);
		AddTemplate(&DrawTemp);
		UpdateTemp();
	}
//
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTFSClick(TObject *Sender)
{
	sys.m_DisFontSmooth = sys.m_DisFontSmooth ? FALSE : TRUE;
	m_ChangeTemp = 1;
	UpdateTemp();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::CMMML(TMessage Message)
{
	if( sys.m_LogLink != 2 ) return;
	if( LogLink.m_pLink == NULL ) return;

	switch(Message.WParam){
		case MML_NOTIFYSESSION:
			LogLink.NotifySession((LPCSTR)Message.LParam);
			UpdateLogLink();
			break;
		case MML_QRETURN:
			if( !LogLink.IsLink() ) return;
			if( LogLink.QReturn(&Log.m_sd, (const mmLOGDATA *)Message.LParam) ){
				UpdateTextData();
			}
			break;
		case MML_VFO:
			if( !LogLink.IsLink() ) return;
			if( Message.LParam ){
				LogFreq->Text = (LPCSTR)Message.LParam;
			}
			break;
	}
	Message.Result = TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::CMMMR(TMessage Message)
{
	if( pRadio == NULL ) return;

	switch(Message.WParam){
		case MMR_DEFCOMMAND:
			{
				LPCSTR p;
				switch(Message.LParam){
					case 1:
						p = RADIO.CmdRx.c_str();
						break;
					case 2:
						p = RADIO.CmdTx.c_str();
						break;
					default:
						p = RADIO.CmdInit.c_str();
						break;
				}
				pRadio->SendCommand(p);
			}
			break;
		case MMR_VFO:
			pRadio->UpdateFreq(double(Message.LParam)/10000.0);
			break;
	}
	Message.Result = TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OnDrawClipboard(TMessage Message)
{
	SBPaste->Enabled = !SBTX->Down && ::IsClipboardFormatAvailable(CF_BITMAP);
	if( m_hClipNext ) ::SendMessage(m_hClipNext, WM_DRAWCLIPBOARD, Message.WParam, Message.LParam);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OnChangeCbChain(TMessage Message)
{
	if( (HWND)Message.WParam == m_hClipNext ){
		m_hClipNext = (HWND)Message.LParam;
	}
	if( m_hClipNext ) ::SendMessage(m_hClipNext, WM_CHANGECBCHAIN, Message.WParam, Message.LParam);
    Message.Result = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHJClick(TObject *Sender)
{
	ShowHelp(this, "JASTA.TXT", MsgEng);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHNClick(TObject *Sender)
{
	ShowHelp(this, "NVCG.TXT", MsgEng);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBCWMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
#if 1
		RECT rc;
		::GetWindowRect(TabTX->Handle, &rc);
		PopupCW->Popup(rc.left + SBCW->Left + SBCW->Width/2, rc.top + SBCW->Top + SBCW->Height/2);
#else
		AnsiString as = sys.m_CWText;
		if( InputMB("MMSSTV", MsgEng ? "Enter text" : "テキストを入力", as, TRUE) == TRUE ){
			sys.m_CWText = as;
		}
#endif
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSAMClick(TObject *Sender)
{
	KSAM->Checked = !KSAM->Checked;
    if( KSAM->Checked ) ChangeTxMode(SSTVSET.m_TxMode);
}
//---------------------------------------------------------------------------

void __fastcall TMmsstv::SBListMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ) KOLClick(NULL);
}
//---------------------------------------------------------------------------

void __fastcall TMmsstv::PBoxLvlMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbLeft ){
		KVOutClick(NULL);
    }
    else {
		KVOutClick(KVOut);
    }
}
//---------------------------------------------------------------------------

void __fastcall TMmsstv::SBTXIDClick(TObject *Sender)
{
	sys.m_TXFSKID = SBTXID->Down;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::SBDFontMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		if( DrawMain.pSel != NULL ){
			if( DrawMain.pSel->m_Command == CM_TEXT ){
				SaveUndo();
				CDrawText *pText = (CDrawText *)(DrawMain.pSel);
				FontDialog->Font->Assign(pText->pFont);
                InvFontCharset(FontDialog->Font);
				pText->UpdateFont(FontDialog->Font);
				if( KTAP->Checked ) DrawMain.AdjustTransPoint();
				UpdateTemp();
			}
			else if( DrawMain.pSel->m_Command == CM_LIB ){
				CDrawLib *pLib = (CDrawLib *)DrawMain.pSel;
                LOGFONT lf;
				TMemoryStream *sp = SaveUndo(NULL);
                if( pLib->GetFont(&lf) ){
					lf.lfCharSet = InvFontCharset(lf.lfCharSet);
                    pLib->SetFont(&lf);
					UpdateTemp();
					UpdateUndo(sp);
				}
				else {
					delete sp;
				}
			}
		}
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KTHClick(TObject *Sender)
{
	sys.m_Temp24 = !sys.m_Temp24;
	m_ChangeTemp = 1;
	UpdatePic();
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KEEClick(TObject *Sender)
{
	g_ExecPB.Exec(pBitmapTXM, (HWND)Handle, CM_EDITEXIT, 0);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::OnEditExit(TMessage Message)
{
	if( Message.LParam & 0x40000000 ){
		if( g_ExecPB.IsChanged() ){
			int no = (Message.LParam & 0x00007fff);
            if( no == UDHist->Position ){
	    	    if( g_ExecPB.LoadBitmap(pBitmapHist, FALSE) ){
					PBoxHistPaint(NULL);
					DrawMain.UpdatePic(pBitmapHistF);
					UpdatePic();
                }
            }
        }
    }
	else if( Message.LParam & 0x80000000 ){
		if( g_ExecPB.IsChanged() ){
			int no = (Message.LParam & 0x00007fff) + 1;
			Graphics::TBitmap *pBitmap = new Graphics::TBitmap;
            LoadBitmapSN(pBitmap, no);
    	    if( g_ExecPB.LoadBitmap(pBitmap, FALSE) ){
            	SaveBitmapSN(pBitmap, no);
				UpdateTabS();
    	    }
	        delete pBitmap;
        }
	}
	else if( g_ExecPB.LoadBitmap(pBitmapTXM, FALSE) ){
		AdjustPage(pgTX);
		MakeTxBitmap(0);
		PBoxTXPaint(NULL);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KSEClick(TObject *Sender)
{
	if( PopupS->PopupComponent == PBoxTX ){
		KEEClick(NULL);
	}
    else {
    	if( m_DragNo < 0 ) return;
		Graphics::TBitmap *pBitmap = LoadBitmapS(m_DragNo);
		g_ExecPB.Exec(pBitmap, (HWND)Handle, CM_EDITEXIT, 0x80000000 + m_DragDataNo);
		delete pBitmap;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KXPClick(TObject *Sender)
{
	ExecPB(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KHEClick(TObject *Sender)
{
	g_ExecPB.Exec(pBitmapHist, (HWND)Handle, CM_EDITEXIT, 0x40000000 + UDHist->Position);
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AdjustCWMenu(void)
{
    int max = PopupCW->Items->IndexOf(NCW);
	TMenuItem *pm;
    AnsiString as;
	int i;
	for( i = 0; i < sys.m_nCWMenu; i++ ){
		if( i >= max ){
			TMenuItem *pm = new TMenuItem (this);
			PopupCW->Items->Insert(i, pm);
            max++;
        }
		pm = PopupCW->Items->Items[i];
		char bf[256];
		MacroText(bf, sys.m_CWMenu[i].c_str(), sizeof(bf)-1);
        as = bf;
		if( pm->Caption != as ){
			pm->Caption = as;
        }
        pm->OnClick = KCWSendClick;
    }
	int N = i;
    for( ; i < max; i++ ){
		PopupCW->Items->Delete(N);
    }

    max = KCWM->IndexOf(NCWE);
	for( i = 0; i < sys.m_nCWMenu; i++ ){
		if( i >= max ){
			TMenuItem *pm = new TMenuItem (this);
			KCWM->Insert(i, pm);
            max++;
        }
		pm = KCWM->Items[i];
		if( pm->Caption != sys.m_CWMenu[i] ){
			pm->Caption = sys.m_CWMenu[i];
        }
        pm->OnClick = KCWEditClick;
    }
	N = i;
    for( ; i < max; i++ ){
		KCWM->Delete(N);
    }
	BOOL f = sys.m_nCWMenu != 0;
	NCW->Visible = f;
	NCWE->Visible = f;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KCWClick(TObject *Sender)
{
	AnsiString as = sys.m_CWText;
	if( InputMB(MsgEng ? "CW button" : "CWボタン", MsgEng ? "Enter text" : "テキストを入力", as, TRUE) == TRUE ){
		sys.m_CWText = as;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::PopupCWPopup(TObject *Sender)
{
	AdjustCWMenu();
	KCWADD->Enabled = sys.m_nCWMenu < CWMENUMAX;

	char bf[256];
	MacroText(bf, sys.m_CWText.c_str(), sizeof(bf)-1);
    AnsiString as = MsgEng ? "Set button text... (" : "ボタンの送信テキストの設定... (";
    as += bf;
    as += ")";
    KCW->Caption = as;

    switch(sys.m_CWIDWPM){
		case 28:
			KCWSH->Checked = TRUE;
            break;
		case 24:
			KCWSM->Checked = TRUE;
            break;
        case 20:
			KCWSL->Checked = TRUE;
            break;
        case 18:
			KCWSLL->Checked = TRUE;
            break;
		default:
			KCWSI->Checked = TRUE;
			break;
    }
    if( KCWSI->Checked ){
	    as = sys.m_CWIDWPM;
	    as += " WPM ";
	}
    else {
		as = "";
    }
	as += MsgEng ? "Enter PARIS speed..." : "PARIS速度で入力...";
	KCWSI->Caption = as;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KCWADDClick(TObject *Sender)
{
	if( sys.m_nCWMenu >= CWMENUMAX ) return;

	AnsiString as;
	if( InputMB(MsgEng ? "Add CW menu" : "CWメニュー追加", MsgEng ? "Enter text" : "テキストを入力", as, TRUE) == TRUE ){
		if( !as.IsEmpty() ){
			sys.m_CWMenu[sys.m_nCWMenu++] = as;
		}
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KCWSendClick(TObject *Sender)
{
	int n = PopupCW->Items->IndexOf((TMenuItem *)Sender);
	if( (n >= 0) && (n < sys.m_nCWMenu) ){
		SendCWID(sys.m_CWMenu[n].c_str());
        Application->ProcessMessages();
		SBCW->Down = FALSE;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KCWEditClick(TObject *Sender)
{
	int n = KCWM->IndexOf((TMenuItem *)Sender);
	if( (n >= 0) && (n < sys.m_nCWMenu) ){
		AnsiString as = sys.m_CWMenu[n];
		if( InputMB(MsgEng ? "Edit CW menu" : "CWメニュー編集", MsgEng ? "Enter text (NULL for delete)" : "テキストを入力(NULLでメニューを削除)", as, TRUE) == TRUE ){
			if( as.IsEmpty() ){
				for( int i = n; i < (sys.m_nCWMenu - 1); i++ ){
					sys.m_CWMenu[i] = sys.m_CWMenu[i+1];
                }
                sys.m_nCWMenu--;
            }
            else {
				sys.m_CWMenu[n] = as;
            }
		}
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KCWSHClick(TObject *Sender)
{
	sys.m_CWIDWPM = 28;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KCWSMClick(TObject *Sender)
{
	sys.m_CWIDWPM = 24;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KCWSLClick(TObject *Sender)
{
	sys.m_CWIDWPM = 20;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KCWSLLClick(TObject *Sender)
{
	sys.m_CWIDWPM = 18;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KCWSIClick(TObject *Sender)
{
	AnsiString as = sys.m_CWIDWPM;
	if( InputMB("MMSSTV", "CW Speed (PARIS 10 - 50 WPM)", as, FALSE) == TRUE ){
		int d;
		if( sscanf(as.c_str(), "%d", &d) == 1 ){
			if( (d >= 10) && (d <= 50) ){
				sys.m_CWIDWPM = d;
            }
        }
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRSetClick(TObject *Sender)
{
	TRADIOSetDlg *pBox = new TRADIOSetDlg(this);

	if( (sys.m_TxRxName != "NONE") && !strcmp(sys.m_TxRxName.c_str(), RADIO.StrPort) ){
		strcpy(RADIO.StrPort, "NONE");
		RADIO.change = 1;
	}
	if( pBox->Execute() == TRUE ){
		if( (sys.m_TxRxName != "NONE") && !strcmp(sys.m_TxRxName.c_str(), RADIO.StrPort) ){
			sys.m_TxRxName = "NONE";
            COMM.change = 1;
		}
	}
	delete pBox;
	if( COMM.change || RADIO.change ){
		OpenCloseCom();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::LoadRadioDef(LPCSTR pName)
{
	CWaitCursor w;
	try {
		TMemIniFile *pIniFile = new TMemIniFile(pName);
		RADIO.CmdInit = pIniFile->ReadString("RADIO", "CmdInit", RADIO.CmdInit);
		RADIO.CmdRx = pIniFile->ReadString("RADIO", "CmdRx", RADIO.CmdRx);
		RADIO.CmdTx = pIniFile->ReadString("RADIO", "CmdTx", RADIO.CmdTx);
		char bf[32];
		sprintf( bf, "%02X", RADIO.Cmdxx);
		AnsiString as = pIniFile->ReadString("RADIO", "Cmdxx", bf);
		int d;
	    sscanf(as.c_str(), "%X", &d);
	    RADIO.Cmdxx = d;
		RADIO.PollType = pIniFile->ReadInteger("RADIO", "PollType", 0);
		RADIO.PollInterval = pIniFile->ReadInteger("RADIO", "PollInterval", 0);
		delete pIniFile;
		OpenCloseCom();
    }
    catch(...){
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRLoadClick(TObject *Sender)
{
	TOpenDialog *pBox = Mmsstv->OpenDialog;
	pBox->Options >> ofCreatePrompt;
	pBox->Options << ofFileMustExist;
	if( MsgEng ){
		pBox->Title = "Open command file";
		pBox->Filter = "Command files(*.rcm)|*.rcm|";
	}
	else {
		pBox->Title = "コマンド定義ファイルを開く";
		pBox->Filter = "コマンド定義ファイル(*.rcm)|*.rcm|";
	}
	pBox->FileName = "";
	pBox->DefaultExt = "rcm";
	pBox->InitialDir = BgnDir;
	NormalWindow(this);
	if( pBox->Execute() == TRUE ){
		LoadRadioDef(AnsiString(pBox->FileName).c_str());
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::AdjustRadioMenu(void)
{
    int max = KRadio->IndexOf(NRadio);
	TMenuItem *pm;
	int i;
	for( i = 0; i < m_nRadioMenu; i++ ){
		if( i >= max ){
			TMenuItem *pm = new TMenuItem (this);
			KRadio->Insert(i, pm);
            max++;
        }
		pm = KRadio->Items[i];
		if( pm->Caption != m_RadioMenu[i].strTTL ){
			pm->Caption = m_RadioMenu[i].strTTL;
        }
        pm->OnClick = KRadioCmdClick;
        pm->Enabled = pRadio != NULL;
    }
	int N = i;
    for( ; i < max; i++ ){
		KRadio->Delete(N);
    }

    max = KRadioS->IndexOf(NRadioE);
	for( i = 0; i < m_nRadioMenu; i++ ){
		if( i >= max ){
			TMenuItem *pm = new TMenuItem (this);
			KRadioS->Insert(i, pm);
            max++;
        }
		pm = KRadioS->Items[i];
		if( pm->Caption != m_RadioMenu[i].strTTL ){
			pm->Caption = m_RadioMenu[i].strTTL;
        }
        pm->OnClick = KRadioEditClick;
    }
	N = i;
    for( ; i < max; i++ ){
		KRadioS->Delete(N);
    }
	BOOL f = m_nRadioMenu != 0;
	NRadio->Visible = f;
	NRadioE->Visible = f;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRadioCmdClick(TObject *Sender)
{
	if( !pRadio ) return;

    int n = KRadio->IndexOf((TMenuItem *)Sender);
    if( (n >= 0) && (n < RADIOMENUMAX) ){
		LPCSTR p = m_RadioMenu[n].strCMD.c_str();
        if( CheckEXT(p, "RCM") && IsFile(p) ){
			LoadRadioDef(p);
        }
        else {
			pRadio->SendCommand(p);
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRadioEditClick(TObject *Sender)
{
    int n = KRadioS->IndexOf((TMenuItem *)Sender);
    if( (n >= 0) && (n < RADIOMENUMAX) ){
		AnsiString strTTL = m_RadioMenu[n].strTTL;
        AnsiString strCMD = m_RadioMenu[n].strCMD;

		TRMenuDialog *pBox = new TRMenuDialog(this);
		int r = pBox->Execute(strTTL, strCMD);
        BOOL fNull = strTTL.IsEmpty() && strCMD.IsEmpty();
		if( (r == IDOK) && !fNull ){
			if( !strCMD.IsEmpty() ){
				if( strTTL.IsEmpty() ){
					char bf[64];
			        sprintf(bf, "Radio command #d", n + 1);
					strTTL = bf;
	            }
	            m_RadioMenu[n].strTTL = strTTL;
	            m_RadioMenu[n].strCMD = strCMD;
	            AdjustRadioMenu();
	        }
	    }
        else if( (r == 1024) || fNull ){
			for( int i = n; i < (m_nRadioMenu - 1); i++ ){
	            m_RadioMenu[i].strTTL = m_RadioMenu[i+1].strTTL;
	            m_RadioMenu[i].strCMD = m_RadioMenu[i+1].strCMD;
            }
            m_nRadioMenu--;
            AdjustRadioMenu();
        }
        else if( r == 1025 ){
			if( m_nRadioMenu < RADIOMENUMAX ){
				for( int i = m_nRadioMenu; i > n; i-- ){
		            m_RadioMenu[i].strTTL = m_RadioMenu[i-1].strTTL;
		            m_RadioMenu[i].strCMD = m_RadioMenu[i-1].strCMD;
	            }
	            m_RadioMenu[n].strTTL = strTTL;
	            m_RadioMenu[n].strCMD = strCMD;
	            m_nRadioMenu++;
	            AdjustRadioMenu();
            }
        }
        delete pBox;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRadioAddClick(TObject *Sender)
{
	if( m_nRadioMenu >= RADIOMENUMAX ) return;

	AnsiString strTTL, strCMD;

	TRMenuDialog *pBox = new TRMenuDialog(this);
	if( pBox->Execute(strTTL, strCMD, TRUE) ){
		if( !strCMD.IsEmpty() ){
			if( strTTL.IsEmpty() ){
				char bf[64];
		        sprintf(bf, "Radio command #d", m_nRadioMenu + 1);
				strTTL = bf;
            }
            m_RadioMenu[m_nRadioMenu].strTTL = strTTL;
            m_RadioMenu[m_nRadioMenu].strCMD = strCMD;
            m_nRadioMenu++;
            AdjustRadioMenu();
        }
    }
    delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TMmsstv::KRadioClick(TObject *Sender)
{
	AdjustRadioMenu();
    KRadioAdd->Enabled = m_nRadioMenu < RADIOMENUMAX;
}
//---------------------------------------------------------------------------

