load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_library")

proto_library(
    name = "order_proto",
    srcs = ["order.proto"],
)

cc_proto_library(
    name = "order_cc_proto",
    deps = [":order_proto"],
    deps = [":order_proto"]
)

cc_library(
    name = "repository",
    srcs = [
        "repository.h",
        "repository_impl.cpp",
        "repository_impl.h",
    ],
    copts = [
        "-std=c++14",
        "repository_impl.h",
        "repository_impl.cpp",
    ],
    deps = [
        ":order_cc_proto",
        "@google_absl//absl/status",
    ],
    copts = [
        "-std=c++14",
    ],
)

cc_test(
    name = "repository_test",
    srcs = ["repository_test.cpp"],
    deps = [
        "@gtest//:gtest_main",
        ":repository",
    ],
    copts = [
        "-std=c++14",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
    deps = [
        ":repository",
        "@gtest//:gtest_main",
    ],
)
