#ifndef GMOCK_HPP
#define GMOCK_HPP

#include 
#include 
#include 
#include 

namespace testing {

// Mock function interface
template
class MockFunction {
private:
    std::function mock_impl;
    int call_count = 0;
    int expected_calls = -1;  // -1 means no expectation set

public:
    void SetImplementation(std::function impl) {
        mock_impl = impl;
    }

    void ExpectCalls(int n) {
        expected_calls = n;
        call_count = 0;
    }

    R operator()(Args... args) {
        if (!mock_impl) {
            throw std::runtime_error("Mock implementation not set");
        }
        call_count++;
        return mock_impl(args...);
    }

    bool VerifyExpectations() {
        return expected_calls == -1 || call_count == expected_calls;
    }

    int GetCallCount() const {
        return call_count;
    }
};

// Mock class base
class MockBase {
protected:
    std::map expectations_met;

public:
    virtual ~MockBase() = default;
    
    bool VerifyAllExpectations() const {
        for (const auto& exp : expectations_met) {
            if (!exp.second) return false;
        }
        return true;
    }
};

// Expectation macros
#define EXPECT_CALL(mock_obj, method, times) \
    mock_obj.method.ExpectCalls(times)

#define ON_CALL(mock_obj, method) \
    mock_obj.method.SetImplementation

#define MOCK_METHOD(return_type, method_name, args) \
    MockFunction method_name

#define MOCK_CONST_METHOD(return_type, method_name, args) \
    const MockFunction method_name

// Matcher interface
template
class Matcher {
public:
    virtual ~Matcher() = default;
    virtual bool Matches(const T& arg) const = 0;
    virtual std::string Description() const = 0;
};

// Common matchers
template
class EqMatcher : public Matcher {
private:
    T expected;

public:
    explicit EqMatcher(const T& value) : expected(value) {}

    bool Matches(const T& arg) const override {
        return arg == expected;
    }

    std::string Description() const override {
        return "equals " + std::to_string(expected);
    }
};

// Matcher creation functions
template
EqMatcher Eq(const T& value) {
    return EqMatcher(value);
}

} // namespace testing

#endif // GMOCK_HPP