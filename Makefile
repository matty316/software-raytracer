# GNU Make workspace makefile autogenerated by Premake

ifndef config
  config=debug_x64
endif

ifndef verbose
  SILENT = @
endif

ifeq ($(config),debug_x64)
  raylib_config = debug_x64
  raytracing_config = debug_x64
endif
ifeq ($(config),debug_x86)
  raylib_config = debug_x86
  raytracing_config = debug_x86
endif
ifeq ($(config),debug_arm64)
  raylib_config = debug_arm64
  raytracing_config = debug_arm64
endif
ifeq ($(config),release_x64)
  raylib_config = release_x64
  raytracing_config = release_x64
endif
ifeq ($(config),release_x86)
  raylib_config = release_x86
  raytracing_config = release_x86
endif
ifeq ($(config),release_arm64)
  raylib_config = release_arm64
  raytracing_config = release_arm64
endif

PROJECTS := raylib raytracing

.PHONY: all clean help $(PROJECTS) 

all: $(PROJECTS)

raylib:
ifneq (,$(raylib_config))
	@echo "==== Building raylib ($(raylib_config)) ===="
	@${MAKE} --no-print-directory -C raylib-master -f Makefile config=$(raylib_config)
endif

raytracing: raylib
ifneq (,$(raytracing_config))
	@echo "==== Building raytracing ($(raytracing_config)) ===="
	@${MAKE} --no-print-directory -C game -f Makefile config=$(raytracing_config)
endif

clean:
	@${MAKE} --no-print-directory -C raylib-master -f Makefile clean
	@${MAKE} --no-print-directory -C game -f Makefile clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "  debug_x64"
	@echo "  debug_x86"
	@echo "  debug_arm64"
	@echo "  release_x64"
	@echo "  release_x86"
	@echo "  release_arm64"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   raylib"
	@echo "   raytracing"
	@echo ""
	@echo "For more information, see https://github.com/premake/premake-core/wiki"