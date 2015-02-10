/**
 * @file
 * 
 * @brief Implementation file for some third-party helper code for working with Windows APIs.
 */

#ifdef _WIN32

//Headers
#include <win/win_common_third_party.h>
#include <common.h>

//Libraries
#include <iostream>
#include <malloc.h>    
#include <stdio.h>
#include <tchar.h>

using namespace xmem::common::win::third_party;

#pragma comment(lib, "advapi32.lib")

BOOL xmem::common::win::third_party::SetPrivilege(
	HANDLE hToken,          // access token handle
	LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
	BOOL bEnablePrivilege   // to enable or disable privilege
	)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(
		NULL,            // lookup privilege on local system
		lpszPrivilege,   // privilege to lookup 
		&luid))        // receives LUID of privilege
	{
		printf("LookupPrivilegeValue error: %u\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.

	if (!AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		std::cerr << "AdjustTokenPrivileges error: " << GetLastError() << std::endl;
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

	{
		std::cerr << "The token does not have the specified privilege." << std::endl;
		return FALSE;
	}

	return TRUE;
}

DWORD xmem::common::win::third_party::CountSetBits(ULONG_PTR bitMask)
{
	DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
	DWORD bitSetCount = 0;
	ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
	DWORD i;
	
	for (i = 0; i <= LSHIFT; ++i)
	{
		bitSetCount += ((bitMask & bitTest)?1:0);
		bitTest/=2;
	}

	return bitSetCount;
}

void xmem::common::win::third_party::DumpMap(std::map<std::tstring, double> const &m)
{
	std::map<std::tstring, double>::const_iterator itr = m.begin();
	while (m.end() != itr)
	{
		tcout << itr->first << " " << itr->second << std::endl;
		++itr;
	}
}

double xmem::common::win::third_party::DumpMapValue(std::map<std::tstring, double> const &m)
{
	std::map<std::tstring, double>::const_iterator itr = m.begin();
	if (itr != m.end())
		return itr->second;
	return 0;
}

#else
#error This file should only be used in Windows builds.
#endif
