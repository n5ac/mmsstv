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



#ifndef MMRP_H
#define	MMRP_H

enum {
	MMR_DEFCOMMAND,
	MMR_VFO,
};
#define	mmrpstatusRX		0x0001
#define	mmrpstatusTXBUSY	0x0002
#define	mmrpstatusFREQ		0x0004
#define	mmrpstatusDEFCMD	0x0008

typedef void (__stdcall *tmmrpSetHandle)(HWND hWnd, UINT uMsg);
typedef LONG (__stdcall *tmmrpOpen)(HWND hWnd, UINT uMsg);
typedef void (__stdcall *tmmrpClose)(void);
typedef DWORD (__stdcall *tmmrpGetStatus)(void);
typedef void (__stdcall *tmmrpSetPTT)(LONG tx);
typedef void (__stdcall *tmmrpPutChar)(BYTE c);
typedef BYTE (__stdcall *tmmrpGetChar)(void);
typedef void (__stdcall *tmmrpPolling)(void);
typedef LONG (__stdcall *tmmrpGetFreq)(void);
typedef LONG (__stdcall *tmmrpGetDefCommand)(void);

#endif
