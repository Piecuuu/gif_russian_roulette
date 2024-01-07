cmake_minimum_required(VERSION 3.5)
project(gif_russian_roulette VERSION 1.0 DESCRIPTION "A fun game that randomises GIFs from your favorites and sends them to a channel")

add_subdirectory(lib/discord_gif_extractor)

add_executable(${PROJECT_NAME}
  src/main.cc
)

target_link_libraries(${PROJECT_NAME}
  discord_gif_extractor_lib
  ${Protobuf_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  lib/discord_gif_extractor/src
  lib/discord_gif_extractor/lib/argparse/include/
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)
