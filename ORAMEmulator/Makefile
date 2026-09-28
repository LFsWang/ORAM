CXX ?= g++
AR ?= ar
BUILD_DIR ?= build
CXXFLAGS ?= -O3
# spdlog requires thread support during both compilation and linking.
THREAD_FLAGS ?= -pthread
CPPFLAGS = -I./3rd_lib/spdlog-1.15.3/include -DSPDLOG_COMPILED_LIB
WARNFLAGS = -Wall -Wextra -Wpedantic
COMMON_FLAGS = -std=c++20 $(CPPFLAGS) $(WARNFLAGS) $(CXXFLAGS) $(THREAD_FLAGS) -MMD -MP

APP_SOURCES = main.cpp \
	Experiment/experiment.cpp \
	Experiment/00-selfTest.cpp \
	Experiment/01-Zeq3456.cpp \
	Experiment/02-Z45.cpp
SPDLOG_SOURCES = \
	3rd_lib/spdlog-1.15.3/src/spdlog.cpp \
	3rd_lib/spdlog-1.15.3/src/stdout_sinks.cpp \
	3rd_lib/spdlog-1.15.3/src/color_sinks.cpp \
	3rd_lib/spdlog-1.15.3/src/file_sinks.cpp \
	3rd_lib/spdlog-1.15.3/src/async.cpp \
	3rd_lib/spdlog-1.15.3/src/cfg.cpp \
	3rd_lib/spdlog-1.15.3/src/bundled_fmtlib_format.cpp

APP_OBJECTS = $(APP_SOURCES:%.cpp=$(BUILD_DIR)/obj/%.o)
OMP_OBJECTS = $(APP_SOURCES:%.cpp=$(BUILD_DIR)/obj-omp/%.o)
SPDLOG_OBJECTS = $(SPDLOG_SOURCES:%.cpp=$(BUILD_DIR)/obj/%.o)
SPDLOG_LIBRARY = $(BUILD_DIR)/libspdlog.a

.PHONY: all omp quick clean

all: $(BUILD_DIR)/oram-emulator.exe

omp: $(BUILD_DIR)/oram-emulator-omp.exe

$(BUILD_DIR)/oram-emulator.exe: $(APP_OBJECTS) $(SPDLOG_LIBRARY)
	$(CXX) $(APP_OBJECTS) $(SPDLOG_LIBRARY) $(THREAD_FLAGS) -o $@

$(BUILD_DIR)/oram-emulator-omp.exe: $(OMP_OBJECTS) $(SPDLOG_LIBRARY)
	$(CXX) $(OMP_OBJECTS) $(SPDLOG_LIBRARY) $(THREAD_FLAGS) -fopenmp -o $@

$(SPDLOG_LIBRARY): $(SPDLOG_OBJECTS)
	$(AR) rcs $@ $^

$(BUILD_DIR)/obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(COMMON_FLAGS) -c $< -o $@

$(BUILD_DIR)/obj-omp/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(COMMON_FLAGS) -fopenmp -c $< -o $@

quick: all
	$(BUILD_DIR)/oram-emulator.exe --experiment all --profile quick

clean:
	$(RM) -r $(BUILD_DIR)

-include $(APP_OBJECTS:.o=.d) $(OMP_OBJECTS:.o=.d) $(SPDLOG_OBJECTS:.o=.d)
