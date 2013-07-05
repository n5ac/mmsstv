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

#include "RepSet.h"
#include "Main.h"
#include "MacroKey.h"
#include "TextEdit.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TRepSetDlg *RepSetDlg;
const int _ToneTbl[]={750, 1000, 1477, 1500, 1633, 1750, 2100, 2300, 0};
//---------------------------------------------------------------------
__fastcall TRepSetDlg::TRepSetDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		CancelBtn->Caption = "Cancel";
	}
	int i;
	CBTone->Items->Clear();
	for( i = 0; _ToneTbl[i]; i++ ){
		char bf[32];
		sprintf(bf, "%u", _ToneTbl[i]);
		CBTone->Items->Add(bf);
	}
	CBMode->Items->Clear();
	for( i = 0; i < smEND; i++ ){
		CBMode->Items->Add(SSTVModeList[i]);
	}
}
//---------------------------------------------------------------------
void __fastcall TRepSetDlg::UpdateBtn(void)
{
	BBtn->Enabled = CBEnb->Checked;
}
//---------------------------------------------------------------------
void __fastcall TRepSetDlg::UpdateStat(void)
{
	CSSTVDEM *dp = Mmsstv->pDem;
	char bf[256];

	sprintf(bf, "State\t%u\tAns\t%u\r\nRX\t%u/%u\tTX\t%u\r\n",
		dp->m_repmode,
		dp->m_repANS,
		dp->m_repRLY,
		dp->m_repRX,
		dp->m_repTX
	);
	if( dp->m_RepSQ && !dp->m_Sync && !Mmsstv->SBTX->Down ){
		char bbf[64];
		sprintf(bbf, "Level\t%d\t", dp->m_repsig );
		strcat(bf, bbf);
	}
	if( sys.m_RepBeacon && dp->m_Repeater ){
		char bbf[64];
		sprintf(bbf, "Beacon\t%d,  %ds",
			Mmsstv->m_RepBeaconCount,
			sys.m_RepBeacon - ((::GetTickCount() - Mmsstv->m_RepTick)/1000)
		);
		strcat(bf, bbf);
	}
	LS->Caption = bf;
}
//---------------------------------------------------------------------
int __fastcall TRepSetDlg::Execute(void)
{
	CBEnb->Checked = Mmsstv->pDem->m_Repeater;
	LS->Enabled = CBEnb->Checked;

	AnsCW->Text = sys.m_RepAnsCW;
	TA->Text = sys.m_RepTimeA;
	TB->Text = sys.m_RepTimeB;
	TC->Text = sys.m_RepTimeC;
	TD->Text = sys.m_RepTimeD;
	RGS->ItemIndex = sys.m_RepSenseLvl;
	ETempT->Text = sys.m_RepTempTX;
	ETempB->Text = sys.m_RepTempBeacon;
	EFD->Text = sys.m_RepFolder;

	CBMode->ItemIndex = sys.m_RepBeaconMode;
	ESQ->Text = Mmsstv->pDem->m_RepSQ;
	CBQT->Text = sys.m_RepQuietnessTime / 60;
	CBTone->Text = Mmsstv->pDem->m_RepTone;
	CBAS->Checked = sys.m_RepBottomAdj;
	CBFLT->Checked = sys.m_RepBeaconFilter;
	for( int i = 0; _ToneTbl[i]; i++ ){
		if( _ToneTbl[i] == Mmsstv->pDem->m_RepTone ){
			CBTone->ItemIndex = i;
			break;
		}
	}
	UpdateBtn();
	switch(sys.m_RepBeacon){
		case 10*60:
			CGB->ItemIndex = 1;
			break;
		case 20*60:
			CGB->ItemIndex = 2;
			break;
		case 30*60:
			CGB->ItemIndex = 3;
			break;
		case 60*60:
			CGB->ItemIndex = 4;
			break;
		case 120*60:
			CGB->ItemIndex = 5;
			break;
		default:
			CGB->ItemIndex = 0;
	}
	UpdateStat();
	int r = ShowModal();
	if( r != mrCancel ){
		int d;
		sscanf(AnsiString(TA->Text).c_str(), "%u", &d);	//ja7ude 0428
		if( d >= 100 ) sys.m_RepTimeA = d;
		sscanf(AnsiString(TB->Text).c_str(), "%u", &d);	//ja7ude 0428
		if( d >= 100 ) sys.m_RepTimeB = d;
		sscanf(AnsiString(TC->Text).c_str(), "%u", &d);	//ja7ude 0428
		if( d >= 100 ) sys.m_RepTimeC = d;
		sscanf(AnsiString(TD->Text).c_str(), "%u", &d);	//ja7ude 0428
		if( d >= 100 ) sys.m_RepTimeD = d;
		sys.m_RepAnsCW = AnsCW->Text;
		sys.m_RepSenseLvl = RGS->ItemIndex;
		Mmsstv->pDem->SetRepSenseLvl();

		sys.m_RepTempTX = ETempT->Text;
		sys.m_RepTempBeacon = ETempB->Text;
		sys.m_RepFolder = EFD->Text;

		sscanf(AnsiString(ESQ->Text).c_str(), "%u", &d);	//ja7ude 0428
		if( (d >= 0) && (d <= 32767) ) Mmsstv->pDem->m_RepSQ = d;

		sscanf(AnsiString(CBQT->Text).c_str(), "%u", &d);	//ja7ude 0428
		if( (d >= 0) && (d <= 20) ) sys.m_RepQuietnessTime = d * 60;
		switch(CGB->ItemIndex){
			case 1:
				sys.m_RepBeacon = 10*60;
				break;
			case 2:
				sys.m_RepBeacon = 20*60;
				break;
			case 3:
				sys.m_RepBeacon = 30*60;
				break;
			case 4:
				sys.m_RepBeacon = 60*60;
				break;
			case 5:
				sys.m_RepBeacon = 120*60;
				break;
			default:
				sys.m_RepBeacon = 0;
				break;
		}
		sys.m_RepBeaconMode = CBMode->ItemIndex;
		sys.m_RepBottomAdj = CBAS->Checked;
		sys.m_RepBeaconFilter = CBFLT->Checked;
		sscanf(AnsiString(CBTone->Text).c_str(), "%u", &d);	//ja7ude 0428
		if( (d >= 100) && (d < 2700) ){
			Mmsstv->pDem->m_RepTone = d;
		}
		Mmsstv->pDem->InitRepeater();
		Mmsstv->pDem->SetRepeater(CBEnb->Checked);
	}
	return r;
}
//---------------------------------------------------------------------
void __fastcall TRepSetDlg::MBtnClick(TObject *Sender)
{
	TMacroKeyDlg *pBox = new TMacroKeyDlg(this);
	AnsiString as = AnsCW->Text;
	if( pBox->Execute(as) > 0 ){
		AnsCW->SetFocus();
		AnsCW->SelStart = strlen(AnsiString(AnsCW->Text).c_str());	//ja7ude 0428
		AnsCW->SelLength = 0;
		for( LPCSTR p = as.c_str(); *p; p++ ){
			::PostMessage(AnsCW->Handle, WM_CHAR, *p, 0);
		}
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TRepSetDlg::TimerTimer(TObject *Sender)
{
	UpdateStat();
}
//---------------------------------------------------------------------------
void __fastcall TRepSetDlg::BBtnClick(TObject *Sender)
{
	ModalResult = 1024;
}
//---------------------------------------------------------------------------
void __fastcall TRepSetDlg::CBEnbClick(TObject *Sender)
{
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TRepSetDlg::HBtnClick(TObject *Sender)
{
	ShowHelp(this, "Repeater.txt");
}
//---------------------------------------------------------------------------


