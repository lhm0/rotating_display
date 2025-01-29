// Define a variable to keep track of the selected entry
let selectedEntryIndex = -1;

// Function to load time zone list from the server
function loadTimeZoneList() {
  const timeZoneListDiv = document.getElementById('timeZone-list');
  // Clear the existing content of the timeZoneListDiv
  timeZoneListDiv.innerHTML = '';

  // Make an HTTP GET request to fetch the JSON data from the server
  fetch('timeZoneData?part=1')
  .then(response => response.json())
  .then(data => {

    // Iterate through each entry in the JSON data and create a list item
    data.forEach((entry, index) => {
      const listItem = createListItem(entry.entry, index);
      timeZoneListDiv.appendChild(listItem);
    });
  })

  // Make an HTTP GET request to fetch the JSON data from the server
  fetch('timeZoneData?part=2')
  .then(response => response.json())
  .then(data => {
    // Iterate through each entry in the JSON data and create a list item
    data.forEach((entry, index) => {
      const adjustedIndex = index + 20; // Increase index by 20 to start from 21
      const listItem = createListItem(entry.entry, adjustedIndex);
      timeZoneListDiv.appendChild(listItem);
    });
  })

  .catch(error => {
    console.error('Error loading time zone data:', error);
  });
}

// Helper function to create a list item with click event listener
function createListItem(text, index) {
  const listItem = document.createElement('div');
  listItem.classList.add('timeZone-entry');
  listItem.textContent = text;

  // Add a click event listener to each entry
  listItem.addEventListener('click', () => {
    selectedEntryIndex = index; // Update the selected index

    // Remove 'selected' class from all entries
    const allEntries = document.querySelectorAll('.timeZone-entry');
    allEntries.forEach(entry => entry.classList.remove('selected'));

    // Add 'selected' class to the clicked entry
    listItem.classList.add('selected');
  });

  return listItem;
}

function updateTime() {
  const timeDisplay = document.getElementById("time-display");

  serverSecond = serverSecond +1;
  if (serverSecond >= 60) {
    serverSecond = 0;
    serverMinute = serverMinute+1;
    if (serverMinute >= 60) {
      serverMinute = 0;
      serverHour = serverHour+1;
      if (serverHour >= 24) {
        serverHour = 0;
      }
    }
  }

  // Formatierung der Stunden, Minuten und Sekunden (fügt führende Nullen hinzu)
  let formattedHour = String(serverHour).padStart(2, '0');
  let formattedMinute = String(serverMinute).padStart(2, '0');
  let formattedSecond = String(serverSecond).padStart(2, '0');

  // Setze die formatierte Zeit in das HTML-Element
  timeDisplay.textContent = `${formattedHour}:${formattedMinute}:${formattedSecond}`;
}

let serverHour;
let serverMinute;
let serverSecond;

async function fetchServerTime() {
  try {
      // Führe eine HTTP GET-Anfrage an den Arduino-Server durch
      const response = await fetch('/server-time'); // Ersetze <arduino-ip> mit der IP-Adresse des Arduino
      
      // Überprüfe, ob die Antwort erfolgreich war
      if (!response.ok) {
          throw new Error('Network response was not ok');
      }
      
      // Parsen der JSON-Antwort
      const data = await response.json();
      
      // Speichern der Zeit in Variablen
      serverHour = data.hour;
      serverMinute = data.minute;
      serverSecond = data.second;

      // Ausgabe in der Konsole (optional)
      console.log('Stunden: ' + serverHour);
      console.log('Minuten: ' + serverMinute);
      console.log('Sekunden: ' + serverSecond);
      
  } catch (error) {
      // Fehlerbehandlung
      console.error('Es gab ein Problem mit der Anfrage:', error);
  }
}

// Event listener for when the DOM is fully loaded
document.addEventListener('DOMContentLoaded', () => {
  console.log('Document loaded.');
  loadTimeZoneList();
});

// Event listener for selectTimeZone button
document.getElementById('selectTimeZone').addEventListener('click', () => {
  let selectedEntryNumber;
  if (selectedEntryIndex !== -1) {
    selectedEntryNumber = selectedEntryIndex + 1;
    console.log('Selected entry number:', selectedEntryNumber);

  // Get the selected entry's text
  const selectedEntryText = document.querySelectorAll('.timeZone-entry')[selectedEntryIndex].textContent;

  // Display the selected entry in the HTML field with id=selectedTimeZone
  const selectedTimeZoneField = document.getElementById('selectedTimeZone');
  selectedTimeZoneField.textContent = `Selected time zone: ${selectedEntryText}`;

  } else {
    console.log('No entry selected.');
  }

  var xhr = new XMLHttpRequest();

  xhr.open("GET", "/timeZoneUpdate?value=" + selectedEntryNumber);

  // Define what happens when the request completes
  xhr.onload = function () {
    if (xhr.status >= 200 && xhr.status < 300) {
      // Request was successful, call fetchServerTime
      fetchServerTime();
    } else {
      // Handle errors or unsuccessful requests
      console.error('Failed to update time zone. Status:', xhr.status);
    }
  };

  // Define what happens in case of an error
  xhr.onerror = function () {
    console.error('Request failed');
  };

  // Send the request
  xhr.send();

  // Reset the selected entry index
  selectedEntryIndex = -1;

});

// Event listener for done button
document.getElementById('done').addEventListener('click', () => {

  window.location.href = "/configDone";
});

window.onload = fetchServerTime;

document.addEventListener('DOMContentLoaded', function () {
  // Ruft updateTime() jede Sekunde auf
  setInterval(updateTime, 1000);
});