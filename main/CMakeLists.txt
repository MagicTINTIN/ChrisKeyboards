idf_component_register(
    SRCS "main.cc"
         "esp_hidd_prf_api.c"
         "hid_dev.c"
         "hid_device_le_prf.c"
    # SRCS "tusb_hid_example_main.cc"
    # SRCS "detect_main.cc"
    # SRCS "detect_main2.cc"
    # SRCS "reversed_main.cc"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_gpio
    PRIV_REQUIRES esp_driver_ledc bt nvs_flash 
    )
