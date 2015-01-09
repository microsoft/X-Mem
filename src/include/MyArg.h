/**
 * @file
 * 
 * @brief Extensions to third-party optionparser-related code.
 */

#ifndef __MY_ARG_H
#define __MY_ARG_H

#include <cstdint>
#include <stdio.h>
#include <include/ExampleArg.h>

namespace xmem {
	namespace config {
		namespace third_party {

			class MyArg : public ExampleArg
			{
			public:
				/**
				 * @brief Checks an option that it is an integer.
				 */
				static ArgStatus Integer(const Option& option, bool msg) {
					char* endptr = 0;
					if (option.arg != 0 && strtol(option.arg, &endptr, 10)) {};
					if (endptr != option.arg && *endptr == 0)
						return ARG_OK;

					if (msg)
						printError("Option '", option, "' requires an integer argument\n");
					return ARG_ILLEGAL;
				}

				/**
				 * @brief Checks an option that it is a nonnegative integer.
				 */
				static ArgStatus NonnegativeInteger(const Option& option, bool msg) {
					char* endptr = 0;
					int64_t tmp = -1;
					if (option.arg != 0)
						tmp = strtol(option.arg, &endptr, 10);
					if (endptr != option.arg && *endptr == 0 && tmp >= 0)
						return ARG_OK;

					if (msg)
						printError("Option '", option, "' requires a non-negative integer argument\n");
					return ARG_ILLEGAL;
				}

				/**
				 * @brief Checks an option that it is a positive integer.
				 */
				static ArgStatus PositiveInteger(const Option& option, bool msg) {
					char* endptr = 0;
					int64_t tmp = -1;
					if (option.arg != 0)
						tmp = strtol(option.arg, &endptr, 10);
					if (endptr != option.arg && *endptr == 0 && tmp > 0)
						return ARG_OK;

					if (msg)
						printError("Option '", option, "' requires a positive integer argument\n");
					return ARG_ILLEGAL;
				}
			};
		};
	};
};

#endif
