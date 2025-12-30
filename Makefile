TARGET := STM32_Debugger_Plotter

CXX := g++
AR := ar

BUILD_DIR := build
OBJS_DIR := $(BUILD_DIR)/objs
BIN_DIR := $(BUILD_DIR)/bin

RAYLIB_DIR := lib/raylib
IMGUI_DIR := lib/imgui
IMPLOT_DIR := lib/implot
RLIMGUI_DIR := lib/rlImGui

RAYLIB_SRC_DIR := $(RAYLIB_DIR)/src
RAYLIB_LIB := $(RAYLIB_SRC_DIR)/libraylib.a

IMGUI_SRC := \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp

IMPLOT_SRC := \
	$(IMPLOT_DIR)/implot.cpp \
	$(IMPLOT_DIR)/implot_items.cpp

RLIMGUI_SRC := $(RLIMGUI_DIR)/rlImGui.cpp

PROJ_SRCS := \
	src/debug/STM32Detector.cpp \
	src/debug/test_detector.cpp \


SRCS := $(PROJ_SRCS) $(IMGUI_SRC) $(IMPLOT_SRC) $(RLIMGUI_SRC)

INCLUDES := -Isrc -I$(RAYLIB_SRC_DIR) -I$(IMGUI_DIR) -I$(IMPLOT_DIR) -I$(RLIMGUI_DIR)

CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wno-missing-field-initializers $(INCLUDES)

LDFLAGS :=
LDLIBS :=
EXE :=

# OS Detection
ifeq ($(OS),Windows_NT)
	LDLIBS += -lopengl32 -lgdi32 -lwinmm -lshell32 -lws2_32
	EXE := .exe
	MKDIR = if not exist "$(subst /,\,$1)" mkdir "$(subst /,\,$1)"
	RM = if exist "$(subst /,\,$(BUILD_DIR))" rmdir /S /Q "$(subst /,\,$(BUILD_DIR))"
	RM_RAYLIB = $(MAKE) -C $(RAYLIB_SRC_DIR) clean
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		LDLIBS += -lGL -lm -lpthread -ldl -lrt -lX11
	endif
	ifeq ($(UNAME_S),Darwin)
		LDLIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	endif
	MKDIR = mkdir -p $1
	RM = rm -rf $(BUILD_DIR)
	RM_RAYLIB = $(MAKE) -C $(RAYLIB_SRC_DIR) clean
endif

OBJS := $(patsubst %.cpp,$(OBJS_DIR)/%.o,$(SRCS))

.PHONY: all run clean raylib submodules

all: $(BIN_DIR)/$(TARGET)$(EXE)

submodules:
	git submodule update --init --recursive

raylib: $(RAYLIB_LIB)

$(RAYLIB_LIB):
	$(MAKE) -C $(RAYLIB_SRC_DIR) PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC

$(BIN_DIR)/$(TARGET)$(EXE): $(RAYLIB_LIB) $(OBJS)
	$(call MKDIR,$(BIN_DIR))
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(RAYLIB_LIB) $(LDLIBS)

$(OBJS_DIR)/%.o: %.cpp
	$(call MKDIR,$(dir $@))
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	$(BIN_DIR)/$(TARGET)$(EXE)

clean:
	$(RM)
	-$(RM_RAYLIB)

