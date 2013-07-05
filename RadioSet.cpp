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

#include "RadioSet.h"
#include "Main.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
LPCSTR	__MK[]={
	"YAESU FT-1000D, 1000MP, 920",                  //AA6YQ 1.66 cosmetic, MakerIndex=0
    "YAESU FT-9000, 2000, 950, 450",                //AA6YQ 1.66         , MakerIndex=1
	"YAESU FT-736, 817, 847, 857, 897",                            //AA6YQ 1.66 cosmetic, MakerIndex=2
	"Icom  xx=addr 01-7F",                          //                   , MakerIndex=3
	"Ten-Tec Omni VI  xx=addr 00-64",               //                   , MakerIndex=4
	"Kenwood, Elecraft ",                                      //                   , MakerIndex=5
	"JRC JST-245",                                  //                   , MakerIndex=6
	"Clear",                                        //                   , MakerIndex=6
	NULL,                                           //                   , MakerIndex=8
};
LPCSTR	__MT[][3]={
	{"", "\\$000000000F", "\\$000000010F\\w10" },
    {"", "TX0;", "TX1;\\w10" },                       //AA6YQ 1.66
	{"\\$0000000000", "\\$0000000088", "\\$0000000008\\w10" },
	{"", "\\$FEFExxE01C0000FD", "\\$FEFExxE01C0001FD\\w10" },
	{"", "\\$FEFExxE01602FD", "\\$FEFExxE01601FD\\w10" },
	{"", "RX;", "TX;\\w10" },
	{"", "H1\\rX0\\rH0\\r", "H1\\rX1\\rH0\\r\\w10" },
	{"", "", "" },
};

#define	MAKER_UNKNOWN	7

typedef struct {
	LPCSTR	pKey;
	int		r;
}POLLDEF;

const POLLDEF	__VT0[]={
	{ "NONE", 0 },
	{ "FT-1000MP", RADIO_POLLYAESUHF },
	{ "FT-1000D", RADIO_POLLFT1000D },
	{ "FT-920", RADIO_POLLFT920 },
	{ NULL, 0 },
};
const POLLDEF	__VT1[]={
	{ "NONE", 0 },
    { "FT-9000", RADIO_POLLFT9000 },    //1.66B AA6YQ
    { "FT-2000", RADIO_POLLFT2000 },    //1.66B AA6YQ
    { "FT-950", RADIO_POLLFT950 },      //1.66B AA6YQ
    { "FT-450", RADIO_POLLFT450 },      //1.66B AA6YQ
	{ NULL, 0 },
};
const POLLDEF	__VT2[]={
	{ "NONE", 0 },
	{ "FT-817, 847, 857, 897", RADIO_POLLYAESUVU },
	{ NULL, 0 },
};
const POLLDEF	__VT3[]={
	{ "NONE", 0 },
	{ "ICOM CI-V", RADIO_POLLICOM },
	{ "ICOM CI-V (no inquiry)", RADIO_POLLICOMN },
	{ NULL, 0 },
};
const POLLDEF	__VT4[]={
	{ "NONE", 0 },
	{ "Ten-Tec Omni VI", RADIO_POLLOMNIVI },
	{ "Ten-Tec Omni VI (no inquiry)", RADIO_POLLOMNIVIN },
	{ NULL, 0 },
};
const POLLDEF	__VT5[]={
	{ "NONE", 0 },
	{ "KENWOOD", RADIO_POLLKENWOOD },
	{ "KENWOOD (use auto info)", RADIO_POLLKENWOODN },
	{ NULL, 0 },
};
const POLLDEF	__VT6[]={
	{ "NONE", 0 },
	{ "JST245", RADIO_POLLJST245 },
	{ "JST245 (use auto info)", RADIO_POLLJST245N },
	{ NULL, 0 },
};
const POLLDEF	__VTUNKNOWN[]={
	{ "NONE", 0 },
	{ "YAESU FT-1000MP", RADIO_POLLYAESUHF },
	{ "YAESU FT-1000D", RADIO_POLLFT1000D },
	{ "YAESU FT-920", RADIO_POLLFT920 },
	{ "YAESU FT-817, 847, 857, 897", RADIO_POLLYAESUVU },
    { "YAESU FT-9000", RADIO_POLLFT9000 },  //1.66B AA6YQ
    { "YAESU FT-2000", RADIO_POLLFT2000 },  //1.66B AA6YQ
    { "YAESU FT-950", RADIO_POLLFT950 },    //1.66B AA6YQ
    { "YAESU FT-450", RADIO_POLLFT450 },    //1.66B AA6YQ
	{ "ICOM CI-V", RADIO_POLLICOM },
	{ "ICOM CI-V (no inquiry)", RADIO_POLLICOMN },
	{ "Ten-Tec Omni VI", RADIO_POLLOMNIVI },
	{ "Ten-Tec Omni VI (no inquiry)", RADIO_POLLOMNIVIN },
	{ "KENWOOD", RADIO_POLLKENWOOD },
	{ "KENWOOD (use auto info)", RADIO_POLLKENWOODN },
	{ "JST245", RADIO_POLLJST245 },
	{ "JST245 (use auto info)", RADIO_POLLJST245N },
	{ NULL, 0 },
};
const POLLDEF	*__VL[]={
	__VT0, __VT1, __VT2, __VT3, __VT4, __VT5, __VT6, __VTUNKNOWN, NULL,
};
//---------------------------------------------------------------------
__fastcall TRADIOSetDlg::TRADIOSetDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		CancelBtn->Caption = "Cancel";
	}
    else {
		Caption = "リグコントロール(Radio command)";
		GB1->Caption = "ポートの設定";
    }
	int i;
	for( i = 0; __MK[i] != NULL; i++ ){
		Maker->Items->Add(__MK[i]);
	}
	m_InitWidth = CmdInit->Width;
	m_PollType = 0;
	m_Maker = 0;
	m_MMList.QueryList("MMR");
	for( int i = 0; i < m_MMList.GetCount(); i++ ){
		PortName->Items->Insert(1, m_MMList.GetItemName(i));
	}
	PortName->DropDownCount = PortName->Items->Count;
	m_DisEvent = 0;
}
//---------------------------------------------------------------------
void __fastcall TRADIOSetDlg::UpdateUI(void)
{
	int f;
	if( !strcmpi(AnsiString(PortName->Text).c_str(), "PSKGNR")||	//ja7ude 0428
		!strcmpi(AnsiString(PortName->Text).c_str(), "WD5GNR")||	//ja7ude 0428
		!strcmpi(AnsiString(PortName->Text).c_str(), "LOGGER") ){	//ja7ude 0428
		GCmd->Visible = FALSE;
		if( !strcmpi(AnsiString(PortName->Text).c_str(), "LOGGER") ){	//ja7ude 0428
			GGNR->Visible = FALSE;
//			LTNX->Caption = "Thanks to Bob Furzer (K4CY) who is releasing the fine programs.";
		}
		else {
			GGNR->Visible = TRUE;
//            LTNX->Caption = "Thanks to Al Williams (WD5GNR) who is releasing the fine programs.";
		}
		LGNR->Visible = OpenGNR->Checked;
		CmdGNR->Visible = OpenGNR->Checked;
		RefBtn->Visible = OpenGNR->Checked;
		LPTT->Visible = OpenGNR->Checked;
		Maker->Visible = FALSE;
		LoadBtn->Visible = FALSE;
		SaveBtn->Visible = FALSE;
		f = FALSE;
	}
	else {
		GGNR->Visible = FALSE;
		GCmd->Visible = TRUE;
		Maker->Visible = TRUE;
		LoadBtn->Visible = TRUE;
		SaveBtn->Visible = TRUE;
		f = IsXX();
		Lxx->Visible = f;
		Cmdxx->Visible = f;
		AddrScan->Visible = f && m_PollType;
		if( f ){
			CmdInit->Width = m_InitWidth;
		}
		else {
			CmdInit->Width = CmdRx->Width;
		}
		f = (m_MMList.IndexOf(AnsiString(PortName->Text).c_str()) < 0) ? TRUE : FALSE;	//ja7ude 0428
	}
	Label2->Enabled = f;
	Baud->Enabled = f;
	BitLen->Enabled = f;
	Stop->Enabled = f;
	Parity->Enabled = f;
	GroupBox3->Enabled = f;
	flwXON->Enabled = f;
	flwCTS->Enabled = f && !CBPTT->Checked;
	CBPTT->Enabled = f;
	Label3->Enabled = f;
	ByteWait->Enabled = f;
	Label8->Enabled = f;
	f = TRUE;
	LVFO->Visible = f;
	VFOType->Visible = f;
	LInt->Visible = f;
	LInts->Visible = f;
	PollInt->Visible = f;
	LVFO->Enabled = f;
	VFOType->Enabled = f;
	LInt->Enabled = f;
	LInts->Enabled = f;
	PollInt->Enabled = f;
}
//---------------------------------------------------------------------
int __fastcall TRADIOSetDlg::Execute(void)
{
	PortName->Text = RADIO.StrPort;
	Baud->Text = RADIO.BaudRate;
	BitLen->ItemIndex = RADIO.BitLen;
	Parity->ItemIndex = RADIO.Parity;
	Stop->ItemIndex = RADIO.Stop;
	flwXON->Checked = RADIO.flwXON;
	flwCTS->Checked = RADIO.flwCTS;
	CBPTT->Checked = RADIO.usePTT;

	ByteWait->Text = int(RADIO.ByteWait);

	char bf[32];
	sprintf( bf, "%02X", RADIO.Cmdxx);
	Cmdxx->Text = bf;
	CmdInit->Text = RADIO.CmdInit;
	CmdRx->Text = RADIO.CmdRx;
	CmdTx->Text = RADIO.CmdTx;

	CmdGNR->Text = RADIO.cmdGNR;
	OpenGNR->Checked = RADIO.openGNR;
	m_PollType = RADIO.PollType;
	PollInt->Text = (RADIO.PollInterval + 5)/10.0;
	AddrScan->Checked = RADIO.PollScan;
	SetMaker();
	UpdateUI();
	if( ShowModal() == IDOK ){
		StrCopy(RADIO.StrPort, AnsiString(PortName->Text).c_str(), 31);	//ja7ude 0428
		int dd;
		if( sscanf(AnsiString(Baud->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0428
			RADIO.BaudRate = dd;
		}
		RADIO.BitLen = BitLen->ItemIndex;
		RADIO.Parity = Parity->ItemIndex;
		RADIO.Stop = Stop->ItemIndex;
		RADIO.flwXON = flwXON->Checked;
		RADIO.flwCTS = flwCTS->Checked;
		RADIO.usePTT = CBPTT->Checked;

		if( sscanf(AnsiString(ByteWait->Text).c_str(), "%u", &dd) == 1 ){	//ja7ude 0428
			if( (dd >= 0) && (dd <= 1000) ){
				RADIO.ByteWait = dd;
			}
		}
		if( sscanf(AnsiString(Cmdxx->Text).c_str(), "%X", &dd) == 1 ){	//ja7ude 0428
			RADIO.Cmdxx = dd & 0x000000ff;
		}
		RADIO.CmdInit = CmdInit->Text;
		RADIO.CmdRx = CmdRx->Text;
		RADIO.CmdTx = CmdTx->Text;

		RADIO.cmdGNR = CmdGNR->Text;
		RADIO.openGNR = OpenGNR->Checked;

		RADIO.PollType = m_PollType;
		double d;
		if( sscanf(AnsiString(PollInt->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0428
			if( d >= 0.5 ){
				RADIO.PollInterval = int((d * 10.0) - 5);
				if( RADIO.PollInterval < 0 ) RADIO.PollInterval = 0;
			}
		}
		RADIO.PollScan = AddrScan->Checked;
		RADIO.change = 1;
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------------
void __fastcall TRADIOSetDlg::LoadBtnClick(TObject *Sender)
{
//
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
		CWaitCursor w;
		TMemIniFile *pIniFile = new TMemIniFile(pBox->FileName);
		CmdInit->Text = pIniFile->ReadString("RADIO", "CmdInit", CmdInit->Text);
		CmdRx->Text = pIniFile->ReadString("RADIO", "CmdRx", CmdRx->Text);
		CmdTx->Text = pIniFile->ReadString("RADIO", "CmdTx", CmdTx->Text);
		Cmdxx->Text = pIniFile->ReadString("RADIO", "Cmdxx", Cmdxx->Text);
		m_PollType = pIniFile->ReadInteger("RADIO", "PollType", 0);
		PollInt->Text = (pIniFile->ReadInteger("RADIO", "PollInterval", 0) + 5)/10.0;
		AddrScan->Checked = 0;
		delete pIniFile;
		SetMaker();
		UpdateUI();
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TRADIOSetDlg::SaveBtnClick(TObject *Sender)
{
	TSaveDialog *pBox = Mmsstv->SaveDialog;
	if( MsgEng ){
		pBox->Title = "Save command file";
		pBox->Filter = "Command files(*.rcm)|*.rcm|";
	}
	else {
		pBox->Title = "コマンド定義ファイルの作成";
		pBox->Filter = "コマンド定義ファイル(*.rcm)|*.rcm|";
	}
	pBox->FileName = "MyRIG.rcm";
	pBox->DefaultExt = "rcm";
	pBox->InitialDir = BgnDir;
	NormalWindow(this);
	if( pBox->Execute() == TRUE ){
		CWaitCursor w;
		TMemIniFile *pIniFile = new TMemIniFile(pBox->FileName);
		pIniFile->WriteString("RADIO", "CmdInit", CmdInit->Text);
		pIniFile->WriteString("RADIO", "CmdRx", CmdRx->Text);
		pIniFile->WriteString("RADIO", "CmdTx", CmdTx->Text);
		pIniFile->WriteString("RADIO", "Cmdxx", Cmdxx->Text);
		pIniFile->WriteInteger("RADIO", "PollType", m_PollType);
		double d;
		int dd = 5;
		if( sscanf(AnsiString(PollInt->Text).c_str(), "%lf", &d) == 1 ){	//ja7ude 0428
			if( d >= 0.5 ){
				dd = int((d * 10.0) - 5);
				if( dd < 0 ) dd = 0;
			}
		}
		pIniFile->WriteInteger("RADIO", "PollInterval", dd);
		pIniFile->UpdateFile();
		delete pIniFile;
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
int __fastcall TRADIOSetDlg::IsXX(void)
{
	if( strstr(AnsiString(Maker->Text).c_str(), "xx") != NULL ) return 1;	//ja7ude 0428
    if( m_PollType == RADIO_POLLICOM ) return 1;
	if( m_PollType == RADIO_POLLICOMN ) return 1;
	if( m_PollType == RADIO_POLLOMNIVI ) return 1;
	if( m_PollType == RADIO_POLLOMNIVIN ) return 1;
	return 0;
}
//---------------------------------------------------------------------------
int __fastcall TRADIOSetDlg::IsCompatible(int PollType, int MakerIndex)
{

    if (MakerIndex == 5)    {       //MakerIndex 5 is Kenwood
        if (PollType == 0){
            return 1;
        }
        else if (PollType == RADIO_POLLKENWOOD) {
            return 1;
        }
        else if (PollType == RADIO_POLLKENWOODN) {
            return 1;
        }
        else {
            return 0;
        }
    }
    else if (MakerIndex == 1)   {  //MakerIndex 1 is Yaesu FT-9000 et al
        if (PollType == 0){
            return 1;
        }
        else if (PollType == RADIO_POLLFT9000) {
            return 1;
        }
        else if (PollType == RADIO_POLLFT2000) {
            return 1;
        }
        else if (PollType == RADIO_POLLFT950) {
            return 1;
        }
        else if (PollType == RADIO_POLLFT450) {
            return 1;
        }
        else {
            return 0;
        }

    }
    else {
        return 1;
    }
}
//---------------------------------------------------------------------------
int __fastcall TRADIOSetDlg::IsSame(LPCSTR t, LPCSTR v)
{
	for( ; *v && *t; v++, t++ ){
		if( (*v=='x')&&(*(v+1)=='x') ){
			t++;
			v++;
		}
		else if( (*t == '\\')&&(*(t+1) == 'w') ){
			return 1;
		}
		else if( (*v == '\\')&&(*(v+1) == 'w') ){
			return 1;
		}
		else if( *t != *v ){
			return 0;
		}
	}
	return *t != *v ? 0 : 1;
}
//---------------------------------------------------------------------------
int __fastcall TRADIOSetDlg::GetVFOType(LPCSTR pKey)
{
	for( int i = 0; __VL[i] != NULL; i++ ){
		const POLLDEF	*p = __VL[i];
		for( ; p->pKey != NULL; p++ ){
			if( !strcmp(p->pKey, pKey) ) return p->r;
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
LPCSTR __fastcall TRADIOSetDlg::GetVFOName(int r)
{
	for( int i = 0; __VL[i] != NULL; i++ ){
		const POLLDEF	*p = __VL[i];
		for( ; p->pKey != NULL; p++ ){
			if( p->r == r ) return p->pKey;
		}
	}
	return "NONE";
}
//---------------------------------------------------------------------------
void __fastcall TRADIOSetDlg::SetVFOList(void)
{
	m_DisEvent++;
	const POLLDEF	*p = __VL[m_Maker];
	VFOType->Items->Clear();
	for( ; p->pKey != NULL; p++ ){
		VFOType->Items->Add(p->pKey);
	}
	VFOType->Text = GetVFOName(m_PollType);
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TRADIOSetDlg::SetMaker(void)
{
	int i;
	for( i = 0; __MK[i] != NULL; i++ ){
		if( IsSame(AnsiString(CmdRx->Text).c_str(), __MT[i][1]) && IsSame(AnsiString(CmdTx->Text).c_str(), __MT[i][2]) && IsCompatible (m_PollType,i)){       //AA6YQ 1.66	//ja7ude 0428
			m_DisEvent++;
			Maker->Text = __MK[i];
			m_DisEvent--;
			m_Maker = i;
			SetVFOList();
			return;
		}
	}
	m_Maker = MAKER_UNKNOWN;
	m_DisEvent++;
	Maker->Text = "Unknown";
	SetVFOList();
	m_DisEvent--;
}

void __fastcall TRADIOSetDlg::MakerChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	m_PollType = 0;
	VFOType->Text = "NONE";
	PollInt->Text = "1";
	AddrScan->Checked = 0;
	for( int i = 0; __MK[i] != NULL; i++ ){
		if( Maker->Text == __MK[i] ){
			CmdInit->Text = __MT[i][0];
			CmdRx->Text = __MT[i][1];
			CmdTx->Text = __MT[i][2];
			m_Maker = i;
			if( m_Maker == 3 ){		// Ten-Tec Omni-VI
				Cmdxx->Text = "04";
			}
			SetVFOList();
			break;
		}
	}
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TRADIOSetDlg::PortNameChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TRADIOSetDlg::RefBtnClick(TObject *Sender)
{
	TOpenDialog *pBox = Mmsstv->OpenDialog;
	pBox->Options >> ofCreatePrompt;
	pBox->Options << ofFileMustExist;
	if( MsgEng ){
		pBox->Title = "Program file";
		pBox->Filter = "Program files(*.exe;*.lnk)|*.exe;*.lnk|";
	}
	else {
		pBox->Title = "プログラムファイル";
		pBox->Filter = "プログラムファイル(*.exe;*.lnk)|*.exe;*.lnk|";
	}
	pBox->FileName = CmdGNR->Text.c_str();
	pBox->DefaultExt = "exe";
	char bf[256];
	SetDirName(bf, AnsiString(CmdGNR->Text).c_str());	//ja7ude 0428
	pBox->InitialDir = bf;
	NormalWindow(this);
	if( pBox->Execute() == TRUE ){
		CmdGNR->Text = pBox->FileName.c_str();
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TRADIOSetDlg::VFOTypeChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	m_PollType = GetVFOType(AnsiString(VFOType->Text).c_str());	//ja7ude 0428
	UpdateUI();
}
//---------------------------------------------------------------------------
