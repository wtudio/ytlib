#include <algorithm>
#include <vector>

#include "cube.h"
#include "ytlib/gl/gl_sys.hpp"
#include "ytlib/misc/misc_macro.h"

using namespace std;
using namespace ytlib;

cube cb;                          // 待画立方体
double thr;                       // 阈值，小于等于其的不画
float max_val, min_val, color_d;  // 颜色变换
float cube_d, de_x, de_y, de_z;   // 位置变换
double base_len;

std::vector<float> val2rgba(float h) {
  assert(h >= 0.0 && h <= 360.0);
  float s = 0.5, v = 0.5, r = 0, g = 0, b = 0;
  int i = ((int)(h / 60) % 6);
  float f = (h / 60) - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);
  switch (i) {
    case 0:
      r = v;
      g = t;
      b = p;
      break;
    case 1:
      r = q;
      g = v;
      b = p;
      break;
    case 2:
      r = p;
      g = v;
      b = t;
      break;
    case 3:
      r = p;
      g = q;
      b = v;
      break;
    case 4:
      r = t;
      g = p;
      b = v;
      break;
    case 5:
      r = v;
      g = p;
      b = q;
      break;
    default:
      break;
  }
  return std::vector<float>{r, g, b, 1.0f};
}

// 核心函数，cube绘图。使用opengl。坐标系-10000~10000
void CubeDisplay() {
  // 画立方体，六个方向进行遍历
  float curX, curY, curZ, curX_, curY_, curZ_;

  for (int32_t xx = 0; xx < cb.x; ++xx) {
    curX = (xx - de_x) * cube_d;
    curX_ = curX + cube_d;
    for (uint32_t yy = 0; yy < cb.y; ++yy) {
      curY = (yy - de_y) * cube_d;
      curY_ = curY + cube_d;
      for (uint32_t zz = 0; zz < cb.z; ++zz) {
        if (cb.val[xx][yy][zz] <= thr) continue;
        curZ = (zz - de_z) * cube_d;
        curZ_ = curZ + cube_d;
        bool colorFlag = false;
        if (xx == 0 || cb.val[xx - 1][yy][zz] <= thr) {
          if (!colorFlag) {
            float h = (cb.val[xx][yy][zz] - min_val) * color_d;
            glColor4fv(&(val2rgba(h)[0]));
            colorFlag = true;
          }
          glBegin(GL_POLYGON);
          glVertex3f(curX, curY, curZ);
          glVertex3f(curX, curY_, curZ);
          glVertex3f(curX, curY_, curZ_);
          glVertex3f(curX, curY, curZ_);
          glEnd();
        }
        if (xx == cb.x - 1 || cb.val[xx + 1][yy][zz] <= thr) {
          if (!colorFlag) {
            float h = (cb.val[xx][yy][zz] - min_val) * color_d;
            glColor4fv(&(val2rgba(h)[0]));
            colorFlag = true;
          }
          glBegin(GL_POLYGON);
          glVertex3f(curX_, curY, curZ);
          glVertex3f(curX_, curY, curZ_);
          glVertex3f(curX_, curY_, curZ_);
          glVertex3f(curX_, curY_, curZ);
          glEnd();
        }
        if (yy == 0 || cb.val[xx][yy - 1][zz] <= thr) {
          if (!colorFlag) {
            float h = (cb.val[xx][yy][zz] - min_val) * color_d;
            glColor4fv(&(val2rgba(h)[0]));
            colorFlag = true;
          }
          glBegin(GL_POLYGON);
          glVertex3f(curX, curY, curZ);
          glVertex3f(curX, curY, curZ_);
          glVertex3f(curX_, curY, curZ_);
          glVertex3f(curX_, curY, curZ);
          glEnd();
        }
        if (yy == cb.y - 1 || cb.val[xx][yy + 1][zz] <= thr) {
          if (!colorFlag) {
            float h = (cb.val[xx][yy][zz] - min_val) * color_d;
            glColor4fv(&(val2rgba(h)[0]));
            colorFlag = true;
          }
          glBegin(GL_POLYGON);
          glVertex3f(curX, curY_, curZ);
          glVertex3f(curX_, curY_, curZ);
          glVertex3f(curX_, curY_, curZ_);
          glVertex3f(curX, curY_, curZ_);
          glEnd();
        }
        if (zz == 0 || cb.val[xx][yy][zz - 1] <= thr) {
          if (!colorFlag) {
            float h = (cb.val[xx][yy][zz] - min_val) * color_d;
            glColor4fv(&(val2rgba(h)[0]));
            colorFlag = true;
          }
          glBegin(GL_POLYGON);
          glVertex3f(curX, curY, curZ);
          glVertex3f(curX_, curY, curZ);
          glVertex3f(curX_, curY_, curZ);
          glVertex3f(curX, curY_, curZ);
          glEnd();
        }
        if (zz == cb.z - 1 || cb.val[xx][yy][zz + 1] <= thr) {
          if (!colorFlag) {
            float h = (cb.val[xx][yy][zz] - min_val) * color_d;
            glColor4fv(&(val2rgba(h)[0]));
            colorFlag = true;
          }
          glBegin(GL_POLYGON);
          glVertex3f(curX, curY, curZ_);
          glVertex3f(curX, curY_, curZ_);
          glVertex3f(curX_, curY_, curZ_);
          glVertex3f(curX_, curY, curZ_);
          glEnd();
        }
      }
    }
  }
}

static void CubeKeyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 'z':
      thr += ((max_val - min_val) / 100.0);
      break;
    case 'x':
      thr -= ((max_val - min_val) / 100.0);
      break;
    default:
      break;
  }
}

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  cb = cube(128, 128, 128);
  int ct = 1;
  for (uint32_t i = 0; i < cb.x; ++i) {
    for (uint32_t j = 0; j < cb.y; ++j) {
      for (uint32_t k = 0; k < cb.z; ++k) {
        cb.val[i][j][k] = ct++;
      }
    }
  }

  // 位置变换
  base_len = 10000.0;
  cube_d = 1.8 * base_len / max(max(cb.x, cb.y), cb.z);
  de_x = cb.x / 2.0f, de_y = cb.y / 2.0f, de_z = cb.z / 2.0f;

  // 颜色变换
  pair<double*, double*> minmaxVal = minmax_element(cb.val[0][0], cb.val[0][0] + cb.x * cb.y * cb.z);
  max_val = *(minmaxVal.second) + 1.0;
  min_val = *(minmaxVal.first) - 1.0;
  color_d = 360.0f / (max_val - min_val);
  thr = min_val;

  // 初始化
  GlSys::Cfg cfg;
  cfg.w = 1000;
  cfg.h = 1000;
  cfg.title = "cube";
  cfg.base_len = base_len;

  GlSys::Ins().Init(cfg);
  GlSys::Ins().SetKeyboardFunc(CubeKeyboard);
  GlSys::Ins().SetDisplayFunc(CubeDisplay);
  GlSys::Ins().Start();

  DBG_PRINT("********************end test*******************");
  return 0;
}
