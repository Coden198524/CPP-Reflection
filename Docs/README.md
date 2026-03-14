# C++ 反射库 - 技术文档

[![加入聊天](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/CPP-Reflection/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## 简介

本文档是 **C++ 反射库**（CPP-Reflection）的完整技术参考手册。该库提供了一套完整的 C++ 运行时类型反射系统，允许在运行时对类型、方法、字段、枚举等进行内省和操作。

C++ 本身不支持运行时反射，本库通过以下两个核心组件解决了这一问题：

- **Runtime（运行时库）**：提供反射数据的查询和操作接口
- **Parser（解析器）**：基于 LibClang 分析 C++ 源代码，自动生成反射元数据

该库由 Austin Brunkhorst 开发，最初用于游戏项目和编辑器开发。更多背景信息请参阅作者博客：

> https://austinbrunkhorst.com/cpp-reflection-part-1/

---

## 文档目录

| 文档 | 说明 |
|------|------|
| [架构概览](./Architecture.md) | 系统整体架构、设计哲学、组件关系 |
| [快速入门](./GettingStarted.md) | 环境搭建、第一个反射示例、基础用法 |
| [Runtime API 参考](./Runtime-API.md) | 运行时库完整 API 文档（Type、Enum、Class、Method、Field、Variant 等） |
| [Parser 使用指南](./Parser-Guide.md) | 解析器工作原理、元数据生成、与构建系统集成 |
| [构建系统](./BuildSystem.md) | CMake 构建配置、依赖安装、各平台构建说明 |
| [示例详解](./Examples.md) | 五个示例程序的详细讲解（数组、枚举、方法、属性、序列化） |
| [高级主题](./Advanced-Topics.md) | 自定义类型注册、序列化、属性系统、性能优化 |

---

## 项目结构

```
CPP-Reflection/
├── Source/
│   ├── Runtime/          # 反射运行时库（核心）
│   │   ├── Type.h        # 类型基类
│   │   ├── Variant.h     # 类型安全的值容器
│   │   ├── Enum.h        # 枚举反射
│   │   ├── Constructor.h # 构造函数反射
│   │   ├── Method.h      # 方法反射
│   │   ├── Field.h       # 字段反射
│   │   ├── Array.h       # 数组反射
│   │   └── ...           # 其他运行时组件
│   ├── Parser/           # 元数据解析器
│   │   ├── Main.cpp      # 解析器入口
│   │   └── ...           # 解析器组件
│   ├── Common/           # 公共工具
│   └── CMake/            # CMake 模块
├── Examples/             # 使用示例
│   ├── Arrays.cpp        # 数组反射示例
│   ├── Enums.cpp         # 枚举反射示例
│   ├── FunctionsAndMethods.cpp  # 方法调用示例
│   ├── MetaProperties.cpp       # 元属性示例
│   └── Serialization.cpp        # 序列化示例
├── Resources/            # 附加资源
└── Docs/                 # 本文档目录
```

---

## 核心概念速览

### 类型系统

反射库的核心是类型系统。每个被反射的 C++ 类型都对应一个 `Type` 对象，通过它可以访问该类型的所有元数据：

```cpp
#include "TestReflectionModule.h"
using namespace ursine;
using namespace meta;

// 初始化反射系统
MetaInitialize( UsingModule( TestModule ) );

// 获取类型信息
Type myType = typeof( MyClass );
std::cout << myType.GetName() << std::endl;
```

### Variant（变体类型）

`Variant` 是库中用于类型安全地存储和传递任意类型值的容器，类似于 `std::any`，但与反射系统深度集成：

```cpp
Variant value = 42;
Variant strValue = std::string("hello");

// 通过反射调用方法时，参数和返回值均使用 Variant
```

### 元数据注解

通过在 C++ 代码中添加注解，Parser 可以提取并生成反射元数据：

```cpp
class Meta(Enable) MyClass
{
public:
    float Meta(Enable) myField;
    void Meta(Enable) MyMethod();
};
```

---

## 依赖要求

| 依赖 | 最低版本 | 用途 |
|------|----------|------|
| LLVM / LibClang | 3.8.0 | Parser 解析 C++ AST |
| Boost | 1.59.0 | 工具库支持 |
| C++ 编译器 | C++11 | MSVC 14 / G++ 4.8 / Clang++ 3.6 |
| CMake | 3.0+ | 构建系统 |

---

## 快速构建

### 构建运行时库

```bash
mkdir Build && cd Build
cmake -G "<生成器>" ../Source/Runtime
cmake --build . --target MetaRuntime
```

### 构建解析器

```bash
mkdir Build && cd Build
cmake -G "<生成器>" ../Source/Parser -DLLVM_ROOT=<LLVM路径> -DBOOST_ROOT=<Boost路径>
cmake --build . --target MetaParser
```

### 构建示例

```bash
mkdir Build && cd Build
cmake -G "<生成器>" ../Examples -DLLVM_ROOT=<LLVM路径> -DBOOST_ROOT=<Boost路径>
cmake --build .
```

> 详细构建说明请参阅 [构建系统文档](./BuildSystem.md)。

---

## 功能特性

- **类型反射**：在运行时查询类型名称、基类、是否为指针/引用等信息
- **枚举反射**：枚举值与字符串之间的双向转换
- **方法反射**：通过名称动态调用类的成员方法和静态方法
- **字段反射**：运行时读写对象的成员变量
- **构造函数反射**：通过反射动态创建对象实例
- **数组反射**：统一接口操作各种数组类型
- **Variant 系统**：类型安全的值存储与传递
- **元属性系统**：为类型和成员附加自定义元数据
- **序列化支持**：基于反射实现 JSON 序列化/反序列化

---

## 阅读建议

**新用户**建议按以下顺序阅读：

1. [架构概览](./Architecture.md) — 理解整体设计
2. [快速入门](./GettingStarted.md) — 动手运行第一个示例
3. [构建系统](./BuildSystem.md) — 配置开发环境
4. [示例详解](./Examples.md) — 通过示例学习用法
5. [Runtime API 参考](./Runtime-API.md) — 深入了解 API

**有经验的用户**可直接查阅：

- [Runtime API 参考](./Runtime-API.md) — 完整 API 文档
- [Parser 使用指南](./Parser-Guide.md) — 集成到自己的项目
- [高级主题](./Advanced-Topics.md) — 高级用法和最佳实践

---

## 相关资源

- [项目主页 README](../README.md)
- [作者博客系列（英文）](https://austinbrunkhorst.com/cpp-reflection-part-1/)
- [GitHub 仓库](https://github.com/AustinBrunkhorst/CPP-Reflection)
- [社区聊天室](https://gitter.im/CPP-Reflection/Lobby)

---

*本文档基于 CPP-Reflection 库源代码编写，旨在帮助开发者快速理解和使用该反射系统。*
