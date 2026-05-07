#pragma once

#include "exp_common.h"
#include <fstream>
#include <iomanip>
#include <string>

/** 默认目录：tests/output（相对当前工作目录，一般为 ColorWatermark 工程根） */
inline std::string testsOutputDir() {
    return "tests/output";
}

inline void testsEnsureOutputSubdir(const std::string& csvPath) {
    size_t pos = csvPath.find_last_of("\\/");
    if (pos != std::string::npos)
        expEnsureDir(csvPath.substr(0, pos));
    else
        expEnsureDir(testsOutputDir());
}

inline void testsCsvOpen(std::ofstream& out, const std::string& path) {
    testsEnsureOutputSubdir(path);
    out.open(path, std::ios::out | std::ios::trunc);
    out << std::fixed << std::setprecision(9);
}
