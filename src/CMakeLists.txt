# Source containing 'main()'
set(SRC
    crc7.c
    dac.c
    dbg.c
    dht22.c
    ftseg.c
    fx.c
    ht16k33.c
    led_gamma.c
    main.c
    neopixels.c
    timer.c
    # sdcard.c
	# spi.c
    twi_master.c
    uart.c
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
