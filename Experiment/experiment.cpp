#include "experiment.h"

bool RunNamedExperiment(const std::string &name, const ExperimentConfig &config) {
    if (name == "self-test")
        return TestSelf(config);
    if (name == "uniform-z")
        return TestUniformZ(config);
    if (name == "single-layer")
        return TestSingleLayer(config);
    if (name == "leaf-size")
        return TestLeafSize(config);
    if (name == "all") {
        return TestSelf(config) && TestUniformZ(config) &&
               TestSingleLayer(config) && TestLeafSize(config);
    }

    spdlog::error("Unknown experiment: {}", name);
    return false;
}
