#include "experiment.h"

[[maybe_unused]] static size_t Z6uniform(std::size_t) {
    return 6;
}
[[maybe_unused]] static size_t Z5uniform(std::size_t) {
    return 5;
}
[[maybe_unused]] static size_t Z4uniform(std::size_t) {
    return 4;
}
[[maybe_unused]] static size_t Z3uniform(std::size_t) {
    return 3;
}

static void report(NonRecursivePathORAM<uint32_t> &oram) {
    auto &Cnt = oram.Cnt;
    if (oram.Stash.size() >= Cnt.ReportStashSizes.size()) {
        Cnt.ReportStashSizes.resize(oram.Stash.size() + 1, 0);
    }
    Cnt.ReportStashSizes[oram.Stash.size()]++;
}

static bool CheckORAM(ORAM<uint32_t> *TestOram, size_t N, TestEnvContent &Cnt) {
    spdlog::info("Start ORAM Speed");
    TestOram->init(N);

    std::uniform_int_distribution<size_t> UniformN(0, N - 1);
    std::bernoulli_distribution pWrite(0.5);

    auto start_time = std::chrono::high_resolution_clock::now();
    constexpr size_t Billion = 1000000000;

    spdlog::info("Step 1, insert index 0 to N-1");
    for (size_t i = 0; i < N; ++i) {
        TestOram->write(i, Cnt.mt());
    }
    spdlog::info("Step 2, random read/write warming-up for {} rounds", Billion);
    Cnt.ReportStashSizes.clear(); // reset
    for (size_t i = 0, j = 0; i < Billion; ++i, ++j) {
        auto Ind = UniformN(Cnt.mt);
        TestOram->read(Ind);
        TestOram->report();

        if (j == 10000000) {
            j = 0;
            spdlog::info("  Warm-up progress: {} / {} ({}%)", i, Billion, (i * 100) / Billion);
        }

        // if (i > 40000000) { // For debugging purpose, skip the long test
        //     spdlog::debug("short cut warm-up after 40 million ops");
        //     break;
        // }
    }

    // Backup, report stash size distribution
    Cnt.flog->info("Stash size distribution for During warm-up:");
    for (size_t i = 0; i < Cnt.ReportStashSizes.size(); ++i) {
        Cnt.flog->info("  Stash size {}: {} times", i, Cnt.ReportStashSizes[i]);
    }
    Cnt.flog->flush();

    // spdlog::debug("short cut warm-up after 40 million ops");
    // return true; // For debugging purpose, skip the long test

    spdlog::info("Step 3, start the real test for {} rounds", 10 * Billion);
    Cnt.ReportStashSizes.clear(); // reset

    // round-robin pattern
    size_t Idx = 0;
    for (size_t i = 0, j = 0; i < 10 * Billion; ++i, ++j) {
        TestOram->read(Idx);
        TestOram->report();
        Idx = (Idx + 1) % N;

        if (j == Billion) {
            j = 0;
            spdlog::info("  Test progress: {} / {} ({}%)", i, 10 * Billion, (i * 100) / (10 * Billion));

            // Backup, report stash size distribution
            Cnt.flog->info("Stash size distribution for During test at op {}:", i);
            for (int i = 0; i < (int)Cnt.ReportStashSizes.size(); ++i) {
                Cnt.flog->info("  Stash size {}: {} times", i, Cnt.ReportStashSizes[i]);
                Cnt.flog->flush();
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> second = end_time - start_time;
    spdlog::info("Finish CheckORAM in {} seconds", second.count());
    return true;
}

void Test01() {
    spdlog::info("Start Test01");

    std::tuple<std::string, NonRecursivePathORAM<uint32_t>::Zfunction> Zfuncs[] = {
        std::make_tuple("Uniform6", Z6uniform),
        std::make_tuple("Uniform5", Z5uniform),
        std::make_tuple("Uniform4", Z4uniform),
        std::make_tuple("Uniform3", Z3uniform),
    };

    for (auto &[name, getZ] : Zfuncs) {
        spdlog::info("Test Z function variant: {}", name);

        TestEnvContent Cnt;
        Cnt.mt.seed(0);

        Cnt.ReportStashSizes.clear();
        std::string logfilename = "logs/Test1/NPathORAM_" + name + ".log"; // std::format("logs/Test1/NPathORAM_{}.log", name);
        Cnt.flog = spdlog::basic_logger_mt(logfilename, logfilename);

        NonRecursivePathORAM<uint32_t> NPathORAM(Cnt, getZ, report);
        if (!CheckORAM(&NPathORAM, 1 << 15, Cnt)) {
            spdlog::error("Test1 with Z function {} failed!", name);
        }

        Cnt.flog->info("Stash size distribution for {}:", name);
        for (int i = 0; i < (int)Cnt.ReportStashSizes.size(); ++i) {
            Cnt.flog->info("  Stash size {}: {} times", i, Cnt.ReportStashSizes[i]);
        }
    }
}
