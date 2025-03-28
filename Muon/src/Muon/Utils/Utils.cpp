/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/3
Description : Utility functions
----------------------------------------------*/

#include <Muon.h>
#include <Muon/Utils/Utils.h>

namespace Muon
{
	inline void MUON_API Print(const char* str)
	{
		OutputDebugStringA(str);
	}
}