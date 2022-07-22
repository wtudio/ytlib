#include <chrono>
#include <string>
#include <thread>

#include <unifex/sync_wait.hpp>

#include "event2/event.h"

#include "adapters/libevent.h"
#include "async.h"
#include "hiredis.h"

#include "redis_cli.hpp"

using namespace std;

const char redis_host[] = "9.135.63.43";
const int redis_port = 56379;

void SyncTest() {
  redisContext *ctx = nullptr;
  redisReply *reply = nullptr;

  // connect
  ctx = redisConnect(redis_host, redis_port);
  if (ctx == nullptr) {
    printf("Connection error: can't allocate redis context\n");
    exit(1);
  } else if (ctx->err) {
    printf("Connection error: %s\n", ctx->errstr);
    redisFree(ctx);
    exit(1);
  }

  // ping
  reply = static_cast<redisReply *>(redisCommand(ctx, "PING"));
  if (reply == nullptr) {
    printf("PING failed, no rsp\n");
  } else {
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);
  }

  // set
  reply = static_cast<redisReply *>(redisCommand(ctx, "SET %s %s", "foo", "hello world"));
  if (reply == nullptr) {
    printf("SET failed, no rsp\n");
  } else {
    printf("SET: %s\n", reply->str);
    freeReplyObject(reply);
  }

  // get
  reply = static_cast<redisReply *>(redisCommand(ctx, "GET foo"));
  if (reply == nullptr) {
    printf("GET failed, no rsp\n");
  } else {
    printf("GET foo: %s\n", reply->str);
    freeReplyObject(reply);
  }

  // disconnects and frees the context
  redisFree(ctx);
}

void AsyncTest() {
  auto work = []() -> unifex::task<void> {
    ytlib::RedisClient::Cfg cfg{
        .host = redis_host,
        .port = redis_port};

    ytlib::RedisClient app(cfg);

    auto set_ret = co_await app.Command("SET foo hello world");

    auto get_ret = co_await app.Command("GET foo");

    app.Stop();

    co_return;
  };

  unifex::sync_wait(work());
}

void OnConnect(const redisAsyncContext *ctx, int status) {
  printf("OnConnect, status %d\n", status);
}

void OnDisconnect(const redisAsyncContext *ctx, int status) {
  printf("OnDisconnect, status %d\n", status);
}

void RedisCallback(redisAsyncContext *ctx, void *r, void *privdata) {
  redisReply *reply = static_cast<redisReply *>(r);
  if (reply == nullptr) {
    if (ctx->errstr) {
      printf("get err: %s\n", ctx->errstr);
    }
    return;
  }
  printf("%s\n", reply->str);
}

void Loop(event_base *events, uint32_t loop_times = 100) {
  uint32_t ct = 0;
  for (; ct < loop_times; ++ct) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto ret_code = event_base_loop(events, EVLOOP_NONBLOCK);
    if (ret_code == -1) {
      printf("Exec error occurred\n");
    } else if (ret_code == 1) {
      printf("Exec end, ret_code = 1\n");
      break;
    }
  }
  printf("Exec end, ct: %d\n", ct);
}

void AsyncTest2() {
  event_base *events = event_base_new();
  redisAsyncContext *ctx = nullptr;

  // connect
  printf("start connect\n");
  ctx = redisAsyncConnect(redis_host, redis_port);
  if (ctx == nullptr) {
    printf("Connection error: can't allocate redis context\n");
    exit(1);
  } else if (ctx->err) {
    printf("Connection error: %s\n", ctx->errstr);
    redisAsyncFree(ctx);
    exit(1);
  } else {
    redisLibeventAttach(ctx, events);
    redisAsyncSetConnectCallback(ctx, OnConnect);
    redisAsyncSetDisconnectCallback(ctx, OnDisconnect);
  }
  Loop(events);

  // ping
  printf("start cmd ping\n");
  redisAsyncCommand(ctx, RedisCallback, NULL, "PING");
  Loop(events);

  // set
  printf("start cmd set\n");
  redisAsyncCommand(ctx, RedisCallback, NULL, "SET %s %s", "foo", "hello world");
  Loop(events);

  // get
  printf("start cmd get\n");
  redisAsyncCommand(ctx, RedisCallback, NULL, "GET foo");
  Loop(events);

  // disconnects and frees the context
  printf("start disconnect\n");
  redisAsyncDisconnect(ctx);
  Loop(events);

  printf("start del\n");
  if (ctx) {
    redisAsyncFree(ctx);
    ctx = nullptr;
  }
  Loop(events);

  if (events) {
    event_base_free_nofinalize(events);
    events = nullptr;
  }
}

int32_t main(int32_t argc, char **argv) {
  // SyncTest();

  // AsyncTest();

  AsyncTest2();

  return 0;
}
