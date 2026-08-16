PRESET ?= x64-debug
TEST_PRESET ?= x64-debug-test

arch = $(firstword $(subst -, ,$(1)))
build_dir = build/$(call arch,$(1))/$(patsubst $(call arch,$(1))-%,%,$(1))

BUILD_DIR := $(call build_dir,$(PRESET))
TEST_BUILD_DIR := $(call build_dir,$(TEST_PRESET))
JOBS ?= $(shell nproc)
SOURCES = $(shell find include lib tests \( -name '*.hpp' -o -name '*.cpp' \))

.PHONY: all build configure clean distclean format format-check rebuild \
        test test-configure test-build

all: build

configure:
	cmake --preset $(PRESET)

$(BUILD_DIR)/CMakeCache.txt:
	@$(MAKE) configure

build: $(BUILD_DIR)/CMakeCache.txt
	cmake --build $(BUILD_DIR) -j $(JOBS)

clean:
	@cmake --build $(BUILD_DIR) --target clean 2>/dev/null || true

distclean:
	rm -rf build compile_commands.json

rebuild: distclean build

format:
	clang-format -i $(SOURCES)

format-check:
	clang-format --dry-run -Werror $(SOURCES)

test-configure:
	cmake --preset $(TEST_PRESET)

$(TEST_BUILD_DIR)/CMakeCache.txt:
	@$(MAKE) test-configure

test-build: $(TEST_BUILD_DIR)/CMakeCache.txt
	cmake --build $(TEST_BUILD_DIR) -j $(JOBS)

test: test-build
	ctest --test-dir $(TEST_BUILD_DIR) --output-on-failure $(TEST_ARGS)
