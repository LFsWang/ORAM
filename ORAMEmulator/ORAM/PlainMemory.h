#pragma once

#include "Base.h"

// This is not a ORAM, it use for test the correctness
template <typename DataT>
class PlainMemory : public ORAM<DataT> {
  public:
    PlainMemory(TestEnvContent &Cnt) : ORAM<DataT>(Cnt) {};
    void init(std::size_t _N) override {
        N = _N;
        M.resize(N);
    }

    DataT read(std::size_t index) override {
        if (index >= N) [[unlikely]] {
            spdlog::error("PlainMemory read overflow read: {} ! ", index);
        }
        return M[index];
    }

    void write(std::size_t index, DataT Val) override {
        if (index >= N) [[unlikely]] {
            spdlog::error("PlainMemory read overflow read: {} ! ", index);
        }
        M[index] = Val;
    }

  private:
    std::vector<DataT> M;
    std::size_t N;
};
