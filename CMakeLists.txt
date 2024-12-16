cmake_minimum_required(VERSION 3.22.2)
project(tg_bot)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/tg_bot_lib)
#SET(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
#设置可执行文件输出
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
##设置库文件输出
#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/lib)
#设置RPATH
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH $ORIGIN:$ORIGIN/lib)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")# for Linux
    set(LIB_HOME_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/lib/debian)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")    # for Windows
    set(LIB_HOME_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/lib/win)
elseif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    set(LIB_HOME_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/lib/freebsd)
endif()

if ( CMAKE_BUILD_TYPE AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(LIB_HOME_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/lib/debug)
endif ()


message(${LIB_HOME_PREFIX})

set(Td_DIR  ${LIB_HOME_PREFIX}/tdlib/lib/cmake/Td)
find_package(Td  REQUIRED)

link_directories(${LIB_HOME_PREFIX}/cryptlib/)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")          # for Linux
    set(hiredis_DIR  ${LIB_HOME_PREFIX}/hiredis/lib/cmake/hiredis)
    find_package(hiredis REQUIRED)
    set(CRYPT_NAME "libcryptopp")
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")    # for Windows
    set(hiredis_DIR  ${LIB_HOME_PREFIX}/hiredis/share/hiredis)
    find_package(hiredis REQUIRED)
    set(CRYPT_NAME "cryptopp")
elseif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    set(hiredis_DIR  ${LIB_HOME_PREFIX}/hiredis/lib/cmake/hiredis)
    find_package(hiredis REQUIRED)
    set(CRYPT_NAME "libcryptopp")
endif()



set(redis++_DIR ${LIB_HOME_PREFIX}/redis++/share/cmake/redis++ )
find_package(redis++ REQUIRED)

set(ZXing_DIR ${LIB_HOME_PREFIX}/zxing/lib/cmake/ZXing)
find_package(ZXing REQUIRED)


file(GLOB_RECURSE src
        src/*.cpp
        src/include/*.h
        src/handler/include/*.h
        src/handler/*.cpp
        src/redis/include/*.h
        src/redis/*.cpp
        src/orc/include/*.h
        src/orc/*.cpp
        src/plugins/command/include/*.h
        src/plugins/command/*.cpp
        src/plugins/include/*.h
        src/plugins/*.cpp
        main.cpp
)



add_executable(tg_bot
      ${src}
)

message(${hiredis_INCLUDE_DIRS})

target_link_libraries(tg_bot PRIVATE hiredis::hiredis redis++::redis++ ZXing::Core Td::TdStatic )
#set_target_properties(tg_bot PROPERTIES LINK_FLAGS "-Wl,-rpath,./")
#set_target_properties(tg_bot PROPERTIES INSTALL_RPATH ".")
#target_include_directories(tg_bot PRIVATE ${hiredis_INCLUDE_DIRS})
#set_property(TARGET tg_bot PROPERTY CXX_STANDARD 14)


#install(TARGETS tg_bot DESTINATION install/bin)
#install(FILES ${LIB_HOME_PREFIX}/hiredis/lib/hiredisd.lib DESTINATION install/bin)