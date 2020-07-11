#pragma once
#include <cassert>
#ifndef assertm
#define assertm(exp, msg) assert(((void)msg, exp))
#endif