# 高级主题

## 概述

本文档涵盖 CPP-Reflection 库的高级用法，包括自定义类型注册、序列化定制、元属性系统深度使用、性能优化以及与游戏引擎集成的最佳实践。

---

## 自定义类型注册

### 手动注册类型

除了使用 Parser 自动生成注册代码外，也可以手动注册类型。这在以下场景中有用：
- 第三方库的类型（无法修改源码添加注解）
- 动态生成的类型
- 需要精细控制注册过程

```cpp
#include <TypeDatabase.h>

// 假设有一个第三方类型
struct ThirdPartyVector3 {
    float x, y, z;
};

// 手动注册
void RegisterThirdPartyTypes()
{
    auto &type = TypeDatabase::Register<ThirdPartyVector3>("ThirdPartyVector3");

    // 注册默认构造函数
    type.AddConstructor<ThirdPartyVector3>();

    // 注册字段
    type.AddField("x", &ThirdPartyVector3::x);
    type.AddField("y", &ThirdPartyVector3::y);
    type.AddField("z", &ThirdPartyVector3::z);
}
```

### 注册模板类型

对于模板类型，需要为每个具体实例化分别注册：

```cpp
// 注册 Array<int> 类型
TypeDatabase::Register<ursine::Array<int>>("Array<int>");

// 注册 Array<float> 类型
TypeDatabase::Register<ursine::Array<float>>("Array<float>");
```

### 注册继承关系

```cpp
struct Base { virtual ~Base() = default; };
struct Derived : Base {};

void RegisterTypes()
{
    auto &baseType = TypeDatabase::Register<Base>("Base");
    auto &derivedType = TypeDatabase::Register<Derived>("Derived");

    // 注册继承关系
    derivedType.AddBaseClass<Base>();
}
```

---

## 自定义序列化

### 序列化钩子

反射系统支持在序列化前后执行自定义逻辑。通过在类型上添加特定的元属性来启用：

```cpp
// 定义序列化钩子元属性
struct SerializeHook {
    using Callback = std::function<void(Variant &)>;
    Callback onSerialize;
    Callback onDeserialize;
};
```

### 字段序列化覆盖

可以为特定字段提供自定义的序列化 getter：

```cpp
// 使用自定义 getter 序列化
Type::SerializationGetterOverride customGetter =
    [](const Variant &instance, const Field &field) -> Variant {
        // 自定义序列化逻辑
        if (field.GetName() == "password") {
            // 对密码字段进行加密
            std::string pwd = field.GetValue(instance).GetValue<std::string>();
            return Variant(encrypt(pwd));
        }
        return field.GetValue(instance);
    };

Type t = typeof( UserData );
Json json = t.SerializeJson(instance, customGetter);
```

### 跳过字段序列化

通过元属性标记不需要序列化的字段：

```cpp
// 定义 NonSerializable 元属性
struct NonSerializable {};

struct PlayerData {
    std::string Meta(Enable) name;
    int Meta(Enable) score;
    // 临时缓存字段，不需要序列化
    float Meta(Enable, NonSerializable) cachedValue;
};
```

在自定义序列化逻辑中检查此属性：

```cpp
Type::SerializationGetterOverride skipNonSerializable =
    [](const Variant &instance, const Field &field) -> Variant {
        // 如果字段有 NonSerializable 属性，返回无效 Variant
        if (field.GetMeta().GetProperty(typeof(NonSerializable)).IsValid())
            return Variant();  // 无效 Variant 会被跳过
        return field.GetValue(instance);
    };
```

### 自定义 JSON 格式

对于需要特殊 JSON 格式的类型，可以完全自定义序列化逻辑：

```cpp
// 将 Vector3 序列化为数组格式 [x, y, z] 而非对象格式
struct Vector3 {
    float x, y, z;
};

// 注册时添加自定义序列化器
// （需要修改生成的注册代码或手动注册）
```

---

## 元属性系统深度使用

### 定义复杂元属性

元属性可以是任意 C++ 类型，包括包含函数的类型：

```cpp
// 验证器元属性
struct Validator {
    using ValidateFn = std::function<bool(const Variant &)>;
    ValidateFn validate;

    Validator(ValidateFn fn) : validate(fn) {}
};

// 使用示例（在注解中）
struct PlayerStats {
    int Meta(Enable, Validator([](const Variant &v) {
        return v.ToInt() >= 0 && v.ToInt() <= 100;
    })) health;
};
```

### 运行时查询元属性

```cpp
Type t = typeof( PlayerStats );
Field healthField = t.GetField("health");
const MetaManager &meta = healthField.GetMeta();

// 检查是否有 Validator 属性
Variant validatorVariant = meta.GetProperty(typeof(Validator));
if (validatorVariant.IsValid()) {
    Validator &validator = validatorVariant.GetValue<Validator>();

    // 验证值
    Variant testValue = 150;
    bool isValid = validator.validate(testValue);
    std::cout << "150 is valid: " << isValid << std::endl;  // false
}
```

### 元属性继承

元属性不会自动继承，但可以通过遍历基类来实现类似效果：

```cpp
// 查找类型及其所有基类上的某个元属性
Variant FindPropertyInHierarchy(const Type &type, const Type &propType)
{
    // 先查当前类型
    Variant prop = type.GetMeta().GetProperty(propType);
    if (prop.IsValid())
        return prop;

    // 再查基类
    for (const Type &base : type.GetBaseClasses()) {
        prop = FindPropertyInHierarchy(base, propType);
        if (prop.IsValid())
            return prop;
    }

    return Variant();  // 未找到
}
```

### 编辑器集成示例

元属性系统非常适合游戏编辑器集成，可以为字段附加 UI 提示信息：

```cpp
// 编辑器相关元属性
struct DisplayName {
    std::string name;
    DisplayName(const std::string &n) : name(n) {}
};

struct Tooltip {
    std::string text;
    Tooltip(const std::string &t) : text(t) {}
};

struct Category {
    std::string name;
    Category(const std::string &n) : name(n) {}
};

// 使用
struct PhysicsBody {
    float Meta(Enable,
        DisplayName("质量"),
        Tooltip("物体的质量，单位：千克"),
        Category("物理"),
        Range(0.001f, 10000.0f)
    ) mass;

    float Meta(Enable,
        DisplayName("摩擦系数"),
        Category("物理"),
        Range(0.0f, 1.0f),
        Slider(SliderType::Horizontal)
    ) friction;
};
```

---

## 动态类型操作

### 通过字符串名称操作类型

在脚本系统或配置驱动的场景中，经常需要通过字符串名称操作类型：

```cpp
// 通过字符串创建对象
std::string typeName = "SoundEffect";
Type t = Type::GetFromName(typeName);

if (t.IsValid()) {
    Variant instance = t.GetConstructor().Invoke();

    // 通过字符串设置字段
    t.GetField("volume").SetValue(instance, 75.0f);

    // 通过字符串调用方法
    t.GetMethod("Load").Invoke(instance, std::string("bgm.wav"));
}
```

### 遍历所有类型

```cpp
// 获取所有已注册类型
Type::List allTypes = Type::GetTypes();

for (const Type &type : allTypes) {
    if (!type.IsClass()) continue;

    std::cout << "类型: " << type.GetName() << std::endl;

    // 遍历所有字段
    for (const Field &field : type.GetFields()) {
        std::cout << "  字段: " << field.GetName()
                  << " (" << field.GetType().GetName() << ")" << std::endl;
    }

    // 遍历所有方法
    for (const Method &method : type.GetMethods()) {
        std::cout << "  方法: " << method.GetName() << std::endl;
    }
}
```

### 深度复制对象

利用反射实现通用的深度复制：

```cpp
Variant DeepCopy(const Variant &source)
{
    Type t = source.GetType();

    if (!t.IsClass())
        return source;  // 基本类型直接复制

    // 创建新实例
    Variant copy = t.GetConstructor().Invoke();

    // 复制所有字段
    for (const Field &field : t.GetFields()) {
        Variant fieldValue = field.GetValue(source);
        field.SetValue(copy, DeepCopy(fieldValue));
    }

    return copy;
}
```

---

## 性能优化

### 缓存 Type 和 Field 对象

`Type::GetFromName()` 和 `Type::GetField()` 涉及字符串查找，在热路径中应缓存结果：

```cpp
// 不推荐：每帧都进行字符串查找
void Update()
{
    Type t = Type::GetFromName("Player");  // 字符串查找
    Field f = t.GetField("health");        // 字符串查找
    f.SetValue(player, newHealth);
}

// 推荐：初始化时缓存
class PlayerSystem {
    Type m_playerType;
    Field m_healthField;
    Method m_updateMethod;

public:
    void Initialize() {
        m_playerType = typeof(Player);
        m_healthField = m_playerType.GetField("health");
        m_updateMethod = m_playerType.GetMethod("Update");
    }

    void Update(Variant &player, float dt) {
        m_updateMethod.Invoke(player, dt);  // 直接调用，无字符串查找
    }
};
```

### 使用引用语义避免复制

`Variant` 默认使用值语义（复制数据）。对于大型对象，使用 `NoCopy` 策略避免不必要的复制：

```cpp
LargeObject obj;

// 不推荐：复制整个对象
Variant v1 = obj;

// 推荐：持有引用，不复制
Variant v2(obj, variant_policy::NoCopy{});

// 注意：使用引用语义时，需确保原对象的生命周期长于 Variant
```

### 批量字段操作

对于需要序列化大量对象的场景，预先获取字段列表可以提高效率：

```cpp
// 预先获取字段列表
Type t = typeof(MyClass);
const std::vector<Field> &fields = t.GetFields();

// 批量处理多个对象
for (auto &obj : objects) {
    Variant v(obj, variant_policy::NoCopy{});
    for (const Field &field : fields) {
        // 处理字段...
    }
}
```

---

## 与游戏引擎集成

### 组件系统

反射系统非常适合实现 ECS（Entity-Component-System）中的组件序列化：

```cpp
// 组件基类
class Component {
public:
    virtual ~Component() = default;
    virtual Type GetType() const = 0;
};

// 通过反射序列化任意组件
Json SerializeComponent(const Component &comp)
{
    Type t = comp.GetType();
    Variant v = &comp;
    return t.SerializeJson(v);
}

// 通过反射反序列化组件
std::unique_ptr<Component> DeserializeComponent(
    const std::string &typeName, const Json &json)
{
    Type t = Type::GetFromName(typeName);
    if (!t.IsValid()) return nullptr;

    Variant instance = t.GetDynamicConstructor().Invoke();
    t.DeserializeJson(instance, json);

    return std::unique_ptr<Component>(
        instance.GetValue<Component*>()
    );
}
```

### 脚本绑定

反射系统可以作为 C++ 与脚本语言（如 Lua）之间的桥梁：

```cpp
// 将反射类型暴露给 Lua
void ExposTypeToLua(lua_State *L, const Type &type)
{
    // 创建 Lua 表
    lua_newtable(L);

    // 为每个方法创建 Lua 函数
    for (const Method &method : type.GetMethods()) {
        std::string name = method.GetName();

        // 捕获方法引用，创建 Lua 闭包
        lua_pushstring(L, name.c_str());
        lua_pushlightuserdata(L, (void*)&method);
        lua_pushcclosure(L, [](lua_State *L) -> int {
            Method *m = (Method*)lua_touserdata(L, lua_upvalueindex(1));
            // 从 Lua 栈读取参数，调用方法，将结果压栈
            // ...
            return 1;
        }, 1);
        lua_settable(L, -3);
    }
}
```

### 属性面板（Inspector）

在游戏编辑器中，利用反射自动生成属性面板：

```cpp
void DrawInspector(Variant &object)
{
    Type t = object.GetType();

    for (const Field &field : t.GetFields()) {
        const MetaManager &meta = field.GetMeta();
        Type fieldType = field.GetType();

        // 获取显示名称（如果有）
        std::string displayName = field.GetName();
        Variant displayNameProp = meta.GetProperty(typeof(DisplayName));
        if (displayNameProp.IsValid())
            displayName = displayNameProp.GetValue<DisplayName>().name;

        // 根据字段类型绘制不同的 UI 控件
        if (fieldType == typeof(float)) {
            float value = field.GetValue(object).ToFloat();

            // 检查是否有 Range 属性
            Variant rangeProp = meta.GetProperty(typeof(Range));
            if (rangeProp.IsValid()) {
                Range &range = rangeProp.GetValue<Range>();
                // 绘制滑块
                if (DrawSlider(displayName, value, range.min, range.max))
                    field.SetValue(object, value);
            } else {
                // 绘制普通输入框
                if (DrawFloat(displayName, value))
                    field.SetValue(object, value);
            }
        }
        else if (fieldType == typeof(int)) {
            int value = field.GetValue(object).ToInt();
            if (DrawInt(displayName, value))
                field.SetValue(object, value);
        }
        else if (fieldType == typeof(std::string)) {
            std::string value = field.GetValue(object).ToString();
            if (DrawString(displayName, value))
                field.SetValue(object, value);
        }
        else if (fieldType.IsEnum()) {
            // 绘制枚举下拉框
            const Enum &e = fieldType.GetEnum();
            std::vector<std::string> keys = e.GetKeys();
            Variant currentVal = field.GetValue(object);
            std::string currentKey = e.GetKey(currentVal);

            if (DrawCombo(displayName, currentKey, keys)) {
                field.SetValue(object, e.GetValue(currentKey));
            }
        }
        else if (fieldType.IsClass()) {
            // 递归绘制嵌套对象
            Variant nested = field.GetValueReference(object);
            DrawInspector(nested);
        }
    }
}
```

---

## 多模块管理

### 按需加载模块

对于大型项目，可以将反射数据分割为多个模块，按需加载：

```cpp
// 游戏启动时只加载核心模块
MetaInitialize( UsingModule( CoreModule ) );

// 进入编辑器时加载编辑器模块
if (isEditorMode) {
    MetaInitialize( UsingModule( EditorModule ) );
}

// 加载特定关卡的模块
MetaInitialize( UsingModule( Level1Module ) );
```

### 模块依赖管理

```cpp
// 确保依赖模块先加载
void InitializeAllModules()
{
    // 按依赖顺序加载
    MetaInitialize( UsingModule( MathModule ) );      // 无依赖
    MetaInitialize( UsingModule( CoreModule ) );      // 依赖 Math
    MetaInitialize( UsingModule( GameplayModule ) );  // 依赖 Core
    MetaInitialize( UsingModule( UIModule ) );        // 依赖 Core
}
```

---

## 调试技巧

### 打印类型信息

```cpp
void PrintTypeInfo(const Type &type)
{
    std::cout << "=== 类型: " << type.GetName() << " ===" << std::endl;

    std::cout << "基类: ";
    for (const Type &base : type.GetBaseClasses())
        std::cout << base.GetName() << " ";
    std::cout << std::endl;

    std::cout << "字段:" << std::endl;
    for (const Field &f : type.GetFields()) {
        std::cout << "  " << f.GetType().GetName()
                  << " " << f.GetName();
        if (f.IsReadOnly()) std::cout << " [只读]";
        std::cout << std::endl;
    }

    std::cout << "方法:" << std::endl;
    for (const Method &m : type.GetMethods()) {
        std::cout << "  " << m.GetName();
        if (m.IsConst()) std::cout << " [const]";
        std::cout << std::endl;
    }
}
```

### 验证反射数据完整性

```cpp
void ValidateReflectionData()
{
    Type::List allTypes = Type::GetTypes();

    for (const Type &type : allTypes) {
        if (!type.IsClass()) continue;

        // 检查是否有默认构造函数
        if (!type.GetConstructor().IsValid()) {
            std::cerr << "警告: " << type.GetName()
                      << " 没有默认构造函数" << std::endl;
        }

        // 检查字段类型是否都已注册
        for (const Field &field : type.GetFields()) {
            Type fieldType = field.GetType().GetDecayedType();
            if (fieldType.IsClass() && !fieldType.IsValid()) {
                std::cerr << "警告: " << type.GetName()
                          << "::" << field.GetName()
                          << " 的类型未注册" << std::endl;
            }
        }
    }
}
```

---

## 相关文档

- [架构概览](./Architecture.md) — 理解系统整体设计
- [Runtime API 参考](./Runtime-API.md) — 完整 API 文档
- [Parser 使用指南](./Parser-Guide.md) — 元数据生成配置
- [示例详解](./Examples.md) — 基础用法示例
