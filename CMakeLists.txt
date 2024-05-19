# CMake simple example

## [main]
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
# Almost all CMake files should start with this
# You should always specify a range with the newest
# and oldest tested versions of CMake. This will ensure
# you pick up the best policies.
cmake_minimum_required(VERSION 3.1...3.29)

# This is your project statement. You should always list languages;
# Listing the version is nice here since it sets lots of useful variables
project(
  ModernCMakeExample
  VERSION 1.0
  LANGUAGES C)

# If you set any CMAKE_ variables, that can go here.
# (But usually don't do this, except maybe for C++ standard)

# Find packages go here.

# You should usually split this into folders, but this is a simple example

# This is a "default" library, and will match the *** variable setting.
# Other common choices are STATIC, SHARED, and MODULE
# Including header files here helps IDEs but is not required.
# Output libname matches target name, with the usual extensions on your system
# Link each target with other targets or add options, etc.

if(WIN32)
    include_directories(/opt/homebrew/include deps/sokol deps/stb deps/glfw/include/GLFW deps/sokol/util)
    link_directories(lib/glfw)
elseif(APPLE)
    include_directories(/opt/homebrew/include deps/sokol deps/stb deps/sokol/util)
    link_directories(/opt/homebrew/Cellar/glfw/3.4/lib)
elseif(UNIX)
    message("no linux yet")
endif()

# Adding something we can run - Output name matches target name
file(GLOB src
     "game/*.h"
     "game/*.c"
)
add_executable(main ${src}
        game/game.h
        game/menu.h
        game/gfx.h
        game/button.h)

# Make sure you link your targets with this command. It can also link libraries and
# even flags, so linking a target that does not exist will not give a configure-time error.
if(WIN32)
    target_link_libraries(main glfw3)
elseif(APPLE)
    target_link_libraries(main glfw3 "-framework Cocoa" "-framework OpenGL" "-framework IOKit" "-framework Carbon")
elseif(UNIX)
    message("no linux yet")
endif()
