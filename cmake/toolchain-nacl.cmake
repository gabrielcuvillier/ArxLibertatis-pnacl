set(NACL 1)

set(PLATFORM_NAME	"NaCl")				     
set(PLATFORM_C_COMPILER  "gcc")				      
set(PLATFORM_CXX_COMPILER "g++")			       
set(PLATFORM_TOOLCHAIN	"linux_x86_newlib")	
set(PLATFORM_ROOT "x86_64-nacl")		  
set(PLATFORM_ARCH "x86_64-nacl")			
set(PLATFORM_EXE_SUFFIX ".nexe")				    

set(PLATFORM_PREFIX "$ENV{NACL_SDK_ROOT}/toolchain/${PLATFORM_TOOLCHAIN}")

include_directories($ENV{NACL_SDK_ROOT}/include)
link_directories($ENV{NACL_SDK_ROOT}/lib/newlib_x86_64/Release)

set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "x86_64")
set(CMAKE_FIND_ROOT_PATH 	"${PLATFORM_PREFIX}/${PLATFORM_ROOT}")
set(CMAKE_AR "${PLATFORM_PREFIX}/bin/${PLATFORM_ARCH}-ar")
set(CMAKE_RANLIB "${PLATFORM_PREFIX}/bin/${PLATFORM_ARCH}-ranlib")
set(CMAKE_STRIP "${PLATFORM_PREFIX}/bin/${PLATFORM_ARCH}-strip")
set(CMAKE_C_COMPILER "${PLATFORM_PREFIX}/bin/${PLATFORM_ARCH}-${PLATFORM_C_COMPILER}")
set(CMAKE_CXX_COMPILER "${PLATFORM_PREFIX}/bin/${PLATFORM_ARCH}-${PLATFORM_CXX_COMPILER}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
