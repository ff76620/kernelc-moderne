#ifndef GTEST_HPP
#define GTEST_HPP

#include 
#include 
#include 
#include 

namespace testing {

// Test result structure
struct TestResult {
    std::string test_name;
    bool passed;
    std::string message;
};

// Test case class
class Test {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}

public:
    virtual ~Test() {}
    virtual void TestBody() = 0;
};

// Test suite class
class TestSuite {
private:
    std::vector>> tests;
    std::vector results;

public:
    void AddTest(const std::string& name, std::function test) {
        tests.push_back({name, test});
    }

    void Run() {
        for (const auto& test : tests) {
            TestResult result;
            result.test_name = test.first;
            
            try {
                test.second();
                result.passed = true;
                result.message = "Test passed";
            } catch (const std::exception& e) {
                result.passed = false;
                result.message = e.what();
            }

            results.push_back(result);
        }
    }

    void PrintResults() {
        int passed = 0;
        for (const auto& result : results) {
            std::cout << (result.passed ? "[PASS] " : "[FAIL] ")
                      << result.test_name << ": " 
                      << result.message << std::endl;
            if (result.passed) passed++;
        }
        
        std::cout << "\nTotal tests: " << results.size() 
                  << "\nPassed: " << passed
                  << "\nFailed: " << (results.size() - passed) 
                  << std::endl;
    }
};

// Assertion macros
#define ASSERT_TRUE(condition) \
    if (!(condition)) throw std::runtime_error("Assertion failed: " #condition)

#define ASSERT_FALSE(condition) \
    if (condition) throw std::runtime_error("Assertion failed: " #condition)

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) throw std::runtime_error("Assertion failed: " #expected " != " #actual)

#define ASSERT_NE(val1, val2) \
    if ((val1) == (val2)) throw std::runtime_error("Assertion failed: " #val1 " == " #val2)

#define TEST_F(test_fixture, test_name) \
    class test_fixture##_##test_name##_Test : public test_fixture { \
    public: \
        void TestBody(); \
    }; \
    void test_fixture##_##test_name##_Test::TestBody()

} // namespace testing

#endif // GTEST_HPP