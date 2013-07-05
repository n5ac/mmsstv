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
#ifndef LogListH
#define LogListH
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
//----------------------------------------------------------------------------
#include "LogFile.h"
#include <Grids.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
//----------------------------------------------------------------------------
class TLogListDlg : public TForm
{
__published:        
	TStringGrid *Grid;
	TMainMenu *MainMenu;
	TMenuItem *KFile;
	TMenuItem *KOpen;
	TOpenDialog *OpenDialog;
	TMenuItem *KEdit;
	TMenuItem *KFlush;
	TMenuItem *KDelCur;
	TMenuItem *KDelSel;
	TMenuItem *N1;
	TMenuItem *KExit;
	TMenuItem *KFind;
	TMenuItem *KFindTop;
	TMenuItem *KFindBottom;
	TMenuItem *KFindConT;
	TMenuItem *KFindConB;
	TMenuItem *N2;
	TMenuItem *KTop;
	TMenuItem *KBottom;
	TMenuItem *N3;
	TMenuItem *KSortDate;
	TMenuItem *KInsCur;
	TMenuItem *N4;
	TMenuItem *N5;
	TMenuItem *KMTextRead;
	TMenuItem *KMTextWrite;
	TSaveDialog *SaveDialog;
	TMenuItem *KSelAll;
	TMenuItem *KHelp;
	TMenuItem *KHlpLog;
	TMenuItem *N6;
	TMenuItem *KReIndex;
	TMenuItem *N7;
	TMenuItem *KExport;
	TMenuItem *KExportText;
	TMenuItem *KImport;
	TMenuItem *KImportText;
	TMenuItem *KImportLog200;
	TMenuItem *KExportLog200;
	TMenuItem *KImportHamlog;
	TMenuItem *KExportHamlog;
	TMenuItem *KLogOpt;
	TMenuItem *KOpt;
	TMenuItem *KExportADIF;
	TMenuItem *KImportADIF;
	TMenuItem *KExportCabrillo;void __fastcall GridKeyPress(TObject *Sender, char &Key);
	void __fastcall KOpenClick(TObject *Sender);
	void __fastcall KFlushClick(TObject *Sender);
	void __fastcall KDelCurClick(TObject *Sender);
	void __fastcall KDelSelClick(TObject *Sender);
	void __fastcall KExitClick(TObject *Sender);
	void __fastcall KFindTopClick(TObject *Sender);
	void __fastcall KFindBottomClick(TObject *Sender);
	void __fastcall KTopClick(TObject *Sender);
	void __fastcall KBottomClick(TObject *Sender);
	void __fastcall KSortDateClick(TObject *Sender);
	
	void __fastcall GridDblClick(TObject *Sender);
	void __fastcall KInsCurClick(TObject *Sender);
	void __fastcall KMTextReadClick(TObject *Sender);
	void __fastcall KMTextWriteClick(TObject *Sender);
	void __fastcall KSelAllClick(TObject *Sender);
	void __fastcall KHlpLogClick(TObject *Sender);
	void __fastcall KReIndexClick(TObject *Sender);
	
	void __fastcall KExportTextClick(TObject *Sender);
	void __fastcall KImportTextClick(TObject *Sender);
	void __fastcall KExportLog200Click(TObject *Sender);
	void __fastcall KImportLog200Click(TObject *Sender);
	void __fastcall KImportHamlogClick(TObject *Sender);
	void __fastcall KExportHamlogClick(TObject *Sender);
	void __fastcall KLogOptClick(TObject *Sender);
	void __fastcall KExportADIFClick(TObject *Sender);
	void __fastcall KImportADIFClick(TObject *Sender);
	void __fastcall KExportCabrilloClick(TObject *Sender);
	void __fastcall GridDrawCell(TObject *Sender, int Col, int Row,
		  TRect &Rect, TGridDrawState State);
private:
	void __fastcall AdjustTopRow(void);
	void __fastcall UpdateLogCount(int sw);
	void __fastcall UpdateMenu(void);
	void __fastcall OnIdle(TObject *Sender, bool &Done);

	int __fastcall SureRead(void);

	void __fastcall LoadMmlogText(LPCSTR pName);
	void __fastcall SaveMmlogText(LPCSTR pName);
	int __fastcall LoadText(LPCSTR pName);
	int __fastcall SaveText(LPCSTR pName);
	int __fastcall LoadLog200(LPCSTR pName);
	int __fastcall SaveLog200(LPCSTR pName);
	int __fastcall LoadHamLog(LPCSTR pName);
	int __fastcall SaveHamLog(LPCSTR pName);
	int __fastcall LoadHamLog5(LPCSTR pName);
	int __fastcall SaveHamLog5(LPCSTR pName);

	int __fastcall LoadADIF(LPCSTR pName);
	int __fastcall SaveADIF(LPCSTR pName);

	int __fastcall SaveCabrillo(LPCSTR pName);

	AnsiString	m_FindCall;

	void __fastcall SetTimeZone(void);
	AnsiString	m_TimeZone;
	int			m_DateWidth;
public:
	virtual __fastcall TLogListDlg(TComponent* AOwner);

	void __fastcall TLogListDlg::Execute(void);
};
//----------------------------------------------------------------------------
//extern TLogListDlg *LogListDlg;
//----------------------------------------------------------------------------
#endif    
