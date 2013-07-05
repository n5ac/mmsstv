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
#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
#include "Sound.h"
#include "Comm.h"
#include "CRadio.h"
#include "Draw.h"
#include "InputWin.h"
#include "RxView.h"
#include "HistView.h"
#include "FileView.h"
#include "PrevView.h"
#include "CtrBtn.h"
#include "StockVew.h"
//#include <OleCtnrs.hpp>
#define STOCKPAGE   4
#define STOCKMAX    300
#define DFFTSIZE    768
//---------------------------------------------------------------------------
class TMmsstv : public TForm
{
__published:	// IDE 管理のコンポーネント
	TPageControl *Page;
	TMainMenu *MainMenu;
	TMenuItem *KFile;
	TTabSheet *TabRX;
	TTabSheet *TabHist;
	TPanel *PanelRX;
	TPaintBox *PBoxRX;
	TPanel *PanelFFT;
	TPanel *PanelWater;
	TTimer *Timer;
	TPaintBox *PBoxFFT;
	TPaintBox *PBoxWater;
	TMenuItem *KOpt;
	TMenuItem *KHelp;
	TMenuItem *KFRecTim;
	TMenuItem *KFRec;
	TMenuItem *KFPlay;
	TOpenDialog *OpenDialog;
	TSaveDialog *SaveDialog;
	TMenuItem *N1;
	TMenuItem *KExit;
	TMenuItem *KFPlayPos;
	TMenuItem *KFRecRew;
	TMenuItem *KFRecStop;
	TTabSheet *TabSync;
	TPanel *PanelSync;
	TPaintBox *PBoxD12;
	TGroupBox *GBMode;
	TSpeedButton *SBAuto;
	TSpeedButton *SBM1;
	TSpeedButton *SBM2;
	TSpeedButton *SBM3;
	TSpeedButton *SBM4;
	TSpeedButton *SBM7;
	TSpeedButton *SBLK;
	TPanel *PanelLvl;
	TPaintBox *PBoxLvl;
	TPanel *PanelHist;
	TPaintBox *PBoxHist;
	TUpDown *UDHist;
	TSpeedButton *SBLatest;
	TLabel *HistStat;
	TSpeedButton *SBPhase;
	TSpeedButton *SBSlant;
	TSpeedButton *SBSampSet;
	TTabSheet *TabTX;
	TTabSheet *TabTemp;
	TPanel *PanelTX;
	TPaintBox *PBoxTX;
	TPanel *PanelTemp;
	TPaintBox *PBoxTemp;
	TSpeedButton *SBPaste;
	TSpeedButton *SBCopy;
	TSpeedButton *SBTX;
	TMenuItem *KOSC;
	TMenuItem *KView;
	TMenuItem *KOM;
	TGroupBox *GBLog;
	TEdit *HisCall;
	TLabel *L1;
	TLabel *L2;
	TEdit *HisName;
	TLabel *L3;
	TComboBox *HisRST;
	TLabel *L4;
	TComboBox *MyRST;
	TLabel *L5;
	TEdit *HisQTH;
	TLabel *L6;
	TEdit *EditNote;
	TSpeedButton *SBQSO;
	TSpeedButton *SBData;
	TSpeedButton *SBFind;
	TSpeedButton *SBList;
	TComboBox *LogFreq;
	TSpeedButton *SBInit;
	TMenuItem *N2;
	TMenuItem *KLogFlush;
	TMenuItem *KLogOpen;
	TMenuItem *KVer;
	TSpeedButton *SBDSel;
	TSpeedButton *SBDline;
	TSpeedButton *SBDBox;
	TSpeedButton *SBDDel;
	TSpeedButton *SBDText;
	TSpeedButton *SBDPic;
	TSpeedButton *SBDPaste;
	TSpeedButton *SBDFont;
	TFontDialog *FontDialog;
	TGroupBox *GB1;
	TSpeedButton *SBAFC;
	TTabControl *TabS;
	TPanel *PanelS;
	TPaintBox *PBoxS;
	TSpeedButton *SBTune;
	TPanel *PCS;
	TUpDown *UDStock;
	TLabel *LS;
	TEdit *EditQSL;
	TLabel *L7;
	TColorDialog *ColorDialog;
	TSpeedButton *SBDLineset;
	TSpeedButton *SBDCol;
	TPopupMenu *PopupS;
	TMenuItem *KSL;
	TMenuItem *KSC;
	TMenuItem *KSP;
	TMenuItem *N3;
	TMenuItem *KSS;
	TMenuItem *KSCan;
	TMenuItem *N4;
	TMenuItem *KSD;
	TSpeedButton *SBDBoxS;
	TCheckBox *CBSTemp;
	TMenuItem *KOL;
	TPopupMenu *PopupT;
	TMenuItem *KTC;
	TMenuItem *KTP;
	TMenuItem *KTD;
	TMenuItem *N5;
	TMenuItem *KTB;
	TMenuItem *KTF;
	TMenuItem *N6;
	TMenuItem *KTCan;
	TSpeedButton *SBDHead;
	TSpeedButton *SBM5;
	TSpeedButton *SBM6;
	TSpeedButton *SBM8;
	TSpeedButton *SBM9;
	TMenuItem *KTT;
	TMenuItem *KTTC;
	TSpeedButton *SBHView;
	TSpeedButton *SBRView;
	TSpeedButton *SBTView;
	TSpeedButton *SBDS240;
	TMenuItem *N8;
	TMenuItem *KHlpMain;
	TMenuItem *N9;
	TMenuItem *KHlpPad;
	TMenuItem *N10;
	TMenuItem *KS;
	TMenuItem *KF;
	TMenuItem *KR;
	TMenuItem *KFL1;
	TMenuItem *KFM1;
	TMenuItem *KFH1;
	TMenuItem *KFT1;
	TMenuItem *N11;
	TMenuItem *KFL2;
	TMenuItem *KFM2;
	TMenuItem *KFH2;
	TMenuItem *KFT2;
	TMenuItem *KRS;
	TMenuItem *KRM;
	TMenuItem *KRF;
	TSpeedButton *SBWHist;
	TMenuItem *N12;
	TSpeedButton *SBUseTemp;
	TSpeedButton *SBDA240;
	TSpeedButton *SBUS240;
	TSpeedButton *SBUA240;
	TMenuItem *N13;
	TMenuItem *KTL;
	TMenuItem *KTS;
	TPaintBox *PBoxG;
	TMenuItem *KFFTS;
	TMenuItem *KFSOF;
	TMenuItem *KFSS;
	TMenuItem *KFSL;
	TUpDown *UDSamp;
	TLabel *SyncStat;
	TPopupMenu *PopupH;
	TMenuItem *KHCD;
	TOpenDialog *DirDialog;
	TMenuItem *N14;
	TMenuItem *KSCD;
	TMenuItem *KHD;
	TMenuItem *N15;
	TMenuItem *KHS;
	TMenuItem *KEdit;
	TMenuItem *KER;
	TMenuItem *KEC;
	TMenuItem *KEP;
	TMenuItem *KES;
	TMenuItem *N17;
	TMenuItem *KEA;
	TMenuItem *N16;
	TMenuItem *KHCan;
	TMenuItem *N18;
	TMenuItem *KHT;
	TMenuItem *KHFJ;
	TMenuItem *KHFB;
	TMenuItem *KSFJ;
	TMenuItem *KSFB;
	TPopupMenu *PopupR;
	TMenuItem *KRAS;
	TMenuItem *KRAR;
	TMenuItem *N19;
	TMenuItem *KRCan;
	TMenuItem *N20;
	TMenuItem *KRC;
	TMenuItem *KRSW;
	TMenuItem *KTA;
	TMenuItem *N21;
	TMenuItem *KVR;
	TMenuItem *N22;
	TMenuItem *KRR;
	TMenuItem *N23;
	TMenuItem *KTBT;
	TCheckBox *CBASave;
	TMenuItem *N24;
	TMenuItem *KHACD;
	TMenuItem *KHAC;
	TCheckBox *CBSDraft;
	TMenuItem *KRH;
	TSpeedButton *SBULog;
	TMenuItem *KHDA;
	TMenuItem *N25;
	TMenuItem *KHWeb;
	TMenuItem *KHRef;
	TMenuItem *KHACT;
	TMenuItem *KVS;
	TMenuItem *KFSQ;
	TMenuItem *KP;
	TMenuItem *KP1;
	TMenuItem *KP2;
	TMenuItem *KP3;
	TMenuItem *KP4;
	TMenuItem *KP5;
	TMenuItem *KP6;
	TMenuItem *KP7;
	TMenuItem *KP8;
	TMenuItem *N26;
	TMenuItem *KPA;
	TMenuItem *KPD;
	TMenuItem *N27;
	TMenuItem *KPDef;
	TMenuItem *KPA1;
	TMenuItem *KPA2;
	TMenuItem *KPA3;
	TMenuItem *KPA4;
	TMenuItem *KPA5;
	TMenuItem *KPA6;
	TMenuItem *KPA7;
	TMenuItem *KPA8;
	TMenuItem *KPD1;
	TMenuItem *KPD2;
	TMenuItem *KPD3;
	TMenuItem *KPD4;
	TMenuItem *KPD5;
	TMenuItem *KPD6;
	TMenuItem *KPD7;
	TMenuItem *KPD8;
	TMenuItem *KHACJ;
	TMenuItem *KVH;
	TSpeedButton *SBDList;
	TMenuItem *N28;
	TMenuItem *KTE;
	TMenuItem *KTTD;
	TMenuItem *N29;
	TMenuItem *KRSA;
	TSpeedButton *SBTXFil;
	TSpeedButton *SBHistFil;
	TMenuItem *N30;
	TMenuItem *KSTP;
	TMenuItem *KVF;
	TMenuItem *KRASY;
	TMenuItem *N31;
	TMenuItem *KFFTW;
	TMenuItem *KFW30;
	TMenuItem *KFW20;
	TMenuItem *KFW15;
	TMenuItem *N32;
	TMenuItem *KTFX;
	TMenuItem *N33;
	TMenuItem *KRB;
	TMenuItem *N34;
	TMenuItem *KHDS;
	TMenuItem *KHDS1;
	TMenuItem *KHDS2;
	TMenuItem *KHDS3;
	TMenuItem *KHDS4;
	TMenuItem *KHDS5;
	TMenuItem *KHDS6;
	TMenuItem *N35;
	TMenuItem *KVW;
	TMenuItem *N36;
	TMenuItem *KFTA;
	TMenuItem *N37;
	TMenuItem *KFTD;
	TMenuItem *KFTD1;
	TMenuItem *KFTD2;
	TMenuItem *KFTD3;
	TMenuItem *KFTD4;
	TMenuItem *KX;
	TMenuItem *KX1;
	TMenuItem *KX2;
	TMenuItem *KX3;
	TMenuItem *KX4;
	TMenuItem *KX5;
	TMenuItem *KX6;
	TMenuItem *KX7;
	TMenuItem *KX8;
	TMenuItem *N38;
	TMenuItem *KXA;
	TMenuItem *KXD;
	TMenuItem *N40;
	TMenuItem *KXR;
	TMenuItem *KXSM;
	TMenuItem *KXA1;
	TMenuItem *KXA2;
	TMenuItem *KXA3;
	TMenuItem *KXA4;
	TMenuItem *KXA5;
	TMenuItem *KXA6;
	TMenuItem *KXA7;
	TMenuItem *KXA8;
	TMenuItem *KXD1;
	TMenuItem *KXD2;
	TMenuItem *KXD3;
	TMenuItem *KXD4;
	TMenuItem *KXD5;
	TMenuItem *KXD6;
	TMenuItem *KXD7;
	TMenuItem *KXD8;
	TMenuItem *KFTD5;
	TMenuItem *KTAP;
	TMenuItem *KTAC;
	TMenuItem *N39;
	TSpeedButton *SBLMS;
	TMenuItem *KPInit;
	TMenuItem *KVL;
	TMenuItem *KVLSG;
	TMenuItem *KVLSY;
	TMenuItem *KTPA;
	TMenuItem *KHTB;
	TMenuItem *N42;
	TMenuItem *KHAP;
	TMenuItem *N43;
	TMenuItem *KSAI;
	TPopupMenu *PopupM;
	TMenuItem *KRRC;
	TMenuItem *KSSZ;
	TMenuItem *KSS1;
	TMenuItem *KSS2;
	TMenuItem *KSS3;
	TMenuItem *KSS4;
	TMenuItem *KHlpUp;
	TMenuItem *KSSD;
	TMenuItem *N41;
	TMenuItem *KTSZ;
	TMenuItem *KTSZ1;
	TMenuItem *KTSZ2;
	TMenuItem *KTSZ3;
	TMenuItem *KTSZ4;
	TMenuItem *N45;
	TMenuItem *KTSZT;
	TMenuItem *KSIS;
	TMenuItem *KTSD;
	TMenuItem *KHSD;
	TMenuItem *KRSD;
	TSpeedButton *SBTO;
	TMenuItem *KVOut;
	TMenuItem *KVIn;
	TMenuItem *N44;
	TMenuItem *KRCS;
	TMenuItem *KRSR;
	TSpeedButton *SBAdj;
	TMenuItem *KTU;
	TSpeedButton *SBRXID;
	TMenuItem *KSOFF;
	TMenuItem *KSFFT;
	TMenuItem *KSFQ;
	TPopupMenu *PopupC;
	TMenuItem *N46;
	TMenuItem *KSF;
	TMenuItem *KTDF;
	TMenuItem *KTF1;
	TMenuItem *KTF3;
	TMenuItem *KTKA;
	TMenuItem *KRI;
	TMenuItem *KRI1;
	TMenuItem *KRI2;
	TMenuItem *KRI3;
	TMenuItem *KVC;
	TMenuItem *KRCD;
	TMenuItem *KORep;
	TMenuItem *KTMD;
	TMenuItem *KEW;
	TMenuItem *KEW2;
	TMenuItem *KEW3;
	TMenuItem *KEW4;
	TMenuItem *KEW1;
	TMenuItem *KVSD;
	TMenuItem *KSTD;
	TMenuItem *KSTS;
	TMenuItem *KVSD1;
	TMenuItem *KVSD2;
	TMenuItem *KVSD3;
	TMenuItem *N47;
	TMenuItem *KHVS;
	TMenuItem *KHVS1;
	TMenuItem *KHVS2;
	TMenuItem *KHVS3;
	TMenuItem *KHVS4;
	TMenuItem *N48;
	TMenuItem *KRFS;
	TMenuItem *KSB;
	TMenuItem *KHC;
	TMenuItem *N49;
	TMenuItem *KSRR;
	TMenuItem *KHMT;
	TMenuItem *KHMT3;
	TMenuItem *KHMT4;
	TMenuItem *KSMS;
	TMenuItem *KSMS4;
	TMenuItem *KSMS9;
	TMenuItem *KSMSX;
	TMenuItem *KTF4;
	TMenuItem *KTSA;
	TMenuItem *KTSI;
	TMenuItem *KTLA;
	TMenuItem *KTLI;
	TMenuItem *KTPI;
	TMenuItem *KTPB;
	TMenuItem *KTPO;
	TMenuItem *KTF2;
	TMenuItem *Report31;
	TMenuItem *KRD;
	TSpeedButton *SBRXFil;
	TSpeedButton *SBFS;
	TMenuItem *KTO;
	TMenuItem *KTOI;
	TMenuItem *KTOT;
	TMenuItem *KTOP;
	TMenuItem *KTOC;
	TMenuItem *KTOE;
	TMenuItem *KTOS;
	TMenuItem *KTOB;
	TMenuItem *N50;
	TMenuItem *KTOCB;
	TMenuItem *KTOR;
	TMenuItem *KTX;
	TMenuItem *N51;
	TMenuItem *KVSE1;
	TMenuItem *KVSE2;
	TMenuItem *KVSE3;
	TMenuItem *KTCI;
	TPopupMenu *PopupL;
	TMenuItem *KLM1;
	TMenuItem *KLM2;
	TMenuItem *KLM3;
	TMenuItem *N52;
	TMenuItem *KSA;
	TMenuItem *KTFS;
	TMenuItem *N7;
	TMenuItem *KHN;
	TMenuItem *KHJ;
	TSpeedButton *SBCW;
	TMenuItem *KSAM;
	TMenuItem *KXS;
	TMenuItem *KEX;
	TSpeedButton *SBTXID;
	TMenuItem *KTH;
	TMenuItem *N53;
	TMenuItem *KEE;
	TMenuItem *KSE;
	TMenuItem *N54;
	TMenuItem *KXP;
	TMenuItem *KHE;
	TMenuItem *KRCC;
	TSpeedButton *SBPL;
	TSpeedButton *SBPR;
	TMenuItem *KRCR;
	TPopupMenu *PopupCW;
	TMenuItem *KCW;
	TMenuItem *KCWM;
	TMenuItem *NCW;
	TMenuItem *N55;
	TMenuItem *KCWCancel;
	TMenuItem *KCWS;
	TMenuItem *KCWSH;
	TMenuItem *KCWSM;
	TMenuItem *KCWSL;
	TMenuItem *KCWSLL;
	TMenuItem *N57;
	TMenuItem *KCWSI;
	TMenuItem *KRadio;
	TMenuItem *KRSet;
	TMenuItem *KRLoad;
	TMenuItem *N58;
	TMenuItem *NRadio;
	TMenuItem *KRadioS;
	TMenuItem *KRadioAdd;
	TMenuItem *NRadioE;
	TMenuItem *NCWE;
	TMenuItem *KCWADD;
	void __fastcall FormPaint(TObject *Sender);
	
	void __fastcall TimerTimer(TObject *Sender);
	void __fastcall PBoxFFTPaint(TObject *Sender);
	void __fastcall PBoxWaterPaint(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall PBoxRXPaint(TObject *Sender);
	void __fastcall KFRecTimClick(TObject *Sender);
	void __fastcall KFRecClick(TObject *Sender);
	void __fastcall KFPlayClick(TObject *Sender);
	void __fastcall KFPlayPosClick(TObject *Sender);
	void __fastcall KFRecRewClick(TObject *Sender);
	void __fastcall KFRecStopClick(TObject *Sender);


	void __fastcall PBoxD12MouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall PBoxD12Paint(TObject *Sender);
	void __fastcall SBAutoClick(TObject *Sender);
	void __fastcall UDHistClick(TObject *Sender, TUDBtnType Button);
	void __fastcall PBoxHistPaint(TObject *Sender);
	void __fastcall SBLatestClick(TObject *Sender);
	void __fastcall PBoxD12MouseMove(TObject *Sender, TShiftState Shift,
		  int X, int Y);
	void __fastcall SBSampSetClick(TObject *Sender);
	void __fastcall SBPasteClick(TObject *Sender);
	void __fastcall PBoxTXPaint(TObject *Sender);
	void __fastcall SBCopyClick(TObject *Sender);
	void __fastcall SBTXClick(TObject *Sender);
	void __fastcall PageChange(TObject *Sender);
	void __fastcall PBoxTempPaint(TObject *Sender);
	void __fastcall KOSCClick(TObject *Sender);
	void __fastcall KOMClick(TObject *Sender);
	
	void __fastcall KExitClick(TObject *Sender);
	void __fastcall SBListClick(TObject *Sender);
	void __fastcall SBFindClick(TObject *Sender);
	void __fastcall SBFindMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall SBDataClick(TObject *Sender);
	void __fastcall SBDataMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall SBInitClick(TObject *Sender);
	void __fastcall TempDelay(void);
	void __fastcall LogFreqChange(TObject *Sender);
	void __fastcall HisCallKeyPress(TObject *Sender, char &Key);
	void __fastcall SBQSOClick(TObject *Sender);
	void __fastcall KLogOpenClick(TObject *Sender);
	void __fastcall KLogFlushClick(TObject *Sender);
	void __fastcall KVerClick(TObject *Sender);
	void __fastcall PBoxTempMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall PBoxTempMouseMove(TObject *Sender, TShiftState Shift,
		  int X, int Y);
	void __fastcall PBoxTempMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall SBDlineClick(TObject *Sender);
	void __fastcall SBDBoxClick(TObject *Sender);
	void __fastcall SBDSelClick(TObject *Sender);
	void __fastcall SBDDelClick(TObject *Sender);
	void __fastcall SBDTextClick(TObject *Sender);
	void __fastcall SBDPicClick(TObject *Sender);
	void __fastcall SBDPasteClick(TObject *Sender);
	void __fastcall PBoxTempDblClick(TObject *Sender);
	void __fastcall SBDFontClick(TObject *Sender);
	void __fastcall PBoxSDragDrop(TObject *Sender, TObject *Source,
		  int X, int Y);
	void __fastcall PBoxSDragOver(TObject *Sender, TObject *Source,
		  int X, int Y, TDragState State, bool &Accept);
	void __fastcall PBoxSPaint(TObject *Sender);
	void __fastcall SBAFCClick(TObject *Sender);

	
	
	
	
	void __fastcall PBoxSMouseDown(TObject *Sender,
		  TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall TabSChange(TObject *Sender);
	void __fastcall PBoxTXDragOver(TObject *Sender, TObject *Source, int X,
		  int Y, TDragState State, bool &Accept);
	void __fastcall PBoxTXDragDrop(TObject *Sender, TObject *Source, int X,
		  int Y);
	void __fastcall UDStockClick(TObject *Sender, TUDBtnType Button);
	void __fastcall SBTuneClick(TObject *Sender);
	void __fastcall SBDLinesetClick(TObject *Sender);
	void __fastcall SBDColClick(TObject *Sender);
	
	void __fastcall KSLClick(TObject *Sender);
	void __fastcall KSSClick(TObject *Sender);
	void __fastcall KSCClick(TObject *Sender);

	void __fastcall KSPClick(TObject *Sender);
	void __fastcall KSDClick(TObject *Sender);
	void __fastcall PopupSPopup(TObject *Sender);
	void __fastcall SBDBoxSClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall HisCallChange(TObject *Sender);
	void __fastcall CBSTempClick(TObject *Sender);
	void __fastcall KOLClick(TObject *Sender);
	void __fastcall KTDClick(TObject *Sender);
	void __fastcall KTBClick(TObject *Sender);
	void __fastcall KTFClick(TObject *Sender);
	void __fastcall SBDHeadClick(TObject *Sender);
	void __fastcall SBMClick(TObject *Sender);
	void __fastcall KTTClick(TObject *Sender);
	void __fastcall KTTCClick(TObject *Sender);
	void __fastcall SBHViewClick(TObject *Sender);
	void __fastcall SBRViewClick(TObject *Sender);
	void __fastcall SBTViewClick(TObject *Sender);
	void __fastcall SBDS240Click(TObject *Sender);
	void __fastcall SBLKClick(TObject *Sender);
	void __fastcall PopupTPopup(TObject *Sender);
	void __fastcall PBoxTXMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall KHlpMainClick(TObject *Sender);
	void __fastcall KHlpPadClick(TObject *Sender);
	void __fastcall KFileClick(TObject *Sender);
	void __fastcall KFL1Click(TObject *Sender);
	void __fastcall KFM1Click(TObject *Sender);
	void __fastcall KFH1Click(TObject *Sender);
	void __fastcall KFT1Click(TObject *Sender);
	void __fastcall KFL2Click(TObject *Sender);
	void __fastcall KFM2Click(TObject *Sender);
	void __fastcall KFH2Click(TObject *Sender);
	void __fastcall KFT2Click(TObject *Sender);
	void __fastcall KRSClick(TObject *Sender);
	void __fastcall KRMClick(TObject *Sender);
	void __fastcall KRFClick(TObject *Sender);
	void __fastcall KViewClick(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
		  TShiftState Shift);
	void __fastcall FormKeyUp(TObject *Sender, WORD &Key,
		  TShiftState Shift);
	void __fastcall SBWHistClick(TObject *Sender);
	void __fastcall KVOutClick(TObject *Sender);
	
	void __fastcall SBUseTempClick(TObject *Sender);
	void __fastcall SBDA240Click(TObject *Sender);
	void __fastcall SBUS240Click(TObject *Sender);
	void __fastcall SBUA240Click(TObject *Sender);
	void __fastcall KTLClick(TObject *Sender);
	void __fastcall KTSAClick(TObject *Sender);
	void __fastcall PBoxGPaint(TObject *Sender);

	void __fastcall KFSOFClick(TObject *Sender);
	void __fastcall KFSSClick(TObject *Sender);
	void __fastcall KFSLClick(TObject *Sender);

	void __fastcall UDSampClick(TObject *Sender, TUDBtnType Button);
	void __fastcall SBPhaseClick(TObject *Sender);
	void __fastcall KHCDClick(TObject *Sender);
	void __fastcall KSCDClick(TObject *Sender);
	
	void __fastcall SBTuneMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall KHDClick(TObject *Sender);
	void __fastcall KHSClick(TObject *Sender);
	void __fastcall KEditClick(TObject *Sender);
	void __fastcall KERClick(TObject *Sender);
	void __fastcall KESClick(TObject *Sender);
	void __fastcall PopupHPopup(TObject *Sender);
	void __fastcall KHFJClick(TObject *Sender);
	void __fastcall KHFBClick(TObject *Sender);
	void __fastcall KSFJClick(TObject *Sender);
	void __fastcall KSFBClick(TObject *Sender);
	void __fastcall KRASClick(TObject *Sender);
	void __fastcall KRARClick(TObject *Sender);
	void __fastcall PopupRPopup(TObject *Sender);
	void __fastcall KTAClick(TObject *Sender);
	void __fastcall KVRClick(TObject *Sender);
	void __fastcall KRRClick(TObject *Sender);
	void __fastcall KTBTClick(TObject *Sender);
	
	void __fastcall KHACDClick(TObject *Sender);
	void __fastcall SBULogClick(TObject *Sender);
	void __fastcall KHDAClick(TObject *Sender);
	void __fastcall KHWebClick(TObject *Sender);
	void __fastcall KHRefClick(TObject *Sender);
	void __fastcall KVSClick(TObject *Sender);

	void __fastcall KFSQClick(TObject *Sender);
	void __fastcall EditQSLDragDrop(TObject *Sender, TObject *Source,
		  int X, int Y);
	void __fastcall EditQSLDragOver(TObject *Sender, TObject *Source,
		  int X, int Y, TDragState State, bool &Accept);
	void __fastcall KPAClick(TObject *Sender);
	void __fastcall KPDClick(TObject *Sender);
	void __fastcall KP1Click(TObject *Sender);

	void __fastcall KPDefClick(TObject *Sender);
	void __fastcall KPClick(TObject *Sender);
	void __fastcall KVHClick(TObject *Sender);
	void __fastcall SBDListClick(TObject *Sender);
	void __fastcall KTEClick(TObject *Sender);
	void __fastcall KTTDClick(TObject *Sender);
	void __fastcall HisNameChange(TObject *Sender);
	void __fastcall HisQTHChange(TObject *Sender);
	void __fastcall EditNoteChange(TObject *Sender);
	void __fastcall SBTXFilClick(TObject *Sender);
	void __fastcall SBHistFilClick(TObject *Sender);
	void __fastcall KSTPClick(TObject *Sender);
	void __fastcall KVFClick(TObject *Sender);
	void __fastcall KRASYClick(TObject *Sender);
	void __fastcall KFW30Click(TObject *Sender);
	void __fastcall KFW20Click(TObject *Sender);
	void __fastcall KFW15Click(TObject *Sender);
	void __fastcall KTFXClick(TObject *Sender);
	void __fastcall KRBClick(TObject *Sender);
	
	void __fastcall KHDS1Click(TObject *Sender);
	void __fastcall KHDS2Click(TObject *Sender);
	void __fastcall KHDS3Click(TObject *Sender);
	void __fastcall KHDS4Click(TObject *Sender);
	void __fastcall KHDS5Click(TObject *Sender);
	void __fastcall KHDS6Click(TObject *Sender);
	void __fastcall KVWClick(TObject *Sender);
	void __fastcall KFTAClick(TObject *Sender);
	void __fastcall KFTD1Click(TObject *Sender);
	void __fastcall KFTD2Click(TObject *Sender);
	void __fastcall KFTD3Click(TObject *Sender);
	void __fastcall KFTD4Click(TObject *Sender);
	void __fastcall KXRClick(TObject *Sender);
	void __fastcall KXSClick(TObject *Sender);
	void __fastcall KX1Click(TObject *Sender);
	void __fastcall KXA1Click(TObject *Sender);
	void __fastcall KXD1Click(TObject *Sender);
	void __fastcall KXClick(TObject *Sender);
	void __fastcall KFTD5Click(TObject *Sender);
	void __fastcall KTAPClick(TObject *Sender);
	void __fastcall KTACClick(TObject *Sender);
	void __fastcall SBLMSClick(TObject *Sender);
	void __fastcall KPInitClick(TObject *Sender);
	void __fastcall KVLSGClick(TObject *Sender);
	void __fastcall KVLSYClick(TObject *Sender);
	void __fastcall KHTBClick(TObject *Sender);
	void __fastcall OpenDialogSelectionChange(TObject *Sender);
	void __fastcall KTPAClick(TObject *Sender);

	void __fastcall KSAIClick(TObject *Sender);
	void __fastcall PopupMPopup(TObject *Sender);

	void __fastcall SBMMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall KRRCClick(TObject *Sender);
	void __fastcall KSSZClick(TObject *Sender);
	void __fastcall KHlpUpClick(TObject *Sender);
	void __fastcall KSSDClick(TObject *Sender);
	void __fastcall KTSZClick(TObject *Sender);
	void __fastcall KTSDClick(TObject *Sender);
	void __fastcall KHSDClick(TObject *Sender);
	void __fastcall KRSDClick(TObject *Sender);
	void __fastcall SBTOClick(TObject *Sender);
	void __fastcall KRCSClick(TObject *Sender);
	void __fastcall KRSRClick(TObject *Sender);
	
	void __fastcall KTUClick(TObject *Sender);
	
	void __fastcall SBRXIDClick(TObject *Sender);
	void __fastcall KSOFFClick(TObject *Sender);
	void __fastcall SBRXIDMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall KSFClick(TObject *Sender);
	void __fastcall KTF1Click(TObject *Sender);
	void __fastcall KTKAClick(TObject *Sender);
	void __fastcall UDStockMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall KRI1Click(TObject *Sender);
	
	void __fastcall HisNameKeyPress(TObject *Sender, char &Key);
	void __fastcall KVCClick(TObject *Sender);
	void __fastcall KRCDClick(TObject *Sender);
	void __fastcall KORepClick(TObject *Sender);
	void __fastcall KTMDClick(TObject *Sender);
	void __fastcall KEWClick(TObject *Sender);
	void __fastcall PBoxSMouseMove(TObject *Sender, TShiftState Shift,
		  int X, int Y);
	void __fastcall PBoxSDblClick(TObject *Sender);
	void __fastcall HisCallDblClick(TObject *Sender);
	void __fastcall HisQTHDblClick(TObject *Sender);

	void __fastcall KVSD1Click(TObject *Sender);
	void __fastcall KHVSClick(TObject *Sender);
	void __fastcall KRFSClick(TObject *Sender);
	void __fastcall KSBClick(TObject *Sender);
	void __fastcall KSMS4Click(TObject *Sender);
	
	void __fastcall KHMT3Click(TObject *Sender);
	
	void __fastcall KHelpClick(TObject *Sender);
	void __fastcall OpenDialogClose(TObject *Sender);
	void __fastcall KTSIClick(TObject *Sender);
	void __fastcall KTLIClick(TObject *Sender);
	void __fastcall KTPBClick(TObject *Sender);
	
	
	void __fastcall SBRXFilClick(TObject *Sender);
	
	void __fastcall UDHistMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	
	void __fastcall KTOIClick(TObject *Sender);
	void __fastcall KTOClick(TObject *Sender);
	void __fastcall KTOPClick(TObject *Sender);
	void __fastcall KTOCClick(TObject *Sender);
	
	
	
	
	
	
	
	
	
	
	
	
	void __fastcall IMenuClick(TObject *Sender);
	void __fastcall KVSDClick(TObject *Sender);
	void __fastcall KVSE1Click(TObject *Sender);
	void __fastcall KTCIClick(TObject *Sender);
	void __fastcall KTOSClick(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall PBoxFFTMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall PBoxFFTMouseMove(TObject *Sender, TShiftState Shift,
		  int X, int Y);
	void __fastcall PBoxFFTMouseUp(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall SBTXMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall SBLMSMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall KLM1Click(TObject *Sender);

	void __fastcall KSAClick(TObject *Sender);
	void __fastcall KTFSClick(TObject *Sender);
	void __fastcall KHNClick(TObject *Sender);
	void __fastcall KHJClick(TObject *Sender);
	void __fastcall SBCWClick(TObject *Sender);
	void __fastcall SBCWMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	
	
	void __fastcall KSAMClick(TObject *Sender);
	
	void __fastcall SBListMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	
	void __fastcall PBoxLvlMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall SBTXIDClick(TObject *Sender);
	void __fastcall SBDFontMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall KTHClick(TObject *Sender);
	void __fastcall KEEClick(TObject *Sender);
	void __fastcall KSEClick(TObject *Sender);
	void __fastcall KXPClick(TObject *Sender);
	void __fastcall KHEClick(TObject *Sender);
	void __fastcall KRCCClick(TObject *Sender);
	
	void __fastcall SBPLClick(TObject *Sender);
	
	
	
	
	void __fastcall SBPLMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	
	void __fastcall SBAdjMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall KRCRClick(TObject *Sender);
	void __fastcall KCWClick(TObject *Sender);
	void __fastcall PopupCWPopup(TObject *Sender);
	void __fastcall KCWSHClick(TObject *Sender);
	void __fastcall KCWSMClick(TObject *Sender);
	void __fastcall KCWSLClick(TObject *Sender);
	void __fastcall KCWSLLClick(TObject *Sender);
	void __fastcall KCWSIClick(TObject *Sender);
	void __fastcall KRSetClick(TObject *Sender);
	void __fastcall KRLoadClick(TObject *Sender);
	
	void __fastcall KRadioAddClick(TObject *Sender);
	void __fastcall KRadioClick(TObject *Sender);
	void __fastcall KCWADDClick(TObject *Sender);
private:	// ユーザー宣言
	int     m_InitFirst;
	int     m_DisEvent;
//	int     m_Timer1S;
	int		m_ChangeTemp;

	int     m_SMax;

	short   m_Y36[800];
	short   m_D36[2][800];
	int     m_DSEL;

	CAlignList  AlignFix;
	CAlignList  AlignMode;
	CAlignList  AlignDsp;
	CAlignList  AlignLog;
	CAlignList  AlignPCS;
	void __fastcall EntryAlign(void);
	void __fastcall StartOption(void);

	void __fastcall UpdateLogLink(void);
	void __fastcall UpdateTitle(void);
	void __fastcall UpdateLMS(void);
	void __fastcall AdjustFocus(void);

	void __fastcall UpdateSBTO(void);
	void __fastcall UpdateUI(void);
	void __fastcall UpdateUITemp(void);

//    void __fastcall OnIdle(TObject *Sender, bool &Done);
	void __fastcall UpdateToneFreq(void);
	void __fastcall UpdateSystemFont(void);
	void __fastcall ReadSampFreq(void);
	void __fastcall ReadCombList(TMemIniFile *tp, AnsiString &as, LPCSTR pKey, LPCSTR pDef);
	void __fastcall ReadRegister(void);
	void __fastcall WriteRegister(void);

	void __fastcall UpdateModeBtn(void);

	void __fastcall ClearDraw(Graphics::TBitmap *pBitmap, TPaintBox *pBox, TColor col);
	void __fastcall SyncSSTV(void);
	int __fastcall GetPixelLevel(short *ip);
	int __fastcall GetPictureLevel(short *ip);
	int __fastcall GetPictureLevelDiff(short *ip);
	void __fastcall DrawSSTV(short *ip, short *sp);
	void __fastcall DrawSSTVNormal(short *ip, short *sp);
	void __fastcall DrawSSTVDiff(short *ip, short *sp);
	void __fastcall InfoRxLost(void);
	void __fastcall InfoTxLost(void);
	void __fastcall TrackTxMode(int sw);
	void __fastcall WriteHistory(int sw);
	void __fastcall DrawSSTV(void);
	void __fastcall DrawLvl(void);
	void __fastcall AdjustPBoxSize(int l);
	void __fastcall KMClick(TObject *Sender);

public:
	Graphics::TBitmap *pBitmapFFT;
	Graphics::TBitmap *pBitmapWater;
	Graphics::TBitmap *pBitmapRX;
	Graphics::TBitmap *pBitmapD12;
	Graphics::TBitmap *pBitmapLvl;
	Graphics::TBitmap *pBitmapHist;
	Graphics::TBitmap *pBitmapHistF;
	Graphics::TBitmap *pBitmapTX;
	Graphics::TBitmap *pBitmapTXM;
	Graphics::TBitmap *pBitmapTemp;
	Graphics::TBitmap *pBitmapNearest;

	Graphics::TBitmap *pBitmapSS;
	Graphics::TBitmap *pBitmapST[STOCKPAGE];

	int         m_MW;           // タイトルバーとメニューバーの幅
	int         m_ReqHistF;
	void __fastcall DeleteHistF(void);
	void __fastcall AdjustHistF(void);
	void __fastcall TrigHistF(void);

private:
	CRecentMenu RecentMenu;
	void __fastcall OnRecentClick(TObject *Sender);
	void __fastcall RecentAdd(LPCSTR pNew, int f);
	void __fastcall UpdateRecentMenu(void);

	int     m_Slant;
	int     m_SlantX, m_SlantY;
	int     m_SlantX2, m_SlantY2;
	void __fastcall DispSyncStat(void);
	double __fastcall GetSyncSamp(int X, int Y);
	void __fastcall DrawSync(void);

//    void __fastcall GetRY(int &Y, int &RY, int &BY, TColor col);
	void __fastcall LineR24(CSSTVMOD *mp);
	void __fastcall LineR36(CSSTVMOD *mp);
	void __fastcall LineR72(CSSTVMOD *mp);
	void __fastcall LineAVT(CSSTVMOD *mp);
	void __fastcall LineSCT(CSSTVMOD *mp, double tw);
	void __fastcall LineMRT(CSSTVMOD *mp, double tw);
	void __fastcall LineSC2180(CSSTVMOD *mp, double S, double tw);
	void __fastcall LinePD(CSSTVMOD *mp, double tw);
	void __fastcall LineP(CSSTVMOD *mp, double S, double P, double C);
	void __fastcall LineMP(CSSTVMOD *mp, double tw);
	void __fastcall LineMR(CSSTVMOD *mp, double tw);
	void __fastcall LineRM(CSSTVMOD *mp, double ts, double tw);
	void __fastcall LineMN(CSSTVMOD *mp, double tw);
	void __fastcall LineMC(CSSTVMOD *mp, double tw);

	BYTE            *m_pRowBuf;
	void __fastcall OutputMMV(void);
	void __fastcall OutputFSKID(void);
	void __fastcall OutputCWID(void);
	int __fastcall IsSR(void);
	void __fastcall SendSSTV(void);
	void __fastcall OutHEAD(void);

	int     m_SendY;
	void __fastcall DrawTXCursor_(int y);
	void __fastcall DrawTXCursor(int y);
	void __fastcall DeleteTXCursor(void);
	void __fastcall MakeTxBitmap(int sw);

	void __fastcall AutoCopyJob(SYSTEMTIME tim, Graphics::TBitmap *pSource);

	void __fastcall UpdateTxClip(void);
	int     m_TxClipType;
//calibration
	short   *pCalibration;
public:
	void __fastcall MakeCalibrationTable(void);

//Hist
	CBitmapHist RxHist;
	void __fastcall UpdateHist(void);
	void __fastcall DispHistStat(void);
	void __fastcall GetTempHistSize(int &x, int &y);
public:
//Mode
	int             m_ExtMode;
	int             m_HistM;
	int             m_RXW, m_RXH, m_RXPH;
	int             m_TXW, m_TXH, m_TXPH;
	int             m_ModeAssignTX[9];
	int             m_ModeAssignRX[9];
private:
	TSpeedButton *__fastcall GetModeBtn(int n);
	int __fastcall GetButtonNo(TSpeedButton *tp);
	void __fastcall UpdateModeCaption(void);
	void __fastcall ChangeTxMode(int mode);
	int __fastcall FindModeAssign(int *ip, int m);
	void __fastcall AdjustTempSC(int &X, int &Y);
	void __fastcall AdjustTempView(int sw);
	void __fastcall ShowSize(TPaintBox *pBox, int w, int h, int fs, TColor col);
	void __fastcall ShowSize(Graphics::TBitmap *pBitmap, int x, int y, int w, int h, int fs, TColor col);
	void __fastcall ShowNo(Graphics::TBitmap *pBitmap, int x, int y, int n, int fs, TColor col);

//Recv
public:
	SYSTEMTIME      m_StartTime;
private:
	double          m_Z[3];
	int             m_AX, m_AY;
//Tune
	DWORD   m_TuneTimer;
	int     m_TuneTX;
//Susp
	int		m_SuspMinimized;
//Sync
	void __fastcall CopyStgBuf(void);
	void __fastcall InfoSampFreq(void);
	CWaveStrage     WaveStg;
	int             m_ReqSampChg;
	double          m_SampFreq;
	void __fastcall DrawSlantInfo(void);
	void __fastcall DrawShiftInfo(void);
	void __fastcall CorrectSlant(void);
	int __fastcall AdjustSyncPos(int n);
	void __fastcall ReSyncSSTV(void);
	void __fastcall RedrawSampFreq(int sw);
	void __fastcall UpdateSampFreq(void);
	void __fastcall RedrawSSTV(void);
	void __fastcall RedrawAdjustSync(void);
	void __fastcall ShiftSSTV(TObject *Sender, int step);

//FFT
	void __fastcall InitDrawFFT(void);
	void __fastcall CalcFFT(void);
	void __fastcall DrawFFT(int sw);
	void __fastcall DrawWater(int sw);
public:
	void __fastcall InitFFT(void);
private:
	int     m_FFTWINDOW;
	int     m_XW;
	int     m_FM;
	int     m_low;
	int     m_of;
	int     m_FX[7];
	int     m_fft[DFFTSIZE];

	int     m_FFTFlag;

	int		m_FFTMax;
	int	    m_FFTSumMax;

	int     m_FFTMAX[DFFTSIZE];

	int		m_WaterMax;
	int	    m_WaterSumMax;
	int		m_WaterMin;

	int     m_MainPage;
//Log
	void __fastcall AddCall(LPCSTR p);
	void __fastcall KCClick(TObject *Sender);
	int             m_Dupe;
	void __fastcall UpdateCallsign(void);
	void __fastcall UpdateLogData(void);
	void __fastcall UpdateTextData(void);
	void __fastcall FindCall(void);
	void __fastcall AutoLogSave(void);
	void __fastcall SetGreetingString(LPSTR t, LPCSTR pCall, int type);

	void __fastcall DateBitmap(Graphics::TBitmap *pBitmap, SYSTEMTIME &tim);

// Draw
	int             m_TempHold;
	int             m_TempTimer;
	int             m_TempX, m_TempY;
	int             m_DrawTrans;
	int             m_DrawCmd;
	CDraw           *pDraw;
	CDraw           *pPaste;
public:
	CDrawGroup      DrawMain;
	CDrawGroup      DrawText;
	CDrawGroup      DrawTemp;
	TMemoryStream   *pUndo;
//    CGrid			TempGrid;
private:
	void __fastcall DeleteUndo(void);
	void __fastcall SaveUndo(void);
	TMemoryStream *__fastcall SaveUndo(TMemoryStream *ps);
	void __fastcall UpdateUndo(TMemoryStream *ps);
	void __fastcall PBoxTempCursor(int X, int Y);


public:
	void __fastcall RxAutoPush(int sw);
	void __fastcall UpdateTemp(void);
	void __fastcall UpdatePic(void);
	void __fastcall UpdatePic(int sw);
	void __fastcall DropOle(int X, int Y, LPCSTR pName, int trans);
	void __fastcall DropLib(int X, int Y, LPCSTR pName);
	void __fastcall DropPic(int X, int Y, Graphics::TBitmap *pSrc, int type);
private:
// Stock
	void __fastcall GetStockXY(int &X1, int &Y1, int &X2, int &Y2, int n);
	int __fastcall TMmsstv::GetStockNo(int X, int Y);
	int             m_DragNo;
	int             m_DragDataNo;
	int             m_DragPMax;
	int             m_TabSXX, m_TabSYY;

	int             m_TxSPage;
	int             m_PSPage;
	int             m_TSPage[STOCKPAGE];
	int             m_APSPage;
	int             m_ATSPage[STOCKPAGE];
	DWORD           m_PSSize[18];

	int             m_StockPage;
	int             m_StockOff;

	void __fastcall UpdateTabS(void);
	void __fastcall SaveBitmapS(Graphics::TBitmap *pBitmap, int n);
	void __fastcall SaveBitmapSN(Graphics::TBitmap *pBitmap, int n);
	Graphics::TBitmap* __fastcall LoadBitmapS(int n);
	void __fastcall LoadBitmapSN(Graphics::TBitmap *pBitmap, int n);

public:
	int __fastcall LoadBitmapMenu(Graphics::TBitmap *pBitmap, int rs);
	int __fastcall SaveBitmapMenu(Graphics::TBitmap *pBitmap, LPCSTR pName, LPCSTR pFolder);
	int __fastcall LoadTemplateMenu(CDrawGroup *pItem, int isw);
	int __fastcall SaveTemplateMenu(CDrawGroup *pItem, int isw);
	int __fastcall SaveTemplateMenu(CDrawGroup *pItem, LPCSTR pName, LPCSTR pFolder, int isw);
	void __fastcall AddTemplate(CDrawGroup *pGroup);
	void __fastcall AddItem(CDraw *pItem, int b);

	void __fastcall LoadCurrentTemp(void);
	void __fastcall LoadStockTemp(CDrawGroup *pItem, int n, TCanvas *tp);
	void __fastcall SaveStockTemp(CDrawGroup *pItem, int n);
	void __fastcall DeleteStockTemp(int n);

private:

	void __fastcall LoadDefTemp(void);
	void __fastcall MoveStockDir(LPCSTR pNewDir);
	void __fastcall ChangeStockFormat(int sw);
	void __fastcall ReleaseStockResource(void);

	int     m_SyncAccuracy;
	int     m_SyncAccuracyN;

	void __fastcall InitAutoStop(void);
	int __fastcall GetSqerrPos(int n);
	int __fastcall AutoStop(int x, short *sp);
	int __fastcall AutoStopJob(void);

	int     m_SpecDown;
//Auto
	int     m_Mult;
//AutoStop
	int     m_AutoStopPos;
	int     m_AutoStopAPos[16];
	int     m_AutoStopCnt;
	int     m_AutoStopACnt;
//AutoSync
	int     m_AutoSyncCount;
	int     m_AutoSyncPos;
	int     m_AutoSyncDis;
	int     m_AutoSyncDiff;
//AutoSlant
	int     m_ASBgnPos;
	int     m_ASCurY;
	int     m_ASDis;
	int     m_ASBitMask;
	double  m_ASLmt[7];
	int     m_ASPos[4];
	CSmooz  m_ASAvg;
//Palette
	DYNAMIC HPALETTE __fastcall GetPalette(void);
//RxView
	int     m_TimerRXV;
	int     m_TimerRXS;
	TRxViewDlg *pRxView;
	TRxViewDlg *pSyncView;
//HistView
	THistViewDlg *pHistView;
	void __fastcall OpenHistView(void);
	void __fastcall CloseHistView(void);

//ImageView
	void __fastcall AdjustFileView(void);
	void __fastcall ShowFileView(int i, int ttl);
	void __fastcall CloseFileView(int n);

public:
	void __fastcall ReOpenFileView(TFileViewDlg *pView);
	void __fastcall LoadAllFileView(void);
	TFileViewDlg *pFileView[8];
	int     m_FileViewClose;
//Preview
	int     m_PreViewFlag;
	TPrevViewBox *pPreView;
	int __fastcall OpenDialogExecute(int sw);
private:
//PROFILE
	PRODEM  m_DemPro[9];
	TMenuItem *__fastcall GetKP(int n);
	TMenuItem *__fastcall GetKPA(int n);
	TMenuItem *__fastcall GetKPD(int n);
	void __fastcall UpdateProFileMenu(void);
	void __fastcall SetProFile(int n);
	void __fastcall InitProfile(void);

//PROGRAM
	TMenuItem *__fastcall GetKX(int n);
	TMenuItem *__fastcall GetKXA(int n);
	TMenuItem *__fastcall GetKXD(int n);
	void __fastcall UpdateProgramMenu(void);
	void __fastcall ExecExtCmd(int n);
	int __fastcall AssignExtCmd(int n);

//LMSDelay
	int __fastcall GetLMSDelay(void);

// WebRef
	CWebRef		WebRef;

// Comm
	void __fastcall OpenCloseCom(void);

	CComm   *pComm;
	CCradio *pRadio;

//コントロールWnd
	TCtrBtnWnd *pCtrBtn;

	int     m_SyncPos, m_SyncRPos;
	int     m_SyncMax, m_SyncMin;
public:		// ユーザー宣言
	__fastcall TMmsstv(TComponent* Owner);

	TSound *pSound;
	CSSTVDEM *pDem;
	CSSTVMOD *pMod;
	void __fastcall MacroDate(LPSTR t, SYSTEMTIME &now);
	int __fastcall MacroText(LPSTR t, LPCSTR p, int size);

	void __fastcall WndProc(TMessage &Message);
	void __fastcall WndCopyData(TMessage &Message);
	void __fastcall OnMini(TObject *Sender);
	void __fastcall OnMSG(tagMSG &Msg, bool &Handled);
	void __fastcall AppException(TObject *Sender, Exception *E);
	void __fastcall DropFile(tagMSG &Msg);

	void __fastcall UpdateTXUI(void);
	void __fastcall ToTX(void);
	void __fastcall ToRX(void);

	void __fastcall CopyBitmap(Graphics::TBitmap *pBitmap);
	int __fastcall PasteBitmap(Graphics::TBitmap *pBitmap, int rs);

	void __fastcall GetFFTRect(int &FM, int &low);
	void __fastcall MultProc(void);
	void __fastcall AdjustPage(int sw);


	DWORD           m_RepTick;
	int             m_RepBeaconCount;
	int             m_RepBeaconPos;
	int             m_RepTXPos;

	void __fastcall Repeater(void);
	void __fastcall SendCWID(LPCSTR p);
	int __fastcall LoadRepTemplate(LPCSTR pList, int &Pos);
	void __fastcall SendBeacon(void);
	void __fastcall SetBeaconTick(void);
	void __fastcall SetBeaconDelay(void);
	void __fastcall GetBeaconBitmap(Graphics::TBitmap *pBitmap, int n);
	void __fastcall MakeBeacon(int sw, int fsw, int nn);
	void __fastcall MakeReplay(void);
	void __fastcall MakeThImage(int mm);

	int m_OrgTop;
	int m_OrgLeft;
	DWORD   m_dwVersion;

//ウインドウの分離
	int m_ClientHeight[2];
	int m_ClientWidth[2];
	TStockView *pStockView;
	TStockView *pWaterView;
	TStockView *pLogView;
	void __fastcall DivStockView(void);
	void __fastcall AdjustStockView(void);
	void __fastcall MoveParent(TWinControl *pNew, TWinControl *pOld, TWinControl *pCtr);
	void __fastcall DivWaterView(void);
	void __fastcall DivControl(int sw);

	void __fastcall UpdatePriority(void);

	int     m_AppErr;

    HWND	m_hClipNext;
//CWMenu
	void __fastcall AdjustCWMenu(void);
	void __fastcall KCWSendClick(TObject *Sender);
	void __fastcall KCWEditClick(TObject *Sender);

//RadioMenu
public:
	int			m_nRadioMenu;
private:
	RADIOMENU	m_RadioMenu[RADIOMENUMAX];

	void __fastcall LoadRadioDef(LPCSTR pName);
    void __fastcall AdjustRadioMenu(void);
	void __fastcall KRadioCmdClick(TObject *Sender);
	void __fastcall KRadioEditClick(TObject *Sender);

//#define CM_FFT      WM_USER+403
#define CM_CRADIO   WM_USER+400
#define	CM_CMML		WM_USER+401
#define	CM_CMMR		WM_USER+402
#define	CM_EDITEXIT	WM_USER+404
protected:
	void __fastcall CMfft(TMessage *Message);
	void __fastcall OnMove(TMessage *Message);
	void __fastcall CMMML(TMessage Message);
	void __fastcall CMMMR(TMessage Message);
	void __fastcall OnEditExit(TMessage Message);
	void __fastcall OnDrawClipboard(TMessage Message);
	void __fastcall OnChangeCbChain(TMessage Message);

BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(CM_FFT, TMessage*, CMfft)
	MESSAGE_HANDLER(WM_MOVE, TMessage*, OnMove)
	MESSAGE_HANDLER(CM_CMML, TMessage, CMMML)
	MESSAGE_HANDLER(CM_CMMR, TMessage, CMMMR)
	MESSAGE_HANDLER(CM_EDITEXIT, TMessage, OnEditExit)
    MESSAGE_HANDLER(WM_DRAWCLIPBOARD, TMessage, OnDrawClipboard)
    MESSAGE_HANDLER(WM_CHANGECBCHAIN, TMessage, OnChangeCbChain)
END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TMmsstv *Mmsstv;
//---------------------------------------------------------------------------
#endif

 
