#pragma once

namespace ytlib {
namespace ytrpc {

using RpcHandle = std::function<RetSender<Status, google::protobuf::Message>(const Ctx&, const google::protobuf::Message&)>;
using RpcFilterHandle = std::function<task<tuple<Status, google::protobuf::Message>>(const Ctx&, const google::protobuf::Message&, const RpcHandle&)>;

}
}  // namespace ytlib