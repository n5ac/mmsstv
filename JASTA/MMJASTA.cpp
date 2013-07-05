//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("MMJASTA.res");
USEFORM("Main.cpp", MainWnd);
USEUNIT("ComLib.cpp");
USEUNIT("LogFile.cpp");
USEFORM("OptDlg.cpp", OptDlgBox);
USEFORM("TextEdit.cpp", TextEditDlg);
USEFORM("QsoDlg.cpp", QSODlgBox);
USEUNIT("country.cpp");
USEFORM("FEdit.cpp", FileEdit);
USEUNIT("LogConv.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->CreateForm(__classid(TMainWnd), &MainWnd);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}
//---------------------------------------------------------------------------
