// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "tfx_bsl/cc/statistics/statistics_sql_submodule.h"

#include <stdexcept>
#include <string>

#include "absl/status/status.h"
#include "absl/types/optional.h"
#include "tfx_bsl/cc/statistics/sql_util.h"
#include "pybind11/attr.h"
#include "pybind11/pybind11.h"
#include "tensorflow_metadata/proto/v0/statistics.pb.h"

namespace tfx_bsl {
namespace {
namespace py = pybind11;

using tensorflow::metadata::v0::FeatureNameStatistics;

void DefineEvaluatePredicate(py::module m) {
  m.def(
      "EvaluateUnaryStatsPredicate",
      [](const std::string& feature_stats_serialized,
         const std::string& query) {
        FeatureNameStatistics feature_stats;
        if (!feature_stats.ParseFromString(feature_stats_serialized)) {
          throw std::runtime_error("Failed to parse FeatureNameStatistics.");
        }
        zetasql_base::StatusOr<bool> result_or =
            statistics::EvaluatePredicate(feature_stats, query);
        if (!result_or.ok()) {
          throw std::runtime_error(result_or.status().ToString());
        }
        return result_or.value();
      },
      py::doc("Evaluates a SQL predicate over a single FeatureNameStatistics "
              "proto bound to 'feature'."),
      py::call_guard<py::gil_scoped_release>());
  m.def(
      "EvaluateBinaryStatsPredicate",
      [](const std::string& base_feature_stats_serialized,
         const std::string& test_feature_stats_serialized,
         const std::string& query) {
        FeatureNameStatistics base_feature_stats;
        if (!base_feature_stats.ParseFromString(
                base_feature_stats_serialized)) {
          throw std::runtime_error("Failed to parse FeatureNameStatistics.");
        }
        FeatureNameStatistics test_feature_stats;
        if (!test_feature_stats.ParseFromString(
                test_feature_stats_serialized)) {
          throw std::runtime_error("Failed to parse FeatureNameStatistics.");
        }
        zetasql_base::StatusOr<bool> result_or = statistics::EvaluatePredicate(
            base_feature_stats, test_feature_stats, query);
        if (!result_or.ok()) {
          throw std::runtime_error(result_or.status().ToString());
        }
        return result_or.value();
      },
      py::doc("Evaluates a SQL predicate over a pair of FeatureNameStatistics "
              "protos bound to 'feature_base' and 'feature_test'."),
      py::call_guard<py::gil_scoped_release>());
}
}  // namespace

void DefineStatisticsSqlSubmodule(py::module main_module) {
  auto m = main_module.def_submodule("statistics_sql");
  m.doc() = "Pybind11 bindings for (TFDV) statistics sql utilities.";
  DefineEvaluatePredicate(m);
}

}  // namespace tfx_bsl
