#pragma once

#include "common.hpp"

#ifndef WINDOWS

#include <format>
#include <unistd.h>
#include <sys/types.h>
#include <libgen.h>
#include <sys/wait.h>
#include <string.h>
#include <glob.h>
#include <sys/stat.h>

class Postprocessor
{
public:
    static void assemble(const string &input_path, const string &output_path);
    static void run_linker(const vector<string> &inputs, const string &output);
    static string create_tmpfile();

private:
    Postprocessor();
    static void run_subprocess(const vector<string> &argv);
    static char *find_file(const char *patern);
    static bool file_exists(const char *path);
    static char *find_libpath();
    static char *find_gcc_libpath();
};

#endif /* WINDOWS */