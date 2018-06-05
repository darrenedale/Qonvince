
lessThan(QT_MAJOR_VERSION, 5) {
    error("Qt version 5 is required")
}

*-g++ {
    QMAKE_CXXFLAGS_RELEASE += -O3
}

*-clang {
    QMAKE_CXXFLAGS_RELEASE += -O3
}

*-msvc* {
    QMAKE_CXXFLAGS_RELEASE += /O3
}
