# --- Packet Parser ---
adtf_add_filter(radar_filter
    radar_filter.h
    radar_filter.cpp
)

# Link udp_sender — pulls in radar_decoder transitively
target_link_libraries(radar_filter PRIVATE udp_sender)

adtf_install_filter(radar_filter bin)
adtf_create_plugindescription(
    TARGET      radar_filter
    PLUGIN_SUBDIR "bin"
)