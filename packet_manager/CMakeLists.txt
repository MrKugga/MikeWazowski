# --- Packet Parser ---
adtf_add_filter(packet_parser_filter
    packet_parser_filter.h
    packet_parser_filter.cpp
)

# Link against the radar_decoder library —
# this also pulls in its include dirs and adtf::filtersdk transitively
target_link_libraries(packet_parser_filter PRIVATE radar_decoder)

adtf_install_filter(packet_parser_filter bin)
adtf_create_plugindescription(
    TARGET      packet_parser_filter
    PLUGIN_SUBDIR "bin"
)