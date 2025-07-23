#include <doctest/doctest.h>
#include "init_log.hpp"
#include <iostream>
int main(int argc, char** argv) {
    // 1. 创建 doctest 上下文
    doctest::Context context;

    // 2. 您可以在这里添加或覆盖 doctest 的默认设置
    // 例如：context.setOption("abort-after", 5); // 失败5次后中止
    // 例如：context.setOption("no-breaks", true); // 在断点处不中断

    // 将命令行参数传递给 doctest
    context.applyCommandLine(argc, argv);

    // 3. 执行您自己的应用程序初始化代码
    init_log();
    std::cout << "cpp_shopping 主程序启动成功！" << std::endl;
    
    // 4. 运行 doctest 测试用例
    // 这将执行上面定义的 TEST_CASE("cpp_shopping main logic")
    int test_result = context.run();

    // 5. 检查测试结果
    if (context.shouldExit()) {
        // 如果doctest因为--exit之类的选项需要退出，
        // 或者因为测试失败而需要立即退出，则直接返回结果
        return test_result;
    }

    // 6. 在测试运行后，您可以继续执行其他应用程序逻辑
    // （如果您的设计是先跑通测试再继续的话）
    std::cout << "所有测试用例执行完毕。" << std::endl;
    
    // 重置上下文状态（如果需要的话）
    context.clearFilters(); 

    // 这里可以继续执行其他非测试相关的代码...
    // std::cout << "执行其他主程序任务..." << std::endl;

    return test_result;
}