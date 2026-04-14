#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <string>
#include <array>

namespace imageprocessor {

/**
 * Represents an RGBA pixel value.
 * All channel values are clamped to [0, 255].
 */
struct Pixel {
    uint8_t r, g, b, a;

    Pixel() : r(0), g(0), b(0), a(255) {}
    Pixel(int r, int g, int b, int a = 255)
        : r(clamp(r)), g(clamp(g)), b(clamp(b)), a(clamp(a)) {}

    static uint8_t clamp(int value) {
        return static_cast<uint8_t>(std::max(0, std::min(255, value)));
    }

    /** Returns grayscale luminance: Y = 0.299*R + 0.587*G + 0.114*B */
    int toGray() const {
        return clamp(static_cast<int>(std::round(0.299 * r + 0.587 * g + 0.114 * b)));
    }

    /** Converts to HSL. Returns array [H(0-360), S(0-1), L(0-1)]. */
    std::array<double, 3> toHSL() const {
        double rn = r / 255.0, gn = g / 255.0, bn = b / 255.0;
        double mx = std::max({rn, gn, bn});
        double mn = std::min({rn, gn, bn});
        double delta = mx - mn;
        double h = 0, s = 0, l = (mx + mn) / 2.0;

        if (delta != 0) {
            s = l < 0.5 ? delta / (mx + mn) : delta / (2.0 - mx - mn);
            if (mx == rn) h = std::fmod((gn - bn) / delta, 6.0);
            else if (mx == gn) h = (bn - rn) / delta + 2;
            else h = (rn - gn) / delta + 4;
            h *= 60;
            if (h < 0) h += 360;
        }
        return {h, s, l};
    }

    /** Creates a Pixel from HSL values. */
    static Pixel fromHSL(double h, double s, double l, int a = 255) {
        double c = (1.0 - std::abs(2.0 * l - 1.0)) * s;
        double x = c * (1.0 - std::abs(std::fmod(h / 60.0, 2.0) - 1.0));
        double m = l - c / 2.0;
        double rp, gp, bp;
        if (h < 60)       { rp = c; gp = x; bp = 0; }
        else if (h < 120) { rp = x; gp = c; bp = 0; }
        else if (h < 180) { rp = 0; gp = c; bp = x; }
        else if (h < 240) { rp = 0; gp = x; bp = c; }
        else if (h < 300) { rp = x; gp = 0; bp = c; }
        else              { rp = c; gp = 0; bp = x; }
        return Pixel(
            static_cast<int>(std::round((rp + m) * 255)),
            static_cast<int>(std::round((gp + m) * 255)),
            static_cast<int>(std::round((bp + m) * 255)),
            a
        );
    }

    bool operator==(const Pixel& o) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
    bool operator!=(const Pixel& o) const { return !(*this == o); }
};

} // namespace imageprocessor
