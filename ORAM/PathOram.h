#pragma once

#include "Base.h"
#include <bitset>
#include <functional>
#include <list>

[[maybe_unused]] static std::size_t Zuniform5(std::size_t) {
    return 5;
}

template <typename DataT>
class NonRecursivePathORAM : public ORAM<DataT> {
  public:
    using Zfunction = std::size_t (*)(size_t);
    using ReportFunc = void (*)(NonRecursivePathORAM<DataT> &);
    using ORAM<DataT>::Cnt;

    static constexpr size_t npos = size_t(-1);

    Zfunction getZ = Zuniform5;
    ReportFunc reportFunc = nullptr;

    NonRecursivePathORAM(TestEnvContent &Cnt) : ORAM<DataT>(Cnt) {};
    NonRecursivePathORAM(TestEnvContent &Cnt, Zfunction getZ, ReportFunc reportFunc) : ORAM<DataT>(Cnt), getZ(getZ), reportFunc(reportFunc) {};

    struct InnerData {
        size_t Index;
        DataT D;

        InnerData() : Index(npos), D(0) {};
        InnerData(size_t Index, DataT D) : Index(Index), D(D) {}
    };

    // size_t getZ(size_t layer) {
    //     // by paper, Z >= 4 to make the fail rate is negligible.
    //     if (layer == L)
    //         return 3;                             // leaf: Z_leaf = 3
    //     auto a = (L * 2) / (1ULL << (layer + 2)); // ≈ floor(L / 2^(layer+1))
    //     auto b = 3 + (layer % 2);                 // 3,4,3,4,3,4,...
    //     return std::max<size_t>(a, b);
    // }

    void init(size_t _N) override {
        lastMaxStashSize = 0;
        Ops = 0;

        N = _N;
        L = 0;

        // 3.2 Server Storage
        // L = ceil(log(N)) - 1
        while ((1ULL << L) < N)
            L++;
        L--;

        Stash.clear();

        // Tree with 2**L leaf, and use 1 base
        M.clear();
        M.resize(2 * (1ULL << L));

        Pos.resize(N);
        fill(Pos.begin(), Pos.end(), npos);
        UniformN = std::uniform_int_distribution<size_t>(0, (1ULL << L) - 1);

        /* Some Important Parameters */
        // auto leaves = (1ULL << L);

        unsigned long long Stor_variant = 0;
        for (size_t layer = 0; layer <= L; ++layer) {
            auto nodes = (layer == L) ? (1ULL << L) : (1ULL << layer);
            Stor_variant += nodes * getZ(layer);
        }

        auto Stor_uniform5 = ((1ULL << (L + 1)) - 1) * 5ULL;

        unsigned long long BW_blocks = 0;
        for (size_t layer = 0; layer <= L; ++layer) {
            BW_blocks += getZ(layer);
        }
        BW_blocks <<= 1;

        auto BW_uniform5 = (L + 1) * 2 * 5;

        spdlog::info("Init NonRecursivePathORAM with N = {}, L = {}, TreeSize = {}", N, L, M.size());
        spdlog::info("Total Server Storage (ours) = {} blocks", Stor_variant);
        spdlog::info("Total Server Storage (uniform Z=5) = {} blocks", Stor_uniform5);
        spdlog::info("Storage ratio (uniform/ours) = {:.4f}x", double(Stor_uniform5) / double(Stor_variant));
        spdlog::info("Storage saving (ours vs uniform) = {:.2f}%", 100.0 * (double(Stor_uniform5 - Stor_variant) / double(Stor_uniform5)));
        spdlog::info("Theoretical Bandwidth per access (ours) = {} blocks", BW_blocks);
        spdlog::info("Theoretical Bandwidth per access (uniform Z=5) = {} blocks", BW_uniform5);
        spdlog::info("Bandwidth saving (ours vs uniform) = {:.2f}%", 100.0 * (double(BW_uniform5 - BW_blocks) / double(BW_uniform5)));
        // spdlog::info("DataT size = {} bytes", sizeof(DataT));
    }

    DataT read(size_t index) override {
        Ops++;
        if (index >= N) [[unlikely]] {
            spdlog::error("PlainMemory read overflow read: {} ! ", index);
        }

        DataT Ret;
        Access(index, false, Ret); // false => this is a read
        return Ret;
    }

    void write(size_t index, DataT Val) override {
        Ops++;
        if (index >= N) [[unlikely]] {
            spdlog::error("PlainMemory read overflow read: {} ! ", index);
        }
        Access(index, true, Val); // true => this is a write
    }

    void dump() {
        spdlog::error("Pos Map");
        for (size_t i = 0; i < N; ++i) {
            spdlog::info(" Pos[{}]={}", i, Pos[i]);
        }
        spdlog::error("Dump tree");
        for (size_t i = 0; i < M.size(); ++i) {
            spdlog::warn("  Node {}", i);
            for (auto [id, v] : M[i])
                spdlog::info("       ind {}, val", id, v);
        }

        spdlog::error("Dump Stash");
        for (auto [id, v] : Stash)
            spdlog::info("       ind {}, val", id, v);
    }

    void report() override {
        if (reportFunc != nullptr) {
            reportFunc(*this);
        }
    }

    std::size_t stash_size() override {
        return Stash.size();
    }

    void Access(size_t Idx, bool NeedWrite, DataT &Val) {
        size_t Path = Pos[Idx];
        if (Path == npos) [[unlikely]] {
            // First time meet
            Path = UniformN(Cnt.mt);
            Stash.emplace_back(Idx, 0);
        }
        Pos[Idx] = UniformN(Cnt.mt);

        std::bitset<64> PathBit(Path);
        std::size_t TreeId = 1;
        for (std::size_t layer = 0; layer <= L; ++layer) {
            for (auto &D : M[TreeId]) {
                Stash.emplace_back(D);
            }
            M[TreeId].clear();

            if (layer != L)
                TreeId = TreeId * 2 + PathBit[layer];
        }

        bool find = false;
        for (auto &[I, D] : Stash) {
            if (I == Idx) {
                if (NeedWrite)
                    D = Val;
                else
                    Val = D;
                find = true;
            }
        }
        if (!find) {
            spdlog::warn("can not found the data from index {}", Idx);
        }

        for (size_t layer = L; layer != npos; --layer) {
            auto eq = [&](size_t Pos) -> bool {
                for (int i = 0; i < int(layer); ++i)
                    if (PathBit[i] != (Pos >> i & 1))
                        return false;
                return true;
            };
            auto Ptr = Stash.begin();
            while (Ptr != Stash.end() && M[TreeId].size() < getZ(layer)) {
                if (eq(Pos[Ptr->Index])) {
                    M[TreeId].emplace_back(*Ptr);
                    Ptr = Stash.erase(Ptr);
                } else {
                    Ptr++;
                }
            }
            TreeId >>= 1;
        }

        // In paper, the Stash is 0 at almost time.
        // Let see some large case
        // if (Stash.size() > lastMaxStashSize) {
        //     // It will updated by report
        //     // lastMaxStashSize = Stash.size();
        //     spdlog::trace("Stash size = {:>5}, opnum = {}", Stash.size(), Ops);
        // }
    }

    std::uniform_int_distribution<size_t> UniformN;
    std::vector<size_t> Pos;
    std::list<InnerData> Stash;
    std::vector<std::vector<InnerData>> M;
    size_t N;
    size_t L;
    // size_t Z;
    size_t lastMaxStashSize;
    size_t Ops;
};
