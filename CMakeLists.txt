PROJECT(WIP CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

IF (WIN32)
    option(USE_D3D11  "Use directx11 otherwise GL(glfw)" OFF)
    set(GLFW_WIN32 1)
    IF(USE_D3D11)
        ADD_DEFINITIONS(-DUSE_D3D)
        set(GLFW_WIN32 0)
        list( APPEND RHI_INC src/D3D11)
        list( APPEND RHI_SRC src/D3D11/D3D11RHI.cpp)
    ELSE()
        list( APPEND RHI_INC src/OpenGL)
        list( APPEND RHI_SRC src/OpenGL/OpenGLRHI.cpp)
    ENDIF(USE_D3D11)
    MESSAGE(STATUS "Now is windows")
ELSEIF (APPLE)  
    MESSAGE(STATUS "Not surpport Apple systems.")  
ELSEIF (UNIX)
    set(GLFW_X11 1)
    list( APPEND RHI_INC src/OpenGL)
    list( APPEND RHI_SRC src/OpenGL/OpenGLRHI.cpp)
    MESSAGE(STATUS "Now is Linux")
ENDIF ()  






set(CMAKE_BUILD_TYPE Debug CACHE STRING "set build type to debug")

#head file path
INCLUDE_DIRECTORIES(
	src/RBMath/Inc
	src/RBMath/Inc/Platform
	src
	src/thirdpart
	src/Platform
	src/flyUI
	libs/glfw/include/GLFW
	src/thirdpart/glad/include/glad
	src/thirdpart/glad/include/KHR
	src/thirdpart/Box2D
	libs/FreeImage/include
	libs/freetype
	libs/freetype/include
	libs/fmod_studio/inc
	libs/fmod_lowlevel/inc
	lib/fmod_fsbank/inc
	src/thirdpart/imgui
    ${RHI_INC}
)


AUX_SOURCE_DIRECTORY(src DIR_SRCS)
AUX_SOURCE_DIRECTORY(src/RBMath/Src DIR_SRCS)
AUX_SOURCE_DIRECTORY(src/thirdpart DIR_SRCS)
AUX_SOURCE_DIRECTORY(src/Platform DIR_SRCS)

AUX_SOURCE_DIRECTORY(src/thirdpart/Box2D/Rope DIR_SRCS_BOX2D)
AUX_SOURCE_DIRECTORY(src/thirdpart/Box2D/Dynamics DIR_SRCS_BOX2D)
AUX_SOURCE_DIRECTORY(src/thirdpart/Box2D/Dynamics/Joints DIR_SRCS_BOX2D)
AUX_SOURCE_DIRECTORY(src/thirdpart/Box2D/Dynamics/Contacts DIR_SRCS_BOX2D)
AUX_SOURCE_DIRECTORY(src/thirdpart/Box2D/Common DIR_SRCS_BOX2D)
AUX_SOURCE_DIRECTORY(src/thirdpart/Box2D/Collision DIR_SRCS_BOX2D)
AUX_SOURCE_DIRECTORY(src/thirdpart/Box2D/Collision/Shapes DIR_SRCS_BOX2D)

AUX_SOURCE_DIRECTORY(src/thirdpart/glad/src DIR_SRCS_LIB_GLAD)

AUX_SOURCE_DIRECTORY(src/flyUI DIR_UI)

AUX_SOURCE_DIRECTORY(src/thirdpart/imgui DIR_IMGUI)

list( APPEND DIR_SRCS ${DIR_SRCS_BOX2D})
list( APPEND DIR_SRCS ${DIR_SRCS_LIB_GLAD})
list( APPEND DIR_SRCS ${RHI_SRC})
list( APPEND DIR_SRCS ${DIR_UI})
list( APPEND DIR_SRCS ${DIR_IMGUI})
MESSAGE(STATUS "${DIR_SRCS}")

IF (WIN32)

set(GLFW3_LIBRARY ${PROJECT_SOURCE_DIR}/libs/glfw/lib/win32/glfw3.lib)
set(FREEIMAGE_LIBRARY 
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/win32/freeimage.lib
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/win32/jpeg.lib
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/win32/mng.lib
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/win32/openexr.lib
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/win32/openjpeg.lib
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/win32/png.lib
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/win32/rawlite.lib
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/win32/tiff.lib
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/win32/zlib.lib
)
set(LUA_LIBRARY ${PROJECT_SOURCE_DIR}/libs/lua523/lib/win32/lua.lib)

set(FMOD_LIBRARY 
${PROJECT_SOURCE_DIR}/libs/fmod_fsbank/lib/fsbank_vc.lib
${PROJECT_SOURCE_DIR}/libs/fmod_lowlevel/lib/fmod_vc.lib
${PROJECT_SOURCE_DIR}/libs/fmod_studio/lib/fmodstudio_vc.lib
)

set(FREETYPE_LIBRARY ${PROJECT_SOURCE_DIR}/libs/freetype/lib/freetype.lib)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /machine:x86" )
set(CMAKE_VS_PLATFORM_TOOLSET "v120" CACHE STRING "Platform Toolset" FORCE)
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -DNOMINMAX -DVC_EXTRALEAN)


IF(MSVC)
	# Warning C4065: switch statement contains 'default' but no 'case' labels
	# Warning C4244: 'argument': conversion from 'type1' to 'type2', possible loss of data
	# Warning C4800: 'type': forcing value to bool 'true' or 'false' (performance warning)
	# /GR-: Disables run-time type information (RTTI).
	# /INCREMENTAL:NO: Disables incremental linking.
	# /MT: Creates a multithreaded executable file using LIBCMT.lib.
	# /MTd: Creates a debug multithreaded executable file using LIBCMTD.lib.
	# /Oi: Generates intrinsic functions.
	# /W0: Disables all compiler warnings.
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W0 /Oi")
	set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MT")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MTd")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4065 /wd4244 /wd4800 /GR- /Oi")
	set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	string(REGEX REPLACE "/D NDEBUG" "" MSVC_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${MSVC_CXX_FLAGS_RELWITHDEBINFO} /MTd")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:msvcrt.lib")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO /NODEFAULTLIB:libcmt.lib")
	set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /INCREMENTAL:NO /NODEFAULTLIB:libcmt.lib")
	
	
	FILE(GLOB  HEADERS "src/*.h")
	SOURCE_GROUP("inc" FILES ${HEADERS})
	SOURCE_GROUP("box2d" FILES ${DIR_SRCS_BOX2D})
	SOURCE_GROUP("ui" FILES ${DIR_UI})
	SOURCE_GROUP("glad" FILES ${DIR_SRCS_LIB_GLAD})
ENDIF()


ELSEIF(UNIX)

set(GLFW3_LIBRARY ${PROJECT_SOURCE_DIR}/libs/glfw/lib/linux/libglfw3.a)
set(FREEIMAGE_LIBRARY 
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/libfreeimage.a 
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/libjpeg.a 
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/libmng.a 
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/libopenexr.a 
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/libopenjpeg.a 
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/libpng.a 
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/librawlite.a 
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/libtiff.a 
${PROJECT_SOURCE_DIR}/libs/FreeImage/lib/libzlib.a 
)
set(LUA_LIBRARY ${PROJECT_SOURCE_DIR}/libs/lua523/lib/liblua.a)
add_compile_options(-std=c++11)
ENDIF()

list( APPEND DEBUG_LIBS ${GLFW3_LIBRARY})
list( APPEND DEBUG_LIBS ${FREEIMAGE_LIBRARY})
list( APPEND DEBUG_LIBS ${LUA_LIBRARY})
list( APPEND DEBUG_LIBS ${FREETYPE_LIBRARY})
list( APPEND DEBUG_LIBS ${FMOD_LIBRARY})

find_package (Threads REQUIRED)

find_package( OpenGL REQUIRED )
list( APPEND DEBUG_LIBS ${OPENGL_gl_LIBRARY} )


IF(GLFW_X11)
find_package(X11 REQUIRED)
list(APPEND DEBUG_LIBS 
${X11_X11_LIB} 
${CMAKE_THREAD_LIBS_INIT}
${X11_Xrandr_LIB}
${X11_Xinerama_LIB}
${X11_Xcursor_LIB}
)
ENDIF()



SET(CMAKE_C_COMPILER gcc)
SET(CMAK_CXX_COMPILER g++)


SET(DST ${DIR_SRCS})


message(STATUS  "linking:${DEBUG_LIBS}")


#add link library
#TARGET_LINK_LIBRARIES(DST rbmath)

SET(APP_NAME wip_engine)



SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

ADD_EXECUTABLE(${APP_NAME} ${DST} ${} ${DIR_SRCS_LIB_GLAD})



IF(UNIX)
TARGET_LINK_LIBRARIES( ${APP_NAME} ${DEBUG_LIBS} dl)
ELSEIF(WIN32)
TARGET_LINK_LIBRARIES( ${APP_NAME} ${DEBUG_LIBS})
ENDIF()
