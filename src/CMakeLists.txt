# Source containing 'main()'
set(SRC
    anim.c
    dac.c
    dbg.c
    dht22.c
    ftseg.c
    fx.c
    ht16k33.c
    # led_gamma.c
    main.c
    # neopixels.c
    timer.c
    twi_master.c
    uart.c
)

add_avr_executable(
    fw
    ${SRC}
)

avr_target_link_libraries(
    fw
    m
)
