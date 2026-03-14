# Runtime API 参考

## 概述

Runtime 库位于 `Source/Runtime/`，提供了 C++ 反射系统的核心运行时接口。所有类型均位于 `ursine::meta` 命名空间下。

```cpp
using namespace ursine;
using namespace meta;
```

---

## Type 类

`Type` 是整个反射系统的入口，代表一个 C++ 类型的反射句柄。它是轻量级对象，可以按值传递。

### 获取 Type 对象

```cpp
// 编译时获取（推荐）
Type t = typeof( MyClass );

// 运行时通过字符串名称获取
Type t = Type::GetFromName("MyClass");

// 通过对象实例推导
MyClass obj;
Type t = Type::Get(obj);

// 获取所有已注册类型
Type::List allTypes = Type::GetTypes();
```

### 类型判断方法

| 方法 | 返回值 | 说明 |
|------|--------|------|
| `IsValid()` | `bool` | 类型是否有效（非 INVALID） |
| `IsPrimitive()` | `bool` | 是否为基本类型（int, bool, char 等） |
| `IsFloatingPoint()` | `bool` | 是否为浮点类型（float, double 等） |
| `IsSigned()` | `bool` | 是否为有符号类型 |
| `IsEnum()` | `bool` | 是否为枚举类型 |
| `IsPointer()` | `bool` | 是否为指针类型 |
| `IsClass()` | `bool` | 是否为类或结构体 |
| `IsArray()` | `bool` | 是否为数组类型 |

### 类型信息方法

```cpp
Type t = typeof( MyClass );

// 获取类型名称
std::string name = t.GetName();  // "MyClass"

// 获取类型ID
TypeID id = t.GetID();

// 获取去除修饰符后的类型（const int* -> int）
Type decayed = t.GetDecayedType();

// 获取数组元素类型（Array<double> -> double）
Type elemType = t.GetArrayType();

// 获取枚举对象（仅枚举类型有效）
const Enum &e = t.GetEnum();

// 获取元数据管理器
const MetaManager &meta = t.GetMeta();
```

### 继承关系

```cpp
Type base = typeof( BaseClass );
Type derived = typeof( DerivedClass );

// 判断是否派生自某类型
bool derives = derived.DerivesFrom(base);
bool derives2 = derived.DerivesFrom<BaseClass>();

// 获取所有基类
const Type::Set &bases = derived.GetBaseClasses();

// 获取所有派生类
const Type::Set &deriveds = base.GetDerivedClasses();
```

### 构造与析构

```cpp
Type t = typeof( MyClass );

// 获取默认构造函数
const Constructor &ctor = t.GetConstructor();

// 获取指定签名的构造函数
const Constructor &ctor2 = t.GetConstructor({ typeof(int), typeof(float) });

// 获取动态构造函数（返回堆上指针）
const Constructor &dynCtor = t.GetDynamicConstructor();

// 获取数组构造函数
const Constructor &arrCtor = t.GetArrayConstructor();

// 获取析构函数
const Destructor &dtor = t.GetDestructor();

// 销毁实例
Variant instance = /* ... */;
t.Destroy(instance);
```

### 方法访问

```cpp
Type t = typeof( MyClass );

// 获取所有方法
std::vector<Method> methods = t.GetMethods();

// 获取指定名称的方法（第一个重载）
const Method &m = t.GetMethod("MethodName");

// 获取指定签名的方法重载
const Method &m2 = t.GetMethod("MethodName", { typeof(int), typeof(float) });

// 获取所有静态方法
std::vector<Function> staticMethods = t.GetStaticMethods();

// 获取静态方法
const Function &sf = t.GetStaticMethod("StaticMethod");
```

### 字段访问

```cpp
Type t = typeof( MyClass );

// 获取所有字段
const std::vector<Field> &fields = t.GetFields();

// 获取指定字段
const Field &f = t.GetField("fieldName");

// 获取所有静态字段
std::vector<Global> staticFields = t.GetStaticFields();

// 获取指定静态字段
const Global &sf = t.GetStaticField("staticField");
```

### 全局函数与变量

```cpp
// 获取所有全局变量
std::vector<Global> globals = Type::GetGlobals();

// 获取指定全局变量
const Global &g = Type::GetGlobal("globalVarName");

// 获取所有全局函数
std::vector<Function> funcs = Type::GetGlobalFunctions();

// 获取指定全局函数
const Function &f = Type::GetGlobalFunction("FunctionName");
```

### JSON 序列化

```cpp
// 序列化为 JSON
MyClass obj;
Json json = Type::SerializeJson(obj);

// 反序列化
MyClass restored = Type::DeserializeJson<MyClass>(json);

// 通过 Variant 序列化
Variant v = obj;
Json json2 = t.SerializeJson(v);

// 通过 Variant 反序列化
Variant v2 = t.DeserializeJson(json);
```

---

## Variant 类

`Variant` 是类型安全的值容器，用于在反射调用中传递任意类型的值。

### 构造

```cpp
// 从值构造（复制语义）
Variant v1 = 42;
Variant v2 = 3.14f;
Variant v3 = std::string("hello");
Variant v4 = myObject;

// 引用语义（不复制，持有引用）
Variant v5(myObject, variant_policy::NoCopy{});

// 包装 meta::Object 派生类
Variant v6(myObjectPtr, variant_policy::WrapObject{});

// 数组类型
Array<int> arr = {1, 2, 3};
Variant v7 = arr;
```

### 类型查询

```cpp
Variant v = 42;

// 获取类型
Type t = v.GetType();

// 有效性检查
bool valid = v.IsValid();

// 是否为 const
bool isConst = v.IsConst();

// 是否为数组
bool isArr = v.IsArray();
```

### 值提取

```cpp
Variant v = 42;

// 基本类型转换
int i = v.ToInt();
bool b = v.ToBool();
float f = v.ToFloat();
double d = v.ToDouble();
std::string s = v.ToString();

// 类型安全提取（类型不匹配时触发断言）
int &ref = v.GetValue<int>();

// 获取数组包装器
Variant arrVariant = myArray;
ArrayWrapper wrapper = arrVariant.GetArray();
```

### JSON 序列化

```cpp
Variant v = myObject;
Json json = v.SerializeJson();
```

---

## Enum 类

`Enum` 提供枚举类型的反射接口，支持枚举值与字符串之间的双向转换。

### 获取 Enum 对象

```cpp
// 通过 Type 获取
Type enumType = typeof( MyEnum );
const Enum &e = enumType.GetEnum();
```

### 枚举信息

```cpp
// 有效性检查
bool valid = e.IsValid();

// 获取枚举名称
std::string name = e.GetName();  // "MyEnum"

// 获取枚举的 Type
Type t = e.GetType();

// 获取父类型（如果是嵌套枚举）
Type parent = e.GetParentType();

// 获取底层整数类型
Type underlying = e.GetUnderlyingType();
```

### 枚举值操作

```cpp
// 获取所有键（字符串名称）
std::vector<std::string> keys = e.GetKeys();
// 结果: ["Red", "Green", "Blue"]

// 获取所有值（Variant 形式）
std::vector<Variant> values = e.GetValues();

// 枚举值转字符串
Variant val = Color::Red;
std::string key = e.GetKey(val);  // "Red"

// 字符串转枚举值
Variant v = e.GetValue("Green");
Color color = v.GetValue<Color>();  // Color::Green
```

---

## Constructor 类

`Constructor` 代表一个类型的构造函数。

### 使用构造函数

```cpp
Type t = typeof( MyClass );

// 获取默认构造函数
const Constructor &ctor = t.GetConstructor();

// 检查有效性
if (ctor.IsValid()) {
    // 调用构造函数（栈上对象）
    Variant instance = ctor.Invoke();

    // 带参数调用
    Variant instance2 = ctor.Invoke(arg1, arg2);
}

// 动态构造（堆上对象，需要手动销毁）
const Constructor &dynCtor = t.GetDynamicConstructor();
Variant ptr = dynCtor.Invoke();
// 使用完毕后
t.Destroy(ptr);
```

### Constructor 属性

```cpp
// 获取所属类型
Type classType = ctor.GetClassType();

// 是否为动态构造函数
bool isDynamic = ctor.IsDynamic();

// 获取参数签名
const InvokableSignature &sig = ctor.GetSignature();
```

---

## Destructor 类

`Destructor` 代表类型的析构函数，通常通过 `Type::Destroy()` 间接使用。

```cpp
Type t = typeof( MyClass );
const Destructor &dtor = t.GetDestructor();

Variant instance = /* 动态构造的实例 */;
dtor.Invoke(instance);
```

---

## Method 类

`Method` 代表类的成员方法。

### 方法信息

```cpp
const Method &m = t.GetMethod("Update");

// 有效性检查
bool valid = m.IsValid();

// 是否为 const 方法
bool isConst = m.IsConst();

// 获取所属类型
Type classType = m.GetClassType();

// 获取方法名
std::string name = m.GetName();

// 获取参数签名
const InvokableSignature &sig = m.GetSignature();
```

### 调用方法

```cpp
Variant instance = /* 对象实例 */;
const Method &m = t.GetMethod("Update");

// 无参调用
Variant result = m.Invoke(instance);

// 带参调用（可变参数模板）
Variant result2 = m.Invoke(instance, 0.016f);

// 使用 ArgumentList 调用
ArgumentList args = { 0.016f };
Variant result3 = m.Invoke(instance, args);
```

---

## Function 类

`Function` 代表静态方法或全局函数，接口与 `Method` 类似但不需要实例。

```cpp
const Function &f = t.GetStaticMethod("Create");

// 调用静态方法
Variant result = f.Invoke(arg1, arg2);
```

---

## Field 类

`Field` 代表类的成员字段。

### 字段信息

```cpp
const Field &f = t.GetField("health");

// 有效性检查
bool valid = f.IsValid();

// 是否为只读字段
bool readOnly = f.IsReadOnly();

// 获取字段类型
Type fieldType = f.GetType();

// 获取所属类型
Type classType = f.GetClassType();

// 获取字段名
const std::string &name = f.GetName();
```

### 读写字段值

```cpp
Variant instance = /* 对象实例 */;
const Field &f = t.GetField("health");

// 读取字段值（返回副本）
Variant value = f.GetValue(instance);
int health = value.ToInt();

// 读取字段引用（避免复制）
Variant ref = f.GetValueReference(instance);

// 写入字段值
f.SetValue(instance, 100);
f.SetValue(instance, Variant(100));
```

---

## Global 类

`Global` 代表全局变量或静态成员变量，接口与 `Field` 类似但不需要实例。

```cpp
const Global &g = Type::GetGlobal("globalCounter");

// 读取
Variant value = g.GetValue();

// 写入
g.SetValue(42);
```

---

## ArrayWrapper 类

`ArrayWrapper` 提供统一的数组操作接口，屏蔽了底层数组类型的差异。

### 获取 ArrayWrapper

```cpp
Array<int> arr = {1, 2, 3, 4, 5};
Variant arrVariant = arr;
ArrayWrapper wrapper = arrVariant.GetArray();
```

### 数组操作

```cpp
// 获取大小
size_t size = wrapper.Size();

// 读取元素
Variant elem = wrapper.GetValue(0);
int val = elem.ToInt();
// 或者
int val2 = wrapper.GetValue(0).GetValue<int>();

// 修改元素
wrapper.SetValue(0, 1000);

// 插入元素（在指定索引前插入）
wrapper.Insert(0, -100);

// 删除元素
wrapper.Remove(5);

// 有效性检查
bool valid = wrapper.IsValid();

// 是否为 const 数组
bool isConst = wrapper.IsConst();
```

### 完整示例

```cpp
#include "TestReflectionModule.h"
#include "TestTypes.h"

using namespace ursine;
using namespace meta;

int main(void)
{
    MetaInitialize( UsingModule( TestModule ) );

    Array<int> intArray { 1, 2, 3, 4, 5 };
    Variant intArrayVariant = intArray;
    ArrayWrapper arrayWrapper = intArrayVariant.GetArray();

    // 修改第一个元素
    arrayWrapper.SetValue(0, 1000);

    // 在开头插入元素
    arrayWrapper.Insert(0, -100);

    // 删除最后一个元素
    arrayWrapper.Remove(5);

    size_t size = arrayWrapper.Size();
    for (size_t i = 0; i < size; ++i)
        std::cout << arrayWrapper.GetValue(i).ToInt() << " ";

    return 0;
}
```

---

## MetaManager 类

`MetaManager` 管理附加在类型或成员上的元属性（Meta Properties）。

```cpp
// 获取类型的元数据管理器
const MetaManager &typeMeta = t.GetMeta();

// 获取字段的元数据管理器
const MetaManager &fieldMeta = f.GetMeta();

// 检查是否有某个属性
bool hasRange = fieldMeta.GetProperty(typeof(Range)).IsValid();

// 获取属性值
Range &range = fieldMeta.GetProperty(typeof(Range)).GetValue<Range>();
```

---

## Argument 类

`Argument` 是方法调用时的参数包装器，通常由 `Variant` 隐式转换而来，无需直接使用。

```cpp
// 通常不需要直接构造 Argument
// 在调用 Invoke 时，参数会自动转换为 Argument
method.Invoke(instance, 42, 3.14f, std::string("hello"));
```

---

## 初始化宏

### MetaInitialize

```cpp
// 初始化反射系统并加载指定模块
MetaInitialize( UsingModule( ModuleName ) );

// 加载多个模块
MetaInitialize(
    UsingModule( Module1 ),
    UsingModule( Module2 )
);
```

### typeof

```cpp
// 编译时获取类型的 Type 对象
Type t = typeof( MyClass );
Type t2 = typeof( int );
Type t3 = typeof( std::string );
```

---

## 相关文档

- [架构概览](./Architecture.md) — 理解类型系统设计
- [快速入门](./GettingStarted.md) — 基础用法示例
- [示例详解](./Examples.md) — 完整示例程序讲解
- [高级主题](./Advanced-Topics.md) — 序列化、元属性等高级用法
