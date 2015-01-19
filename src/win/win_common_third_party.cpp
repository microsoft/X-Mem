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

/*int32_t xmem::common::win::third_party::query_sys_info()
{
#ifdef VERBOSE
	std::cout << std::endl;
	std::cout << "Querying system information..." << std::endl;
#endif

	LPFN_GLPI glpi;
	BOOL done = FALSE;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
	DWORD returnLength = 0;
	DWORD logicalProcessorCount = 0;
	DWORD numaNodeCount = 0;
	DWORD processorCoreCount = 0;
	DWORD processorL1CacheCount = 0;
	DWORD processorL2CacheCount = 0;
	DWORD processorL3CacheCount = 0;
	DWORD processorPackageCount = 0;
	DWORD byteOffset = 0;
	PCACHE_DESCRIPTOR Cache;

	glpi = (LPFN_GLPI) GetProcAddress(
							GetModuleHandle(TEXT("kernel32")),
							"GetLogicalProcessorInformation");
	if (NULL == glpi) 
	{
		_tprintf(TEXT("\nGetLogicalProcessorInformation is not supported.\n"));
		return (1);
	}

	while (!done)
	{
		DWORD rc = glpi(buffer, &returnLength);

		if (FALSE == rc) 
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
			{
				if (buffer) 
					free(buffer);

				buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
						returnLength);

				if (NULL == buffer) 
				{
					_tprintf(TEXT("\nError: Allocation failure\n"));
					return (2);
				}
			} 
			else 
			{
				_tprintf(TEXT("\nError %d\n"), GetLastError());
				return (3);
			}
		} 
		else
		{
			done = TRUE;
		}
	}

	ptr = buffer;

	while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) 
	{
		switch (ptr->Relationship) 
		{
		case RelationNumaNode:
			// Non-NUMA systems report a single record of this type.
			numaNodeCount++;
			break;

		case RelationProcessorCore:
			processorCoreCount++;

			// A hyperthreaded core supplies more than one logical processor.
			logicalProcessorCount += xmem::common::win::third_party::CountSetBits(ptr->ProcessorMask);
			break;

		case RelationCache:
			// Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
			Cache = &ptr->Cache;
			if (Cache->Level == 1)
			{
				processorL1CacheCount++;
			}
			else if (Cache->Level == 2)
			{
				processorL2CacheCount++;
			}
			else if (Cache->Level == 3)
			{
				processorL3CacheCount++;
			}
			break;

		case RelationProcessorPackage:
			// Logical processors share a physical package.
			processorPackageCount++;
			break;

		default:
			_tprintf(TEXT("\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n"));
			break;
		}
		byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
		ptr++;
	}

#ifdef VERBOSE
	//_tprintf(TEXT("\nGetLogicalProcessorInformation results:\n"));
	_tprintf(TEXT("Number of NUMA nodes: %d\n"), 
			 numaNodeCount);
	_tprintf(TEXT("Number of physical processor packages: %d\n"), 
			 processorPackageCount);
	_tprintf(TEXT("Number of processor cores: %d\n"), 
			 processorCoreCount);
	_tprintf(TEXT("Number of logical processors: %d\n"), 
			 logicalProcessorCount);
	_tprintf(TEXT("Number of processor L1/L2/L3 caches: %d/%d/%d\n"), 
			 processorL1CacheCount,
			 processorL2CacheCount,
			 processorL3CacheCount);
#endif
	
	free(buffer);

	xmem::common::g_num_logical_cpus = logicalProcessorCount;
	xmem::common::g_num_nodes = numaNodeCount;
	xmem::common::g_num_physical_packages = processorPackageCount;

	if (!xmem::common::win::config_page_size()) //check success
		return -1;

#ifdef VERBOSE
#ifdef USE_LARGE_PAGES
	_tprintf(TEXT("Large page size: %d B\n"), xmem::common::g_large_page_size);
#else
	_tprintf(TEXT("Page size: %d B\n"), PAGE_SIZE);
#endif
#endif
	
	return 0;
}
*/

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
