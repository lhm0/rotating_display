	let gateway = `ws://${window.location.hostname}/ws`;
	let websocket;

  window.addEventListener('load', onLoad);

  function onLoad(event) {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
  }
  
  function onOpen(event) {
    console.log('Connection opened');
  }

  function onClose(event) {
    console.log('Connection closed');
  }
  
  function onMessage(event) {
    console.log("ws Message!");
    getWeatherParam();
  }

  function getWeatherParam() {
    fetch('/getWeatherParam')
      .then((response) => {
        if (response.ok) {
          return response.json();
        } else {
          throw new Error('Error on request. Status: ' + response.status);
        }
      })
      .then((data) => {
        document.getElementById("inputField1").value = data.apiKey;
        document.getElementById("inputField2").value = data.location;
        document.getElementById("inputField3").value = data.country;
      })
      .catch((error) => {
        console.error('Error on Request:', error); 
      });
  }

  function getWeather() {
    fetch('/getWeather')
      .then((response) => {
        if (response.ok) {
          return response.json();
        } else {
          throw new Error('Error on request. Status: ' + response.status);
        }
      })
      .then((data) => {
        document.getElementById("w_temp").innerHTML = "temperature: " + data.w_temp + "°C";
        document.getElementById("w_humi").innerHTML = "humidity: " + data.w_humi + "%";
        var path = "/w_img" + data.w_icon + ".PNG";
        document.getElementById("w_data").src = path;
      })
      .catch((error) => {
        console.error('Error on Request:', error); 
      });
  }
  

 	function submitWeatherParam() {
    // Get the value from the input field
    let textFieldValue1 = document.getElementById("inputField1").value;
    let textFieldValue2 = document.getElementById("inputField2").value;
    let textFieldValue3 = document.getElementById("inputField3").value;
    let xhr = new XMLHttpRequest();
    xhr.open("GET", "/uploadWeatherParam?value1="+textFieldValue1+"&value2="+textFieldValue2+"&value3="+textFieldValue3, true);  
    xhr.send(); 
  }

  function configDone() {
    window.location.href = "/configDone";
  }

// Event listener for when the DOM is fully loaded
document.addEventListener('DOMContentLoaded', () => {
  console.log('Document loaded.');
  getWeatherParam();
  getWeather();

   // Set up a timer to call getWeatherParam() every 2 seconds
   setInterval(getWeather, 2000);
});

// Event listener for update data button
document.getElementById('update').addEventListener('click', () => {
  submitWeatherParam();
});

// Event listener for done button
document.getElementById('configDone').addEventListener('click', () => {
  configDone();
});
