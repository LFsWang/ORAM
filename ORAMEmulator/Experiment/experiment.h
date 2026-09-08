#pragma once

#include <spdlog/spdlog.h>

#include "../ORAM/ORAM.h"
#include "../ORAM/TestContent.h"

#include <cstddef>
#include <cstdint>
#include <string>

struct ExperimentConfig {
    std::size_t memory_size = 1U << 15;
    std::uint64_t correctness_rounds = 100'000;
    std::uint64_t speed_rounds = 100'000;
    std::uint64_t warmup_rounds = 10'000;
    std::uint64_t test_rounds = 100'000;
    std::uint32_t seed = 0;
    std::string output_root = "logs";
};

bool TestSelf(const ExperimentConfig &config);
bool TestUniformZ(const ExperimentConfig &config);
bool TestSingleLayer(const ExperimentConfig &config);
bool TestLeafSize(const ExperimentConfig &config);

bool RunNamedExperiment(const std::string &name, const ExperimentConfig &config);
