cc_library(
    name = "wsd",
    srcs = glob(
        include = [
            "src/*.cc",
            "src/*.cpp",
            "include/wsd/**/*.h",
        ],
        exclude = [
            "src/benchmark*.cc",
            "src/benchmark*.cpp",
            "include/wsd/detail/benchmark*.h",
        ],
    ),
    hdrs = glob(
        include = ["include/wsd/*.h"],
        exclude = ["include/wsd/benchmark*.h"]
    ),
    includes = ["include"],
    deps = [
        "@boost//:boost",
    ],
    copts = [
        "-std=c++11",
    ],
    visibility = [ "//visibility:public" ],
)

cc_library(
    name = "benchmark",
    srcs = [
        "include/wsd/detail/benchmark_detail.h",
        "src/benchmark.cpp",
    ],
    hdrs = ["include/wsd/benchmark.h"],
    strip_include_prefix = "include",
    deps = [
        "@gflags//:gflags",
        ":wsd",
    ],
    copts = [
        "-std=c++11",
        "-Iinclude",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "benchmark_main",
    srcs = [
        "src/benchmark_main.cpp",
    ],
    deps = [
        ":benchmark",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
        "-Iinclude",
    ],
    visibility = ["//visibility:public"],
)

