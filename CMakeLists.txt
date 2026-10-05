cmake_minimum_required(VERSION 3.16)

execute_process(
  COMMAND git describe --tags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE ABLS_AGENT_GPIOD_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

if(NOT ABLS_AGENT_GPIOD_VERSION)
  set(ABLS_AGENT_GPIOD_VERSION "unknown")
endif()

if(ABLS_AGENT_GPIOD_VERSION MATCHES "^v?([0-9]+\\.[0-9]+)")
  set(ABLS_AGENT_GPIOD_VERSION_NUMBER "${CMAKE_MATCH_1}")
else()
  set(ABLS_AGENT_GPIOD_VERSION_NUMBER "0.0")
endif()

project(abls-agent-gpiod
  VERSION ${ABLS_AGENT_GPIOD_VERSION_NUMBER}
  DESCRIPTION "Abls Habitat GPIOD runtime"
  HOMEPAGE_URL "https://docs.abls-habitat.fr"
  LANGUAGES C
)

include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)
pkg_check_modules(LIBGPIOD REQUIRED libgpiod>=2.0)
pkg_check_modules(ABLS_LIBS REQUIRED abls-libs)
pkg_check_modules(ABLS_AGENT_LIBS REQUIRED abls-agent-libs)

add_executable(abls-agent-gpiod
  src/gpiod.c
)

target_include_directories(abls-agent-gpiod PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${GLIB_INCLUDE_DIRS}
  ${JSON_GLIB_INCLUDE_DIRS}
  ${LIBGPIOD_INCLUDE_DIRS}
  ${ABLS_LIBS_INCLUDE_DIRS}
  ${ABLS_AGENT_LIBS_INCLUDE_DIRS}
)

target_compile_options(abls-agent-gpiod PRIVATE
  -Wall -Wextra -Wno-unused-parameter -Wno-unused-result -pipe -g -ggdb
)

target_compile_definitions(abls-agent-gpiod PRIVATE
  ABLS_AGENT_GPIOD_VERSION="${ABLS_AGENT_GPIOD_VERSION}"
)

target_link_libraries(abls-agent-gpiod PRIVATE
  ${GLIB_LIBRARIES}
  ${JSON_GLIB_LIBRARIES}
  ${LIBGPIOD_LIBRARIES}
  ${ABLS_LIBS_EXACT}
  ${ABLS_AGENT_LIBS_LIBRARIES}
)

install(TARGETS abls-agent-gpiod
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
)

execute_process(
  COMMAND pkg-config --variable=systemdsystemunitdir systemd
  OUTPUT_VARIABLE ABLS_SYSTEMD_UNIT_INSTALL_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

if(NOT ABLS_SYSTEMD_UNIT_INSTALL_DIR)
  set(ABLS_SYSTEMD_UNIT_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/systemd/system")
endif()

install(FILES packaging/systemd/abls-agent-gpiod@.service
  DESTINATION ${ABLS_SYSTEMD_UNIT_INSTALL_DIR}
  COMPONENT runtime
)

set(CPACK_PACKAGE_NAME "abls-agent-gpiod")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_RELEASE "1")
set(CPACK_PACKAGE_VENDOR "Abls-Habitat")
set(CPACK_PACKAGE_CONTACT "sebastien.lefevre@abls-habitat.fr")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_LICENSE "GPLv3")

set(CPACK_GENERATOR "RPM;DEB")
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL runtime)

set(CPACK_RPM_RUNTIME_PACKAGE_NAME "abls-agent-gpiod")
set(CPACK_RPM_RUNTIME_PACKAGE_GROUP "abls")
set(CPACK_RPM_RUNTIME_PACKAGE_SUMMARY "Abls-Habitat GPIOD runtime")
set(CPACK_RPM_RUNTIME_PACKAGE_REQUIRES "glib2, json-glib, libgpiod, abls-libs, abls-agent-libs")
set(CPACK_RPM_RUNTIME_POST_INSTALL_SCRIPT_FILE "${CMAKE_SOURCE_DIR}/packaging/rpm/runtime.postinst")
set(CPACK_RPM_RUNTIME_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_SOURCE_DIR}/packaging/rpm/runtime.preun")
set(CPACK_RPM_RUNTIME_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_SOURCE_DIR}/packaging/rpm/runtime.postun")
set(CPACK_RPM_RUNTIME_FILE_NAME "abls-agent-gpiod-${PROJECT_VERSION}-${CPACK_PACKAGE_RELEASE}.rpm")
set(CPACK_RPM_RUNTIME_DEBUGINFO_PACKAGE ON)
set(CPACK_RPM_RUNTIME_DEBUGINFO_FILE_NAME "RPM-DEFAULT")
set(CPACK_RPM_RUNTIME_BUILD_SOURCE_DIRS_PREFIX "/usr/src/abls/gpiod")

set(ABLS_DEBIAN_GLIB_RUNTIME_DEPEND "libglib2.0-0t64 | libglib2.0-0")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_NAME "abls-agent-gpiod")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_DEPENDS "${ABLS_DEBIAN_GLIB_RUNTIME_DEPEND}, libjson-glib-1.0-0, libgpiod3, abls-libs, abls-agent-libs")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_RUNTIME_PACKAGE_CONTROL_EXTRA
  "${CMAKE_SOURCE_DIR}/packaging/deb/postinst"
  "${CMAKE_SOURCE_DIR}/packaging/deb/prerm"
  "${CMAKE_SOURCE_DIR}/packaging/deb/postrm"
)

include(CPack)
