from flask import Flask, request, render_template
import platform
import requests

from plot import plot_data


app = Flask(__name__)
socket_route = "192.168.1.321"


@app.route("/")
def route_index():
    return render_template("index.html")


@app.route("/measure")
def route_measure():
    sample_rate = request.args.get("sr", 50000, int)
    capture_time = request.args.get("ct", 500, int)

    #resp = requests.get(f"http://{socket_route}/{sample_rate}?ct={capture_time}")
    plot_data([(1,2), (3,4), (5,6), (7,8)], ())

if __name__ == "__main__":
    app.run("0.0.0.0", 8080 if platform.system() == "Linux" else 80, debug=True)

