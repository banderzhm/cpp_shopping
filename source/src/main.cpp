#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include "init_log.hpp"

int main(int argc, char** argv) {
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    
    // 初始化日志
    init_log();
    
    int res = context.run();
    
    return res;
} 