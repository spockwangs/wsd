cc_test(
    name = "callback_test",
    srcs = ["callback_test.cc"],
    deps = [
        "//:wsd",
        "@gtest//:gtest_main",
        ":es_test",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)

cc_library(
    name = "es_test",
    srcs = [
        "es_test.cc",
    ],
    hdrs = [
        "es_test.h",
    ],
    deps = [
        "@gtest//:gtest",
    ],
    copts = [
        "-std=c++11",
    ],
)

cc_test(
    name = "loading_cache_test",
    srcs = ["loading_cache_test.cc"],
    deps = [
        "//:wsd",
        "@gtest//:gtest_main",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)

cc_test(
    name = "url_test",
    srcs = [
        "url_test.cc",
    ],
    deps = [
        "@gtest//:gtest_main",
        "//:wsd",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)

cc_test(
    name = "promise_test",
    srcs = [
        "promise_test.cc",
    ],
    deps = [
        "@gtest//:gtest_main",
        "//:wsd",
        ":es_test",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)

cc_test(
    name = "when_all_test",
    srcs = [
        "when_all_test.cc",
    ],
    deps = [
        "@gtest//:gtest_main",
        "//:wsd",
        ":es_test",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)

cc_test(
    name = "concurrent_map_test",
    srcs = [
        "concurrent_map_test.cc",
    ],
    deps = [
        "@gtest//:gtest_main",
        "//:wsd",
        ":es_test",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)

cc_test(
    name = "singleton_test",
    srcs = [
        "singleton_test.cc",
    ],
    deps = [
        "@gtest//:gtest_main",
        "//:wsd",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)

cc_test(
    name = "hazard_pointer_test",
    srcs = [
        "hazard_pointer_test.cc",
    ],
    deps = [
        "@gtest//:gtest_main",
        "//:wsd",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)

cc_test(
    name = "ebr_test",
    srcs = [
        "ebr_test.cc",
    ],
    deps = [
        "@gtest//:gtest_main",
        "//:wsd",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)

cc_test(
    name = "combining_tree_test",
    srcs = [
        "combining_tree_test.cc",
    ],
    deps = [
        "//:wsd",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)

cc_test(
    name = "benchmark_test",
    srcs = ["benchmark_test.cc"],
    deps = [
        "//:benchmark_main",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)
    
cc_test(
    name = "bitonic_test",
    srcs = ["bitonic_test.cpp"],
    deps = [
        "@gtest//:gtest_main",
        "//:wsd",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)

cc_test(
    name = "bitonic_bench",
    srcs = ["bitonic_bench.cpp"],
    deps = [
        "//:wsd",
        "//:benchmark_main",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)        

cc_test(
    name = "hash_map_bench",
    srcs = ["hash_map_bench.cpp"],
    deps = [
        "//:benchmark_main",
    ],
    copts = [
        "-std=c++14",
    ],
    linkopts = [
        "-lfolly",
    ],
    linkstatic = True,
)        
