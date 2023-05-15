#pragma once

namespace test4 {

struct TestCtx {
  std::string meta;
};

struct RpcStatus {
  int st = 0;
};

struct TestReq {
  int in = 0;
};

struct TestRsp {
  int out = 0;
};

template <typename Rsp, typename Req, typename Receiver>
struct RpcOperationState final {
  template <typename Receiver2>
    requires std::constructible_from<Receiver, Receiver2>
  explicit RpcOperationState(TestCtx ctx, const Req& req, Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : ctx_(ctx), req_(req), receiver_((Receiver2&&)r) {}

  void start() noexcept {
    try {
      Rsp rsp;
      rsp.out = req_.in;

      RpcStatus st{1};

      if constexpr (unifex::is_stop_never_possible_v<unifex::stop_token_type_t<Receiver&>>) {
        unifex::set_value(std::move(receiver_), std::move(st), std::move(rsp));
      } else {
        if (unifex::get_stop_token(receiver_).stop_requested()) {
          unifex::set_done(std::move(receiver_));
        } else {
          unifex::set_value(std::move(receiver_), std::move(st), std::move(rsp));
        }
      }
    } catch (...) {
      unifex::set_error(std::move(receiver_), std::current_exception());
    }
  }

  TestCtx ctx_;
  const Req& req_;

  Receiver receiver_;
};

template <typename Rsp, typename Req>
struct RpcSender {
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<RpcStatus, Rsp>>;

  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  static constexpr bool sends_done = true;

  RpcSender(TestCtx ctx, const Req& req)
      : ctx_(ctx), req_(req) {}

  template <typename Receiver>
  RpcOperationState<Rsp, Req, unifex::remove_cvref_t<Receiver>> connect(Receiver&& receiver) {
    return RpcOperationState<Rsp, Req, unifex::remove_cvref_t<Receiver>>(ctx_, req_, (Receiver&&)receiver);
  }

  TestCtx ctx_;
  const Req& req_;
};

auto RpcFun(TestCtx ctx, const TestReq& req) -> RpcSender<TestRsp, TestReq> {
  return RpcSender<TestRsp, TestReq>(ctx, req);
}

auto RpcFun2(TestCtx ctx, const TestReq& req) {
  unifex::inline_scheduler inline_sche;
  return unifex::schedule(inline_sche) |
         unifex::then([ctx, req]() -> std::tuple<RpcStatus, TestRsp> {
           TestRsp rsp;
           rsp.out = req.in;

           RpcStatus st{1};
           return std::tuple<RpcStatus, TestRsp>(st, rsp);
         });
}

inline void Test4() {
  auto work = []() -> unifex::task<void> {
    DBG_PRINT("rpc start");

    TestCtx ctx{"msg"};
    TestReq req{42};

    // const auto& [status, rsp] = co_await RpcFun(ctx, req);

    const auto& [status, rsp] = co_await RpcFun2(ctx, req);

    DBG_PRINT("rpc end, status: %d, rsp: %d", status.st, rsp.out);
  };

  unifex::sync_wait(work());
}

}  // namespace test4