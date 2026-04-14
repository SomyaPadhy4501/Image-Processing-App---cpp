#pragma once

#include "operations/ImageOperation.h"
#include "model/ImageModel.h"
#include <memory>
#include <string>
#include <stack>
#include <deque>

namespace imageprocessor {

/** Command interface for undo/redo pattern. */
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getDescription() const = 0;
};

/** Wraps an ImageOperation as a Command with snapshot-based undo. */
class OperationCommand : public Command {
    ImageModel& model_;
    std::unique_ptr<ImageOperation> operation_;
    std::unique_ptr<ImageModel> snapshot_;
public:
    OperationCommand(ImageModel& model, std::unique_ptr<ImageOperation> op)
        : model_(model), operation_(std::move(op)) {
        if (!operation_) throw std::invalid_argument("Operation cannot be null");
    }

    void execute() override {
        snapshot_ = model_.deepCopy();
        auto result = operation_->apply(model_);
        model_.setFrom(*result);
    }

    void undo() override {
        if (!snapshot_) throw std::logic_error("Cannot undo: not executed");
        model_.setFrom(*snapshot_);
    }

    std::string getDescription() const override { return operation_->getName(); }
    ImageOperation* getOperation() const { return operation_.get(); }
};

/** Manages command history with undo/redo stacks. */
class CommandHistory {
    std::deque<std::unique_ptr<Command>> undoStack_;
    std::deque<std::unique_ptr<Command>> redoStack_;
    int maxSize_;
public:
    explicit CommandHistory(int maxSize = 0) : maxSize_(maxSize) {}

    void executeCommand(std::unique_ptr<Command> cmd) {
        if (!cmd) throw std::invalid_argument("Command cannot be null");
        cmd->execute();
        undoStack_.push_back(std::move(cmd));
        redoStack_.clear();
        if (maxSize_ > 0 && static_cast<int>(undoStack_.size()) > maxSize_)
            undoStack_.pop_front();
    }

    bool undo() {
        if (undoStack_.empty()) return false;
        auto cmd = std::move(undoStack_.back()); undoStack_.pop_back();
        cmd->undo();
        redoStack_.push_back(std::move(cmd));
        return true;
    }

    bool redo() {
        if (redoStack_.empty()) return false;
        auto cmd = std::move(redoStack_.back()); redoStack_.pop_back();
        cmd->execute();
        undoStack_.push_back(std::move(cmd));
        return true;
    }

    bool canUndo() const { return !undoStack_.empty(); }
    bool canRedo() const { return !redoStack_.empty(); }
    int getUndoSize() const { return static_cast<int>(undoStack_.size()); }
    int getRedoSize() const { return static_cast<int>(redoStack_.size()); }
    void clear() { undoStack_.clear(); redoStack_.clear(); }

    std::string getUndoDescription() const {
        return undoStack_.empty() ? "" : undoStack_.back()->getDescription();
    }
    std::string getRedoDescription() const {
        return redoStack_.empty() ? "" : redoStack_.back()->getDescription();
    }
};

} // namespace imageprocessor
