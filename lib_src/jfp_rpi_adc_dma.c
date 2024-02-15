#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pigpio.h>
#include <string.h>
#include <math.h>

#define CLK_PIN 18
#define DATA_PINS { 4, 17, 27, 22, 10, 9, 11, 5, 6, 13, 19, 26 }
#define DATA_PINS_COUNT 12

// DMA registers
#define BCM2835_PERI_BASE   0x3F000000
#define GPIO_BASE           (BCM2835_PERI_BASE + 0x200000) /* GPIO controller */
#define BLOCK_SIZE          (4*1024)

#define GPLEV0              0x34    /* GPIO Level 0 */

volatile unsigned long SAMPLE_RATE;
volatile float CAPTURE_TIME;
char TARGET_FILE[50] = "samples.csv";

volatile unsigned int *gpio;

void write_samples(FILE* file, int* samples, long int* timings) {
    for (int i = 1; i < SAMPLE_RATE * CAPTURE_TIME; i++) {
        // load first sample bits and add them
        int val = 0;
        for (int j = 0; j < DATA_PINS_COUNT; j++) {
            val |= (samples[i * DATA_PINS_COUNT + j] << j);
        }

        // write the time and the samples to the file
        fprintf(file, "%ld", timings[i]);
        fprintf(file, ",%d\n", val);
    }
}


void monitor_pins(int clk_pin, int* data_pins, int sample_rate, int* samples, long int* timings, struct timespec start_time, long int sample_steps) {
    unsigned int reg = gpio[GPLEV0 / 4];
    long int sample_step = 0;

    // some cleanup transfers
    for(int i = 0; i < 50; i++){
        gpioWrite(clk_pin, PI_HIGH);
        sleep(.001);
	gpioWrite(clk_pin, PI_LOW);
	sleep(.001);
    }

    while (sample_step < sample_steps) {
        // get the current time
        struct timespec current_time;
        struct timespec sleep_time_spec;
        clock_gettime(CLOCK_REALTIME, &current_time);

        // toggle the clock pin and take samples
        gpioWrite(clk_pin, PI_HIGH);
        sample_step++;

        long sample_bank = sample_step * DATA_PINS_COUNT;
        for (int i = 0; i < DATA_PINS_COUNT; i++) {
//            samples[sample_bank + i] = (int)(reg >> data_pins[i]) & 1;
            samples[sample_bank + i] = gpioRead(data_pins[i]); // this is slower
	    timings[sample_step] = current_time.tv_nsec - start_time.tv_nsec;
        }

        gpioWrite(clk_pin, PI_LOW);

        // sleep for the appropriate amount of time to maintain the sample rate
        long int sleep_time = sample_step * 1000000000 / sample_rate - (current_time.tv_sec - start_time.tv_sec) * 1000000000 - (current_time.tv_nsec - start_time.tv_nsec);
        if (sleep_time > 0) {
            sleep_time_spec.tv_sec = sleep_time / 1000000000;
            sleep_time_spec.tv_nsec = sleep_time % 1000000000;
            nanosleep(&sleep_time_spec, NULL);
        }
    }
}


void run_sampler() {
    // initialize wiringPi
    if (gpioInitialise() == -1) {
        fprintf(stderr, "Failed to initialize wiringPi\n");
        exit(1);
    }

    int clk_pin = CLK_PIN;
    int data_pins[DATA_PINS_COUNT] = DATA_PINS;
    int sample_rate = SAMPLE_RATE;
    const char* target_file_path = TARGET_FILE;
    long int sample_steps = floor(CAPTURE_TIME * SAMPLE_RATE);

    // make space in memory for the samples and leave some buffer space
    printf("Allocating memory\n");
    int* samples = malloc(sizeof(int) * sample_steps * DATA_PINS_COUNT + 100);
    long int* timings = malloc(sizeof(long int) * sample_steps + 100);
    if(!samples){
	printf("Failed to allocate memory\n");
	return;
    }

    // set up the pins
    gpioSetMode(clk_pin, PI_OUTPUT);
    for (int i = 0; i < DATA_PINS_COUNT; i++) {
        gpioSetMode(data_pins[i], PI_INPUT);
    }

    // open /dev/mem
    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd == -1) {
        fprintf(stderr, "Failed to open /dev/mem\n");
        exit(1);
    }

    // Map DMA registers into virtual memory
    void* dma_map = mmap(
            NULL,                   // Any address in our space will do
            BLOCK_SIZE,             // Map length
            PROT_READ | PROT_WRITE, // Enable reading & writing to the mapped memory
            MAP_SHARED,             // Shared with other processes
            mem_fd,                 // File to map
            GPIO_BASE               // Offset to GPIO peripheral
    );

    // close the file as it is no longer needed
    close(mem_fd);

    if (dma_map == MAP_FAILED) {
        fprintf(stderr, "Failed to mmap DMA registers\n");
        exit(1);
    }

    // Get a pointer to the DMA channel registers
    gpio = (volatile unsigned int *)dma_map;

    // start the clock
    struct timespec start_time;
    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    printf("Taking measurements...\n");
    monitor_pins(clk_pin, data_pins, sample_rate, samples, timings, start_time, sample_steps);

    // get the current time
    clock_gettime(CLOCK_REALTIME, &current_time);

    // write the samples to the file
    FILE* target_file = fopen(target_file_path, "w");
    if (target_file == NULL) {
        fprintf(stderr, "Failed to open target file\n");
        exit(1);
    }

    printf("Writing samples to file...\n");
    write_samples(target_file, samples, timings);

    fclose(target_file);

    // Unmap DMA registers
    munmap(dma_map, 0x1000);


    // print the execution time
    long exec_time = (current_time.tv_sec - start_time.tv_sec) * 1000000000 + (current_time.tv_nsec - start_time.tv_nsec);
    printf("Execution time: %8ld nanoseconds\n", exec_time);
    printf("Should have taken: %8f seconds\n", CAPTURE_TIME);
    printf("Actual sample rate: %8ld S/s\n", sample_steps * 1000000000 / exec_time);
}

int main(int argc, char* argv[]) {
    if(argc < 3 || (argc == 2 && argv[1] == "help") || argc > 4){
	printf("Usage: dma <sample-rate> <capture-time> [<output-file>]\n");
	printf("\tSample Rate: Samples per second\n\tCapture Time: Capture duration in µs\n");
	return -1;
    }

    SAMPLE_RATE = atoi(argv[1]);
    CAPTURE_TIME = ((float)atoi(argv[2]))/1000000;
    if(argc == 4){
        strcpy(TARGET_FILE, argv[3]);
    }

    run_sampler();
    return 0;
}
