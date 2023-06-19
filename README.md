#### Simple DMA Implementation on Raspberry Pi 3

> Requirements:
> - PiGPIO

##### Compile:
```sh
$ gcc -o dma rpi_adc_dma.c -lpigpio
```

##### Run:
```sh
$ sudo ./dma
```
