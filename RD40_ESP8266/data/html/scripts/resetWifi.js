function getWifiParam() {
    fetch('/getWifiParam')
    .then((response) => {
        if (response.ok) {
            return response.json();
        } else {
            throw new Error('Error on request. Status: ' + response.status);
        }
    })

    .then((data) => {
        document.getElementById("inputSSID").value = data.ssid;
        document.getElementById("inputPassword").value = data.password;
    })

    .catch((error) => {
        console.error('Error on Request:', error); 
    });
}
  

function submitWifiParam() {
    // Get the value from the input field
    let textFieldValue1 = document.getElementById("inputSSID").value;
    let textFieldValue2 = document.getElementById("inputPassword").value;
    let xhr = new XMLHttpRequest();
    xhr.open("GET", "/uploadWifiParam?value1="+textFieldValue1+"&value2="+textFieldValue2, true);  
    xhr.send(); 
  }

function resetButton() {
	if (confirm("WiFi Credentials will be deleted. You need to re-connect to the controller. Are you sure?")) {
		var xhr = new XMLHttpRequest();
		xhr.open("GET", "/reset?", true);
		xhr.send();
	} 
}

function configDone() {
	window.location.href = "/configDone";
}

// Event listener for when the DOM is fully loaded
document.addEventListener('DOMContentLoaded', () => {
	getWifiParam();
});

// Event listener for reset data button
document.getElementById('reset').addEventListener('click', () => {
	resetButton();
	getWifiParam();
});

// Event listener for done button
document.getElementById('done').addEventListener('click', () => {
	submitWifiParam();
    configDone();
});
  