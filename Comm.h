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



///----------------------------------------------------------
///  ÇqÇrÇQÇRÇQÇbí êMÉNÉâÉX
///
///  (C) JE3HHT Makoto.Mori
///
//---------------------------------------------------------------------------
#ifndef CommH
#define CommH
#include "sstv.h"
//---------------------------------------------------------------------------
#include <Classes.hpp>	//JA7UDE 0428
//---------------------------------------------------------------------------
#define	CR	0x0d
#define	LF	0x0a
#define	COMM_CLOSE	1

typedef struct {
	int	change;

	int Baud;
	int	BitLen;
	int	Stop;
	int	Parity;
}COMMPARA;
extern	COMMPARA	COMM;

typedef long (__stdcall *extfskOpen)(long para);
typedef void (__stdcall *extfskClose)(void);
typedef long (__stdcall *extfskIsTxBusy)(void);
typedef void (__stdcall *extfskPutChar)(BYTE c);
typedef void (__stdcall *extfskSetPTT)(long tx);

class CEXTFSK
{
private:
	//HANDLE			m_hLib;	//ja7ude 0521
	HINSTANCE		m_hLib;
	extfskOpen		fextfskOpen;
	extfskClose		fextfskClose;
	extfskIsTxBusy	fextfskIsTxBusy;
	extfskPutChar	fextfskPutChar;
	extfskSetPTT	fextfskSetPTT;
private:
	FARPROC GetProc(LPCSTR pName);

public:
	__fastcall CEXTFSK(LPCSTR pName);
	__fastcall ~CEXTFSK();
	long __fastcall IsLib(void){return m_hLib != NULL;};
	long __fastcall Open(long para);
	void __fastcall Close(void);
	long __fastcall IsTxBusy(void);
	void __fastcall PutChar(BYTE c);
	void __fastcall SetPTT(long tx);
};

#define	COMM_TXBUFSIZE		MODBUFMAX
class CComm
{
public:
	BOOL	m_CreateON;		// ÉNÉäÉGÉCÉgÉtÉâÉO
	volatile	int	m_Command;
	DCB		m_dcb;			// ÇcÇbÇa
	HANDLE	m_fHnd;			// ÉtÉ@ÉCÉãÉnÉìÉhÉã

	int     m_ptt;
	int     m_scan;
	CEXTFSK	*m_pEXT;
protected:
private:
	int __fastcall PTTOpen(void);

public:
	__fastcall CComm(void);
	__fastcall ~CComm(){
		Close();
	};
	inline BOOL __fastcall IsOpen(void){
		return m_CreateON;
	};
	BOOL __fastcall Open(LPCTSTR PortName);
	void __fastcall Close(void);
	int __fastcall SetPTT(void);
	void __fastcall SetPTT(int sw);
	void __fastcall SetScan(int sw);
};

void InitCOMMPara(void);
#endif






