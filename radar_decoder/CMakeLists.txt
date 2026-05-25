add_library(radar_decoder STATIC
    crc_utils.h
    crc_utils.cpp
    decoded_messages.h
    decoder.h
    decoder.cpp
    validation_result.h
    validator.h
    validator.cpp
)

# C++17 for std::variant
target_compile_features(radar_decoder PUBLIC cxx_std_17)

# Anyone linking radar_decoder gets these include dirs automatically
target_include_directories(radar_decoder PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}   # radar_decoder/ headers
    ${CMAKE_SOURCE_DIR}+"/assignment"           # assignment/RadarTypes.h at project root
)

# radar_decoder uses ADTF LOG_INFO in crc_utils.cpp
target_link_libraries(radar_decoder PUBLIC adtf::filtersdk)