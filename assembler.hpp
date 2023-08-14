#pragma once

#include "common.hpp"

#ifndef WINDOWS

#include <unistd.h>
#include <libgen.h>

class Assembler{

    static void assemble(const string &input_path, const string &output_path);
};


#endif /* WINDOWS */