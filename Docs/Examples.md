# 示例程序详解

## 概述

`Examples/` 目录包含五个示例程序，每个程序演示反射库的一个核心功能领域。所有示例都依赖 `TestTypes.h` 中定义的测试类型。

---

## 测试类型定义

所有示例共享以下测试类型（`Examples/TestTypes.h`）：

```cpp
#pragma once

#include <Meta.h>
#include <string>
#include <Array.h>
#include "TestProperties.h"

// 测试枚举
enum TestEnum
{
    One,
    Two,
    Three,
    Four,
    Five,
    Eighty = 80
} Meta(Enable);

// 音效结构体（带元属性）
struct SoundEffect
{
    // volume 字段附加了 Range 和 Slider 元属性
    float Meta(Enable, Range(0.0f, 100.0f), Slider(SliderType::Horizontal)) volume;

    void Load(const std::string &filename)
    {
        std::cout << "Loaded sound effect \"" << filename << "\"." << std::endl;
    }
} Meta(Enable);

// 复杂类型（包含多种字段类型）
struct ComplexType
{
    std::string stringValue;
    int intValue;
    float floatValue;
    double doubleValue;

    SoundEffect soundEffect;
    ursine::Array<int> arrayValue;
    TestEnum enumValue;

    ComplexType(void) = default;
} Meta(Enable);
```

---

## 示例一：数组反射（Arrays.cpp）

**源文件：** `Examples/Arrays.cpp`

### 功能演示

演示如何通过 `ArrayWrapper` 统一操作反射数组，包括读取、修改、插入和删除元素。

### 完整代码

```cpp
#include "TestReflectionModule.h"
#include "TestTypes.h"
#include "TestProperties.h"

using namespace ursine;
using namespace meta;

int main(void)
{
    MetaInitialize( UsingModule( TestModule ) );

    // 创建一个整数数组
    Array<int> intArray { 1, 2, 3, 4, 5 };

    // 将数组包装为 Variant
    Variant intArrayVariant = intArray;

    // 获取数组包装器
    ArrayWrapper arrayWrapper = intArrayVariant.GetArray( );

    // 修改第一个元素（索引 0）的值为 1000
    arrayWrapper.SetValue( 0, 1000 );

    // 在索引 0 处插入 -100（原有元素后移）
    arrayWrapper.Insert( 0, -100 );

    // 删除索引 5 处的元素
    arrayWrapper.Remove( 5 );

    size_t size = arrayWrapper.Size( );

    std::cout << "values: ";
    for (size_t i = 0; i < size; ++i)
    {
        // GetValue() 返回 Variant，ToInt() 转换为整数
        std::cout << arrayWrapper.GetValue( i ).ToInt( ) << " ";
    }

    return 0;
}
```

### 执行流程分析

1. `MetaInitialize` — 加载 TestModule 的反射数据
2. 创建 `Array<int>` 并初始化为 `{1, 2, 3, 4, 5}`
3. 转换为 `Variant`，再通过 `GetArray()` 获取 `ArrayWrapper`
4. `SetValue(0, 1000)` — 数组变为 `{1000, 2, 3, 4, 5}`
5. `Insert(0, -100)` — 数组变为 `{-100, 1000, 2, 3, 4, 5}`
6. `Remove(5)` — 删除索引5的元素（值为5），数组变为 `{-100, 1000, 2, 3, 4}`
7. 遍历输出所有元素

### 预期输出

```
values: -100 1000 2 3 4
```

### 关键 API

| API | 说明 |
|-----|------|
| `Variant::GetArray()` | 从 Variant 获取 ArrayWrapper |
| `ArrayWrapper::SetValue(index, value)` | 修改指定索引的元素 |
| `ArrayWrapper::Insert(index, value)` | 在指定索引前插入元素 |
| `ArrayWrapper::Remove(index)` | 删除指定索引的元素 |
| `ArrayWrapper::Size()` | 获取数组大小 |
| `ArrayWrapper::GetValue(index)` | 获取指定索引的元素（返回 Variant） |

---

## 示例二：枚举反射（Enums.cpp）

**源文件：** `Examples/Enums.cpp`

### 功能演示

演示枚举类型的反射基础用法。本示例较为简洁，主要展示反射系统的初始化。

### 完整代码

```cpp
#include "TestReflectionModule.h"
#include "TestTypes.h"
#include "TestProperties.h"

using namespace ursine;
using namespace meta;

int main(void)
{
    MetaInitialize( UsingModule( TestModule ) );

    return 0;
}
```

### 扩展用法

虽然示例代码简洁，但枚举反射的完整用法如下：

```cpp
MetaInitialize( UsingModule( TestModule ) );

// 获取枚举类型
Type enumType = typeof( TestEnum );
const Enum &testEnum = enumType.GetEnum();

// 获取所有枚举键
std::vector<std::string> keys = testEnum.GetKeys();
for (const auto &key : keys)
    std::cout << key << std::endl;
// 输出: One, Two, Three, Four, Five, Eighty

// 枚举值转字符串
Variant val = TestEnum::Eighty;
std::string key = testEnum.GetKey(val);
std::cout << key << std::endl;  // "Eighty"

// 字符串转枚举值
Variant v = testEnum.GetValue("Three");
TestEnum e = v.GetValue<TestEnum>();  // TestEnum::Three

// 获取底层整数值
std::cout << v.ToInt() << std::endl;  // 2（Three 的索引）
```

### 关键 API

| API | 说明 |
|-----|------|
| `Type::GetEnum()` | 获取枚举的 Enum 对象 |
| `Enum::GetKeys()` | 获取所有枚举键名 |
| `Enum::GetValues()` | 获取所有枚举值（Variant 形式） |
| `Enum::GetKey(value)` | 枚举值转字符串 |
| `Enum::GetValue(key)` | 字符串转枚举值 |

---

## 示例三：方法与字段（FunctionsAndMethods.cpp）

**源文件：** `Examples/FunctionsAndMethods.cpp`

### 功能演示

演示如何通过反射创建对象实例、读写字段值、调用成员方法。

### 完整代码

```cpp
#include "TestReflectionModule.h"
#include "TestTypes.h"
#include "TestProperties.h"
#include "TypeCreator.h"

using namespace ursine::meta;

int main(void)
{
    MetaInitialize( UsingModule( TestModule ) );

    // 获取 SoundEffect 类型
    Type soundEffectType = typeof( SoundEffect );

    // 获取 volume 字段
    Field volumeField = soundEffectType.GetField( "volume" );

    // 获取 Load 方法（返回第一个重载）
    Method loadMethod = soundEffectType.GetMethod( "Load" );

    // 通过反射创建 SoundEffect 实例
    Variant effect = TypeCreator::Create( soundEffectType );

    // 设置 volume 字段为 85.0f
    volumeField.SetValue( effect, 85.0f );

    // 读取 volume 字段值
    float volumeValue = volumeField.GetValue( effect ).ToFloat( );
    std::cout << "SoundEffect.volume: " << volumeValue << std::endl;

    // 调用 Load 方法
    loadMethod.Invoke( effect, std::string { "Explosion.wav" } );

    return 0;
}
```

### 执行流程分析

1. 通过 `typeof(SoundEffect)` 获取类型句柄
2. 通过 `GetField("volume")` 获取字段反射对象
3. 通过 `GetMethod("Load")` 获取方法反射对象
4. `TypeCreator::Create()` 通过反射动态创建实例
5. `SetValue(effect, 85.0f)` 通过反射写入字段
6. `GetValue(effect).ToFloat()` 通过反射读取字段
7. `Invoke(effect, "Explosion.wav")` 通过反射调用方法

### 预期输出

```
SoundEffect.volume: 85
Loaded sound effect "Explosion.wav".
```

### 关键 API

| API | 说明 |
|-----|------|
| `Type::GetField(name)` | 获取字段反射对象 |
| `Type::GetMethod(name)` | 获取方法反射对象（第一个重载） |
| `TypeCreator::Create(type)` | 通过反射创建对象实例 |
| `Field::SetValue(instance, value)` | 写入字段值 |
| `Field::GetValue(instance)` | 读取字段值（返回 Variant） |
| `Method::Invoke(instance, args...)` | 调用方法 |

---

## 示例四：元属性（MetaProperties.cpp）

**源文件：** `Examples/MetaProperties.cpp`

### 功能演示

演示如何访问附加在字段上的自定义元属性（Meta Properties），如 `Range` 和 `Slider`。

### 完整代码

```cpp
#include "TestReflectionModule.h"
#include "TestTypes.h"
#include "TestProperties.h"

using namespace ursine::meta;

int main(void)
{
    MetaInitialize( UsingModule( TestModule ) );

    // 获取 SoundEffect 类型
    Type soundEffectType = typeof( SoundEffect );

    // 获取 volume 字段
    Field volumeField = soundEffectType.GetField( "volume" );

    // 获取字段的元数据管理器
    const MetaManager &volumeMeta = volumeField.GetMeta( );

    // 获取 Range 属性并转换为具体类型
    Range &volumeRange = volumeMeta.GetProperty( typeof( Range ) ).GetValue<Range>( );

    std::cout << "SoundEffect::volume [Range.min]: " << volumeRange.min << std::endl;
    std::cout << "SoundEffect::volume [Range.max]: " << volumeRange.max << std::endl;

    // 获取 Slider 属性
    Slider &volumeSlider = volumeMeta.GetProperty( typeof( Slider ) ).GetValue<Slider>( );

    // 获取 SliderType 枚举
    Type sliderTypeEnumType = typeof( SliderType );
    const Enum &sliderTypeEnum = sliderTypeEnumType.GetEnum( );

    // 将枚举值转换为字符串
    std::cout << "SoundEffect::volume [Slider.type]: "
              << sliderTypeEnum.GetKey( volumeSlider.type )
              << std::endl;

    return 0;
}
```

### 执行流程分析

1. 获取 `SoundEffect` 类型和 `volume` 字段
2. 通过 `GetMeta()` 获取字段的元数据管理器
3. `GetProperty(typeof(Range))` 按类型查找元属性，返回 `Variant`
4. `GetValue<Range>()` 将 Variant 转换为具体的 `Range` 类型
5. 同样方式获取 `Slider` 属性
6. 通过枚举反射将 `SliderType` 枚举值转换为字符串

### 预期输出

```
SoundEffect::volume [Range.min]: 0
SoundEffect::volume [Range.max]: 100
SoundEffect::volume [Slider.type]: Horizontal
```

### 元属性定义（TestProperties.h）

```cpp
// Range 元属性：定义数值范围
struct Range {
    float min;
    float max;
    Range(float min, float max) : min(min), max(max) {}
} Meta(Enable);

// SliderType 枚举
enum class SliderType {
    Horizontal,
    Vertical
} Meta(Enable);

// Slider 元属性：定义滑块类型
struct Slider {
    SliderType type;
    Slider(SliderType type) : type(type) {}
} Meta(Enable);
```

### 关键 API

| API | 说明 |
|-----|------|
| `Field::GetMeta()` | 获取字段的元数据管理器 |
| `MetaManager::GetProperty(type)` | 按类型获取元属性（返回 Variant） |
| `Variant::GetValue<T>()` | 将 Variant 转换为具体类型 |

---

## 示例五：序列化（Serialization.cpp）

**源文件：** `Examples/Serialization.cpp`

### 功能演示

演示如何使用反射系统实现 JSON 序列化和反序列化，支持嵌套对象、数组和枚举。

### 完整代码

```cpp
#include "TestReflectionModule.h"
#include "TestTypes.h"
#include "TestProperties.h"

using namespace ursine;
using namespace meta;

int main(void)
{
    MetaInitialize( UsingModule( TestModule ) );

    ///////////////////////////////////////////////////////////////////////////
    // 序列化
    ///////////////////////////////////////////////////////////////////////////

    ComplexType complexType;
    complexType.stringValue = "Testing";
    complexType.intValue = 23;
    complexType.floatValue = 77.0f;
    complexType.doubleValue = 86.35f;
    complexType.soundEffect.volume = 50.0f;
    complexType.arrayValue = { 1, 2, 3, 4, 5 };
    complexType.enumValue = Eighty;

    // 将对象序列化为 JSON
    std::cout << "Serialized: "
              << Variant( complexType ).SerializeJson( ).dump( )
              << std::endl;

    ///////////////////////////////////////////////////////////////////////////
    // 反序列化
    ///////////////////////////////////////////////////////////////////////////

    std::string complexJson = R"(
        {
            "arrayValue": [ 5, 6, 7, 8 ],
            "doubleValue": 100.0,
            "enumValue": "Two",
            "floatValue": 98.5,
            "intValue": -25,
            "soundEffect": { "volume": 100.0 },
            "stringValue": "Deserialization!"
        }
    )";

    std::string jsonError;

    // 从 JSON 反序列化为 ComplexType 对象
    ComplexType deserializedComplexType = Type::DeserializeJson<ComplexType>(
        Json::parse( complexJson, jsonError )
    );

    std::cout << "Deserialized: "
              << Variant( deserializedComplexType ).SerializeJson( ).dump( )
              << std::endl;

    return 0;
}
```

### 执行流程分析

**序列化流程：**
1. 创建并填充 `ComplexType` 对象
2. 将对象包装为 `Variant`
3. 调用 `SerializeJson()` — 反射系统遍历所有字段，递归序列化
4. 枚举值自动转换为字符串（`Eighty` → `"Eighty"`）
5. 嵌套对象（`soundEffect`）递归序列化
6. 数组自动序列化为 JSON 数组

**反序列化流程：**
1. 解析 JSON 字符串
2. `Type::DeserializeJson<ComplexType>()` — 反射系统创建对象并填充字段
3. 枚举字符串自动转换为枚举值（`"Two"` → `TestEnum::Two`）
4. 嵌套对象递归反序列化

### 预期输出

```json
Serialized: {"arrayValue":[1,2,3,4,5],"doubleValue":86.35,"enumValue":"Eighty","floatValue":77.0,"intValue":23,"soundEffect":{"volume":50.0},"stringValue":"Testing"}

Deserialized: {"arrayValue":[5,6,7,8],"doubleValue":100.0,"enumValue":"Two","floatValue":98.5,"intValue":-25,"soundEffect":{"volume":100.0},"stringValue":"Deserialization!"}
```

### 关键 API

| API | 说明 |
|-----|------|
| `Variant::SerializeJson()` | 将 Variant 中的对象序列化为 JSON |
| `Type::SerializeJson(instance)` | 静态方法，序列化指定类型的实例 |
| `Type::DeserializeJson<T>(json)` | 静态模板方法，从 JSON 反序列化 |
| `Type::DeserializeJson(json)` | 从 JSON 反序列化，返回 Variant |

---

## 示例对比总结

| 示例 | 核心概念 | 难度 |
|------|----------|------|
| Arrays | ArrayWrapper 数组操作 | ⭐⭐ |
| Enums | 枚举反射与转换 | ⭐ |
| FunctionsAndMethods | 字段读写、方法调用 | ⭐⭐ |
| MetaProperties | 元属性系统 | ⭐⭐⭐ |
| Serialization | JSON 序列化/反序列化 | ⭐⭐⭐ |

---

## 相关文档

- [快速入门](./GettingStarted.md) — 基础用法入门
- [Runtime API 参考](./Runtime-API.md) — 完整 API 文档
- [高级主题](./Advanced-Topics.md) — 自定义序列化和元属性
