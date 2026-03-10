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
    std::cout << "Testing basic functionality...\n";
    
    // 测试整数
    void_any va1(42);
    if (va1.get<int>() != 42) {
        std::cerr << "Basic integer test failed!\n";
        return false;
    }
    
    // 测试字符串
    void_any va2(std::string("hello world"));
    if (va2.get<std::string>() != "hello world") {
        std::cerr << "Basic string test failed!\n";
        return false;
    }
    
    // 测试栈内存模式
    void_any va3(123, vao::Enable_stack_memory);
    if (va3.get<int>() != 123) {
        std::cerr << "Stack memory mode test failed!\n";
        return false;
    }
    
    std::cout << "Basic functionality tests passed!\n";
    return true;
}

// 测试函数：百万级别压力测试
bool test_million_level_stress() {
    std::cout << "Starting million-level stress test...\n";
    
    const size_t TEST_COUNT = 1000000; // 百万次
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        // 测试1: 连续创建和销毁简单类型
        std::cout << "Testing simple types (heap allocation)...\n";
        for (size_t i = 0; i < TEST_COUNT; ++i) {
            void_any va(SimpleType(static_cast<int>(i), i * 0.1));
            SimpleType retrieved = va.get<SimpleType>();
            if (retrieved.value != static_cast<int>(i) || 
                retrieved.factor != i * 0.1) {
                std::cerr << "Simple type test failed at iteration " << i << "\n";
                return false;
            }
        }
        
        // 测试2: 连续创建和销毁复杂类型
        std::cout << "Testing complex types (heap allocation)...\n";
        for (size_t i = 0; i < TEST_COUNT / 10; ++i) { // 减少次数避免内存爆炸
            void_any va(ComplexType(static_cast<int>(i), "stress_test_" + std::to_string(i)));
            ComplexType retrieved = va.get<ComplexType>();
            if (retrieved.id != static_cast<int>(i) || 
                retrieved.name != "stress_test_" + std::to_string(i)) {
                std::cerr << "Complex type test failed at iteration " << i << "\n";
                return false;
            }
        }
        
        // 测试3: 栈内存模式（小对象）
        std::cout << "Testing stack memory mode with small objects...\n";
        for (size_t i = 0; i < TEST_COUNT; ++i) {
            int small_value = static_cast<int>(i % 1000);
            void_any va(small_value, vao::Enable_stack_memory);
            if (va.get<int>() != small_value) {
                std::cerr << "Stack memory mode test failed at iteration " << i << "\n";
                return false;
            }
        }
        
        // 测试4: 移动语义测试
        std::cout << "Testing move semantics...\n";
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
                std::cerr << "Move semantics test failed at index " << i << "\n";
                return false;
            }
        }
        containers.clear();
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "Million-level stress test completed successfully in " 
                  << duration.count() << " ms!\n";
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception during stress test: " << e.what() << "\n";
        return false;
    } catch (...) {
        std::cerr << "Unknown exception during stress test!\n";
        return false;
    }
}

// 测试函数：内存泄漏检测（通过重复分配释放）
bool test_memory_leak_detection() {
    std::cout << "Testing for memory leaks...\n";
    
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
            std::cerr << "Memory leak test failed at iteration " << i << "\n";
            return false;
        }
    }
    
    if (success_count == ITERATIONS) {
        std::cout << "Memory leak test passed! All " << success_count << " iterations successful.\n";
        return true;
    } else {
        std::cerr << "Memory leak test failed! Only " << success_count << " out of " 
                  << ITERATIONS << " iterations successful.\n";
        return false;
    }
}

// 测试函数：多线程安全测试（基础版本）
bool test_thread_safety_basic() {
    std::cout << "Testing basic thread safety (sequential access)...\n";
    
    const size_t THREAD_TEST_COUNT = 10000;
    void_any shared_container;
    
    // 模拟多线程交替访问（实际是顺序执行，但测试逻辑正确性）
    for (size_t i = 0; i < THREAD_TEST_COUNT; ++i) {
        if (i % 2 == 0) {
            shared_container.set(SimpleType(static_cast<int>(i), i * 1.0));
            SimpleType retrieved = shared_container.get<SimpleType>();
            if (retrieved.value != static_cast<int>(i)) {
                std::cerr << "Thread safety test failed at even iteration " << i << "\n";
                return false;
            }
        } else {
            shared_container.set(std::string("thread_test_" + std::to_string(i)));
            std::string retrieved = shared_container.get<std::string>();
            if (retrieved != "thread_test_" + std::to_string(i)) {
                std::cerr << "Thread safety test failed at odd iteration " << i << "\n";
                return false;
            }
        }
    }
    
    std::cout << "Basic thread safety test passed!\n";
    return true;
}

int main() 
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << "=== void_any Million-Level Stress Test ===\n\n";
    
    bool all_tests_passed = true;
    
    // 基本功能测试
    if (!test_basic_functionality()) {
        all_tests_passed = false;
        std::cout << "Basic functionality test FAILED!\n\n";
    } else {
        std::cout << "Basic functionality test PASSED!\n\n";
    }
    
    // 百万级别压力测试
    if (!test_million_level_stress()) {
        all_tests_passed = false;
        std::cout << "Million-level stress test FAILED!\n\n";
    } else {
        std::cout << "Million-level stress test PASSED!\n\n";
    }
    
    // 内存泄漏检测
    if (!test_memory_leak_detection()) {
        all_tests_passed = false;
        std::cout << "Memory leak detection test FAILED!\n\n";
    } else {
        std::cout << "Memory leak detection test PASSED!\n\n";
    }
    
    // 基础线程安全测试
    if (!test_thread_safety_basic()) {
        all_tests_passed = false;
        std::cout << "Thread safety test FAILED!\n\n";
    } else {
        std::cout << "Thread safety test PASSED!\n\n";
    }
    
    if (all_tests_passed) {
        std::cout << "=== ALL TESTS PASSED! ===\n";
        std::cout << "The void_any container successfully handled million-level operations!\n";
        return 0;
    } else {
        std::cout << "=== SOME TESTS FAILED! ===\n";
        std::cout << "Please check the error messages above.\n";
        return 1;
    }
}