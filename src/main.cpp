#include "view/CLIView.h"
#include "controller/ImageController.h"
#include "factory/FilterFactory.h"
#include <iostream>
#include <string>
#include <unordered_map>

using namespace imageprocessor;

/**
 * Batch mode: ./image_processor <input> <operation> <output> [param=value ...]
 * Interactive: ./image_processor  (no args — launches CLI menu)
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        // Interactive CLI mode
        CLIView view;
        view.run();
        return 0;
    }

    // Batch mode: image_processor <input> <operation> <output> [params...]
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <input> <operation> <output> [key=value ...]\n";
        std::cerr << "       " << argv[0] << "   (no args for interactive mode)\n";
        std::cerr << "\nOperations:\n";
        for (const auto& name : FilterFactory::getAllOperationNames())
            std::cerr << "  " << name << "\n";
        return 1;
    }

    std::string inputPath  = argv[1];
    std::string operation  = argv[2];
    std::string outputPath = argv[3];

    // Parse optional params: key=value
    std::unordered_map<std::string, double> params;
    for (int i = 4; i < argc; i++) {
        std::string arg = argv[i];
        auto eq = arg.find('=');
        if (eq != std::string::npos) {
            std::string key = arg.substr(0, eq);
            double val = std::stod(arg.substr(eq + 1));
            params[key] = val;
        }
    }

    try {
        auto model = ImageModel::loadFromFile(inputPath);
        auto op = FilterFactory::create(operation, params);
        auto result = op->apply(*model);
        result->saveToFile(outputPath);
        std::cout << "OK " << result->getWidth() << "x" << result->getHeight() << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
