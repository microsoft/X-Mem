/**
 * @file
 * 
 * @brief Header file for some common Windows helper stuff.
 * 
 * (C) 2014 Microsoft Corporation
 */

#ifndef __WIN_COMMON_H
#define __WIN_COMMON_H

#ifdef _WIN32

namespace xmem {
	namespace common {
		namespace win {
			/**
			 * @brief Configures global variables to track the OS/platform-specific page size in bytes.
			 * @returns True on success.
			 */
			bool config_page_size();
		};
	};
};

#else
#error Windows is the only supported OS at this time.
#endif

#endif
