#include <string>

#include "google/protobuf/compiler/code_generator.h"
#include "google/protobuf/compiler/plugin.h"
#include "google/protobuf/compiler/plugin.pb.h"
#include "google/protobuf/descriptor.h"

#include <memory>

std::string ProtoFileBaseName(const std::string& full_name) {
  std::size_t p = full_name.rfind(".");
  assert(p != std::string::npos);

  return full_name.substr(0, p);
}

void WriteToFile(google::protobuf::compiler::GeneratorContext* context,
                 const std::string& file_name, const std::string& file_context) {
  std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> output(context->Open(file_name));
  google::protobuf::io::CodedOutputStream coded_out(output.get());
  coded_out.WriteRaw(file_context.data(), file_context.size());
}

class MyGenerator final : public google::protobuf::compiler::CodeGenerator {
 public:
  MyGenerator() = default;
  ~MyGenerator() override = default;
  bool Generate(const google::protobuf::FileDescriptor* file, const std::string& parameter,
                google::protobuf::compiler::GeneratorContext* context, std::string* error) const override {
    WriteToFile(context, ProtoFileBaseName(file->name()) + ".testrpc.pb.h", "#pragma once");

    WriteToFile(context, ProtoFileBaseName(file->name()) + ".testrpc.pb.cc", "#include <string>");

    return true;
  }
};

int main(int argc, char* argv[]) {
  MyGenerator generator;
  return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
