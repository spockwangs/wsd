load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_library")

proto_library(
    name = "order_proto",
    srcs = ["order.proto"],
)

cc_proto_library(
    name = "order_cc_proto",
    deps = [":order_proto"],
)

cc_library(
    name = "repository",
    srcs = [
        "repository.h",
        "repository_impl.h",
    ],
    deps = [
        ":order_cc_proto",
        "@google_absl//absl/status:status",
        "@google_absl//absl/status:statusor",
    ],
    copts = [
        "-std=c++14",
    ],
)

cc_library(
    name = "entity",
    srcs = [
        "entity.h",
    ],
    copts = [
        "-std=c++14",
    ],
)

cc_test(
    name = "repository_test",
    srcs = [
        "repository_test.cpp",
    ],
    deps = [
        "@gtest//:gtest_main",
        ":repository",
        ":entity",
    ],
    copts = [
        "-std=c++14",
        "-Wall",
        "-Werror",
    ],
    linkstatic = True,
)
