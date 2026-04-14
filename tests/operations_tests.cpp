#include <gtest/gtest.h>
#include "operations/AllOperations.h"
#include <memory>

using namespace imageprocessor;

// ===== Helper =====
std::unique_ptr<ImageModel> solid(int w, int h, int r, int g, int b) {
    auto m = std::make_unique<ImageModel>(w, h);
    Pixel p(r, g, b);
    for (int y=0;y<h;y++) for (int x=0;x<w;x++) m->setPixel(x, y, p);
    return m;
}

std::unique_ptr<ImageModel> gradient(int w, int h) {
    auto m = std::make_unique<ImageModel>(w, h);
    for (int y=0;y<h;y++) for (int x=0;x<w;x++) {
        int v = w > 1 ? 255*x/(w-1) : 128;
        m->setPixel(x, y, Pixel(v, v, v));
    }
    return m;
}

std::unique_ptr<ImageModel> checker(int w, int h) {
    auto m = std::make_unique<ImageModel>(w, h);
    for (int y=0;y<h;y++) for (int x=0;x<w;x++) {
        bool white = (x+y) % 2 == 0;
        m->setPixel(x, y, Pixel(white?255:0, white?255:0, white?255:0));
    }
    return m;
}

// ==================== COLOR TESTS ====================

TEST(GrayscaleTest, White) {
    auto r = GrayscaleOperation().apply(*solid(3,3,255,255,255));
    auto p = r->getPixel(0,0);
    EXPECT_EQ(255, p.r); EXPECT_EQ(p.r, p.g); EXPECT_EQ(p.g, p.b);
}
TEST(GrayscaleTest, Black) { EXPECT_EQ(0, GrayscaleOperation().apply(*solid(3,3,0,0,0))->getPixel(0,0).r); }
TEST(GrayscaleTest, Red) { EXPECT_EQ(76, GrayscaleOperation().apply(*solid(3,3,255,0,0))->getPixel(0,0).r); }
TEST(GrayscaleTest, PreservesAlpha) {
    auto m = std::make_unique<ImageModel>(1,1); m->setPixel(0,0,Pixel(100,150,200,128));
    EXPECT_EQ(128, GrayscaleOperation().apply(*m)->getPixel(0,0).a);
}
TEST(GrayscaleTest, Name) { EXPECT_EQ("Grayscale", GrayscaleOperation().getName()); }

TEST(SepiaTest, Black) { auto p = SepiaOperation().apply(*solid(3,3,0,0,0))->getPixel(0,0); EXPECT_EQ(0, p.r); }
TEST(SepiaTest, WarmTone) { auto p = SepiaOperation().apply(*solid(3,3,128,128,128))->getPixel(0,0); EXPECT_GT(p.r, p.b); }
TEST(SepiaTest, Clamps) { auto p = SepiaOperation().apply(*solid(3,3,255,255,255))->getPixel(0,0); EXPECT_LE(p.r, 255); }

TEST(BrightnessTest, Increase) { EXPECT_EQ(150, BrightnessOperation(50).apply(*solid(3,3,100,100,100))->getPixel(0,0).r); }
TEST(BrightnessTest, Decrease) { EXPECT_EQ(50, BrightnessOperation(-50).apply(*solid(3,3,100,100,100))->getPixel(0,0).r); }
TEST(BrightnessTest, Zero) { EXPECT_EQ(100, BrightnessOperation(0).apply(*solid(3,3,100,100,100))->getPixel(0,0).r); }
TEST(BrightnessTest, ClampHigh) { EXPECT_EQ(255, BrightnessOperation(50).apply(*solid(3,3,250,250,250))->getPixel(0,0).r); }
TEST(BrightnessTest, ClampLow) { EXPECT_EQ(0, BrightnessOperation(-50).apply(*solid(3,3,10,10,10))->getPixel(0,0).r); }
TEST(BrightnessTest, GetDelta) { EXPECT_EQ(50, BrightnessOperation(50).getDelta()); }
TEST(BrightnessTest, DeltaClamped) { EXPECT_EQ(255, BrightnessOperation(300).getDelta()); }

TEST(ContrastTest, Increase) { EXPECT_GT(ContrastOperation(2.0).apply(*solid(3,3,200,200,200))->getPixel(0,0).r, 200); }
TEST(ContrastTest, MidGray) { EXPECT_EQ(128, ContrastOperation(2.0).apply(*solid(3,3,128,128,128))->getPixel(0,0).r); }
TEST(ContrastTest, Zero) { EXPECT_EQ(128, ContrastOperation(0.0).apply(*solid(3,3,200,200,200))->getPixel(0,0).r); }

TEST(InvertTest, Black) { EXPECT_EQ(255, InvertOperation().apply(*solid(3,3,0,0,0))->getPixel(0,0).r); }
TEST(InvertTest, White) { EXPECT_EQ(0, InvertOperation().apply(*solid(3,3,255,255,255))->getPixel(0,0).r); }
TEST(InvertTest, DoubleIdentity) {
    auto m = solid(3,3,100,150,200);
    auto r = InvertOperation().apply(*InvertOperation().apply(*m));
    EXPECT_EQ(m->getPixel(0,0), r->getPixel(0,0));
}

TEST(ChannelTest, Red) {
    auto r = ChannelIsolationOperation(Channel::RED).apply(*solid(3,3,100,150,200));
    EXPECT_EQ(100, r->getPixel(0,0).r); EXPECT_EQ(0, r->getPixel(0,0).g);
}
TEST(ChannelTest, Green) {
    auto r = ChannelIsolationOperation(Channel::GREEN).apply(*solid(3,3,100,150,200));
    EXPECT_EQ(0, r->getPixel(0,0).r); EXPECT_EQ(150, r->getPixel(0,0).g);
}
TEST(ChannelTest, Blue) {
    auto r = ChannelIsolationOperation(Channel::BLUE).apply(*solid(3,3,100,150,200));
    EXPECT_EQ(0, r->getPixel(0,0).r); EXPECT_EQ(200, r->getPixel(0,0).b);
}

TEST(HueShiftTest, Zero) {
    auto r = HueShiftOperation(0).apply(*solid(3,3,255,0,0));
    EXPECT_NEAR(255, r->getPixel(0,0).r, 2);
}
TEST(HueShiftTest, Shift120) {
    auto r = HueShiftOperation(120).apply(*solid(3,3,255,0,0));
    EXPECT_NEAR(0, r->getPixel(0,0).r, 5);
    EXPECT_NEAR(255, r->getPixel(0,0).g, 5);
}
TEST(HueShiftTest, Shift360Identity) {
    auto r = HueShiftOperation(360).apply(*solid(3,3,200,100,50));
    EXPECT_NEAR(200, r->getPixel(0,0).r, 2);
}

TEST(SaturationTest, Zero) {
    auto r = SaturationOperation(0).apply(*solid(3,3,255,0,0));
    auto p = r->getPixel(0,0);
    EXPECT_NEAR(p.r, p.g, 2); EXPECT_NEAR(p.g, p.b, 2);
}
TEST(SaturationTest, Identity) {
    auto r = SaturationOperation(1.0).apply(*solid(3,3,200,100,50));
    EXPECT_NEAR(200, r->getPixel(0,0).r, 2);
}

// ==================== FILTER TESTS ====================

TEST(GaussianBlurTest, UniformUnchanged) {
    EXPECT_NEAR(128, GaussianBlurOperation(3).apply(*solid(5,5,128,128,128))->getPixel(2,2).r, 1);
}
TEST(GaussianBlurTest, BlursEdges) {
    auto r = GaussianBlurOperation(3).apply(*checker(10,10));
    EXPECT_GT(r->getPixel(5,5).r, 0); EXPECT_LT(r->getPixel(5,5).r, 255);
}
TEST(GaussianBlurTest, InvalidKernel) {
    EXPECT_THROW(GaussianBlurOperation(2), std::invalid_argument);
}
TEST(GaussianBlurTest, InvalidSigma) {
    EXPECT_THROW(GaussianBlurOperation(3, 0), std::invalid_argument);
}

TEST(SharpenTest, UniformUnchanged) {
    EXPECT_NEAR(100, SharpenOperation().apply(*solid(5,5,100,100,100))->getPixel(2,2).r, 1);
}
TEST(SharpenTest, PreservesDim) {
    auto r = SharpenOperation().apply(*solid(7,5,128,128,128));
    EXPECT_EQ(7, r->getWidth()); EXPECT_EQ(5, r->getHeight());
}

TEST(EdgeDetectionTest, UniformNoEdges) {
    EXPECT_NEAR(0, EdgeDetectionOperation().apply(*solid(5,5,128,128,128))->getPixel(2,2).r, 1);
}
TEST(EdgeDetectionTest, GradientHasEdges) {
    EXPECT_GT(EdgeDetectionOperation().apply(*gradient(10,10))->getPixel(5,5).r, 0);
}
TEST(EdgeDetectionTest, PreservesDim) {
    auto r = EdgeDetectionOperation().apply(*solid(8,6,0,0,0));
    EXPECT_EQ(8, r->getWidth()); EXPECT_EQ(6, r->getHeight());
}

TEST(EmbossTest, PreservesDim) {
    auto r = EmbossOperation().apply(*solid(6,4,100,100,100));
    EXPECT_EQ(6, r->getWidth()); EXPECT_EQ(4, r->getHeight());
}

TEST(BoxBlurTest, UniformUnchanged) {
    EXPECT_NEAR(100, BoxBlurOperation(3).apply(*solid(5,5,100,100,100))->getPixel(2,2).r, 1);
}
TEST(BoxBlurTest, Invalid) { EXPECT_THROW(BoxBlurOperation(2), std::invalid_argument); }

TEST(MedianFilterTest, UniformUnchanged) {
    EXPECT_EQ(100, MedianFilterOperation(3).apply(*solid(5,5,100,100,100))->getPixel(2,2).r);
}
TEST(MedianFilterTest, RemovesNoise) {
    auto m = solid(5,5,128,128,128);
    m->setPixel(2,2,Pixel(0,0,0));
    EXPECT_EQ(128, MedianFilterOperation(3).apply(*m)->getPixel(2,2).r);
}
TEST(MedianFilterTest, Invalid) { EXPECT_THROW(MedianFilterOperation(2), std::invalid_argument); }

// ==================== TRANSFORM TESTS ====================

TEST(RotateCWTest, DimensionsSwapped) {
    auto r = RotateCWOperation().apply(ImageModel(4,3));
    EXPECT_EQ(3, r->getWidth()); EXPECT_EQ(4, r->getHeight());
}
TEST(RotateCWTest, FourRotationsIdentity) {
    auto m = solid(4,4,100,150,200);
    RotateCWOperation op;
    auto r = op.apply(*op.apply(*op.apply(*op.apply(*m))));
    EXPECT_EQ(m->getPixel(1,2), r->getPixel(1,2));
}

TEST(RotateCCWTest, DimensionsSwapped) {
    auto r = RotateCCWOperation().apply(ImageModel(4,3));
    EXPECT_EQ(3, r->getWidth()); EXPECT_EQ(4, r->getHeight());
}
TEST(RotateCCWTest, CWThenCCWIsIdentity) {
    auto m = solid(4,3,100,150,200);
    auto r = RotateCCWOperation().apply(*RotateCWOperation().apply(*m));
    for (int y=0;y<3;y++) for (int x=0;x<4;x++)
        EXPECT_EQ(m->getPixel(x,y), r->getPixel(x,y));
}

TEST(HFlipTest, DoubleFlipIdentity) {
    auto m = solid(5,5,100,150,200); m->setPixel(0,0,Pixel(1,2,3));
    HorizontalFlipOperation op;
    auto r = op.apply(*op.apply(*m));
    EXPECT_EQ(m->getPixel(0,0), r->getPixel(0,0));
}

TEST(VFlipTest, DoubleFlipIdentity) {
    auto m = solid(5,5,100,150,200); m->setPixel(0,0,Pixel(1,2,3));
    VerticalFlipOperation op;
    auto r = op.apply(*op.apply(*m));
    EXPECT_EQ(m->getPixel(0,0), r->getPixel(0,0));
}

TEST(CropTest, BasicCrop) {
    auto m = solid(10,10,128,128,128);
    auto r = CropOperation(2,2,5,5).apply(*m);
    EXPECT_EQ(5, r->getWidth()); EXPECT_EQ(5, r->getHeight());
}
TEST(CropTest, InvalidDims) { EXPECT_THROW(CropOperation(0,0,0,5), std::invalid_argument); }
TEST(CropTest, NegativeStart) { EXPECT_THROW(CropOperation(-1,0,5,5), std::invalid_argument); }
TEST(CropTest, OutOfBounds) {
    auto m = solid(5,5,128,128,128);
    EXPECT_THROW(CropOperation(10,10,5,5).apply(*m), std::invalid_argument);
}

TEST(ResizeTest, Upscale) {
    auto r = ResizeOperation(10,10).apply(*solid(5,5,100,100,100));
    EXPECT_EQ(10, r->getWidth()); EXPECT_NEAR(100, r->getPixel(5,5).r, 1);
}
TEST(ResizeTest, Downscale) {
    auto r = ResizeOperation(5,5).apply(*solid(10,10,100,100,100));
    EXPECT_EQ(5, r->getWidth()); EXPECT_EQ(5, r->getHeight());
}
TEST(ResizeTest, InvalidDims) { EXPECT_THROW(ResizeOperation(0,5), std::invalid_argument); }

// ==================== ADVANCED TESTS ====================

TEST(HistEqTest, PreservesDim) {
    auto r = HistogramEqualizationOperation().apply(*gradient(10,10));
    EXPECT_EQ(10, r->getWidth());
}
TEST(HistEqTest, AllBlack) {
    EXPECT_EQ(0, HistogramEqualizationOperation().apply(*solid(5,5,0,0,0))->getPixel(2,2).r);
}
TEST(HistEqTest, PreservesAlpha) {
    auto m = std::make_unique<ImageModel>(3,3);
    for (int y=0;y<3;y++) for (int x=0;x<3;x++) m->setPixel(x,y,Pixel(128,128,128,100));
    EXPECT_EQ(100, HistogramEqualizationOperation().apply(*m)->getPixel(1,1).a);
}

TEST(PosterizeTest, InvalidLevels) {
    EXPECT_THROW(PosterizeOperation(1), std::invalid_argument);
    EXPECT_THROW(PosterizeOperation(257), std::invalid_argument);
}
TEST(PosterizeTest, PreservesDim) {
    auto r = PosterizeOperation(4).apply(*solid(8,4,128,128,128));
    EXPECT_EQ(8, r->getWidth()); EXPECT_EQ(4, r->getHeight());
}
TEST(PosterizeTest, PreservesAlpha) {
    auto m = std::make_unique<ImageModel>(1,1); m->setPixel(0,0,Pixel(128,128,128,64));
    EXPECT_EQ(64, PosterizeOperation(4).apply(*m)->getPixel(0,0).a);
}

TEST(DitherTest, BlackStaysBlack) { EXPECT_EQ(0, DitherOperation().apply(*solid(5,5,0,0,0))->getPixel(2,2).r); }
TEST(DitherTest, WhiteStaysWhite) { EXPECT_EQ(255, DitherOperation().apply(*solid(5,5,255,255,255))->getPixel(2,2).r); }
TEST(DitherTest, OutputBinary) {
    auto r = DitherOperation().apply(*gradient(20,20));
    for (int y=0;y<20;y++) for (int x=0;x<20;x++) {
        int v = r->getPixel(x,y).r;
        EXPECT_TRUE(v == 0 || v == 255);
    }
}

TEST(VignetteTest, CenterBrighter) {
    auto r = VignetteOperation(1.0).apply(*solid(10,10,200,200,200));
    EXPECT_GE(r->getPixel(5,5).r, r->getPixel(0,0).r);
}
TEST(VignetteTest, ZeroIntensity) {
    auto r = VignetteOperation(0.0).apply(*solid(5,5,200,200,200));
    EXPECT_NEAR(200, r->getPixel(0,0).r, 1);
}
TEST(VignetteTest, PreservesDim) {
    auto r = VignetteOperation(0.5).apply(*solid(8,6,128,128,128));
    EXPECT_EQ(8, r->getWidth()); EXPECT_EQ(6, r->getHeight());
}
