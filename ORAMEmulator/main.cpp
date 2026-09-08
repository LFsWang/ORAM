#include "Experiment/experiment.h"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

namespace {

void PrintUsage(const char *program) {
    std::cout
        << "Usage: " << program << " --experiment NAME [options]\n\n"
        << "Experiments:\n"
        << "  self-test     Correctness and speed smoke test\n"
        << "  uniform-z     Uniform Z=3,4,5,6 dataset\n"
        << "  single-layer  Z=5 with one layer reduced to 4 (S=0..14)\n"
        << "  leaf-size     Z=5 with leaf capacity 0..3\n"
        << "  all           Run all datasets in the order above\n\n"
        << "Options:\n"
        << "  --profile quick|full  Default: quick\n"
        << "  --seed N              Default: 0\n"
        << "  --output DIR          Default: logs\n"
        << "  --confirm-full        Required with --profile full\n"
        << "  --help                Show this message\n\n"
        << "The full profile performs billions of accesses and can take hours or\n"
        << "days. It never starts unless --confirm-full is supplied explicitly.\n";
}

ExperimentConfig QuickConfig() {
    return ExperimentConfig{};
}

ExperimentConfig FullConfig() {
    ExperimentConfig config;
    config.correctness_rounds = 10'000'000;
    config.speed_rounds = 10'000'000;
    config.warmup_rounds = 1'000'000'000;
    config.test_rounds = 10'000'000'000;
    return config;
}

} // namespace

int main(int argc, char **argv) {
    std::string experiment;
    std::string profile = "quick";
    std::string output = "logs";
    std::uint32_t seed = 0;
    bool confirmFull = false;

    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--help") {
                PrintUsage(argv[0]);
                return 0;
            }
            if (arg == "--confirm-full") {
                confirmFull = true;
                continue;
            }
            if (i + 1 >= argc) {
                std::cerr << "Missing value after " << arg << "\n";
                PrintUsage(argv[0]);
                return 2;
            }
            const std::string value = argv[++i];
            if (arg == "--experiment")
                experiment = value;
            else if (arg == "--profile")
                profile = value;
            else if (arg == "--output")
                output = value;
            else if (arg == "--seed")
                seed = static_cast<std::uint32_t>(std::stoul(value));
            else {
                std::cerr << "Unknown option: " << arg << "\n";
                PrintUsage(argv[0]);
                return 2;
            }
        }
    } catch (const std::exception &error) {
        std::cerr << "Invalid command line: " << error.what() << "\n";
        return 2;
    }

    if (experiment.empty()) {
        PrintUsage(argv[0]);
        return 0;
    }

    ExperimentConfig config;
    if (profile == "quick") {
        config = QuickConfig();
    } else if (profile == "full") {
        if (!confirmFull) {
            std::cerr << "Refusing to start the full profile without --confirm-full.\n";
            return 2;
        }
        config = FullConfig();
    } else {
        std::cerr << "Unknown profile: " << profile << "\n";
        return 2;
    }

    config.seed = seed;
    config.output_root = output;

    spdlog::set_level(spdlog::level::info);
    spdlog::info(
        "profile={}, experiment={}, N={}, seed={}, correctness_rounds={}, "
        "speed_rounds={}, warmup_rounds={}, test_rounds={}",
        profile, experiment, config.memory_size, config.seed,
        config.correctness_rounds, config.speed_rounds,
        config.warmup_rounds, config.test_rounds);

    return RunNamedExperiment(experiment, config) ? 0 : 1;
}
