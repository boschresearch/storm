#ifndef _STORM_PARSER_READVALUES_H
#define _STORM_PARSER_READVALUES_H

#include "src/utility/cstring.h"

namespace storm
{
	namespace parser
	{

		template<typename T>
		T readValue(char const* buf);

		template<>
		double readValue<double>(char const* buf)
		{
		    return utility::cstring::checked_strtod(buf, &buf);
        }

	}
}	

#endif
