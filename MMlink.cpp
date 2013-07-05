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

#include "MMLink.h"
//---------------------------------------------------------------------------
__fastcall CMMList::CMMList(void)
{
	m_pList = NULL;
}
//---------------------------------------------------------------------------
__fastcall CMMList::~CMMList()
{
	delete m_pList;
}
//---------------------------------------------------------------------------
int __fastcall CMMList::QueryList(LPCSTR pFilter)
{
	if( m_pList == NULL ) m_pList = new TStringList;
	m_pList->Clear();
	m_FilterLen = strlen(pFilter);

	HANDLE			hFind;
	WIN32_FIND_DATA	fd;

	char	Name[MAX_PATH];
	sprintf(Name, "%s*.%s", BgnDir, pFilter);
	hFind = ::FindFirstFile(Name, &fd);
	if( hFind != INVALID_HANDLE_VALUE ){
		while(1){
			m_pList->Add(fd.cFileName);
			if( !FindNextFile(hFind, &fd) ) break;
		}
		::FindClose(hFind);
	}
	return m_pList->Count;
}
//---------------------------------------------------------------------------
LPCSTR __fastcall CMMList::GetItemName(int n)
{
	if( n < 0 ) return NULL;
	if( n >= m_pList->Count ) return NULL;

	m_Name = m_pList->Strings[n].c_str();
	LPSTR p = lastp(m_Name.c_str());
	p -= m_FilterLen;
	*p = 0;
	return m_Name.c_str();
}
//---------------------------------------------------------------------------
LPCSTR __fastcall CMMList::GetFileName(int n)
{
	if( n < 0 ) return NULL;
	if( n >= m_pList->Count ) return NULL;

	m_Name = m_pList->Strings[n];
	return m_Name.c_str();
}
//---------------------------------------------------------------------------
int __fastcall CMMList::IndexOf(LPCSTR pKey)
{
	for( int i = 0; i < m_pList->Count; i++ ){
		if( !strcmpi(GetItemName(i), pKey) ) return i;
	}
	return -1;
}
//***************************************************************************
// CMMLink class
//---------------------------------------------------------------------------
__fastcall CMMLink::CMMLink(HWND hWnd, UINT uMsg)
{
	m_hLib = NULL;
	m_hWnd = hWnd;
	m_uMsg = uMsg;
	m_Caps = 0;
}
//---------------------------------------------------------------------------
__fastcall CMMLink::~CMMLink()
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall CMMLink::FreeLib(void)
{
	if( m_hLib ){
		FreeLibrary(m_hLib);
		m_hLib = NULL;
	}
}
//---------------------------------------------------------------------------
FARPROC __fastcall CMMLink::GetProc(LPCSTR pName)
{
	if( !m_hLib ) return NULL;

	FARPROC fn = ::GetProcAddress(m_hLib, pName+1);
	if( fn == NULL ){
		fn = ::GetProcAddress(m_hLib, pName);
		if( fn == NULL ) FreeLib();
	}
	return fn;
}
//---------------------------------------------------------------------------
void __fastcall CMMLink::Close(void)
{
	if( IsLib() ){
		fmmlClose();
		FreeLib();
	}
	m_Connected = FALSE;
}
//---------------------------------------------------------------------------
BOOL __fastcall CMMLink::Open(LPCSTR pItemName)
{
	Close();

	m_ItemName = pItemName;
	m_SessionName = m_ItemName;
	char LibName[MAX_PATH];
	if( !*GetEXT(pItemName) ){
		sprintf(LibName, "%s.mml", pItemName);
		pItemName = LibName;
	}

	m_hLib = ::LoadLibrary(pItemName);
	if( m_hLib ){

		fmmlOpen = (tmmlOpen)GetProc("_mmlOpen");
		fmmlClose = (tmmlClose)GetProc("_mmlClose");
		fmmlSetHandle = (tmmlSetHandle)GetProc("_mmlSetHandle");
		fmmlIsCap = (tmmlIsCap)GetProc("_mmlIsCap");
		fmmlIsConnected = (tmmlIsConnected)GetProc("_mmlIsConnected");
		fmmlGetSessionName = (tmmlGetSessionName)GetProc("_mmlGetSessionName");
		fmmlQuery = (tmmlQuery)GetProc("_mmlQuery");
		fmmlSetFreq = (tmmlSetFreq)GetProc("_mmlSetFreq");
		fmmlLog = (tmmlLog)GetProc("_mmlLog");
		fmmlLogClear = (tmmlLogClear)GetProc("_mmlLogClear");
		fmmlSetPTT = (tmmlSetPTT)GetProc("_mmlSetPTT");
		fmmlOnCopyData = (tmmlOnCopyData)GetProc("_mmlOnCopyData");
		fmmlEventVFO = (tmmlEventVFO)GetProc("_mmlEventVFO");

		if( m_hLib ){
			if( fmmlOpen(m_hWnd, m_uMsg) ){
				m_Caps = fmmlIsCap();
			}
			else {
				FreeLib();
			}
		}
	}
	return IsLib();
}
//---------------------------------------------------------------------------
void __fastcall CMMLink::SetHandle(HWND hWnd, UINT uMsg)
{
	m_hWnd = hWnd;
	m_uMsg = uMsg;
	if( !IsLib() ) return;
	fmmlSetHandle(hWnd, m_uMsg);
}
//---------------------------------------------------------------------------
void __fastcall CMMLink::NotifySession(LPCSTR pSession)
{
	if( pSession ){
		m_Connected = TRUE;
		m_SessionName = pSession;
	}
	else {
		m_Connected = FALSE;
	}
}
//---------------------------------------------------------------------------
BOOL __fastcall CMMLink::IsConnected(void)
{
	if( !IsLib() ) return FALSE;
	if( m_Caps & capNOTIFYSESSION ) return m_Connected;
	return fmmlIsConnected();
}
//---------------------------------------------------------------------------
LPCSTR __fastcall CMMLink::GetSessionName(void)
{
	if( !IsLib() ) return NULL;
	if( m_Caps & capNOTIFYSESSION ) return m_SessionName.c_str();
	return fmmlGetSessionName();
}
//---------------------------------------------------------------------------
void __fastcall CMMLink::Query(LPCSTR pCall)
{
	if( !IsLib() ) return;
	fmmlQuery(pCall);
}
//---------------------------------------------------------------------------
void __fastcall CMMLink::SetFreq(LPCSTR pFreq)
{
	if( !IsLib() ) return;
	fmmlSetFreq(pFreq);
}
//---------------------------------------------------------------------------
void __fastcall CMMLink::SetPTT(int ptt)
{
	if( !IsLib() ) return;
	fmmlSetPTT(ptt);
}
//---------------------------------------------------------------------------
void __fastcall CMMLink::LogWrite(SDMMLOG *sp, int sw)
{
	if( !IsLib() ) return;
	mmLOGDATA ml;
	ConvFormat(&ml, sp);
	fmmlLog(&ml, sw);
}
//---------------------------------------------------------------------------
void __fastcall CMMLink::Clear(void)
{
	if( !IsLib() ) return;
	fmmlLogClear();
}
//---------------------------------------------------------------------------
void __fastcall CMMLink::EventVFO(void)
{
	if( !IsLib() ) return;
	fmmlEventVFO();
}
//---------------------------------------------------------------------------
LONG __fastcall CMMLink::OnCopyData(HWND hSender, const COPYDATASTRUCT *pcds)
{
	if( !IsLib() ) return FALSE;
	if( !(m_Caps & capWMCOPYDATA) ) return FALSE;
	return fmmlOnCopyData(hSender, pcds);
}
//---------------------------------------------------------------------------
void __fastcall CMMLink::SetTime(SYSTEMTIME *tp, int sw)
{
	if( sw ){
		m_TimeLogOFF = *tp;
	}
	else {
		m_TimeLogON = *tp;
	}
}
//---------------------------------------------------------------------------
void __fastcall CMMLink::ConvFormat(mmLOGDATA *pLog, const SDMMLOG *sp)
{
	memset(pLog, 0, sizeof(mmLOGDATA));

	if( sp->btime ) pLog->m_TimeLogON = m_TimeLogON;
	if( sp->etime ) pLog->m_TimeLogOFF = m_TimeLogOFF;
	StrCopy(pLog->m_Call, sp->call, sizeof(pLog->m_Call)- 1);
	strcpy(pLog->m_Mode, Log.GetModeString(sp->mode));
	strcpy(pLog->m_Freq, Log.GetFreqString(sp->band, sp->fq));
	memcpy(pLog->m_His, sp->ur, sizeof(pLog->m_His));
	memcpy(pLog->m_My, sp->my, sizeof(pLog->m_My));
	StrCopy(pLog->m_Name, sp->name, sizeof(pLog->m_Name)- 1);
	StrCopy(pLog->m_QTH, sp->qth, sizeof(pLog->m_QTH)- 1);
	StrCopy(pLog->m_Pow, sp->pow, sizeof(pLog->m_Pow)- 1);
	StrCopy(pLog->m_Note, sp->rem, sizeof(pLog->m_Note)- 1);
	StrCopy(pLog->m_QSL, sp->qsl, sizeof(pLog->m_QSL)- 1);
	StrCopy(pLog->m_DXCC, sp->opt1, sizeof(pLog->m_DXCC)- 1);
	StrCopy(pLog->m_Cont, sp->opt2, sizeof(pLog->m_Cont)- 1);
	pLog->m_QSLS = sp->send;
	pLog->m_QSLR = sp->recv;
}


//***************************************************************************
// CMMRadio class
//---------------------------------------------------------------------------
__fastcall CMMRadio::CMMRadio(HWND hWnd, UINT uMsg)
{
	m_hLib = NULL;
	m_hWnd = hWnd;
	m_uMsg = uMsg;
}
//---------------------------------------------------------------------------
__fastcall CMMRadio::~CMMRadio()
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall CMMRadio::FreeLib(void)
{
	if( m_hLib ){
		FreeLibrary(m_hLib);
		m_hLib = NULL;
	}
}
//---------------------------------------------------------------------------
FARPROC __fastcall CMMRadio::GetProc(LPCSTR pName)
{
	if( !m_hLib ) return NULL;

	FARPROC fn = ::GetProcAddress(m_hLib, pName+1);
	if( fn == NULL ){
		fn = ::GetProcAddress(m_hLib, pName);
		if( fn == NULL ) FreeLib();
	}
	return fn;
}
//---------------------------------------------------------------------------
void __fastcall CMMRadio::Close(void)
{
	if( IsLib() ){
		fmmrpClose();
		FreeLib();
	}
}
//---------------------------------------------------------------------------
BOOL __fastcall CMMRadio::Open(LPCSTR pItemName)
{
	Close();

	m_ItemName = pItemName;
	char LibName[MAX_PATH];
	if( !*GetEXT(pItemName) ){
		sprintf(LibName, "%s.mmr", pItemName);
		pItemName = LibName;
	}
	m_hLib = ::LoadLibrary(pItemName);
	if( m_hLib ){

		fmmrpOpen = PROC(mmrpOpen);
		fmmrpClose = PROC(mmrpClose);
		fmmrpSetHandle = PROC(mmrpSetHandle);
		fmmrpGetStatus = PROC(mmrpGetStatus);
		fmmrpSetPTT = PROC(mmrpSetPTT);
		fmmrpPutChar = PROC(mmrpPutChar);
		fmmrpGetChar = PROC(mmrpGetChar);
		fmmrpPolling = PROC(mmrpPolling);
		fmmrpGetFreq = PROC(mmrpGetFreq);
		fmmrpGetDefCommand = PROC(mmrpGetDefCommand);

		if( m_hLib ){
			if( !fmmrpOpen(m_hWnd, m_uMsg) ){
				FreeLib();
			}
		}
	}
	return IsLib();
}
//---------------------------------------------------------------------------
void __fastcall CMMRadio::SetHandle(HWND hWnd, UINT uMsg)
{
	m_hWnd = hWnd;
	m_uMsg = uMsg;
	if( !IsLib() ) return;
	fmmrpSetHandle(hWnd, uMsg);
}
//---------------------------------------------------------------------------
void __fastcall CMMRadio::SetPTT(int ptt)
{
	if( !IsLib() ) return;
	fmmrpSetPTT(ptt);
}
//---------------------------------------------------------------------------
void __fastcall CMMRadio::PutChar(BYTE c)
{
	if( !IsLib() ) return;
	fmmrpPutChar(c);
}
//---------------------------------------------------------------------------
BYTE __fastcall CMMRadio::GetChar(void)
{
	if( !IsLib() ) return 0;
	return fmmrpGetChar();
}
//---------------------------------------------------------------------------
DWORD __fastcall CMMRadio::GetStatus(void)
{
	if( !IsLib() ) return 0;
	return fmmrpGetStatus();
}
//---------------------------------------------------------------------------
void __fastcall CMMRadio::Polling(void)
{
	if( !IsLib() ) return;
	fmmrpPolling();
}
//---------------------------------------------------------------------------
DWORD __fastcall CMMRadio::GetFreq(void)
{
	if( !IsLib() ) return 0;
	return fmmrpGetFreq();
}
//---------------------------------------------------------------------------
DWORD __fastcall CMMRadio::GetDefCommand(void)
{
	if( !IsLib() ) return 0;
	return fmmrpGetDefCommand();
}

