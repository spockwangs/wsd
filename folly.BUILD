cc_library(
    name = "folly",
    hdrs = glob(["include/folly/**"]),
    strip_include_prefix = "include",
    srcs = glob(["lib/libfolly.a"]),
    deps = [
        "@glog//:glog",
    ],
    copts = [
        "-std=c++14",
    ],
    linkopts = [
        "-ldouble-conversion",
    ],
    visibility = ["//visibility:public"],
)
