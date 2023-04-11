#include <coroutine>
#include <cstdarg>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <stack>
#include <thread>
#include <vector>

#include "gl_game_displayer.hpp"
#include "world.hpp"
#include "ytlib/boost_tools_asio/asio_tools.hpp"
#include "ytlib/misc/misc_macro.h"

using namespace std;
using namespace ytlib;

std::shared_ptr<World> AddGameWorld(const std::shared_ptr<AsioExecutor>& game_sys_ptr) {
  // init game world
  WorldCfg cfg;
  auto game_world_ptr = std::make_shared<World>(game_sys_ptr->IO(), cfg);
  game_world_ptr->SetGameDisplayer(std::dynamic_pointer_cast<GameDisplayerInf>(GlGameDisplayer::InsPtr()));

  auto ship1 = std::make_shared<Ship>();
  ship1->name_ = "ship1";
  ship1->id_ = 1;
  ship1->mass_ = 1.0;
  ship1->thrust_ = 1.0;
  ship1->speed_ = {1.0, 2.0, 3.0};
  game_world_ptr->ship_map_.emplace(ship1->id_, ship1);

  auto ship2 = std::make_shared<Ship>();
  ship2->name_ = "ship2";
  ship2->id_ = 2;
  ship2->mass_ = 1.0;
  ship2->thrust_ = 2.0;
  ship2->speed_ = {2.0, 2.0, 4.0};
  game_world_ptr->ship_map_.emplace(ship2->id_, ship2);

  game_sys_ptr->RegisterSvrStopFunc([&game_world_ptr] { game_world_ptr->Stop(); });

  game_world_ptr->Start();
  game_world_ptr->game_displayer_->Start();

  return game_world_ptr;
}

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start game-------------------");

  auto game_sys_ptr = std::make_shared<AsioExecutor>(8);
  game_sys_ptr->EnableStopSignal();

  game_sys_ptr->Start();

  auto game_world_ptr = AddGameWorld(game_sys_ptr);

  std::this_thread::sleep_for(std::chrono::seconds(5));

  game_sys_ptr->Stop();
  game_sys_ptr->Join();

  // report
  uint64_t run_time = (game_world_ptr->game_end_time_point_ - game_world_ptr->game_start_time_point_).count();

  auto ship1 = game_world_ptr->ship_map_[1];
  auto ship2 = game_world_ptr->ship_map_[2];

  stringstream ss;
  ss << "game report:" << endl
     << "run time:" << run_time << "ns, " << static_cast<float>(run_time) / 1e9 << "s" << endl
     << "frame count: " << game_world_ptr->frame_count_ << endl
     << "fps: " << static_cast<float>(game_world_ptr->frame_count_) * 1e9 / run_time << endl;
  DBG_PRINT("%s", ss.str().c_str());

  ss.str("");
  ss << "ship1:" << endl
     << "end location: " << ship1->location_ << endl;
  DBG_PRINT("%s", ss.str().c_str());

  ss.str("");
  ss << "ship2:" << endl
     << "end location: " << ship2->location_ << endl;
  DBG_PRINT("%s", ss.str().c_str());

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());

  DBG_PRINT("********************end game*******************");
  return 0;
}
