#pragma once

#include "operations/AllOperations.h"
#include <string>
#include <unordered_map>
#include <functional>

namespace imageprocessor {

/** Factory for creating ImageOperation instances from string names and parameters. */
class FilterFactory {
public:
    static std::unique_ptr<ImageOperation> create(const std::string& name,
            const std::unordered_map<std::string, double>& params = {}) {
        auto getParam = [&](const std::string& key, double def) -> double {
            auto it = params.find(key);
            return it != params.end() ? it->second : def;
        };
        auto getInt = [&](const std::string& key, int def) -> int {
            return static_cast<int>(getParam(key, def));
        };

        if (name == "grayscale") return std::make_unique<GrayscaleOperation>();
        if (name == "sepia") return std::make_unique<SepiaOperation>();
        if (name == "brightness") return std::make_unique<BrightnessOperation>(getInt("delta", 50));
        if (name == "contrast") return std::make_unique<ContrastOperation>(getParam("factor", 1.5));
        if (name == "invert") return std::make_unique<InvertOperation>();
        if (name == "channel_red") return std::make_unique<ChannelIsolationOperation>(Channel::RED);
        if (name == "channel_green") return std::make_unique<ChannelIsolationOperation>(Channel::GREEN);
        if (name == "channel_blue") return std::make_unique<ChannelIsolationOperation>(Channel::BLUE);
        if (name == "hue_shift") return std::make_unique<HueShiftOperation>(getParam("degrees", 90));
        if (name == "saturation") return std::make_unique<SaturationOperation>(getParam("factor", 1.5));
        if (name == "gaussian_blur") return std::make_unique<GaussianBlurOperation>(getInt("kernelSize", 5));
        if (name == "sharpen") return std::make_unique<SharpenOperation>();
        if (name == "edge_detection") return std::make_unique<EdgeDetectionOperation>();
        if (name == "emboss") return std::make_unique<EmbossOperation>();
        if (name == "box_blur") return std::make_unique<BoxBlurOperation>(getInt("size", 3));
        if (name == "median_filter") return std::make_unique<MedianFilterOperation>(getInt("size", 3));
        if (name == "rotate_cw") return std::make_unique<RotateCWOperation>();
        if (name == "rotate_ccw") return std::make_unique<RotateCCWOperation>();
        if (name == "horizontal_flip") return std::make_unique<HorizontalFlipOperation>();
        if (name == "vertical_flip") return std::make_unique<VerticalFlipOperation>();
        if (name == "crop") return std::make_unique<CropOperation>(
            getInt("x",0), getInt("y",0), getInt("width",100), getInt("height",100));
        if (name == "resize") return std::make_unique<ResizeOperation>(
            getInt("width",200), getInt("height",200));
        if (name == "histogram_equalization") return std::make_unique<HistogramEqualizationOperation>();
        if (name == "posterize") return std::make_unique<PosterizeOperation>(getInt("levels", 4));
        if (name == "dither") return std::make_unique<DitherOperation>();
        if (name == "vignette") return std::make_unique<VignetteOperation>(getParam("intensity", 0.5));

        throw std::invalid_argument("Unknown operation: " + name);
    }

    /** Returns list of all supported operation names. */
    static std::vector<std::string> getAllOperationNames() {
        return {"grayscale","sepia","brightness","contrast","invert",
                "channel_red","channel_green","channel_blue","hue_shift","saturation",
                "gaussian_blur","sharpen","edge_detection","emboss","box_blur","median_filter",
                "rotate_cw","rotate_ccw","horizontal_flip","vertical_flip","crop","resize",
                "histogram_equalization","posterize","dither","vignette"};
    }
};

} // namespace imageprocessor
