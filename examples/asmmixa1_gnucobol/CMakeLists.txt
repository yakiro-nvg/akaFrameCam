# Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
# Unauthorized copying of this file, via any medium is strictly prohibited.
add_library(ASMMIXA1 SHARED ${CMAKE_CURRENT_SOURCE_DIR}/asmmixa1.cpp)
target_link_libraries(ASMMIXA1 akaFrameCam)