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
CMAKE_SOURCE_DIR = /mnt/e/WorkSpace/xzmjx

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/e/WorkSpace/xzmjx/cmake-build-debug

# Include any dependencies generated for this target.
include tests/CMakeFiles/test_yaml.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tests/CMakeFiles/test_yaml.dir/compiler_depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/test_yaml.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/test_yaml.dir/flags.make

tests/CMakeFiles/test_yaml.dir/test_yaml.cpp.o: tests/CMakeFiles/test_yaml.dir/flags.make
tests/CMakeFiles/test_yaml.dir/test_yaml.cpp.o: ../tests/test_yaml.cpp
tests/CMakeFiles/test_yaml.dir/test_yaml.cpp.o: tests/CMakeFiles/test_yaml.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/e/WorkSpace/xzmjx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tests/CMakeFiles/test_yaml.dir/test_yaml.cpp.o"
	cd /mnt/e/WorkSpace/xzmjx/cmake-build-debug/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tests/CMakeFiles/test_yaml.dir/test_yaml.cpp.o -MF CMakeFiles/test_yaml.dir/test_yaml.cpp.o.d -o CMakeFiles/test_yaml.dir/test_yaml.cpp.o -c /mnt/e/WorkSpace/xzmjx/tests/test_yaml.cpp

tests/CMakeFiles/test_yaml.dir/test_yaml.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_yaml.dir/test_yaml.cpp.i"
	cd /mnt/e/WorkSpace/xzmjx/cmake-build-debug/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/e/WorkSpace/xzmjx/tests/test_yaml.cpp > CMakeFiles/test_yaml.dir/test_yaml.cpp.i

tests/CMakeFiles/test_yaml.dir/test_yaml.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_yaml.dir/test_yaml.cpp.s"
	cd /mnt/e/WorkSpace/xzmjx/cmake-build-debug/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/e/WorkSpace/xzmjx/tests/test_yaml.cpp -o CMakeFiles/test_yaml.dir/test_yaml.cpp.s

# Object files for target test_yaml
test_yaml_OBJECTS = \
"CMakeFiles/test_yaml.dir/test_yaml.cpp.o"

# External object files for target test_yaml
test_yaml_EXTERNAL_OBJECTS =

../bin/test_yaml: tests/CMakeFiles/test_yaml.dir/test_yaml.cpp.o
../bin/test_yaml: tests/CMakeFiles/test_yaml.dir/build.make
../bin/test_yaml: ../lib/libxzmjx.a
../bin/test_yaml: tests/CMakeFiles/test_yaml.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/e/WorkSpace/xzmjx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/test_yaml"
	cd /mnt/e/WorkSpace/xzmjx/cmake-build-debug/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_yaml.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/test_yaml.dir/build: ../bin/test_yaml
.PHONY : tests/CMakeFiles/test_yaml.dir/build

tests/CMakeFiles/test_yaml.dir/clean:
	cd /mnt/e/WorkSpace/xzmjx/cmake-build-debug/tests && $(CMAKE_COMMAND) -P CMakeFiles/test_yaml.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/test_yaml.dir/clean

tests/CMakeFiles/test_yaml.dir/depend:
	cd /mnt/e/WorkSpace/xzmjx/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/e/WorkSpace/xzmjx /mnt/e/WorkSpace/xzmjx/tests /mnt/e/WorkSpace/xzmjx/cmake-build-debug /mnt/e/WorkSpace/xzmjx/cmake-build-debug/tests /mnt/e/WorkSpace/xzmjx/cmake-build-debug/tests/CMakeFiles/test_yaml.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/test_yaml.dir/depend

