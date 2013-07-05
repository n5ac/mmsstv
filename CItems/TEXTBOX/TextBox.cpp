//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
#include "ComLib.h"
#include "About.h"
//---------------------------------------------------------------------------
USEUNIT("Comlib.cpp");
USEFORM("About.cpp", AboutDlg);
USERC("textbox.rc");
USEFORM("TextDlg.cpp", TextDlgBox);
USEFORM("CodeVw.cpp", CodeView);
//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*)
{
	switch(reason){
		case DLL_PROCESS_ATTACH:
			{
				InitSys();

				char bf[512];
				::GetModuleFileName(hinst, bf, sizeof(bf));
				::SetDirName(sys.m_BgnDir, bf);

				_setmbcp(_MB_CP_ANSI);
				HWND hMM = ::FindWindow("TMmsstv", NULL);
				if( hMM != NULL ) Application->Handle = hMM;

                if( sys.m_BitPixel < 24 ){
					sys.m_pBitmapTemp = new Graphics::TBitmap;
					sys.m_pBitmapTemp->Width = 1;
            		sys.m_pBitmapTemp->Height = 1;
                }
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
				SaveStringsToIniFile(sys.m_pTextList, "Strings", ININAME, TRUE);
				delete sys.m_pTextList;
				sys.m_pTextList = NULL;
			}
			if( sys.m_pStyleList != NULL ){
				SaveStringsToIniFile(sys.m_pStyleList, "Styles", ININAME, FALSE);
				delete sys.m_pStyleList;
				sys.m_pStyleList = NULL;
			}
            if( sys.m_pFontList != NULL ){
				delete sys.m_pFontList;
                sys.m_pFontList = NULL;
            }
            if( sys.m_pBitmapTemp != NULL ){
				delete sys.m_pBitmapTemp;
                sys.m_pBitmapTemp = NULL;
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
#if UseHWND
	TAboutDlg *pBox = new TAboutDlg(hWnd);
#else
	TAboutDlg *pBox = new TAboutDlg(NULL);
#endif
	pBox->ShowModal();
	delete pBox;
#if UseHWND
	::SetForegroundWindow(hWnd);
#endif
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
#if UseHWND
	::SetForegroundWindow(hWnd);
#endif
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
void mcmPos(HANDLE hObj, DWORD pos, DWORD size, DWORD tsize)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;
	pItem->SetPos(pos, size, tsize);
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
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
void mcmSetEx(HANDLE hObj, DWORD ex)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;

	pItem->SetEx(ex);
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
void mcmGetFont(HANDLE hObj, LOGFONT *pLogfont)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;

	pItem->GetFont(pLogfont);
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
void mcmSetFont(HANDLE hObj, LOGFONT *pLogfont)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;

	pItem->SetFont(pLogfont);
}

