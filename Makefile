# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Produce verbose output by default.
VERBOSE = 1

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/shuaishuai/project/sylar_server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/shuaishuai/project/sylar_server

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/local/bin/ccmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/local/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/shuaishuai/project/sylar_server/CMakeFiles /home/shuaishuai/project/sylar_server//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/shuaishuai/project/sylar_server/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named server

# Build rule for target.
server: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 server
.PHONY : server

# fast build rule for target.
server/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/build
.PHONY : server/fast

#=============================================================================
# Target rules for targets named test

# Build rule for target.
test: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 test
.PHONY : test

# fast build rule for target.
test/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/build
.PHONY : test/fast

#=============================================================================
# Target rules for targets named test_config

# Build rule for target.
test_config: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 test_config
.PHONY : test_config

# fast build rule for target.
test_config/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_config.dir/build.make CMakeFiles/test_config.dir/build
.PHONY : test_config/fast

#=============================================================================
# Target rules for targets named test_thread

# Build rule for target.
test_thread: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 test_thread
.PHONY : test_thread

# fast build rule for target.
test_thread/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_thread.dir/build.make CMakeFiles/test_thread.dir/build
.PHONY : test_thread/fast

#=============================================================================
# Target rules for targets named test_util

# Build rule for target.
test_util: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 test_util
.PHONY : test_util

# fast build rule for target.
test_util/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_util.dir/build.make CMakeFiles/test_util.dir/build
.PHONY : test_util/fast

#=============================================================================
# Target rules for targets named test_fiber

# Build rule for target.
test_fiber: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 test_fiber
.PHONY : test_fiber

# fast build rule for target.
test_fiber/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_fiber.dir/build.make CMakeFiles/test_fiber.dir/build
.PHONY : test_fiber/fast

server/config.o: server/config.cc.o
.PHONY : server/config.o

# target to build an object file
server/config.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/config.cc.o
.PHONY : server/config.cc.o

server/config.i: server/config.cc.i
.PHONY : server/config.i

# target to preprocess a source file
server/config.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/config.cc.i
.PHONY : server/config.cc.i

server/config.s: server/config.cc.s
.PHONY : server/config.s

# target to generate assembly for a file
server/config.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/config.cc.s
.PHONY : server/config.cc.s

server/fiber.o: server/fiber.cc.o
.PHONY : server/fiber.o

# target to build an object file
server/fiber.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/fiber.cc.o
.PHONY : server/fiber.cc.o

server/fiber.i: server/fiber.cc.i
.PHONY : server/fiber.i

# target to preprocess a source file
server/fiber.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/fiber.cc.i
.PHONY : server/fiber.cc.i

server/fiber.s: server/fiber.cc.s
.PHONY : server/fiber.s

# target to generate assembly for a file
server/fiber.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/fiber.cc.s
.PHONY : server/fiber.cc.s

server/log.o: server/log.cc.o
.PHONY : server/log.o

# target to build an object file
server/log.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/log.cc.o
.PHONY : server/log.cc.o

server/log.i: server/log.cc.i
.PHONY : server/log.i

# target to preprocess a source file
server/log.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/log.cc.i
.PHONY : server/log.cc.i

server/log.s: server/log.cc.s
.PHONY : server/log.s

# target to generate assembly for a file
server/log.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/log.cc.s
.PHONY : server/log.cc.s

server/thread.o: server/thread.cc.o
.PHONY : server/thread.o

# target to build an object file
server/thread.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/thread.cc.o
.PHONY : server/thread.cc.o

server/thread.i: server/thread.cc.i
.PHONY : server/thread.i

# target to preprocess a source file
server/thread.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/thread.cc.i
.PHONY : server/thread.cc.i

server/thread.s: server/thread.cc.s
.PHONY : server/thread.s

# target to generate assembly for a file
server/thread.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/thread.cc.s
.PHONY : server/thread.cc.s

server/util.o: server/util.cc.o
.PHONY : server/util.o

# target to build an object file
server/util.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/util.cc.o
.PHONY : server/util.cc.o

server/util.i: server/util.cc.i
.PHONY : server/util.i

# target to preprocess a source file
server/util.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/util.cc.i
.PHONY : server/util.cc.i

server/util.s: server/util.cc.s
.PHONY : server/util.s

# target to generate assembly for a file
server/util.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/server/util.cc.s
.PHONY : server/util.cc.s

tests/test.o: tests/test.cc.o
.PHONY : tests/test.o

# target to build an object file
tests/test.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/tests/test.cc.o
.PHONY : tests/test.cc.o

tests/test.i: tests/test.cc.i
.PHONY : tests/test.i

# target to preprocess a source file
tests/test.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/tests/test.cc.i
.PHONY : tests/test.cc.i

tests/test.s: tests/test.cc.s
.PHONY : tests/test.s

# target to generate assembly for a file
tests/test.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/tests/test.cc.s
.PHONY : tests/test.cc.s

tests/test_config.o: tests/test_config.cc.o
.PHONY : tests/test_config.o

# target to build an object file
tests/test_config.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_config.dir/build.make CMakeFiles/test_config.dir/tests/test_config.cc.o
.PHONY : tests/test_config.cc.o

tests/test_config.i: tests/test_config.cc.i
.PHONY : tests/test_config.i

# target to preprocess a source file
tests/test_config.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_config.dir/build.make CMakeFiles/test_config.dir/tests/test_config.cc.i
.PHONY : tests/test_config.cc.i

tests/test_config.s: tests/test_config.cc.s
.PHONY : tests/test_config.s

# target to generate assembly for a file
tests/test_config.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_config.dir/build.make CMakeFiles/test_config.dir/tests/test_config.cc.s
.PHONY : tests/test_config.cc.s

tests/test_fiber.o: tests/test_fiber.cc.o
.PHONY : tests/test_fiber.o

# target to build an object file
tests/test_fiber.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_fiber.dir/build.make CMakeFiles/test_fiber.dir/tests/test_fiber.cc.o
.PHONY : tests/test_fiber.cc.o

tests/test_fiber.i: tests/test_fiber.cc.i
.PHONY : tests/test_fiber.i

# target to preprocess a source file
tests/test_fiber.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_fiber.dir/build.make CMakeFiles/test_fiber.dir/tests/test_fiber.cc.i
.PHONY : tests/test_fiber.cc.i

tests/test_fiber.s: tests/test_fiber.cc.s
.PHONY : tests/test_fiber.s

# target to generate assembly for a file
tests/test_fiber.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_fiber.dir/build.make CMakeFiles/test_fiber.dir/tests/test_fiber.cc.s
.PHONY : tests/test_fiber.cc.s

tests/test_thread.o: tests/test_thread.cc.o
.PHONY : tests/test_thread.o

# target to build an object file
tests/test_thread.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_thread.dir/build.make CMakeFiles/test_thread.dir/tests/test_thread.cc.o
.PHONY : tests/test_thread.cc.o

tests/test_thread.i: tests/test_thread.cc.i
.PHONY : tests/test_thread.i

# target to preprocess a source file
tests/test_thread.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_thread.dir/build.make CMakeFiles/test_thread.dir/tests/test_thread.cc.i
.PHONY : tests/test_thread.cc.i

tests/test_thread.s: tests/test_thread.cc.s
.PHONY : tests/test_thread.s

# target to generate assembly for a file
tests/test_thread.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_thread.dir/build.make CMakeFiles/test_thread.dir/tests/test_thread.cc.s
.PHONY : tests/test_thread.cc.s

tests/test_util.o: tests/test_util.cc.o
.PHONY : tests/test_util.o

# target to build an object file
tests/test_util.cc.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_util.dir/build.make CMakeFiles/test_util.dir/tests/test_util.cc.o
.PHONY : tests/test_util.cc.o

tests/test_util.i: tests/test_util.cc.i
.PHONY : tests/test_util.i

# target to preprocess a source file
tests/test_util.cc.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_util.dir/build.make CMakeFiles/test_util.dir/tests/test_util.cc.i
.PHONY : tests/test_util.cc.i

tests/test_util.s: tests/test_util.cc.s
.PHONY : tests/test_util.s

# target to generate assembly for a file
tests/test_util.cc.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_util.dir/build.make CMakeFiles/test_util.dir/tests/test_util.cc.s
.PHONY : tests/test_util.cc.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... server"
	@echo "... test"
	@echo "... test_config"
	@echo "... test_fiber"
	@echo "... test_thread"
	@echo "... test_util"
	@echo "... server/config.o"
	@echo "... server/config.i"
	@echo "... server/config.s"
	@echo "... server/fiber.o"
	@echo "... server/fiber.i"
	@echo "... server/fiber.s"
	@echo "... server/log.o"
	@echo "... server/log.i"
	@echo "... server/log.s"
	@echo "... server/thread.o"
	@echo "... server/thread.i"
	@echo "... server/thread.s"
	@echo "... server/util.o"
	@echo "... server/util.i"
	@echo "... server/util.s"
	@echo "... tests/test.o"
	@echo "... tests/test.i"
	@echo "... tests/test.s"
	@echo "... tests/test_config.o"
	@echo "... tests/test_config.i"
	@echo "... tests/test_config.s"
	@echo "... tests/test_fiber.o"
	@echo "... tests/test_fiber.i"
	@echo "... tests/test_fiber.s"
	@echo "... tests/test_thread.o"
	@echo "... tests/test_thread.i"
	@echo "... tests/test_thread.s"
	@echo "... tests/test_util.o"
	@echo "... tests/test_util.i"
	@echo "... tests/test_util.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

