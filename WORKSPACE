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

http_archive(
    name = "glog",
    urls = ["https://github.com/google/glog/archive/refs/tags/v0.5.0.zip"],
    strip_prefix = "glog-0.5.0",
    repo_mapping = { "@com_github_gflags_gflags" : "@gflags" },
)

http_archive(
    name = "google_benchmark",
    urls = ["https://github.com/google/benchmark/archive/refs/tags/v1.6.0.zip"],
    sha256 = "3da225763533aa179af8438e994842be5ca72e4a7fed4d7976dc66c8c4502f58",
    strip_prefix = "benchmark-1.6.0",
)

http_archive(
    name = "google_absl",
    urls = ["https://github.com/abseil/abseil-cpp/archive/refs/tags/20211102.0.zip"],
    sha256 = "a4567ff02faca671b95e31d315bab18b42b6c6f1a60e91c6ea84e5a2142112c2",
    strip_prefix = "abseil-cpp-20211102.0",
)
