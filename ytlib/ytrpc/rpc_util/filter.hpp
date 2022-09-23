#pragma once

#include <concepts>
#include <functional>
#include <list>
#include <memory>

#include <unifex/task.hpp>

namespace ytlib {
namespace ytrpc {

template <typename CtxType, typename StatusType, typename ReqType, typename RspType>
class FilterMgr {
 public:
  using RpcHandle = std::function<unifex::task<StatusType>(const CtxType&, const ReqType&, RspType&)>;
  using FilterHandle = std::function<unifex::task<StatusType>(const CtxType&, const ReqType&, RspType&, const RpcHandle&)>;

  template <typename T>
  requires std::constructible_from<FilterHandle, T>
  void RegisterFilter(T&& filter) {
    filter_list_.emplace_back((T &&) filter);
  }

  unifex::task<StatusType> InvokeRpc(const RpcHandle& rpc, const CtxType& ctx, const ReqType& req, RspType& rsp) {
    const RpcHandle* cur_handle_ptr = &rpc;
    std::list<RpcHandle> handle_list;

    for (const auto& filter : filter_list_) {
      cur_handle_ptr = &(handle_list.emplace_back(std::bind(filter, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, *cur_handle_ptr)));
    }

    co_return co_await (*cur_handle_ptr)(ctx, req, rsp);
  }

 private:
  std::list<FilterHandle> filter_list_;
};

}  // namespace ytrpc
}  // namespace ytlib