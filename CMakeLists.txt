cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(RadarPlugin)

#find_package(ADTF COMPONENTS filtersdk ui)
find_package(ADTF_sdk REQUIRED)
find_package(adtffiltersdk REQUIRED)

# radar_decoder is built first — packet_manager depends on it
add_subdirectory(radar_decoder)
add_subdirectory(packet_manager)
add_subdirectory(udp_sender) 