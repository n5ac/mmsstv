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

#include "PlayDlg.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
//TPlayDlgBox *PlayDlgBox;
//---------------------------------------------------------------------
__fastcall TPlayDlgBox::TPlayDlgBox(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;

	pWave = NULL;
	m_DisEvent = 0;

	if( MsgEng ){
		Caption = "Adjust Play position";
		OKBtn->Caption = "Hide";
		StopBtn->Caption = "Stop";
		LRec->Caption = "RecTime";
		LNow->Caption = "NowTime";
	}
}
//---------------------------------------------------------------------
void __fastcall TPlayDlgBox::UpdateItem(void)
{
	char bf[32];

	sprintf(bf, "%u[s]", int(pWave->m_length * 0.5 / SampFreq));
	LTime->Caption = bf;
	sprintf(bf, "%u[s]", int(pWave->GetPos() * 0.5 / SampFreq));
	LPos->Caption = bf;
	m_DisEvent++;
	ScrollBar->Max = int(pWave->m_length * 0.5 / SampFreq);
	ScrollBar->Position = int(pWave->GetPos() * 0.5 / SampFreq);
	CheckPause->Checked = pWave->m_pause;
	m_DisEvent--;
}
//---------------------------------------------------------------------
void __fastcall TPlayDlgBox::Execute(CWaveFile *wp)
{
	pWave = wp;
	pWave->m_autopause = 1;
	LName->Caption = pWave->m_FileName;
	UpdateItem();
	ShowModal();
	pWave->m_autopause = 0;
}
//---------------------------------------------------------------------
void __fastcall TPlayDlgBox::TimerTimer(TObject *Sender)
{
	UpdateItem();
}
//---------------------------------------------------------------------------
void __fastcall TPlayDlgBox::ScrollBarChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	int pos = ScrollBar->Position;
	pWave->Seek(pos * SampFreq*2);
	if( !CheckPause->Checked ) pWave->m_pause = 0;
	char bf[32];
	sprintf(bf, "%u[s]", pos);
	LPos->Caption = bf;
}
//---------------------------------------------------------------------------
void __fastcall TPlayDlgBox::CheckPauseClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	pWave->m_pause = CheckPause->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TPlayDlgBox::StopBtnClick(TObject *Sender)
{
	pWave->FileClose();
	ModalResult = mrOk;
}
//---------------------------------------------------------------------------
