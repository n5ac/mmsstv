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



#ifndef MML_H
#define	MML_H

#define	capNOTIFYSESSION	0x0001
#define	capWMCOPYDATA		0x0002

enum {
	MML_NOTIFYSESSION,
	MML_QRETURN,
	MML_VFO,
};

#pragma pack(push, 1)
typedef struct {
	SYSTEMTIME	m_TimeLogON;
	SYSTEMTIME	m_TimeLogOFF;
	char		m_Call[16+1];
	char		m_Mode[6+1]; 	// Name of the mode (eg. "RTTY", "SSTV")
	char		m_Freq[16+1];	// MHz (eg.  "14.080")
	char		m_His[20+1];	// His/Her RST(V) and contest number.
	char		m_My[20+1];
	char		m_Name[16+1];	// His/Her name
	char		m_QTH[28+1];	// His/Her QTH
	char		m_Pow[4+1];
	char		m_Note[56+1];	// Note
	char		m_QSL[54+1];	// Note or QSL information
	BYTE		m_QSLS;			// QSL sent
	BYTE		m_QSLR;			// QSL rcvd
	char		m_DXCC[8+1];	// DXCC keyword (eg.  JA, W, VK, etc...)
	char		m_Cont[8+1];	// The continental keyword (AS/OC/NA/SA/EU/AF)
}mmLOGDATA;
#pragma pack(pop)


typedef BOOL (__stdcall *tmmlOpen)(HWND hWnd, UINT uMsg);
typedef void (__stdcall *tmmlClose)(void);
typedef void (__stdcall *tmmlSetHandle)(HWND hWnd, UINT uMsg);
typedef DWORD (__stdcall *tmmlIsCap)(void);
typedef BOOL (__stdcall *tmmlIsConnected)(void);
typedef LPCSTR (__stdcall *tmmlGetSessionName)(void);
typedef void (__stdcall *tmmlQuery)(LPCSTR pCall);
typedef void (__stdcall *tmmlSetFreq)(LPCSTR pFreq);
typedef void (__stdcall *tmmlLog)(const mmLOGDATA *pLog, int sw);
typedef void (__stdcall *tmmlLogClear)(void);
typedef void (__stdcall *tmmlSetPTT)(LONG sw);
typedef LONG (__stdcall *tmmlOnCopyData)(HWND hSender, const COPYDATASTRUCT *pcds);
typedef void (__stdcall *tmmlEventVFO)(void);
#endif
