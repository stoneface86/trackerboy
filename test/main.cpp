
#include "config.hpp"

#include <iostream>
#include <memory>
#include <cstring>

#include <QtTest/QtTest>

enum ExitCodes {
    ExitArguments = 1,
    ExitNoTest = 2,
    ExitNoInstantiation = 3
};


template <class Test, class... Tests>
QMetaObject const* searchTest(const char *name) {
    if (strcmp(name, Test::staticMetaObject.className()) == 0) {
        return &Test::staticMetaObject;
    }

    if constexpr (sizeof...(Tests) != 0) {
        return searchTest<Tests...>(name);
    } else {
        return nullptr;
    }
}


int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <testname> [QTest args...]\n";
        return ExitArguments;
    }

    auto meta = searchTest<CONFIG_TESTS>(argv[1]);
    if (meta == nullptr) {
        std::cerr << "unknown test case\n";
        return ExitNoTest;
    }

    std::unique_ptr<QObject> test(meta->newInstance());
    if (test == nullptr) {
        std::cerr << "could not instantiate test class\n";
        return ExitNoInstantiation;
    }

    return QTest::qExec(test.get(), argc - 1, argv + 1);
}
