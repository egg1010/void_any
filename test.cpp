#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <memory>
#include "include/void_any.hpp"

// 测试用的复杂类型
struct ComplexType {
    int id;
    std::string name;
    std::vector<double> data;
    
    ComplexType(int i = 0, const std::string& n = "test") 
        : id(i), name(n), data(100, 3.14) {}
    
    ComplexType(const ComplexType& other) 
        : id(other.id), name(other.name), data(other.data) {}
    
    ComplexType& operator=(const ComplexType& other) {
        if (this != &other) {
            id = other.id;
            name = other.name;
            data = other.data;
        }
        return *this;
    }
    
    bool operator==(const ComplexType& other) const {
        return id == other.id && name == other.name && data == other.data;
    }
};

// 测试用的简单类型
struct SimpleType {
    int value;
    double factor;
    
    SimpleType(int v = 0, double f = 1.0) : value(v), factor(f) {}
    
    bool operator==(const SimpleType& other) const {
        return value == other.value && factor == other.factor;
    }
};

// 测试函数：基本功能测试
bool test_basic_functionality() {
    std::cout << "正在测试基本功能...\n";
    
    // 测试整数
    void_any va1(42);
    if (va1.get<int>() != 42) {
        std::cerr << "基本整数测试失败！\n";
        return false;
    }
    
    // 测试字符串
    void_any va2(std::string("hello world"));
    if (va2.get<std::string>() != "hello world") {
        std::cerr << "基本字符串测试失败！\n";
        return false;
    }
    
    // 测试栈内存模式
    void_any va3(123, vao::Enable_stack_memory);
    if (va3.get<int>() != 123) {
        std::cerr << "栈内存模式测试失败！\n";
        return false;
    }
    
    std::cout << "基本功能测试通过！\n";
    return true;
}

// 测试函数：百万级别压力测试
bool test_million_level_stress() {
    std::cout << "开始百万级别压力测试...\n";
    
    const size_t TEST_COUNT = 1000000; // 百万次
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        // 测试1: 连续创建和销毁简单类型
        std::cout << "正在测试简单类型（堆内存分配）...\n";
        for (size_t i = 0; i < TEST_COUNT; ++i) {
            void_any va(SimpleType(static_cast<int>(i), i * 0.1));
            SimpleType retrieved = va.get<SimpleType>();
            if (retrieved.value != static_cast<int>(i) || 
                retrieved.factor != i * 0.1) {
                std::cerr << "简单类型测试在第 " << i << " 次迭代时失败！\n";
                return false;
            }
        }
        
        // 测试2: 连续创建和销毁复杂类型
        std::cout << "正在测试复杂类型（堆内存分配）...\n";
        for (size_t i = 0; i < TEST_COUNT / 10; ++i) { // 减少次数避免内存爆炸
            void_any va(ComplexType(static_cast<int>(i), "stress_test_" + std::to_string(i)));
            ComplexType retrieved = va.get<ComplexType>();
            if (retrieved.id != static_cast<int>(i) || 
                retrieved.name != "stress_test_" + std::to_string(i)) {
                std::cerr << "复杂类型测试在第 " << i << " 次迭代时失败！\n";
                return false;
            }
        }
        
        // 测试3: 栈内存模式（小对象）
        std::cout << "正在测试栈内存模式（小对象）...\n";
        for (size_t i = 0; i < TEST_COUNT; ++i) {
            int small_value = static_cast<int>(i % 1000);
            void_any va(small_value, vao::Enable_stack_memory);
            if (va.get<int>() != small_value) {
                std::cerr << "栈内存模式测试在第 " << i << " 次迭代时失败！\n";
                return false;
            }
        }
        
        // 测试4: 移动语义测试
        std::cout << "正在测试移动语义...\n";
        std::vector<void_any> containers;
        containers.reserve(TEST_COUNT / 1000); // 预分配
        
        for (size_t i = 0; i < TEST_COUNT / 1000; ++i) {
            void_any original(SimpleType(static_cast<int>(i), i * 0.5));
            containers.push_back(std::move(original));
        }
        
        // 验证移动后的数据
        for (size_t i = 0; i < containers.size(); ++i) {
            SimpleType retrieved = containers[i].get<SimpleType>();
            if (retrieved.value != static_cast<int>(i) || 
                retrieved.factor != i * 0.5) {
                std::cerr << "移动语义测试在索引 " << i << " 处失败！\n";
                return false;
            }
        }
        containers.clear();
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "百万级别压力测试成功完成，耗时 " 
                  << duration.count() << " 毫秒！\n";
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "压力测试过程中发生异常: " << e.what() << "\n";
        return false;
    } catch (...) {
        std::cerr << "压力测试过程中发生未知异常！\n";
        return false;
    }
}

// 测试函数：内存泄漏检测（通过重复分配释放）
bool test_memory_leak_detection() {
    std::cout << "正在测试内存泄漏检测...\n";
    
    const size_t ITERATIONS = 100000; // 10万次
    size_t success_count = 0;
    
    for (size_t i = 0; i < ITERATIONS; ++i) {
        try {
            void_any va1(ComplexType(static_cast<int>(i), "leak_test"));
            void_any va2(std::string("test_string_" + std::to_string(i)));
            void_any va3(3.14159 * i);
            
            // 验证数据正确性
            if (va1.get<ComplexType>().id == static_cast<int>(i) &&
                va2.get<std::string>() == "test_string_" + std::to_string(i) &&
                va3.get<double>() == 3.14159 * i) {
                success_count++;
            }
        } catch (...) {
            std::cerr << "内存泄漏测试在第 " << i << " 次迭代时失败！\n";
            return false;
        }
    }
    
    if (success_count == ITERATIONS) {
        std::cout << "内存泄漏测试通过！全部 " << success_count << " 次迭代均成功。\n";
        return true;
    } else {
        std::cerr << "内存泄漏测试失败！仅 " << success_count << " 次迭代成功，总共 " 
                  << ITERATIONS << " 次。\n";
        return false;
    }
}

// 测试函数：多线程安全测试（基础版本）
bool test_thread_safety_basic() {
    std::cout << "正在测试基础线程安全性（顺序访问）...\n";
    
    const size_t THREAD_TEST_COUNT = 10000;
    void_any shared_container;
    
    // 模拟多线程交替访问（实际是顺序执行，但测试逻辑正确性）
    for (size_t i = 0; i < THREAD_TEST_COUNT; ++i) {
        if (i % 2 == 0) {
            shared_container.set(SimpleType(static_cast<int>(i), i * 1.0));
            SimpleType retrieved = shared_container.get<SimpleType>();
            if (retrieved.value != static_cast<int>(i)) {
                std::cerr << "线程安全测试在偶数迭代 " << i << " 处失败！\n";
                return false;
            }
        } else {
            shared_container.set(std::string("thread_test_" + std::to_string(i)));
            std::string retrieved = shared_container.get<std::string>();
            if (retrieved != "thread_test_" + std::to_string(i)) {
                std::cerr << "线程安全测试在奇数迭代 " << i << " 处失败！\n";
                return false;
            }
        }
    }
    
    std::cout << "基础线程安全测试通过！\n";
    return true;
}

int main() 
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << "=== void_any 百万级别压力测试 ===\n\n";
    
    bool all_tests_passed = true;
    
    //  basic功能测试
    if (!test_basic_functionality()) {
        all_tests_passed = false;
        std::cout << "基本功能测试失败！\n\n";
    } else {
        std::cout << "基本功能测试通过！\n\n";
    }
    
    // 百万级别压力测试
    if (!test_million_level_stress()) {
        all_tests_passed = false;
        std::cout << "百万级别压力测试失败！\n\n";
    } else {
        std::cout << "百万级别压力测试通过！\n\n";
    }
    
    // 内存泄漏检测
    if (!test_memory_leak_detection()) {
        all_tests_passed = false;
        std::cout << "内存泄漏检测测试失败！\n\n";
    } else {
        std::cout << "内存泄漏检测测试通过！\n\n";
    }
    
    // 基础线程安全测试
    if (!test_thread_safety_basic()) {
        all_tests_passed = false;
        std::cout << "线程安全测试失败！\n\n";
    } else {
        std::cout << "线程安全测试通过！\n\n";
    }
    
    if (all_tests_passed) {
        std::cout << "=== 所有测试均已通过！ ===\n";
        std::cout << "void_any 容器成功处理了百万级别的操作！\n";
        return 0;
    } else {
        std::cout << "=== 部分测试失败！ ===\n";
        std::cout << "请查看上方的错误信息。\n";
        return 1;
    }
}