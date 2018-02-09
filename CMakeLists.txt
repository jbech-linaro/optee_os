project (optee-os-headers)

################################################################################
# Built library
################################################################################
add_library(${PROJECT_NAME} INTERFACE)

################################################################################
# Public and private header and library dependencies
################################################################################
target_include_directories(${PROJECT_NAME}
	INTERFACE lib/libutee/include
	INTERFACE lib/libutils/ext/include
	INTERFACE core/include
	INTERFACE core/include/tee
)

################################################################################
# Install targets
################################################################################
# FIXME: Probably no need to export the header files ...
#install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
#DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
