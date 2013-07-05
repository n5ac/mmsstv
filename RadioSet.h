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
#ifndef RadioSetH
#define RadioSetH
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
//----------------------------------------------------------------------------
#include "Cradio.h"
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
//----------------------------------------------------------------------------
class TRADIOSetDlg : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TGroupBox *GB1;
	TComboBox *PortName;
	TLabel *Label1;
	TLabel *Label2;
	TComboBox *Baud;
	TRadioGroup *BitLen;
	TRadioGroup *Stop;
	TRadioGroup *Parity;
	TGroupBox *GroupBox3;
	TCheckBox *flwXON;
	TCheckBox *flwCTS;
	TLabel *Label3;
	TComboBox *ByteWait;
	TLabel *Label8;
	TButton *LoadBtn;
	TButton *SaveBtn;
	TComboBox *Maker;
	TGroupBox *GGNR;
	TLabel *LGNR;
	TEdit *CmdGNR;
	TButton *RefBtn;
	TCheckBox *OpenGNR;
	TLabel *LPTT;
	TGroupBox *GCmd;
	TLabel *Label4;
	TLabel *Label5;
	TLabel *Label6;
	TLabel *Lxx;
	TLabel *LVFO;
	TEdit *CmdInit;
	TEdit *CmdRx;
	TEdit *CmdTx;
	TEdit *Cmdxx;
	TComboBox *VFOType;
	TComboBox *PollInt;
	TLabel *LInt;
	TLabel *LInts;
	TCheckBox *AddrScan;
	TGroupBox *GB2;
	TCheckBox *CBPTT;
	void __fastcall LoadBtnClick(TObject *Sender);
	void __fastcall SaveBtnClick(TObject *Sender);
	void __fastcall MakerChange(TObject *Sender);
	void __fastcall PortNameChange(TObject *Sender);
	void __fastcall RefBtnClick(TObject *Sender);
	
	void __fastcall VFOTypeChange(TObject *Sender);
	
	
private:
	int				m_DisEvent;
	int __fastcall IsXX(void);
	int __fastcall IsCompatible(int PollType, int MakerIndex);
	int __fastcall IsSame(LPCSTR v, LPCSTR t);
	void __fastcall SetMaker(void);
	void __fastcall UpdateUI(void);
	void __fastcall SetVFOList(void);
	int __fastcall GetVFOType(LPCSTR pKey);
	LPCSTR __fastcall GetVFOName(int r);
	int				m_Maker;
	int				m_PollType;
	int				m_InitWidth;
	CMMList			m_MMList;
public:
	virtual __fastcall TRADIOSetDlg(TComponent* AOwner);
	int __fastcall Execute(void);
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif    
