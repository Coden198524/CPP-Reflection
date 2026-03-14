# 构建系统文档

## 概述

CPP-Reflection 使用 CMake 作为构建系统，项目包含三个独立的构建目标：

| 目标 | CMakeLists.txt 位置 | 输出 | 依赖 |
|------|---------------------|------|------|
| **MetaRuntime** | `Source/Runtime/CMakeLists.txt` | 静态库 | 无 |
| **MetaParser** | `Source/Parser/CMakeLists.txt` | 可执行文件 | LLVM, Boost |
| **Examples** | `Examples/CMakeLists.txt` | 多个可执行文件 | LLVM, Boost |

---

## 依赖安装

### LLVM 3.8.0+（LibClang）

MetaParser 依赖 LibClang 进行 C++ AST 解析。

**Windows：**

1. 下载预编译安装包：
   - [32位](http://llvm.org/releases/3.8.0/LLVM-3.8.0-win32.exe)
   - [64位](http://llvm.org/releases/3.8.0/LLVM-3.8.0-win64.exe)
2. 运行安装程序
3. 设置环境变量：

```bat
set LLVM_ROOT=C:\Program Files\LLVM
```

**Linux（Ubuntu/Mint）：**

```bash
sudo apt-get install libclang-3.8-dev
export LLVM_ROOT=/usr/lib/llvm-3.8
```

**macOS（Homebrew）：**

```bash
brew install llvm@3.8
export LLVM_ROOT=$(brew --prefix llvm@3.8)
```

### Boost 1.59.0+

MetaParser 使用 Boost 的 Program Options 和 Filesystem 库。

**从源码编译（推荐）：**

1. 下载 [Boost 1.59.0 源码](https://sourceforge.net/projects/boost/files/boost/1.59.0/)

2. 编译安装：

**Unix：**
```bash
cd boost_1_59_0
./bootstrap.sh
./b2 install --prefix=/usr/local
export BOOST_ROOT=/usr/local
```

**Windows：**
```bat
cd boost_1_59_0
bootstrap.bat
b2 install --prefix=C:\Boost
set BOOST_ROOT=C:\Boost
```

### C++ 编译器

需要支持 C++11 的编译器：

| 编译器 | 最低版本 | 平台 |
|--------|----------|------|
| MSVC | 14 (Visual Studio 2015) | Windows |
| G++ | 4.8 | Linux |
| Clang++ | 3.6 | macOS / Linux |

---

## 构建 MetaRuntime

运行时库没有外部依赖，构建最为简单。

### 步骤

```bash
# 从项目根目录开始
mkdir Build && cd Build

# 生成构建系统
cmake -G "<生成器>" ../Source/Runtime

# 构建
cmake --build . --target MetaRuntime
```

### 各平台生成器示例

**Linux / macOS：**
```bash
cmake -G "Unix Makefiles" ../Source/Runtime
cmake --build . --target MetaRuntime
```

**Windows（Visual Studio 2015）：**
```bat
cmake -G "Visual Studio 14 2015 Win64" ..\Source\Runtime
cmake --build . --target MetaRuntime --config Release
```

**Windows（MinGW）：**
```bat
cmake -G "MinGW Makefiles" ..\Source\Runtime
cmake --build . --target MetaRuntime
```

**Ninja（跨平台）：**
```bash
cmake -G "Ninja" ../Source/Runtime
cmake --build . --target MetaRuntime
```

### 输出

构建成功后，在 Build 目录下生成 `MetaRuntime` 静态库文件：
- Linux: `libMetaRuntime.a`
- Windows: `MetaRuntime.lib`

---

## 构建 MetaParser

解析器依赖 LLVM 和 Boost，需要先安装这两个依赖。

### 步骤

```bash
mkdir Build && cd Build

# 使用环境变量（推荐）
cmake -G "<生成器>" ../Source/Parser

# 或手动指定路径
cmake -G "<生成器>" ../Source/Parser \
    -DLLVM_ROOT=/usr/lib/llvm-3.8 \
    -DBOOST_ROOT=/usr/local
```

### CMake 变量

| 变量 | 说明 | 默认值 |
|------|------|--------|
| `LLVM_ROOT` | LLVM 安装根目录 | 从环境变量读取 |
| `BOOST_ROOT` | Boost 安装根目录 | 从环境变量读取 |

### 完整示例

**Linux：**
```bash
mkdir Build && cd Build
cmake -G "Unix Makefiles" ../Source/Parser \
    -DLLVM_ROOT=/usr/lib/llvm-3.8 \
    -DBOOST_ROOT=/usr/local
cmake --build . --target MetaParser
```

**Windows：**
```bat
mkdir Build && cd Build
cmake -G "Visual Studio 14 2015 Win64" ..\Source\Parser ^
    -DLLVM_ROOT="C:\Program Files\LLVM" ^
    -DBOOST_ROOT="C:\Boost"
cmake --build . --target MetaParser --config Release
```

### 输出

构建成功后生成 `MetaParser` 可执行文件：
- Linux: `MetaParser`
- Windows: `MetaParser.exe`

---

## 构建示例

示例程序依赖 MetaRuntime 和 MetaParser，同时也需要 LLVM 和 Boost。

### 步骤

```bash
mkdir Build && cd Build

cmake -G "<生成器>" ../Examples \
    -DLLVM_ROOT=$LLVM_ROOT \
    -DBOOST_ROOT=$BOOST_ROOT

cmake --build .
```

### 示例列表

构建完成后会生成以下可执行文件：

| 可执行文件 | 源文件 | 演示功能 |
|-----------|--------|----------|
| `Arrays` | `Examples/Arrays.cpp` | 数组反射和操作 |
| `Enums` | `Examples/Enums.cpp` | 枚举反射 |
| `FunctionsAndMethods` | `Examples/FunctionsAndMethods.cpp` | 方法调用 |
| `MetaProperties` | `Examples/MetaProperties.cpp` | 元属性系统 |
| `Serialization` | `Examples/Serialization.cpp` | JSON 序列化 |

### 运行示例

```bash
# Linux / macOS
./Arrays
./Enums
./FunctionsAndMethods
./MetaProperties
./Serialization

# Windows
Arrays.exe
Enums.exe
FunctionsAndMethods.exe
MetaProperties.exe
Serialization.exe
```

---

## CMake 模块

项目在 `Source/CMake/` 目录下提供了辅助 CMake 模块：

### 查找 LLVM

CMake 通过 `LLVM_ROOT` 变量定位 LLVM 安装：

```cmake
# 查找 LibClang
find_library(LibClang_LIBRARY
    NAMES clang libclang
    PATHS ${LLVM_ROOT}/lib
)

find_path(LibClang_INCLUDE_DIR
    NAMES clang-c/Index.h
    PATHS ${LLVM_ROOT}/include
)
```

### 查找 Boost

```cmake
# 查找 Boost 组件
find_package(Boost 1.59.0 REQUIRED
    COMPONENTS filesystem program_options
)
```

---

## 集成到自己的项目

### 方式一：作为子目录

```cmake
cmake_minimum_required(VERSION 3.0)
project(MyProject)

set(CMAKE_CXX_STANDARD 11)

# 添加 Runtime 作为子目录
add_subdirectory(third_party/CPP-Reflection/Source/Runtime)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp MetaRuntime)
target_include_directories(MyApp PRIVATE
    third_party/CPP-Reflection/Source/Runtime
)
```

### 方式二：预编译库

```cmake
cmake_minimum_required(VERSION 3.0)
project(MyProject)

set(CMAKE_CXX_STANDARD 11)

# 指定预编译的 MetaRuntime 路径
set(META_RUNTIME_DIR /path/to/CPP-Reflection/Build)

add_executable(MyApp main.cpp)

target_link_libraries(MyApp
    ${META_RUNTIME_DIR}/libMetaRuntime.a
)

target_include_directories(MyApp PRIVATE
    /path/to/CPP-Reflection/Source/Runtime
)
```

### 添加 Parser 自动化

```cmake
# 定义 Parser 路径
set(META_PARSER /path/to/MetaParser)

# 自定义命令：在编译前运行 Parser
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/Generated/MyModule.generated.cpp
    COMMAND ${META_PARSER}
        --target MyModule
        --source-root ${CMAKE_CURRENT_SOURCE_DIR}/Source
        --in-source ${CMAKE_CURRENT_SOURCE_DIR}/Source/AllTypes.h
        --module-header ${CMAKE_CURRENT_SOURCE_DIR}/Source/MyModule.h
        --out-source ${CMAKE_BINARY_DIR}/Generated/MyModule.generated.cpp
        --out-dir ${CMAKE_BINARY_DIR}/Generated
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/AllTypes.h
    COMMENT "Generating reflection data..."
)

# 将生成的源文件加入编译
add_executable(MyApp
    main.cpp
    ${CMAKE_BINARY_DIR}/Generated/MyModule.generated.cpp
)
```

---

## 常见构建问题

### CMake 找不到 LLVM

**症状：** `Could not find LibClang`

**解决方案：**
```bash
# 确认 LLVM 安装路径
ls $LLVM_ROOT/lib/libclang*

# 手动指定
cmake -DLLVM_ROOT=/actual/path/to/llvm ..
```

### CMake 找不到 Boost

**症状：** `Could not find Boost`

**解决方案：**
```bash
# 确认 Boost 安装路径
ls $BOOST_ROOT/lib/libboost_*

# 手动指定
cmake -DBOOST_ROOT=/actual/path/to/boost ..

# 如果 Boost 安装在非标准位置
cmake -DBoost_NO_SYSTEM_PATHS=ON -DBOOST_ROOT=/path/to/boost ..
```

### 链接错误：未定义的引用

**症状：** `undefined reference to 'ursine::meta::...'`

**解决方案：** 确保链接了 MetaRuntime 库：
```cmake
target_link_libraries(MyApp MetaRuntime)
```

### Windows 下 LibClang DLL 找不到

**症状：** 运行 MetaParser 时报 `libclang.dll not found`

**解决方案：** 将 LLVM 的 `bin` 目录添加到 PATH：
```bat
set PATH=%LLVM_ROOT%\bin;%PATH%
```

### 编译器版本不兼容

**症状：** 大量 C++11 语法错误

**解决方案：** 确保使用支持 C++11 的编译器，并在 CMake 中启用：
```cmake
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

---

## 相关文档

- [快速入门](./GettingStarted.md) — 从零开始搭建环境
- [Parser 使用指南](./Parser-Guide.md) — Parser 的详细使用说明
- [示例详解](./Examples.md) — 示例程序的构建和运行
