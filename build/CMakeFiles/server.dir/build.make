# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

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
CMAKE_BINARY_DIR = /home/shuaishuai/project/sylar_server/build

# Include any dependencies generated for this target.
include CMakeFiles/server.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/server.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/server.dir/flags.make

CMakeFiles/server.dir/server/config.cc.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/server/config.cc.o: ../server/config.cc
CMakeFiles/server.dir/server/config.cc.o: CMakeFiles/server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/shuaishuai/project/sylar_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/server.dir/server/config.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/config.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/server.dir/server/config.cc.o -MF CMakeFiles/server.dir/server/config.cc.o.d -o CMakeFiles/server.dir/server/config.cc.o -c /home/shuaishuai/project/sylar_server/server/config.cc

CMakeFiles/server.dir/server/config.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/server/config.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/config.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/shuaishuai/project/sylar_server/server/config.cc > CMakeFiles/server.dir/server/config.cc.i

CMakeFiles/server.dir/server/config.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/server/config.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/config.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/shuaishuai/project/sylar_server/server/config.cc -o CMakeFiles/server.dir/server/config.cc.s

CMakeFiles/server.dir/server/fiber.cc.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/server/fiber.cc.o: ../server/fiber.cc
CMakeFiles/server.dir/server/fiber.cc.o: CMakeFiles/server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/shuaishuai/project/sylar_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/server.dir/server/fiber.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/fiber.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/server.dir/server/fiber.cc.o -MF CMakeFiles/server.dir/server/fiber.cc.o.d -o CMakeFiles/server.dir/server/fiber.cc.o -c /home/shuaishuai/project/sylar_server/server/fiber.cc

CMakeFiles/server.dir/server/fiber.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/server/fiber.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/fiber.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/shuaishuai/project/sylar_server/server/fiber.cc > CMakeFiles/server.dir/server/fiber.cc.i

CMakeFiles/server.dir/server/fiber.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/server/fiber.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/fiber.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/shuaishuai/project/sylar_server/server/fiber.cc -o CMakeFiles/server.dir/server/fiber.cc.s

CMakeFiles/server.dir/server/log.cc.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/server/log.cc.o: ../server/log.cc
CMakeFiles/server.dir/server/log.cc.o: CMakeFiles/server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/shuaishuai/project/sylar_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/server.dir/server/log.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/log.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/server.dir/server/log.cc.o -MF CMakeFiles/server.dir/server/log.cc.o.d -o CMakeFiles/server.dir/server/log.cc.o -c /home/shuaishuai/project/sylar_server/server/log.cc

CMakeFiles/server.dir/server/log.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/server/log.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/log.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/shuaishuai/project/sylar_server/server/log.cc > CMakeFiles/server.dir/server/log.cc.i

CMakeFiles/server.dir/server/log.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/server/log.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/log.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/shuaishuai/project/sylar_server/server/log.cc -o CMakeFiles/server.dir/server/log.cc.s

CMakeFiles/server.dir/server/thread.cc.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/server/thread.cc.o: ../server/thread.cc
CMakeFiles/server.dir/server/thread.cc.o: CMakeFiles/server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/shuaishuai/project/sylar_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/server.dir/server/thread.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/thread.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/server.dir/server/thread.cc.o -MF CMakeFiles/server.dir/server/thread.cc.o.d -o CMakeFiles/server.dir/server/thread.cc.o -c /home/shuaishuai/project/sylar_server/server/thread.cc

CMakeFiles/server.dir/server/thread.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/server/thread.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/thread.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/shuaishuai/project/sylar_server/server/thread.cc > CMakeFiles/server.dir/server/thread.cc.i

CMakeFiles/server.dir/server/thread.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/server/thread.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/thread.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/shuaishuai/project/sylar_server/server/thread.cc -o CMakeFiles/server.dir/server/thread.cc.s

CMakeFiles/server.dir/server/util.cc.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/server/util.cc.o: ../server/util.cc
CMakeFiles/server.dir/server/util.cc.o: CMakeFiles/server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/shuaishuai/project/sylar_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/server.dir/server/util.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/util.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/server.dir/server/util.cc.o -MF CMakeFiles/server.dir/server/util.cc.o.d -o CMakeFiles/server.dir/server/util.cc.o -c /home/shuaishuai/project/sylar_server/server/util.cc

CMakeFiles/server.dir/server/util.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/server/util.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/util.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/shuaishuai/project/sylar_server/server/util.cc > CMakeFiles/server.dir/server/util.cc.i

CMakeFiles/server.dir/server/util.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/server/util.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"server/util.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/shuaishuai/project/sylar_server/server/util.cc -o CMakeFiles/server.dir/server/util.cc.s

# Object files for target server
server_OBJECTS = \
"CMakeFiles/server.dir/server/config.cc.o" \
"CMakeFiles/server.dir/server/fiber.cc.o" \
"CMakeFiles/server.dir/server/log.cc.o" \
"CMakeFiles/server.dir/server/thread.cc.o" \
"CMakeFiles/server.dir/server/util.cc.o"

# External object files for target server
server_EXTERNAL_OBJECTS =

../lib/libserver.so: CMakeFiles/server.dir/server/config.cc.o
../lib/libserver.so: CMakeFiles/server.dir/server/fiber.cc.o
../lib/libserver.so: CMakeFiles/server.dir/server/log.cc.o
../lib/libserver.so: CMakeFiles/server.dir/server/thread.cc.o
../lib/libserver.so: CMakeFiles/server.dir/server/util.cc.o
../lib/libserver.so: CMakeFiles/server.dir/build.make
../lib/libserver.so: CMakeFiles/server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/shuaishuai/project/sylar_server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX shared library ../lib/libserver.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/server.dir/build: ../lib/libserver.so
.PHONY : CMakeFiles/server.dir/build

CMakeFiles/server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/server.dir/clean

CMakeFiles/server.dir/depend:
	cd /home/shuaishuai/project/sylar_server/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/shuaishuai/project/sylar_server /home/shuaishuai/project/sylar_server /home/shuaishuai/project/sylar_server/build /home/shuaishuai/project/sylar_server/build /home/shuaishuai/project/sylar_server/build/CMakeFiles/server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/server.dir/depend

