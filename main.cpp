#include <bits/stdc++.h>

#include "ORAM/ORAM.h"
#include "ORAM/TestContent.h"

using namespace std;

bool CheckORAM(ORAM<uint64_t> *TestOram, size_t N, int Rounds, TestEnvContent &Cnt) {
    spdlog::info("Start CheckORAM");

    PlainMemory<uint64_t> AnwserOram(Cnt);

    AnwserOram.init(N);
    TestOram->init(N);

    std::uniform_int_distribution<size_t> UniformN(0, N - 1);
    std::bernoulli_distribution pWrite(0.5);

    auto start_time = std::chrono::high_resolution_clock::now();
    while (Rounds--) {
        if (pWrite(Cnt.mt64)) {
            auto Val = Cnt.mt64();
            auto Ind = UniformN(Cnt.mt64);

            AnwserOram.write(Ind, Val);
            TestOram->write(Ind, Val);
        } else {
            auto Ind = UniformN(Cnt.mt64);
            if (AnwserOram.read(Ind) != TestOram->read(Ind)) {
                spdlog::error("GG, value different!"); // LOL
                return false;
            }
        }
        TestOram->report();
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    return true;
}

int main() {
    spdlog::set_level(spdlog::level::trace);

    TestEnvContent Cnt;
    Cnt.flog = spdlog::basic_logger_mt("log", "logs/log.log");
    Cnt.flog->set_level(spdlog::level::trace);

    random_device rd;
    if (rd.entropy() != 0) {
        spdlog::info("Using random device as seed range({}-{})", rd.min(), rd.max());
        Cnt.mt64.seed(rd());
    } else {
        spdlog::error("random device is unavailable!");
        Cnt.mt64.seed(time(nullptr));
    }

    spdlog::info("Start NPathORAM Test");
    int RoundCnt = 0;
    while (true) {
        spdlog::info("Test NonRecursivePathORAM... round {}", ++RoundCnt);

        NonRecursivePathORAM<uint64_t> NPathORAM(Cnt);
        if (!CheckORAM(&NPathORAM, 1 << 20, 1'000'000, Cnt)) {
            // NPathORAM.dump();
            break;
        }
    }
    return 0;
}