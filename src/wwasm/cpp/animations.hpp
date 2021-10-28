#pragma once
#include <algorithm>
#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <stdexcept>
#include <string>

namespace wwasm {
struct prp {
  float f_;
  float real_f_;
  prp(float f = 0) : f_(f) {}
  operator float() const{ return f_; }
};

prp operator"" _cnt(long double val) { return prp(float(val)); }
prp operator"" _cnt(unsigned long long val) { return prp(float(val)); }

struct frm {
  std::map<std::string, prp> props;
  frm() {}
  frm(const std::string& name, const prp& prop) { props.insert({name, prop}); }
  const prp operator[](const std::string& str) const {
    return props.at(str);
  }
  prp& operator[](const std::string& str) { return props.at(str); }
};

frm operator|(const frm& lhs, const frm& rhs) {
  auto res = lhs;
  for (const auto& [id, prop] : rhs.props) {
    res.props.insert({id, prop});
  }
  return res;
}

float lerp(float a, float b, float f) {
  // f = (3.0 * f * f) - (2.0 * f * f * f);
  return (a * (1.0 - f)) + (b * f);
}

frm interp(const frm& lhs, const frm& rhs, float t) {
  auto check = [](auto const& lhs, auto const& rhs) {
    return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin(),
      [](auto a, auto b) { return a.first == b.first; });
  };
  if (!check(rhs.props, rhs.props)) throw std::runtime_error("keys arn't equal");

  auto res = lhs;
  for (const auto& [id, _] : lhs.props) {
    res[id] = lerp(lhs[id], rhs[id], t);
  }
  return res;
}


struct anim {
  std::list<std::pair<float, frm>> frames_;
  bool loop = true;   
  std::chrono::time_point<std::chrono::system_clock> start_, last_;

  std::list<std::pair<float, frm>>::iterator cur_, next_;
  anim(const std::list<std::pair<float, frm>>& frames) : frames_(frames) {
    if (frames.size() < 2) throw std::runtime_error("need 2 or more frames");
    reset();
  }

  void reset() {
    last_ = start_ = std::chrono::system_clock::now();
    cur_ = frames_.begin();
    next_ = ++frames_.begin();
  }
  void nextFrame() {
    if (++next_ == frames_.end()) {
      reset();
    } else {
      ++cur_;
    }
  }
  frm get() {
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<float> dt = now - last_;
    std::chrono::duration<float> ts = now - start_;
    if (ts.count() >= next_->first) {
      nextFrame();
      last_ = now;
    }

    dt = now - last_;
    ts = now - last_;
    auto t = dt.count() / (next_->first - cur_->first);
    // std::cout << cur_->first << " " << next_->first << std::endl;
    t = std::min<float>(std::max<float>(0, t), 1);
    return interp(cur_->second, next_->second, t);
  }

};
}
