cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(RadarPlugin)

find_package(ADTF COMPONENTS filtersdk ui)
message(STATUS "ADTF found at: ${ADTF_DIR}")
message(STATUS "ADTF version:  ${ADTF_VERSION}")
#find_package(adtf_display_toolbox REQUIRED)  # for cMixin

# --- Packet Parser ---
adtf_add_filter(packet_parser_filter
    packet_manager/packet_parser_filter.h
    packet_manager/packet_parser_filter.cpp)
adtf_install_filter(packet_parser_filter bin)
adtf_create_plugindescription(TARGET packet_parser_filter PLUGIN_SUBDIR "bin")
