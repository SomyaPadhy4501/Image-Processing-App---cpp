#pragma once

#include "operations/ImageOperation.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <numeric>
#include <sstream>

namespace imageprocessor {

// ==================== COLOR OPERATIONS ====================

class GrayscaleOperation : public ImageOperation {
public:
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                Pixel p = src.getPixel(x, y);
                int g = p.toGray();
                result->setPixel(x, y, Pixel(g, g, g, p.a));
            }
        return result;
    }
    std::string getName() const override { return "Grayscale"; }
};

class SepiaOperation : public ImageOperation {
public:
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                Pixel p = src.getPixel(x, y);
                int nr = static_cast<int>(0.393*p.r + 0.769*p.g + 0.189*p.b);
                int ng = static_cast<int>(0.349*p.r + 0.686*p.g + 0.168*p.b);
                int nb = static_cast<int>(0.272*p.r + 0.534*p.g + 0.131*p.b);
                result->setPixel(x, y, Pixel(nr, ng, nb, p.a));
            }
        return result;
    }
    std::string getName() const override { return "Sepia"; }
};

class BrightnessOperation : public ImageOperation {
    int delta_;
public:
    explicit BrightnessOperation(int delta) : delta_(std::max(-255, std::min(255, delta))) {}
    int getDelta() const { return delta_; }
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                Pixel p = src.getPixel(x, y);
                result->setPixel(x, y, Pixel(p.r+delta_, p.g+delta_, p.b+delta_, p.a));
            }
        return result;
    }
    std::string getName() const override { return "Brightness (" + std::to_string(delta_) + ")"; }
};

class ContrastOperation : public ImageOperation {
    double factor_;
public:
    explicit ContrastOperation(double f) : factor_(std::max(0.0, f)) {}
    double getFactor() const { return factor_; }
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                Pixel p = src.getPixel(x, y);
                int nr = static_cast<int>(std::round(factor_*(p.r-128)+128));
                int ng = static_cast<int>(std::round(factor_*(p.g-128)+128));
                int nb = static_cast<int>(std::round(factor_*(p.b-128)+128));
                result->setPixel(x, y, Pixel(nr, ng, nb, p.a));
            }
        return result;
    }
    std::string getName() const override { return "Contrast"; }
};

class InvertOperation : public ImageOperation {
public:
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                Pixel p = src.getPixel(x, y);
                result->setPixel(x, y, Pixel(255-p.r, 255-p.g, 255-p.b, p.a));
            }
        return result;
    }
    std::string getName() const override { return "Invert"; }
};

enum class Channel { RED, GREEN, BLUE };

class ChannelIsolationOperation : public ImageOperation {
    Channel ch_;
public:
    explicit ChannelIsolationOperation(Channel ch) : ch_(ch) {}
    Channel getChannel() const { return ch_; }
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                Pixel p = src.getPixel(x, y);
                int r = ch_ == Channel::RED   ? p.r : 0;
                int g = ch_ == Channel::GREEN ? p.g : 0;
                int b = ch_ == Channel::BLUE  ? p.b : 0;
                result->setPixel(x, y, Pixel(r, g, b, p.a));
            }
        return result;
    }
    std::string getName() const override { return "Channel Isolation"; }
};

class HueShiftOperation : public ImageOperation {
    double degrees_;
public:
    explicit HueShiftOperation(double deg) : degrees_(deg) {}
    double getDegrees() const { return degrees_; }
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                Pixel p = src.getPixel(x, y);
                auto hsl = p.toHSL();
                double newH = std::fmod(hsl[0] + degrees_, 360.0);
                if (newH < 0) newH += 360;
                result->setPixel(x, y, Pixel::fromHSL(newH, hsl[1], hsl[2], p.a));
            }
        return result;
    }
    std::string getName() const override { return "Hue Shift"; }
};

class SaturationOperation : public ImageOperation {
    double factor_;
public:
    explicit SaturationOperation(double f) : factor_(std::max(0.0, f)) {}
    double getFactor() const { return factor_; }
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                Pixel p = src.getPixel(x, y);
                auto hsl = p.toHSL();
                double newS = std::min(1.0, hsl[1] * factor_);
                result->setPixel(x, y, Pixel::fromHSL(hsl[0], newS, hsl[2], p.a));
            }
        return result;
    }
    std::string getName() const override { return "Saturation"; }
};

// ==================== FILTER OPERATIONS ====================

class GaussianBlurOperation : public ImageOperation {
    int kernelSize_;
    double sigma_;
    std::vector<std::vector<double>> kernel_;

    void generateKernel() {
        kernel_.resize(kernelSize_, std::vector<double>(kernelSize_));
        int off = kernelSize_ / 2;
        double sum = 0;
        for (int y = 0; y < kernelSize_; y++)
            for (int x = 0; x < kernelSize_; x++) {
                int dx = x - off, dy = y - off;
                kernel_[y][x] = std::exp(-(dx*dx + dy*dy) / (2*sigma_*sigma_));
                sum += kernel_[y][x];
            }
        for (auto& row : kernel_) for (auto& v : row) v /= sum;
    }
public:
    explicit GaussianBlurOperation(int ks = 3, double sigma = 1.0)
        : kernelSize_(ks), sigma_(sigma) {
        if (ks < 3 || ks % 2 == 0) throw std::invalid_argument("Kernel must be odd >= 3");
        if (sigma <= 0) throw std::invalid_argument("Sigma must be positive");
        generateKernel();
    }
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        int off = kernelSize_ / 2;
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                double rs=0,gs=0,bs=0;
                for (int ky = 0; ky < kernelSize_; ky++)
                    for (int kx = 0; kx < kernelSize_; kx++) {
                        int px = std::clamp(x+kx-off, 0, src.getWidth()-1);
                        int py = std::clamp(y+ky-off, 0, src.getHeight()-1);
                        Pixel p = src.getPixel(px, py);
                        rs += p.r * kernel_[ky][kx];
                        gs += p.g * kernel_[ky][kx];
                        bs += p.b * kernel_[ky][kx];
                    }
                Pixel orig = src.getPixel(x, y);
                result->setPixel(x, y, Pixel((int)std::round(rs),(int)std::round(gs),(int)std::round(bs),orig.a));
            }
        return result;
    }
    std::string getName() const override { return "Gaussian Blur"; }
};

class SharpenOperation : public ImageOperation {
public:
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        static const double K[3][3] = {{0,-1,0},{-1,5,-1},{0,-1,0}};
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                double rs=0,gs=0,bs=0;
                for (int ky=-1;ky<=1;ky++) for (int kx=-1;kx<=1;kx++) {
                    int px=std::clamp(x+kx,0,src.getWidth()-1);
                    int py=std::clamp(y+ky,0,src.getHeight()-1);
                    Pixel p=src.getPixel(px,py);
                    rs+=p.r*K[ky+1][kx+1]; gs+=p.g*K[ky+1][kx+1]; bs+=p.b*K[ky+1][kx+1];
                }
                Pixel orig=src.getPixel(x,y);
                result->setPixel(x,y,Pixel((int)std::round(rs),(int)std::round(gs),(int)std::round(bs),orig.a));
            }
        return result;
    }
    std::string getName() const override { return "Sharpen"; }
};

class EdgeDetectionOperation : public ImageOperation {
public:
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        static const int SX[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
        static const int SY[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                double gxR=0,gxG=0,gxB=0, gyR=0,gyG=0,gyB=0;
                for (int ky=-1;ky<=1;ky++) for (int kx=-1;kx<=1;kx++) {
                    int px=std::clamp(x+kx,0,src.getWidth()-1);
                    int py=std::clamp(y+ky,0,src.getHeight()-1);
                    Pixel p=src.getPixel(px,py);
                    gxR+=p.r*SX[ky+1][kx+1]; gxG+=p.g*SX[ky+1][kx+1]; gxB+=p.b*SX[ky+1][kx+1];
                    gyR+=p.r*SY[ky+1][kx+1]; gyG+=p.g*SY[ky+1][kx+1]; gyB+=p.b*SY[ky+1][kx+1];
                }
                Pixel orig=src.getPixel(x,y);
                result->setPixel(x,y,Pixel(
                    std::min(255,(int)std::sqrt(gxR*gxR+gyR*gyR)),
                    std::min(255,(int)std::sqrt(gxG*gxG+gyG*gyG)),
                    std::min(255,(int)std::sqrt(gxB*gxB+gyB*gyB)), orig.a));
            }
        return result;
    }
    std::string getName() const override { return "Edge Detection (Sobel)"; }
};

class EmbossOperation : public ImageOperation {
public:
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        static const double K[3][3] = {{-2,-1,0},{-1,1,1},{0,1,2}};
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                double rs=0,gs=0,bs=0;
                for (int ky=-1;ky<=1;ky++) for (int kx=-1;kx<=1;kx++) {
                    int px=std::clamp(x+kx,0,src.getWidth()-1);
                    int py=std::clamp(y+ky,0,src.getHeight()-1);
                    Pixel p=src.getPixel(px,py);
                    rs+=p.r*K[ky+1][kx+1]; gs+=p.g*K[ky+1][kx+1]; bs+=p.b*K[ky+1][kx+1];
                }
                Pixel orig=src.getPixel(x,y);
                result->setPixel(x,y,Pixel((int)std::round(rs),(int)std::round(gs),(int)std::round(bs),orig.a));
            }
        return result;
    }
    std::string getName() const override { return "Emboss"; }
};

class BoxBlurOperation : public ImageOperation {
    int size_;
public:
    explicit BoxBlurOperation(int s = 3) : size_(s) {
        if (s < 3 || s % 2 == 0) throw std::invalid_argument("Size must be odd >= 3");
    }
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        int off = size_ / 2;
        double weight = 1.0 / (size_ * size_);
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                double rs=0,gs=0,bs=0;
                for (int ky=-off;ky<=off;ky++) for (int kx=-off;kx<=off;kx++) {
                    int px=std::clamp(x+kx,0,src.getWidth()-1);
                    int py=std::clamp(y+ky,0,src.getHeight()-1);
                    Pixel p=src.getPixel(px,py);
                    rs+=p.r*weight; gs+=p.g*weight; bs+=p.b*weight;
                }
                Pixel orig=src.getPixel(x,y);
                result->setPixel(x,y,Pixel((int)std::round(rs),(int)std::round(gs),(int)std::round(bs),orig.a));
            }
        return result;
    }
    std::string getName() const override { return "Box Blur"; }
};

class MedianFilterOperation : public ImageOperation {
    int size_;
public:
    explicit MedianFilterOperation(int s = 3) : size_(s) {
        if (s < 3 || s % 2 == 0) throw std::invalid_argument("Size must be odd >= 3");
    }
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        int off = size_ / 2;
        int count = size_ * size_;
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++) {
                std::vector<int> rv(count), gv(count), bv(count);
                int idx = 0;
                for (int ky=-off;ky<=off;ky++) for (int kx=-off;kx<=off;kx++) {
                    int px=std::clamp(x+kx,0,src.getWidth()-1);
                    int py=std::clamp(y+ky,0,src.getHeight()-1);
                    Pixel p=src.getPixel(px,py);
                    rv[idx]=p.r; gv[idx]=p.g; bv[idx]=p.b; idx++;
                }
                std::sort(rv.begin(),rv.end());
                std::sort(gv.begin(),gv.end());
                std::sort(bv.begin(),bv.end());
                Pixel orig=src.getPixel(x,y);
                result->setPixel(x,y,Pixel(rv[count/2],gv[count/2],bv[count/2],orig.a));
            }
        return result;
    }
    std::string getName() const override { return "Median Filter"; }
};

// ==================== TRANSFORM OPERATIONS ====================

class RotateCWOperation : public ImageOperation {
public:
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getHeight(), src.getWidth());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++)
                result->setPixel(src.getHeight()-1-y, x, src.getPixel(x, y));
        return result;
    }
    std::string getName() const override { return "Rotate 90 CW"; }
};

class RotateCCWOperation : public ImageOperation {
public:
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getHeight(), src.getWidth());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++)
                result->setPixel(y, src.getWidth()-1-x, src.getPixel(x, y));
        return result;
    }
    std::string getName() const override { return "Rotate 90 CCW"; }
};

class HorizontalFlipOperation : public ImageOperation {
public:
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++)
                result->setPixel(src.getWidth()-1-x, y, src.getPixel(x, y));
        return result;
    }
    std::string getName() const override { return "Horizontal Flip"; }
};

class VerticalFlipOperation : public ImageOperation {
public:
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y = 0; y < src.getHeight(); y++)
            for (int x = 0; x < src.getWidth(); x++)
                result->setPixel(x, src.getHeight()-1-y, src.getPixel(x, y));
        return result;
    }
    std::string getName() const override { return "Vertical Flip"; }
};

class CropOperation : public ImageOperation {
    int sx_, sy_, cw_, ch_;
public:
    CropOperation(int sx, int sy, int cw, int ch) : sx_(sx), sy_(sy), cw_(cw), ch_(ch) {
        if (cw <= 0 || ch <= 0) throw std::invalid_argument("Crop dims must be positive");
        if (sx < 0 || sy < 0) throw std::invalid_argument("Start must be non-negative");
    }
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        int maxX = std::min(sx_ + cw_, src.getWidth());
        int maxY = std::min(sy_ + ch_, src.getHeight());
        int aw = maxX - sx_, ah = maxY - sy_;
        if (aw <= 0 || ah <= 0) throw std::invalid_argument("Crop region outside bounds");
        auto result = std::make_unique<ImageModel>(aw, ah);
        for (int y = 0; y < ah; y++)
            for (int x = 0; x < aw; x++)
                result->setPixel(x, y, src.getPixel(sx_+x, sy_+y));
        return result;
    }
    std::string getName() const override { return "Crop"; }
};

class ResizeOperation : public ImageOperation {
    int nw_, nh_;
public:
    ResizeOperation(int nw, int nh) : nw_(nw), nh_(nh) {
        if (nw <= 0 || nh <= 0) throw std::invalid_argument("Dimensions must be positive");
    }
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(nw_, nh_);
        for (int y = 0; y < nh_; y++)
            for (int x = 0; x < nw_; x++) {
                double srcX = nw_ == 1 ? 0 : (double)x*(src.getWidth()-1)/(nw_-1);
                double srcY = nh_ == 1 ? 0 : (double)y*(src.getHeight()-1)/(nh_-1);
                int x0=(int)std::floor(srcX), y0=(int)std::floor(srcY);
                int x1=std::min(x0+1,src.getWidth()-1), y1=std::min(y0+1,src.getHeight()-1);
                double xf=srcX-x0, yf=srcY-y0;
                Pixel p00=src.getPixel(x0,y0), p10=src.getPixel(x1,y0);
                Pixel p01=src.getPixel(x0,y1), p11=src.getPixel(x1,y1);
                auto bi = [&](int a, int b, int c, int d) {
                    double top = a + (b-a)*xf, bot = c + (d-c)*xf;
                    return (int)std::round(top + (bot-top)*yf);
                };
                result->setPixel(x, y, Pixel(
                    bi(p00.r,p10.r,p01.r,p11.r), bi(p00.g,p10.g,p01.g,p11.g),
                    bi(p00.b,p10.b,p01.b,p11.b), bi(p00.a,p10.a,p01.a,p11.a)));
            }
        return result;
    }
    std::string getName() const override { return "Resize"; }
};

// ==================== ADVANCED OPERATIONS ====================

class HistogramEqualizationOperation : public ImageOperation {
public:
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        int total = src.getPixelCount();
        int histR[256]={}, histG[256]={}, histB[256]={};
        for (int y=0;y<src.getHeight();y++)
            for (int x=0;x<src.getWidth();x++) {
                Pixel p=src.getPixel(x,y);
                histR[p.r]++; histG[p.g]++; histB[p.b]++;
            }
        auto buildCDF = [&](int hist[256]) {
            std::vector<int> cdf(256);
            int cum[256]; cum[0]=hist[0];
            for (int i=1;i<256;i++) cum[i]=cum[i-1]+hist[i];
            int cmin=0;
            for (int i=0;i<256;i++) if (cum[i]>0) { cmin=cum[i]; break; }
            for (int i=0;i<256;i++)
                cdf[i] = total==cmin ? i : std::clamp((int)std::round(255.0*(cum[i]-cmin)/(total-cmin)),0,255);
            return cdf;
        };
        auto cdfR=buildCDF(histR), cdfG=buildCDF(histG), cdfB=buildCDF(histB);
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        for (int y=0;y<src.getHeight();y++)
            for (int x=0;x<src.getWidth();x++) {
                Pixel p=src.getPixel(x,y);
                result->setPixel(x,y,Pixel(cdfR[p.r],cdfG[p.g],cdfB[p.b],p.a));
            }
        return result;
    }
    std::string getName() const override { return "Histogram Equalization"; }
};

class PosterizeOperation : public ImageOperation {
    int levels_;
public:
    explicit PosterizeOperation(int levels) : levels_(levels) {
        if (levels < 2 || levels > 256) throw std::invalid_argument("Levels must be 2-256");
    }
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        auto result = std::make_unique<ImageModel>(src.getWidth(), src.getHeight());
        double step = 256.0 / levels_;
        for (int y=0;y<src.getHeight();y++)
            for (int x=0;x<src.getWidth();x++) {
                Pixel p=src.getPixel(x,y);
                auto post = [&](int v) { return std::min(255,(int)((int)(v/step)*step + step/2)); };
                result->setPixel(x,y,Pixel(post(p.r),post(p.g),post(p.b),p.a));
            }
        return result;
    }
    std::string getName() const override { return "Posterize"; }
};

class DitherOperation : public ImageOperation {
public:
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        int w=src.getWidth(), h=src.getHeight();
        std::vector<std::vector<double>> gray(h, std::vector<double>(w));
        for (int y=0;y<h;y++) for (int x=0;x<w;x++) gray[y][x]=src.getPixel(x,y).toGray();
        auto result = std::make_unique<ImageModel>(w, h);
        for (int y=0;y<h;y++) for (int x=0;x<w;x++) {
            double old=gray[y][x];
            int nw = old < 128 ? 0 : 255;
            double err = old - nw;
            Pixel orig=src.getPixel(x,y);
            result->setPixel(x,y,Pixel(nw,nw,nw,orig.a));
            if (x+1<w) gray[y][x+1]+=err*7.0/16.0;
            if (y+1<h) {
                if (x-1>=0) gray[y+1][x-1]+=err*3.0/16.0;
                gray[y+1][x]+=err*5.0/16.0;
                if (x+1<w) gray[y+1][x+1]+=err*1.0/16.0;
            }
        }
        return result;
    }
    std::string getName() const override { return "Dither (Floyd-Steinberg)"; }
};

class VignetteOperation : public ImageOperation {
    double intensity_;
public:
    explicit VignetteOperation(double i = 0.5) : intensity_(std::clamp(i, 0.0, 2.0)) {}
    std::unique_ptr<ImageModel> apply(const ImageModel& src) override {
        int w=src.getWidth(), h=src.getHeight();
        double cx=w/2.0, cy=h/2.0, maxD=std::sqrt(cx*cx+cy*cy);
        auto result = std::make_unique<ImageModel>(w, h);
        for (int y=0;y<h;y++) for (int x=0;x<w;x++) {
            double dx=x-cx, dy=y-cy;
            double nd=std::sqrt(dx*dx+dy*dy)/maxD;
            double f=std::max(0.0, 1.0 - intensity_*nd*nd);
            Pixel p=src.getPixel(x,y);
            result->setPixel(x,y,Pixel((int)(p.r*f),(int)(p.g*f),(int)(p.b*f),p.a));
        }
        return result;
    }
    std::string getName() const override { return "Vignette"; }
};

} // namespace imageprocessor
