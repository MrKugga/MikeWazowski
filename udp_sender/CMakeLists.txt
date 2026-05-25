add_library(udp_sender STATIC
    udp_radar_packet.h
    cycle_accumulator.h
    cycle_accumulator.cpp
)


target_include_directories(udp_sender PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}   # udp_sender/ headers
)

# udp_sender depends on radar_decoder for tRDIDetection etc.
# and on adtf::filtersdk for LOG_INFO if needed
target_link_libraries(udp_sender PUBLIC
    radar_decoder
)