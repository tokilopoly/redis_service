# CMake generated Testfile for 
# Source directory: E:/download/hiredis-1.1.0/hiredis-1.1.0
# Build directory: E:/download/hiredis-1.1.0/hiredis-1.1.0/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(hiredis-test "E:/download/hiredis-1.1.0/hiredis-1.1.0/test.sh")
  set_tests_properties(hiredis-test PROPERTIES  _BACKTRACE_TRIPLES "E:/download/hiredis-1.1.0/hiredis-1.1.0/CMakeLists.txt;248;ADD_TEST;E:/download/hiredis-1.1.0/hiredis-1.1.0/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(hiredis-test "E:/download/hiredis-1.1.0/hiredis-1.1.0/test.sh")
  set_tests_properties(hiredis-test PROPERTIES  _BACKTRACE_TRIPLES "E:/download/hiredis-1.1.0/hiredis-1.1.0/CMakeLists.txt;248;ADD_TEST;E:/download/hiredis-1.1.0/hiredis-1.1.0/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(hiredis-test "E:/download/hiredis-1.1.0/hiredis-1.1.0/test.sh")
  set_tests_properties(hiredis-test PROPERTIES  _BACKTRACE_TRIPLES "E:/download/hiredis-1.1.0/hiredis-1.1.0/CMakeLists.txt;248;ADD_TEST;E:/download/hiredis-1.1.0/hiredis-1.1.0/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(hiredis-test "E:/download/hiredis-1.1.0/hiredis-1.1.0/test.sh")
  set_tests_properties(hiredis-test PROPERTIES  _BACKTRACE_TRIPLES "E:/download/hiredis-1.1.0/hiredis-1.1.0/CMakeLists.txt;248;ADD_TEST;E:/download/hiredis-1.1.0/hiredis-1.1.0/CMakeLists.txt;0;")
else()
  add_test(hiredis-test NOT_AVAILABLE)
endif()
