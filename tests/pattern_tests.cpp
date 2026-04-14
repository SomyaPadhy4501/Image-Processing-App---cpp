#include <gtest/gtest.h>
#include "command/Command.h"
#include "factory/FilterFactory.h"
#include "builder/TransformationBuilder.h"
#include "controller/ImageController.h"
#include <memory>
#include <filesystem>

using namespace imageprocessor;

std::unique_ptr<ImageModel> makeSolid(int w, int h, int r, int g, int b) {
    auto m = std::make_unique<ImageModel>(w, h);
    Pixel p(r, g, b);
    for (int y=0;y<h;y++) for (int x=0;x<w;x++) m->setPixel(x, y, p);
    return m;
}

// ==================== COMMAND TESTS ====================

TEST(OperationCommandTest, Execute) {
    auto model = makeSolid(3,3,100,150,200);
    auto op = std::make_unique<GrayscaleOperation>();
    OperationCommand cmd(*model, std::move(op));
    cmd.execute();
    auto p = model->getPixel(0,0);
    EXPECT_EQ(p.r, p.g); EXPECT_EQ(p.g, p.b);
}

TEST(OperationCommandTest, Undo) {
    auto model = makeSolid(3,3,100,150,200);
    auto orig = model->getPixel(0,0);
    OperationCommand cmd(*model, std::make_unique<GrayscaleOperation>());
    cmd.execute();
    cmd.undo();
    EXPECT_EQ(orig, model->getPixel(0,0));
}

TEST(OperationCommandTest, UndoBeforeExecute) {
    auto model = makeSolid(3,3,100,100,100);
    OperationCommand cmd(*model, std::make_unique<GrayscaleOperation>());
    EXPECT_THROW(cmd.undo(), std::logic_error);
}

TEST(OperationCommandTest, NullOp) {
    auto model = makeSolid(3,3,100,100,100);
    EXPECT_THROW(OperationCommand(*model, nullptr), std::invalid_argument);
}

TEST(OperationCommandTest, GetDescription) {
    auto model = makeSolid(3,3,100,100,100);
    OperationCommand cmd(*model, std::make_unique<GrayscaleOperation>());
    EXPECT_EQ("Grayscale", cmd.getDescription());
}

TEST(CommandHistoryTest, Empty) {
    CommandHistory h;
    EXPECT_FALSE(h.canUndo()); EXPECT_FALSE(h.canRedo());
    EXPECT_EQ(0, h.getUndoSize());
}

TEST(CommandHistoryTest, ExecuteAndUndo) {
    auto m = makeSolid(3,3,100,100,100);
    CommandHistory h;
    h.executeCommand(std::make_unique<OperationCommand>(*m, std::make_unique<InvertOperation>()));
    EXPECT_TRUE(h.canUndo()); EXPECT_EQ(1, h.getUndoSize());
    h.undo();
    EXPECT_FALSE(h.canUndo()); EXPECT_TRUE(h.canRedo());
}

TEST(CommandHistoryTest, Redo) {
    auto m = makeSolid(3,3,100,100,100);
    CommandHistory h;
    h.executeCommand(std::make_unique<OperationCommand>(*m, std::make_unique<InvertOperation>()));
    h.undo();
    h.redo();
    EXPECT_TRUE(h.canUndo()); EXPECT_FALSE(h.canRedo());
}

TEST(CommandHistoryTest, RedoClearedOnNew) {
    auto m = makeSolid(3,3,100,100,100);
    CommandHistory h;
    h.executeCommand(std::make_unique<OperationCommand>(*m, std::make_unique<InvertOperation>()));
    h.undo();
    h.executeCommand(std::make_unique<OperationCommand>(*m, std::make_unique<GrayscaleOperation>()));
    EXPECT_FALSE(h.canRedo());
}

TEST(CommandHistoryTest, Clear) {
    auto m = makeSolid(3,3,100,100,100);
    CommandHistory h;
    h.executeCommand(std::make_unique<OperationCommand>(*m, std::make_unique<InvertOperation>()));
    h.clear();
    EXPECT_FALSE(h.canUndo()); EXPECT_FALSE(h.canRedo());
}

TEST(CommandHistoryTest, UndoEmptyFalse) {
    EXPECT_FALSE(CommandHistory().undo());
}

TEST(CommandHistoryTest, RedoEmptyFalse) {
    EXPECT_FALSE(CommandHistory().redo());
}

TEST(CommandHistoryTest, NullCommand) {
    CommandHistory h;
    EXPECT_THROW(h.executeCommand(nullptr), std::invalid_argument);
}

TEST(CommandHistoryTest, MaxSize) {
    auto m = makeSolid(3,3,100,100,100);
    CommandHistory h(3);
    for (int i=0;i<5;i++)
        h.executeCommand(std::make_unique<OperationCommand>(*m, std::make_unique<InvertOperation>()));
    EXPECT_EQ(3, h.getUndoSize());
}

TEST(CommandHistoryTest, Descriptions) {
    auto m = makeSolid(3,3,100,100,100);
    CommandHistory h;
    EXPECT_TRUE(h.getUndoDescription().empty());
    h.executeCommand(std::make_unique<OperationCommand>(*m, std::make_unique<GrayscaleOperation>()));
    EXPECT_EQ("Grayscale", h.getUndoDescription());
    h.undo();
    EXPECT_EQ("Grayscale", h.getRedoDescription());
}

// ==================== FACTORY TESTS ====================

TEST(FilterFactoryTest, CreateAll) {
    for (const auto& name : FilterFactory::getAllOperationNames()) {
        auto op = FilterFactory::create(name);
        EXPECT_NE(nullptr, op.get()) << "Failed for: " << name;
        EXPECT_FALSE(op->getName().empty());
    }
}

TEST(FilterFactoryTest, CreateGrayscale) {
    auto op = FilterFactory::create("grayscale");
    EXPECT_EQ("Grayscale", op->getName());
}

TEST(FilterFactoryTest, CreateBrightnessWithParams) {
    auto op = FilterFactory::create("brightness", {{"delta", 75}});
    EXPECT_NE(nullptr, op.get());
}

TEST(FilterFactoryTest, UnknownThrows) {
    EXPECT_THROW(FilterFactory::create("unknown"), std::invalid_argument);
}

TEST(FilterFactoryTest, OperationCount) {
    EXPECT_GE(FilterFactory::getAllOperationNames().size(), 24u);
}

// ==================== BUILDER TESTS ====================

TEST(TransformationBuilderTest, EmptyBuild) {
    auto pipeline = TransformationBuilder().build();
    EXPECT_TRUE(pipeline.isEmpty()); EXPECT_EQ(0, pipeline.size());
}

TEST(TransformationBuilderTest, SingleOp) {
    auto pipeline = TransformationBuilder().addGrayscale().build();
    EXPECT_EQ(1, pipeline.size());
}

TEST(TransformationBuilderTest, ChainedOps) {
    auto pipeline = TransformationBuilder()
        .addGrayscale().addContrast(1.5).addSharpen().build();
    EXPECT_EQ(3, pipeline.size());
}

TEST(TransformationBuilderTest, FluentAPI) {
    auto b = TransformationBuilder().addBrightness(50).addSepia().addInvert();
    EXPECT_EQ(3, b.size());
}

TEST(TransformationBuilderTest, Clear) {
    auto b = TransformationBuilder().addGrayscale().addSharpen();
    EXPECT_EQ(2, b.size());
    b.clear();
    EXPECT_EQ(0, b.size());
}

TEST(TransformationBuilderTest, ApplyPipeline) {
    auto m = makeSolid(5,5,100,150,200);
    auto pipeline = TransformationBuilder().addGrayscale().build();
    auto r = pipeline.apply(*m);
    auto p = r->getPixel(0,0);
    EXPECT_EQ(p.r, p.g);
}

TEST(TransformationBuilderTest, EmptyPipelineNoop) {
    auto m = makeSolid(5,5,128,128,128);
    auto r = TransformationBuilder().build().apply(*m);
    EXPECT_EQ(128, r->getPixel(0,0).r);
}

TEST(TransformationBuilderTest, AllMethods) {
    auto b = TransformationBuilder()
        .addGrayscale().addSepia().addBrightness(50).addContrast(1.5)
        .addInvert().addHueShift(45).addSaturation(1.5)
        .addGaussianBlur(3).addSharpen().addEdgeDetection()
        .addEmboss().addBoxBlur(3).addMedianFilter(3)
        .addRotateCW().addRotateCCW()
        .addHorizontalFlip().addVerticalFlip()
        .addHistogramEqualization().addPosterize(4)
        .addDither().addVignette(0.5);
    EXPECT_EQ(21, b.size());
}

// ==================== CONTROLLER TESTS ====================

TEST(ImageControllerTest, InitialState) {
    ImageController c;
    EXPECT_FALSE(c.hasImage());
    EXPECT_EQ(nullptr, c.getModel());
    EXPECT_FALSE(c.canUndo());
}

TEST(ImageControllerTest, LoadAndSave) {
    // Create test image
    auto m = makeSolid(10,10,128,128,128);
    m->saveToFile("ctrl_test.png");

    ImageController c;
    c.loadImage("ctrl_test.png");
    EXPECT_TRUE(c.hasImage());
    EXPECT_EQ(10, c.getModel()->getWidth());

    c.saveImage("ctrl_test_out.png");
    EXPECT_TRUE(std::filesystem::exists("ctrl_test_out.png"));

    std::filesystem::remove("ctrl_test.png");
    std::filesystem::remove("ctrl_test_out.png");
}

TEST(ImageControllerTest, ApplyOperation) {
    auto m = makeSolid(5,5,100,100,100);
    m->saveToFile("ctrl_op_test.png");

    ImageController c;
    c.loadImage("ctrl_op_test.png");
    c.applyOperation("invert");
    EXPECT_EQ(155, c.getModel()->getPixel(0,0).r);

    std::filesystem::remove("ctrl_op_test.png");
}

TEST(ImageControllerTest, UndoRedo) {
    auto m = makeSolid(5,5,100,100,100);
    m->saveToFile("ctrl_ur_test.png");

    ImageController c;
    c.loadImage("ctrl_ur_test.png");
    auto before = c.getModel()->getPixel(0,0);
    c.applyOperation("invert");
    EXPECT_TRUE(c.canUndo());
    c.undo();
    EXPECT_EQ(before, c.getModel()->getPixel(0,0));
    c.redo();
    EXPECT_NE(before, c.getModel()->getPixel(0,0));

    std::filesystem::remove("ctrl_ur_test.png");
}

TEST(ImageControllerTest, NoImageThrows) {
    ImageController c;
    EXPECT_THROW(c.applyOperation("grayscale"), std::logic_error);
    EXPECT_THROW(c.saveImage("test.png"), std::logic_error);
}
