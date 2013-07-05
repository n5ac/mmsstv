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




#ifndef MMLinkH
#define MMLinkH

#include "ComLib.h"
#include "LogConv.h"
#include "mml.h"
#include "mmrp.h"

#define PROC(Key) ((t##Key)GetProc("_" #Key))

class CMMList
{
private:
	TStringList	*m_pList;
	AnsiString	m_Name;
	int			m_FilterLen;
public:
	__fastcall CMMList(void);
	__fastcall ~CMMList();
public:	// List functions
	int __fastcall QueryList(LPCSTR pFilter);
	int __fastcall GetCount(void){return m_pList->Count;};
	LPCSTR __fastcall GetItemName(int n);
	LPCSTR __fastcall GetFileName(int n);
	int __fastcall IndexOf(LPCSTR pKey);
	inline BOOL __fastcall IsQuery(void){return m_pList != NULL;};
};

class CMMLink
{
private:
	AnsiString	m_ItemName;

	HWND		m_hWnd;
	UINT		m_uMsg;

	HINSTANCE	m_hLib;	//ja7ude 0521
	DWORD		m_Caps;

	int			m_Connected;
	AnsiString	m_SessionName;

	SYSTEMTIME	m_TimeLogON;
	SYSTEMTIME	m_TimeLogOFF;
private:
	tmmlOpen			fmmlOpen;
	tmmlClose			fmmlClose;
	tmmlSetHandle		fmmlSetHandle;
	tmmlIsCap			fmmlIsCap;
	tmmlIsConnected		fmmlIsConnected;
	tmmlGetSessionName	fmmlGetSessionName;
	tmmlQuery			fmmlQuery;
	tmmlSetFreq			fmmlSetFreq;
	tmmlLog				fmmlLog;
	tmmlLogClear		fmmlLogClear;
	tmmlSetPTT			fmmlSetPTT;
	tmmlOnCopyData		fmmlOnCopyData;
	tmmlEventVFO		fmmlEventVFO;
private:
	void __fastcall FreeLib(void);
	FARPROC __fastcall GetProc(LPCSTR pName);
	void __fastcall ConvFormat(mmLOGDATA *pLog, const SDMMLOG *sp);
public:
	__fastcall CMMLink(HWND hWnd, UINT uMsg);
	__fastcall ~CMMLink();
public: // LogLink functions
	BOOL __fastcall Open(LPCSTR pLibName);
	void __fastcall Close(void);
	void __fastcall SetHandle(HWND hWnd, UINT uMsg);
	inline BOOL __fastcall IsLib(void){ return (m_hLib != NULL);};
	inline LPCSTR __fastcall GetItemName(void){return m_ItemName.c_str();};

	void __fastcall NotifySession(LPCSTR pSession);

	BOOL __fastcall IsConnected(void);
	LPCSTR __fastcall GetSessionName(void);
	void __fastcall Query(LPCSTR pCall);
	void __fastcall SetFreq(LPCSTR pFreq);
	void __fastcall SetPTT(int ptt);
	void __fastcall LogWrite(SDMMLOG *sp, int sw);
	void __fastcall Clear(void);
	void __fastcall EventVFO(void);
	LONG __fastcall OnCopyData(HWND hSender, const COPYDATASTRUCT *pcds);
	inline DWORD __fastcall GetCaps(void){return m_Caps;};
	void __fastcall SetTime(SYSTEMTIME *tp, int sw);

};


class CMMRadio
{
private:
	AnsiString	m_ItemName;

	HINSTANCE	m_hLib;	//ja7ude 0521
	HWND		m_hWnd;
	UINT		m_uMsg;

private:
	tmmrpSetHandle		fmmrpSetHandle;
	tmmrpOpen			fmmrpOpen;
	tmmrpClose			fmmrpClose;
	tmmrpGetStatus		fmmrpGetStatus;
	tmmrpSetPTT			fmmrpSetPTT;
	tmmrpPutChar		fmmrpPutChar;
	tmmrpGetChar		fmmrpGetChar;
	tmmrpPolling		fmmrpPolling;
	tmmrpGetFreq		fmmrpGetFreq;
	tmmrpGetDefCommand	fmmrpGetDefCommand;

private:
	void __fastcall FreeLib(void);
	FARPROC __fastcall GetProc(LPCSTR pName);
public:
	__fastcall CMMRadio(HWND hWnd, UINT uMsg);
	__fastcall ~CMMRadio();
public:
	void __fastcall SetHandle(HWND hWnd, UINT uMsg);
	BOOL __fastcall Open(LPCSTR pLibName);
	void __fastcall Close(void);
	inline BOOL __fastcall IsLib(void){ return (m_hLib != NULL);};
	inline LPCSTR __fastcall GetItemName(void){return m_ItemName.c_str();};

	void __fastcall SetPTT(int ptt);
	void __fastcall PutChar(BYTE c);
	BYTE __fastcall GetChar(void);
	DWORD __fastcall GetStatus(void);
	void __fastcall Polling(void);
	DWORD __fastcall GetFreq(void);
    DWORD __fastcall GetDefCommand(void);
};
#endif
