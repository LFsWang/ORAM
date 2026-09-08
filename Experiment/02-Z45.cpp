#include "experiment.h"

#include <atomic>
#include <filesystem>
#include <string>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace {

using PathORAM = NonRecursivePathORAM<uint32_t>;
constexpr size_t PaperLeafLayer = 14;

template <size_t Layer>
size_t Z5SingleLayer(std::size_t layer) {
    return layer == Layer ? 4 : 5;
}

template <size_t Capacity>
size_t Z5LeafCapacity(std::size_t layer) {
    return layer == PaperLeafLayer ? Capacity : 5;
}

struct Variant {
    const char *name;
    PathORAM::Zfunction getZ;
};

void Report(PathORAM &oram) {
    auto &content = oram.Cnt;
    if (oram.Stash.size() >= content.ReportStashSizes.size())
        content.ReportStashSizes.resize(oram.Stash.size() + 1, 0);
    content.ReportStashSizes[oram.Stash.size()]++;
}

bool RunVariant(const Variant &variant, const std::string &dataset,
                const ExperimentConfig &config) {
    const auto outputDir = std::filesystem::path(config.output_root) / dataset;
    const auto logPath = outputDir / (std::string(variant.name) + ".log");
    const std::string loggerName = dataset + ":" + variant.name;

    TestEnvContent content;
    content.mt.seed(config.seed);
    content.flog = spdlog::basic_logger_mt(loggerName, logPath.string(), true);
    content.flog->info(
        "metadata dataset={} variant={} N={} seed={} warmup_rounds={} test_rounds={}",
        dataset, variant.name, config.memory_size, config.seed,
        config.warmup_rounds, config.test_rounds);

    PathORAM oram(content, variant.getZ, Report);
    oram.init(config.memory_size);
    std::uniform_int_distribution<size_t> uniformIndex(0, config.memory_size - 1);

    spdlog::info("{} / {}: initialize {} blocks", dataset, variant.name,
                 config.memory_size);
    for (size_t i = 0; i < config.memory_size; ++i)
        oram.write(i, content.mt());

    spdlog::info("{} / {}: warm up for {} accesses", dataset, variant.name,
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
                 variant.name, config.test_rounds);
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

    spdlog::info("{} / {}: wrote {}", dataset, variant.name,
                 logPath.string());
    return true;
}

bool RunVariants(const std::string &dataset,
                 const std::vector<Variant> &variants,
                 const ExperimentConfig &config) {
    if (config.memory_size != (1U << 15)) {
        spdlog::error("{} requires N=2^15 because paper layers are numbered 0..14",
                      dataset);
        return false;
    }

    std::filesystem::create_directories(
        std::filesystem::path(config.output_root) / dataset);
    std::atomic<bool> passed{true};

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
    for (std::int64_t i = 0;
         i < static_cast<std::int64_t>(variants.size()); ++i) {
        if (!RunVariant(variants[static_cast<size_t>(i)], dataset, config))
            passed.store(false);
    }
    return passed.load();
}

} // namespace

bool TestSingleLayer(const ExperimentConfig &config) {
    spdlog::info("Start dataset single-layer");
    const std::vector<Variant> variants = {
        {"s0", Z5SingleLayer<0>},
        {"s1", Z5SingleLayer<1>},
        {"s2", Z5SingleLayer<2>},
        {"s3", Z5SingleLayer<3>},
        {"s4", Z5SingleLayer<4>},
        {"s5", Z5SingleLayer<5>},
        {"s6", Z5SingleLayer<6>},
        {"s7", Z5SingleLayer<7>},
        {"s8", Z5SingleLayer<8>},
        {"s9", Z5SingleLayer<9>},
        {"s10", Z5SingleLayer<10>},
        {"s11", Z5SingleLayer<11>},
        {"s12", Z5SingleLayer<12>},
        {"s13", Z5SingleLayer<13>},
        {"s14", Z5SingleLayer<14>},
    };
    return RunVariants("single-layer", variants, config);
}

bool TestLeafSize(const ExperimentConfig &config) {
    spdlog::info("Start dataset leaf-size");
    // leaf4 is single-layer/s14; leaf5 is uniform-z/z5.
    const std::vector<Variant> variants = {
        {"leaf0", Z5LeafCapacity<0>},
        {"leaf1", Z5LeafCapacity<1>},
        {"leaf2", Z5LeafCapacity<2>},
        {"leaf3", Z5LeafCapacity<3>},
    };
    return RunVariants("leaf-size", variants, config);
}
