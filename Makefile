.PHONY: all vkStuff helloTriangle utils main clean shaders

CC=g++ --std=c++20
ARGS=-lvulkan -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -o

SLANG ?= slangc
ENTRY_POINTS := -entry vertMain -entry fragMain
SLANG_ARGS = -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name

SHADER_SRC := shaders

APPS_PATH=src/applications
UTILS_PATH= src/utils
BUILD_PATH= build

UTILS=utils
TRIANGLE=helloTriangleApp

PROGS=$(UTILS) $(TRIANGLE)

all: $(PROGS)

$(TRIANGLE): $(UTILS)
	$(SLANG) $(SHADER_SRC)/shaders.slang $(SLANG_ARGS) $(ENTRY_POINTS) -o $(BUILD_PATH)/$(SHADER_SRC).spv
	$(CC) $(APPS_PATH)/$(TRIANGLE).cpp $(BUILD_PATH)/$(UTILS).o $(ARGS) $(BUILD_PATH)/$(TRIANGLE).out

$(UTILS): $(BUILD_PATH)
	$(CC) -c $(UTILS_PATH)/$(UTILS).cpp -o $(BUILD_PATH)/$(UTILS).o

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)

clean:
	rm -rf *.out *.o *.spv
