/**
 * @file
 * 
 * @brief Header file for some third-party helper code for working with Windows APIs.
 */

#ifndef __WIN_THIRD_PARTY
#define __WIN_THIRD_PARTY

#ifdef _WIN32

//Libraries
#include <windows.h>
#include <cstdint>
#include <string>
#include <map>

//These are used by CPdhQuery class and DumpMap() third-party code. See below and win_CPdhQuery.h.
namespace std
{
	typedef std::basic_string<TCHAR> tstring;
	typedef std::basic_ostream<TCHAR> tostream;
	typedef std::basic_istream<TCHAR> tistream;
	typedef std::basic_ostringstream<TCHAR> tostringstream;
	typedef std::basic_istringstream<TCHAR> tistringstream;
	typedef std::basic_stringstream<TCHAR> tstringstream;
} 

#ifdef UNICODE
#define tcout std::wcout
#else
#define tcout std::cout
#endif

namespace xmem {
	namespace common {
		namespace win {
			namespace third_party {
				/**
				 * @brief Helpful typedef used by query_sys_info().
				 * Source: MSDN example code, http://msdn.microsoft.com/en-us/library/windows/desktop/ms683194(v=vs.85).aspx, retrieved September 2014.
				 */
				typedef BOOL (WINAPI *LPFN_GLPI)(
					PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, 
					PDWORD);
				/**
				 * @brief Third-party code for setting Windows privileges.
				 * Source: MSDN, http://msdn.microsoft.com/en-us/library/windows/desktop/aa446619(v=vs.85).aspx, retrieved September 2014.
				 * @returns True on success.
				 */
				BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);

				/**
				 * @brief Third-party code for counting set bits in the processor mask.
				 * Source: MSDN example code, http://msdn.microsoft.com/en-us/library/windows/desktop/ms683194(v=vs.85).aspx, retrieved September 2014.
				 * @returns The number of bits set in the processor mask.
				 */
				DWORD CountSetBits(ULONG_PTR bitMask);
	
				/**
				 * @brief Third-party code for querying important Windows platform information.
				 * Source: MSDN example code, http://msdn.microsoft.com/en-us/library/windows/desktop/ms683194(v=vs.85).aspx, retrieved September 2014.
				 * The function name has been changed from the original example, and code modified slightly.
				 * @returns 0 on success.
				 */
				int32_t query_sys_info();

				/**
				 * @brief Third-party code for printing the map values for some std::map object.
				 * Source: http://askldjd.wordpress.com/2011/01/05/a-pdh-helper-class-cpdhquery/, retrieved September 2014.
				 * @author Alan Ning
				 */
				void DumpMap(std::map<std::tstring, double> const &m);
				
				/**
				 * @brief Modified version of DumpMap() third-party code for retrieving the value for some std::map object.
				 *
				 * Source: http://askldjd.wordpress.com/2011/01/05/a-pdh-helper-class-cpdhquery/, retrieved September 2014.
				 * @author Alan Ning
				 */
				double DumpMapValue(std::map<std::tstring, double> const &m);
			};
		};
	};
};

#endif

#endif
