#pragma once

#include "model/ImageModel.h"
#include "command/Command.h"
#include "factory/FilterFactory.h"
#include <string>
#include <memory>

namespace imageprocessor {

/**
 * Controller bridging CLI view and model.
 * Manages image loading/saving, operation application, and undo/redo.
 */
class ImageController {
    std::unique_ptr<ImageModel> model_;
    CommandHistory history_;
public:
    ImageController() : history_(50) {}

    void loadImage(const std::string& path) {
        model_ = ImageModel::loadFromFile(path);
        history_.clear();
    }

    void saveImage(const std::string& path) const {
        if (!model_) throw std::logic_error("No image loaded");
        model_->saveToFile(path);
    }

    void applyOperation(const std::string& name,
            const std::unordered_map<std::string, double>& params = {}) {
        if (!model_) throw std::logic_error("No image loaded");
        auto op = FilterFactory::create(name, params);
        auto cmd = std::make_unique<OperationCommand>(*model_, std::move(op));
        history_.executeCommand(std::move(cmd));
    }

    bool undo() { return history_.undo(); }
    bool redo() { return history_.redo(); }

    bool hasImage() const { return model_ != nullptr; }
    ImageModel* getModel() { return model_.get(); }
    const ImageModel* getModel() const { return model_.get(); }
    CommandHistory& getHistory() { return history_; }
    bool canUndo() const { return history_.canUndo(); }
    bool canRedo() const { return history_.canRedo(); }
};

} // namespace imageprocessor
