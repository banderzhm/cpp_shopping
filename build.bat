@echo off
echo 正在设置Visual Studio环境...

:: 尝试查找Visual Studio安装路径
set "VS_PATH=F:\program\visul_studio\idea"
if not exist "%VS_PATH%" (
    echo 未找到Visual Studio安装路径，请修改脚本中的VS_PATH变量
    exit /b 1
)

:: 设置Visual Studio环境变量
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"
if %ERRORLEVEL% neq 0 (
    echo 设置Visual Studio环境变量失败
    exit /b 1
)

echo Visual Studio环境已设置

:: 创建构建目录
if not exist "build" mkdir build

:: 配置CMake
echo 正在配置CMake...
cmake -S . -B build
if %ERRORLEVEL% neq 0 (
    echo CMake配置失败
    exit /b 1
)

:: 构建项目
echo 正在构建项目...
cmake --build build
if %ERRORLEVEL% neq 0 (
    echo 项目构建失败
    exit /b 1
)

echo 构建完成 