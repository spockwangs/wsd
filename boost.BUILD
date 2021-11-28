cc_library(
    name = "boost",
    hdrs = glob(["include/boost/**"]),
    strip_include_prefix = "include",
    srcs = glob(["lib/libboost_thread-mt.a", "lib/libboost_system-mt.a"]),
    visibility = ["//visibility:public"],
)
