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
USERES("Mmsstv.res");
USEFORM("Main.cpp", Mmsstv);
USEUNIT("Sound.cpp");
USEUNIT("fir.cpp");
USEUNIT("sstv.cpp");
USEUNIT("Wave.cpp");
USEUNIT("Fft.cpp");
USEUNIT("ComLib.cpp");
USEUNIT("Draw.cpp");
USEUNIT("Comm.cpp");
USEUNIT("cradio.cpp");
USEFORM("TextIn.cpp", TextInDlg);
USEFORM("RxView.cpp", RxViewDlg);
USEFORM("Option.cpp", OptionDlg);
USEFORM("HistView.cpp", HistViewDlg);
USEFORM("fileview.cpp", FileViewDlg);
USEFORM("CtrBtn.cpp", CtrBtnWnd);
USEFORM("ZoomView.cpp", ZoomViewDlg);
USEFORM("ListText.cpp", ListTextDlg);
USEFORM("PicFilte.cpp", PicFilterDlg);
USEFORM("StockVew.cpp", StockView);
USEFORM("PicSel.cpp", PicSelDlg);
USEFORM("MacroKey.cpp", MacroKeyDlg);
USEFORM("ColorSet.cpp", ColorSetDlg);
USEFORM("LineSet.cpp", LineSetDlg);
USEFORM("PicRect.cpp", PicRectDlg);
USEUNIT("LogFile.cpp");
USEUNIT("Loglink.cpp");
USEUNIT("country.cpp");
USEUNIT("Mmcg.cpp");
USEUNIT("jpeg\Jutils.c");
USEUNIT("jpeg\Jcapistd.c");
USEUNIT("jpeg\Jccoefct.c");
USEUNIT("jpeg\Jccolor.c");
USEUNIT("jpeg\Jcdctmgr.c");
USEUNIT("jpeg\Jchuff.c");
USEUNIT("jpeg\Jcinit.c");
USEUNIT("jpeg\Jcmainct.c");
USEUNIT("jpeg\Jcmarker.c");
USEUNIT("jpeg\Jcmaster.c");
USEUNIT("jpeg\Jcomapi.c");
USEUNIT("jpeg\Jcparam.c");
USEUNIT("jpeg\Jcphuff.c");
USEUNIT("jpeg\Jcprepct.c");
USEUNIT("jpeg\Jcsample.c");
USEUNIT("jpeg\Jctrans.c");
USEUNIT("jpeg\Jdapimin.c");
USEUNIT("jpeg\Jdapistd.c");
USEUNIT("jpeg\Jdatadst.c");
USEUNIT("jpeg\Jdatasrc.c");
USEUNIT("jpeg\Jdcoefct.c");
USEUNIT("jpeg\Jdcolor.c");
USEUNIT("jpeg\Jddctmgr.c");
USEUNIT("jpeg\Jdhuff.c");
USEUNIT("jpeg\Jdinput.c");
USEUNIT("jpeg\Jdmainct.c");
USEUNIT("jpeg\Jdmarker.c");
USEUNIT("jpeg\Jdmaster.c");
USEUNIT("jpeg\Jdmerge.c");
USEUNIT("jpeg\Jdphuff.c");
USEUNIT("jpeg\Jdpostct.c");
USEUNIT("jpeg\Jdsample.c");
USEUNIT("jpeg\Jdtrans.c");
USEUNIT("jpeg\Jerror.c");
USEUNIT("jpeg\Jfdctflt.c");
USEUNIT("jpeg\Jfdctfst.c");
USEUNIT("jpeg\Jfdctint.c");
USEUNIT("jpeg\Jidctflt.c");
USEUNIT("jpeg\Jidctfst.c");
USEUNIT("jpeg\Jidctint.c");
USEUNIT("jpeg\Jidctred.c");
USEUNIT("jpeg\Jmemmgr.c");
USEUNIT("jpeg\Jmemnobs.c");
USEUNIT("jpeg\Jquant1.c");
USEUNIT("jpeg\Jquant2.c");
USEUNIT("jpeg\Jcapimin.c");
USEUNIT("jpeg\jhandle.C");
USEFORM("PrevView.cpp", PrevViewBox);
USEFORM("LogList.cpp", LogListDlg);
USEFORM("LogSet.cpp", LogSetDlg);
USEFORM("Qsodlg.cpp", QSODlgBox);
USEFORM("ConvDef.cpp", ConvDefDlg);
USEFORM("MmcgDlg.cpp", MmcgDlgBox);
USEUNIT("LogConv.cpp");
USEFORM("VerDsp.cpp", VerDspDlg);
USEFORM("ClockAdj.cpp", ClockAdjDlg);
USEFORM("InputWin.cpp", InputWinDlg);
USEFORM("ColorBar.cpp", ColorBarDlg);
USEFORM("LinearDs.cpp", LinearDspDlg);
USEFORM("FreqDisp.cpp", FreqDispDlg);
USEFORM("RadioSet.cpp", RADIOSetDlg);
USEFORM("TextEdit.cpp", TextEditDlg);
USEFORM("LogPic.cpp", LogPicDlg);
USEFORM("BitMask.cpp", BitMaskDlg);
USEFORM("ExtCmd.cpp", ExtCmdDlg);
USEFORM("PerSpect.cpp", PerSpectDlg);
USEFORM("RepSet.cpp", RepSetDlg);
USEFORM("Scope.cpp", TScope);
USEFORM("PlayDlg.cpp", PlayDlgBox);
USEUNIT("MMlink.cpp");
USEFILE("mml.h");
USEFILE("mmrp.h");
USEFILE("mmw.h");
USEFORM("TH5Len.cpp", TH5LenDlg);
USEUNIT("Hamlog5.cpp");
USEFORM("RMenuDlg.cpp", RMenuDialog);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	LPSTR pCmd = GetCommandLine();
    if( strstr(pCmd, "-Z") == NULL ){
		HWND hWnd = ::FindWindow("TMmsstv", NULL);
		if( hWnd != NULL ){
			hWnd = ::FindWindow("TAppBuilder", NULL);
			if( hWnd == NULL ){
				return 0;
			}
		}
	}

	try
	{
		Application->Initialize();
		Application->CreateForm(__classid(TMmsstv), &Mmsstv);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}
//---------------------------------------------------------------------------

