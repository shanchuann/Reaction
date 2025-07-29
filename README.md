# Reaction 响应式框架

响应式编程是一种以“数据驱动视图”和“自动更新”为核心的编程范式。其目标是当底层数据发生变化时，系统能够自动刷新所有依赖于该数据的部分，从而避免繁琐的手动更新逻辑，提升系统的实时性、稳定性和维护性。

## 基本概念
- 数据流 ( Data Flow ) : 将数据视作一系列随时间变化的值流，当某个值更新时，其变化将自动传播到所有依赖者。

- 依赖追踪 ( Dependency Tracking ) : 系统自动记录数据之间的依赖关系，确保任何变化都能触发对应更新操作。

- 声明式编程 (Declarative Programming ) :开发者只需声明“数据之间的关系”，而不必显式编写“如何更新”的逻辑，框架会自动完成这些工作。

## 配置要求
编译器：兼容 C++20 ( GCC 10+、Clang 12+、MSVC 19.30+)

构建系统：CMake 3.15+

## 安装
要构建和安装响应式框架，请按照以下步骤作

```shell
git clone https://github.com/shanchuann/Reaction.git && cd Reaction
cmake -B build
cmake --build build/
cmake --install build/ --prefix /your/install/path
```
安装后，您可以在自己的基于 CMake 的项目中包含并链接 reaction :
```cmake
find_package(reaction REQUIRED)
target_link_libraries(your_target PRIVATE reaction)
```

## 快速开始
```cpp
#include <reaction/reaction.h>
#include <iostream>
#include <iomanip>
#include <cmath>

int main() {
    using namespace reaction;

    // 1. Reactive variables for stock prices
    auto buyPrice = var(100.0);      // Price at which stock was bought
    auto currentPrice = var(105.0);  // Current market price

    // 2. Use 'calc' to compute profit or loss amount
    auto profit = calc([&]() {
        return currentPrice() - buyPrice();
    });

    // 3. Use 'expr' to compute percentage gain/loss
    auto profitPercent = expr(std::abs(currentPrice - buyPrice) / buyPrice * 100);

    // 4. Use 'action' to print the log whenever values change
    auto logger = action([&]() {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "[Stock Update] Current Price: $" << currentPrice()
                  << ", Profit: $" << profit()
                  << " (" << profitPercent() << "%)" << std::endl;
    });

    // Simulate price changes
    currentPrice.value(110.0).value(95.0);  // Stock price increases
    *buyPrice = 90.0;                       // Buy price adjusted

    return 0;
}

```

## 基本用法
1. 反应变量：`var`

    使用 var<T> 定义反应式状态变量。

    ```cpp
    auto a = reaction::var(1);         // int variable
    auto b = reaction::var(3.14);      // double variable
    ```
    方法样式的 get 值: `auto val = a.get();`

    简要做法: `auto val = a();`

    方法式赋值: `a.value(2);`

    指针样式赋值: `*a = 2;`

2. 派生计算：`calc`

    使用 **calc** 基于一个或多个 var 实例创建响应式计算。

    Lambda 捕获风格:
    ```cpp
    auto a = reaction::var(1);
    auto b = reaction::var(3.14);
    auto sum = reaction::calc([=]() {
        return a() + b();  // Retrieve current values using a() and b()
    });
    ```
    参数绑定样式 ( 高性能)
    ```cpp
    auto ds = reaction::calc([](auto aa, auto bb) {
        return std::to_string(aa) + std::to_string(bb);
    }, a, b);  // Dependencies: a and b
    ```
3. 声明式表达式：`expr`

    expr 提供了一种简洁明了的语法来声明响应式表达式。当任何因变量发生变化时，结果会自动更新。
    ```cpp
    auto a = reaction::var(1);
    auto b = reaction::var(2);
    auto result = reaction::expr(a + b * 3);  // result updates automatically when 'a' or 'b' change
    ```
4. 反应性副作用：`action`

    注册作以在观察到的变量发生变化时执行副作用。
    ```cpp
    int val = 10;
    auto a = reaction::var(1);
    auto dds = reaction::action([&val]() {
        val = a();
    });
    ```

    当然，要获得高性能，可以使用参数绑定样式。

    ```cpp
    int val = 10;
    auto a = reaction::var(1);
    auto dds = reaction::action([&val](auto aa) {
        val = aa;
    }, a);
    ```
5. 反应式结构体字段：`Field`

    对于具有响应式字段的复杂类型，允许您定义其成员单独响应式的类似结构的变量。
    
    下面是一个 `PersonField` 类的示例：
    ```cpp
    class PersonField : public reaction::FieldBase {
    public:
        PersonField(std::string name, int age):
            m_name(reaction::field(this, name)),
            m_age(reaction::field(this, age)){}

        std::string getName() const { return m_name.get(); }
        void setName(const std::string &name) { *m_name = name; }
        int getAge() const { return m_age.get(); }
        void setAge(int age) { *m_age = age; }

    private:
        reaction::Field<std::string> m_name;
        reaction::Field<int> m_age;
    };

    auto p = reaction::var(PersonField{"Jack", 18});
    auto action = reaction::action(
        []() {
            std::cout << "Action Trigger , name = " << p().getName() << " age = " << p().getAge() << '\n';
        });

    p->setName("Jackson"); // Action Trigger
    p->setAge(28);         // Action Trigger
    ```

6. 复制和移动语义支持
    ```cpp
    auto a = reaction::var(1);
    auto b = reaction::var(3.14);
    auto ds = reaction::calc([]() { return a() + b(); });
    auto ds_copy = ds;
    auto ds_move = std::move(ds);
    EXPECT_FALSE(static_cast<bool>(ds));
    ```

7. 重置节点和依赖项

    反应框架允许您通过替换计算函数来重置计算节点。当在最初创建节点后需要使用不同的逻辑或不同的依赖项重新计算结果时，此机制非常有用。

    > Note: 返回值类型无法更改

    下面是一个演示重置功能的示例

    ```cpp
    TEST(TestReset, ReactionTest) {
        auto a = reaction::var(1);
        auto b = reaction::var(std::string{"2"});
        auto ds = reaction::calc([]() { return std::to_string(a()); });
        auto ret = ds.set([=]() { return b() + "set"; });
        EXPECT_EQ(ret, reaction::ReactionError::NoErr);

        ret = ds.set([=]() { return a(); });
        EXPECT_EQ(ret, reaction::ReactionError::ReturnTypeErr);

        ret = ds.set([=]() { return ds(); });
        EXPECT_EQ(ret, reaction::ReactionError::CycleDepErr);
    }
    ```

8. 触发模式

    该框架支持各种触发模式来控制何时重新评估反应式计算。此示例演示了三种模式：

    值变化触发器：仅当基础值实际更改时，才会触发响应式计算。
    
    阈值触发器：当值超过指定阈值时，将触发反应式计算。
    
    始终触发：无论值是否已更改，始终触发。
    
    触发模式可以通过 `type` 参数指定
    ```cpp
    using namespace reaction;
    auto stockPrice = var(100.0);
    auto profit = expr<ChangedTrigger>(stockPrice() - 100.0);
    auto assignAction = action([=]() {  // defalut AlwaysTrigger
        std::cout << "Checky assign, price = " << stockPrice() <<'\n';
    });
    auto sellAction = action<ThresholdTrigger>([=]() {
        std::cout << "It's time to sell, profit = " << profit() <<'\n';
    });
    sellAction.setThreshold([=]() {
        return profit() > 5.0;
    });
    *stockPrice = 100.0; // assignAction trigger
    *stockPrice = 101.0; // assignAction, profit trigger
    *stockPrice = 106.0; // all trigger
    ```
    您可以在代码中自己定义触发模式，只需包含 **checkTrigger** 方法即可：
    ```cpp
    struct MyTrigger {
        bool checkTrigger() {
            // do something
            return true;
        }
    };
    auto a = var(1);
    auto b = expr<MyTrigger>(a + 1);
    ```
9. 无效策略

    在框架中，用户获取的所有数据源实际上都是弱引用的形式，其实际内存在观察者地图中进行管理。

    用户可以手动调用`close`方法，这样所有依赖的数据源也会被关闭。
    ```cpp
    auto a = reaction::var(1);
    auto b = reaction::var(2);
    auto dsA = reaction::calc([=]() { return a(); });
    auto dsB = reaction::calc([=]() { return dsA() + b(); });
    dsA.close(); //dsB will automatically close, cause dsB dependents dsA.
    EXPECT_FALSE(static_cast<bool>(dsA));
    EXPECT_FALSE(static_cast<bool>(dsB));
    ```
    但是，对于用户获取的弱引用的生命周期结束的场景，框架针对不同的场景制定了多种策略。

    **DirectCloseStrategy**：当节点的任何依赖项无效时，节点将立即关闭（无效）。

    **KeepCalcStrategy**：节点继续重新计算，其依赖关系正常工作。

    **LastValStrategy**：节点保留最后一个有效值，其依赖项使用该值进行计算。

    下面是一个简明的示例，说明了所有三种策略：
    ```cpp
    {
        auto a = var(1);
        auto b = calc([]() { return a(); });
        {
            auto temp = calc([]() { return a(); }); // default is DirectCloseStrategy
            b.set([]() { return temp(); });
        }
        // temp lifecycle ends, b will end too.
        EXPECT_FALSE(static_cast<bool>(b));
    }
    
    {
        auto a = var(1);
        auto b = calc([]() { return a(); });
        {
            auto temp = calc<AlwaysTrigger, KeepCalcStrategy>([]() { return a(); }); // default is DirectFailureStrategy
            b.set([]() { return temp(); });
        }
        // temp lifecycle ends, b not be influenced.
        EXPECT_TRUE(static_cast<bool>(b));
        EXPECT_EQ(b.get(), 1);
        a.value(2);
        EXPECT_EQ(b.get(), 2);
    }

    {
        auto a = var(1);
        auto b = calc([]() { return a(); });
        {
            auto temp = calc<AlwaysTrigger, LastValStrategy>([]() { return a(); }); // default is DirectFailureStrategy
            b.set([]() { return temp(); });
        }
        // temp lifecycle ends, b use its last val to calculate.
        EXPECT_TRUE(static_cast<bool>(b));
        EXPECT_EQ(b.get(), 1);
        a.value(2);
        EXPECT_EQ(b.get(), 1);
    }
    ```

    您同样可以在代码中自己定义策略，只需包含 `handleInvalid` 方法即可：
    ```cpp
    struct MyStrategy {
        void handleInvalid() {
            std::cout << "Invalid" << std::endl;
        }
    };
    auto a = var(1);
    auto b = expr<AlwaysTrigger, MyStrategy>(a + 1);
    ```

## 总结

Reaction 框架借助 C++ 20 的强大能力，希望为 C++ 开发者提供一种全新的响应式思维模型，特别是在嵌入式界面、工业软件与复杂状态管理等高性能场景中。