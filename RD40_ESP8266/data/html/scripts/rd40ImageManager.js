// Define global variables
let currentPath = "/"; // current directory path
let fileList = document.getElementById("file-list"); // file list DOM element
let fileItems = []; // list of file item DOM elements
let selectedFile = ""; // name of the selected file/folder
let selectedFilePath = ""; // path of the selected file/folder

let scrollDirection = 1; // Standardmäßig positiv für normale Scrollrichtung
let touchStartX = 0; //                                                     
let touchStartY = 0; //                                                     
let mousedownTime = 0; // time when mouse button was pressed down           
let mouseupTime = 0; // time when mouse button was released                 
let clickHandled = false; //                                                
let longClickTimeout; // timeout ID for handling long click events          
let isTouchDevice = false; // flag to indicate if device is touch-enabled   
let touchMoved; //                                                          

let originalPixels; // Declare a global variable to store the original pixel buffer of the image.
let currentPixels; // Declare a global variable to store the current pixel buffer of the displayed image.
let previewPixels; // Declare a flobal variable to store the preview pixels
let invert = false; // RD40 image will be inverted, when true
let rd40FileName; // filename of the RD40 image

// Load the list of files and directories in the current path
function loadFileList() {
    console.log('Loading file list...');

    // Send an HTTP GET request to the server to retrieve a list of files and directories in the current path
    fetch(`/filelist?path=${encodeURIComponent(currentPath)}`)
    .then(response => response.json())
    .then(data => {
        console.log('Received file list:', data);

        // Clear any previous elements from the file list display
        fileList.innerHTML = '';

        // If the current path is not the root directory, add a ".." directory to represent the parent directory
        if (currentPath !== "/") {
            const parentItem = document.createElement("div");
            parentItem.textContent = "..";
            parentItem.classList.add("directory");
            parentItem.onclick = () => {
                // Update the path to the parent directory and reload the file list display
                currentPath = currentPath.split('/').slice(0, -2).join('/') + '/';
                loadFileList();
            };
            fileList.appendChild(parentItem);
        }

        // Create DOM elements for each file or directory in the list and add them to the file list display
        const fileItems = data.map(file => {
            const item = document.createElement("div");
            item.textContent = file;

            // Add the "directory" class if it is a directory, to visually highlight it
            if (file.indexOf('.') === -1) {
                item.classList.add("directory");
            }

            // Add a click handler to select the element on click
            item.onclick = () => {
                // Remove the "selected" class from any other elements in the file list
                fileItems.forEach(el => el.classList.remove("selected"));
                // Add the "selected" class to the clicked element
                item.classList.add("selected");
            };
            return item;
        });

        // Add all created file and directory elements to the file list display
        fileItems.forEach(item => fileList.appendChild(item));
    })
    .catch(error => {
        console.error('Error loading file list:', error);
    });

    // Send an HTTP GET request to the server to retrieve information about the available disk space
    fetch('/diskspace')
    .then(response => response.json())
    .then(data => {
        const { totalBytes, usedBytes } = data;
        console.log(`Total Bytes: ${totalBytes}, Used Bytes: ${usedBytes}`);

        // Update the disk space display with the retrieved information
        const diskSpaceDisplay = document.getElementById('diskSpace');
        diskSpaceDisplay.textContent = "SPIFFS total: " + totalBytes + " bytes, free: " + (totalBytes-usedBytes) + " bytes";    
    })
    .catch(error => console.error(error));
}

// Download a file from the server using the specified URL
function downloadFile(fileUrl) {
    console.log("fileUrl = " + fileUrl);
  
    // Send an HTTP GET request to the server to retrieve the file data as a blob object
    fetch('/downloadfile?filename=' + fileUrl)
    .then(response => response.blob())
    .then(blob => {
        var fileName = getFileNameFromUrl(fileUrl); // Extract the filename from the URL
        var link = document.createElement('a');
        link.href = window.URL.createObjectURL(blob);
        link.download = fileName;
        link.click(); // Clicks the "Download" link to download the file
      })
      .catch(function(error) {
        console.log('Problem beim Herunterladen der Datei:', error);
      });
}
  
// Extract the file name from a given URL
function getFileNameFromUrl(url) {
    // Splits the URL by "/" and returns the last element of the resulting array, which is the filename
    var parts = url.split("/");
    return parts[parts.length - 1];
}

// Replaces the file extension of a filename with the extension ".rd40".
function replaceFileExtension(filename) {
    const dotIndex = filename.lastIndexOf('.');
    if (dotIndex !== -1) {
      const basename = filename.substring(0, dotIndex);
      return `${basename}.rd40`;
    } else {
      return `${filename}.rd40`;
    }
}

// Get the size of a file with the specified name
function getFileSize(filename) {
    console.log("getFileSize: " + filename);
  
    return fetch(`/fileSize?filename=${filename}`)
      .then(response => response.json()) // Parse response as JSON
      .then(data => {
        console.log(filename + " fileSize: " + data.size);
        return data.size;
      })
      .catch(error => console.error(error));
}

// Preview the contents of a file with the specified name
function previewFile(fileName) {
    console.log("previewFile: "+fileName);

    // If the file is an image, download it and display it in an <img> element.
    if (/\.(gif|jpe?g|tiff?|png|webp|bmp)$/i.test(fileName)) {
        console.log("previewFile1: "+'/downloadfile?filename=' + encodeURIComponent(fileName));
        fetch('/downloadfile?filename=' + encodeURIComponent(fileName))
        .then(response => response.blob())
        .then(blob => {
            // Hide the textarea and show the image element
            var textareaElement = document.getElementsByTagName("textarea")[0];
            textareaElement.style.display = "none";     
            var imgElement = document.getElementsByTagName("img")[0];
            imgElement.style.display = "block";

            // Create a URL for the blob and set it as the image source
            const objectURL = URL.createObjectURL(blob);
            document.getElementById("image-preview").src = objectURL;
        })
        .catch(error => {
            console.error('Error downloading file:', error);
        });
} 

// If the file is an rd40 file, download it and display it in an <img> element.
if (/\.(rd40)$/i.test(fileName)) {
    console.log("previewFile1: " + '/downloadfile?filename=' + encodeURIComponent(fileName));
    fetch('/downloadfile?filename=' + encodeURIComponent(fileName))
      .then(response => response.blob())
      .then(blob => {
        const reader = new FileReader();
        reader.onloadend = function () {
          const arrayBuffer = reader.result;
          const dataView = new DataView(arrayBuffer);
  
          // Create a new Uint8Array to store the file data in the "pixels" array
          const pixels = new Uint8Array(dataView.buffer, dataView.byteOffset, dataView.byteLength);
  
          // Hide the textarea and show the image element
          var textareaElement = document.getElementsByTagName("textarea")[0];
          textareaElement.style.display = "none";
          var imgElement = document.getElementById("image-preview");
          imgElement.style.display = "block";
    
          let row, col;
          for (col = 0; col < 110; col++) {
            for (row = 0; row < 110; row++) {
              const pixelIndex = 4 * (col + (109-row) * 110);
              const pixelByte = Math.floor(row / 8);
              const pixelBit = row % 8;
              const pixelValue = (pixels[col * 14 + pixelByte] & (1 << pixelBit)) ? 255 : 0;
              previewPixels[pixelIndex] = pixelValue; // Red channel
              previewPixels[pixelIndex + 1] = pixelValue; // Green channel
              previewPixels[pixelIndex + 2] = pixelValue; // Blue channel
              previewPixels[pixelIndex + 3] = 255; // Alpha channel (fully opaque)
            }
          }
          // Display the modified image in the HTML element.
          displayRD40('image-preview', previewPixels);
        };
        reader.readAsArrayBuffer(blob);
      })
      .catch(error => {
        console.error('Error downloading file:', error);
      });
  }

  // If the file is a plain text file, download it and display the contents in a <textarea> element.
    if (/\.(txt|css|html|js)$/i.test(fileName)) {
        fetch('/downloadfile?filename=' + encodeURIComponent(fileName))
        .then(response => response.text())
        .then(text => {
            // Hide the image element and show the textarea
            var imgElement = document.getElementsByTagName("img")[0];
            imgElement.style.display = "none";
            var textareaElement = document.getElementsByTagName("textarea")[0];
            textareaElement.style.display = "inline";
            
            // Set the file contents as the textarea value
            document.getElementById("text-box").value = text;
        });
    } 

    // Request the file size and display it.
    if (/\.(gif|jpe?g|tiff?|png|webp|bmp|txt|css|html|js|rd40)$/i.test(fileName)) {
        console.log("request fileSize of " + fileName);

        // Request the file size and create a Promise to handle the result.
        const fileSizePromise = getFileSize(fileName);
        fileSizePromise.then(fileSize => {
            // Update the file size display with the filename and size in bytes.
            const fileSizeDisplay = document.getElementById('fileSize');
            fileSizeDisplay.textContent = getFileNameFromUrl(fileName) + " (" + fileSize + "bytes)";
        }).catch(error => {
            console.error(error);
        });
    }
}

// Clear the currently selected file path
function clearSelectedFilePath() {
  selectedFilePath = "";
  document.getElementById("text-selected-file").value = selectedFilePath;
  document.getElementById("image-preview").style.display = "none";
  document.getElementsByClassName("text4")[0].style.display = "none";
  document.getElementById('fileSize').textContent = "";
}

function handleFileSelect() {
  const input = document.createElement('input');
  input.type = 'file';
  input.accept = 'image/*';
  
  input.onchange = (event) => {
    const file = event.target.files[0];
    const reader = new FileReader();
    
    reader.onload = (event) => {
      const img = new Image();
      
      img.onload = () => {
        const canvas = document.createElement('canvas');
        canvas.width = 110;
        canvas.height = 110;
        
        const ctx = canvas.getContext('2d');
        ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
        
        const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
        originalPixels = new Uint8ClampedArray(imageData.data);
        currentPixels = new Uint8ClampedArray(originalPixels);
        
        // Manipulate the image if needed
        manipulate(2.55 * parseInt(document.getElementById('thresholdSlider').value));
        
        // Update the displayed filename
        rd40FileName = document.getElementById('rd40-filename');
        rd40FileName.textContent = replaceFileExtension(file.name);
      };
      
      img.src = event.target.result;
    };
    
    reader.readAsDataURL(file);
  };
  
  input.click();
}

// Loads an image from the server, resizes it to 110x110 pixels, and stores it in the global variable "originalPixels".
async function loadImg(filePath) {
  // Fetch the image file from the server.
  const response = await fetch('/downloadfile?filename=' + filePath);
  const blob = await response.blob();

  // Create an image object and load the image data into it.
  const img = new Image();
  const imgLoaded = new Promise(resolve => {
    img.onload = () => resolve();
  });
  img.src = URL.createObjectURL(blob);
  await imgLoaded;

  // Create a canvas element and resize the image to 110x110 pixels.
  const canvas = document.createElement('canvas');
  canvas.width = 110;
  canvas.height = 110;
  const ctx = canvas.getContext('2d');
  ctx.drawImage(img, 0, 0, 110, 110);

  // Get the pixel buffer of the resized image and store it in the global variable.
  const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
  originalPixels = new Uint8ClampedArray(imageData.data);
  currentPixels = new Uint8ClampedArray(originalPixels); // Make a copy of the pixel buffer for display.

  // retrieve filename and display
  rd40FileName = document.getElementById('rd40-filename');
  const newRd40FileName = replaceFileExtension(getFileNameFromUrl(filePath));
  rd40FileName.textContent = newRd40FileName;
}

// Manipulates the current pixel buffer of the displayed image using the specified threshold.
function manipulate(threshold) {
    // Loop over the pixels and set each pixel to white or black based on its brightness.
    console.log("currentPixels.length: " + currentPixels.length);
    for (let i = 0; i < originalPixels.length; i += 4) {
      const r = originalPixels[i];
      const g = originalPixels[i + 1];
      const b = originalPixels[i + 2];
  
      // Compute the brightness of the pixel.
      const brightness = (r + g + b) / 3;
  
      // Set the pixel to white if its brightness is above the threshold, and black otherwise.
      if ((brightness > threshold) ^ invert) {
        currentPixels[i] = 255;
        currentPixels[i + 1] = 255;
        currentPixels[i + 2] = 255;
        currentPixels[i + 3] = 255;
      } else {
        currentPixels[i] = 0;
        currentPixels[i + 1] = 0;
        currentPixels[i + 2] = 0;
        currentPixels[i + 3] = 255;
      }
    }
  
    // Display the modified image in the HTML element.
    displayRD40('image-rd40', currentPixels);
}

// Displays the RD40 image in an HTML element.
function displayRD40(elementId, pixels) {
    // Create a canvas element and set its dimensions to match the image size.
    const canvas = document.createElement('canvas');
    canvas.width = 110;
    canvas.height = 110;
  
    // Draw the image onto the canvas.
    const ctx = canvas.getContext('2d');
    const imageData = ctx.createImageData(canvas.width, canvas.height);
    imageData.data.set(pixels);
    ctx.putImageData(imageData, 0, 0);
  
    // Set the source of the <img> element to the data URL of the canvas.
    const dataUrl = canvas.toDataURL();
    const imgElement = document.getElementById(elementId);
    imgElement.src = dataUrl;
    imgElement.style.display = 'block';
}

// handle click on rd40 image
function handleClickRd40() {
  displayRD40('image-rd40', originalPixels);      // call displayRD40 with originalPixels

  setTimeout(function() {
    displayRD40('image-rd40', currentPixels);     // call displayRD40 with currentPixels after 1 second
  }, 1000);                                       // 1000 ms = 1 s
}
  


// create RD40 data structure and upload file to server
function saveRD40(fileName) {
    // Create an array to hold the RD40 data
    const rd40Data = new Uint8Array(1540);
  
    // Loop through each column of the image
    for (let col = 0; col < 110; col++) {
      // Loop through each row of the column
      for (let row = 0; row < 110; row++) {
        // Calculate the index in the currentPixels array
        const pixelIndex = col + (109-row) * 110; 
  
        // Get the pixel value (0 or 1) from currentPixels
        const pixel = currentPixels[pixelIndex*4];
  
        // Calculate the byte index and bit position within the byte
        const byteIndex = col*14 + Math.floor(row / 8);
        const bitPosition = row % 8;

        // Set the bit in the rd40Data array based on the pixel value
        if (pixel !== 0) {
          rd40Data[byteIndex] |= (1 << bitPosition);
        } else {
          rd40Data[byteIndex] &= ~(1 << bitPosition);
        }
      }
    }
    
    // Create a new FormData object to send the file as a binary payload
    const formData = new FormData();
    formData.append('file', new Blob([rd40Data], { type: 'application/octet-stream' }), fileName);
  
    // Send an HTTP POST request to the server to upload the file
    fetch(`/uploadfile?path=${encodeURIComponent(fileName)}`, {
      method: 'POST',
      body: formData,
    })
    .then(response => {
      if (response.ok) {
        console.log('File uploaded successfully!');
        // Reload the file list display to reflect the uploaded file
        loadFileList();
      } else {
        console.error('Failed to upload file!');
      }
    })
    .catch(error => {
      console.error('Error occurred while uploading file:', error);
    });
}


function getRD40Names() {
  fetch('/getRD40Names')
    .then((response) => {
      if (response.ok) {
        return response.json();
      } else {
        throw new Error('Fehler bei der Anfrage. Status: ' + response.status);
      }
    })
    .then((data) => {
      if (data.clockface!="") document.getElementById("rd40-clock-face").textContent = getFileNameFromUrl(data.clockface);
      else document.getElementById("rd40-clock-face").textContent = "default";
      document.getElementById("rd40-logo").textContent = getFileNameFromUrl(data.logo);
      document.getElementById("rd40-image").textContent = getFileNameFromUrl(data.image);
    })
    .catch((error) => {
      console.error('Fehler bei der Anfrage:', error);
    });
}

// Extract the file name from a given URL
function getFileNameFromUrl(url) {
  // Splits the URL by "/" and returns the last element of the resulting array, which is the filename
  var parts = url.split("/");
  return parts[parts.length - 1];
}

// Create a canvas element and resize the image to 110x110 pixels.
const canvasPreview = document.createElement('canvas');
canvasPreview.width = 110;
canvasPreview.height = 110;
const ctx = canvasPreview.getContext('2d');

// Get the pixel buffer of the resized image and store it in the global variable.
const imageDataPreview = ctx.getImageData(0, 0, canvasPreview.width, canvasPreview.height);
previewPixels = new Uint8ClampedArray(imageDataPreview.data);
originalPixels = new Uint8ClampedArray(imageDataPreview.data);
currentPixels = new Uint8ClampedArray(imageDataPreview.data);

// Event listener for when the DOM is fully loaded
document.addEventListener('DOMContentLoaded', () => {
    console.log('Document loaded.');
    loadFileList();
    getRD40Names();
});

// Add an event listener to the threshold slider.
document.getElementById('thresholdSlider').addEventListener('input', event => {
    // Get the current value of the slider.
    const threshold = parseInt(event.target.value) * 2.55;
    // Manipulate the displayed image.
    manipulate(threshold);
});

// Event listener for load button
document.getElementById("load-img").onclick = () => {
    if (selectedFilePath !== "") {
        loadImg(selectedFilePath)
            .then(() => {
                manipulate(2.55 * parseInt(document.getElementById('thresholdSlider').value));
            })
            .catch(error => {
                console.error(error);
            });
    } else {
      handleFileSelect();
    }
    clearSelectedFilePath();
};

// Event listener for "save-rd40" button
document.getElementById("save-rd40").onclick = () => {
    console.log("rd40FileName: " + rd40FileName.textContent);
    console.log("currentPath: " + currentPath);
    console.log("path: " + currentPath + rd40FileName.textContent);    
    saveRD40(rd40FileName.textContent);
};
  
// Event listener for done button
document.getElementById("fm-done").onclick = () => {
  window.location.href = "/configDone";
};

// Event listener for invert button
document.getElementById("invert").onclick = () => {
    invert = !invert;
    manipulate(2.55*parseInt(document.getElementById('thresholdSlider').value));
};  

// add click handler for image-rd40
document.getElementById('image-rd40').addEventListener('click', handleClickRd40);

// Event listener for "select clock face" button
document.getElementById("select-clock-face").onclick = () => {
  if (selectedFilePath !== "") {
        let rememberSelectedFilePath = selectedFilePath;
        // Send an HTTP POST request to the server to delete the specified file
        fetch(`/selectclockface?filename=${selectedFilePath}`)
        .then((response) => {
          if (response.ok) {
            document.getElementById("rd40-clock-face").textContent = getFileNameFromUrl(rememberSelectedFilePath);
            console.log("rememberSelectedFilePath: " + getFileNameFromUrl(rememberSelectedFilePath));
          } else {
            document.getElementById("rd40-clock-face").textContent = "default";
            console.log("Fehler bei der Anfrage. Status: " + response.status);
          }
        })
        .catch((error) => {
          // Hier kannst du Fehlerbehandlung durchführen
          console.log("Fehler bei der Anfrage:" + error);
        });
  } else {
    alert("Click and hold to select a .rd40 file.");
  }
  clearSelectedFilePath();
};

// Delete a file without confirmation
function deleteFile(filename) {
    // Send an HTTP POST request to the server to delete the specified file
    fetch(`/deletefile?filename=${filename}`)
    .then(response => response.json())
    .then(data => {
        console.log('File deleted:', filename);
        // Reload the file list display to reflect the deleted file
        loadFileList();
    })
    .catch(error => {
        console.error('Error deleting file:', error);
    });
}

// Event listener for "clear clock face" button
document.getElementById("clr-clock-face").onclick = () => {
  
  deleteFile("/variables/clockFacePath.txt");
  document.getElementById("rd40-clock-face").textContent = "default";

};

// Event listener for "select logo" button
document.getElementById("select-logo").onclick = () => {
  if (selectedFilePath !== "") {
        let rememberSelectedFilePath = selectedFilePath;
        // Send an HTTP POST request to the server to delete the specified file
        fetch(`/selectlogo?filename=${selectedFilePath}`)
        .then((response) => {
          if (response.ok) {
            document.getElementById("rd40-logo").textContent = getFileNameFromUrl(rememberSelectedFilePath);
            console.log("rememberSelectedFilePath: " + getFileNameFromUrl(rememberSelectedFilePath));
          } else {
            console.log("Fehler bei der Anfrage. Status: " + response.status);
          }
        })
        .catch((error) => {
          // Hier kannst du Fehlerbehandlung durchführen
          console.log("Fehler bei der Anfrage:" + error);
        });
  } else {
    alert("Click and hold to select a .rd40 file.");
  }
  clearSelectedFilePath();
};

// Event listener for "select image" button
document.getElementById("select-image").onclick = () => {
  if (selectedFilePath !== "") {
        let rememberSelectedFilePath = selectedFilePath;
        // Send an HTTP POST request to the server to delete the specified file
        fetch(`/selectimage?filename=${selectedFilePath}`)
        .then((response) => {
          if (response.ok) {
            document.getElementById("rd40-image").textContent = getFileNameFromUrl(rememberSelectedFilePath);
            console.log("rememberSelectedFilePath: " + getFileNameFromUrl(rememberSelectedFilePath));
          } else {
            console.log("Fehler bei der Anfrage. Status: " + response.status);
          }
        })
        .catch((error) => {
          // Hier kannst du Fehlerbehandlung durchführen
          console.log("Fehler bei der Anfrage:" + error);
        });
  } else {
    alert("Click and hold to select a .rd40 file.");
  }
  clearSelectedFilePath();
};

// Event listener for mousedown event on file list
fileList.onmousedown = (event) => {
  if (event.target !== fileList && !isTouchDevice && !clickHandled) {
    mousedownTime = new Date().getTime();
    clickHandled = true;

    longClickTimeout = setTimeout(() => {
      handleLongClick(event);
    }, 500);
  }
};

// Event listener for mouseup event on file list
fileList.onmouseup = (event) => {
  if (event.target !== fileList && !isTouchDevice && clickHandled) {
    clearTimeout(longClickTimeout);

    mouseupTime = new Date().getTime();
    const timeDifference = mouseupTime - mousedownTime;

    if (timeDifference < 500) {
      handleShortClick(event);
    }

    // Wartezeit von 500 ms, bevor weitere Klicks akzeptiert werden
    setTimeout(() => {
      clickHandled = false;
    }, 500);
  }
};

// Event listener for touchstart event on file list
fileList.addEventListener('touchstart', (event) => {
  if (event.target !== fileList && !clickHandled) {
    isTouchDevice = true;
    touchStartX = event.touches[0].clientX;
    touchStartY = event.touches[0].clientY;
    touchMoved = false;
    clickHandled = true;

    mousedownTime = new Date().getTime();

    longClickTimeout = setTimeout(() => {
      handleLongClick(event);
    }, 500);
  }
});

// Event listener for touchmove event on file list
fileList.addEventListener('touchmove', (event) => {
  if (event.target !== fileList) {
    const touchEndX = event.touches[0].clientX;
    const touchEndY = event.touches[0].clientY;
    const touchDistance = Math.sqrt(
      Math.pow(touchEndX - touchStartX, 2) + Math.pow(touchEndY - touchStartY, 2)
    );

    if (touchDistance > 10) {
      touchMoved = true;
      clearTimeout(longClickTimeout);
    }
  }
});

// Event listener for touchend event on file list
fileList.addEventListener('touchend', (event) => {
  if (event.target !== fileList && clickHandled) {
    clearTimeout(longClickTimeout);

    mouseupTime = new Date().getTime();
    const timeDifference = mouseupTime - mousedownTime;

    if (!touchMoved && timeDifference < 500) {
      handleShortClick(event);
    }

    // Wartezeit von 500 ms, bevor weitere Klicks akzeptiert werden
    setTimeout(() => {
      clickHandled = false;
    }, 500);
  }

  touchMoved = false;
});

// Check if it is a touch-enabled device
if ('ontouchstart' in window || navigator.msMaxTouchPoints) {
    scrollDirection = -1; // Reverse scroll direction for touch-enabled devices
}  
  
// Handle short click event on file item
function handleShortClick(event) {
  fileItems.forEach(item => {
    item.classList.remove("selected");
  });

  event.target.classList.add("selected");
  const fileName = event.target.textContent;
  
  if (fileName.indexOf('.') === -1) {
    currentPath += fileName + "/";
    console.log("Current path: " + currentPath);
    loadFileList();
  }

  previewFile(currentPath + fileName);
}

// Handle long click event on file item
function handleLongClick(event) {
  selectedFile = event.target.textContent;
  if (selectedFile.indexOf("..") !== -1) {
    // Do nothing
  } else if (selectedFile.indexOf(".") !== -1) {
    selectedFilePath = currentPath + selectedFile;
    document.getElementById("text-selected-file").value = "Selected file: " + selectedFilePath;
    previewFile(selectedFilePath);
  } else {
    selectedFilePath = currentPath + selectedFile +"/";
    document.getElementById("text-selected-file").value = "Selected folder: " + selectedFilePath;
  }
}

