  
function getPageData() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            var data = JSON.parse(this.responseText);
            console.log("brightness: " + data.brightness);
            console.log("mode: " + data.mode);
            document.getElementById("brightnessSlider").value = data.brightness;
            document.querySelector('#option' + (parseInt(data.mode)+1).toString() ).checked = true;
            document.getElementById("brightnessValue").innerHTML = data.brightness + " %";
        }
    };
    xhr.open("GET", "/getParam", true);
    xhr.send();
}

function toggleCheckbox(element) {
    console.log("element.id: " + element.id);
    var xhr = new XMLHttpRequest();
    if(element.checked) { 
        xhr.open("GET", "/updateradiobutton?button="+element.id); 
        xhr.send();
    }
}

function updateSliderBrightness(element) {
    var sliderValue = document.getElementById("brightnessSlider").value;
    document.getElementById("brightnessValue").innerHTML = sliderValue + "%";
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/slider?value="+sliderValue, true);
    xhr.send();
}

function configWeather() {
    window.location.href = "/configWeather";
}

function resetWifi() {
    window.location.href = "/resetWifi";
}

function fileManager() {
    window.location.href = "/fileManager";
}

function rd40ImageManager() {
    window.location.href = "/rd40ImageManager";
}

function timeZone() {
    window.location.href = "/timeZone";
}

document.addEventListener("DOMContentLoaded", function(event) { 
    getPageData();
});

// Event listener for "configure weather" button
document.getElementById('configure-weather').addEventListener('click', () => {
    configWeather();
});

// Event listener for "reset wifi" button
document.getElementById('reset-wifi').addEventListener('click', () => {
    resetWifi();
});

// Event listener for "file browser" button
document.getElementById('file-browser').addEventListener('click', () => {
    fileManager();
});

// Event listener for "image manager" button
document.getElementById('image-manager').addEventListener('click', () => {
    rd40ImageManager();
});

// Event listener for "file browser" button
document.getElementById('time-zone').addEventListener('click', () => {
    timeZone();
});

// Event listener for brightness slider
document.getElementById('brightnessSlider').addEventListener('change', () => {
    updateSliderBrightness();
});
