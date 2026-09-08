#include "experiment.h"

#include <atomic>
#include <filesystem>
#include <string>
#include <tuple>

namespace {

using PathORAM = NonRecursivePathORAM<uint32_t>;

size_t Z6uniform(std::size_t) { return 6; }
size_t Z5uniform(std::size_t) { return 5; }
size_t Z4uniform(std::size_t) { return 4; }
size_t Z3uniform(std::size_t) { return 3; }

void Report(PathORAM &oram) {
    auto &content = oram.Cnt;
    if (oram.Stash.size() >= content.ReportStashSizes.size())
        content.ReportStashSizes.resize(oram.Stash.size() + 1, 0);
    content.ReportStashSizes[oram.Stash.size()]++;
}

bool RunVariant(PathORAM::Zfunction getZ, const std::string &dataset,
                const std::string &variant, const ExperimentConfig &config) {
    const auto outputDir = std::filesystem::path(config.output_root) / dataset;
    std::filesystem::create_directories(outputDir);
    const auto logPath = outputDir / (variant + ".log");
    const std::string loggerName = dataset + ":" + variant;

    TestEnvContent content;
    content.mt.seed(config.seed);
    content.flog = spdlog::basic_logger_mt(loggerName, logPath.string(), true);
    content.flog->info(
        "metadata dataset={} variant={} N={} seed={} warmup_rounds={} test_rounds={}",
        dataset, variant, config.memory_size, config.seed,
        config.warmup_rounds, config.test_rounds);

    PathORAM oram(content, getZ, Report);
    oram.init(config.memory_size);
    std::uniform_int_distribution<size_t> uniformIndex(0, config.memory_size - 1);

    spdlog::info("{} / {}: initialize {} blocks", dataset, variant,
                 config.memory_size);
    for (size_t i = 0; i < config.memory_size; ++i)
        oram.write(i, content.mt());

    spdlog::info("{} / {}: warm up for {} accesses", dataset, variant,
                 config.warmup_rounds);
    content.ReportStashSizes.clear();
    for (std::uint64_t i = 0; i < config.warmup_rounds; ++i) {
        oram.read(uniformIndex(content.mt));
        oram.report();
    }

    content.flog->info("Stash size distribution after warm-up:");
    for (size_t i = 0; i < content.ReportStashSizes.size(); ++i)
        content.flog->info("  Stash size {}: {} times", i,
                           content.ReportStashSizes[i]);
    content.flog->flush();

    spdlog::info("{} / {}: measure {} round-robin accesses", dataset,
                 variant, config.test_rounds);
    content.ReportStashSizes.clear();
    size_t index = 0;
    for (std::uint64_t i = 0; i < config.test_rounds; ++i) {
        oram.read(index);
        oram.report();
        index = (index + 1) % config.memory_size;
    }

    content.flog->info("Stash size distribution finished:");
    for (size_t i = 0; i < content.ReportStashSizes.size(); ++i)
        content.flog->info("  Stash size {}: {} times", i,
                           content.ReportStashSizes[i]);
    content.flog->flush();
    spdlog::drop(loggerName);

    spdlog::info("{} / {}: wrote {}", dataset, variant, logPath.string());
    return true;
}

} // namespace

bool TestUniformZ(const ExperimentConfig &config) {
    spdlog::info("Start dataset uniform-z");
    const std::tuple<const char *, PathORAM::Zfunction> variants[] = {
        {"z3", Z3uniform},
        {"z4", Z4uniform},
        {"z5", Z5uniform},
        {"z6", Z6uniform},
    };

    std::atomic<bool> passed{true};
    constexpr std::size_t variantCount = sizeof(variants) / sizeof(variants[0]);

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
    for (std::int64_t i = 0;
         i < static_cast<std::int64_t>(variantCount); ++i) {
        const auto &[name, getZ] = variants[static_cast<std::size_t>(i)];
        if (!RunVariant(getZ, "uniform-z", name, config))
            passed.store(false);
    }
    return passed.load();
}
