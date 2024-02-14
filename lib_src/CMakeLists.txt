cmake_minimum_required(VERSION 3.18)
project(osci_pi C)

set(CMAKE_C_STANDARD 11)

add_executable(
        osci_pi_no_dma
        rpi_adc_no_dma.c
)
add_executable(
        osci_pi_dma
        rpi_adc_dma.c
)
add_executable(
        read_test
        read_test.c
)
