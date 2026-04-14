#include "model/ImageModel.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <algorithm>
#include <cstring>

namespace imageprocessor {

static void validateDims(int w, int h) {
    if (w <= 0 || h <= 0) {
        throw std::invalid_argument("Dimensions must be positive: " +
            std::to_string(w) + "x" + std::to_string(h));
    }
}

ImageModel::ImageModel(int width, int height)
    : width_((validateDims(width, height), width)),
      height_(height),
      data_(width * height * 4, 0)
{
    // Set alpha to 255 by default
    for (int i = 3; i < static_cast<int>(data_.size()); i += 4) {
        data_[i] = 255;
    }
}

std::unique_ptr<ImageModel> ImageModel::loadFromFile(const std::string& path) {
    int w, h, channels;
    unsigned char* raw = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!raw) {
        throw std::runtime_error("Failed to load image: " + path + " - " + stbi_failure_reason());
    }
    auto model = std::make_unique<ImageModel>(w, h);
    std::memcpy(model->data_.data(), raw, w * h * 4);
    stbi_image_free(raw);
    return model;
}

void ImageModel::saveToFile(const std::string& path) const {
    std::string lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    auto endsWith = [](const std::string& s, const std::string& suffix) {
        return s.size() >= suffix.size() &&
               s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
    };

    int ok = 0;
    if (endsWith(lower, ".png")) {
        ok = stbi_write_png(path.c_str(), width_, height_, 4, data_.data(), width_ * 4);
    } else if (endsWith(lower, ".jpg") || endsWith(lower, ".jpeg")) {
        ok = stbi_write_jpg(path.c_str(), width_, height_, 4, data_.data(), 90);
    } else if (endsWith(lower, ".bmp")) {
        ok = stbi_write_bmp(path.c_str(), width_, height_, 4, data_.data());
    } else {
        ok = stbi_write_png(path.c_str(), width_, height_, 4, data_.data(), width_ * 4);
    }

    if (!ok) {
        throw std::runtime_error("Failed to save image: " + path);
    }
}

Pixel ImageModel::getPixel(int x, int y) const {
    validateCoordinates(x, y);
    int idx = (y * width_ + x) * 4;
    return Pixel(data_[idx], data_[idx + 1], data_[idx + 2], data_[idx + 3]);
}

void ImageModel::setPixel(int x, int y, const Pixel& pixel) {
    validateCoordinates(x, y);
    int idx = (y * width_ + x) * 4;
    data_[idx]     = pixel.r;
    data_[idx + 1] = pixel.g;
    data_[idx + 2] = pixel.b;
    data_[idx + 3] = pixel.a;
}

std::unique_ptr<ImageModel> ImageModel::deepCopy() const {
    auto copy = std::make_unique<ImageModel>(width_, height_);
    copy->data_ = data_;
    return copy;
}

void ImageModel::setFrom(const ImageModel& other) {
    width_ = other.width_;
    height_ = other.height_;
    data_ = other.data_;
}

void ImageModel::validateCoordinates(int x, int y) const {
    if (!isValidCoordinate(x, y)) {
        throw std::out_of_range("Coordinates (" + std::to_string(x) + ", " +
            std::to_string(y) + ") out of bounds for " +
            std::to_string(width_) + "x" + std::to_string(height_));
    }
}

} // namespace imageprocessor
