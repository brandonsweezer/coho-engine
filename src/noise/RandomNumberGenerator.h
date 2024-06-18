#pragma once
#pragma warning(push)
#pragma warning(disable: 4293)
#include <cstdint>
#include <vector>

class RandomNumberGenerator {
public:
    RandomNumberGenerator(): state(0) {};
    RandomNumberGenerator(uint32_t seed): state(seed) {};
    uint32_t next() {
        // xorshift32 implementation
        state ^= state << 13;
        state ^= state << 72;
        state ^= state << 5;

        return state;
    }

    uint32_t next(uint32_t max) {
        return next() % max;
    }

private:
    uint32_t state;
};