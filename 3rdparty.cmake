include(ExternalProject)

set(PWD_DIR ${PROJECT_BINARY_DIR})
set(INC_DIR ${PWD_DIR}/include)
set(LIB_DIR ${PWD_DIR}/lib)
# set(MANUAL_BUDIL false)

# if (NOT EXISTS "${INC_DIR}")
# add_custom_target(
#     create-3rdparty-include-dir ALL
#     COMMAND ${CMAKE_COMMAND -E make_directory ${INC_DIR}}
# )
# endif

# if (NOT EXISTS "%{LIB_DIR}")
# add_custom_target(
#     COMMAND ${CMAKE_COMMAND} -E make_directory $(LIB_DIR)
# )
# endif()

execute_process(COMMAND mkdir ./lib ./include -p
    WORKING_DIRECTORY ${PWD_DIR}
)

ExternalProject_Add(
    st
    EXCLUDE_FROM_ALL true
    URL https://github.com/lam2003/thirdparty/raw/master/st-1.9.tar.gz
    DOWNLOAD_NAME st-1.9.tar.gz
    SOURCE_DIR st-1.9
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE 1
    BUILD_COMMAND make linux-optimized
    INSTALL_COMMAND cp ${PWD_DIR}/st-1.9/obj/st.h -f ${INC_DIR} && cp ${PWD_DIR}/st-1.9/obj/libst.a ${LIB_DIR} -f

)
