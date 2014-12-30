/**
 * @file
 * 
 * @brief Implementation file for some common Windows helper stuff.
 * 
 * (C) 2014 Microsoft Corporation
 */

//Headers
#include <common/common.h>
#include "win_common.h"

//Libraries
#include <windows.h>

#ifdef _WIN32

using namespace xmem::common::win;

bool xmem::common::win::config_page_size() {
	//Get page size
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	DWORD pgsz = sysinfo.dwPageSize;
	xmem::common::g_page_size = pgsz;

#ifdef USE_LARGE_PAGES
	xmem::common::g_large_page_size = GetLargePageMinimum();
#else
	xmem::common::g_large_page_size = 0;
#endif

	return true;
}

#else 
#error Windows is the only supported OS at this time.
#endif
