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
#ifndef TextEditH
#define TextEditH
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.StdCtrls.hpp>
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
#include <vcl\Dialogs.hpp>
*/
enum TFontPitch { fpDefault, fpVariable, fpFixed };	//ja7ude 0521
//----------------------------------------------------------------------------
class TTextEditDlg : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TMemo *Memo;
	TButton *FontBtn;
	TFontDialog *FontDialog;
	void __fastcall FontBtnClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	
	
	
	
	void __fastcall FormPaint(TObject *Sender);
	
	
private:
	int		InitFlag;
	LPCSTR	ap;

	CAlignList	AlignList;
	void __fastcall EntryAlignControl(void);
public:
	virtual __fastcall TTextEditDlg(TComponent* AOwner);

	int __fastcall Execute(AnsiString &as, int flag, LPCSTR pTitle = NULL);
};
//----------------------------------------------------------------------------
//extern TTextEditDlg *TextEditDlg;

extern void __fastcall ShowHelp(TForm *pForm, LPCSTR pName);
extern void __fastcall ShowHelp(TForm *pForm, LPCSTR pName, int sw);
//----------------------------------------------------------------------------
#endif
