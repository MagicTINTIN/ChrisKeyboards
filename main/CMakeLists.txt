idf_component_register(
    # SRCS "tusb_hid_example_main.cc"
    SRCS "detect_main.cc"
    # SRCS "reversed_main.cc"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_gpio
    )
