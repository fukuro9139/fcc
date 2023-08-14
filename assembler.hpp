#pragma once

#include "common.hpp"

#ifndef WINDOWS

#include <unistd.h>
#include <sys/types.h>
#include <libgen.h>
#include <sys/wait.h>
#include <string.h>

class Assembler
{
public:
    static void assemble(const string &input_path, const string &output_path);
    static string create_tmpfile();

private:
    Assembler();
};

#endif /* WINDOWS */