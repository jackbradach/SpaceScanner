# Source containing 'main()'
set(SRC
    dbg.c
    led_gamma.c
    main.c
    neopixels.c
    uart.c
    # timer.c
	spi.c
	# mcp23s18.c
	# ioexp_lcd_shim.c
	# ioexp_input_shim.c
	# hd44780.c
    # ili9341.c
	# clcd.c
    # pgm_iter.c
)

add_avr_executable(
    fw
    ${SRC}
)

avr_target_link_libraries(
    fw
    m
)
