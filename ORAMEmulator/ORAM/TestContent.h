#pragma once

#include "spdlog/sinks/basic_file_sink.h"
#include <memory>
#include <random>

struct TestEnvContent {
    std::mt19937 mt;
    std::shared_ptr<spdlog::logger> flog;

    std::vector<uint64_t> ReportStashSizes;
};
