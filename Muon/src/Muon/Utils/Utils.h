/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/3
Description : Utility functions
----------------------------------------------*/
#ifndef MUON_UTILS_H
#define MUON_UTILS_H

namespace Muon
{
	inline void MUON_API Print(const char* str);
	inline void MUON_API Print(const wchar_t* str);
	inline void MUON_API Printf(const char* format, ...);
	inline void MUON_API Printf(const wchar_t* format, ...);
}

#endif