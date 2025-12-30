#pragma once

#include "TestContent.h"

#include <cstddef>
#include <spdlog/spdlog.h>

template <typename DataT>
class ORAM {
  public:
    ORAM(TestEnvContent &Cnt) : Cnt(Cnt) {};
    virtual void init(std::size_t N) = 0; /* Create a memory [0: N-1] */
    virtual DataT read(std::size_t index) = 0;
    virtual void write(std::size_t index, DataT Val) = 0;
    virtual bool report() { return false; }; // called after an operation, return true to stop
    virtual std::size_t stash_size() { return -1; }

  protected:
    TestEnvContent &Cnt;
};
