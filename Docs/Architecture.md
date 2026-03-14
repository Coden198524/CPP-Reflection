# 系统架构概览

## 简介

CPP-Reflection 是一个为 C++11 设计的运行时反射系统。由于 C++ 标准本身不提供运行时类型内省能力，本库通过"离线解析 + 运行时查询"的两阶段架构解决了这一问题。

---

## 整体架构

```
┌─────────────────────────────────────────────────────────┐
│                     开发阶段（编译前）                     │
│                                                         │
│   C++ 源代码                                             │
│   (含 Meta() 注解)  ──►  MetaParser  ──►  生成的元数据   │
│                          (LibClang)       (.generated.h) │
└─────────────────────────────────────────────────────────┘
                                │
                                ▼ 编译进可执行文件
┌─────────────────────────────────────────────────────────┐
│                     运行阶段                              │
│                                                         │
│   MetaInitialize()  ──►  反射数据库  ──►  Type / Enum   │
│   (注册元数据)             (内存中)        Method / Field │
│                                           Variant / Array│
└─────────────────────────────────────────────────────────┘
```

整个系统分为两个独立组件：

| 组件 | 目录 | 职责 |
|------|------|------|
| **Runtime（运行时库）** | `Source/Runtime/` | 提供反射数据的存储和查询接口 |
| **Parser（解析器）** | `Source/Parser/` | 分析 C++ 源码，生成反射元数据代码 |

---

## Runtime 架构

### 核心类层次

```
Type                          ← 所有类型的统一入口
├── TypeID (uint32_t)         ← 类型的唯一数字标识
├── TypeData (内部存储)        ← 类型的完整元数据
│   ├── constructors[]
│   ├── destructor
│   ├── methods[]
│   ├── fields[]
│   ├── staticMethods[]
│   ├── staticFields[]
│   ├── baseClasses (Set)
│   └── derivedClasses (Set)
│
├── Enum                      ← 枚举类型的反射
├── Constructor               ← 构造函数反射
├── Destructor                ← 析构函数反射
├── Method                    ← 成员方法反射
├── Function                  ← 静态/全局函数反射
├── Field                     ← 成员字段反射
└── Global                    ← 静态/全局变量反射

Variant                       ← 类型安全的值容器
└── VariantBase               ← 内部存储基类
    ├── VariantContainer<T>   ← 值语义存储
    └── VariantRefObject      ← 引用语义存储

ArrayWrapper                  ← 统一数组操作接口
└── ArrayWrapperBase          ← 内部基类
    └── ArrayWrapperContainer ← 具体数组类型适配
```

### 类型系统设计

每个 C++ 类型在反射数据库中对应一个唯一的 `TypeID`（32位整数）。`Type` 类是一个轻量级句柄，仅持有 `TypeID` 和 `isArray` 标志，通过 ID 查询全局数据库获取完整元数据。

```cpp
// Type 是轻量级句柄，可以按值传递
class Type {
    TypeID m_id;      // 类型唯一ID
    bool m_isArray;   // 是否为数组类型
};
```

这种设计使得 `Type` 对象可以高效地按值传递和比较，而不需要复制完整的类型数据。

### Variant 系统设计

`Variant` 是库中最核心的数据传递机制，用于在反射调用中统一表示任意类型的值：

```
Variant
├── 值语义（默认）：内部复制数据，生命周期独立
├── 引用语义（NoCopy策略）：持有引用，不复制
└── 对象包装（WrapObject策略）：包装 meta::Object 派生类
```

`Variant` 支持以下转换：
- `ToInt()` / `ToBool()` / `ToFloat()` / `ToDouble()` / `ToString()`
- `GetValue<T>()` — 类型安全地提取具体类型值
- `GetArray()` — 获取数组包装器
- `SerializeJson()` — 序列化为 JSON

### 元数据容器（MetaContainer）

`Method`、`Field`、`Constructor` 等反射对象都继承自 `MetaContainer`，允许附加任意键值对形式的元属性：

```cpp
class MetaContainer {
    MetaManager m_meta;  // 存储附加的元属性
};
```

元属性在 Parser 阶段通过 `Meta(key, value)` 注解提取，在运行时通过 `GetMeta()` 访问。

---

## Parser 架构

### 解析流程

```
命令行参数
    │
    ▼
ReflectionOptions     ← 解析命令行选项（输入文件、输出路径、模块名等）
    │
    ▼
ReflectionParser      ← 主解析器，驱动整个流程
    │
    ├── LibClang API  ← 解析 C++ AST
    │       │
    │       ▼
    │   Cursor / CursorType  ← 封装 LibClang 游标和类型
    │
    ├── LanguageTypes/  ← 各种语言构造的抽象
    │   ├── Class       ← 类/结构体
    │   ├── Enum        ← 枚举
    │   ├── Function    ← 函数
    │   └── ...
    │
    ├── MetaDataManager ← 提取和管理 Meta() 注解
    │
    └── Module/         ← 模块系统，管理生成的代码
            │
            ▼
        生成的 .generated.h 文件（包含注册代码）
```

### 关键设计决策

**为什么使用 LibClang？**

LibClang 提供了完整的 C++ AST 访问能力，能够准确处理模板、命名空间、继承等复杂语言特性，比正则表达式或手写解析器更可靠。

**注解语法**

Parser 通过识别特殊的 `Meta(...)` 宏来标记需要反射的类型和成员：

```cpp
// 标记类需要反射
class Meta(Enable) MyClass {
public:
    // 标记字段需要反射，并附加元属性
    float Meta(Enable, Serializable) health;

    // 标记方法需要反射
    void Meta(Enable) Update(float dt);
};

// 标记枚举需要反射
enum class Meta(Enable) Direction {
    North, South, East, West
};
```

**模块系统**

生成的反射代码被组织为"模块"，每个模块包含一组类型的注册代码。使用时通过 `MetaInitialize(UsingModule(ModuleName))` 加载：

```cpp
// 生成的模块文件（自动生成，不要手动编辑）
// TestReflectionModule.h
MetaInitialize( UsingModule( TestModule ) );
```

---

## 数据流

### 反射数据注册流程

```
1. 程序启动
       │
       ▼
2. MetaInitialize( UsingModule(X) )
       │
       ▼
3. 执行生成的注册代码
   - TypeDatabase::Register<MyClass>()
   - 注册构造函数、方法、字段...
       │
       ▼
4. 全局 TypeDatabase 填充完毕
       │
       ▼
5. 可以通过 typeof(MyClass) 或 Type::GetFromName("MyClass") 查询
```

### 方法调用流程

```
用户代码: method.Invoke(instance, arg1, arg2)
       │
       ▼
Method::Invoke()
       │
       ▼
MethodInvoker<ClassType, ReturnType, ArgTypes...>
       │
       ▼
将 Variant 参数解包为具体类型
       │
       ▼
调用实际的 C++ 成员函数
       │
       ▼
将返回值包装为 Variant 返回
```

---

## 命名空间结构

所有运行时类型都位于 `ursine::meta` 命名空间下：

```cpp
namespace ursine {
    namespace meta {
        class Type;
        class Variant;
        class Enum;
        class Constructor;
        class Destructor;
        class Method;
        class Function;
        class Field;
        class Global;
        class Argument;
        class ArrayWrapper;
        // ...
    }
}
```

使用时通常引入命名空间：

```cpp
using namespace ursine;
using namespace meta;
```

---

## 设计原则

### 1. 零开销抽象（Zero-overhead Abstraction）

反射数据在编译时由 Parser 生成，运行时只需查表，不需要运行时代码分析。对于不使用反射的代码路径，没有额外开销。

### 2. 类型安全

`Variant` 系统在内部记录实际类型，`GetValue<T>()` 在类型不匹配时会触发断言，避免了 `void*` 的不安全性。

### 3. 非侵入式设计

被反射的类不需要继承任何基类（除非使用 `WrapObject` 策略），只需在 Parser 阶段添加 `Meta()` 注解即可。

### 4. 模块化

反射数据按模块组织，可以按需加载，避免将所有类型数据都加载到内存中。

---

## 与其他反射库的对比

| 特性 | CPP-Reflection | RTTR | Boost.Reflect |
|------|---------------|------|---------------|
| 元数据生成 | 自动（Parser） | 手动注册 | 手动注册 |
| 依赖 | LibClang, Boost | 无 | Boost |
| 注解语法 | Meta() 宏 | 无 | 无 |
| 数组支持 | 统一 ArrayWrapper | 有 | 有限 |
| 序列化 | 内置 JSON | 无 | 无 |

---

## 相关文档

- [快速入门](./GettingStarted.md) — 从零开始使用本库
- [Runtime API 参考](./Runtime-API.md) — 完整的运行时 API 文档
- [Parser 使用指南](./Parser-Guide.md) — 如何配置和运行解析器
- [构建系统](./BuildSystem.md) — CMake 构建配置详解
