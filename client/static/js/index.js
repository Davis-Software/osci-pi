(() => {
    const PRESETS = {
        1: {
            sampleRate: 10000,
            captureTime: 100000
        },
        2: {
            sampleRate: 10000,
            captureTime: 50000
        },
        3: {
            sampleRate: 150000,
            captureTime: 5600
        },
        4: {
            sampleRate: 150000,
            captureTime: 1200
        }
    }

    function hrNumberLarge(number, values = ["", "K", "M"]) {
        if(number >= 1000) {
            return `${(number / 1000)} ${values[1]}`;
        } else if(number >= 1000000) {
            return `${(number / 1000000)} ${values[2]}`;
        }
        return `${number} ${values[0]}`;
    }
    function hrNumber(number) {
        return new Intl.NumberFormat().format(number).replaceAll(",", " ");
    }

    const img = document.querySelector("img");

    const sampleRateInput = document.querySelector("input[name='sample-rate']");
    const captureTimeInput = document.querySelector("input[name='capture-time']");

    const presetButton1 = document.querySelector("#preset-1");
    const presetButton2 = document.querySelector("#preset-2");
    const presetButton3 = document.querySelector("#preset-3");
    const presetButton4 = document.querySelector("#preset-4");

    const startButton = document.querySelector("#start");


    function setPreset(preset) {
        sampleRateInput.value = PRESETS[preset].sampleRate;
        captureTimeInput.value = PRESETS[preset].captureTime;
        sampleRateInput.dispatchEvent(new Event("input"));
        captureTimeInput.dispatchEvent(new Event("input"));
    }
    sampleRateInput.addEventListener("input", () => {
        sampleRateInput.parentElement.querySelector("span").innerText = hrNumberLarge(sampleRateInput.value, ["S", "KS", "MS"]) + "/s";
    })
    captureTimeInput.addEventListener("input", () => {
        captureTimeInput.parentElement.querySelector("span").innerText = hrNumber(captureTimeInput.value) + " µs";
    })
    setPreset(1);


    presetButton1.addEventListener("click", () => {
        setPreset(1);
    })
    presetButton2.addEventListener("click", () => {
        setPreset(2);
    })
    presetButton3.addEventListener("click", () => {
        setPreset(3);
    })
    presetButton4.addEventListener("click", () => {
        setPreset(4);
    })


    startButton.addEventListener("click", () => {
        startButton.disabled = true;
        const sampleRate = sampleRateInput.value;
        const captureTime = captureTimeInput.value;

        img.src = `/measure?sr=${sampleRate}&ct=${captureTime}&t=${Date.now()}`
    })
    img.addEventListener("load", () => {
        startButton.disabled = false;
    })
})()