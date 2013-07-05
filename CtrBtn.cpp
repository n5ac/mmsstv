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

#include "CtrBtn.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//TCtrBtnWnd *CtrBtnWnd;
//---------------------------------------------------------------------------
__fastcall TCtrBtnWnd::TCtrBtnWnd(TComponent* Owner)
	: TForm(Owner)
{
	Font->Name = sys.m_WinFontName;
	Font->Charset = sys.m_WinFontCharset;
	if( MsgEng ){
		SBRXStop->Caption = "Stop RX";
		SBLock->Caption = "Lock RX";
//        SBSync->Caption = "ReSync";
		SBHist->Caption = "CopyHist";
		SBTX->Caption = "TX(F9)";
	}
	SBSync->Caption = Mmsstv->KRFS->Caption;
	AlignView.EntryControl(SBRXStop, this, SBRXStop->Font);
	AlignView.EntryControl(SBLock, this, SBLock->Font);
	AlignView.EntryControl(SBSync, this, SBSync->Font);
	AlignView.EntryControl(SBHist, this, SBHist->Font);
	AlignView.EntryControl(SBTX, this, SBTX->Font);
	SBTX->OnMouseDown = Mmsstv->SBTXMouseDown;
//    Caption = "";
}
//---------------------------------------------------------------------------
void __fastcall TCtrBtnWnd::UpdateBtn(void)
{
	if( Mmsstv->pDem->m_Sync ){
		SBRXStop->Enabled = TRUE;
		SBSync->Enabled = (SSTVSET.m_Mode != smAVT) ? TRUE : FALSE;
	}
	else {
		SBRXStop->Enabled = (Mmsstv->pDem->m_SyncMode != -1) ? FALSE : TRUE;
		SBSync->Enabled = FALSE;
		SBRXStop->Down = SBRXStop->Enabled;
	}
	SBLock->Down = Mmsstv->SBLK->Down;
	SBHist->Enabled = Mmsstv->SBWHist->Enabled && !Mmsstv->pDem->m_Sync;
	SBTX->Down = Mmsstv->SBTX->Down;
	if( sys.m_Repeater ) SBTX->Update();
}
//---------------------------------------------------------------------------
void __fastcall TCtrBtnWnd::SBRXStopClick(TObject *Sender)
{
	Mmsstv->RxAutoPush(TRUE);
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TCtrBtnWnd::SBHistClick(TObject *Sender)
{
	Mmsstv->SBWHistClick(NULL);
	if( Mmsstv->pSound->m_Tx && !Mmsstv->SBTune->Down && Mmsstv->SBUseTemp->Down ){
		MultProc();
		Mmsstv->SBUseTemp->Down = FALSE;
		Mmsstv->SBUseTempClick(NULL);
	}
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TCtrBtnWnd::SBTXClick(TObject *Sender)
{
	Mmsstv->AdjustPage(pgTX);
	Mmsstv->SBTX->Down = Mmsstv->SBTX->Down ? FALSE : TRUE;
	Mmsstv->SBTXClick(NULL);
	UpdateBtn();
}
//---------------------------------------------------------------------------
void __fastcall TCtrBtnWnd::FormKeyDown(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	Mmsstv->FormKeyDown(Sender, Key, Shift);
}
//---------------------------------------------------------------------------
void __fastcall TCtrBtnWnd::FormKeyUp(TObject *Sender, WORD &Key,
	  TShiftState Shift)
{
	Mmsstv->FormKeyUp(Sender, Key, Shift);
}
//---------------------------------------------------------------------------
void __fastcall TCtrBtnWnd::FormResize(TObject *Sender)
{
	AlignView.NewAlign(this);
}
//---------------------------------------------------------------------------
void __fastcall TCtrBtnWnd::SBLockClick(TObject *Sender)
{
	Mmsstv->SBLK->Down = SBLock->Down;
    Mmsstv->SBLKClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TCtrBtnWnd::SBSyncClick(TObject *Sender)
{
	Mmsstv->KRFSClick(NULL);
}
//---------------------------------------------------------------------------

