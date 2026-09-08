#include "experiment.h"

// Check the correctness of an ORAM implementation
static bool CheckORAM(ORAM<uint32_t> *TestOram, size_t N, std::uint64_t Rounds, TestEnvContent &Cnt) {
    spdlog::info("Start CheckORAM");
    PlainMemory<uint32_t> AnwserOram(Cnt);

    AnwserOram.init(N);
    TestOram->init(N);

    std::uniform_int_distribution<size_t> UniformN(0, N - 1);
    std::bernoulli_distribution pWrite(0.5);

    auto start_time = std::chrono::high_resolution_clock::now();
    while (Rounds--) {
        if (pWrite(Cnt.mt)) {
            auto Val = Cnt.mt();
            auto Ind = UniformN(Cnt.mt);

            AnwserOram.write(Ind, Val);
            TestOram->write(Ind, Val);
        } else {
            auto Ind = UniformN(Cnt.mt);
            if (AnwserOram.read(Ind) != TestOram->read(Ind)) {
                spdlog::error("GG, value different!"); // LOL
                return false;
            }
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> second = end_time - start_time;
    spdlog::info("Finish CheckORAM in {} seconds", second.count());
    return true;
}

static bool CheckORAMSpeed(ORAM<uint32_t> *TestOram, size_t N, std::uint64_t Rounds, TestEnvContent &Cnt) {
    spdlog::info("Start ORAM Speed");
    TestOram->init(N);

    std::uniform_int_distribution<size_t> UniformN(0, N - 1);
    std::bernoulli_distribution pWrite(0.5);

    auto start_time = std::chrono::high_resolution_clock::now();
    while (Rounds--) {
        if (pWrite(Cnt.mt)) {
            auto Val = Cnt.mt();
            auto Ind = UniformN(Cnt.mt);

            TestOram->write(Ind, Val);
        } else {
            auto Ind = UniformN(Cnt.mt);
            TestOram->read(Ind);
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> second = end_time - start_time;
    spdlog::info("Finish CheckORAMSpeed in {} seconds", second.count());
    return true;
}

bool TestSelf(const ExperimentConfig &config) {
    spdlog::info("Start self-test");

    TestEnvContent Cnt;
    Cnt.mt.seed(config.seed);
    bool passed = true;
    {
        NonRecursivePathORAM<uint32_t> NPathORAM(Cnt);
        if (!CheckORAM(&NPathORAM, config.memory_size, config.correctness_rounds, Cnt)) {
            spdlog::error("NonRecursivePathORAM failed!");
            passed = false;
        }
    }
    {
        Cnt.mt.seed(config.seed);
        NonRecursivePathORAM<uint32_t> NPathORAM(Cnt);
        if (!CheckORAMSpeed(&NPathORAM, config.memory_size, config.speed_rounds, Cnt)) {
            spdlog::error("NonRecursivePathORAM Speed test failed!");
            passed = false;
        }
    }
    return passed;
}
