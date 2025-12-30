#include <bits/stdc++.h>
#include <omp.h>

#include "ORAM/ORAM.h"
#include "ORAM/TestContent.h"

using namespace std;

#ifdef PASSCHECK
#define PS(X)
#warning "PASSCHECK is defined, so the reference ORAM will not be used."
#else
#define PS(X) X
#endif

bool CheckORAM(ORAM<uint64_t> *TestOram, size_t N, int64_t Rounds, TestEnvContent &Cnt) {
    spdlog::info("Start CheckORAM");

    PS(PlainMemory<uint64_t> AnwserOram(Cnt);)

    PS(AnwserOram.init(N);)
    TestOram->init(N);

    std::uniform_int_distribution<size_t> UniformN(0, N - 1);
    std::bernoulli_distribution pWrite(0.5);

    auto start_time = std::chrono::high_resolution_clock::now();
    while (Rounds--) {
        if (pWrite(Cnt.mt64)) {
            auto Val = Cnt.mt64();
            auto Ind = UniformN(Cnt.mt64);

            PS(AnwserOram.write(Ind, Val);)
            TestOram->write(Ind, Val);
        } else {
            auto Ind = UniformN(Cnt.mt64);
#ifdef PASSCHECK
            TestOram->read(Ind);
#else
            if (AnwserOram.read(Ind) != TestOram->read(Ind)) {
                spdlog::error("GG, value different!"); // LOL
                return false;
            }

#endif
        }
        if (TestOram->report()) {
            spdlog::info("ORAM terminated by report function.");
            return false;
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> second = end_time - start_time;
    spdlog::info("Finish CheckORAM in {} seconds", second.count());
    return true;
}

void Test1() {
#ifdef TEST1_DISABLED
    spdlog::info("Test1 is disabled.");
    return;
#endif
    spdlog::info("Start Test1");

    // Name, Rounds, Zfunction
    std::tuple<std::string, std::size_t, NonRecursivePathORAM<uint64_t>::Zfunction> Zfuncs[] = {
        // Because Z5 is very slow, so we split it to 5 variants to run in parallel
        // Ryzeon 7900 has 12 cores, so 10 threads is ok.

        // std::make_tuple("Uniform5", 100, [](std::size_t) { return 5; }),
        std::make_tuple("Uniform5-1", 10, [](std::size_t) { return 5; }),
        std::make_tuple("Uniform5-2", 10, [](std::size_t) { return 5; }),
        std::make_tuple("Uniform5-3", 10, [](std::size_t) { return 5; }),
        std::make_tuple("Uniform5-4", 10, [](std::size_t) { return 5; }),
        std::make_tuple("Uniform5-5", 10, [](std::size_t) { return 5; }),
        std::make_tuple("Uniform5-6", 10, [](std::size_t) { return 5; }),
        std::make_tuple("Uniform5-7", 10, [](std::size_t) { return 5; }),
        std::make_tuple("Uniform5-8", 10, [](std::size_t) { return 5; }),
        std::make_tuple("Uniform5-9", 10, [](std::size_t) { return 5; }),
        std::make_tuple("Uniform5-10", 10, [](std::size_t) { return 5; }),

        std::make_tuple("Uniform4", 100, [](std::size_t) { return 4; }),
        std::make_tuple("Uniform3", 100, [](std::size_t) { return 3; }),
    };
    size_t ZfuncSize = sizeof(Zfuncs) / sizeof(Zfuncs[0]);
    vector<mt19937_64> MTs(ZfuncSize);

    random_device rd;
    mt19937_64 mt64;
    if (rd.entropy() != 0) {
        spdlog::info("Using random device as seed range({}-{})", rd.min(), rd.max());
        mt64.seed(rd());
    } else {
        spdlog::error("random device is unavailable!");
        mt64.seed(time(nullptr));
    }
    for (size_t zi = 0; zi < ZfuncSize; ++zi) {
        MTs[zi].seed(mt64());
    }

    auto reportFunc = [](NonRecursivePathORAM<uint64_t> &oram) {
        if (oram.Stash.size() > oram.lastMaxStashSize) {
            oram.lastMaxStashSize = oram.Stash.size();
            if (oram.Stash.size() == 15) {
                oram.Cnt.flog->info("Stash size = {:>5}, opnum = {}", oram.Stash.size(), oram.Ops);
                oram.Cnt.flog->flush();
                spdlog::trace("Stash size = {:>5}, opnum = {}", oram.Stash.size(), oram.Ops);
                return true;
            }
            // spdlog::trace("Stash size = {:>5}, opnum = {}", oram.Stash.size(), oram.Ops);
        }
        return false;
    };

#pragma omp parallel for
    for (size_t zi = 0; zi < ZfuncSize; ++zi) {
        auto [name, rounds, getZ] = Zfuncs[zi];
        spdlog::info("Test Z function variant: {}", name);

        string logfilename = std::format("logs/Test1/NPathORAM_{}.log", name);

        TestEnvContent Cnt;
        Cnt.mt64 = MTs[zi];
        Cnt.flog = spdlog::basic_logger_mt(logfilename, logfilename, true);
        Cnt.flog->set_level(spdlog::level::trace);

        while (rounds--) {
            spdlog::info("Starting round {}, Z function {}", 1'000'000'000'000, name);
            NonRecursivePathORAM<uint64_t> NPathORAM(Cnt, getZ, reportFunc);
            if (CheckORAM(&NPathORAM, 1 << 20, 1'000'000'000, Cnt)) {
                /*We assume CheckORAM returns true on failure, because it need terminate early*/
                spdlog::error("Test1 with Z function {} failed!", name);
            }
        }
    }
}

int main() {
    spdlog::set_level(spdlog::level::trace);

    Test1();

    return 0;
}