/**
 * @file gl_sys.hpp
 * @brief 基于freeglut的图形系统封装
 * @note 基于freeglut的图形系统封装
 * @author WT
 * @date 2022-03-25
 */
#pragma once

#include <freeglut.h>
#include <string>

namespace ytlib {

class GlSys {
 public:
  using DisplayFunc = void (*)(void);
  using KeyboardFunc = void (*)(unsigned char key, int x, int y);

 public:
  struct Cfg {
    bool print_fps = true;       // 是否打印帧率
    std::string title = "demo";  // 标题

    int w = 800;  // 窗体宽
    int h = 800;  // 窗体高

    int x = 0;  // 窗体左上角x坐标
    int y = 0;  // 窗体左上角y坐标

    double base_len = 10000.0;  // 坐标系轴长度
  };

 public:
  static GlSys& Ins() {
    static GlSys instance;
    return instance;
  }

  GlSys(const GlSys&) = delete;             ///< no copy
  GlSys& operator=(const GlSys&) = delete;  ///< no copy

  ~GlSys() { Stop(); }

  void SetDisplayFunc(DisplayFunc display_func) {
    display_func_ = display_func;
  }

  void SetKeyboardFunc(KeyboardFunc keyboard_func) {
    keyboard_func_ = keyboard_func;
  }

  void Init(const GlSys::Cfg& cfg) {
    cfg_ = cfg;
  }

  void Start() {
    int argc = 1;
    char argv0[] = "demo";
    char** argv = new char* [] { argv0, nullptr };

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(cfg_.w, cfg_.h);
    glutInitWindowPosition(cfg_.x, cfg_.y);
    glutCreateWindow(cfg_.title.c_str());
    glClearColor(0.0, 0.0, 0.0, 0.3);
    glClear(GL_COLOR_BUFFER_BIT);
    SetCamera(0.0f, 0.0f);
    glutReshapeFunc(GlSys::OnReshape);
    glutMouseFunc(GlSys::OnMouse);
    glutMotionFunc(GlSys::OnMotion);
    glutKeyboardFunc(GlSys::OnKeyboard);
    glutDisplayFunc(GlSys::OnDisplay);

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    glutMainLoop();
  }

  void Stop() {
    glutLeaveMainLoop();
  }

 private:
  GlSys() {}

  void SetCamera(double x, double y) {
    const double deg2rad = 0.017453;
    double alpha, fy;  // 单位是弧度
    if ((fy_ + y) > 1.0f && (fy_ + y) < 179.0f) {
      alpha_ += x;  // 根据鼠标移动的方向设置新的球坐标
      fy_ += y;
      if (alpha_ > 360.0f) alpha_ -= 360.0f;
      if (alpha_ < 0.0f) alpha_ += 360.0f;  // 将水平偏角锁定在0°到360°之间
      alpha = (alpha_) * (deg2rad);
      fy = fy_ * (deg2rad);                   // 角度转弧度
      camera_x_ = r_ * sin(fy) * cos(alpha);  // 极坐标转直角坐标
      camera_z_ = r_ * sin(fy) * sin(alpha);
      camera_y_ = r_ * cos(fy);  // 注意：竖直方向的是y轴
    }
  }

  static void OnReshape(int w, int h) {
    GlSys& ins = Ins();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    double len = ins.cfg_.base_len * 1.8;
    glOrtho(-len, len, -len, len, -len, len);
  }

  // 鼠标点击事件
  static void OnMouse(int button, int state, int x, int y) {
    GlSys& ins = Ins();
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
      ins.left_but_down_ = true;
      ins.mouse_x_ = x;
      ins.mouse_y_ = y;
    }
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
      ins.left_but_down_ = false;
    }
  }

  // 鼠标按下时的鼠标移动事件
  static void OnMotion(int x, int y) {
    GlSys& ins = Ins();
    if (ins.left_but_down_) {
      ins.SetCamera(float(x - ins.mouse_x_), float(ins.mouse_y_ - y));
      ins.mouse_x_ = x;
      ins.mouse_y_ = y;
    }
  }

  static void OnKeyboard(unsigned char key, int x, int y) {
    GlSys& ins = Ins();
    switch (key) {
      case 27:  //按ESCAPE时退出窗口
        ins.Stop();
        break;
      default:
        break;
    }
    if (ins.keyboard_func_) {
      ins.keyboard_func_(key, x, y);
    }
  }

  static void OnDisplay() {
    GlSys& ins = Ins();
    if (ins.cfg_.print_fps) {
      static int frame = 0, time, timebase = 0;
      //计算帧率
      frame++;
      time = glutGet(GLUT_ELAPSED_TIME);
      if (time - timebase > 1000) {
        printf("fps:%4.2f\n", frame * 1000.0 / (time - timebase));
        timebase = time;
        frame = 0;
      }
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);

    /*
    cameraX, cameraY, cameraZ : 设定摄像机位置，你所看到的景物将根据摄像机的位置改变而改变，这就是鼠标调整摄像机位置的结果
    0.0, 0.0, 0.0 : 这三个参数是摄像机观察的点
    0.0, 1.0, 0.0 : 这三个参数指定了视图的上方向*
    */
    gluLookAt(ins.camera_x_, ins.camera_y_, ins.camera_z_,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);
    glPolygonMode(GL_FRONT, GL_FILL);  // 设置正面为填充模式
    glPolygonMode(GL_BACK, GL_LINE);   // 设置反面为线形模式
    glFrontFace(GL_CW);                // 设置逆时针方向为正面
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);  // 不渲染反面
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glPushMatrix();

    // 坐标系原点
    glPointSize(10);
    glBegin(GL_POINTS);
    glColor4f(0.2f, 0.8f, 0.5f, 0.3f);
    glVertex3f(0.0, 0.0, 0.0);
    glEnd();

    // 坐标系三条轴
    float arrSize = 300.0;
    glBegin(GL_LINES);
    glColor4f(1.0f, 1.0f, 0.0f, 0.7f);
    glVertex3f(ins.cfg_.base_len, 0.0, 0.0);
    glVertex3f(-ins.cfg_.base_len, 0.0, 0.0);
    glVertex3f(ins.cfg_.base_len, 0.0, 0.0);
    glVertex3f(ins.cfg_.base_len - arrSize, arrSize / 2, 0.0);
    glVertex3f(ins.cfg_.base_len, 0.0, 0.0);
    glVertex3f(ins.cfg_.base_len - arrSize, -arrSize / 2, 0.0);
    glVertex3f(ins.cfg_.base_len, 0.0, 0.0);
    glVertex3f(ins.cfg_.base_len - arrSize, 0.0, arrSize / 2);
    glVertex3f(ins.cfg_.base_len, 0.0, 0.0);
    glVertex3f(ins.cfg_.base_len - arrSize, 0.0, -arrSize / 2);

    glVertex3f(0.0, ins.cfg_.base_len, 0.0);
    glVertex3f(0.0, -ins.cfg_.base_len, 0.0);
    glVertex3f(0.0, ins.cfg_.base_len, 0.0);
    glVertex3f(0.0, ins.cfg_.base_len - arrSize, arrSize / 2);
    glVertex3f(0.0, ins.cfg_.base_len, 0.0);
    glVertex3f(0.0, ins.cfg_.base_len - arrSize, -arrSize / 2);
    glVertex3f(0.0, ins.cfg_.base_len, 0.0);
    glVertex3f(arrSize / 2, ins.cfg_.base_len - arrSize, 0.0);
    glVertex3f(0.0, ins.cfg_.base_len, 0.0);
    glVertex3f(-arrSize / 2, ins.cfg_.base_len - arrSize, 0.0);

    glVertex3f(0.0, 0.0, ins.cfg_.base_len);
    glVertex3f(0.0, 0.0, -ins.cfg_.base_len);
    glVertex3f(0.0, 0.0, ins.cfg_.base_len);
    glVertex3f(arrSize / 2, 0.0, ins.cfg_.base_len - arrSize);
    glVertex3f(0.0, 0.0, ins.cfg_.base_len);
    glVertex3f(-arrSize / 2, 0.0, ins.cfg_.base_len - arrSize);
    glVertex3f(0.0, 0.0, ins.cfg_.base_len);
    glVertex3f(0.0, arrSize / 2, ins.cfg_.base_len - arrSize);
    glVertex3f(0.0, 0.0, ins.cfg_.base_len);
    glVertex3f(0.0, -arrSize / 2, ins.cfg_.base_len - arrSize);
    glEnd();

    // 坐标系文字
    glColor4f(0.8f, 0.0f, 0.0f, 1.0f);
    glRasterPos3i(0, 0, 0);
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'O');
    glRasterPos3i(ins.cfg_.base_len, 0, 0);
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'X');
    glRasterPos3i(0, ins.cfg_.base_len, 0);
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'Y');
    glRasterPos3i(0, 0, ins.cfg_.base_len);
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'Z');

    // 3d空间具体形状
    if (ins.display_func_) {
      ins.display_func_();
    }

    glPopMatrix();
    glFlush();
    glutSwapBuffers();  // 交换前后台缓冲区
    glutPostRedisplay();
  }

 private:
  GlSys::Cfg cfg_;  // 配置

  // 用于记录鼠标位置
  int32_t mouse_x_ = 0;
  int32_t mouse_y_ = 0;

  // 记录鼠标左键按下的状态
  bool left_but_down_ = false;

  // 用于摄像机定位
  double camera_x_, camera_y_, camera_z_;

  // 球坐标下，距离、水平偏角、竖直偏角，单位均用角度
  double r_ = 3.0;
  double alpha_ = 60.0;
  double fy_ = 45.0;

  DisplayFunc display_func_;    // display函数
  KeyboardFunc keyboard_func_;  // 键盘输入处理函数
};

}  // namespace ytlib
