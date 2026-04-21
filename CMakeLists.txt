# CMakeLists.txt

idf_component_register(
    SRCS "src/unit_asr.cpp"
    INCLUDE_DIRS "src"
    REQUIRES driver log
)