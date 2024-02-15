import matplotlib.pyplot as plt
import numpy as np
from random import randint


def plot_data(data: list[tuple], show_range: tuple):
    fig, ax = plt.subplots(figsize=(12, 6))

    x, y = np.zip(*data)
    ax.plot(x, y, label="")

    ax.legend()
    plt.show()

