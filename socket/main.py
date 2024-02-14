import os
import subprocess
from flask import Flask, request


app = Flask(__name__)
base_path = "/root/osci-pi/lib_src"


@app.route("/<sample_rate>")
def index(sample_rate: int):
    if not sample_rate.isdigit():
        return "No valid sample rate"

    capture_time = request.args.get("ct", "1000", str)

    if not capture_time.isdigit():
        return "No valid capture time"

    stdout = subprocess.run([os.path.join(base_path, "dma"), sample_rate, capture_time, "socket_out.csv"], cwd=base_path, capture_output=True)

    with open(os.path.join(base_path, "socket_out.csv"), "r") as f:
        return {
            "status": stdout.returncode,
            "stdout": stdout.stdout.decode("utf-8"),
            "data": f.read()
        }


app.run("0.0.0.0", 80, debug=True)

