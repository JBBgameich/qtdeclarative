TEMPLATE=subdirs
SUBDIRS=\
    qml \
    headersclean \
    qmldevtools \
    cmake \
    installed_cmake

!mac {
SUBDIRS += \
    quick \
    particles \
    qmltest
}

installed_cmake.depends = cmake

testcocoon: SUBDIRS -= headersclean
