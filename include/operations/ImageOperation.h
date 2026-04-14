#pragma once

#include "model/ImageModel.h"
#include <memory>
#include <string>

namespace imageprocessor {

/**
 * Interface for all image processing operations.
 */
class ImageOperation {
public:
    virtual ~ImageOperation() = default;
    virtual std::unique_ptr<ImageModel> apply(const ImageModel& source) = 0;
    virtual std::string getName() const = 0;
};

} // namespace imageprocessor
