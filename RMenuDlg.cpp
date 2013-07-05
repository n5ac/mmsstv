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

#include "RMenuDlg.h"
#include "Main.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
//TRMenuDialog *RMenuDialog;
static LPCSTR g_help={
"コマンド\r\n"
"\r\n"
"\\$##...	##=00-FF,  送信する複数バイトを16進数で定義する\r\n"
"		(例:\\$FE55AA  -> FEh,55h,AAh)\r\n"
"\\x##	##=00-FF,  送信する１バイトを16進数で定義する\r\n"
"		(例:\\xFE\\x55\\xAA  -> FEh, 55h, AAh)\r\n"
"\\w##	##=00-99,  ディレイ時間を10ms単位の10進数で定義する\r\n"
"		(例:\\w05  -> wait 50ms)\r\n"
"\\r	キャリッジリターンを送信する\r\n"
"\\n	ラインフィードを送信する\r\n"
"\\c....	以降はすべてコメント\r\n"
"\\\\	'\\'文字を送信する\r\n"
"その他	その文字列を送信する\r\n"
"\r\n"
"##の代わりにxxを記述すると、その部分はxx入力ボックスで定義した16進数に置き換わります。\r\n"
"この変換はICOMの機種アドレスを指定する場合に使用します。\r\n"
"		例:\\$FEFExxE01C0001FD	（xxの部分が定義内容に置き換わる）"
};
static LPCSTR g_helpeng={
" This command sends a radio command to the rig. It will work for any rig. Use the following keyword to specify byte data in hexadecimal number.\r\n"
"\r\n"
"\\$##...		##=00-FF, Specify the byte data string in the hexadecimal format\r\n"
"			(Example: \\$FE55AA  -> $FE,$55,$AA)\r\n"
"			ICOM CI-V address can be expressed by xx\r\n"
"\\x##		##=00-FF, Specify one byte in the hexadecimal format\r\n"
" 			(Example: \\xFE\\x55\\xAA  -> $FE, $55, $AA)\r\n"
"\\w##		##=00-99, Specify the delay time\r\n"
"			(Example: \\w05  -> wait 50ms)\r\n"
"\\r		Send a carriage return\r\n"
"\\n		Send a line feed\r\n"
"\\c....		Comment\r\n"
"\\\\		'\\' send character\r\n"
"Others		Send the character as is"
};

//---------------------------------------------------------------------
__fastcall TRMenuDialog::TRMenuDialog(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;

	if( MsgEng ){
		LH->Font->Name = "Arial";
		LH->Font->Charset = ((TForm *)AOwner)->Font->Charset;

		L1->Caption = "Caption";
        L2->Caption = "Command";
		Caption = "Edit menu";
		SBDel->Caption = "Delete this menu";
		SBIns->Caption = "Insert this menu";
        CancelBtn->Caption = "Cancel";
		LH->Caption = g_helpeng;
    }
    else {
		LH->Caption = g_help;
    }
}
//---------------------------------------------------------------------
void __fastcall TRMenuDialog::UpdateUI(void)
{
	if( SBIns->Visible ){
		SBIns->Enabled = Mmsstv->m_nRadioMenu < RADIOMENUMAX;
    }
}
//---------------------------------------------------------------------
int __fastcall TRMenuDialog::Execute(AnsiString &strTTL, AnsiString &strCMD, int fAdd)
{
	if( fAdd ){
		Caption = MsgEng ? "Add menu" : "メニューの追加";
		SBDel->Visible = FALSE;
		SBIns->Visible = FALSE;
    }
	ETTL->Text = strTTL;
    ECMD->Text = strCMD;
    UpdateUI();
    int r = ShowModal();
	strTTL = ETTL->Text;
    strCMD = ECMD->Text;
	return r;
}
//---------------------------------------------------------------------
void __fastcall TRMenuDialog::SBDelClick(TObject *Sender)
{
	ModalResult = 1024;
}
//---------------------------------------------------------------------------
void __fastcall TRMenuDialog::SBInsClick(TObject *Sender)
{
	ModalResult = 1025;
}
//---------------------------------------------------------------------------
void __fastcall TRMenuDialog::SBRefClick(TObject *Sender)
{
	TOpenDialog *pBox = new TOpenDialog(this);
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
	if( pBox->Execute() == TRUE ){
		ECMD->Text = pBox->FileName.c_str();
	}
    delete pBox;
}
//---------------------------------------------------------------------------

