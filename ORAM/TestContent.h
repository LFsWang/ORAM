#pragma once

#include "spdlog/sinks/basic_file_sink.h"
#include <memory>
#include <random>

struct TestEnvContent {
    std::mt19937_64 mt64;
    std::shared_ptr<spdlog::logger> flog;
};
