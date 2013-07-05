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

#include "VerDsp.h"
#include "ComLib.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TVerDspDlg *VerDspDlg;
//---------------------------------------------------------------------
__fastcall TVerDspDlg::TVerDspDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	IconImage->Picture->Assign(Application->Icon);
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	LFree->Font->Name = ((TForm *)AOwner)->Font->Name;
	LFree->Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		Caption = "Version Information";
		LFree->Caption = "Free Software";
	}
	Version->Caption = VERTTL;
	LInfo->Caption =
	"I would like to thank my friends in both groups\r\n\r\n"
	"  MM-SSTV@yahoogroups.com\r\n"
	"  mmhamsoft@egroups.co.jp\r\n\r\n"
	"I do not list all those names, for fear that I would forget one.\r\n"
	"And too small space in the window for writing all the name..."
	;
}
//---------------------------------------------------------------------
