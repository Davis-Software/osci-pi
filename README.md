#### Simple DMA Implementation on Raspberry Pi 3

> Requirements:
> - PiGPIO

##### Compile:
```sh
$ gcc -o dma rpi_adc_dma.c -lpigpio
```

or for the jf version:
```sh
$ gcc -o dma jfp_rpi_adc_dma.c -lpigpio -lm
```

##### Run:
```sh
$ sudo ./dma
```

or for the jf version:
```sh
$ sudo ./dma <sample-rate> <capture-time> [<output-file>]
```
* The sample rate is in Samples per second
* The capture time is in milliseconds
