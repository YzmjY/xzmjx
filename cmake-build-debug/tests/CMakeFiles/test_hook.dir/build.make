# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.23

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
CMAKE_SOURCE_DIR = /mnt/d/WorkSpace/xzmjx

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/d/WorkSpace/xzmjx/cmake-build-debug

# Include any dependencies generated for this target.
include tests/CMakeFiles/test_hook.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tests/CMakeFiles/test_hook.dir/compiler_depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/test_hook.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/test_hook.dir/flags.make

tests/CMakeFiles/test_hook.dir/test_hook.cpp.o: tests/CMakeFiles/test_hook.dir/flags.make
tests/CMakeFiles/test_hook.dir/test_hook.cpp.o: ../tests/test_hook.cpp
tests/CMakeFiles/test_hook.dir/test_hook.cpp.o: tests/CMakeFiles/test_hook.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/d/WorkSpace/xzmjx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tests/CMakeFiles/test_hook.dir/test_hook.cpp.o"
	cd /mnt/d/WorkSpace/xzmjx/cmake-build-debug/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tests/CMakeFiles/test_hook.dir/test_hook.cpp.o -MF CMakeFiles/test_hook.dir/test_hook.cpp.o.d -o CMakeFiles/test_hook.dir/test_hook.cpp.o -c /mnt/d/WorkSpace/xzmjx/tests/test_hook.cpp

tests/CMakeFiles/test_hook.dir/test_hook.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_hook.dir/test_hook.cpp.i"
	cd /mnt/d/WorkSpace/xzmjx/cmake-build-debug/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/d/WorkSpace/xzmjx/tests/test_hook.cpp > CMakeFiles/test_hook.dir/test_hook.cpp.i

tests/CMakeFiles/test_hook.dir/test_hook.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_hook.dir/test_hook.cpp.s"
	cd /mnt/d/WorkSpace/xzmjx/cmake-build-debug/tests && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/d/WorkSpace/xzmjx/tests/test_hook.cpp -o CMakeFiles/test_hook.dir/test_hook.cpp.s

# Object files for target test_hook
test_hook_OBJECTS = \
"CMakeFiles/test_hook.dir/test_hook.cpp.o"

# External object files for target test_hook
test_hook_EXTERNAL_OBJECTS =

../bin/test/test_hook: tests/CMakeFiles/test_hook.dir/test_hook.cpp.o
../bin/test/test_hook: tests/CMakeFiles/test_hook.dir/build.make
../bin/test/test_hook: ../lib/libxzmjx.a
../bin/test/test_hook: tests/CMakeFiles/test_hook.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/d/WorkSpace/xzmjx/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/test/test_hook"
	cd /mnt/d/WorkSpace/xzmjx/cmake-build-debug/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_hook.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/test_hook.dir/build: ../bin/test/test_hook
.PHONY : tests/CMakeFiles/test_hook.dir/build

tests/CMakeFiles/test_hook.dir/clean:
	cd /mnt/d/WorkSpace/xzmjx/cmake-build-debug/tests && $(CMAKE_COMMAND) -P CMakeFiles/test_hook.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/test_hook.dir/clean

tests/CMakeFiles/test_hook.dir/depend:
	cd /mnt/d/WorkSpace/xzmjx/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/d/WorkSpace/xzmjx /mnt/d/WorkSpace/xzmjx/tests /mnt/d/WorkSpace/xzmjx/cmake-build-debug /mnt/d/WorkSpace/xzmjx/cmake-build-debug/tests /mnt/d/WorkSpace/xzmjx/cmake-build-debug/tests/CMakeFiles/test_hook.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/test_hook.dir/depend

