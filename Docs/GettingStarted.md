# 快速入门指南

## 简介

本指南将帮助你从零开始搭建 CPP-Reflection 开发环境，并运行第一个反射示例。完成本指南后，你将了解如何：

- 安装必要的依赖
- 构建运行时库和解析器
- 为自己的类型添加反射注解
- 使用反射 API 查询和操作类型信息

---

## 前置要求

在开始之前，请确保你的系统满足以下要求：

| 工具 | 最低版本 | 说明 |
|------|----------|------|
| CMake | 3.0+ | 构建系统 |
| C++ 编译器 | C++11 | MSVC 14 / G++ 4.8 / Clang++ 3.6 |
| LLVM / LibClang | 3.8.0 | Parser 依赖 |
| Boost | 1.59.0 | Parser 依赖 |

> 注意：如果你只使用 Runtime 库（不运行 Parser），则不需要 LLVM 和 Boost。

---

## 第一步：安装依赖

### 安装 LLVM 3.8

**Windows：**

下载预编译二进制文件：
- [32位版本](http://llvm.org/releases/3.8.0/LLVM-3.8.0-win32.exe)
- [64位版本](http://llvm.org/releases/3.8.0/LLVM-3.8.0-win64.exe)

安装完成后，设置环境变量：

```bat
set LLVM_ROOT=C:\Program Files\LLVM
```

**Linux（以 Ubuntu/Mint 为例）：**

```bash
sudo apt-get install libclang-3.8-dev
```

安装路径通常为 `/usr/lib/llvm-3.8`，设置环境变量：

```bash
export LLVM_ROOT=/usr/lib/llvm-3.8
```

### 安装 Boost 1.59

从 [Boost 官网](https://sourceforge.net/projects/boost/files/boost/1.59.0/) 下载源码，然后按照以下步骤编译：

**Unix 系统：**

```bash
./bootstrap.sh
./b2 install
export BOOST_ROOT=/usr/local
```

**Windows：**

```bat
bootstrap.bat
b2 install
set BOOST_ROOT=C:\boost_1_59_0
```

---

## 第二步：构建运行时库

运行时库（MetaRuntime）没有外部依赖，构建最为简单。

```bash
# 在项目根目录下
mkdir Build && cd Build

# 生成构建系统（以 Unix Makefiles 为例）
cmake -G "Unix Makefiles" ../Source/Runtime

# 构建
cmake --build . --target MetaRuntime
```

**Windows 使用 Visual Studio：**

```bat
mkdir Build && cd Build
cmake -G "Visual Studio 14 2015 Win64" ..\Source\Runtime
cmake --build . --target MetaRuntime --config Release
```

构建成功后，你将得到 `MetaRuntime` 静态库文件。

---

## 第三步：构建解析器

解析器（MetaParser）需要 LLVM 和 Boost 依赖。

```bash
mkdir Build && cd Build

# 如果已设置环境变量
cmake -G "Unix Makefiles" ../Source/Parser

# 如果未设置环境变量，手动指定路径
cmake -G "Unix Makefiles" ../Source/Parser \
    -DLLVM_ROOT=/usr/lib/llvm-3.8 \
    -DBOOST_ROOT=/usr/local

cmake --build . --target MetaParser
```

---

## 第四步：构建并运行示例

```bash
mkdir Build && cd Build

cmake -G "Unix Makefiles" ../Examples \
    -DLLVM_ROOT=$LLVM_ROOT \
    -DBOOST_ROOT=$BOOST_ROOT

cmake --build .
```

运行示例：

```bash
# 运行数组反射示例
./Arrays

# 运行枚举反射示例
./Enums

# 运行方法调用示例
./FunctionsAndMethods

# 运行元属性示例
./MetaProperties

# 运行序列化示例
./Serialization
```

---

## 第五步：第一个反射程序

下面通过一个完整示例演示如何使用反射库。

### 1. 定义需要反射的类型

在头文件中使用 `Meta(Enable)` 注解标记需要反射的类型和成员：

```cpp
// MyTypes.h
#pragma once

#include <Meta.h>
#include <string>

// 标记枚举需要反射
enum class Meta(Enable) Color {
    Red,
    Green,
    Blue
};

// 标记结构体需要反射
struct Meta(Enable) Player {
    // 标记字段需要反射
    std::string Meta(Enable) name;
    int Meta(Enable) health;
    float Meta(Enable) speed;

    // 标记方法需要反射
    void Meta(Enable) TakeDamage(int amount) {
        health -= amount;
    }

    std::string Meta(Enable) GetName() const {
        return name;
    }
};
```

### 2. 运行解析器生成元数据

在构建流程中运行 MetaParser，它会分析你的头文件并生成反射注册代码：

```bash
MetaParser --target MyModule --input MyTypes.h --output Generated/
```

这会生成 `Generated/MyModule.generated.h`，其中包含类型注册代码。

### 3. 在程序中使用反射

```cpp
// main.cpp
#include "Generated/MyModule.generated.h"
#include "MyTypes.h"

using namespace ursine;
using namespace meta;

int main(void)
{
    // 初始化反射系统，加载模块
    MetaInitialize( UsingModule( MyModule ) );

    // -------------------------------------------------------
    // 查询类型信息
    // -------------------------------------------------------
    Type playerType = typeof( Player );
    std::cout << "类型名称: " << playerType.GetName() << std::endl;

    // -------------------------------------------------------
    // 创建对象实例
    // -------------------------------------------------------
    Variant player = playerType.GetConstructor().Invoke();

    // -------------------------------------------------------
    // 读写字段
    // -------------------------------------------------------
    Field nameField = playerType.GetField("name");
    Field healthField = playerType.GetField("health");

    nameField.SetValue(player, std::string("英雄"));
    healthField.SetValue(player, 100);

    std::string name = nameField.GetValue(player).GetValue<std::string>();
    int health = healthField.GetValue(player).ToInt();

    std::cout << "玩家名称: " << name << std::endl;
    std::cout << "玩家血量: " << health << std::endl;

    // -------------------------------------------------------
    // 调用方法
    // -------------------------------------------------------
    Method takeDamageMethod = playerType.GetMethod("TakeDamage");
    takeDamageMethod.Invoke(player, 30);

    std::cout << "受伤后血量: " << healthField.GetValue(player).ToInt() << std::endl;

    // -------------------------------------------------------
    // 枚举反射
    // -------------------------------------------------------
    Type colorType = typeof( Color );
    const Enum &colorEnum = colorType.GetEnum();

    // 枚举值转字符串
    Variant redValue = Color::Red;
    std::cout << "Red 的字符串: " << colorEnum.GetKey(redValue) << std::endl;

    // 字符串转枚举值
    Variant blueValue = colorEnum.GetValue("Blue");
    std::cout << "Blue 的整数值: " << blueValue.ToInt() << std::endl;

    return 0;
}
```

### 4. 预期输出

```
类型名称: Player
玩家名称: 英雄
玩家血量: 100
受伤后血量: 70
Red 的字符串: Red
Blue 的整数值: 2
```

---

## 常用 API 速查

### 获取类型

```cpp
// 通过类型名（编译时）
Type t1 = typeof( MyClass );

// 通过字符串名（运行时）
Type t2 = Type::GetFromName("MyClass");

// 通过对象实例
MyClass obj;
Type t3 = Type::Get(obj);
```

### 操作字段

```cpp
Type t = typeof( MyClass );
Field f = t.GetField("fieldName");

Variant instance = /* ... */;

// 读取字段值
Variant value = f.GetValue(instance);
int intVal = value.ToInt();
float floatVal = value.ToFloat();
std::string strVal = value.ToString();

// 写入字段值
f.SetValue(instance, 42);
f.SetValue(instance, std::string("hello"));
```

### 调用方法

```cpp
Method m = t.GetMethod("methodName");

// 无参调用
Variant result = m.Invoke(instance);

// 带参调用
Variant result2 = m.Invoke(instance, arg1, arg2);

// 带参调用（使用 ArgumentList）
ArgumentList args = { arg1, arg2 };
Variant result3 = m.Invoke(instance, args);
```

### 创建对象

```cpp
// 默认构造
Variant obj = t.GetConstructor().Invoke();

// 带参构造
Variant obj2 = t.GetConstructor({ typeof(int), typeof(float) }).Invoke(1, 2.0f);

// 动态构造（返回堆上的指针）
Variant obj3 = t.GetDynamicConstructor().Invoke();
// 使用完毕后需要销毁
t.Destroy(obj3);
```

### 枚举操作

```cpp
Type enumType = typeof( MyEnum );
const Enum &e = enumType.GetEnum();

// 获取所有键
std::vector<std::string> keys = e.GetKeys();

// 值转字符串
std::string key = e.GetKey( MyEnum::Value );

// 字符串转值
Variant val = e.GetValue("Value");
```

---

## 集成到自己的项目

### CMakeLists.txt 配置

```cmake
cmake_minimum_required(VERSION 3.0)
project(MyProject)

set(CMAKE_CXX_STANDARD 11)

# 添加 MetaRuntime 库
add_subdirectory(path/to/CPP-Reflection/Source/Runtime)

# 你的可执行文件
add_executable(MyApp main.cpp)

# 链接 MetaRuntime
target_link_libraries(MyApp MetaRuntime)

# 包含头文件路径
target_include_directories(MyApp PRIVATE
    path/to/CPP-Reflection/Source/Runtime
)
```

### 构建流程集成

在 CMake 中添加自定义命令，在编译前自动运行 MetaParser：

```cmake
# 运行解析器生成反射代码
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Generated/MyModule.generated.h
    COMMAND MetaParser
        --target MyModule
        --input ${CMAKE_CURRENT_SOURCE_DIR}/MyTypes.h
        --output ${CMAKE_CURRENT_BINARY_DIR}/Generated/
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/MyTypes.h
    COMMENT "Generating reflection metadata..."
)
```

---

## 常见问题

**Q: 初始化时报错找不到类型？**

确保在使用任何反射 API 之前调用了 `MetaInitialize(UsingModule(...))`，且传入了正确的模块名。

**Q: `GetField` 返回无效字段？**

检查字段是否在头文件中添加了 `Meta(Enable)` 注解，并且 Parser 已重新运行生成了最新的元数据。

**Q: `Invoke` 调用失败？**

确认方法签名与调用时传入的参数类型匹配。反射系统支持重载，可以通过 `GetMethod(name, signature)` 指定具体重载。

**Q: Windows 下找不到 LibClang？**

确保 `LLVM_ROOT` 环境变量指向 LLVM 安装目录，且该目录下存在 `lib/libclang.lib`。

---

## 下一步

- [架构概览](./Architecture.md) — 深入理解系统设计
- [Runtime API 参考](./Runtime-API.md) — 完整 API 文档
- [示例详解](./Examples.md) — 详细了解每个示例程序
- [高级主题](./Advanced-Topics.md) — 序列化、元属性等高级用法
