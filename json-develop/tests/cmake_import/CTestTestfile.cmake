# CMake generated Testfile for 
# Source directory: /home/ubuntu/Desktop/Audio_comm_github/Visual_feedback_for_audio_communication/json-develop/tests/cmake_import
# Build directory: /home/ubuntu/Desktop/Audio_comm_github/Visual_feedback_for_audio_communication/json-develop/tests/cmake_import
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(cmake_import_configure "/usr/bin/cmake" "-G" "Unix Makefiles" "-A" "" "-DCMAKE_CXX_COMPILER=/usr/bin/c++" "-Dnlohmann_json_DIR=/home/ubuntu/Desktop/Audio_comm_github/Visual_feedback_for_audio_communication/json-develop" "/home/ubuntu/Desktop/Audio_comm_github/Visual_feedback_for_audio_communication/json-develop/tests/cmake_import/project")
set_tests_properties(cmake_import_configure PROPERTIES  FIXTURES_SETUP "cmake_import" LABELS "not_reproducible" _BACKTRACE_TRIPLES "/home/ubuntu/Desktop/Audio_comm_github/Visual_feedback_for_audio_communication/json-develop/tests/cmake_import/CMakeLists.txt;1;add_test;/home/ubuntu/Desktop/Audio_comm_github/Visual_feedback_for_audio_communication/json-develop/tests/cmake_import/CMakeLists.txt;0;")
add_test(cmake_import_build "/usr/bin/cmake" "--build" ".")
set_tests_properties(cmake_import_build PROPERTIES  FIXTURES_REQUIRED "cmake_import" LABELS "not_reproducible" _BACKTRACE_TRIPLES "/home/ubuntu/Desktop/Audio_comm_github/Visual_feedback_for_audio_communication/json-develop/tests/cmake_import/CMakeLists.txt;9;add_test;/home/ubuntu/Desktop/Audio_comm_github/Visual_feedback_for_audio_communication/json-develop/tests/cmake_import/CMakeLists.txt;0;")
