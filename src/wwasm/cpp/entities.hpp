#pragma once
#include "canvas.hpp"

namespace wwasm {
struct Line : public Canvas::Entity {
  using Pt = typename Canvas::Pt;
  using CTR = typename Canvas::CTR;

  Line(Pt a, Pt b, Col col = Dracula::red) : a_(a), b_(b), col_(col) {}
  virtual void render(CTR canvas) override {
    drawLine(canvas, a_.real(), a_.imag(), b_.real(), b_.imag(), col_.r, col_.g,
             col_.b);
  }

  Pt a_, b_;
  Col col_;

 private:
  void drawLine(CTR canvas, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g,
                uint8_t b, float size = 0.5, size_t iterations = 3) {
    x0 -= canvas.x_;
    x1 -= canvas.x_;
    y0 -= canvas.y_;
    y1 -= canvas.y_;

    x0 *= canvas.zoom_;
    x1 *= canvas.zoom_;
    y0 *= canvas.zoom_;
    y1 *= canvas.zoom_;

    x0 += canvas.w_ / 2;
    x1 += canvas.w_ / 2;
    y0 += canvas.h_ / 2;
    y1 += canvas.h_ / 2;

    for (float i = -size; i < size; i += 2 * size / iterations) {
      for (float j = -size; j < size; j += 2 * size / iterations) {
        putLine(canvas, x0 + i, y0 + j, x1 + i, y1 + j, r, g, b);
        putLine(canvas, x0, y0, x1, y1, r, g, b);
      }
    }
  }

  /* https://rosettacode.org/wiki/Xiaolin_Wu%27s_line_algorithm#C.2B.2B */
  void putLine(CTR canvas, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g,
               uint8_t b) {
    auto ipart = [](float x) -> int { return int(std::floor(x)); };
    auto round = [](float x) -> float { return std::round(x); };
    auto fpart = [](float x) -> float { return x - std::floor(x); };
    auto rfpart = [=](float x) -> float { return 1 - fpart(x); };

    const bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
      std::swap(x0, y0);
      std::swap(x1, y1);
    }
    if (x0 > x1) {
      std::swap(x0, x1);
      std::swap(y0, y1);
    }

    const float dx = x1 - x0;
    const float dy = y1 - y0;
    const float gradient = (dx == 0) ? 1 : dy / dx;

    int xpx11;
    float intery;
    {
      const float xend = round(x0);
      const float yend = y0 + gradient * (xend - x0);
      const float xgap = rfpart(x0 + 0.5);
      xpx11 = int(xend);
      const int ypx11 = ipart(yend);
      if (steep) {
        canvas.setPixel(ypx11, xpx11, rfpart(yend) * xgap, r, g, b);
        canvas.setPixel(ypx11 + 1, xpx11, fpart(yend) * xgap, r, g, b);
      } else {
        canvas.setPixel(xpx11, ypx11, rfpart(yend) * xgap, r, g, b);
        canvas.setPixel(xpx11, ypx11 + 1, fpart(yend) * xgap, r, g, b);
      }
      intery = yend + gradient;
    }

    int xpx12;
    {
      const float xend = round(x1);
      const float yend = y1 + gradient * (xend - x1);
      const float xgap = rfpart(x1 + 0.5);
      xpx12 = int(xend);
      const int ypx12 = ipart(yend);
      if (steep) {
        canvas.setPixel(ypx12, xpx12, rfpart(yend) * xgap, r, g, b);
        canvas.setPixel(ypx12 + 1, xpx12, fpart(yend) * xgap, r, g, b);
      } else {
        canvas.setPixel(xpx12, ypx12, rfpart(yend) * xgap);
        canvas.setPixel(xpx12, ypx12 + 1, fpart(yend) * xgap, r, g, b);
      }
    }

    if (steep) {
      for (int x = xpx11 + 1; x < xpx12; x++) {
        canvas.setPixel(ipart(intery), x, rfpart(intery), r, g, b);
        canvas.setPixel(ipart(intery) + 1, x, fpart(intery), r, g, b);
        intery += gradient;
      }
    } else {
      for (int x = xpx11 + 1; x < xpx12; x++) {
        canvas.setPixel(x, ipart(intery), rfpart(intery), r, g, b);
        canvas.setPixel(x, ipart(intery) + 1, fpart(intery), r, g, b);
        intery += gradient;
      }
    }
  }
};

struct Ngon : public Canvas::Entity {
  using Pt = typename Canvas::Pt;
  using CTR = typename Canvas::CTR;

  Ngon(Pt origin, float size, int n, Col col) : origin_(origin), size_(size), n_(n), col_(col) {
    Pt cur{0, size_}, last{0, size_};
    auto shift = std::polar<float>(1, 3.14159265358979323846 * 2 / n_);
    cur *= shift;

    for (int i = 0; i < n_; ++i) {
      Line line(last + origin, cur + origin, col_);
      lines_.push_back(line);
      cur *= shift;
      last *= shift;
    }
  }
  virtual void render(CTR canvas) override {
    for (auto& i : lines_) i.render(canvas);
  }
  std::vector<Line> lines_;
  Pt origin_;
  float size_;
  int n_;
  Col col_;
};

struct Img : public Canvas::Entity {
  using Pt = typename Canvas::Pt;
  using CTR = typename Canvas::CTR;

  typedef int LONG;
  typedef unsigned short WORD;
  typedef unsigned int DWORD;
  
  #pragma pack(push, 1)
  struct BITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
  };

  struct BITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
  };
  #pragma pack(pop)

  unsigned char* buffer;
  BITMAPFILEHEADER* file_header;
  BITMAPINFOHEADER* info_header;

  Pt origin_;
  
  Img(Pt origin, unsigned char ptr[], int length) : origin_(origin) {
    buffer = ptr;

    file_header = (BITMAPFILEHEADER*)(buffer);
    info_header = (BITMAPINFOHEADER*)(buffer + sizeof(BITMAPFILEHEADER));
  }

  virtual void applyFrm(const frm& f) override {
    origin_.real(f["pos_x"]);
    origin_.imag(f["pos_y"]);
  }
  
  virtual void render(CTR canvas) override {
    if (!file_header) return;

    int w = info_header->biWidth;
    int h = info_header->biHeight;

    int count = 0;

    float x0 = 0 + origin_.real();
    float y0 = 0 + origin_.imag();
    float x1 = w + origin_.real();
    float y1 = h + origin_.imag();

    x0 -= canvas.x_;
    x1 -= canvas.x_;
    y0 -= canvas.y_;
    y1 -= canvas.y_;

    x0 *= canvas.zoom_;
    x1 *= canvas.zoom_;
    y0 *= canvas.zoom_;
    y1 *= canvas.zoom_;

    x0 += canvas.w_ / 2;
    x1 += canvas.w_ / 2;
    y0 += canvas.h_ / 2;
    y1 += canvas.h_ / 2;

    float xs = std::max<int>(0, -x0);
    float ys = std::max<int>(0, -y0);

    x0 = std::max<int>(0, x0);
    x1 = std::min<int>(canvas.w_, x1);
    y0 = std::max<int>(0, y0);
    y1 = std::min<int>(canvas.h_, y1);

    float x = 0;
    float y = 0;
    float offset = 1.0 / canvas.zoom_;

    x = xs * offset;
    for (int i = x0; i < x1; ++i) {
      y = ys * offset;
      for (int j = y0; j < y1; ++j) {
        int count = ((h - int(y) - 1) * w + (w - int(x) - 1) + 1) * 3;
        y += offset;    
        if (y > h or x > w) continue;
        unsigned char r = buffer[file_header->bfSize - count];
        ++count;
        unsigned char g = buffer[file_header->bfSize - count];
        ++count;
        unsigned char b = buffer[file_header->bfSize - count];

        canvas.setXYRGBA(i, j, r, g, b, 255);
      }
      x += offset;
    }
  }
};

// struct Rect : Entity {
//   LineT(Pt a, Pt b, Col col = Dracula::red) : a_(a), b_(b), col_(col) {}
//   virtual void render(CTR canvas) override {
//     drawLine(canvas, a_.real(), a_.imag(), b_.real(), b_.imag(), col_.r,
//     col_.g, col_.b);
//   }
// }
}