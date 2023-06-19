#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <wiringPi.h>

#define CLK_PIN 6
#define DATA_PINS { 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12 }
#define DATA_PINS_COUNT 12

#define SAMPLE_RATE 300000 // S/s

#define TARGET_FILE "samples.csv"
#define CAPTURE_TIME 1.0 // seconds

void write_samples(FILE* file, int* samples) {
    for (int i = 0; i < SAMPLE_RATE * CAPTURE_TIME; i++) {
        // calculate the time for a single sample
        struct timespec time_for_sample;
        time_for_sample.tv_sec = 0;
        time_for_sample.tv_nsec = 1000000000 / SAMPLE_RATE;

        // load first sample bits and add them
        int val = 0;
        for (int j = 0; j < DATA_PINS_COUNT; j++) {
            val |= (samples[i * DATA_PINS_COUNT + j] << j);
        }

        // write the time and the samples to the file
        long t = time_for_sample.tv_nsec * i;
        fprintf(file, "%8ld", t);
        fprintf(file, ",%d\n", val);
    }
}

void monitor_pins(int clk_pin, int* data_pins, int sample_rate, int* samples, struct timespec start_time, long int sample_steps) {
    long int sample_step = 0;
    while (sample_step < sample_steps) {
        // get the current time
        struct timespec current_time;
        clock_gettime(CLOCK_REALTIME, &current_time);

        // toggle the clock pin and take samples
        digitalWrite(clk_pin, HIGH);
        sample_step++;
        for (int i = 0; i < DATA_PINS_COUNT; i++) {
            // read the value of each data pin and store it in the samples array
            samples[i] = digitalRead(data_pins[i]);
        }
        digitalWrite(clk_pin, LOW);

        // sleep for the appropriate amount of time
        long int sleep_time = sample_step * 1000000000 / sample_rate - (current_time.tv_sec - start_time.tv_sec) * 1000000000 - (current_time.tv_nsec - start_time.tv_nsec);
        if (sleep_time > 0) {
            struct timespec sleep_time_spec;
            sleep_time_spec.tv_sec = sleep_time / 1000000000;
            sleep_time_spec.tv_nsec = sleep_time % 1000000000;
            nanosleep(&sleep_time_spec, NULL);
        }
    }
}

void run_sampler() {
    // initialize wiringPi
    if (wiringPiSetupGpio() == -1) {
        fprintf(stderr, "Failed to initialize wiringPi\n");
        exit(1);
    }

    int clk_pin = CLK_PIN;
    int data_pins[DATA_PINS_COUNT] = DATA_PINS;
    int sample_rate = SAMPLE_RATE;
    const char* target_file_path = TARGET_FILE;
    long int sample_steps = CAPTURE_TIME * SAMPLE_RATE;

    // make space in memory for the samples
    int* samples = malloc(sizeof(int) * sample_steps * DATA_PINS_COUNT);

    // set up the pins
    pinMode(clk_pin, OUTPUT);
    for (int i = 0; i < DATA_PINS_COUNT; i++) {
        pinMode(data_pins[i], INPUT);
    }

    // start the clock
    struct timespec start_time;
    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    monitor_pins(clk_pin, data_pins, sample_rate, samples, start_time, sample_steps);

    // get the current time
    clock_gettime(CLOCK_REALTIME, &current_time);

    // write the samples to the file
    FILE* target_file = fopen(target_file_path, "w");
    if (target_file == NULL) {
        fprintf(stderr, "Failed to open target file\n");
        exit(1);
    }

    printf("Writing samples to file...\n");
    write_samples(target_file, samples);

    fclose(target_file);

    // print the execution time
    long exec_time = (current_time.tv_sec - start_time.tv_sec) * 1000000000 + (current_time.tv_nsec - start_time.tv_nsec);
    printf("Execution time: %8ld nanoseconds\n", exec_time);
    printf("Should have taken: %8f seconds\n", CAPTURE_TIME);
    printf("Actual sample rate: %8ld S/s\n", sample_steps * 1000000000 / exec_time);
}

int main() {
    run_sampler();
    return 0;
}
