add_contract(tg.bind tg.bind ${CMAKE_CURRENT_SOURCE_DIR}/src/tg.bind.cpp)

target_include_directories(tg.bind
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include/tg.bind
)

set_target_properties(tg.bind
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options( tg.bind PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
