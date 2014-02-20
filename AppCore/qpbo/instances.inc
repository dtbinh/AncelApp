#include "QPBO.h"

#ifdef _MSC_VER
#pragma warning(disable: 4661)
#endif

namespace KQPBO
{

	inline void QPBO::get_type_information(char*& type_name, char*& type_format)
{
	type_name = "double";
	type_format = "Lf";
}

};

