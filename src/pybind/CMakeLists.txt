include(Distutils)

set(CYTHON_MODULE_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cython_modules)

# Keep the default version last
if(WITH_PYTHON3)
  set(py_vers 3)
endif()
list(APPEND py_vers 2)

foreach(python_version ${py_vers})
  if(${python_version} EQUAL 2)
    set(PYTHON_VERSION "")
  else(${python_version} EQUAL 2)
    set(PYTHON_VERSION ${python_version})
  endif(${python_version} EQUAL 2)

  find_package(Cython REQUIRED)

  add_subdirectory(rados rados${PYTHON_VERSION})
  add_custom_target(cython${PYTHON_VERSION}_modules ALL
    DEPENDS cython${PYTHON_VERSION}_rados)

  if(WITH_RBD)
    add_subdirectory(rbd rbd${PYTHON_VERSION})
    add_dependencies(cython${PYTHON_VERSION}_modules cython${PYTHON_VERSION}_rbd)
  endif()
  if(WITH_CEPHFS)
    add_subdirectory(cephfs cephfs${PYTHON_VERSION})
    add_dependencies(cython${PYTHON_VERSION}_modules cython${PYTHON_VERSION}_cephfs)
  endif()

  # if CMAKE_INSTALL_PREFIX is an empty string, must replace
  # it with "/" to make PYTHON_INSTALL_TEMPLATE an absolute path to be
  # consistent with all other installation paths.
  if(CMAKE_INSTALL_PREFIX)
    set(PYTHON_INSTALL_TEMPLATE "${CMAKE_INSTALL_PREFIX}")
  else(CMAKE_INSTALL_PREFIX)
    set(PYTHON_INSTALL_TEMPLATE "/")
  endif(CMAKE_INSTALL_PREFIX)

  execute_process(
    COMMAND
    ${PYTHON${PYTHON_VERSION}_EXECUTABLE} -c "from distutils import sysconfig; print(sysconfig.get_python_lib(prefix='${PYTHON_INSTALL_TEMPLATE}'))"
    OUTPUT_VARIABLE "PYTHON${PYTHON_VERSION}_INSTDIR"
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ceph_argparse.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ceph_daemon.py
    ${CMAKE_CURRENT_SOURCE_DIR}/ceph_volume_client.py
    DESTINATION ${PYTHON${PYTHON_VERSION}_INSTDIR})
endforeach()

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/ceph_rest_api.py
  DESTINATION ${PYTHON_INSTDIR})
