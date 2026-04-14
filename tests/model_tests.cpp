#include <gtest/gtest.h>
#include "model/Pixel.h"
#include "model/ImageModel.h"
#include <memory>
#include <filesystem>

using namespace imageprocessor;

// ===== Helper functions =====

std::unique_ptr<ImageModel> createSolid(int w, int h, int r, int g, int b) {
    auto m = std::make_unique<ImageModel>(w, h);
    Pixel p(r, g, b);
    for (int y=0;y<h;y++) for (int x=0;x<w;x++) m->setPixel(x, y, p);
    return m;
}

std::unique_ptr<ImageModel> createGradient(int w, int h) {
    auto m = std::make_unique<ImageModel>(w, h);
    for (int y=0;y<h;y++) for (int x=0;x<w;x++) {
        int v = 255 * x / std::max(1, w - 1);
        m->setPixel(x, y, Pixel(v, v, v));
    }
    return m;
}

std::unique_ptr<ImageModel> createCheckerboard(int w, int h, int ts) {
    auto m = std::make_unique<ImageModel>(w, h);
    for (int y=0;y<h;y++) for (int x=0;x<w;x++) {
        bool white = ((x/ts) + (y/ts)) % 2 == 0;
        m->setPixel(x, y, Pixel(white?255:0, white?255:0, white?255:0));
    }
    return m;
}

// ===== Pixel Tests =====

TEST(PixelTest, Construction) {
    Pixel p(100, 150, 200, 255);
    EXPECT_EQ(100, p.r); EXPECT_EQ(150, p.g); EXPECT_EQ(200, p.b); EXPECT_EQ(255, p.a);
}

TEST(PixelTest, DefaultAlpha) {
    Pixel p(100, 150, 200);
    EXPECT_EQ(255, p.a);
}

TEST(PixelTest, Clamping) {
    Pixel p(-10, 300, 128);
    EXPECT_EQ(0, p.r); EXPECT_EQ(255, p.g); EXPECT_EQ(128, p.b);
}

TEST(PixelTest, ClampMethod) {
    EXPECT_EQ(0, Pixel::clamp(-5));
    EXPECT_EQ(255, Pixel::clamp(300));
    EXPECT_EQ(128, Pixel::clamp(128));
}

TEST(PixelTest, ToGray) {
    Pixel white(255, 255, 255);
    EXPECT_EQ(255, white.toGray());
    Pixel black(0, 0, 0);
    EXPECT_EQ(0, black.toGray());
    Pixel red(255, 0, 0);
    EXPECT_EQ(76, red.toGray());
}

TEST(PixelTest, ToHSL_Red) {
    auto hsl = Pixel(255, 0, 0).toHSL();
    EXPECT_NEAR(0, hsl[0], 1.0);
    EXPECT_NEAR(1.0, hsl[1], 0.01);
    EXPECT_NEAR(0.5, hsl[2], 0.01);
}

TEST(PixelTest, ToHSL_Green) {
    auto hsl = Pixel(0, 255, 0).toHSL();
    EXPECT_NEAR(120, hsl[0], 1.0);
}

TEST(PixelTest, ToHSL_Blue) {
    auto hsl = Pixel(0, 0, 255).toHSL();
    EXPECT_NEAR(240, hsl[0], 1.0);
}

TEST(PixelTest, ToHSL_Gray) {
    auto hsl = Pixel(128, 128, 128).toHSL();
    EXPECT_NEAR(0, hsl[1], 0.01);
}

TEST(PixelTest, FromHSL_RoundTrip) {
    Pixel orig(200, 100, 50);
    auto hsl = orig.toHSL();
    Pixel rec = Pixel::fromHSL(hsl[0], hsl[1], hsl[2]);
    EXPECT_NEAR(orig.r, rec.r, 2);
    EXPECT_NEAR(orig.g, rec.g, 2);
    EXPECT_NEAR(orig.b, rec.b, 2);
}

TEST(PixelTest, FromHSL_WithAlpha) {
    Pixel p = Pixel::fromHSL(0, 1.0, 0.5, 128);
    EXPECT_EQ(128, p.a);
}

TEST(PixelTest, Equality) {
    EXPECT_EQ(Pixel(100, 150, 200), Pixel(100, 150, 200));
    EXPECT_NE(Pixel(100, 150, 200), Pixel(101, 150, 200));
}

// ===== ImageModel Tests =====

TEST(ImageModelTest, Creation) {
    ImageModel m(100, 50);
    EXPECT_EQ(100, m.getWidth());
    EXPECT_EQ(50, m.getHeight());
}

TEST(ImageModelTest, InvalidDimensions) {
    EXPECT_THROW(ImageModel(0, 10), std::invalid_argument);
    EXPECT_THROW(ImageModel(10, -1), std::invalid_argument);
}

TEST(ImageModelTest, PixelAccess) {
    ImageModel m(10, 10);
    Pixel p(100, 150, 200);
    m.setPixel(5, 5, p);
    EXPECT_EQ(p, m.getPixel(5, 5));
}

TEST(ImageModelTest, OutOfBounds) {
    ImageModel m(10, 10);
    EXPECT_THROW(m.getPixel(10, 5), std::out_of_range);
    EXPECT_THROW(m.getPixel(-1, 5), std::out_of_range);
}

TEST(ImageModelTest, DeepCopy) {
    auto orig = createSolid(10, 10, 100, 150, 200);
    auto copy = orig->deepCopy();
    EXPECT_EQ(orig->getPixel(0, 0), copy->getPixel(0, 0));
    copy->setPixel(0, 0, Pixel(0, 0, 0));
    EXPECT_NE(orig->getPixel(0, 0), copy->getPixel(0, 0));
}

TEST(ImageModelTest, GetPixelCount) {
    ImageModel m(10, 20);
    EXPECT_EQ(200, m.getPixelCount());
}

TEST(ImageModelTest, IsValidCoordinate) {
    ImageModel m(10, 10);
    EXPECT_TRUE(m.isValidCoordinate(0, 0));
    EXPECT_TRUE(m.isValidCoordinate(9, 9));
    EXPECT_FALSE(m.isValidCoordinate(10, 0));
    EXPECT_FALSE(m.isValidCoordinate(-1, 0));
}

TEST(ImageModelTest, SetFrom) {
    auto target = std::make_unique<ImageModel>(10, 10);
    auto source = createSolid(10, 10, 255, 0, 0);
    target->setFrom(*source);
    EXPECT_EQ(Pixel(255, 0, 0), target->getPixel(5, 5));
}

TEST(ImageModelTest, SaveAndLoad) {
    auto model = createSolid(10, 10, 128, 128, 128);
    std::string path = "test_output.png";
    model->saveToFile(path);
    auto loaded = ImageModel::loadFromFile(path);
    EXPECT_EQ(10, loaded->getWidth());
    EXPECT_EQ(10, loaded->getHeight());
    std::filesystem::remove(path);
}

TEST(ImageModelTest, LoadNonExistent) {
    EXPECT_THROW(ImageModel::loadFromFile("nonexistent.png"), std::runtime_error);
}
