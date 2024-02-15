from io import BytesIO
import matplotlib.pyplot as plt


def plot_data(x: list, y: list, info: str):
    fig, ax = plt.subplots(figsize=(12, 5))

    # Display the range of the data
    ax.plot(x, list(map(lambda a: 0, x)), linestyle="--", color="black")
    ax.plot(x, y)
    ax.set_xlabel(f"Time (ns) | {info}")
    ax.set_ylabel("Voltage (V)")
    ax.set_title("Time - Voltage")

    buf = BytesIO()
    plt.savefig(buf, format="png")
    buf.seek(0)

    return buf
