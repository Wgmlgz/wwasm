#pragma once
#include "canvas.hpp"
#include "entities.hpp"

namespace wwasm {
std::map<std::string, std::shared_ptr<Canvas>> canvas_map{
  {"main_canvas", std::shared_ptr<Canvas>(new Canvas(500, 500))},
  {"aboba", std::shared_ptr<Canvas>(new Canvas(500, 500))}
};

std::map<std::string, bool> init;

auto a = frm{"pos_x", 0_cnt} | frm{"pos_y", 0_cnt};
auto b = frm{"pos_x", 500_cnt} | frm{"pos_y", 500_cnt};
auto c = frm{"pos_x", -500_cnt} | frm{"pos_y", 500_cnt};

auto an = anim({{0, a}, {2,b}, {6, c}, {7, a}});

extern "C" {
  EMSCRIPTEN_KEEPALIVE uint8_t* getCanvasData(char* ptr, int w, int h) {
    std::string id(ptr);
    auto& canvas = *canvas_map.at(id);

    auto zoom = ioGetDouble(id + "_zoom");
    if (zoom == 0) zoom = 1;

    canvas.reset(w, h);
    canvas.setZoom(zoom);
    canvas.setX(ioGetDouble(id + "_x"));
    canvas.setY(ioGetDouble(id + "_y"));
    
    if (!init[id]) {
      init[id] = true;
      canvas.pushEntity("frog", new Img({0, 0}, frog_bmp, frog_bmp_len));

      canvas.pushEntity("pentagon", new Ngon({-100, 0}, 50, 5, Dracula::green));
      canvas.pushEntity("triangle", new Ngon({600, 0}, 100, 3, Dracula::pink));

      canvas.pushEntity("line", new Line({-200, -200}, {700, -200}, Dracula::purple));
    }
    
    canvas.entities_[0]->applyFrm(an.get());
    return canvas.render();
  }
}
}