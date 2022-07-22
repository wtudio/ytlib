#pragma once

#include <unifex/task.hpp>

#include "adapters/poll.h"
#include "hiredis.h"

namespace ytlib {

class RedisClient : public std::enable_shared_from_this<RedisClient> {
 public:
  struct Cfg {
    std::string host = "127.0.0.1";
    uint16_t port = 6379;

    /// 校验配置
    static Cfg Verify(const Cfg &verify_cfg) {
      Cfg cfg(verify_cfg);

      return cfg;
    }
  };

  RedisClient(const RedisClient::Cfg &cfg)
      : cfg_(RedisClient::Cfg::Verify(cfg)) {}

  ~RedisClient() {}

  RedisClient(const RedisClient &) = delete;             ///< no copy
  RedisClient &operator=(const RedisClient &) = delete;  ///< no copy

  struct Ret {
  };

  unifex::task<RedisClient::Ret> Command(const std::string &cmd) {
    co_return Ret();
  }

  void Stop() {
  }

  // 需要手动poll
  void Poll() {
  }

  const std::atomic_bool &IsRunning() { return run_flag_; }

  const RedisClient::Cfg &GetCfg() const { return cfg_; }

 private:
  struct SessionCfg {
  };

  class Session : public std::enable_shared_from_this<Session> {
  };

 private:
  const RedisClient::Cfg cfg_;
  std::atomic_bool run_flag_ = true;
  std::shared_ptr<const RedisClient::SessionCfg> session_cfg_ptr_;
  std::shared_ptr<RedisClient::Session> session_ptr_;
};

}  // namespace ytlib