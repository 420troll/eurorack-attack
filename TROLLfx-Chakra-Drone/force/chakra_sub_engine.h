#pragma once
// TROLLfx Chakra Sub Engine: independent, single-voice sine + wavefolder.
// No allocation, no Plaits state changes, no external oscillator dependencies.
#include <algorithm>
#include <cmath>

namespace chakra {
class SubVoice {
public:
    explicit SubVoice(float sample_rate = 44100.f)
        : sample_rate_(sample_rate > 0 ? sample_rate : 44100.f) {}

    // Divide 1..10, glide 0..5000ms, fold and mix 0..1.
    void configure(float divisor, float glide_ms, float fold, float mix, bool harmonic_lock = true, bool phase_lock = true) noexcept {
        const float new_divider = valid(divisor) ? clamp(divisor, 1.f, 10.f) : 10.f;
        harmonic_lock_ = harmonic_lock;
        phase_lock_ = phase_lock;
        const float quantized = harmonic_lock_ ? std::round(new_divider) : new_divider;
        const bool changed = quantized != divider_;
        divider_ = quantized;
        glide_ms_ = valid(glide_ms) ? clamp(glide_ms, 0.f, 5000.f) : 900.f;
        fold_ = valid(fold) ? clamp(fold, 0.f, 1.f) : .2f;
        mix_ = valid(mix) ? clamp(mix, 0.f, 1.f) : .5f;
        // Changing divider while sounding slews from the *current* frequency.
        if (active_ && changed) beginGlide(current_hz_, root_hz_ / divider_);
    }

    void trigger(float root_hz, float velocity = 1.f) noexcept {
        if (!valid(root_hz) || root_hz <= 0.f) return;
        root_hz_ = clamp(root_hz, 1.f, sample_rate_ * .4f);
        velocity_ = valid(velocity) ? clamp(velocity, 0.f, 1.f) : 1.f;
        active_ = true;
        gate_ = true;
        if (phase_lock_) phase_ = 0.f;
        beginGlide(root_hz_, root_hz_ / divider_);
    }

    // Change an owned voice's root without resetting its phase or envelope.
    void retune(float root_hz) noexcept {
        if (!valid(root_hz) || root_hz <= 0.f) return;
        const float next = clamp(root_hz, 1.f, sample_rate_ * .4f);
        if (next == root_hz_) return;
        root_hz_ = next;
        if (active_) beginGlide(current_hz_, root_hz_ / divider_);
    }

    void release() noexcept { gate_ = false; }
    void silence() noexcept { active_ = false; gate_ = false; env_ = 0.f; }
    bool active() const noexcept { return active_; }
    float frequency() const noexcept { return current_hz_; }
    float target() const noexcept { return root_hz_ / divider_; }

    float process() noexcept {
        if (!active_) return 0.f;
        if (glide_index_ < glide_samples_) {
            const float t = static_cast<float>(++glide_index_) / static_cast<float>(glide_samples_);
            const float smooth = t * t * (3.f - 2.f * t);
            current_hz_ = std::exp2(start_log_ + (end_log_ - start_log_) * smooth);
        } else {
            current_hz_ = target();
        }

        const float attack = std::min(1.f, 1.f / (sample_rate_ * .003f));
        const float decay = std::min(1.f, 1.f / (sample_rate_ * .020f));
        env_ += (gate_ ? attack : -decay);
        env_ = clamp(env_, 0.f, 1.f);
        if (!gate_ && env_ <= 0.f) {
            silence();
            return 0.f;
        }

        const float sine = std::sin(6.2831853071795864769f * phase_);
        phase_ += current_hz_ / sample_rate_;
        phase_ -= std::floor(phase_);
        // Continuous symmetric wavefold: folds a boosted sine back within [-1,1].
        // blend=0 guarantees a mathematically clean sine.
        const float driven = sine * (1.f + 5.f * fold_);
        const float folded = (2.f / 3.14159265358979323846f) *
                             std::asin(std::sin(1.57079632679489661923f * driven));
        return mix_ * velocity_ * env_ * ((1.f - fold_) * sine + fold_ * folded);
    }

private:
    static bool valid(float x) noexcept { return std::isfinite(x); }
    static float clamp(float x, float lo, float hi) noexcept {
        return x < lo ? lo : (x > hi ? hi : x);
    }
    void beginGlide(float from, float to) noexcept {
        start_log_ = std::log2(std::max(from, 1.f));
        end_log_ = std::log2(std::max(to, 1.f));
        current_hz_ = from;
        glide_samples_ = static_cast<int>(std::round(glide_ms_ * .001f * sample_rate_));
        glide_index_ = 0;
        if (!glide_samples_) current_hz_ = to;
    }

    float sample_rate_, divider_=10.f, glide_ms_=900.f, fold_=.2f, mix_=.5f;
    float root_hz_=528.f, velocity_=1.f, phase_=0.f, env_=0.f;
    float current_hz_=528.f, start_log_=0.f, end_log_=0.f;
    int glide_samples_=0, glide_index_=0;
    bool active_=false, gate_=false, harmonic_lock_=true, phase_lock_=true;
};
}
