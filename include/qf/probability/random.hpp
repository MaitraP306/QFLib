#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <limits>

#include <qf/core/constants.hpp>
#include <qf/core/types.hpp>
#include <qf/math/scalar.hpp>

namespace qf::probability {

class Xoshiro256StarStar final {
public:
    explicit Xoshiro256StarStar(std::uint64_t seed = 0x9e3779b97f4a7c15ULL) noexcept {
        seed_state(seed);
    }

    [[nodiscard]] std::uint64_t next_u64() noexcept {
        const std::uint64_t result = std::rotl(s_[1] * 5ULL, 7) * 9ULL;
        const std::uint64_t t = s_[1] << 17;
        s_[2] ^= s_[0]; s_[3] ^= s_[1]; s_[1] ^= s_[2]; s_[0] ^= s_[3];
        s_[2] ^= t;
        s_[3] = std::rotl(s_[3], 45);
        return result;
    }

    [[nodiscard]] Real uniform_open() noexcept {
        const std::uint64_t bits = next_u64() >> 11;
        return (static_cast<Real>(bits) + 0.5) * (1.0 / 9007199254740992.0);
    }

    [[nodiscard]] Real normal() noexcept {
        if (has_spare_) {
            has_spare_ = false;
            return spare_;
        }
        const Real u1 = uniform_open();
        const Real u2 = uniform_open();
        const Real r = qf::math::sqrt(-2.0 * qf::math::log(u1));
        const Real theta = qf::constants::two_pi * u2;
        spare_ = r * qf::math::sin(theta);
        has_spare_ = true;
        return r * qf::math::cos(theta);
    }

    [[nodiscard]] Real normal(Real mean, Real standard_deviation) noexcept {
        return mean + standard_deviation * normal();
    }

    void seed_state(std::uint64_t seed) noexcept {
        auto next = [&]() noexcept {
            seed += 0x9e3779b97f4a7c15ULL;
            std::uint64_t z = seed;
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
            z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
            return z ^ (z >> 31);
        };
        for (auto& x : s_) x = next();
        has_spare_ = false;
        spare_ = 0.0;
    }

private:
    std::array<std::uint64_t, 4> s_{};
    Real spare_ = 0.0;
    bool has_spare_ = false;
};

} // namespace qf::probability
