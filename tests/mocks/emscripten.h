#ifndef MOCK_EMSCRIPTEN_H
#define MOCK_EMSCRIPTEN_H

#include <iostream>
#include <string>

// Mock EM_JS macro
// We declare the function so we can implement it in the test file
#define EM_JS(ret, name, args, ...) ret name args

// Mock EMSCRIPTEN_KEEPALIVE
#define EMSCRIPTEN_KEEPALIVE

#endif
