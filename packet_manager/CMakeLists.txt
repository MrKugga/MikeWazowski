# --- Packet Parser ---
adtf_add_filter(packet_parser_filter
    packet_parser_filter.h
    packet_parser_filter.cpp
)

# Link udp_sender — pulls in radar_decoder transitively
target_link_libraries(packet_parser_filter PRIVATE udp_sender)

adtf_install_filter(packet_parser_filter bin)
adtf_create_plugindescription(
    TARGET      packet_parser_filter
    PLUGIN_SUBDIR "bin"
)