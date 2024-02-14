import time
import asyncio
from gpiozero import DigitalInputDevice, DigitalOutputDevice

CLK_PIN = 18
DATA_PINS = [4, 17, 27, 22, 10, 9, 11, 5, 6, 13, 19, 26]
DATA_PINS = reversed(DATA_PINS)

SAMPLE_RATE = 1000

TARGET = "samples.csv"
CAPTURE_TIME = 10 # seconds


class Sampler:
    def __init__(self, sample_rate: int, clk: int, data: list[int], target_file: str, capture_time: int):
        self.sample_rate = sample_rate
        self.clk_pin = DigitalOutputDevice(clk)
        self.data_pins = [
            DigitalInputDevice(pin)
            for pin in data
        ]

        self.target_file = open(target_file, "w")
        self.capture_time = capture_time

        self.start_time = 0

    def _write_samples(self, samples: list[int]):
        t = str(time.time() - self.start_time).replace(".", ",")
        bin_str = [str(sample) for sample in samples]
        val = int("".join(bin_str), 2)
        self.target_file.write(f"{t};{val}\n")

    def _take_samples(self):
        samples = [pin.value for pin in self.data_pins]
        self._write_samples(samples)

    def _pulse_clock(self):
        self.clk_pin.on()
        time.sleep(1 / self.sample_rate)
        self.clk_pin.off()

    async def _monitor_pins(self):
        while time.time() < self.start_time + self.capture_time:
            self.clk_pin.on()
            self._take_samples()
            self.clk_pin.off()
            await asyncio.sleep(1 / self.sample_rate)

    async def _run(self):
        await asyncio.sleep(1)
        self.start_time = time.time()
        await self._monitor_pins()
        self.target_file.close()

    def run(self):
        asyncio.run(self._run())


if __name__ == "__main__":
    sampler = Sampler(SAMPLE_RATE, CLK_PIN, DATA_PINS, TARGET, CAPTURE_TIME)
    sampler.run()

