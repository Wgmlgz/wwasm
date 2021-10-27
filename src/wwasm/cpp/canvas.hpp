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
#include "animations.hpp"
#include "./images/frog.hpp"

using namespace std::string_literals;
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

struct Canvas {
  typedef float CT;
  typedef std::complex<float> Pt;
  typedef Canvas& CTR;

  struct Entity {
    virtual void applyFrm(const frm& f) {};
    virtual void render(CTR canvas) = 0;
    virtual ~Entity() {};
  };

  Canvas(size_t w, size_t h) {
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

  void pushEntity(int z, std::string id, Entity* entity) {
    entities_[z] = std::shared_ptr<Entity>(entity);
  }

  void pushEntity(std::string id, Entity* entity) {
    pushEntity(entities_.size() ? entities_.rbegin()->first + 1 : 0, id, entity);
  }

  uint8_t* data() { return data_; }

  uint8_t* render() {
    for (auto [z, entity] : entities_) {
      entity->render(*this);
    }
    return data();
  }

  float getX() { return x_; }
  float getY() { return y_; }
  size_t getW() { return w_; }
  size_t getH() { return h_; }
  float getZoom() { return zoom_; }

  float lerp(float a, float b, float f) { return (a * (1.0 - f)) + (b * f); }
  void setX(float val) { x_ = val; lerp(x_, val, 0.1);}
  void setY(float val) { y_ = val; lerp(y_, val, 0.1);}
  void setZoom(float val) { zoom_ = val; lerp(zoom_, val, 0.1);}
  
  ~Canvas() { if(data_) delete data_; }

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
  float x_ = 0, y_ = 0, zoom_ = 10;
  uint8_t* data_ = nullptr;
  std::mt19937 rng;
  std::map<int, std::shared_ptr<Entity>> entities_;
};

}  // namespace wwasm
