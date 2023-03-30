cmake_minimum_required(VERSION 3.1)
project(gstreamer)

set( CMAKE_CXX_STANDARD 11)

set( GST_DIR /usr/local/Cellar/gstreamer/1.18.3)   
set( GLIB_DIR /usr/local/Cellar/glib/2.66.7)  
 

find_package(PkgConfig)  
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST_VIDEO REQUIRED gstreamer-video-1.0)  
pkg_check_modules(GST_CTRL REQUIRED gstreamer-controller-1.0)  


message("GST_HEADERS: "${GST_INCLUDE_DIRS})
message("GST_CFLAGS: "${GST_CFLAGS})
message("GST_LINK_LIBRARIES: "${GST_LINK_LIBRARIES}) 

file(GLOB SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*c)
add_executable(gstreamer ${SOURCE_FILE} )# GStreamer
target_include_directories(gstreamer PUBLIC ${GST_INCLUDE_DIRS} ${GST_APP_INCLUDE_DIRS} ${GST_CTRL_INCLUDE_DIRS})
target_compile_options(gstreamer PUBLIC ${GST_CFLAGS} ${GST_APP_CFLAGS} ${GST_CTRL_CFLAGS} )
target_link_libraries(gstreamer ${GST_LINK_LIBRARIES} ${GST_APP_LINK_LIBRARIES} ${GST_CTRL_LINK_LIBRARIES})