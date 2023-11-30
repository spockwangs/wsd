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
    name = "domain",
    hdrs = [
        "repository.h",
        "order.h",
        "entity.h",
    ],
    srcs = [
    ],
    deps = [
        "@google_absl//absl/status:status",
        "@google_absl//absl/status:statusor",
    ],
)

cc_library(
    name = "infra",
    hdrs = [
        "order_dao_kv_impl.h",
        "persistence_repository_impl.h",
        "collection_repository_impl.h",
        "order_dao_db_impl.h",
    ],
    srcs = [
        "order_dao_kv_impl.cpp",
        "order_dao_db_impl.cpp",
    ],
    deps = [
        ":domain",
        ":order_cc_proto",
        "@google_absl//absl/status:status",
        "@google_absl//absl/status:statusor",
    ],
)

cc_test(
    name = "persistence_repository_test",
    srcs = [
        "persistence_repository_test.cpp",
    ],
    deps = [
        "@gtest//:gtest_main",
        ":infra",
    ],
    linkstatic = True,
)

cc_test(
    name = "collection_repository_test",
    srcs = [
        "collection_repository_test.cpp",
    ],
    deps = [
        "@gtest//:gtest_main",
        ":infra",
    ],
    linkstatic = True,
)
