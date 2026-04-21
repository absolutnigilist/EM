add_library(ListSerializer
    ListSerializer.cpp
    ListSerializer.hpp
    ${CMAKE_SOURCE_DIR}/ListNode.hpp
)

target_include_directories(ListSerializer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/List
)

target_link_libraries(ListSerializer PUBLIC
    List
)