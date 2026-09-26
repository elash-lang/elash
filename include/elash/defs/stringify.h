#pragma once

#define _EL_XSTRINGIFY(...) #__VA_ARGS__
#define EL_STRINGIFY(...) _EL_XSTRINGIFY(__VA_ARGS__)
