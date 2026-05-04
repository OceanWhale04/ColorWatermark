#include "CLI.h"
#include "WatermarkSystem.h"
#include <iostream>

int main(int argc, char* argv[]) {
    AppConfig cfg = CLI::parse(argc, argv);
    if (cfg.mode.empty()) {
        std::cerr << "Usage: --mode <embed|extract|compare_clahe|compare_wiener> ...\n";
        return -1;
    }
    WatermarkSystem system;
    return system.run(cfg);
}