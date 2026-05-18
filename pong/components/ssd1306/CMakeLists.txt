idf_component_register(
    SRCS "ssd1306.c" "font.c" "i2c_utils.c" 
    INCLUDE_DIRS "include" 
    PRIV_REQUIRES driver 
) 
