#include "experiment.h"

#include <ctime>
#include <omp.h>

#define MAKE0to14(F, ...) \
    GENFUNCA(F, 14)       \
    GENFUNCA(F, 13)       \
    GENFUNCA(F, 12)       \
    GENFUNCA(F, 11)       \
    GENFUNCA(F, 10)       \
    GENFUNCA(F, 9)        \
    GENFUNCA(F, 8)        \
    GENFUNCA(F, 7)        \
    GENFUNCA(F, 6)        \
    GENFUNCA(F, 5)        \
    GENFUNCA(F, 4)        \
    GENFUNCA(F, 3)        \
    GENFUNCA(F, 2)        \
    GENFUNCA(F, 1)        \
    GENFUNCA(F, 0)

#define GENFUNCA(FNAME, LZ)                                       \
    [[maybe_unused]] static size_t FNAME##LZ(std::size_t layer) { \
        if (layer == LZ)                                          \
            return 4;                                             \
        return 5;                                                 \
    }
MAKE0to14(Z5s)
#undef GENFUNCA

#define GENFUNCA(FNAME, LZ)                                       \
    [[maybe_unused]] static size_t FNAME##LZ(std::size_t layer) { \
        if (layer == LZ)                                          \
            return 3;                                             \
        return 4;                                                 \
    }
    MAKE0to14(Z4s)
#undef MAKE0to14

    // Init NonRecursivePathORAM with N = 32768, L = 14, TreeSize = 32768

    [[maybe_unused]] static size_t Z4uniform(std::size_t) {
    return 4;
}

[[maybe_unused]] static size_t Z3uniform(std::size_t) {
    return 3;
}

[[maybe_unused]] static size_t Z6uniform(std::size_t) {
    return 6;
}

[[maybe_unused]] static size_t Z5uniform(std::size_t) {
    return 5;
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

    spdlog::info("Step 3, start the real test for {} rounds", 10 * Billion);
    Cnt.ReportStashSizes.clear(); // reset

    // round-robin pattern
    size_t Idx = 0;
    size_t Rounds = 10 * Billion;
    for (size_t i = 0, j = 0; i < Rounds; ++i, ++j) {
        TestOram->read(Idx);
        TestOram->report();
        Idx = (Idx + 1) % N;

        if (j == Billion) {
            j = 0;
            spdlog::info("  Test progress: {} / {} ({}%)", i, Rounds, (i * 100) / Rounds);

            // Backup, report stash size distribution
            Cnt.flog->info("Stash size distribution for During test at op {}:", i);
            for (int i = 0; i < (int)Cnt.ReportStashSizes.size(); ++i) {
                Cnt.flog->info("  Stash size {}: {} times", i, Cnt.ReportStashSizes[i]);
                Cnt.flog->flush();
            }
        }
    }

    // Backup, report stash size distribution
    Cnt.flog->info("Stash size distribution finished:");
    for (size_t i = 0; i < Cnt.ReportStashSizes.size(); ++i) {
        Cnt.flog->info("  Stash size {}: {} times", i, Cnt.ReportStashSizes[i]);
    }
    Cnt.flog->flush();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> second = end_time - start_time;
    spdlog::info("Finish CheckORAM in {} seconds", second.count());
    return true;
}

void Test02() {
    spdlog::warn("Start Test02");

    std::tuple<std::string, NonRecursivePathORAM<uint32_t>::Zfunction> Zfuncs[] = {
        std::make_tuple("Z5s14", Z5s14),
        std::make_tuple("Z5s13", Z5s13),
        std::make_tuple("Z5s12", Z5s12),
        std::make_tuple("Z5s11", Z5s11),
        std::make_tuple("Z5s10", Z5s10),
        std::make_tuple("Z5s9", Z5s9),
        std::make_tuple("Z5s8", Z5s8),
        std::make_tuple("Z5s7", Z5s7),
        std::make_tuple("Z5s6", Z5s6),
        std::make_tuple("Z5s5", Z5s5),
        std::make_tuple("Z5s4", Z5s4),
        std::make_tuple("Z5s3", Z5s3),
        std::make_tuple("Z5s2", Z5s2),
        std::make_tuple("Z5s1", Z5s1),
        std::make_tuple("Z5s0", Z5s0),
        std::make_tuple("Z4uniform", Z4uniform),
        std::make_tuple("Z3uniform", Z3uniform),
        std::make_tuple("Z6uniform", Z6uniform),
        std::make_tuple("Z5uniform", Z5uniform),
        // std::make_tuple("Z4s14", Z4s14),
        // std::make_tuple("Z4s13", Z4s13),
        // std::make_tuple("Z4s12", Z4s12),
        // std::make_tuple("Z4s11", Z4s11),
        // std::make_tuple("Z4s10", Z4s10),
        // std::make_tuple("Z4s9", Z4s9),
        // std::make_tuple("Z4s8", Z4s8),
        // std::make_tuple("Z4s7", Z4s7),
        // std::make_tuple("Z4s6", Z4s6),
        // std::make_tuple("Z4s5", Z4s5),
        // std::make_tuple("Z4s4", Z4s4),
        // std::make_tuple("Z4s3", Z4s3),
        // std::make_tuple("Z4s2", Z4s2),
        // std::make_tuple("Z4s1", Z4s1),
        // std::make_tuple("Z4s0", Z4s0),
    };

    std::vector<std::mt19937> MTs(sizeof(Zfuncs) / sizeof(Zfuncs[0]));
    std::random_device rd;
    std::mt19937 base_mt;
    if (rd.entropy() != 0) {
        spdlog::info("Using random device as seed range({}-{})", rd.min(), rd.max());
        base_mt.seed(rd());
    } else {
        spdlog::error("random device is unavailable!");
        base_mt.seed(time(nullptr));
    }
    for (auto &mt : MTs) {
        mt.seed(base_mt());
    }

#pragma omp parallel for
    for (size_t i = 0; i < MTs.size(); ++i) {
        auto &[name, getZ] = Zfuncs[i];
        spdlog::info("Test Z function variant: {}", name);

        TestEnvContent Cnt;
        Cnt.mt = MTs[i];

        Cnt.ReportStashSizes.clear();
        std::string logfilename = "logs/Test2/NPathORAM_" + name + ".log"; // std::format("logs/Test1/NPathORAM_{}.log", name);
        Cnt.flog = spdlog::basic_logger_mt(logfilename, logfilename);

        NonRecursivePathORAM<uint32_t> NPathORAM(Cnt, getZ, report);
        if (!CheckORAM(&NPathORAM, 1 << 15, Cnt)) {
            spdlog::error("Test1 with Z function {} failed!", name);
        }
        Cnt.flog->flush(); // ensure all log is written
    }
}
