add_library(List
    List.cpp
    List.hpp
    ${CMAKE_SOURCE_DIR}/ListNode.hpp
)

target_include_directories(List PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(List PUBLIC
    glog::glog
)