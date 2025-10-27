#include <bits/stdc++.h>
#include "ORAM.h"

using namespace std;

std::mt19937_64 mt;
template<typename DataT>
class NonRecursivePathORAM: public ORAM<DataT> {
    static const size_t npos = size_t(-1);
public:
    struct InnerData {
        size_t Index;
        DataT D;

        InnerData():Index(npos), D(0) {};
        InnerData(size_t Index, DataT D):Index(Index), D(D) {}
    };

    size_t getZ(size_t layer) {
        // by paper, Z >= 4 to make the fail rate is negligible.
        if (layer == L) return 3;                      // leaf: Z_leaf = 3
        auto a = (L*2) / (1ULL << (layer + 2));        // ≈ floor(L / 2^(layer+1))
        auto b = 3 + (layer % 2);                      // 3,4,3,4,3,4,...
        return std::max<size_t>(a, b);
    }

    void init(size_t _N) override {
        N = _N;
        L = 0;
        

        // 3.2 Server Storage
        // L = ceil(log(N)) - 1
        while ((1ULL << L) < N) L++;
        L--;

        Stash.clear();

        // Tree with 2**L leaf, and use 1 base
        M.clear();
        M.resize(2 * (1ULL << L));        

        //mt.seed(7122);
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
        if (index >= N) [[unlikely]] {
            spdlog::error("PlainMemory read overflow read: {} ! ", index);
        }

        DataT Ret;
        Access(index, false, Ret);  // false => this is a read
        return Ret;
    }

    void write(size_t index, DataT Val) override {
        if (index >= N) [[unlikely]] {
            spdlog::error("PlainMemory read overflow read: {} ! ", index);
        }
        Access(index, true, Val);  // true => this is a write
    }

    void dump() {
        spdlog::error("Pos Map");
        for (size_t i = 0; i < N; ++i) {
            spdlog::info(" Pos[{}]={}", i, Pos[i]);
        }
        spdlog::error("Dump tree");
        for (size_t i = 0; i < M.size(); ++i) {
            spdlog::warn("  Node {}", i);
            for (auto [id, v]: M[i])
                spdlog::info("       ind {}, val", id, v);
        }

        spdlog::error("Dump Stash");
        for (auto [id, v]: Stash)
            spdlog::info("       ind {}, val", id, v);
    }

private:
    //std::set<size_t> addrs;
    void Access(size_t Idx, bool NeedWrite, DataT &Val)
    {
        //addrs.insert(Idx);
        size_t Path = Pos[Idx];
        if (Path == npos) [[unlikely]] {
            // First time meet
            Path = UniformN(mt);
            Stash.emplace_back(Idx, 0);
        }
        Pos[Idx] = UniformN(mt);
        // spdlog::info("Put Idx {} at path {}", Idx, Pos[Idx]);

        bitset<64> PathBit(Path);
        size_t TreeId = 1;
        for (size_t layer = 0; layer <= L; ++layer) {
            // Read all data, and ignore dummy data
            for (auto &D: M[TreeId]) {
                Stash.emplace_back(D);
            }
            M[TreeId].clear();

            if (layer != L)
                TreeId = TreeId * 2 + PathBit[layer];
        }

        bool find = false;
        for(auto &[I, D]: Stash) {
            if (I == Idx) {
                if (NeedWrite) D = Val;
                else Val = D;
                find = true;
            }
        }
        if (!find) {
            spdlog::warn("can not found the data from index {}", Idx);
        }

        for (size_t layer = L; layer != npos; --layer) {
            auto eq = [&](size_t Pos)->bool {
                for (int i=0;i<int(layer);++i)
                    if (PathBit[i] != (Pos >> i & 1))
                        return false;
                return true;
            };
            auto Ptr = Stash.begin();
            while (Ptr != Stash.end() && M[TreeId].size() < getZ(layer)) {
                if (eq(Pos[Ptr->Index])) {
                    // spdlog::info("put stash idx {} (Pos={}) into tree node{}", Ptr->Index, Pos[Ptr->Index], TreeId);
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
        static size_t lastMax = 0;
        static size_t cnt = 0;
        cnt++;
        if (Stash.size() > lastMax) {
            lastMax = Stash.size();
            spdlog::trace("Stash size = {:>5}, opnum = {}", Stash.size(), cnt);
        }
        
    }
    //std::mt19937_64 mt;
    std::uniform_int_distribution<size_t> UniformN;
    std::vector<size_t> Pos;
    std::list<InnerData> Stash;
    std::vector<std::vector<InnerData>> M;
    size_t N;
    size_t L;
    //size_t Z;
};

mt19937_64 mt64;
bool CheckORAM(ORAM<long long> *inst, size_t N, int Rounds) {
    spdlog::info("Start CheckORAM");

    PlainMemory<long long> Anwser;
    spdlog::set_level(spdlog::level::trace);
    Anwser.init(N);
    inst->init(N);

    
    std::uniform_int_distribution<size_t> UniformN(0, N-1);

    //while (Rounds--) {
    while (true) {
        if (mt64() % 2) {
            auto Val = mt64();
            auto Ind = UniformN(mt64);

            Anwser.write(Ind, Val);
            inst->write(Ind, Val);
        } else {
            auto Ind = UniformN(mt64);
            if (Anwser.read(Ind) != inst->read(Ind)) {
                spdlog::error("GG, value different!"); // LOL
                return false;
            }
        }
    }

    return true;
}

int main() {
    mt64.seed(time(nullptr));
    mt.seed(time(nullptr) + 0x7122);
    spdlog::info("Start ORAM Test");

    NonRecursivePathORAM<long long> NPathORAM;
    // NPathORAM.init(1 << 3);
    // NPathORAM.write(1, 7122);
    // int v = NPathORAM.read(1);
    // spdlog::info("v = {}", v);
    // PlainMemory<long long> SimpleMemory;
    // spdlog::info("Test SimpleMemory");
    // CheckORAM(&SimpleMemory, 1 << 16, 100000);

    int cnt = 0;
    while (true) {
        cnt++;
        spdlog::info("Test NonRecursivePathORAM... round {}", cnt);
        if (!CheckORAM(&NPathORAM, 1 << 20, 1000'000'000)) {
            //NPathORAM.dump();
            break;
        }
    }
    return 0;
}