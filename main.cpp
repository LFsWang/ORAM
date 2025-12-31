#include <bits/stdc++.h>

#include "ORAM/ORAM.h"
#include "ORAM/TestContent.h"
#include "Experiment/experiment.h"

using namespace std;

// void Test1() {
// #ifdef TEST1_DISABLED
//     spdlog::info("Test1 is disabled.");
//     return;
// #endif
//     spdlog::info("Start Test1");

//     // Name, Rounds, Zfunction
//     std::tuple<std::string, std::size_t, NonRecursivePathORAM<uint64_t>::Zfunction> Zfuncs[] = {
//         std::make_tuple("Uniform6", 10000, [](std::size_t) { return 5; }),
//         std::make_tuple("Uniform5", 10000, [](std::size_t) { return 5; }),
//         std::make_tuple("Uniform4", 10000, [](std::size_t) { return 4; }),
//         std::make_tuple("Uniform3", 10000, [](std::size_t) { return 3; }),
//     };
//     size_t ZfuncSize = sizeof(Zfuncs) / sizeof(Zfuncs[0]);
//     vector<mt19937_64> MTs(ZfuncSize);

//     random_device rd;
//     mt19937_64 mt64;
//     if (rd.entropy() != 0) {
//         spdlog::info("Using random device as seed range({}-{})", rd.min(), rd.max());
//         mt64.seed(rd());
//     } else {
//         spdlog::error("random device is unavailable!");
//         mt64.seed(time(nullptr));
//     }

//     for (size_t zi = 0; zi < ZfuncSize; ++zi) {
//         MTs[zi].seed(mt64());
//     }

// #pragma omp parallel for
//     for (size_t zi = 0; zi < ZfuncSize; ++zi) {
//         auto [name, rounds, getZ] = Zfuncs[zi];
//         spdlog::info("Test Z function variant: {}", name);

//         string logfilename = std::format("logs/Test1/NPathORAM_{}.log", name);
//         vector<size_t> records;

//         auto reportFunc = [&records](NonRecursivePathORAM<uint64_t> &oram) {
//             if (records.size () < oram.Stash.size() + 1) {
//                 records.resize(oram.Stash.size() + 1, 0);
//             }
//             records[oram.Stash.size()]++;
//             // if (oram.Stash.size() > oram.lastMaxStashSize) {
//             //     oram.Cnt.flog->info("Stash size = {:>5}, opnum = {}", oram.Stash.size(), oram.Ops);
//             //     oram.lastMaxStashSize = oram.Stash.size();
//             // }
//             return false;
//         };

//         TestEnvContent Cnt;
//         Cnt.mt64 = MTs[zi];
//         Cnt.flog = spdlog::basic_logger_mt(logfilename, logfilename, true);
//         Cnt.flog->set_level(spdlog::level::trace);

//         for (size_t Run = 1; Run <= rounds; ++Run) {
//             spdlog::info("Run {} / {} for Z function {}", Run, rounds, name);
//             NonRecursivePathORAM<uint64_t> NPathORAM(Cnt, getZ, reportFunc);
//             if (!CheckORAM(&NPathORAM, 1 << 15, 10'000'000, Cnt)) {
//                 spdlog::error("Test1 with Z function {} failed!", name);
//             }
//         }

//         Cnt.flog->info("Stash size distribution for {} in {} runs:", name, rounds);
//         for (int i = 0; i < (int)records.size(); ++i) {
//             Cnt.flog->info("Stash size = {:>5}, num = {}", i, records[i]);
//         }
//     }
// }

int main() {
    spdlog::set_level(spdlog::level::trace);

    RunAllTests();
    // Test0();
//    Test1();

    return 0;
}