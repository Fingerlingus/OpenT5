cmake_minimum_required(VERSION 3.16)
project(OpenT5)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

if(WIN32)
    add_executable(opent5_win32 WIN32 platform/os/win/win_main.c 
                          platform/os/win/win_locale.c
                          sys/sys_critsect.c
                          Com/com.c    Com/com_main.c     Dvar/dvar.c FS/fs.c            
                          I/i_string.c NET/net.c          PMem/pmem.c SEH/seh.c
                          Sys/sys.c    common_c.c         platform/arch/x86/x86.c 
                          CL/cl.c)

    target_link_libraries(opent5_win32 Winmm.dll d3d9.dll)

    if(DEFINED VULKAN_PATH) 
        target_include_directories(opent5_win32 PRIVATE ${VULKAN_PATH})
    endif()
else()
    add_executable(opent5_linux32 Sys/sys_critsect.cpp
                                  Com/com.c    Com/com_main.c     Dvar/dvar.c FS/fs.c            
                                  I/i_string.c NET/net.c          PMem/pmem.c SEH/seh.c
                                  Sys/sys.c    common_c.c         platform/arch/x86/x86.c
                                  CL/cl.c)
    if(DEFINED BUILD_32BIT)
        target_compile_options(opent5_linux32 PRIVATE "-m32")
        target_link_options   (opent5_linux32 PRIVATE "-m32")  
    endif()

    target_include_directories(opent5_linux32 PRIVATE /usr/include /usr/include/c++/12.1.0)
endif()