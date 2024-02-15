from flask import Flask, request, render_template, send_file
import platform
import requests

from plot import plot_data


app = Flask(__name__)
socket_route = "192.168.1.231"


@app.route("/")
def route_index():
    return render_template("index.html")


@app.route("/measure")
def route_measure():
    sample_rate = request.args.get("sr", 10000, int)
    capture_time = request.args.get("ct", 100000, int)

    resp = requests.get(f"http://{socket_route}/{sample_rate}?ct={capture_time}")
    json = resp.json()
    data = json["data"].strip()
    
    parsed_x = []
    parsed_y = []
    for reading in data.split("\n"):
        x, y = reading.split(",")
        parsed_x.append(int(x))
        y_val = int(y)-2048
        
        # Normalize the data to volts
        parsed_y.append(y_val * 0.00299760191846522782)

    plot = plot_data(parsed_x, parsed_y, json["stdout"].split("seconds\n").pop().strip())

    return send_file(plot, mimetype="image/png")


if __name__ == "__main__":
    app.run("0.0.0.0", 8080 if platform.system() == "Linux" else 80, debug=True)

