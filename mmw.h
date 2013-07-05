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



#ifndef MMW_H
#define	MMW_H

typedef void (__stdcall *tmmwPumpMessages)(void);
typedef LONG (__stdcall *tmmwGetTimeout)(void);
typedef void (__stdcall *tmmwSetPTT)(LONG tx);

typedef LONG (__stdcall *tmmwInOpen)(DWORD sampfreq, DWORD size);
typedef void (__stdcall *tmmwInClose)(void);
typedef BOOL (__stdcall *tmmwInRead)(SHORT *buf);

typedef LONG (__stdcall *tmmwGetInExist)(void);
typedef BOOL (__stdcall *tmmwIsInCritical)(void);

typedef LONG (__stdcall *tmmwOutOpen)(DWORD sampfreq, DWORD size);
typedef void (__stdcall *tmmwOutAbort)(void);
typedef void (__stdcall *tmmwOutFlush)(void);
typedef BOOL (__stdcall *tmmwOutWrite)(const SHORT *buf);

typedef BOOL (__stdcall *tmmwIsOutCritical)(void);
typedef BOOL (__stdcall *tmmwIsOutFull)(void);
typedef LONG (__stdcall *tmmwGetOutRemaining)(void);
typedef LONG (__stdcall *tmmwGetOutCounter)(void);
typedef void (__stdcall *tmmwSetOutCounter)(LONG count);

#endif

