#pragma once

#include "factory/FilterFactory.h"
#include <vector>

namespace imageprocessor {

/** Pipeline that executes a sequence of operations. */
class TransformationPipeline {
    std::vector<std::unique_ptr<ImageOperation>> operations_;
public:
    void addOperation(std::unique_ptr<ImageOperation> op) {
        operations_.push_back(std::move(op));
    }

    std::unique_ptr<ImageModel> apply(const ImageModel& source) const {
        std::unique_ptr<ImageModel> current = source.deepCopy();
        for (const auto& op : operations_) {
            current = op->apply(*current);
        }
        return current;
    }

    int size() const { return static_cast<int>(operations_.size()); }
    bool isEmpty() const { return operations_.empty(); }
};

/** Builder for constructing TransformationPipeline with fluent API. */
class TransformationBuilder {
    std::vector<std::pair<std::string, std::unordered_map<std::string, double>>> ops_;
public:
    TransformationBuilder& add(const std::string& name,
            const std::unordered_map<std::string, double>& params = {}) {
        ops_.push_back({name, params});
        return *this;
    }

    TransformationBuilder& addGrayscale() { return add("grayscale"); }
    TransformationBuilder& addSepia() { return add("sepia"); }
    TransformationBuilder& addBrightness(int delta) { return add("brightness", {{"delta", delta}}); }
    TransformationBuilder& addContrast(double f) { return add("contrast", {{"factor", f}}); }
    TransformationBuilder& addInvert() { return add("invert"); }
    TransformationBuilder& addHueShift(double deg) { return add("hue_shift", {{"degrees", deg}}); }
    TransformationBuilder& addSaturation(double f) { return add("saturation", {{"factor", f}}); }
    TransformationBuilder& addGaussianBlur(int ks) { return add("gaussian_blur", {{"kernelSize", (double)ks}}); }
    TransformationBuilder& addSharpen() { return add("sharpen"); }
    TransformationBuilder& addEdgeDetection() { return add("edge_detection"); }
    TransformationBuilder& addEmboss() { return add("emboss"); }
    TransformationBuilder& addBoxBlur(int s) { return add("box_blur", {{"size", (double)s}}); }
    TransformationBuilder& addMedianFilter(int s) { return add("median_filter", {{"size", (double)s}}); }
    TransformationBuilder& addRotateCW() { return add("rotate_cw"); }
    TransformationBuilder& addRotateCCW() { return add("rotate_ccw"); }
    TransformationBuilder& addHorizontalFlip() { return add("horizontal_flip"); }
    TransformationBuilder& addVerticalFlip() { return add("vertical_flip"); }
    TransformationBuilder& addHistogramEqualization() { return add("histogram_equalization"); }
    TransformationBuilder& addPosterize(int lvl) { return add("posterize", {{"levels", (double)lvl}}); }
    TransformationBuilder& addDither() { return add("dither"); }
    TransformationBuilder& addVignette(double i) { return add("vignette", {{"intensity", i}}); }

    TransformationPipeline build() const {
        TransformationPipeline pipeline;
        for (const auto& [name, params] : ops_)
            pipeline.addOperation(FilterFactory::create(name, params));
        return pipeline;
    }

    int size() const { return static_cast<int>(ops_.size()); }
    TransformationBuilder& clear() { ops_.clear(); return *this; }
};

} // namespace imageprocessor
