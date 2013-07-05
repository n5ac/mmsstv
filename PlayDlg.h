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
#ifndef PlayDlgH
#define PlayDlgH
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

#include "Sound.h"
//----------------------------------------------------------------------------
class TPlayDlgBox : public TForm
{
__published:        
	TButton *OKBtn;
	TScrollBar *ScrollBar;
	TLabel *LRec;
	TLabel *LNow;
	TLabel *LTime;
	TLabel *LPos;
	TTimer *Timer;
	TLabel *LName;
	TCheckBox *CheckPause;
	TButton *StopBtn;
	void __fastcall TimerTimer(TObject *Sender);
	void __fastcall ScrollBarChange(TObject *Sender);
	void __fastcall CheckPauseClick(TObject *Sender);
	void __fastcall StopBtnClick(TObject *Sender);
private:
	CWaveFile	*pWave;

	int			m_DisEvent;
public:
	virtual __fastcall TPlayDlgBox(TComponent* AOwner);

	void __fastcall Execute(CWaveFile *wp);
	void __fastcall UpdateItem(void);

};
//----------------------------------------------------------------------------
//extern TPlayDlgBox *PlayDlgBox;
//----------------------------------------------------------------------------
#endif    
