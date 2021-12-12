load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

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

new_local_repository(
    name = "folly",
    path = "/private/var/folders/pr/d3c459bx6ys95bv1qfrtr1_m0000gn/T/fbcode_builder_getdeps-ZUsersZspockZDownloadsZfolly-2021.12.06.00ZbuildZfbcode_builder/installed/folly",
    build_file = "folly.BUILD",
)

http_archive(
    name = "glog",
    urls = ["https://github.com/google/glog/archive/refs/tags/v0.5.0.zip"],
    strip_prefix = "glog-0.5.0",
    repo_mapping = {"@com_github_gflags_gflags" : "@gflags"}
)
