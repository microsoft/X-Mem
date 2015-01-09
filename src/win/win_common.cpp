/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Microsoft
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file
 * 
 * @brief Implementation file for some common Windows helper stuff.
 */

//Headers
#include <include/common.h>
#include <include/win/win_common.h>

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
