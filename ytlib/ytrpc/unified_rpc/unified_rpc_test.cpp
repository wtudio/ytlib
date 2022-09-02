#include <gtest/gtest.h>

#include "unified_context.hpp"
#include "unified_status.hpp"

namespace ytlib {
namespace ytrpc {

class TestCtx {
 public:
  bool OK() {
    return (code == 0) && (frame_code == 0);
  }

  int code = 0;
  int frame_code = 0;
  std::string msg;
};

static const std::string kTestRpcName = "TestRpc";

class AutoReg {
 public:
  AutoReg() {
    UnifiedStatusConversionCenter::Ins().RegUnifiedToNativeConversionFun(
        kTestRpcName,
        [](const UnifiedStatus& unified_status, std::shared_ptr<void>& native_status) {
          if (!native_status)
            native_status = std::static_pointer_cast<void>(std::make_shared<TestCtx>());

          TestCtx& real_native_status = *(static_cast<TestCtx*>(native_status.get()));
          if (unified_status) {
            real_native_status.code = 0;
            real_native_status.frame_code = 0;
            real_native_status.msg = "";
          } else {
            real_native_status.code = -1;
            real_native_status.frame_code = 0;
            real_native_status.msg = unified_status.Msg();
          }
        });

    UnifiedStatusConversionCenter::Ins().GetNativeToUnifiedConversionFun(
        kTestRpcName,
        [](const std::shared_ptr<void>& native_status, UnifiedStatus& unified_status) {
          if (!native_status) return;

          const TestCtx& real_native_status = *(static_cast<const TestCtx*>(native_status.get()));

          if (real_native_status.code == 0 && real_native_status.frame_code == 0) {
            unified_status.SetSuc(true);
          } else {
            unified_status.SetSuc(false);
            unified_status.SetMsg(real_native_status.msg);
          }
        });
  }
};

static AutoReg _auto_reg;

TEST(UNIFIED_TEST, UnifiedStatus) {
  UnifiedStatus st1;
  EXPECT_TRUE(st1);
  EXPECT_TRUE(st1.OK());
  EXPECT_STREQ(st1.Msg().c_str(), "");

  UnifiedStatus st2(false, "test msg");
  EXPECT_FALSE(st2);
  EXPECT_FALSE(st2.OK());
  EXPECT_STREQ(st2.Msg().c_str(), "test msg");
}

TEST(UNIFIED_TEST, UnifiedContext) {
  UnifiedContext ctx;
  EXPECT_FALSE(ctx.IsDone());
  EXPECT_STREQ(ctx.DoneInfo().c_str(), "");
  EXPECT_EQ(ctx.Deadline(), std::chrono::system_clock::time_point::max());

  ctx.AddMeta("k1", "v1");
  EXPECT_EQ(ctx.Meta().size(), 1);
}

}  // namespace ytrpc
}  // namespace ytlib
