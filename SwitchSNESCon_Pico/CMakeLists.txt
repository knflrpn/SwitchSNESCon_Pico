add_executable(SwitchSNESCon_Pico
        SwitchSNESCon_Pico.c
        usb_descriptors.c
        )

pico_generate_pio_header(SwitchSNESCon_Pico ${CMAKE_CURRENT_LIST_DIR}/SNESConRead.pio)

target_include_directories(SwitchSNESCon_Pico PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(SwitchSNESCon_Pico PRIVATE pico_stdlib hardware_pio tinyusb_device tinyusb_board)
pico_add_extra_outputs(SwitchSNESCon_Pico)

# add url via pico_set_program_url
example_auto_set_url(SwitchSNESCon_Pico)
