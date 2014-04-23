// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


#ifdef _DEBUG
#pragma comment (lib, "../../gflog/debug/gflog_d.lib")
#else
#pragma comment (lib, "../../gflog/release/gflog.lib")
#endif // _DEBUG

// TODO: reference additional headers your program requires here
