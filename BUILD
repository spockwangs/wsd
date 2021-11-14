cc_library(
    name = "wsd",
    srcs = glob(["src/*.cc"]) + glob(["include/detail/**/*.h"]),
    hdrs = glob(["include/*.h"]),
    #include_prefix = "wsd",
    strip_include_prefix = "include",
    deps = [
        "@boost//:boost",
    ],
    copts = [
        "-std=c++11",
        "-Iinclude",
    ],
    visibility = [ "//visibility:public" ],
)
