cmake_minimum_required(VERSION 3.12.4 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(BBE_ADD_TEST_PROJECTS    ON CACHE BOOL "If set to OFF, the test projects are not added to the build.")
set(BBE_ADD_EXAMPLE_PROJECTS ON CACHE BOOL "If set to OFF, the example projects are not added to the build.")
set(BBE_ADD_AUDIO            ON CACHE BOOL "If set to OFF, all audio support is stripped from the engine.")
set(BBE_ADD_EXPERIMENTAL     OFF CACHE BOOL "If set to ON, Experimental projects are added.")
set(BBE_ADD_CURL             ON CACHE BOOL "If set to OFF, curl is not included.")

set(BBE_RENDER_MODE "OpenGL" CACHE STRING "Sets the render mode. Possible values are Vulkan, OpenGL, Emscripten, NullRenderer")

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

project(BrotBoxEngine CXX)
if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  set(BBE_RENDER_MODE "Emscripten" CACHE STRING "" FORCE)
  set(BBE_ADD_CURL OFF CACHE BOOL "" FORCE)
  set(BBE_ADD_TEST_PROJECTS OFF CACHE BOOL "" FORCE)
  if(BBE_ADD_AUDIO)
    # I honestly have no idea why this is necessary to build.
    set(LIBTYPE STATIC CACHE BOOL "" FORCE)
  endif()
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

macro(add_trivial_project name)
  add_compile_definitions(BBE_APPLICATION_ASSET_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
  add_executable(${name})
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
      if(ext STREQUAL ".jpg")
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
  
  message(STATUS "Adding lcov flags")
  add_cxx_linker_flag("-fprofile-arcs")
  add_cxx_linker_flag("-ftest-coverage")
endif()

add_subdirectory(BrotBoxEngine)

if(BBE_RENDER_MODE STREQUAL "Vulkan")
  find_package(Vulkan REQUIRED)
  target_include_directories(BrotBoxEngine PUBLIC Vulkan::Vulkan)
  target_link_libraries(BrotBoxEngine Vulkan::Vulkan)
  target_compile_definitions(BrotBoxEngine PUBLIC GLFW_INCLUDE_VULKAN)
  target_compile_definitions(BrotBoxEngine PUBLIC BBE_RENDERER_VULKAN)
  target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-master/backends/imgui_impl_vulkan.cpp)
elseif(BBE_RENDER_MODE STREQUAL "OpenGL")
  add_subdirectory(Third-Party/glew-2.1.0/build/cmake)
  target_include_directories(BrotBoxEngine PUBLIC Third-Party/glew-2.1.0/include)
  target_link_libraries(BrotBoxEngine glew_s)
  target_compile_definitions(BrotBoxEngine PUBLIC BBE_RENDERER_OPENGL)
  target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-master/backends/imgui_impl_opengl3.cpp)
  set_target_properties(glew glew_s glewinfo visualinfo PROPERTIES FOLDER Deps)
elseif(BBE_RENDER_MODE STREQUAL "Emscripten")
  add_compile_options(-sUSE_PTHREADS=1)
  add_link_options(-sEXCEPTION_DEBUG=1)
  add_link_options(--profiling-funcs)
  add_link_options(-sUSE_PTHREADS=1)
  add_link_options(-sPTHREAD_POOL_SIZE=20)
  set_target_properties(BrotBoxEngine PROPERTIES LINK_FLAGS "-s USE_GLFW=3 -s USE_WEBGL2=1")
  target_compile_definitions(BrotBoxEngine PUBLIC BBE_RENDERER_OPENGL)
  target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-master/backends/imgui_impl_opengl3.cpp)
elseif(BBE_RENDER_MODE STREQUAL "NullRenderer")
  target_compile_definitions(BrotBoxEngine PUBLIC BBE_RENDERER_NULL)
  target_compile_definitions(BrotBoxEngine PUBLIC GLFW_INCLUDE_NONE)
else()
  message(FATAL_ERROR "Unknown BBE_RENDER_MODE: ${BBE_RENDER_MODE}")
endif()

target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-master/backends/imgui_impl_glfw.cpp)

if(EMSCRIPTEN)
else()
  include_directories(Third-Party/glfw-3.3.2/include)
  if(NOT BBE_RENDER_MODE STREQUAL "NullRenderer")
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(Third-Party/glfw-3.3.2)
    target_link_libraries(BrotBoxEngine glfw ${GLFW_LIBRARIES})
    set_target_properties(glfw uninstall PROPERTIES FOLDER Deps)
  endif()
endif()

add_subdirectory(Third-Party/libsodium-cmake)
target_link_libraries(BrotBoxEngine sodium)
include_directories(Third-Party/libsodium-cmake/libsodium/src/libsodium/include)
set_target_properties(sodium PROPERTIES FOLDER Deps)

target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-master/imgui.cpp)
target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-master/imgui_draw.cpp)
target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-master/imgui_demo.cpp)
target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-master/imgui_widgets.cpp)
target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/imgui-master/imgui_tables.cpp)
target_include_directories(BrotBoxEngine PUBLIC Third-Party/imgui-master)
target_include_directories(BrotBoxEngine PUBLIC Third-Party/imgui-master/backends)

target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/implot-master/implot.cpp)
target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/implot-master/implot_items.cpp)
target_sources(BrotBoxEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/implot-master/implot_demo.cpp)
target_include_directories(BrotBoxEngine PUBLIC Third-Party/implot-master)

set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
add_subdirectory(Third-Party/box2d-master)
include_directories(Third-Party/box2d-master/include)
target_link_libraries(BrotBoxEngine box2d)
set_target_properties(box2d PROPERTIES FOLDER Deps)


if(WIN32)
  target_link_libraries(BrotBoxEngine wsock32 ws2_32)
endif()

if(BBE_ADD_CURL)
  set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
  set(CURL_DISABLE_TESTS ON CACHE BOOL "" FORCE)
  if(WIN32)
    set(CMAKE_USE_SCHANNEL ON CACHE BOOL "" FORCE)
  endif()
  add_subdirectory(Third-Party/curl-8.8.0)
  target_link_libraries(BrotBoxEngine libcurl)
  include_directories(Third-Party/curl-8.8.0/include)
  target_compile_definitions(BrotBoxEngine PUBLIC BBE_ADD_CURL)
  if(WIN32)
    set_target_properties(libcurl_object PROPERTIES FOLDER Deps)
    set_target_properties(libcurl_static PROPERTIES FOLDER Deps)
  endif()
endif()

if(BBE_ADD_AUDIO)
  set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(AL_LIBTYPE_STATIC ON CACHE BOOL "" FORCE)
  set(LIBTYPE STATIC CACHE STRING "" FORCE)
  add_subdirectory("Third-Party/openal-soft-1.23.1")
  target_include_directories(BrotBoxEngine PUBLIC "Third-Party/openal-soft-1.23.1/include")
  if(NOT EMSCRIPTEN)
    target_link_libraries(BrotBoxEngine OpenAL)
  endif()
  set_target_properties(common ex-common OpenAL openal-info PROPERTIES FOLDER Deps)
else()
  add_compile_definitions(BBE_NO_AUDIO)
endif()

target_include_directories(BrotBoxEngine PUBLIC "Third-Party/stb")
target_include_directories(BrotBoxEngine PUBLIC "Third-Party/minimp3")

if(BBE_ADD_TEST_PROJECTS)
  add_subdirectory(Tests)
  set_target_properties(gtest gtest_main PROPERTIES FOLDER Deps)
endif()
if(BBE_ADD_EXAMPLE_PROJECTS)
  add_subdirectory(Examples)
endif()
