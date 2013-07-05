//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
#include "PerDlg.h"
#include "About.h"
//---------------------------------------------------------------------------
USEUNIT("Comlib.cpp");
USEFORM("About.cpp", AboutDlg);
USEFORM("PerDlg.cpp", PerDlgBox);
USERC("perimg.rc");
//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*)
{
	switch(reason){
		case DLL_PROCESS_ATTACH:
			{
				memset(&sys, 0, sizeof(sys));

				sys.m_dwVersion = ::GetVersion();
				sys.m_WinNT = (sys.m_dwVersion < 0x80000000) ? TRUE : FALSE;

				HWND hMM = ::FindWindow("TMmsstv", NULL);
				if( hMM != NULL ) Application->Handle = hMM;
			}
			break;
		case DLL_PROCESS_DETACH:
			sys.m_fMultProc = NULL;
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

#if UseHWND
	TPerDlgBox *pBox = new TPerDlgBox(hWnd);
#else
	TPerDlgBox *pBox = new TPerDlgBox(NULL);
#endif
	DWORD r = (DWORD)pBox->Execute(pItem);
	delete pBox;
#if UseHWND
	::SetForegroundWindow(hWnd);
#endif
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
	ASSERT(0);  // This function should not be called
	return "";
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
HBITMAP mcmUpdateText(HANDLE hObj, HBITMAP hDest, LPCSTR pText)
{
	ASSERT(0);  // This function should not be called
	return NULL;
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
HBITMAP mcmUpdateImage(HANDLE hObj, HBITMAP hDest, HBITMAP hSrc)
{
	ASSERT(hObj);
	ASSERT(hDest);
	CItem *pItem = (CItem *)hObj;
	if( (hSrc != NULL) && !(pItem->m_sperspect.flag & 0x0000ffff) ){
		// Copy hSrc to the object
		Graphics::TBitmap *pSrc = new Graphics::TBitmap;
		pSrc->Handle = hSrc;
		pItem->CopySource(pSrc);
		pSrc->ReleaseHandle();
		delete pSrc;
	}

	// Draw object to hDest
	Graphics::TBitmap *pDest = new Graphics::TBitmap;
	pDest->Handle = hDest;
	pItem->Draw(pDest);

	HBITMAP hb = pDest->ReleaseHandle();
	ASSERT(hb);
	delete pDest;
	return (hb != hDest) ? hb : NULL;
}
//---------------------------------------------------------------------
extern "C" __declspec(dllexport)
void mcmSetDraft(HANDLE hObj, DWORD draft)
{
	ASSERT(hObj);
	CItem *pItem = (CItem *)hObj;
	pItem->SetDraft(draft);
}
