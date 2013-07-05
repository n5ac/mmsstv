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



//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Option.h"
#include "Main.h"
#include "FreqDisp.h"
#include "radioset.h"
#include "LogFile.h"
#include "ClockAdj.h"
#include "LinearDs.h"
#include "MacroKey.h"
#include "TextEdit.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
int PageIndex = 0;
//---------------------------------------------------------------------
__fastcall TOptionDlg::TOptionDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	m_DisEvent = 1;
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	pDem = NULL;
	if( MsgEng ){
		Caption = "Setup MMSSTV";
		TabDem->Caption = "RX";
		TabTx->Caption = "TX";
		TabMisc->Caption = "Misc";
		CancelBtn->Caption = "Cancel";

		RGDemType->Caption = "Demodulating method";
		RGDemType->Items->Strings[1] = "Zero crossing";
		RGDemType->Items->Strings[2] = "Hilbert T.F.";

		GBCROSS->Caption = "Zero crossing";
		LCORDER->Caption = "Order";

		CBDiff->Caption = "Differentiator";
		LLoopOrder->Caption = "Order";
		LOutOrder->Caption = "Order";
		RGRBuf->Caption = "Rx buffer";
		GBTemp->Caption = "Template";
		GBOut->Caption = "Digital output level";
		GrpCard->Caption = "Sound Card";
		GBHMax->Caption = "History max.";
		LFFTB->Caption = "Background";
		LFFT->Caption = "Signals";
		LFFTT->Caption = "Trails";
		LFFTS->Caption = "Sync marker";
		LFFTF->Caption = "Freq marker";
		RGMSync->Caption = "Auto start";
		CBAStop->Caption = "Auto stop";
		CBARestart->Caption = "Auto restart";
		CBASync->Caption = "Auto resync";
		CBASlant->Caption = "Auto slant";
		CBTxFixed->Caption = "Fixed mode";
		GB2->Caption = "Level converter";
		CBCalWay->Caption = "Polynomial";
		SBTest->Caption = "Calibration";
		LJPEGQ->Caption = "Quality";
		RGSLvl->Caption = "Squelch level";
		RGSLvl->Items->Strings[0] = "Lowest";
		RGSLvl->Items->Strings[1] = "Lower";
		RGSLvl->Items->Strings[2] = "Higher";
		RGSLvl->Items->Strings[3] = "Highest";
		IDDEC->Caption = "Decode FSKID";
		IDFSK->Caption = "Encode FSKID";
		PTTLock->Caption = "Exclusive lock";

		RGLoopBack->Caption = "Loop back";
//        RGLoopBack->Items->Strings[0] = "OFF";
		RGLoopBack->Items->Strings[1] = "Internal";
		RGLoopBack->Items->Strings[2] = "External (full-duplex)";

		RGRxBPF->Items->Strings[1] = "Broad";
		RGRxBPF->Items->Strings[2] = "Sharp";
		RGRxBPF->Items->Strings[3] = "Very sharp";

		SoundPriority->Caption = "Priority";
		SoundPriority->Items->Strings[0] = "Normal";
		SoundPriority->Items->Strings[1] = "Higher";
		SoundPriority->Items->Strings[2] = "Highest";
		SoundPriority->Items->Strings[3] = "Critical";

		Source->Caption = "Source";
		Source->Items->Strings[0] = "Mono";
		Source->Items->Strings[1] = "Left";
		Source->Items->Strings[2] = "Right";
		MemWin->Caption = "Save window location";

		AppPriority->Caption = "Priority of MMSSTV";
		AppPriority->Items->Strings[0] = "Normal";
		AppPriority->Items->Strings[1] = "Higher";

		CB24->Caption = "Always use DIB";
//        AppPriority->Items->Strings[2] = "Highest";
	}
	TxFifo->Items->Assign(RxFifo->Items);
	CWIDFreq->Items->Assign(TuneFreq->Items);
	pllOutFC->Items->Assign(pllLoopFC->Items);
	crossOutFC->Items->Assign(pllLoopFC->Items);
	m_TestMode = 0;
	m_BuffCal = 1.0;

	CWave *pWave = &Mmsstv->pSound->Wave;
	pWave->GetDeviceName();
    for( int i = 0; i < pWave->m_InDevs; i++ ){
		if( !pWave->m_tInDevName[i].IsEmpty() ){
			DevNo->Items->Add(pWave->m_tInDevName[i]);
        }
    }
    for( int i = 0; i < pWave->m_OutDevs; i++ ){
		if( !pWave->m_tOutDevName[i].IsEmpty() ){
			DevNoOut->Items->Add(pWave->m_tOutDevName[i]);
        }
    }

	m_MMListW.QueryList("MMW");
	for( int i = 0; i < m_MMListW.GetCount(); i++ ){
		DevNo->Items->Add(m_MMListW.GetItemName(i));
	}
	DevNo->DropDownCount = DevNo->Items->Count;
	DevNoOut->DropDownCount = DevNoOut->Items->Count;
}
//---------------------------------------------------------------------------
int __fastcall TOptionDlg::IsMMW(LPCSTR p)
{
	if( !p ) return -1;

	for( int i = 0; i < m_MMListW.GetCount(); i++ ){
		if( !strcmp(p, m_MMListW.GetItemName(i)) ) return i;
	}
    return -1;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SetPageIndex(int n)
{
	PageIndex = n;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::UpdateUI(void)
{
	char bf[128];

	sprintf(bf, "%.2lf", m_TxSampOff);
	TxSampOff->Text = bf;
	switch(RGDemType->ItemIndex){
		case 0:
			GBPLL->Visible = TRUE;
			GBCROSS->Visible = FALSE;
			GBPLL->Enabled = TRUE;
			SetGroupEnabled(GBPLL);
			break;
		case 1:
			GBPLL->Visible = FALSE;
			GBCROSS->Visible = TRUE;
			break;
		default:
			GBPLL->Visible = TRUE;
			GBCROSS->Visible = FALSE;
			GBPLL->Enabled = FALSE;
			SetGroupEnabled(GBPLL);
			break;
	}
	switch(RGcrossType->ItemIndex){
		case 0:
			GBCOI->Enabled = TRUE;
			GBCOF->Enabled = FALSE;
			break;
		case 1:
			GBCOI->Enabled = FALSE;
			GBCOF->Enabled = TRUE;
			break;
		default:
			GBCOI->Enabled = FALSE;
			GBCOF->Enabled = FALSE;
			break;
	}
	SetGroupEnabled(GBCOI);
	SetGroupEnabled(GBCOF);
	int f = CBCalWay->Checked ? FALSE : TRUE;
	LOff->Enabled = f;
	L1500->Enabled = f;
	L2300->Enabled = f;
	Off1900->Enabled = f;
	Max1500->Enabled = f;
	Max2300->Enabled = f;

	VEdit->Visible = (RGV->ItemIndex == 1) ? TRUE : FALSE;
	CBASlant->Enabled = RGRBuf->ItemIndex ? TRUE : FALSE;
	TxBpfTap->Enabled = CBTXBPF->Checked;
	DispTxBpf->Enabled = CBTXBPF->Checked;
	TxLpfFreq->Enabled = CBTXLPF->Checked;

	f = IsMMW(AnsiString(DevNo->Text).c_str()) < 0;	//ja7ude 0428
	GB1->Enabled = f;
    SetGroupEnabled(GB1);
    Source->Enabled = f;
	Label6->Enabled = f;
    DevNoOut->Enabled = f;

    CBSTX->Visible = (Source->ItemIndex != 0);
}

int __fastcall TOptionDlg::Execute(CSSTVDEM *fp, CSSTVMOD *mp)
{
	pDem = fp;
	m_ColorChange = 0;

	RGDemType->ItemIndex = fp->m_Type;
	char bf[128];
	sprintf(bf, "%.2lf", sys.m_SampFreq);
	EditSamp->Text = bf;
	m_TxSampOff = sys.m_TxSampOff;
	WinFont->Text = sys.m_WinFontName;
	m_WinFontCharset = sys.m_WinFontCharset;
	m_WinFontStyle = sys.m_WinFontStyle;
	MemWin->Checked = sys.m_MemWindow;
	FontAdj->Text = sys.m_FontAdjSize;

	CBDiff->Checked = sys.m_Differentiator;
	TBDiff->Position = ((sys.m_DiffLevelP * 10) + 0.5);
	pllVCOGain->Text = fp->m_pll.m_vcogain;
	pllLoopOrder->Text = fp->m_pll.m_loopOrder;
	pllLoopFC->Text = fp->m_pll.m_loopFC;
	pllOutOrder->Text = fp->m_pll.m_outOrder;
	pllOutFC->Text = fp->m_pll.m_outFC;

	RGcrossType->ItemIndex = fp->m_fqc.m_Type;
	crossSmooz->Text = fp->m_fqc.m_SmoozFq;
	crossOutOrder->Text = fp->m_fqc.m_outOrder;
	crossOutFC->Text = fp->m_fqc.m_outFC;

	RGRxBPF->ItemIndex = fp->m_bpf;
	RGSLvl->ItemIndex = fp->m_SenseLvl;

	PCLow->Color = sys.m_ColorLow;
	PCHigh->Color = sys.m_ColorHigh;
	PCFFTB->Color = sys.m_ColorFFTB;
	PCFFT->Color = sys.m_ColorFFT;
	PCFFTStg->Color = sys.m_ColorFFTStg;
	PCSync->Color = sys.m_ColorFFTSync;
	PCFreq->Color = sys.m_ColorFFTFreq;

	PortName->Text = sys.m_TxRxName;
	PTTLock->Checked = sys.m_TxRxLock;
	CBRTS->Checked = sys.m_RTSonRX;

	EditCall->Text = sys.m_Call;
	RGLoopBack->ItemIndex = sys.m_echo;

	CBTXBPF->Checked = mp->m_bpf;
	CBTXLPF->Checked = mp->m_lpf;
	TxLpfFreq->Text = mp->m_lpffq;

	TxBpfTap->Text = mp->m_bpftap;

	TBOutLvl->Position = int(mp->m_outgain*64.0/30000.0);
	m_OutGain = Mmsstv->pMod->m_outgain;

	RxFifo->Text = sys.m_SoundFifoRX;
	TxFifo->Text = sys.m_SoundFifoTX;
	SoundPriority->ItemIndex = sys.m_SoundPriority;

	{
	   	AnsiString as = sys.m_SoundDevice.c_str();
		LPSTR p, t;

		p = StrDlm(t, as.c_str());
		int dd;
		if( ATOI(dd, t) ){
			switch(dd){
				case -1:
                    DevNo->Text = "Default";
                	break;
                case -2:
                	break;
                default:
					DevNo->Text = t;
                	break;
            }
        }
        else {
			DevNo->Text = t;
        }

		p = SkipSpace(p);
        if( *p ) StrDlm(t, p);

		if( ATOI(dd, t) ){
			switch(dd){
				case -1:
                    DevNoOut->Text = "Default";
                	break;
                case -2:
                	break;
                default:
					DevNoOut->Text = t;
                	break;
            }
        }
        else if( IsMMW(t) < 0 ){
			DevNoOut->Text = t;
        }
        else {
			DevNoOut->Text = "";
        }
	}

	Source->ItemIndex = sys.m_SoundStereo;
	CBSTX->Checked = sys.m_StereoTX;
	RGMSync->ItemIndex = fp->m_MSync;
	CBAStop->Checked = sys.m_AutoStop;
	CBARestart->Checked = fp->m_SyncRestart;
	CBASync->Checked = sys.m_AutoSync;
	CBASlant->Checked = Mmsstv->KRSA->Checked;
	JPEGQ->Text = sys.m_JPEGQuality;
	CBVari->Checked = mp->m_VariOut;
	CB24->Checked = sys.m_UseB24;

	if( (PageIndex >= 0) && (PageIndex < Page->PageCount) ){
		if( Page->Pages[PageIndex]->TabVisible == FALSE ){
			PageIndex = 0;
		}
		Page->ActivePage = Page->Pages[PageIndex];
	}
	Off1900->Text = sys.m_DemOff;
	if( sys.m_DemBlack > 0.000001 ){
		Max1500->Text = int((128 / sys.m_DemBlack)+0.5);
	}
	else {
		Max1500->Text = 16384;
	}
	if( sys.m_DemWhite > 0.000001 ){
		Max2300->Text = -int((128 / sys.m_DemWhite)+0.5);
	}
	else {
		Max2300->Text = 16384;
	}
	CBCalWay->Checked = sys.m_DemCalibration;

	HistMax->Text = sys.m_HistMax;
	RGRBuf->ItemIndex = sys.m_UseRxBuff;
	TuneFreq->Text = mp->m_TuneFreq;
	TrigTXTime->Text = sys.m_TuneTXTime;
	CBAT->Checked = sys.m_TuneSat;

	switch(sys.m_CWID){
		case 0:
			IDOFF->Checked = TRUE;
			break;
		case 1:
			IDCW->Checked = TRUE;
			break;
		case 2:
			IDMMV->Checked = TRUE;
			break;
	}
	TBCW->Position = sys.m_CWIDWPM - 10;
	CWIDFreq->Text = sys.m_CWIDFreq;
	CWIDText->Text = sys.m_CWIDText;
	MMVID->Text = sys.m_MMVID;
	IDOFFClick(NULL);

	IDDEC->Checked = fp->m_fskdecode;
	IDFSK->Checked = sys.m_TXFSKID;
	CBTxFixed->Checked = sys.m_FixedTxMode;

	RGV->ItemIndex = sys.m_VOX;
	m_VOXSound = sys.m_VOXSound.c_str();

	AppPriority->ItemIndex = sys.m_Priority;

	UpdateUI();
	m_DisEvent = 0;
	int r = ShowModal();
	if( sys.m_TestDem ){
		mp->m_outgain = m_OutGain;
		sys.m_TestDem = 0;
		mp->m_vco.SetSampleFreq(sys.m_SampFreq+sys.m_TxSampOff);
		fp->m_SyncMode = 0;
	}
	if( r == IDOK ){
		double d;
		int dd;

		sys.m_UseB24 = CB24->Checked;

		sscanf(AnsiString(EditSamp->Text).c_str(), "%lf", &d);	//ja7ude 0428
		if( (d >= 5000.0) && (d <= CLOCKMAX) ){
			sys.m_SampFreq = d;
		}
		sys.m_TxSampOff = m_TxSampOff;

		sys.m_WinFontName = WinFont->Text;
		sys.m_WinFontCharset = m_WinFontCharset;
		sys.m_WinFontStyle = m_WinFontStyle;

		if( sscanf(AnsiString(FontAdj->Text).c_str(), "%ld", &dd) == 1 ){	//ja7ude 0428
			sys.m_FontAdjSize = dd;
		}

		sys.m_echo = RGLoopBack->ItemIndex;


		sys.m_RTSonRX = CBRTS->Checked;
		sys.m_TxRxLock = PTTLock->Checked;
		sys.m_TxRxName = PortName->Text;
		if( (PortName->Text != "NONE") && !strcmp(AnsiString(PortName->Text).c_str(), RADIO.StrPort) ){	//ja7ude 0428
			strcpy(RADIO.StrPort, "NONE");
		}

		char bf[MLCALL+1];
		StrCopy(bf, AnsiString(EditCall->Text).c_str(), MLCALL);	//ja7ude 0428
		jstrupr(bf); clipsp(bf);
		sys.m_Call = SkipSpace(bf);

		mp->m_bpf = CBTXBPF->Checked;
		mp->m_lpf = CBTXLPF->Checked;
		sscanf(AnsiString(TxLpfFreq->Text).c_str(), "%lf", &d);	//ja7ude 0428
		if( (d >= 100.0) && (d <= 3000.0) ){
			mp->m_lpffq = d;
		}
		sscanf(AnsiString(TxBpfTap->Text).c_str(), "%u", &dd);	//ja7ude 0428
		if( (dd >= 2) && (dd <= TAPMAX) ){
			mp->m_bpftap = dd;
		}
		mp->CalcFilter();
		mp->m_outgain = TBOutLvl->Position*30000.0/64.0;
		if( mp->m_outgain >= 30000 ) mp->m_outgain = 30000;
		mp->InitGain();
		mp->m_VariOut = CBVari->Checked;

		if( sscanf(AnsiString(RxFifo->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0428
			if( (dd >= 4) && (dd <= WAVE_FIFO_MAX) ){
				sys.m_SoundFifoRX = dd;
			}
		}
		if( sscanf(AnsiString(TxFifo->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0428
			if( (dd >= 4) && (dd <= WAVE_FIFO_MAX) ){
				sys.m_SoundFifoTX = dd;
			}
		}
		sys.m_SoundPriority = SoundPriority->ItemIndex;
		AnsiString in = DevNo->Text.c_str();
		if( !strcmpi(in.c_str(), "default") ) in = "-1";
		AnsiString out = DevNoOut->Text.c_str();
		if( !strcmpi(out.c_str(), "default") ) out = "-1";
		if( in != out ){
			AnsiString as;
			if( IsMMW(in.c_str()) < 0 ){
				as += "\x22";
                as += in;
                as += "\x22,\x22";
                as += out;
                as += "\x22";
            }
            else {
				as += "\x22";
                as += in;
                as += "\x22";
            }
            sys.m_SoundDevice = as;
		}
        else {
			AnsiString as;
			as += "\x22";
            as += in;
            as += "\x22";
			sys.m_SoundDevice = as;
        }

		sys.m_SoundStereo = Source->ItemIndex;
		sys.m_StereoTX = CBSTX->Checked;

		sys.m_Differentiator = CBDiff->Checked;
		sys.m_DiffLevelP = double(TBDiff->Position)/10.0;
		sys.m_DiffLevelM = sys.m_DiffLevelP / 3.0;
		if( sscanf(AnsiString(pllVCOGain->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0428
			if( d > 0.0 ) fp->m_pll.SetVcoGain(d);
		}
		if( sscanf(AnsiString(pllLoopOrder->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0428
			if( (dd > 0) && (dd <= 32) ) fp->m_pll.m_loopOrder = dd;
		}
		if( sscanf(AnsiString(pllLoopFC->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0428
			if( d > 0.0 ) fp->m_pll.m_loopFC = d;
		}
		if( sscanf(AnsiString(pllOutOrder->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0428
			if( (dd > 0) && (dd <= 32) ) fp->m_pll.m_outOrder = dd;
		}
		if( sscanf(AnsiString(pllOutFC->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0428
			if( d > 0.0 ) fp->m_pll.m_outFC = d;
		}
		fp->m_pll.MakeLoopLPF();
		fp->m_pll.MakeOutLPF();

		if( sscanf(AnsiString(crossOutOrder->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0428
			if( (dd > 0) && (dd <= 32) ) fp->m_fqc.m_outOrder = dd;
		}
		if( sscanf(AnsiString(crossOutFC->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0428
			if( d > 0.0 ) fp->m_fqc.m_outFC = d;
		}
		fp->m_fqc.m_Type = RGcrossType->ItemIndex;
		if( sscanf(AnsiString(crossSmooz->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0428
			if( (d >= 500.0) && (d <= 8000.0) ){
				fp->m_fqc.m_SmoozFq = d;
			}
		}
		fp->m_fqc.CalcLPF();
		fp->m_Type = RGDemType->ItemIndex;

		if( sscanf(AnsiString(Off1900->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0428
			sys.m_DemOff = dd;
		}
		if( sscanf(AnsiString(Max1500->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0428
			if( d > 0.000001 ){
				sys.m_DemBlack = 128.0 / d;
			}
		}
		if( sscanf(AnsiString(Max2300->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0428
			d = -d;
			if( d > 0.000001 ){
				sys.m_DemWhite = 128.0 / d;
			}
		}
		sys.m_DemCalibration = CBCalWay->Checked;

		if( sscanf(AnsiString(HistMax->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0428
			if( (dd > 1) && (dd <= 256) ){
				sys.m_HistMax = dd;
			}
		}
		if( sscanf(AnsiString(TuneFreq->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0428
			if( (dd >= 100) && (dd <= 3000) ){
				mp->m_TuneFreq = dd;
			}
		}
		if( sscanf(AnsiString(TrigTXTime->Text).c_str(), "%d", &dd) == 1 ){	//ja7ude 0428
			sys.m_TuneTXTime = dd;
		}
		sys.m_TuneSat = CBAT->Checked;

		sys.m_UseRxBuff = RGRBuf->ItemIndex;
		fp->OpenCloseRxBuff();
		sys.m_ColorLow = PCLow->Color;
		sys.m_ColorHigh = PCHigh->Color;
		sys.m_ColorFFTB = PCFFTB->Color;
		sys.m_ColorFFT = PCFFT->Color;
		sys.m_ColorFFTStg = PCFFTStg->Color;
		sys.m_ColorFFTSync = PCSync->Color;
		sys.m_ColorFFTFreq = PCFreq->Color;

		if( IDCW->Checked ){
			sys.m_CWID = 1;
		}
		else if( IDMMV->Checked ){
			sys.m_CWID = 2;
		}
		else {
			sys.m_CWID = 0;
		}
		sys.m_CWIDWPM = TBCW->Position + 10;
		sys.m_CWIDText = CWIDText->Text.c_str();
		sys.m_MMVID = MMVID->Text.c_str();
		if( sscanf(AnsiString(CWIDFreq->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0521
			sys.m_CWIDFreq = dd;
		}
		sys.m_TXFSKID = IDFSK->Checked;
		fp->m_fskdecode = IDDEC->Checked;

		if( sscanf(AnsiString(JPEGQ->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0521
			if( (dd >= 0) && (dd <= 100) ){
				sys.m_JPEGQuality = dd;
			}
		}

		fp->SetBPF(RGRxBPF->ItemIndex);
		fp->m_MSync = RGMSync->ItemIndex;
		fp->m_SyncRestart = CBARestart->Checked;
		fp->m_SenseLvl = RGSLvl->ItemIndex;
		fp->SetSenseLvl();
		Mmsstv->KRSA->Checked = CBASlant->Checked;

		sys.m_MemWindow = MemWin->Checked;
		sys.m_AutoStop = CBAStop->Checked;
		sys.m_AutoSync = CBASync->Checked;
		sys.m_FixedTxMode = CBTxFixed->Checked;

		sys.m_VOX = RGV->ItemIndex;
		sys.m_VOXSound = m_VOXSound.c_str();

		sys.m_Priority = AppPriority->ItemIndex;
	}
	else {
		r = FALSE;
	}
	PageIndex = GetActiveIndex(Page);
	return r;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::DispTxBpfClick(TObject *Sender)
{
	double HBPF[TAPMAX+1];

	int	   tap;
	sscanf(AnsiString(TxBpfTap->Text).c_str(), "%u", &tap);	//ja7ude 0521
	if( !tap ) tap = 2;

	int lfq = 700 + g_dblToneOffset;
    int hfq = 2700 + g_dblToneOffset;
    if( lfq < 100 ){
		MakeFilter(HBPF, tap, ffLPF, SampFreq, hfq, hfq, 40, 1.0);
    }
    else {
		MakeFilter(HBPF, tap, ffBPF, SampFreq, lfq, hfq, 40, 1.0);
    }

	TFreqDispDlg *pBox = new TFreqDispDlg(this);
	pBox->Execute(HBPF, tap, 1);
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::DispLoopLPFClick(TObject *Sender)
{
	int order;
	double fc;

	sscanf(AnsiString(pllLoopFC->Text).c_str(), "%lf", &fc);	//ja7ude 0521
	if( fc > 0.0 ){
		sscanf(AnsiString(pllLoopOrder->Text).c_str(), "%u", &order);	//ja7ude 0521
		if( (order >= 1) && (order <= 32) ){
			CIIR	iir;
			iir.MakeIIR(fc, SampFreq, order, 0, 1.0);
			TFreqDispDlg *pBox = new TFreqDispDlg(this);
			pBox->Execute(&iir, DemOver+1, 4000);
			delete pBox;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::DispOutLPFClick(TObject *Sender)
{
	int order;
	double fc;

	sscanf(AnsiString(pllOutFC->Text).c_str(), "%lf", &fc);	//ja7ude 0521
	if( fc > 0.0 ){
		sscanf(AnsiString(pllOutOrder->Text).c_str(), "%u", &order);	//ja7ude 0521
		if( (order >= 1) && (order <= 32) ){
			CIIR	iir;
			iir.MakeIIR(fc, SampFreq, order, 0, 1.0);
			TFreqDispDlg *pBox = new TFreqDispDlg(this);
			pBox->Execute(&iir, DemOver+1, 4000);
			delete pBox;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::DispCrossOutLPFClick(TObject *Sender)
{
	int order;
	double fc;

	sscanf(AnsiString(crossOutFC->Text).c_str(), "%lf", &fc);	//ja7ude 0521
	if( fc > 0.0 ){
		sscanf(AnsiString(crossOutOrder->Text).c_str(), "%u", &order);	//ja7ude 0521
		if( (order >= 1) && (order <= 32) ){
			CIIR	iir;
			iir.MakeIIR(fc, SampFreq, order, 0, 1.0);
			TFreqDispDlg *pBox = new TFreqDispDlg(this);
			pBox->Execute(&iir, DemOver+1, 4000);
			delete pBox;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SetCustomColor(void)
{
	TColorDialog *pDialog = Mmsstv->ColorDialog;
	InitCustomColor(pDialog);
	AddCustomColor(pDialog, PCLow->Color);
	AddCustomColor(pDialog, PCHigh->Color);
	AddCustomColor(pDialog, PCFFTB->Color);
	AddCustomColor(pDialog, PCFFT->Color);
	AddCustomColor(pDialog, PCFFTStg->Color);
	AddCustomColor(pDialog, PCSync->Color);
	AddCustomColor(pDialog, PCFreq->Color);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PortNameChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	COMM.change = 1;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::WinFontBtnClick(TObject *Sender)
{
	TFontDialog *pDialog = Mmsstv->FontDialog;
	pDialog->Font->Name = WinFont->Text;
	pDialog->Font->Charset = m_WinFontCharset;
	TFontStyles fs = Code2FontStyle(m_WinFontStyle);
	pDialog->Font->Style = fs;
	NormalWindow(this);
	SetDisPaint();
	if( pDialog->Execute() == TRUE ){
		WinFont->Text = pDialog->Font->Name;
		m_WinFontCharset = pDialog->Font->Charset;
		fs = pDialog->Font->Style;
		m_WinFontStyle = FontStyle2Code(fs);
	}
	ResDisPaint();
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::JaBtnClick(TObject *Sender)
{
	WinFont->Text = "ＭＳ Ｐゴシック";
	m_WinFontCharset = SHIFTJIS_CHARSET;
	m_WinFontStyle = 0;
	if( MsgEng ){
		Application->MessageBox(L"MMSSTVを再起動すると日本語モードになります.\r\n(Please restart MMSSTV for Japanese mode)", L"MMSSTV", MB_ICONINFORMATION|MB_OK);	//ja7ude 0521
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::EngBtnClick(TObject *Sender)
{
	WinFont->Text = "Times New Roman";
	m_WinFontCharset = ANSI_CHARSET;
	m_WinFontStyle = 0;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::RadioBtnClick(TObject *Sender)
{
	// ラジオコントロール
	TRADIOSetDlg *pBox = new TRADIOSetDlg(this);

	if( (PortName->Text != "NONE") && !strcmp(AnsiString(PortName->Text).c_str(), RADIO.StrPort) ){	//ja7ude 0521
		strcpy(RADIO.StrPort, "NONE");
		RADIO.change = 1;
	}
	if( pBox->Execute() == TRUE ){
		if( (PortName->Text != "NONE") && !strcmp(AnsiString(PortName->Text).c_str(), RADIO.StrPort) ){	//ja7ude 0521
			PortName->Text = "NONE";
		}
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SBClockAdjClick(TObject *Sender)
{
	double d;
	sscanf(AnsiString(EditSamp->Text).c_str(), "%lf", &d);
	if( (d < 5000.0) || (d > CLOCKMAX) ){
		d = sys.m_SampFreq;
	}

	TClockAdjDlg *pBox = new TClockAdjDlg(this);
	if( pBox->Execute(Mmsstv->pSound, d) == TRUE ){
		char bf[128];
		sprintf(bf, "%.2lf", d);
		EditSamp->Text = bf;
	}
	delete pBox;
}
#if 0
///----------------------------------------------------------------
///  多項式計算を行う
void MakeTeira(double *T, double *X, double *Y, int N)
{
	if( N > 17 ) N = 17;
	double	Z[20][20];

	int		i, i0, i1, i2, i3;
	int		n0, n1;
	double	w;

	for( i = 0; i < N; i++ ){
		Z[i][0] = 1;
	}
	for( i1 = 0; i1 < N; i1++ ){
		for( i2 = 1; i2 < N; i2++ ){
			w = 1;
			for( i3 = 0; i3 < i2; i3++ ){
				w = w * X[i1];
			}
			Z[i1][i2] = w;
		}
	}
	for( i1 = 0; i1 < N; i1++ ){
		w = 0;
		for( i2 = i1; i2 < N; i2++ ){
			if( ABS(Z[i2][i1]) >= w ){
				w = ABS(Z[i2][i1]);
				i0 = i2;
			}
		}
		if( i0 != N ){
			for( i2 = i1; i2 < N; i2++ ){
				w = Z[i1][i2];
				Z[i1][i2] = Z[i0][i2];
				Z[i0][i2] = w;
			}
			w = Y[i1];
			Y[i1] = Y[i0];
			Y[i0] = w;
		}
		w = Z[i1][i1];
		if( w ){
			for( i2 = i1; i2 < N; i2++ ){
				Z[i1][i2] = Z[i1][i2]/w;
			}
			Y[i1] = Y[i1]/w;
		}
		if( i1 == N ) break;
		n1 = i1 + 1;
		for( i2 = n1; i2 < N; i2++ ){
			w = Z[i2][i1];
			for( i3 = n1; i3 < N; i3++ ){
				Z[i2][i3] = Z[i2][i3] - w*Z[i1][i3];
			}
			Y[i2] = Y[i2] - w * Y[i1];
		}
	}
	if( N > 1 ){
		for( n1 = N - 1; n1 >= 1; n1-- ){
			n0 = n1 - 1;
			for( i2 = n1; i2 < N; i2++ ){
				Y[n0] = Y[n0] - Y[i2] * Z[n0][i2];
			}
		}
	}
	for( i = 0; i < N; i++ ){
		T[i] = Y[i];
	}
}
#endif
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::TimerTimer(TObject *Sender)
{
	if( SBTest->Down ){
		int d = Mmsstv->pDem->m_CurSig;
		switch(m_TestMode){
			case 0:
				m_TestCnt--;
				if( !m_TestCnt ){
					m_TestMode++;
					m_TestCnt = 30 * m_BuffCal;
					m_Avg.SetCount(30);
				}
				break;
			case 1:             // 1900Hzの計測
				m_Off = int(m_Avg.Avg(d) + 0.5);
				Off1900->Text = m_Off;
				m_TestCnt--;
				if( !m_TestCnt ){
					m_TestMode++;
					m_TestCnt = 5 * m_BuffCal;
					sys.m_TestDem = 1500;
				}
				break;
			case 2:
				m_TestCnt--;
				if( !m_TestCnt ){
					m_TestMode++;
					m_TestCnt = 30 * m_BuffCal;
					m_Avg.SetCount(30);
				}
				break;
			case 3:             // 1500Hzの計測
				m_Max1500 = int(m_Avg.Avg(d) + 0.5) - m_Off;
				Max1500->Text = m_Max1500;
				m_TestCnt--;
				if( !m_TestCnt ){
					m_TestMode++;
					m_TestCnt = 5 * m_BuffCal;
					sys.m_TestDem = 2300;
				}
				break;
			case 4:
				m_TestCnt--;
				if( !m_TestCnt ){
					m_TestMode++;
					m_TestCnt = 30 * m_BuffCal;
					m_Avg.SetCount(30);
				}
				break;
			case 5:             // 2300Hzの計測
				m_Max2300 = int(m_Avg.Avg(d) + 0.5) - m_Off;
				Max2300->Text = m_Max2300;
				m_TestCnt--;
				if( !m_TestCnt ){
					m_TestMode++;
					sys.m_TestDem = 1500;
					m_TestCnt = 2 * m_BuffCal;
				}
				break;
			case 6:
				m_TestCnt--;
				if( !m_TestCnt ){
					m_TestMode++;
					m_TestCnt = 5 * m_BuffCal;
					m_Avg.SetCount(3);
					m_SubCnt = 0;
				}
				break;
			case 7:
				m_DataList[m_SubCnt] = m_Avg.Avg(d - m_Off);
				sys.m_Dem17[m_SubCnt] = -m_DataList[m_SubCnt];
				m_TestCnt--;
				if( !m_TestCnt ){
					m_TestCnt = 5 * m_BuffCal;
					m_SubCnt++;
					if( m_SubCnt > 16 ){
						m_TestMode++;
					}
					else {
						sys.m_TestDem += 50;
					}
				}
				break;
			case 8:
				m_TestCnt--;
				if( !m_TestCnt ){
					Timer->Enabled = FALSE;
					sys.m_TestDem = 0;
					SBTest->Down = 0;
					Mmsstv->pDem->m_SyncMode = 0;
					Mmsstv->pMod->m_outgain = m_OutGain;
					Screen->Cursor = crDefault;
					Page->Enabled = TRUE;
					OKBtn->Enabled = TRUE;

					TLinearDspDlg *pBox = new TLinearDspDlg(this);
					pBox->Execute(m_DataList);
					delete pBox;
					ModalResult = mrOk;
				}
				break;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SBTestClick(TObject *Sender)
{
	if( SBTest->Down ){     // テストの開始
		if( Mmsstv->SBTX->Down || Mmsstv->SBTune->Down ) Mmsstv->ToRX();
		double  d;
		int     dd;
		if( sscanf(AnsiString(pllVCOGain->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0521
			if( d > 0.0 ) pDem->m_pll.SetVcoGain(d);
		}
		if( sscanf(AnsiString(pllLoopOrder->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0521
			if( (dd > 0) && (dd < 32) ) pDem->m_pll.m_loopOrder = dd;
		}
		if( sscanf(AnsiString(pllLoopFC->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0521
			if( d > 0.0 ) pDem->m_pll.m_loopFC = d;
		}
		if( sscanf(AnsiString(pllOutOrder->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0521
			if( (dd > 0) && (dd < 32) ) pDem->m_pll.m_outOrder = dd;
		}
		if( sscanf(AnsiString(pllOutFC->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0521
			if( d > 0.0 ) pDem->m_pll.m_outFC = d;
		}
		pDem->m_pll.MakeLoopLPF();
		pDem->m_pll.MakeOutLPF();
		if( sscanf(AnsiString(crossOutOrder->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0521
			if( (dd > 0) && (dd < 32) ) pDem->m_fqc.m_outOrder = dd;
		}
		if( sscanf(AnsiString(crossOutFC->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0521
			if( d > 0.0 ) pDem->m_fqc.m_outFC = d;
		}
		pDem->m_fqc.CalcLPF();
		pDem->m_Type = RGDemType->ItemIndex;

		m_BuffCal = (Mmsstv->pSound->m_BuffSize / SampFreq) / (2048.0/11025.0);
		m_TestMode = 0;
		m_TestCnt = 5;
		Mmsstv->pDem->Stop();
		Mmsstv->pDem->SetWidth(0);
		Mmsstv->pDem->m_SyncMode = 255;
		Mmsstv->pMod->m_outgain = 8192.0;
		Mmsstv->pMod->m_vco.SetSampleFreq(sys.m_SampFreq);
		sys.m_TestDem = 1900;
		Timer->Enabled = TRUE;
		Screen->Cursor = crHourGlass;
		OKBtn->Enabled = FALSE;
		Page->Enabled = FALSE;
	}
}
//---------------------------------------------------------------------------

void __fastcall TOptionDlg::RxBpfBtnClick(TObject *Sender)
{
	if( !RGRxBPF->ItemIndex ) return;

	double HBPF1[TAPMAX+1];
	double HBPF2[TAPMAX+1];
	double HBPF3[TAPMAX+1];

	int	   tap;
	pDem->CalcBPF(HBPF1, HBPF2, HBPF3, tap, RGRxBPF->ItemIndex, SSTVSET.m_TxMode);

	TFreqDispDlg *pBox = new TFreqDispDlg(this);
	pBox->Execute(IsNarrowMode(SSTVSET.m_TxMode) ? HBPF3 : HBPF1, tap, 1);
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PCFFTClick(TObject *Sender)
{
	if( m_DisEvent ) return;
	TPanel *tp = (TPanel *)Sender;

	TColorDialog *pDialog = Mmsstv->ColorDialog;
	pDialog->Color = tp->Color;
	SetCustomColor();
	NormalWindow(this);
	SetDisPaint();
	if( pDialog->Execute() == TRUE ){
		tp->Color = pDialog->Color;
		m_ColorChange = 1;
	}
	ResDisPaint();
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::UDTxSampClick(TObject *Sender,
	  TUDBtnType Button)
{
	if( m_DisEvent ) return;
	if( Button == btNext ){
		m_TxSampOff += 0.02;
	}
	else {
		m_TxSampOff -= 0.02;
	}
	if( m_TxSampOff < 0.0 ){
		m_TxSampOff = -m_TxSampOff;
		m_TxSampOff = NormalSampFreq(m_TxSampOff, 50);
		m_TxSampOff = -m_TxSampOff;
	}
	else {
		m_TxSampOff = NormalSampFreq(m_TxSampOff, 50);
	}
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::CWIDMacClick(TObject *Sender)
{
	if( m_DisEvent ) return;
	if( IDMMV->Checked ){
		TOpenDialog *pDialog = Mmsstv->OpenDialog;
		pDialog->Options >> ofCreatePrompt;
		pDialog->Options << ofFileMustExist;
		if( MsgEng ){
			pDialog->Title = "Choose MMSSTV Sound file";
			pDialog->Filter = "MMSSTV Sound Files(*.mmv)|*.mmv|";
		}
		else {
			pDialog->Title = "MMSSTVサウンドファイルの選択";
			pDialog->Filter = "MMSSTVサウンドファイル(*.mmv)|*.mmv|";
		}
		pDialog->FileName = "";
		pDialog->DefaultExt = "mmv";
		pDialog->InitialDir = BgnDir;
		SetDisPaint();
		NormalWindow(this);
		if( Mmsstv->OpenDialogExecute(FALSE) == TRUE ){
			MMVID->Text = pDialog->FileName.c_str();
		}
		ResDisPaint();
		TopWindow(this);
	}
	else {
		TMacroKeyDlg *pBox = new TMacroKeyDlg(this);
		AnsiString as = CWIDText->Text;
		if( pBox->Execute(as) > 0 ){
			CWIDText->SetFocus();
			CWIDText->SelStart = strlen(AnsiString(CWIDText->Text).c_str());	//ja7ude 0521
			CWIDText->SelLength = 0;
			for( LPCSTR p = as.c_str(); *p; p++ ){
				::PostMessage(CWIDText->Handle, WM_CHAR, *p, 0);
			}
		}
		delete pBox;
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::RGDemTypeClick(TObject *Sender)
{
	if( m_DisEvent ) return;
	CBCalWay->Checked = ((RGDemType->ItemIndex == 1) && (SampBase < 15000.0)) ? TRUE : FALSE;
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::TxSampOffChange(TObject *Sender)
{
	if( m_DisEvent ) return;
	double d;

	if( sscanf(AnsiString(TxSampOff->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0521
		if( (d >= -1500.0) && (d <= 1500.0 ) ){
			m_TxSampOff = d;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::CBCalWayClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::IDOFFClick(TObject *Sender)
{
	if( m_DisEvent ) return;
	if( IDMMV->Checked ){
		L7->Visible = FALSE;
		L8->Visible = FALSE;
		TBCW->Visible = FALSE;
		CWIDFreq->Visible = FALSE;
		L15->Visible = FALSE;
		CWIDText->Visible = FALSE;
		MMVID->Visible = TRUE;
		CWIDMac->Caption = "Ref";
	}
	else {
		L7->Visible = TRUE;
		L8->Visible = TRUE;
		TBCW->Visible = TRUE;
		CWIDFreq->Visible = TRUE;
		L15->Visible = TRUE;
		CWIDText->Visible = TRUE;
		MMVID->Visible = FALSE;
		CWIDMac->Caption = "Macro";
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::VEditClick(TObject *Sender)
{
	TTextEditDlg *pBox = new TTextEditDlg(this);
	pBox->Execute(m_VOXSound, TRUE, "VOX tone  freq(Hz), time(ms), ...");
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::CBATClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	if( CBAT->Checked ){
		int dd = 0;
		sscanf(AnsiString(TrigTXTime->Text).c_str(), "%d", &dd);	//ja7ude 0521
		if( dd < 0 ) TrigTXTime->Text = 5;
		TuneFreq->Text = 1200;
	}
	else {
		TuneFreq->Text = 1750;
		TrigTXTime->Text = -1;
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PortNameDropDown(TObject *Sender)
{
	if( m_DisEvent ) return;

    if( !m_MMList.IsQuery() ){
		m_MMList.QueryList("FSK");
		for( int i = 0; i < m_MMList.GetCount(); i++ ){
			PortName->Items->Add(m_MMList.GetItemName(i));
		}
		PortName->DropDownCount = m_MMList.GetCount() + 18;
    }
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SourceClick(TObject *Sender)
{
	UpdateUI();	
}
//---------------------------------------------------------------------------
// RxBPF=Most, TxBPF=24
// Value = 7170  Celeron 1.06GHz 2003/10/22
void __fastcall TOptionDlg::SBTestMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		CWaitCursor w;
        TSound *pSound = Mmsstv->pSound;
        if( pSound->m_ReqSpeedTest ) return;
		pSound->m_ReqSpeedTest = 1;
        while(pSound->m_ReqSpeedTest) ::Sleep(100);
        InfoMB("Speed coefficient=%u (Sample=5117)", pSound->m_SpeedValue);
    }
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::DevNoChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	m_DisEvent++;
	if( IsMMW(AnsiString(DevNo->Text).c_str()) >= 0 ){	//ja7ude 0521
		DevNoOut->Text = "";
    }
    else if( IsMMW(AnsiString(DevNoOut->Text).c_str()) >= 0 ){	//ja7ude 0521
		DevNoOut->Text = "-1";
    }
	m_DisEvent--;
	CBCalWayClick(Sender);
}
//---------------------------------------------------------------------------
