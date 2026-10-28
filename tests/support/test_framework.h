#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

// A deliberately small test harness so the project needs no external
// libraries.
//
//   TEST_CASE("DDA-01 draws a horizontal line") { ... }
//
//   CHECK(condition)                      fails if condition is false
//   CHECK_EQ(actual, expected)            fails if actual != expected
//   CHECK_NEAR(actual, expected, delta)   fails if |actual - expected| > delta
//
// Each test file ends with:
//
//   int main() { return testing::runAllTests(); }
//
// The program exits with 1 if any check failed. CTest (and therefore the
// GitHub Actions workflow) uses that exit code to mark a test as failed.

#include <cmath>
#include <cstddef>
#include <exception>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// Printing helpers so failed checks can show the values involved.
template <typename First, typename Second>
std::ostream& operator<<(std::ostream& stream,
                         const std::pair<First, Second>& value) {
    return stream << "(" << value.first << ", " << value.second << ")";
}

template <typename Container>
std::ostream& printSequence(std::ostream& stream, const Container& values) {
    const std::size_t limit = 12;
    std::size_t index = 0;

    stream << "[";

    for (const auto& value : values) {
        if (index == limit) {
            stream << ", ... (" << values.size() << " items)";
            break;
        }

        if (index > 0) {
            stream << ", ";
        }

        stream << value;
        index++;
    }

    return stream << "]";
}

template <typename Value>
std::ostream& operator<<(std::ostream& stream,
                         const std::vector<Value>& values) {
    return printSequence(stream, values);
}

template <typename Value>
std::ostream& operator<<(std::ostream& stream,
                         const std::set<Value>& values) {
    return printSequence(stream, values);
}

namespace testing {

struct TestCase {
    std::string name;
    std::function<void()> body;
};

inline std::vector<TestCase>& registeredTests() {
    static std::vector<TestCase> tests;
    return tests;
}

// Failure messages collected while the current test case runs.
inline std::vector<std::string>& currentFailures() {
    static std::vector<std::string> failures;
    return failures;
}

struct TestRegistrar {
    TestRegistrar(const std::string& name, std::function<void()> body) {
        registeredTests().push_back({name, std::move(body)});
    }
};

inline void reportFailure(const char* file, int line,
                          const std::string& message) {
    std::ostringstream failure;
    failure << "    " << file << ":" << line << ": " << message;
    currentFailures().push_back(failure.str());
}

inline int runAllTests() {
    int failedTests = 0;

    for (const TestCase& test : registeredTests()) {
        currentFailures().clear();

        try {
            test.body();
        } catch (const std::exception& error) {
            currentFailures().push_back(
                std::string("    unexpected exception: ") + error.what());
        } catch (...) {
            currentFailures().push_back("    unexpected exception");
        }

        if (currentFailures().empty()) {
            std::cout << "[PASS] " << test.name << "\n";
        } else {
            std::cout << "[FAIL] " << test.name << "\n";

            for (const std::string& failure : currentFailures()) {
                std::cout << failure << "\n";
            }

            failedTests++;
        }
    }

    const int totalTests = static_cast<int>(registeredTests().size());

    std::cout << "\n" << (totalTests - failedTests) << " of " << totalTests
              << " test cases passed.\n";

    return failedTests == 0 ? 0 : 1;
}

}  // namespace testing

#define TESTING_JOIN_INNER(first, second) first##second
#define TESTING_JOIN(first, second) TESTING_JOIN_INNER(first, second)

#define TEST_CASE(name)                                                    \
    static void TESTING_JOIN(testFunction, __LINE__)();                    \
    static const ::testing::TestRegistrar TESTING_JOIN(testRegistrar,      \
                                                       __LINE__)(          \
        name, TESTING_JOIN(testFunction, __LINE__));                       \
    static void TESTING_JOIN(testFunction, __LINE__)()

#define CHECK(condition)                                                   \
    do {                                                                   \
        if (!(condition)) {                                                \
            ::testing::reportFailure(__FILE__, __LINE__,                   \
                                     "CHECK(" #condition ") failed");      \
        }                                                                  \
    } while (false)

#define CHECK_EQ(actual, expected)                                         \
    do {                                                                   \
        const auto& actualValue = (actual);                                \
        const auto& expectedValue = (expected);                            \
        if (!(actualValue == expectedValue)) {                             \
            std::ostringstream message;                                    \
            message << "CHECK_EQ(" #actual ", " #expected ") failed\n"     \
                    << "        actual:   " << actualValue << "\n"         \
                    << "        expected: " << expectedValue;              \
            ::testing::reportFailure(__FILE__, __LINE__, message.str());   \
        }                                                                  \
    } while (false)

#define CHECK_NEAR(actual, expected, tolerance)                            \
    do {                                                                   \
        const double actualValue = static_cast<double>(actual);            \
        const double expectedValue = static_cast<double>(expected);        \
        if (!(std::fabs(actualValue - expectedValue) <= (tolerance))) {    \
            std::ostringstream message;                                    \
            message << "CHECK_NEAR(" #actual ", " #expected ") failed\n"   \
                    << "        actual:   " << actualValue << "\n"         \
                    << "        expected: " << expectedValue               \
                    << " (tolerance " << (tolerance) << ")";               \
            ::testing::reportFailure(__FILE__, __LINE__, message.str());   \
        }                                                                  \
    } while (false)

#endif
