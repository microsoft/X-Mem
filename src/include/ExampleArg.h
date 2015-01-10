/**
 * @file
 * 
 * @brief Slightly-modified third-party code related to OptionParser.
 */

#ifndef __EXAMPLE_ARG_H
#define __EXAMPLE_ARG_H

//Headers
#include <optionparser.h> 

//Libraries
#include <cstdint>
#include <stdio.h>

namespace xmem {
	namespace config {
		namespace third_party {

			class ExampleArg : public Arg
			{
			public:
				static void printError(const char* msg1, const Option& opt, const char* msg2) {
					fprintf(stderr, "ERROR: %s", msg1);
					fwrite(opt.name, opt.namelen, 1, stderr);
					fprintf(stderr, "%s", msg2);
				}

				static ArgStatus Unknown(const Option& option, bool msg) {
					if (msg)
						printError("Unknown option '", option, "'\n");
					return ARG_ILLEGAL;
				}

				static ArgStatus Required(const Option& option, bool msg) {
					if (option.arg != 0)
						return ARG_OK;

					if (msg)
						printError("Option '", option, "' requires an argument\n");
					return ARG_ILLEGAL;
				}

				static ArgStatus NonEmpty(const Option& option, bool msg) {
					if (option.arg != 0 && option.arg[0] != 0)
						return ARG_OK;

					if (msg)
						printError("Option '", option, "' requires a non-empty argument\n");
					return ARG_ILLEGAL;
				}
			};
		};
	};
};

#endif
