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
#ifndef PicFilteH
#define PicFilteH
//----------------------------------------------------------------------------
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <StdCtrls.hpp>
#include <Controls.hpp>
#include <Forms.hpp>
#include <Graphics.hpp>
#include <Classes.hpp>
#include <SysUtils.hpp>
#include <Windows.hpp>
#include <System.hpp>
#include <ComCtrls.hpp>
//----------------------------------------------------------------------------
class TPicFilterDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TPanel *Panel;
	TPaintBox *PBox;
	TTrackBar *TBBright;
	TLabel *L1;
	TTrackBar *TBCont;
	TLabel *L2;
	TTrackBar *TBShape;
	TLabel *L3;
	TGroupBox *GBFilt;
	TSpeedButton *SBFN;
	TSpeedButton *SBFA;
	TSpeedButton *SBFB;
	TCheckBox *CB2D;
	TLabel *L4;
	TTrackBar *TBH;
	TLabel *L5;
	TTrackBar *TBS;
	TButton *ResBtn;
	TSpeedButton *SBBRR;
	TSpeedButton *SBCTR;
	TSpeedButton *SBSPR;
	TSpeedButton *SBCPR;
	TSpeedButton *SBCOR;
	TSpeedButton *SBST;
	TButton *AutoBtn;
	TTimer *Timer;
	TSpeedButton *SBEdit;
	void __fastcall PBoxPaint(TObject *Sender);
	void __fastcall TBBrightChange(TObject *Sender);
	void __fastcall SBFNClick(TObject *Sender);

	void __fastcall ResBtnClick(TObject *Sender);
	void __fastcall SBBRRClick(TObject *Sender);
	void __fastcall SBCTRClick(TObject *Sender);
	void __fastcall SBSPRClick(TObject *Sender);
	void __fastcall SBCPRClick(TObject *Sender);
	void __fastcall SBCORClick(TObject *Sender);
	void __fastcall SBSTClick(TObject *Sender);

	void __fastcall AutoBtnClick(TObject *Sender);
	void __fastcall TimerTimer(TObject *Sender);
	void __fastcall SBEditClick(TObject *Sender);
private:
	int     m_DisEvent;

	Graphics::TBitmap *pBitmap;
	Graphics::TBitmap *pBitmapS;
	Graphics::TBitmap *pBitmapD;

	int     m_Make;
	double  *pH;
	double  *pS;

	int     m_XW;
	int     m_YW;
	int     m_Sync;

	void __fastcall UpdateBtn(void);
	void __fastcall AdjustColor(int x, int y, double &RY, double &BY);
	void __fastcall Filt(int sw);
	void __fastcall Bright(void);
public:
	virtual __fastcall TPicFilterDlg(TComponent* AOwner);
	__fastcall TPicFilterDlg::~TPicFilterDlg();

	int __fastcall TPicFilterDlg::Execute(Graphics::TBitmap *pBmp);

#define	CM_EDITEXIT	WM_USER+404
protected:
	void __fastcall OnEditExit(TMessage Message);

BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(CM_EDITEXIT, TMessage, OnEditExit)
END_MESSAGE_MAP(TForm)
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
