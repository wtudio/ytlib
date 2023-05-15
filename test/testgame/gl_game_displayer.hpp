#pragma once

#include <list>
#include <memory>

#include "game_displayer_inf.hpp"
#include "ytlib/gl/gl_sys.hpp"

namespace ytlib {

class GlGameDisplayer : public GameDisplayerInf, public std::enable_shared_from_this<GlGameDisplayer> {
 public:
  static std::shared_ptr<GlGameDisplayer> InsPtr() {
    static std::shared_ptr<GlGameDisplayer> instance(new GlGameDisplayer());
    return instance;
  }

  virtual ~GlGameDisplayer() {}

  virtual void Start() override {
    GlSys::Ins().Start();
  }

  virtual void Stop() override {
    GlSys::Ins().Stop();
  }

  virtual void SyncBuf(std::shared_ptr<GameBuf> buf) override {
    display_buf_ = buf;
  }

 private:
  GlGameDisplayer() {
    GlSys::Cfg cfg;
    cfg.w = 1000;
    cfg.h = 1000;
    cfg.title = "cube";
    cfg.base_len = 1000.0;

    GlSys::Ins().Init(cfg);
    GlSys::Ins().SetKeyboardFunc(GlGameDisplayer::OnGlGameKeyboard);
    GlSys::Ins().SetDisplayFunc(GlGameDisplayer::OnGlGameDisplay);
  }

  static void OnGlGameDisplay() {
    std::shared_ptr<GameBuf> buf = InsPtr()->display_buf_;
    if (!buf) [[unlilely]]
      return;

    // todo

    for (auto& obj : buf->obj_list) {
      glPointSize(10);
      glBegin(GL_POINTS);
      glColor4f(0.2f, 0.8f, 0.5f, 0.3f);
      glVertex3f(obj.location.x, obj.location.y, obj.location.z);
      glEnd();
    }
  }

  static void OnGlGameKeyboard(unsigned char key, int x, int y) {
    switch (key) {
      case 27:  // 按ESCAPE时退出窗口
        InsPtr()->Stop();
        break;
      default:
        break;
    }
  }

 private:
  std::shared_ptr<GameBuf> display_buf_;
};

}  // namespace ytlib
