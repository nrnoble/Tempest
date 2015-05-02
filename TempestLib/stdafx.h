
#ifndef STDAFX_H
#define STDAFX_H

// disable some warnings that including the standard libraries cause later on
#pragma warning(disable : 4514)	// unreferenced inlines
#pragma warning(disable : 4710)	// function not inlined

// and these are some that I just don't like
#pragma warning(disable : 4571)  // informational... SEH exceptions not caught by catch (...)

// temporarily disable some warnings that get reported while loading system headers
#pragma warning(push)
#pragma warning(disable : 4350)	// non-const reference thing
#pragma warning(disable : 4820)	// padding in structures

// old school C
#include <stdint.h>
#include <stdio.h>

// STL
#include <string>
#include <vector>

// restore warning state
#pragma warning(pop)

#endif
