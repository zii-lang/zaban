PRESET ?= x64-debug
BUILD_DIR := build/$(subst -,/,$(PRESET))
JOBS ?= $(shell nproc)

.PHONY: all build configure clean distclean format format-check rebuild

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
	find include lib -name '*.hpp' -o -name '*.cpp' | xargs clang-format -i

format-check:
	find include lib -name '*.hpp' -o -name '*.cpp' | xargs clang-format --dry-run -Werror
