#ifndef ORAM_H
#define ORAM_H

#include <cstddef>             // size_t
#include <vector>              // std::vector
#include <spdlog/spdlog.h>     // spdlog::error

template<typename DataT>
class ORAM {
public:
    virtual void init(size_t N) = 0; /* Create a memory [0: N-1] */
    virtual DataT read(size_t index) = 0;
    virtual void write(size_t index, DataT Val) = 0;
};

// This is not a ORAM, it use for test the correctness
template<typename DataT>
class PlainMemory: public ORAM<DataT> {
public:
    void init(size_t _N) override {
        N = _N;
        M.resize(N);
    }
    DataT read(size_t index) override {
        if (index >= N) [[unlikely]] {
            spdlog::error("PlainMemory read overflow read: {} ! ", index);
        }
        return M[index];
    }
    void write(size_t index, DataT Val) override {
        if (index >= N) [[unlikely]] {
            spdlog::error("PlainMemory read overflow read: {} ! ", index);
        }
        M[index] = Val;
    }
private:
    std::vector<DataT> M;
    size_t N;
};
#endif // ORAM_H