# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/star/kv-cluster

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/star/kv-cluster/build

# Include any dependencies generated for this target.
include tests/CMakeFiles/test_fiber.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tests/CMakeFiles/test_fiber.dir/compiler_depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/test_fiber.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/test_fiber.dir/flags.make

tests/CMakeFiles/test_fiber.dir/test_fiber.cpp.o: tests/CMakeFiles/test_fiber.dir/flags.make
tests/CMakeFiles/test_fiber.dir/test_fiber.cpp.o: /home/star/kv-cluster/tests/test_fiber.cpp
tests/CMakeFiles/test_fiber.dir/test_fiber.cpp.o: tests/CMakeFiles/test_fiber.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/star/kv-cluster/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tests/CMakeFiles/test_fiber.dir/test_fiber.cpp.o"
	cd /home/star/kv-cluster/build/tests && /usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"tests/test_fiber.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tests/CMakeFiles/test_fiber.dir/test_fiber.cpp.o -MF CMakeFiles/test_fiber.dir/test_fiber.cpp.o.d -o CMakeFiles/test_fiber.dir/test_fiber.cpp.o -c /home/star/kv-cluster/tests/test_fiber.cpp

tests/CMakeFiles/test_fiber.dir/test_fiber.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_fiber.dir/test_fiber.cpp.i"
	cd /home/star/kv-cluster/build/tests && /usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"tests/test_fiber.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/star/kv-cluster/tests/test_fiber.cpp > CMakeFiles/test_fiber.dir/test_fiber.cpp.i

tests/CMakeFiles/test_fiber.dir/test_fiber.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_fiber.dir/test_fiber.cpp.s"
	cd /home/star/kv-cluster/build/tests && /usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"tests/test_fiber.cpp\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/star/kv-cluster/tests/test_fiber.cpp -o CMakeFiles/test_fiber.dir/test_fiber.cpp.s

# Object files for target test_fiber
test_fiber_OBJECTS = \
"CMakeFiles/test_fiber.dir/test_fiber.cpp.o"

# External object files for target test_fiber
test_fiber_EXTERNAL_OBJECTS =

/home/star/kv-cluster/bin/test/test_fiber: tests/CMakeFiles/test_fiber.dir/test_fiber.cpp.o
/home/star/kv-cluster/bin/test/test_fiber: tests/CMakeFiles/test_fiber.dir/build.make
/home/star/kv-cluster/bin/test/test_fiber: /home/star/kv-cluster/lib/libstar.a
/home/star/kv-cluster/bin/test/test_fiber: tests/CMakeFiles/test_fiber.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/star/kv-cluster/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable /home/star/kv-cluster/bin/test/test_fiber"
	cd /home/star/kv-cluster/build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_fiber.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/test_fiber.dir/build: /home/star/kv-cluster/bin/test/test_fiber
.PHONY : tests/CMakeFiles/test_fiber.dir/build

tests/CMakeFiles/test_fiber.dir/clean:
	cd /home/star/kv-cluster/build/tests && $(CMAKE_COMMAND) -P CMakeFiles/test_fiber.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/test_fiber.dir/clean

tests/CMakeFiles/test_fiber.dir/depend:
	cd /home/star/kv-cluster/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/star/kv-cluster /home/star/kv-cluster/tests /home/star/kv-cluster/build /home/star/kv-cluster/build/tests /home/star/kv-cluster/build/tests/CMakeFiles/test_fiber.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/test_fiber.dir/depend

