#include <iostream>
#include <string>

#include "ytlib/misc/misc_macro.h"

#include "opentelemetry/exporters/ostream/span_exporter_factory.h"
#include "opentelemetry/sdk/trace/exporter.h"
#include "opentelemetry/sdk/trace/processor.h"
#include "opentelemetry/sdk/trace/simple_processor_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/provider.h"

#include "opentelemetry/common/attribute_value.h"
#include "opentelemetry/common/key_value_iterable_view.h"
#include "opentelemetry/context/context.h"
#include "opentelemetry/exporters/ostream/metric_exporter_factory.h"
#include "opentelemetry/metrics/async_instruments.h"
#include "opentelemetry/metrics/meter.h"
#include "opentelemetry/metrics/meter_provider.h"
#include "opentelemetry/metrics/observer_result.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/metrics/sync_instruments.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/nostd/variant.h"
#include "opentelemetry/sdk/metrics/aggregation/aggregation_config.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_options.h"
#include "opentelemetry/sdk/metrics/instruments.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include "opentelemetry/sdk/metrics/meter_provider_factory.h"
#include "opentelemetry/sdk/metrics/metric_reader.h"
#include "opentelemetry/sdk/metrics/push_metric_exporter.h"
#include "opentelemetry/sdk/metrics/state/filtered_ordered_attribute_map.h"
#include "opentelemetry/sdk/metrics/view/instrument_selector.h"
#include "opentelemetry/sdk/metrics/view/instrument_selector_factory.h"
#include "opentelemetry/sdk/metrics/view/meter_selector.h"
#include "opentelemetry/sdk/metrics/view/meter_selector_factory.h"
#include "opentelemetry/sdk/metrics/view/view.h"
#include "opentelemetry/sdk/metrics/view/view_factory.h"

#include "opentelemetry/exporters/ostream/log_record_exporter.h"
#include "opentelemetry/exporters/ostream/span_exporter_factory.h"
#include "opentelemetry/logs/log_record.h"
#include "opentelemetry/logs/logger.h"
#include "opentelemetry/logs/logger_provider.h"
#include "opentelemetry/logs/provider.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/sdk/logs/exporter.h"
#include "opentelemetry/sdk/logs/logger_provider.h"
#include "opentelemetry/sdk/logs/logger_provider_factory.h"
#include "opentelemetry/sdk/logs/recordable.h"
#include "opentelemetry/sdk/logs/simple_log_record_processor_factory.h"
#include "opentelemetry/sdk/trace/processor.h"
#include "opentelemetry/sdk/trace/recordable.h"
#include "opentelemetry/sdk/trace/simple_processor_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/sdk/version/version.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/trace/scope.h"
#include "opentelemetry/trace/span.h"
#include "opentelemetry/trace/span_context.h"
#include "opentelemetry/trace/tracer.h"
#include "opentelemetry/trace/tracer_provider.h"

#include "opentelemetry/exporters/otlp/otlp_http_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_http_exporter_options.h"
#include "opentelemetry/sdk/trace/batch_span_processor_factory.h"
#include "opentelemetry/sdk/trace/batch_span_processor_options.h"
#include "opentelemetry/sdk/trace/processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/provider.h"

#include "opentelemetry/exporters/otlp/otlp_http_metric_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_http_metric_exporter_options.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/sdk/metrics/aggregation/default_aggregation.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h"
#include "opentelemetry/sdk/metrics/meter_context_factory.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include "opentelemetry/sdk/metrics/meter_provider_factory.h"

#include "opentelemetry/exporters/otlp/otlp_http_log_record_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_http_log_record_exporter_options.h"
#include "opentelemetry/logs/provider.h"
#include "opentelemetry/sdk/logs/logger_provider_factory.h"
#include "opentelemetry/sdk/logs/processor.h"
#include "opentelemetry/sdk/logs/simple_log_record_processor_factory.h"

#include <opentelemetry/context/propagation/global_propagator.h>
#include <opentelemetry/trace/propagation/http_trace_context.h>

#include "opentelemetry/ext/http/client/http_client_factory.h"
#include "opentelemetry/ext/http/common/url_parser.h"
#include "opentelemetry/trace/semantic_conventions.h"

namespace trace_api = opentelemetry::trace;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace trace_exporter = opentelemetry::exporter::trace;

namespace metrics_sdk = opentelemetry::sdk::metrics;
namespace common = opentelemetry::common;
namespace exportermetrics = opentelemetry::exporter::metrics;
namespace metrics_api = opentelemetry::metrics;

namespace logs_api = opentelemetry::logs;
namespace logs_sdk = opentelemetry::sdk::logs;
namespace logs_exporter = opentelemetry::exporter::logs;

namespace otlp = opentelemetry::exporter::otlp;

namespace resource = opentelemetry::sdk::resource;

namespace context = opentelemetry::context;
namespace propagation = opentelemetry::context::propagation;

namespace http_client = opentelemetry::ext::http::client;

void InitTracer() {
  auto exporter = trace_exporter::OStreamSpanExporterFactory::Create();
  auto processor = trace_sdk::SimpleSpanProcessorFactory::Create(std::move(exporter));
  std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
      trace_sdk::TracerProviderFactory::Create(std::move(processor));
  // set the global trace provider
  // trace_api::Provider::SetTracerProvider(provider);

  const std::shared_ptr<trace_api::TracerProvider> &api_provider = provider;
  trace_api::Provider::SetTracerProvider(api_provider);
}

void InitTracerHttp() {
  auto resource_attributes = resource::ResourceAttributes{
      {"service.name", "service_1"},
      {"sn", "123456"}};
  auto resource = resource::Resource::Create(resource_attributes);

  trace_sdk::BatchSpanProcessorOptions bspOpts{};
  otlp::OtlpHttpExporterOptions opts;
  opts.url = "http://localhost:4318/v1/traces";
  auto exporter = otlp::OtlpHttpExporterFactory::Create(opts);
  auto processor = trace_sdk::BatchSpanProcessorFactory::Create(std::move(exporter), bspOpts);
  std::shared_ptr<trace_api::TracerProvider> provider = trace_sdk::TracerProviderFactory::Create(std::move(processor), resource);
  trace_api::Provider::SetTracerProvider(provider);

  // set global propagator
  opentelemetry::context::propagation::GlobalTextMapPropagator::SetGlobalPropagator(
      opentelemetry::nostd::shared_ptr<opentelemetry::context::propagation::TextMapPropagator>(
          new opentelemetry::trace::propagation::HttpTraceContext()));
}

void CleanupTracer() {
  std::shared_ptr<opentelemetry::trace::TracerProvider> none;
  trace_api::Provider::SetTracerProvider(none);
}

template <typename T>
class HttpTextMapCarrier : public opentelemetry::context::propagation::TextMapCarrier {
 public:
  HttpTextMapCarrier(const T &headers) : headers_(headers) {}
  HttpTextMapCarrier() = default;
  virtual opentelemetry::nostd::string_view Get(
      opentelemetry::nostd::string_view key) const noexcept override {
    std::string key_to_compare = key.data();
    // Header's first letter seems to be  automatically capitaliazed by our test http-server, so
    // compare accordingly.
    // if (key == opentelemetry::trace::propagation::kTraceParent) {
    //   key_to_compare = "Traceparent";
    // } else if (key == opentelemetry::trace::propagation::kTraceState) {
    //   key_to_compare = "Tracestate";
    // }
    auto it = headers_.find(key_to_compare);
    if (it != headers_.end()) {
      return it->second;
    }
    return "";
  }

  virtual void Set(opentelemetry::nostd::string_view key,
                   opentelemetry::nostd::string_view value) noexcept override {
    headers_.insert(std::pair<std::string, std::string>(std::string(key), std::string(value)));
  }

  T headers_;
};

void bar(const std::map<std::string, std::string> &headers) {
  DBG_PRINT("+++++++++++++++++ %lu", headers.size());
  for (auto &itr : headers) {
    DBG_PRINT("%s : %s", itr.first.c_str(), itr.second.c_str());
  }
  DBG_PRINT("-------------------");

  HttpTextMapCarrier<std::map<std::string, std::string>> carrier(headers);
  auto propagator = propagation::GlobalTextMapPropagator::GetGlobalPropagator();
  auto current_ctx = context::RuntimeContext::GetCurrent();
  context::Context ctx = propagator->Extract(carrier, current_ctx);

  auto tracer = opentelemetry::trace::Provider::GetTracerProvider()->GetTracer("my-app-tracer-2");

  trace_api::StartSpanOptions options{
      .parent = trace_api::GetSpan(ctx)->GetContext(),
      .kind = trace_api::SpanKind::kServer,
  };

  auto span = tracer->StartSpan("sp-bar", {{"kkk", "vvv"}}, options);
  span->SetAttribute("k1", "v1");
  span->SetAttribute("k2", "v2");

  span->AddEvent("start");

  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  span->AddEvent("end");

  span->SetStatus(trace_api::StatusCode::kOk);
  span->End();
}

void foo() {
  auto tracer = opentelemetry::trace::Provider::GetTracerProvider()->GetTracer("my-app-tracer");

  trace_api::StartSpanOptions options{
      .kind = trace_api::SpanKind::kClient,
  };
  auto span = tracer->StartSpan("sp-foo", {{"111", "222"}}, options);

  // auto scope = tracer->WithActiveSpan(span);

  span->SetAttribute("k3", "v3");
  span->SetAttribute("k4", "v4");

  span->AddEvent("start");

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  span->AddEvent("start call bar");

  // auto current_ctx = context::RuntimeContext::GetCurrent();
  HttpTextMapCarrier<std::map<std::string, std::string>> carrier;
  auto propagator = propagation::GlobalTextMapPropagator::GetGlobalPropagator();
  // propagator->Inject(carrier, current_ctx);
  opentelemetry::context::Context temp_ctx(trace_api::kSpanKey, span);
  propagator->Inject(carrier, temp_ctx);

  DBG_PRINT("+++++++++++++++++ %lu", carrier.headers_.size());
  for (auto &itr : carrier.headers_) {
    DBG_PRINT("%s : %s", itr.first.c_str(), itr.second.c_str());
  }
  DBG_PRINT("-------------------");

  for (int i = 0; i < 3; ++i) {
    std::thread([&]() {
      bar(carrier.headers_);
    }).detach();
  }
  span->AddEvent("end call bar");

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  span->AddEvent("end");

  span->SetStatus(trace_api::StatusCode::kOk);
  span->End();
}

void InitMetrics(const std::string &name) {
  auto exporter = exportermetrics::OStreamMetricExporterFactory::Create();

  std::string version{"1.2.0"};
  std::string schema{"https://opentelemetry.io/schemas/1.2.0"};

  // Initialize and set the global MeterProvider
  metrics_sdk::PeriodicExportingMetricReaderOptions options;
  options.export_interval_millis = std::chrono::milliseconds(1000);
  options.export_timeout_millis = std::chrono::milliseconds(500);

  auto reader = metrics_sdk::PeriodicExportingMetricReaderFactory::Create(std::move(exporter), options);

  auto provider = opentelemetry::sdk::metrics::MeterProviderFactory::Create();

  provider->AddMetricReader(std::move(reader));

  // counter view
  std::string counter_name = name + "_counter";
  std::string unit = "counter-unit";

  auto instrument_selector = metrics_sdk::InstrumentSelectorFactory::Create(
      metrics_sdk::InstrumentType::kCounter, counter_name, unit);

  auto meter_selector = metrics_sdk::MeterSelectorFactory::Create(name, version, schema);

  auto sum_view = metrics_sdk::ViewFactory::Create(name, "description", unit,
                                                   metrics_sdk::AggregationType::kSum);

  provider->AddView(std::move(instrument_selector), std::move(meter_selector), std::move(sum_view));

  // observable counter view
  std::string observable_counter_name = name + "_observable_counter";

  auto observable_instrument_selector = metrics_sdk::InstrumentSelectorFactory::Create(
      metrics_sdk::InstrumentType::kObservableCounter, observable_counter_name, unit);

  auto observable_meter_selector = metrics_sdk::MeterSelectorFactory::Create(name, version, schema);

  auto observable_sum_view = metrics_sdk::ViewFactory::Create(name, "test_description", unit,
                                                              metrics_sdk::AggregationType::kSum);

  provider->AddView(std::move(observable_instrument_selector), std::move(observable_meter_selector),
                    std::move(observable_sum_view));

  // histogram view
  std::string histogram_name = name + "_histogram";
  unit = "histogram-unit";

  auto histogram_instrument_selector = metrics_sdk::InstrumentSelectorFactory::Create(
      metrics_sdk::InstrumentType::kHistogram, histogram_name, unit);

  auto histogram_meter_selector = metrics_sdk::MeterSelectorFactory::Create(name, version, schema);

  auto histogram_aggregation_config = std::unique_ptr<metrics_sdk::HistogramAggregationConfig>(
      new metrics_sdk::HistogramAggregationConfig);

  histogram_aggregation_config->boundaries_ = std::vector<double>{
      0.0, 50.0, 100.0, 250.0, 500.0, 750.0, 1000.0, 2500.0, 5000.0, 10000.0, 20000.0};

  std::shared_ptr<metrics_sdk::AggregationConfig> aggregation_config(
      std::move(histogram_aggregation_config));

  auto histogram_view = metrics_sdk::ViewFactory::Create(
      name, "description", unit, metrics_sdk::AggregationType::kHistogram, aggregation_config);

  provider->AddView(std::move(histogram_instrument_selector), std::move(histogram_meter_selector),
                    std::move(histogram_view));

  std::shared_ptr<opentelemetry::metrics::MeterProvider> api_provider(std::move(provider));

  metrics_api::Provider::SetMeterProvider(api_provider);
}

void CleanupMetrics() {
  std::shared_ptr<metrics_api::MeterProvider> none;
  metrics_api::Provider::SetMeterProvider(none);
}

namespace {

static opentelemetry::nostd::shared_ptr<metrics_api::ObservableInstrument>
    double_observable_counter;

std::map<std::string, std::string> get_random_attr() {
  const std::vector<std::pair<std::string, std::string>> labels = {{"key1", "value1"},
                                                                   {"key2", "value2"},
                                                                   {"key3", "value3"},
                                                                   {"key4", "value4"},
                                                                   {"key5", "value5"}};
  return std::map<std::string, std::string>{labels[rand() % (labels.size() - 1)],
                                            labels[rand() % (labels.size() - 1)]};
}

class MeasurementFetcher {
 public:
  static void Fetcher(opentelemetry::metrics::ObserverResult observer_result, void * /* state */) {
    if (opentelemetry::nostd::holds_alternative<
            opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObserverResultT<double>>>(
            observer_result)) {
      // double random_incr = (rand() % 5) + 1.1;
      value_ += 100.0;
      std::map<std::string, std::string> labels = get_random_attr();
      opentelemetry::nostd::get<
          opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObserverResultT<double>>>(
          observer_result)
          ->Observe(value_, labels);
    }
  }
  static double value_;
};
double MeasurementFetcher::value_ = 0.0;
}  // namespace

void counter_example(const std::string &name) {
  std::string counter_name = name + "_counter";
  auto provider = metrics_api::Provider::GetMeterProvider();
  opentelemetry::nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter(name, "1.2.0");
  auto double_counter = meter->CreateDoubleCounter(counter_name);

  for (uint32_t i = 0; i < 20; ++i) {
    // double val = (rand() % 700) + 1.1;
    double_counter->Add(100.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void observable_counter_example(const std::string &name) {
  std::string counter_name = name + "_observable_counter";
  auto provider = metrics_api::Provider::GetMeterProvider();
  opentelemetry::nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter(name, "1.2.0");
  double_observable_counter = meter->CreateDoubleObservableCounter(counter_name);
  double_observable_counter->AddCallback(MeasurementFetcher::Fetcher, nullptr);
  for (uint32_t i = 0; i < 20; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void histogram_example(const std::string &name) {
  std::string histogram_name = name + "_histogram";
  auto provider = metrics_api::Provider::GetMeterProvider();
  opentelemetry::nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter(name, "1.2.0");
  auto histogram_counter = meter->CreateDoubleHistogram(histogram_name, "des", "unit");
  auto context = opentelemetry::context::Context{};
  for (uint32_t i = 0; i < 20; ++i) {
    // double val = (rand() % 700) + 1.1;
    std::map<std::string, std::string> labels = get_random_attr();
    auto labelkv = opentelemetry::common::KeyValueIterableView<decltype(labels)>{labels};
    histogram_counter->Record(100.0, labelkv, context);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }
}

void InitLogger() {
  // Create ostream log exporter instance
  auto exporter =
      std::unique_ptr<logs_sdk::LogRecordExporter>(new logs_exporter::OStreamLogRecordExporter);
  auto processor = logs_sdk::SimpleLogRecordProcessorFactory::Create(std::move(exporter));

  std::shared_ptr<opentelemetry::sdk::logs::LoggerProvider> provider(
      opentelemetry::sdk::logs::LoggerProviderFactory::Create(std::move(processor)));

  // Set the global logger provider
  const std::shared_ptr<logs_api::LoggerProvider> &api_provider = provider;
  logs_api::Provider::SetLoggerProvider(api_provider);
}

void CleanupLogger() {
  std::shared_ptr<logs_api::LoggerProvider> none;
  logs_api::Provider::SetLoggerProvider(none);
}

namespace logs = opentelemetry::logs;
namespace trace = opentelemetry::trace;

opentelemetry::nostd::shared_ptr<trace::Tracer> get_tracer() {
  auto provider = trace::Provider::GetTracerProvider();
  return provider->GetTracer("foo_library", OPENTELEMETRY_SDK_VERSION);
}

opentelemetry::nostd::shared_ptr<logs::Logger> get_logger() {
  auto provider = logs::Provider::GetLoggerProvider();
  return provider->GetLogger("foo_library_logger", "foo_library");
}

int32_t main(int32_t argc, char **argv) {
  DBG_PRINT("hello world");

  // InitTracer();
  InitTracerHttp();

  std::this_thread::sleep_for(std::chrono::seconds(1));

  foo();

  std::this_thread::sleep_for(std::chrono::seconds(1));

  CleanupTracer();

  //////////////////////////////////////////////

  // std::string name{"ostream_metric_example"};
  // InitMetrics(name);

  // // counter_example(name);
  // // observable_counter_example(name);
  // histogram_example(name);

  // CleanupMetrics();

  //////////////////////////////////////////////

  // InitTracer();
  // InitLogger();

  // auto span = get_tracer()->StartSpan("span 1");
  // // auto scoped_span = trace::Scope(get_tracer()->StartSpan("foo_library"));
  // auto ctx = span->GetContext();
  // auto logger = get_logger();

  // logger->Debug("body", ctx.trace_id(), ctx.span_id(), ctx.trace_flags());

  // span->End();

  // std::this_thread::sleep_for(std::chrono::seconds(1));

  // CleanupTracer();
  // CleanupLogger();

  return 0;
}
