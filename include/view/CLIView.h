#pragma once

#include "controller/ImageController.h"
#include "factory/FilterFactory.h"
#include <iostream>
#include <string>
#include <sstream>

namespace imageprocessor {

/**
 * Interactive CLI view for the image processor.
 * Menu-driven interface with numbered options.
 */
class CLIView {
    ImageController controller_;

    void printMenu() {
        std::cout << "\n========================================\n";
        std::cout << "         IMAGE PROCESSOR CLI\n";
        std::cout << "========================================\n";
        if (controller_.hasImage()) {
            auto* m = controller_.getModel();
            std::cout << "  Image: " << m->getWidth() << "x" << m->getHeight()
                      << " | History: " << controller_.getHistory().getUndoSize() << "\n";
        } else {
            std::cout << "  No image loaded\n";
        }
        std::cout << "----------------------------------------\n";
        std::cout << "  1. Load image\n";
        std::cout << "  2. Save image\n";
        std::cout << "  3. Apply operation\n";
        std::cout << "  4. Undo\n";
        std::cout << "  5. Redo\n";
        std::cout << "  6. List operations\n";
        std::cout << "  0. Exit\n";
        std::cout << "----------------------------------------\n";
        std::cout << "Choice: ";
    }

    void listOperations() {
        auto names = FilterFactory::getAllOperationNames();
        std::cout << "\nAvailable operations (" << names.size() << "):\n";
        for (int i = 0; i < (int)names.size(); i++)
            std::cout << "  " << (i+1) << ". " << names[i] << "\n";
    }

    void applyOperation() {
        if (!controller_.hasImage()) {
            std::cout << "Error: Load an image first.\n"; return;
        }
        listOperations();
        std::cout << "\nOperation name: ";
        std::string name;
        std::getline(std::cin, name);
        try {
            controller_.applyOperation(name);
            std::cout << "Applied: " << name << "\n";
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }

public:
    void run() {
        std::string input;
        while (true) {
            printMenu();
            std::getline(std::cin, input);
            if (input.empty()) continue;

            switch (input[0]) {
                case '1': {
                    std::cout << "File path: ";
                    std::string path; std::getline(std::cin, path);
                    try { controller_.loadImage(path); std::cout << "Loaded!\n"; }
                    catch (const std::exception& e) { std::cout << "Error: " << e.what() << "\n"; }
                    break;
                }
                case '2': {
                    std::cout << "Save path: ";
                    std::string path; std::getline(std::cin, path);
                    try { controller_.saveImage(path); std::cout << "Saved!\n"; }
                    catch (const std::exception& e) { std::cout << "Error: " << e.what() << "\n"; }
                    break;
                }
                case '3': applyOperation(); break;
                case '4':
                    if (controller_.undo()) std::cout << "Undone.\n";
                    else std::cout << "Nothing to undo.\n";
                    break;
                case '5':
                    if (controller_.redo()) std::cout << "Redone.\n";
                    else std::cout << "Nothing to redo.\n";
                    break;
                case '6': listOperations(); break;
                case '0': std::cout << "Goodbye!\n"; return;
                default: std::cout << "Invalid choice.\n";
            }
        }
    }
};

} // namespace imageprocessor
