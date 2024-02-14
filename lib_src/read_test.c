#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define BCM2835_PERI_BASE   0x3F000000
#define GPIO_BASE           (BCM2835_PERI_BASE + 0x200000) /* GPIO controller */
#define BLOCK_SIZE          (4*1024)

#define GPFSEL2             0x08    /* GPIO Function Select 2 */
#define GPLEV0              0x34    /* GPIO Level 0 */

volatile unsigned int *gpio;

int setup_io()
{
    int mem_fd;
    void *gpio_map;

    // Open /dev/mem
    if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
        printf("Failed to open /dev/mem\n");
        return -1;
    }

    // Map GPIO memory
    gpio_map = mmap(
            NULL,                   // Any address in our space will do
            BLOCK_SIZE,             // Map length
            PROT_READ | PROT_WRITE, // Enable reading & writing to the mapped memory
            MAP_SHARED,             // Shared with other processes
            mem_fd,                 // File to map
            GPIO_BASE               // Offset to GPIO peripheral
    );

    close(mem_fd); // No need to keep mem_fd open after mmap

    if (gpio_map == MAP_FAILED) {
        printf("mmap error %d\n", (int)gpio_map);
        return -1;
    }

    gpio = (volatile unsigned int *)gpio_map;

    return 0;
}

int main()
{
    if (setup_io() == -1) {
        return -1;
    }

    // Set GPIO pin 23 as input
    unsigned int sel_reg = gpio[GPFSEL2 / 4];
    sel_reg &= ~(7 << 9);  // Clear bits 9-11
    sel_reg |= (0 << 9);   // Set bits 9-11 to 0 for input mode
    gpio[GPFSEL2 / 4] = sel_reg;


    while (1) {
        // Read GPIO pin 23
        unsigned int level = (gpio[GPLEV0 / 4] >> 26) & 1;
        printf("GPIO 23 state: %d\n", level);
        usleep(250000);
    }

    return 0;
}
