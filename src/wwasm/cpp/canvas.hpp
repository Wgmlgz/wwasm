#pragma once
#include <stdlib.h>

#include <algorithm>
#include <complex>
#include <cstdint>
#include <functional>
#include <memory>
#include <random>
#include <utility>
#include <vector>
#include <map>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cmath>

#include "em_header.hpp"
#include "io.hpp"

#include "./images/frog.hpp"

namespace wwasm {

namespace Dracula {
  auto red = 0xff5555;
  auto black = 0x282a36;
  auto gray = 0x44475a;
  auto green = 0x50fa7b;
  auto purple = 0xbd93f9;
  auto pink = 0xff79c6;
}

struct Col {
  uint8_t r = 0, g = 0, b = 0, a = 255;
  Col(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255) : r(r_), g(g_), b(b_), a(a_) {};
  Col(int hex) {
    r = ((hex >> 16) & 0xFF); 
    g = ((hex >> 8) & 0xFF);
    b = ((hex) & 0xFF);
  }
};

template <typename T>
struct CanvasT {
  typedef std::complex<T> Pt;
  typedef CanvasT<T>& CTR;

  struct Entity {
    virtual void render(CTR canvas) = 0;
    virtual ~Entity() {};
  };

  struct Line : public Entity {
    Line(Pt a, Pt b, Col col = Dracula::red) : a_(a), b_(b), col_(col) {}
    virtual void render(CTR canvas) override {
      drawLine(canvas, a_.real(), a_.imag(), b_.real(), b_.imag(), col_.r, col_.g, col_.b);
    }
    Pt a_, b_;
    Col col_;
    
   private:
    void drawLine(CTR canvas, T x0, T y0, T x1, T y1, uint8_t r, uint8_t g, uint8_t b, T size = 0.5, size_t iterations = 3) {
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
    void putLine(CTR canvas, T x0, T y0, T x1, T y1, uint8_t r, uint8_t g, uint8_t b) {
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

  struct Ngon : public Entity {
    Ngon(Pt origin, T size, int n, Col col) : origin_(origin), size_(size), n_(n), col_(col) {
      Pt cur{0, size_}, last{0, size_};
      auto shift = std::polar<T>(1, 3.14159265358979323846 * 2 / n_);
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
    T size_;
    int n_;
    Col col_;
  };


  struct Img : public Entity {
    
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
      if (ptr) {
        // file.seekg(0,std::ios::end);
        // file.seekg(0,std::ios::beg);

        buffer = ptr;
        // file.read(&buffer[0],length);

        file_header = (BITMAPFILEHEADER*)(buffer);
        info_header = (BITMAPINFOHEADER*)(buffer + sizeof(BITMAPFILEHEADER));
        std::cout << "good read :)" << std::endl;
        std::cout << length << std::endl;

      } else {
        file_header = nullptr;
        info_header = nullptr;
        std::cout << "error while reading file :(" << std::endl;
      }
    }

    virtual void render(CTR canvas) override {

      if (!file_header) return;
      int w = info_header->biWidth;
      int h = info_header->biHeight;

      int count = 0;
      T x0 = 0 + origin_.real();
      T y0 = 0 + origin_.imag();
      T x1 = w + origin_.real();
      T y1 = h + origin_.imag();

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

      T xs = std::max<int>(0, -x0);
      T ys = std::max<int>(0, -y0);

      x0 = std::max<int>(0, x0);
      x1 = std::min<int>(canvas.w_, x1);
      y0 = std::max<int>(0, y0);
      y1 = std::min<int>(canvas.h_, y1);

      T x = 0;
      T y = 0;
      T offset = 1.0 / canvas.zoom_;

      std::cout << offset << std::endl;

      x = xs * offset;
      for (int i = x0; i < x1; ++i) {
        y = ys * offset;
        for (int j = y0; j < y1; ++j) {
          int count = ((h - int(y) - 1) * w + int(x) + 1) * 3;
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
  //   Line(Pt a, Pt b, Col col = Dracula::red) : a_(a), b_(b), col_(col) {}
  //   virtual void render(CTR canvas) override {
  //     drawLine(canvas, a_.real(), a_.imag(), b_.real(), b_.imag(), col_.r, col_.g, col_.b);
  //   }
  // }

  CanvasT(size_t w, size_t h) {
    /* 4k max  */
    data_ = new uint8_t[33'177'600 + 1024];
    data_ += 1024;
    reset(w, h);
  }

  void reset(int w, int h) {
    h_ = h;
    w_ = w;
    fill(68, 71, 90, 255);
  }

  void invert() {
    for (int i = 0; i < w_ * h_ * 4; i += 4) {
      data_[i] = 255 - data_[i];
      data_[i + 1] = 255 - data_[i + 1];
      data_[i + 2] = 255 - data_[i + 2];
    }
  }

  void fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    for (int i = 0; i < w_; ++i) {
      for (int j = 0; j < h_; ++j) {
        pixel(i, j)[0] = r;
        pixel(i, j)[1] = g;
        pixel(i, j)[2] = b;
        pixel(i, j)[3] = a;
      }
    }
  }

  void randFill() {
    for (int i = 0; i < w_; ++i) {
      for (int j = 0; j < h_; ++j) {
        pixel(i, j)[0] = rng();
        pixel(i, j)[1] = rng();
        pixel(i, j)[2] = rng();
        pixel(i, j)[3] = 255;
      }
    }
  }

  uint8_t* pixel(size_t x, size_t y) {

    return data_ + (h_ - y - 1) * w_ * 4 + x * 4;
  }

  void setXYRGBA(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (x >= w_) return;
    if (y >= h_) return;
    pixel(x, y)[0] = r;
    pixel(x, y)[1] = g;
    pixel(x, y)[2] = b;
    pixel(x, y)[3] = a;
  }

  void pushEntity(int z, Entity* entity) {
    entities_[z] = std::shared_ptr<Entity>(entity);
  }

  void pushEntity(Entity* entity) {
    pushEntity(entities_.size() ? entities_.rbegin()->first + 1 : 0, entity);
  }

  uint8_t* data() { return data_; }

  uint8_t* render() {
    for (auto [z, entity] : entities_) {
      entity->render(*this);
    }
    return data();
  }

  T getX() { return x_; }
  T getY() { return y_; }
  size_t getW() { return w_; }
  size_t getH() { return h_; }
  T getZoom() { return zoom_; }

  T lerp(T a, T b, T f) { return (a * (1.0 - f)) + (b * f); }
  void setX(T val) { x_ = val; lerp(x_, val, 0.1);}
  void setY(T val) { y_ = val; lerp(y_, val, 0.1);}
  void setZoom(T val) { zoom_ = val; lerp(zoom_, val, 0.1);}
  
  ~CanvasT() { if(data_) delete data_; }

  void addPixel(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b,
                uint8_t a) {
    if (x >= w_) return;
    if (y >= h_) return;
    pixel(x, y)[0] = std::max(pixel(x, y)[0], r);
    pixel(x, y)[1] = std::max(pixel(x, y)[1], g);
    pixel(x, y)[2] = std::max(pixel(x, y)[2], b);
    pixel(x, y)[3] = std::max(pixel(x, y)[3], a);
  }

  void setPixel(size_t x, size_t y, float brightness, uint8_t r = 238,
                uint8_t g = 130, uint8_t b = 238) {
    addPixel(x, y, r * brightness, g * brightness, b * brightness, 255);
  }

  size_t w_, h_;
  T x_ = 0, y_ = 0, zoom_ = 10;
  uint8_t* data_ = nullptr;
  std::mt19937 rng;
  std::map<int, std::shared_ptr<Entity>> entities_;
};

using Canvas = CanvasT<float>; 

std::map<int, std::shared_ptr<Canvas>> canvas_map{{0, std::shared_ptr<Canvas>(new Canvas(500, 500))}};
bool init = true;

extern "C" {
  EMSCRIPTEN_KEEPALIVE uint8_t* getCanvasData(int id, int w, int h) {
    auto& canvas = *canvas_map.at(id);

    auto zoom = ioGetDouble(3);
    if (zoom == 0) zoom = 1;

    canvas.reset(w, h);
    canvas.setZoom(zoom);
    canvas.setX(ioGetDouble(1));
    canvas.setY(ioGetDouble(2));
    
    if (init) {
      init = false;
      canvas.pushEntity(new Canvas::Img({0, 0}, frog_bmp, frog_bmp_len));

      canvas.pushEntity(new Canvas::Ngon({-100, 0}, 50, 5, Dracula::green));
      canvas.pushEntity(new Canvas::Ngon({600, 0}, 100, 3, Dracula::pink));

      canvas.pushEntity(new Canvas::Line({-200, -200}, {700, -200}, Dracula::purple));
    }
    
    return canvas.render();
  }
}
}  // namespace wwasm
