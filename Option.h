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



//----------------------------------------------------------------------------
#ifndef OptionH
#define OptionH
//----------------------------------------------------------------------------
	//ja7ude 0428
#include <System.hpp>
#include <Windows.hpp>
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <Graphics.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Controls.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>

#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include "sstv.h"
#include "MMLink.h"
//----------------------------------------------------------------------------
class TOptionDlg : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TPageControl *Page;
	TTabSheet *TabTx;
	TGroupBox *GPTT;
	TComboBox *PortName;
	TLabel *Label13;
	TGroupBox *GroupBox10;
	TCheckBox *CBTXBPF;
	TLabel *Label25;
	TComboBox *TxBpfTap;
	TTabSheet *TabMisc;
	TCheckBox *CBTXLPF;
	TComboBox *TxLpfFreq;
	TLabel *Label28;
	TLabel *Label29;
	TSpeedButton *DispTxBpf;
	TTabSheet *TabDem;
	TGroupBox *GBPLL;
	TLabel *Label1;
	TGroupBox *GroupBox17;
	TLabel *LLoopOrder;
	TComboBox *pllVCOGain;
	TComboBox *pllLoopOrder;
	TLabel *Label3;
	TComboBox *pllLoopFC;
	TLabel *Label4;
	TGroupBox *GroupBox18;
	TLabel *LOutOrder;
	TLabel *Label44;
	TLabel *Label45;
	TComboBox *pllOutOrder;
	TComboBox *pllOutFC;
	TGroupBox *GSFont;
	TEdit *WinFont;
	TButton *WinFontBtn;
	TLabel *Label46;
	TButton *JaBtn;
	TButton *EngBtn;
	TComboBox *FontAdj;
	TSpeedButton *DispLoopLPF;
	TSpeedButton *DispOutLPF;
	TGroupBox *GrpCard;
	TRadioGroup *SoundPriority;
	TGroupBox *GroupBox3;
	TSpeedButton *SBClockAdj;
	TLabel *Label5;
	TCheckBox *MemWin;
	TComboBox *DevNo;
	TLabel *Label11;
	TButton *RadioBtn;
	TComboBox *EditSamp;
	TRadioGroup *Source;
	TGroupBox *GBW;
	TLabel *Label37;
	TLabel *Label38;
	TPanel *PCLow;
	TPanel *PCHigh;
	TGroupBox *GBOut;
	TTrackBar *TBOutLvl;
	TGroupBox *GBTemp;
	TEdit *EditCall;
	TLabel *Label2;
	TTimer *Timer;
	TGroupBox *GB2;
	TSpeedButton *SBTest;
	TEdit *Max1500;
	TEdit *Max2300;
	TEdit *Off1900;
	TLabel *LOff;
	TLabel *L1500;
	TLabel *L2300;
	TGroupBox *GBHMax;
	TComboBox *HistMax;
	TRadioGroup *RGRBuf;
	TRadioGroup *RGMSync;
	TRadioGroup *RGRxBPF;
	TButton *RxBpfBtn;
	TUpDown *UDTxSamp;
	TRadioGroup *RGLoopBack;
	TGroupBox *GB4;
	TCheckBox *CBAStop;
	TCheckBox *CBARestart;
	TGroupBox *GroupBox4;
	TLabel *Label9;
	TComboBox *TuneFreq;
	TLabel *Label10;
	TComboBox *TrigTXTime;
	TLabel *Label12;
	TLabel *Label14;
	TGroupBox *GB5;
	TEdit *CWIDText;
	TComboBox *CWIDFreq;
	TLabel *L15;
	TButton *CWIDMac;
	TGroupBox *GB6;
	TComboBox *JPEGQ;
	TLabel *Label16;
	TLabel *LJPEGQ;
	TGroupBox *GB7;
	TCheckBox *CBTxFixed;
	TRadioGroup *RGSLvl;
	TRadioGroup *RGDemType;
	TGroupBox *GBCROSS;
	TGroupBox *GBCOI;
	TLabel *LCORDER;
	TLabel *Label18;
	TLabel *Label19;
	TSpeedButton *DispCrossOutLPF;
	TComboBox *crossOutOrder;
	TComboBox *crossOutFC;
	TCheckBox *CBASlant;
	TRadioGroup *RGcrossType;
	TGroupBox *GBCOF;
	TComboBox *crossSmooz;
	TLabel *Label17;
	TLabel *Label20;
	TCheckBox *CBDiff;
	TTrackBar *TBDiff;
	TCheckBox *CBASync;
	TEdit *TxSampOff;
	TLabel *L21;
	TLabel *Label21;
	TCheckBox *CBCalWay;
	TGroupBox *GBS;
	TLabel *LFFTB;
	TLabel *LFFT;
	TLabel *LFFTT;
	TLabel *LFFTS;
	TLabel *LFFTF;
	TPanel *PCFFTB;
	TPanel *PCFFT;
	TPanel *PCFFTStg;
	TPanel *PCSync;
	TPanel *PCFreq;
	TTrackBar *TBCW;
	TLabel *L7;
	TLabel *L8;
	TCheckBox *PTTLock;
	TRadioButton *IDOFF;
	TRadioButton *IDCW;
	TRadioButton *IDMMV;
	TEdit *MMVID;
	TCheckBox *CBVari;
	TCheckBox *IDFSK;
	TCheckBox *IDDEC;
	TGroupBox *GB1;
	TLabel *L27;
	TComboBox *RxFifo;
	TComboBox *TxFifo;
	TLabel *L6;
	TRadioGroup *AppPriority;
	TRadioGroup *RGV;
	TButton *VEdit;
	TLabel *L17;
	TCheckBox *CBAT;
	TCheckBox *CBRTS;
	TCheckBox *CB24;
	TCheckBox *CBSTX;
	TLabel *Label6;
	TComboBox *DevNoOut;
	void __fastcall DispTxBpfClick(TObject *Sender);
	void __fastcall PortNameChange(TObject *Sender);
	
	void __fastcall WinFontBtnClick(TObject *Sender);
	void __fastcall JaBtnClick(TObject *Sender);
	void __fastcall EngBtnClick(TObject *Sender);
	
	
	

	void __fastcall DispLoopLPFClick(TObject *Sender);
	void __fastcall DispOutLPFClick(TObject *Sender);
	void __fastcall RadioBtnClick(TObject *Sender);



	
	void __fastcall SBClockAdjClick(TObject *Sender);
	void __fastcall TimerTimer(TObject *Sender);
	
	void __fastcall SBTestClick(TObject *Sender);
	
	
	void __fastcall RxBpfBtnClick(TObject *Sender);
	void __fastcall PCFFTClick(TObject *Sender);
	void __fastcall UDTxSampClick(TObject *Sender, TUDBtnType Button);
	void __fastcall CWIDMacClick(TObject *Sender);
	void __fastcall RGDemTypeClick(TObject *Sender);
	void __fastcall DispCrossOutLPFClick(TObject *Sender);
	void __fastcall TxSampOffChange(TObject *Sender);
	void __fastcall CBCalWayClick(TObject *Sender);
	void __fastcall IDOFFClick(TObject *Sender);
	void __fastcall VEditClick(TObject *Sender);
	
	void __fastcall CBATClick(TObject *Sender);
	void __fastcall PortNameDropDown(TObject *Sender);
	void __fastcall SourceClick(TObject *Sender);
	void __fastcall SBTestMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall DevNoChange(TObject *Sender);
private:
	int	m_DisEvent;

	CSSTVDEM	*pDem;
	double      m_TxSampOff;

	void __fastcall UpdateUI(void);
	void __fastcall SetCustomColor(void);


	BYTE	m_WinFontCharset;
	int		m_WinFontStyle;

	int     m_TestMode;
	int     m_TestCnt;
	int     m_SubCnt;
	CSmooz  m_Avg;
	double  m_DataList[17];
	double  m_DataList2[17];
	int     m_Off;
	int     m_Max1500;
	int     m_Max2300;
	double  m_OutGain;
	double  m_BuffCal;

	AnsiString  m_VOXSound;
    CMMList	m_MMList;
    CMMList	m_MMListW;
private:
	int __fastcall IsMMW(LPCSTR p);

public:
	virtual __fastcall TOptionDlg(TComponent* AOwner);

	int __fastcall Execute(CSSTVDEM *fp, CSSTVMOD *mp);
	int m_ColorChange;
	void __fastcall SetPageIndex(int n);
};
//----------------------------------------------------------------------------
//extern TAgcSetDlg *AgcSetDlg;
extern int PageIndex;
//----------------------------------------------------------------------------
#endif    
