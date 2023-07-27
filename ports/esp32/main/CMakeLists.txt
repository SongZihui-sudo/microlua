LIST( APPEND include_esp32
    ../../../src/
    ../../../src/interface)

LIST(APPEND source_esp32  main.c ../../../src/onelua.c ../../../src/interface/pin.c esp32_pin.c)

idf_component_register(SRCS ${source_esp32} INCLUDE_DIRS ${include_esp32})
