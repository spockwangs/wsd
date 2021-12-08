load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

new_local_repository(
    name = "boost",
    path = "/usr/local/",
    build_file = "boost.BUILD",
)

http_archive(
    name = "gtest",
    urls = ["https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip"],
    sha256 = "353571c2440176ded91c2de6d6cd88ddd41401d14692ec1f99e35d013feda55a",
    strip_prefix = "googletest-release-1.11.0",
)

http_archive(
    name = "gflags",
    urls = ["https://github.com/gflags/gflags/archive/refs/tags/v2.2.2.zip"],
    sha256 = "19713a36c9f32b33df59d1c79b4958434cb005b5b47dc5400a7a4b078111d9b5",
    strip_prefix = "gflags-2.2.2",
)
