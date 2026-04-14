#pragma once

#include "model/Pixel.h"
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <functional>

namespace imageprocessor {

/**
 * Core image data model storing pixel data as a flat RGBA vector.
 * Provides pixel-level access, file I/O (via stb_image), and deep copy.
 */
class ImageModel {
public:
    ImageModel(int width, int height);

    /** Loads from file. Throws on failure. */
    static std::unique_ptr<ImageModel> loadFromFile(const std::string& path);

    /** Saves to file (PNG or JPG based on extension). */
    void saveToFile(const std::string& path) const;

    Pixel getPixel(int x, int y) const;
    void setPixel(int x, int y, const Pixel& pixel);

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    int getPixelCount() const { return width_ * height_; }

    bool isValidCoordinate(int x, int y) const {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    /** Creates a deep copy. */
    std::unique_ptr<ImageModel> deepCopy() const;

    /** Copies data from another model. */
    void setFrom(const ImageModel& other);

    /** Access underlying data. */
    const std::vector<uint8_t>& getData() const { return data_; }
    std::vector<uint8_t>& getData() { return data_; }

private:
    int width_;
    int height_;
    std::vector<uint8_t> data_; // RGBA, 4 bytes per pixel

    void validateCoordinates(int x, int y) const;
};

} // namespace imageprocessor
