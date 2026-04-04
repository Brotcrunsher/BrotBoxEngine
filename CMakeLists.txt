cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(BBE_ADD_TEST_PROJECTS    ON CACHE BOOL "If set to OFF, the test projects are not added to the build.")
set(BBE_ADD_EXAMPLE_PROJECTS ON CACHE BOOL "If set to OFF, the example projects are not added to the build.")
set(BBE_ADD_AUDIO            ON CACHE BOOL "If set to OFF, all audio support is stripped from the engine.")
set(BBE_ADD_PYTHON           ON CACHE BOOL "If set to OFF, Python embedding support is stripped from the engine.")
set(BBE_ADD_EXPERIMENTAL     OFF CACHE BOOL "If set to ON, Experimental projects are added.")
set(BBE_ADD_CURL             ON CACHE BOOL "If set to OFF, curl is not included.")
set(BBE_ENABLE_SANITIZERS    OFF CACHE BOOL "If set to ON, supported compiler sanitizers are added to BrotBoxEngine.")
set(BBE_ENABLE_COVERAGE      OFF CACHE BOOL "If set to ON, GNU coverage instrumentation is added to BrotBoxEngine.")
set(BBE_ENABLE_IMGUI_IMPLOT_DEMO_WINDOWS ON CACHE BOOL "If set to OFF, imgui_demo.cpp and implot_demo.cpp are compiled as lightweight stubs.")
set(BBE_IMPLOT_NUMERIC_TYPES "(ImS32)(float)(double)" CACHE STRING "Numeric types explicitly instantiated by implot_items.cpp. Set to an empty string to keep ImPlot's full upstream type list.")

set(BBE_IMPLOT_EFFECTIVE_NUMERIC_TYPES "${BBE_IMPLOT_NUMERIC_TYPES}")
if(BBE_ENABLE_IMGUI_IMPLOT_DEMO_WINDOWS AND BBE_IMPLOT_EFFECTIVE_NUMERIC_TYPES)
  foreach(BBE_IMPLOT_DEMO_NUMERIC_TYPE IN ITEMS ImS8 ImU32)
    if(NOT BBE_IMPLOT_EFFECTIVE_NUMERIC_TYPES MATCHES "\\(${BBE_IMPLOT_DEMO_NUMERIC_TYPE}\\)")
      string(APPEND BBE_IMPLOT_EFFECTIVE_NUMERIC_TYPES "(${BBE_IMPLOT_DEMO_NUMERIC_TYPE})")
    endif()
  endforeach()
endif()

set(BBE_RENDER_MODE "OpenGL" CACHE STRING "Sets the render mode. Possible values are Vulkan, OpenGL, Emscripten, NullRenderer")

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

project(BrotBoxEngine CXX)
enable_testing()
if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  set(BBE_RENDER_MODE "Emscripten" CACHE STRING "" FORCE)
  set(BBE_ADD_CURL OFF CACHE BOOL "" FORCE)
  set(BBE_ADD_TEST_PROJECTS OFF CACHE BOOL "" FORCE)
  set(BBE_ADD_PYTHON OFF CACHE BOOL "" FORCE)
  if(BBE_ADD_AUDIO)
    # I honestly have no idea why this is necessary to build.
    set(LIBTYPE STATIC CACHE BOOL "" FORCE)
  endif()
  add_compile_options(-matomics -mbulk-memory)
  add_link_options(--shared-memory -matomics -mbulk-memory)
endif()

macro(install_compiled_shaders currProject)
  foreach(shader_path ${compiled_shaders})
    get_filename_component(shader ${shader_path} NAME)
    configure_file("${shader_path}" "${CMAKE_BINARY_DIR}/${currProject}/${shader}" COPYONLY)
  endforeach(shader_path)
endmacro()

macro(start_embed_file cpp_file_string h_file_string h_file_name)
  set(${cpp_file_string} "#include \"BBE/ByteBuffer.h\"\n#include \"${h_file_name}\"\n\n")
  set(${h_file_string} "#include \"BBE/ByteBuffer.h\"\n\n")
  set(warningMsg "")
  string(APPEND warningMsg "// **********************************************************************************************\n")
  string(APPEND warningMsg "// ***                                     !!!WARNING!!!                                      ***\n")
  string(APPEND warningMsg "// *** WARNING: AUTO GENERATED FILE! DO NOT MODIFY BY HAND! YOUR CHANGES WILL BE OVERWRITTEN! ***\n")
  string(APPEND warningMsg "// ***                                     !!!WARNING!!!                                      ***\n")
  string(APPEND warningMsg "// **********************************************************************************************\n\n\n\n")
  string(APPEND ${cpp_file_string} ${warningMsg})
  string(APPEND ${h_file_string} ${warningMsg})
endmacro()

macro(contents_as_hex filename)
  file(READ ${filename} contents HEX)
  string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," contents ${contents})
endmacro()

macro(embed_file_to_source filename cpp_file_string h_file_string)
  message(STATUS "Embedding File: ${filename}")
  contents_as_hex(${filename})
  get_filename_component(fn_wo_ext ${filename} NAME_WE)
  string(APPEND ${h_file_string} "extern const bbe::ByteBuffer ${fn_wo_ext}\;\n")
  string(APPEND ${cpp_file_string} "const bbe::ByteBuffer ${fn_wo_ext} = { ${contents} }\;\n")
endmacro()

macro(end_embed_file cpp_file_string h_file_string cpp_file_name h_file_name)
  file(WRITE ${h_file_name} ${${h_file_string}})
  file(WRITE ${cpp_file_name} ${${cpp_file_string}})
endmacro()

macro(disable_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W0)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${target} PRIVATE -w)
  endif()
endmacro()

macro(add_trivial_project name)
  add_compile_definitions(BBE_APPLICATION_ASSET_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
  add_executable(${name})
  if(MSVC)
    target_compile_options(${name} PRIVATE /wd4244 /wd4267) # I don't quite care about "possible loss of data" in trivial projects.
    target_compile_options(${name} PRIVATE /wd4305 /wd4309) # Same for "truncation of contant" warnings.
  endif()
  target_link_libraries(${name} BrotBoxEngine)
  file(GLOB local_src CONFIGURE_DEPENDS "*.cpp")
  target_sources(${name} PUBLIC "${local_src}")
  install_compiled_shaders(${name})
  target_include_directories(${name} PUBLIC .)
  if(EMSCRIPTEN)
    set_target_properties(${name} PROPERTIES LINK_FLAGS "-s USE_GLFW=3 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_PTHREADS -s NO_DISABLE_EXCEPTION_CATCHING=1 -s USE_WEBGL2=1")
    if(BBE_ADD_AUDIO)
        set_target_properties(${name} PROPERTIES LINK_FLAGS "-s USE_GLFW=3 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_PTHREADS -lopenal -s NO_DISABLE_EXCEPTION_CATCHING=1 -s USE_WEBGL2=1")
    endif()
  endif()
  if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Assets")
    file(GLOB files CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/*")
    start_embed_file(cpp_file_string h_file_string AssetStore.h)
    set(delete_calls "")
    set(hot_reload_arms "")
    string(APPEND h_file_string "#include \"BBE/Image.h\"\n")
    string(APPEND h_file_string "#include \"BBE/Sound.h\"\n")
    string(APPEND h_file_string "#include \"BBE/String.h\"\n")
    string(APPEND h_file_string "#include \"BBE/Model.h\"\n")
    string(APPEND h_file_string "#include \"BBE/FragmentShader.h\"\n")
    string(APPEND h_file_string "namespace assetStore {\n")
    string(APPEND cpp_file_string "#include \"BBE/Window.h\"\n")
    string(APPEND cpp_file_string "#include \"BBE/SimpleFile.h\"\n")
    string(APPEND cpp_file_string "// NOTE: Indentation here looks \"special\", but I don't care as long as the indentation within the generator script looks okay :-)\n")
    string(APPEND cpp_file_string 
    "static void cleanUp()\;
    static void frameStart()\;
    static void registerCleanup()
    {
      static bool registered = false\;
      if(registered) return\;
      registered = true\;
      bbe::Window::INTERNAL_firstInstance->registerFrameStartListener(std::function(frameStart))\;
      bbe::Window::INTERNAL_firstInstance->registerCloseListener(std::function(cleanUp))\;
    }
    ")
    foreach(f ${files})
      get_filename_component(ext ${f} EXT)
      get_filename_component(name_we ${f} NAME_WE)
      set(contents "")
      if(ext STREQUAL ".jpg" OR ext STREQUAL ".png")
        string(APPEND h_file_string "extern const bbe::Image* ${name_we}()\;\n")
        string(APPEND cpp_file_string 
          "static bbe::Image* ${name_we}_data = nullptr\;
          static std::filesystem::file_time_type ${name_we}_modify_time\;
          bool ${name_we}_hot_reload_armed = true\;
          const bbe::Image* assetStore::${name_we}() 
          {
            registerCleanup()\;
            bbe::ByteBuffer raw\;
            if(!${name_we}_hot_reload_armed)
            {
              return ${name_we}_data\;
            }
            if(!bbe::simpleFile::readBinaryFileIfChanged(\"${f}\", raw, ${name_we}_modify_time)) 
            {
              ${name_we}_hot_reload_armed = false\;
              return ${name_we}_data\;
            }
            ${name_we}_hot_reload_armed = false\;
            delete ${name_we}_data\;
            ${name_we}_data = new bbe::Image()\;
            ${name_we}_data->loadRaw(raw)\;
            return ${name_we}_data\;
          }\n")
        string(APPEND delete_calls "delete ${name_we}_data\; ${name_we}_data = nullptr\;")
        string(APPEND hot_reload_arms "${name_we}_hot_reload_armed = true\;")
      elseif(ext STREQUAL ".mp3")
        contents_as_hex(${f})
        string(APPEND h_file_string "extern const bbe::Sound* ${name_we}()\;\n")
        string(APPEND cpp_file_string 
          "static bbe::Sound* ${name_we}_data = nullptr\;
          const bbe::Sound* assetStore::${name_we}() 
          {
            registerCleanup()\;
            static bbe::ByteBuffer raw = { ${contents} }\;
            if(${name_we}_data) return ${name_we}_data\;
            ${name_we}_data = new bbe::Sound()\;
            ${name_we}_data->load(raw, bbe::SoundLoadFormat::MP3)\;
            return ${name_we}_data\;
          }\n")
        string(APPEND delete_calls "delete ${name_we}_data\; ${name_we}_data = nullptr\;")
      elseif(ext STREQUAL ".txt")
        contents_as_hex(${f})
        string(APPEND h_file_string "extern const bbe::String* ${name_we}()\;\n")
        string(APPEND cpp_file_string 
          "const bbe::String* assetStore::${name_we}() 
          {
            registerCleanup()\;
            static bbe::String s = { ${contents} 0x00 }\;
            return &s\;
          }\n")
      elseif(ext STREQUAL ".obj")
        string(APPEND h_file_string "extern const bbe::Model* ${name_we}()\;\n")
        #string(APPEND cpp_file_string 
        #  "static bbe::Model* ${name_we}_data = nullptr\;
        #  const bbe::Model* assetStore::${name_we}() 
        #  {
        #    registerCleanup()\;
        #    static bbe::String s = { ${contents} 0x00 }\;
        #    if(${name_we}_data) return ${name_we}_data\;
        #    ${name_we}_data = new bbe::Model()\;
        #    *${name_we}_data = bbe::Model::fromObj(s)\;
        #    return ${name_we}_data\;
        #  }\n")
        string(APPEND cpp_file_string 
          "static bbe::Model* ${name_we}_data = nullptr\;
          const bbe::Model* assetStore::${name_we}() 
          {
            registerCleanup()\;
            if(${name_we}_data) return ${name_we}_data\;
            bbe::String s = bbe::simpleFile::readFile(\"${f}\")\;
            ${name_we}_data = new bbe::Model()\;
            *${name_we}_data = bbe::Model::fromObj(s)\;
            return ${name_we}_data\;
          }\n")
        string(APPEND delete_calls "delete ${name_we}_data\; ${name_we}_data = nullptr\;")
      elseif(ext STREQUAL ".spv" OR ext STREQUAL ".frag")
        contents_as_hex(${f})
        # .spv is ignored when we are not in vulkan
        if((BBE_RENDER_MODE STREQUAL "Vulkan" AND ext STREQUAL ".spv")
          OR ((BBE_RENDER_MODE STREQUAL "OpenGL" OR BBE_RENDER_MODE STREQUAL "Emscripten") AND ext STREQUAL ".frag"))
          if(ext STREQUAL ".frag")
            string(APPEND contents "0x00")
          endif()
          string(APPEND h_file_string "extern bbe::FragmentShader* ${name_we}()\;\n")
		  if(BBE_RENDER_MODE STREQUAL "Emscripten")
            string(APPEND cpp_file_string 
              "static bbe::FragmentShader* ${name_we}_data = nullptr\;
              bbe::FragmentShader* assetStore::${name_we}() 
              {
                registerCleanup()\;
                static bbe::ByteBuffer raw = { ${contents} }\;
                if(${name_we}_data) return ${name_we}_data\;
                ${name_we}_data = new bbe::FragmentShader()\;
                ${name_we}_data->load(raw)\;
                return ${name_we}_data\;
              }\n")
		  else()
            string(APPEND cpp_file_string 
              "static bbe::FragmentShader* ${name_we}_data = nullptr\;
              static std::filesystem::file_time_type ${name_we}_modify_time\;
              bool ${name_we}_hot_reload_armed = true\;
              bbe::FragmentShader* assetStore::${name_we}() 
              {
                registerCleanup()\;
                bbe::ByteBuffer raw\;
                if(!${name_we}_hot_reload_armed)
                {
                  return ${name_we}_data\;
                }
                if(!bbe::simpleFile::readBinaryFileIfChanged(\"${f}\", raw, ${name_we}_modify_time)) 
                {
                  ${name_we}_hot_reload_armed = false\;
                  return ${name_we}_data\;
                }
                ${name_we}_hot_reload_armed = false\;
                delete ${name_we}_data\;
                ${name_we}_data = new bbe::FragmentShader()\;
                ${name_we}_data->load(raw)\;
                return ${name_we}_data\;
              }\n")
			  string(APPEND hot_reload_arms "${name_we}_hot_reload_armed = true\;")
		  endif()
          string(APPEND delete_calls "delete ${name_we}_data\; ${name_we}_data = nullptr\;")
        endif()
      elseif(ext STREQUAL "")
        # Do nothing. Useful for licenses etc.
      else()
        message(WARNING "Found unknown asset extension, ignoring. ${f}") 
      endif()
    endforeach()
    string(APPEND cpp_file_string 
      "static void cleanUp()
      {
      ${delete_calls}
      }")
    string(APPEND cpp_file_string
      "static void frameStart()
      {
      ${hot_reload_arms}
      }")
    string(APPEND h_file_string "}\n")
    end_embed_file(cpp_file_string h_file_string ${CMAKE_CURRENT_BINARY_DIR}/Code/AssetStore.cpp ${CMAKE_CURRENT_BINARY_DIR}/Code/AssetStore.h)
    target_sources(${name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/Code/AssetStore.cpp")
    target_include_directories(${name} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/Code/")
  endif()
endmacro()

add_library(BrotBoxEngine STATIC)
message(STATUS "C++ Version: ${CMAKE_CXX_STANDARD}")
set_property(TARGET BrotBoxEngine PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
set_property(TARGET BrotBoxEngine PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET BrotBoxEngine PROPERTY CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  macro(add_cxx_linker_flag flag)
    target_compile_options(BrotBoxEngine PUBLIC "${flag}")
    target_link_libraries(BrotBoxEngine "${flag}")
  endmacro()

  if(BBE_ENABLE_SANITIZERS)
    message(STATUS "Adding sanitizers")
    add_cxx_linker_flag("-fsanitize=address")
    add_cxx_linker_flag("-fsanitize=pointer-compare")
    add_cxx_linker_flag("-fsanitize=pointer-subtract")
    add_cxx_linker_flag("-fsanitize=undefined")
    add_cxx_linker_flag("-fsanitize=integer-divide-by-zero")
    add_cxx_linker_flag("-fsanitize=unreachable")
    add_cxx_linker_flag("-fsanitize=vla-bound")
    add_cxx_linker_flag("-fsanitize=null")
    add_cxx_linker_flag("-fsanitize=return")
    add_cxx_linker_flag("-fsanitize=signed-integer-overflow")
    add_cxx_linker_flag("-fsanitize=bounds-strict")
    add_cxx_linker_flag("-fsanitize=enum")
    add_cxx_linker_flag("-fsanitize=bool")
    add_cxx_linker_flag("-fsanitize=vptr")
    add_cxx_linker_flag("-fsanitize=pointer-overflow")
  endif()

  if(BBE_ENABLE_COVERAGE)
    message(STATUS "Adding lcov flags")
    add_cxx_linker_flag("-fprofile-arcs")
    add_cxx_linker_flag("-ftest-coverage")
  endif()
endif()

add_subdirectory(BrotBoxEngine)

if(BBE_RENDER_MODE STREQUAL "Vulkan")
  find_package(Vulkan REQUIRED)
  target_link_libraries(BrotBoxEngine Vulkan::Vulkan)
  target_compile_definitions(BrotBoxEngine PUBLIC GLFW_INCLUDE_VULKAN)
  target_compile_definitions(BrotBoxEngine PUBLIC BBE_RENDERER_VULKAN)
  target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-1.92.7-docking/backends/imgui_impl_vulkan.cpp)
elseif(BBE_RENDER_MODE STREQUAL "OpenGL")
  add_subdirectory(Third-Party/glew-2.3.1/build/cmake)
  target_include_directories(BrotBoxEngine SYSTEM PUBLIC Third-Party/glew-2.3.1/include)
  target_link_libraries(BrotBoxEngine glew_s)
  target_compile_definitions(BrotBoxEngine PUBLIC BBE_RENDERER_OPENGL)
  target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-1.92.7-docking/backends/imgui_impl_opengl3.cpp)
  set_target_properties(glew glew_s glewinfo visualinfo PROPERTIES FOLDER Deps)
  if(MSVC)
    target_link_options(glew PRIVATE /BASE:0x140000000) # Fixes LNK4281 warning: undesirable base address 0x62AA0000 for x64 image; set base address above 4GB for best ASLR optimization
  endif()
  disable_warnings(glewinfo)
elseif(BBE_RENDER_MODE STREQUAL "Emscripten")
  add_compile_options(-sUSE_PTHREADS=1 -matomics -mbulk-memory)
  add_link_options(-sEXCEPTION_DEBUG=1 --profiling-funcs -sUSE_PTHREADS=1 -sPTHREAD_POOL_SIZE=20 -matomics -mbulk-memory --shared-memory)
  set_target_properties(BrotBoxEngine PROPERTIES LINK_FLAGS "-s USE_GLFW=3 -s USE_WEBGL2=1")
  target_compile_definitions(BrotBoxEngine PUBLIC BBE_RENDERER_OPENGL)
  target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-1.92.7-docking/backends/imgui_impl_opengl3.cpp)
elseif(BBE_RENDER_MODE STREQUAL "NullRenderer")
  target_compile_definitions(BrotBoxEngine PUBLIC BBE_RENDERER_NULL)
  target_compile_definitions(BrotBoxEngine PUBLIC GLFW_INCLUDE_NONE)
else()
  message(FATAL_ERROR "Unknown BBE_RENDER_MODE: ${BBE_RENDER_MODE}")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(DBUS REQUIRED IMPORTED_TARGET dbus-1)
  target_link_libraries(BrotBoxEngine PkgConfig::DBUS)
  pkg_check_modules(WAYLAND_CLIENT QUIET IMPORTED_TARGET wayland-client)
  if(WAYLAND_CLIENT_FOUND)
    target_link_libraries(BrotBoxEngine PkgConfig::WAYLAND_CLIENT)
    target_compile_definitions(BrotBoxEngine PRIVATE BBE_USE_WAYLAND_CLIPBOARD)
    set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
  endif()
endif()

if(MSVC)
  # C++17 deprecated wstring_convert without providing an alternative. Thanks.
  target_compile_definitions(BrotBoxEngine PUBLIC _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
endif()

target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-1.92.7-docking/backends/imgui_impl_glfw.cpp)

if(EMSCRIPTEN)
else()
  target_include_directories(BrotBoxEngine SYSTEM PUBLIC Third-Party/glfw-3.4/include)
  if(NOT BBE_RENDER_MODE STREQUAL "NullRenderer")
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(Third-Party/glfw-3.4)
    target_link_libraries(BrotBoxEngine glfw ${GLFW_LIBRARIES})
    set_target_properties(glfw uninstall PROPERTIES FOLDER Deps)
  endif()
endif()

add_subdirectory(Third-Party/libsodium-cmake)
disable_warnings(sodium)
target_link_libraries(BrotBoxEngine sodium)
target_include_directories(BrotBoxEngine SYSTEM PUBLIC Third-Party/libsodium-cmake/libsodium/src/libsodium/include)
set_target_properties(sodium PROPERTIES FOLDER Deps)

set(BBE_IMGUI_DEMO_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-1.92.7-docking/imgui_demo.cpp)
target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-1.92.7-docking/imgui.cpp)
target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-1.92.7-docking/imgui_draw.cpp)
target_sources(BrotBoxEngine PRIVATE ${BBE_IMGUI_DEMO_SOURCE})
target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-1.92.7-docking/imgui_widgets.cpp)
target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-1.92.7-docking/imgui_tables.cpp)
target_include_directories(BrotBoxEngine SYSTEM PUBLIC Third-Party/imgui-1.92.7-docking)
target_include_directories(BrotBoxEngine SYSTEM PUBLIC Third-Party/imgui-1.92.7-docking/backends)

set(BBE_IMPLOT_ITEMS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/implot-0.17/implot_items.cpp)
set(BBE_IMPLOT_DEMO_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/implot-0.17/implot_demo.cpp)
target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/implot-0.17/implot.cpp)
target_sources(BrotBoxEngine PRIVATE ${BBE_IMPLOT_ITEMS_SOURCE})
target_sources(BrotBoxEngine PRIVATE ${BBE_IMPLOT_DEMO_SOURCE})
target_include_directories(BrotBoxEngine SYSTEM PUBLIC Third-Party/implot-0.17)

if(NOT BBE_ENABLE_IMGUI_IMPLOT_DEMO_WINDOWS)
  set_source_files_properties(
    ${BBE_IMGUI_DEMO_SOURCE}
    ${BBE_IMPLOT_DEMO_SOURCE}
    PROPERTIES COMPILE_DEFINITIONS IMGUI_DISABLE_DEMO_WINDOWS)
endif()

if(BBE_IMPLOT_EFFECTIVE_NUMERIC_TYPES)
  set_source_files_properties(
    ${BBE_IMPLOT_ITEMS_SOURCE}
    PROPERTIES COMPILE_DEFINITIONS "IMPLOT_CUSTOM_NUMERIC_TYPES=${BBE_IMPLOT_EFFECTIVE_NUMERIC_TYPES}")
endif()

set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
add_subdirectory(Third-Party/box2d-master)
target_include_directories(BrotBoxEngine SYSTEM PUBLIC Third-Party/box2d-master/include)
target_link_libraries(BrotBoxEngine box2d)
set_target_properties(box2d PROPERTIES FOLDER Deps)


if(WIN32)
  target_compile_definitions(BrotBoxEngine PUBLIC NOMINMAX)
  target_link_libraries(BrotBoxEngine wsock32 ws2_32)
  
  target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/engine.natvis)
  set_target_properties(BrotBoxEngine PROPERTIES VS_DEBUGGER_VISUALIZERS "${CMAKE_CURRENT_SOURCE_DIR}/engine.natvis")
endif()

if(BBE_ADD_CURL)
  set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
  set(CURL_DISABLE_TESTS ON CACHE BOOL "" FORCE)
  if(WIN32)
    set(CURL_USE_SCHANNEL ON CACHE BOOL "" FORCE)
  endif()
  add_subdirectory(Third-Party/curl-8.8.0)
  target_link_libraries(BrotBoxEngine libcurl)
  target_include_directories(BrotBoxEngine SYSTEM PUBLIC Third-Party/curl-8.8.0/include)
  target_compile_definitions(BrotBoxEngine PUBLIC BBE_ADD_CURL)
  if(WIN32)
    set_target_properties(libcurl_object PROPERTIES FOLDER Deps)
    #set_target_properties(libcurl_static PROPERTIES FOLDER Deps)
    set_target_properties(curl_uninstall PROPERTIES FOLDER Deps)
  endif()
endif()

set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "Disable tinyxml2 tests in the BrotBoxEngine superbuild." FORCE)
add_subdirectory(Third-Party/tinyxml2-10.0.0)
target_link_libraries(BrotBoxEngine tinyxml2)
set_target_properties(tinyxml2 PROPERTIES FOLDER Deps)
target_include_directories(BrotBoxEngine SYSTEM PUBLIC Third-Party/tinyxml2-10.0.0)

if(BBE_ADD_AUDIO)
  set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(AL_LIBTYPE_STATIC ON CACHE BOOL "" FORCE)
  set(LIBTYPE STATIC CACHE STRING "" FORCE)
  add_subdirectory("Third-Party/openal-soft-1.25.1")
  target_include_directories(BrotBoxEngine SYSTEM PUBLIC "Third-Party/openal-soft-1.25.1/include")
  if(NOT EMSCRIPTEN)
    target_link_libraries(BrotBoxEngine OpenAL)
  endif()
  set_target_properties(OpenAL openal-info PROPERTIES FOLDER Deps)
  disable_warnings(OpenAL)
  #disable_warnings(common)
  #disable_warnings(ex-common)
  disable_warnings(openal-info)
else()
  add_compile_definitions(BBE_NO_AUDIO)
endif()

target_include_directories(BrotBoxEngine SYSTEM PUBLIC "Third-Party/stb")
target_include_directories(BrotBoxEngine SYSTEM PUBLIC "Third-Party/minimp3")
target_include_directories(BrotBoxEngine SYSTEM PUBLIC "Third-Party/nlohmann")

if(BBE_ADD_PYTHON)
  find_package(Python3 COMPONENTS Development REQUIRED)
  target_include_directories(BrotBoxEngine SYSTEM PUBLIC "${Python3_INCLUDE_DIRS}")
  target_link_libraries(BrotBoxEngine "${Python3_LIBRARIES}")
else()
  target_compile_definitions(BrotBoxEngine PUBLIC BBE_NO_PYTHON)
endif()

if(BBE_ADD_TEST_PROJECTS)
  add_subdirectory(Third-Party/googletest-1.17.0)
  set_target_properties(gtest gtest_main PROPERTIES FOLDER Deps)
  add_subdirectory(Tests)
endif()
if(BBE_ADD_EXAMPLE_PROJECTS)
  add_subdirectory(Examples)
endif()
