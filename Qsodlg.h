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
#ifndef QsoDlgH
#define QsoDlgH
//----------------------------------------------------------------------------
/* JA7UDE 0428
#include <vcl\System.hpp>
#include <vcl\Windows.hpp>
#include <vcl\SysUtils.hpp>
#include <vcl\Classes.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Controls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\ExtCtrls.hpp>
*/
#include <Grids.hpp>
//----------------------------------------------------------------------------
#include "LogFile.h"
#include <Buttons.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
//----------------------------------------------------------------------------
class TQSODlgBox : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TGroupBox *GroupBox1;
	TStringGrid *Grid;
	TLabel *Label1;
	TEdit *EditDate;
	TLabel *LTimeZone;
	TEdit *EditBgn;
	TLabel *Label3;
	TEdit *EditEnd;
	TLabel *Label4;
	TEdit *EditCall;
	TLabel *Label5;
	TEdit *EditName;
	TLabel *Label6;
	TEdit *EditQTH;
	TLabel *Label7;
	TComboBox *EditMy;
	TLabel *Label8;
	TComboBox *EditHis;
	TLabel *Label9;
	TComboBox *EditBand;
	TLabel *Label10;
	TComboBox *EditMode;
	TLabel *Label11;
	TEdit *EditRem;
	TLabel *Label12;
	TEdit *EditQSL;
	TLabel *Label13;
	TLabel *Label14;
	TLabel *Label15;
	TEdit *EditEnv;
	TLabel *Label16;
	TEdit *EditOpt1;
	TLabel *Label17;
	TEdit *EditOpt2;
	TLabel *Label18;
	TComboBox *EditM;
	TComboBox *EditS;
	TComboBox *EditR;
	TLabel *Label19;
	TComboBox *EditPow;
	TLabel *LStat;
	TLabel *Label20;
	TEdit *EditUsr1;
	TLabel *Label21;
	TEdit *EditUsr2;
	TSpeedButton *SBMMCG;
	TButton *LogPicBtn;
	void __fastcall SBMMCGClick(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall EditDateChange(TObject *Sender);
	void __fastcall GridSelectCell(TObject *Sender, int Col, int Row,
		  bool &CanSelect);
	void __fastcall GridDrawCell(TObject *Sender, int Col, int Row,
		  TRect &Rect, TGridDrawState State);
	
	void __fastcall LogPicBtnClick(TObject *Sender);
private:
	CLogFind	*m_fp;
	int		m_EditFlag;

	int		m_CurNo;
	SDMMLOG	m_sd;

	AnsiString	m_Country;
	void __fastcall UpdateBtn(void);
	void __fastcall SetCountry(void);

//	CAlignList	AlignList;
//	void __fastcall EntryAlignControl(void);
	void __fastcall UpdateStat(void);

	void __fastcall UpdateTextData(SDMMLOG *sp, int n);
	void __fastcall UpdateTextData(SDMMLOG *sp);
	void __fastcall UpdateCurData(SDMMLOG *sp);
public:
	virtual __fastcall TQSODlgBox(TComponent* AOwner);
	int __fastcall Execute(CLogFind *fp, SDMMLOG *sp, int n);
	int __fastcall ShowFind(CLogFind *fp);
};
//----------------------------------------------------------------------------
//extern TQSODlgBox *QSODlgBox;
//----------------------------------------------------------------------------
#endif    
