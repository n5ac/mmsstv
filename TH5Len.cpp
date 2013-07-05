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

#include "TH5Len.h"
#include "ComLib.h"
#include "Hamlog5.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
//TTH5LenDlg *TH5LenDlg;
//---------------------------------------------------------------------
__fastcall TTH5LenDlg::TTH5LenDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	Caption = MsgEng ? "Set fields length" : "フィールド長の設定";
}
//---------------------------------------------------------------------
int __fastcall TTH5LenDlg::Execute(void)
{
	UHis->Position = Log.m_LogSet.m_Hamlog5Len[8];
	UMy->Position = Log.m_LogSet.m_Hamlog5Len[9];
	UFreq->Position = Log.m_LogSet.m_Hamlog5Len[10];
	UMode->Position = Log.m_LogSet.m_Hamlog5Len[11];
	UName->Position = Log.m_LogSet.m_Hamlog5Len[12];
	UQTH->Position = Log.m_LogSet.m_Hamlog5Len[13];
	URem->Position = Log.m_LogSet.m_Hamlog5Len[14];
    if( ShowModal() == IDOK ){
		Log.m_LogSet.m_Hamlog5Len[8] = UHis->Position;
		Log.m_LogSet.m_Hamlog5Len[9] = UMy->Position;
		Log.m_LogSet.m_Hamlog5Len[10] = UFreq->Position;
		Log.m_LogSet.m_Hamlog5Len[11] = UMode->Position;
		Log.m_LogSet.m_Hamlog5Len[12] = UName->Position;
		Log.m_LogSet.m_Hamlog5Len[13] = UQTH->Position;
		Log.m_LogSet.m_Hamlog5Len[14] = URem->Position;
		Log.m_LogSet.m_Hamlog5Len[15] = Log.m_LogSet.m_Hamlog5Len[14];
		return TRUE;
    }
    else {
		return FALSE;
    }
}
//---------------------------------------------------------------------

