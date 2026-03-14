# Parser 使用指南

## 概述

MetaParser 是 CPP-Reflection 的代码分析工具，基于 LibClang 解析 C++ 源代码的抽象语法树（AST），提取带有 `Meta()` 注解的类型信息，并生成对应的反射注册代码。

Parser 在**编译前**运行，生成的代码随后被编译进最终的可执行文件。

---

## 工作原理

```
你的 C++ 头文件
(含 Meta() 注解)
        │
        ▼
  MetaParser 可执行文件
  ├── LibClang 解析 AST
  ├── 识别 Meta() 注解
  ├── 提取类型/方法/字段信息
  └── 使用 Mustache 模板渲染
        │
        ▼
  生成的 .generated.h / .generated.cpp
  (包含类型注册代码)
        │
        ▼
  编译进你的程序
        │
        ▼
  运行时通过 MetaInitialize() 加载
```

---

## 命令行参数

MetaParser 通过命令行参数控制解析行为：

| 参数 | 必填 | 说明 |
|------|------|------|
| `--target` | ✓ | 目标模块名称（如 `MyModule`） |
| `--source-root` | ✓ | 源文件根目录（所有头文件的公共父目录） |
| `--in-source` | ✓ | 要解析的输入头文件路径 |
| `--module-header` | ✓ | 声明此反射模块的头文件 |
| `--out-source` | ✓ | 输出的 C++ 模块源文件路径 |
| `--out-dir` | ✓ | 输出目录（生成的头文件和源文件） |
| `--template-dir` | 否 | Mustache 模板目录（默认 `Templates/`） |
| `--pch` | 否 | 预编译头文件名称 |
| `--force-rebuild` | 否 | 忽略缓存，强制重新生成 |
| `--display-diagnostics` | 否 | 显示 LibClang 诊断信息 |
| `--compiler-includes` | 否 | 包含目录列表文件 |
| `--compiler-defines` | 否 | 编译器宏定义列表 |

### 示例调用

```bash
MetaParser \
    --target MyModule \
    --source-root ./Source \
    --in-source ./Source/MyTypes.h \
    --module-header ./Source/MyModule.h \
    --out-source ./Generated/MyModule.generated.cpp \
    --out-dir ./Generated/ \
    --template-dir ./Templates/ \
    --compiler-includes ./includes.txt \
    --compiler-defines MY_DEFINE=1 ANOTHER_DEFINE
```

---

## 注解语法

### 基本注解

使用 `Meta(...)` 宏标记需要反射的元素：

```cpp
#include <Meta.h>

// 标记类/结构体
class Meta(Enable) MyClass { };

// 标记枚举
enum class Meta(Enable) MyEnum { A, B, C };

// 标记字段
struct MyStruct {
    int Meta(Enable) value;
};

// 标记方法
class MyClass {
    void Meta(Enable) Update(float dt);
};
```

### 附加元属性

`Meta()` 宏可以接受多个参数，第一个通常是 `Enable`，后续参数为自定义元属性：

```cpp
struct SoundEffect {
    // 附加 Range 和 Slider 元属性
    float Meta(Enable, Range(0.0f, 100.0f), Slider(SliderType::Horizontal)) volume;
};
```

元属性是普通的 C++ 类型，在运行时通过 `MetaManager` 访问：

```cpp
// 定义元属性类型
struct Range {
    float min;
    float max;
    Range(float min, float max) : min(min), max(max) {}
};
```

### 解析器宏定义

Parser 运行时会自动定义 `__REFLECTION_PARSER__` 宏，可以用于条件编译：

```cpp
#ifdef __REFLECTION_PARSER__
    // 仅在解析阶段可见的代码
    #define Meta(...) __attribute__((annotate(#__VA_ARGS__)))
#else
    // 运行时的实际定义
    #define Meta(...)
#endif
```

---

## 生成的代码结构

Parser 生成两类文件：

### 模块头文件（.h）

声明模块初始化函数：

```cpp
// MyModule.generated.h（示例）
#pragma once

#define DECLARE_META_MODULE(module) \
    extern void Initialize##module##MetaData(void);

DECLARE_META_MODULE(MyModule)

#define UsingModule(module) Initialize##module##MetaData
```

### 模块源文件（.cpp）

包含所有类型的注册代码：

```cpp
// MyModule.generated.cpp（示例，自动生成）
#include "MyModule.generated.h"
#include "MyTypes.h"

void InitializeMyModuleMetaData(void)
{
    // 注册 MyClass
    auto &myClassType = TypeDatabase::Register<MyClass>("MyClass");
    myClassType.AddConstructor<MyClass>();
    myClassType.AddField("value", &MyClass::value);
    myClassType.AddMethod("Update", &MyClass::Update);

    // 注册 MyEnum
    auto &myEnumType = TypeDatabase::Register<MyEnum>("MyEnum");
    myEnumType.AddEnumValue("A", MyEnum::A);
    myEnumType.AddEnumValue("B", MyEnum::B);
    myEnumType.AddEnumValue("C", MyEnum::C);
}
```

> 注意：生成的文件不应手动编辑，每次运行 Parser 都会覆盖。

---

## 与 CMake 集成

### 基本集成

在 CMakeLists.txt 中添加自定义命令，在编译前自动运行 Parser：

```cmake
# 定义 Parser 可执行文件路径
set(META_PARSER_EXE ${CMAKE_BINARY_DIR}/MetaParser)

# 定义输入和输出
set(META_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/Source/MyTypes.h)
set(META_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Generated)
set(META_OUTPUT_SOURCE ${META_OUTPUT_DIR}/MyModule.generated.cpp)

# 创建输出目录
file(MAKE_DIRECTORY ${META_OUTPUT_DIR})

# 添加自定义命令
add_custom_command(
    OUTPUT ${META_OUTPUT_SOURCE}
    COMMAND ${META_PARSER_EXE}
        --target MyModule
        --source-root ${CMAKE_CURRENT_SOURCE_DIR}/Source
        --in-source ${META_INPUT}
        --module-header ${CMAKE_CURRENT_SOURCE_DIR}/Source/MyModule.h
        --out-source ${META_OUTPUT_SOURCE}
        --out-dir ${META_OUTPUT_DIR}
        --template-dir ${CMAKE_CURRENT_SOURCE_DIR}/Templates
    DEPENDS ${META_INPUT}
    COMMENT "Generating reflection metadata for MyModule..."
)

# 将生成的文件加入编译目标
add_executable(MyApp
    main.cpp
    ${META_OUTPUT_SOURCE}
)

target_include_directories(MyApp PRIVATE
    ${META_OUTPUT_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)
```

### 处理多个头文件

如果需要反射多个头文件中的类型，创建一个聚合头文件：

```cpp
// AllTypes.h — 聚合所有需要反射的头文件
#pragma once

#include "Player.h"
#include "Enemy.h"
#include "Item.h"
#include "GameState.h"
```

然后将 `AllTypes.h` 作为 Parser 的输入文件。

### 包含目录配置

如果你的头文件依赖其他包含目录，创建一个包含目录列表文件：

```
# includes.txt
/path/to/include1
/path/to/include2
C:/Libraries/MyLib/include
```

然后通过 `--compiler-includes includes.txt` 传递给 Parser。

---

## 模板系统

Parser 使用 [Mustache](https://mustache.github.io/) 模板引擎生成代码。模板文件位于 `Templates/` 目录下，可以自定义生成的代码格式。

### 主要模板文件

| 模板文件 | 用途 |
|----------|------|
| `Class.h.mustache` | 类的头文件模板 |
| `Class.cpp.mustache` | 类的源文件模板 |
| `Enum.h.mustache` | 枚举的头文件模板 |
| `Module.h.mustache` | 模块头文件模板 |
| `Module.cpp.mustache` | 模块源文件模板 |

---

## 常见问题

### Parser 报错找不到头文件

确保通过 `--compiler-includes` 提供了所有必要的包含目录，或者通过 `--compiler-defines` 提供了必要的宏定义。

```bash
# 创建包含目录文件
echo "/usr/include/c++/7" > includes.txt
echo "/usr/local/include" >> includes.txt

MetaParser --compiler-includes includes.txt ...
```

### 类型没有被反射

检查以下几点：
1. 头文件中是否添加了 `Meta(Enable)` 注解
2. 该头文件是否被 Parser 的输入文件直接或间接包含
3. 是否重新运行了 Parser（或使用 `--force-rebuild`）

### 生成代码编译错误

通常是因为生成的代码引用了不完整的类型。确保：
1. 所有被反射的类型在输入头文件中完整定义
2. 所有依赖的头文件都已包含
3. 使用 `--display-diagnostics` 查看 LibClang 的详细诊断信息

### 增量构建不触发重新生成

CMake 的 `add_custom_command` 只在 `DEPENDS` 列出的文件发生变化时重新运行。如果添加了新的头文件但没有修改输入文件，需要手动触发或使用 `--force-rebuild`。

---

## 相关文档

- [架构概览](./Architecture.md) — 理解 Parser 在整体架构中的位置
- [构建系统](./BuildSystem.md) — 完整的 CMake 构建配置
- [高级主题](./Advanced-Topics.md) — 自定义元属性和高级注解用法
