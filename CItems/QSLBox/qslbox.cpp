//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
#include "ComLib.h"
#include "About.h"
//---------------------------------------------------------------------------
USEUNIT("Comlib.cpp");
USEFORM("About.cpp", AboutDlg);
USERC("qslbox.rc");
USEFORM("TextDlg.cpp", TextDlgBox);
//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*)
{
	switch(reason){
		case DLL_PROCESS_ATTACH:
			{
				memset(&sys, 0, sizeof(sys));

				sys.m_dwVersion = ::GetVersion();
				sys.m_WinNT = (sys.m_dwVersion < 0x80000000) ? TRUE : FALSE;
				sys.m_CodePage = ::GetACP();	// Get codepage

				char bf[512];
				::GetModuleFileName(hinst, bf, sizeof(bf));
				::SetDirName(sys.m_BgnDir, bf);

				HDC hdcScreen = ::CreateDC("DISPLAY", NULL, NULL, NULL);
				sys.m_BitPixel = ::GetDeviceCaps(hdcScreen, BITSPIXEL);
				::DeleteDC(hdcScreen);

				_setmbcp(_MB_CP_ANSI);
			}
			break;
		case DLL_PROCESS_DETACH:
			sys.m_fMultProc = NULL;
			if( sys.m_pDefStg != NULL ){
				CItem *pItem = new CItem;
				pItem->Create(NULL, 0);
				pItem->SaveToInifile("Default", ININAME);
				delete pItem;
				delete sys.m_pDefStg;
				sys.m_pDefStg = NULL;
			}
			if( sys.m_pTextList != NULL ){
				SaveStringsToIniFile(sys.m_pTextList, "Strings", ININAME);
				delete sys.m_pTextList;
				sys.m_pTextList = NULL;
			}
			if( sys.m_pStyleList != NULL ){
				SaveStringsToIniFile(sys.m_pStyleList, "Styles", ININAME);
				delete sys.m_pStyleList;
				sys.m_pStyleList = NULL;
			}
			break;
	}
	return 1;
}
//---------------------------------------------------------------------------
extern "C" __declspec(dllexport)
void mcmLanguage(DWORD lang)
{
	sys.m_MsgEng = lang;
}
//---------------------------------------------------------------------------
extern "C" __declspec(dllexport)
void mcmAboutDialog(HWND hWnd)
{
	TAboutDlg *pBox = new TAboutDlg(hWnd);
	pBox->ShowModal();
	delete pBox;
	::SetForegroundWindow(hWnd);
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
HANDLE mcmCreateObject(LPCBYTE pStorage, DWORD Size)
{
	CItem *pItem = new CItem;
	ASSERT(pItem);
	if( pItem != NULL ) pItem->Create(pStorage, Size);
	return (HANDLE)pItem;
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
DWORD mcmGetItemType(HANDLE hObj)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;
	return pItem->GetItemType();
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
LPCBYTE mcmCreateStorage(HANDLE hObj, LPDWORD pSize)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;
	return pItem->CreateStorage(pSize);
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
void mcmDeleteStorage(HANDLE hObj, LPCBYTE pStorage)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;
	pItem->DeleteStorage(pStorage);
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
void mcmDeleteObject(HANDLE hObj)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;
	if( pItem != NULL ) delete pItem;
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
DWORD mcmEdit(HANDLE hObj, HWND hWnd)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;
	int r = pItem->EditDialog(hWnd);
	::SetForegroundWindow(hWnd);
	return r;
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
DWORD mcmFont(HANDLE hObj, HWND hWnd)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;

	int r = pItem->FontDialog(hWnd);
	::SetForegroundWindow(hWnd);
	return r;
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
DWORD mcmGetOrgSize(HANDLE hObj)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;
	return pItem->GetOrgSize();
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
LPCSTR mcmGetUserText(HANDLE hObj)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;
	return pItem->GetText();
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
HBITMAP mcmUpdateText(HANDLE hObj, HBITMAP hDest, LPCSTR pText)
{
	ASSERT(hObj);
	ASSERT(hDest);
	CItem *pItem = (CItem *)hObj;

	// Draw object to hDest
	Graphics::TBitmap *pDest = new Graphics::TBitmap;
	pDest->Handle = hDest;
	pDest = pItem->Draw(pDest, pText);

	HBITMAP hb = pDest->ReleaseHandle();
	delete pDest;
	return (hb != hDest) ? hb : NULL;
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
HBITMAP mcmUpdateImage(HANDLE hObj, HBITMAP hDest, HBITMAP hSrc)
{
	ASSERT(0);  // This function should not be called
	return NULL;
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
void mcmSetDraft(HANDLE hObj, DWORD draft)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;

	pItem->SetDraft(draft);
}