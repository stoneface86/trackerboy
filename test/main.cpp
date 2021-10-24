
#include "config.hpp"

#include <iostream>
#include <unordered_map>
#include <memory>

#include <QtTest/QtTest>

using TestMap = std::unordered_map<std::string, const QMetaObject *>;

enum ExitCodes {
    ExitArguments = 1,
    ExitNoTest = 2,
    ExitNoInstantiation = 3
};

template <class Test, class... Tests>
void registerTests(TestMap &map) {
    map.emplace(Test::staticMetaObject.className(), &Test::staticMetaObject);
    if constexpr (sizeof...(Tests) != 0) {
        registerTests<Tests...>(map);
    }
}



int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <testname> [QTest args...]\n";
        return ExitArguments;
    }

    TestMap map;
    registerTests<
        CONFIG_TESTS
    >(map);

    // find the test by name
    auto iter = map.find(argv[1]);
    if (iter == map.end()) {
        std::cerr << "unknown test case\n";
        return ExitNoTest;
    }

    // instantiate the test
    std::unique_ptr<QObject> test(iter->second->newInstance());
    if (test == nullptr) {
        std::cerr << "could not instantiate test class\n";
        return ExitNoInstantiation;
    }

    return QTest::qExec(test.get(), argc - 1, argv + 1);
}
