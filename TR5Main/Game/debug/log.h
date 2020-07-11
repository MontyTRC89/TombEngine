#pragma once
#if _DEBUG
#include <iostream>
#include <ostream>
#ifndef Log
#define Log(x) std::cout << x << std::endl;
#endif
#else 
#define Log(x)
#endif