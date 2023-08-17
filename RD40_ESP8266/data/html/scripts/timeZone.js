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


/*
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
      const listItem = document.createElement('div');
      listItem.classList.add('timeZone-entry');
      listItem.textContent = entry.entry;

      // Add a click event listener to each entry
      listItem.addEventListener('click', () => {
        selectedEntryIndex = index; // Update the selected index
    
        // Remove 'selected' class from all entries
        const allEntries = timeZoneListDiv.querySelectorAll('.timeZone-entry');
        allEntries.forEach(entry => entry.classList.remove('selected'));
    
        // Add 'selected' class to the clicked entry
        listItem.classList.add('selected');
      });

      timeZoneListDiv.appendChild(listItem);
    });
  })

  // Make an HTTP GET request to fetch the JSON data from the server
  fetch('timeZoneData?part=2')
  .then(response => response.json())
  .then(data => {
    // Iterate through each entry in the JSON data and create a list item
    data.forEach((entry, index) => {
      const listItem = document.createElement('div');
      listItem.classList.add('timeZone-entry');
      listItem.textContent = entry.entry;

      // Add a click event listener to each entry
      listItem.addEventListener('click', () => {
        selectedEntryIndex = index; // Update the selected index
    
        // Remove 'selected' class from all entries
        const allEntries = timeZoneListDiv.querySelectorAll('.timeZone-entry');
        allEntries.forEach(entry => entry.classList.remove('selected'));
    
        // Add 'selected' class to the clicked entry
        listItem.classList.add('selected');
      });

      timeZoneListDiv.appendChild(listItem);
    });
  })

  .catch(error => {
    console.error('Error loading time zone data:', error);
  });
}
*/

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
  } else {
    console.log('No entry selected.');
  }

  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/timeZoneUpdate?value="+selectedEntryNumber);
  xhr.send();

  selectedEntryIndex = -1;

  window.location.href = "/configDone";
});
