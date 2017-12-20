
/*
    pbrt source code is Copyright(c) 1998-2015
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_CORE_PARALLEL_H
#define PBRT_CORE_PARALLEL_H

#include <mutex>
#include <functional>
#include <atomic>
#include "com/geometry.hpp"

inline uint32_t FloatToBits(float f) {
	uint32_t ui;
	memcpy(&ui, &f, sizeof(float));
	return ui;
}

inline float BitsToFloat(uint32_t ui) {
	float f;
	memcpy(&f, &ui, sizeof(uint32_t));
	return f;
}

// Parallel Declarations
class AtomicFloat {
  public:
    // AtomicFloat Public Methods
    explicit AtomicFloat(float v = 0) { bits = FloatToBits(v); }
    operator float() const { return BitsToFloat(bits); }
    float operator=(float v) {
        bits = FloatToBits(v);
        return v;
    }
    void Add(float v) {
#ifdef PBRT_FLOAT_AS_DOUBLE
        uint64_t oldBits = bits, newBits;
#else
        uint32_t oldBits = bits, newBits;
#endif
        do {
            newBits = FloatToBits(BitsToFloat(oldBits) + v);
        } while (!bits.compare_exchange_weak(oldBits, newBits));
    }

  private:
// AtomicFloat Private Data
#ifdef PBRT_FLOAT_AS_DOUBLE
    std::atomic<uint64_t> bits;
#else
    std::atomic<uint32_t> bits;
#endif
};

void ParallelFor(const std::function<void(int64_t)> &func, int64_t count,
                 int chunkSize = 1);
extern thread_local int ThreadIndex;
void ParallelFor(std::function<void(renderer::Point2dI)> func, const renderer::Point2dI &count);
int MaxThreadIndex();
int NumSystemCores();
void TerminateWorkerThreads();

#endif  // PBRT_CORE_PARALLEL_H
