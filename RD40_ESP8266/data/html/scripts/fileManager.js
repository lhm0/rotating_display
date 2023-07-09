// Define global variables
let currentPath = "/"; // current directory path
let fileList = document.getElementById("file-list"); // file list DOM element
let fileItems = []; // list of file item DOM elements

let scrollDirection = 1; // Standardmäßig positiv für normale Scrollrichtung
let touchStartX = 0; //                                                     
let touchStartY = 0; //                                                     
let mousedownTime = 0; // time when mouse button was pressed down           
let mouseupTime = 0; // time when mouse button was released                 
let clickHandled = false; //                                                
let longClickTimeout; // timeout ID for handling long click events          
let isTouchDevice = false; // flag to indicate if device is touch-enabled   
let touchMoved; //                                                          

let selectedFile = ""; // name of the selected file/folder
let selectedFilePath = ""; // path of the selected file/folder
let previewPixels; // preview pixels


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


// Delete a file or folder with the specified name
function deleteFile(filename) {
    // Show a confirmation dialog before deleting the file
    const confirmDeletion = confirm(`Delete '${filename}': Are you sure?`);
    if (confirmDeletion) {
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
}


// Rename a file or folder with the specified name
function renameFile(filename) {
    console.log("old path: " + filename);

    let isFolder;
    let folderName = "";
    let FileName = "";
    let originPath = "";
    let oldFileName = filename;
    let newFileName = "";
    if (filename.endsWith("/")) { // Check if it is a folder
        isFolder = true;
        var lastSlashIndex = filename.lastIndexOf("/");
        var secondLastSlashIndex = filename.lastIndexOf("/", lastSlashIndex - 1);

        folderName = filename.substring(secondLastSlashIndex + 1, lastSlashIndex);
        console.log("folderName = " + folderName);

        originPath = filename.substring(0, secondLastSlashIndex + 1);

        var newFolderName = prompt("Please rename folder:", folderName); // User input of new folder name
        newFileName = originPath + newFolderName + "/"; // Construct the new path with the new folder name
    } else { // If it's not a folder, it's a file
        isFolder = false;
        var lastSlashIndex = filename.lastIndexOf("/");
        fileName = filename.substring(lastSlashIndex + 1);
        originPath = filename.substring(0, lastSlashIndex + 1);

        var newName = prompt("Please rename file:", fileName); // User input of new file name
        newFileName = originPath + newName; // Construct the new path with the new file name
    }
    
    console.log("new path: " + newFileName);

    // Send an HTTP POST request to the server to rename the specified file
    fetch(`/renamefile?oldfilename=${oldFileName}&newfilename=${newFileName}`)
        .then(response => response.json())
        .then(data => {
            console.log('File renamed:', oldFileName, ' => ', newFileName);
            // Reload the file list display to reflect the renamed file
            loadFileList();
        })
        .catch(error => {
            console.error('Error renaming file:', error);
        });
}


// Copy a file or folder from the source path to the destination path
function copyFile(sourceFile, destPath) {
    // Send an HTTP POST request to the server to copy the specified file
    fetch(`/copyfile?source=${sourceFile}&destination=${destPath}&moveflag=0`)
        .then(response => response.json())
        .then(data => {
            console.log('file copied:', sourceFile);
            // Reload the file list display to reflect the copied file
            loadFileList();
        })
        .catch(error => {
            console.error('Error copying file:', error);
        });
}


// Move a file or folder from the source path to the destination path
function moveFile(sourceFile, destPath) {
    // Send an HTTP POST request to the server to move the specified file
    fetch(`/copyfile?source=${sourceFile}&destination=${destPath}&moveflag=1`)
        .then(response => response.json())
        .then(data => {
            console.log('file moved:', sourceFile);
            // Reload the file list display to reflect the moved file
            loadFileList();
        })
        .catch(error => {
            console.error('Error moving file:', error);
        });
}


// Create a new directory in the current path
function mkDir() {
    // Prompt user to enter directory name
    var directory = prompt("Please enter the directory name:", "");
    if (directory == null || directory == "") {
        // Cancel if user didn't enter anything
        return;
    } else {
        // Trim any leading or trailing whitespace from the input
        directory = directory.trim();
        // Construct the full path for the new directory
        directory = currentPath + directory + "/";
        // Send an HTTP POST request to the server to create the new directory
        fetch(`/mkdir?filename=${directory}`)
            .then(response => response.json())
            .then(data => {
                console.log('Directory created:', directory);
                // Reload the file list display to reflect the new directory
                loadFileList();
            })
            .catch(error => {
                console.error('Error creating directory:', error);
            });
    }
}

// Upload a file to the server
function uploadFile() {
    // Create a new input element for selecting a file
    const fileInput = document.createElement('input');
    fileInput.type = 'file';
    fileInput.accept = 'image/*';

    let totalSpace = 0;
    let usedSpace = 0;

    // Determine free disk space
    fetch('/diskspace')
        .then(response => response.json())
        .then(data => {
            const { totalBytes, usedBytes } = data;
            console.log("Total space: " + totalBytes + "   Used space: " + usedBytes);
            totalSpace = totalBytes;
            usedSpace = usedBytes;
        });

    // Add an event listener to the input element to handle file selection
    fileInput.addEventListener('change', (event) => {
        const file = event.target.files[0];
        const fileSize = file.size;

        // Check if there is enough disk space to upload the file
        if (fileSize > (totalSpace - usedSpace)) {
            console.log("ERROR: Not enough disk space");
            openInfoBox(totalSpace, usedSpace, fileSize);
        } else {
            // Create a new FormData object to send the file as a binary payload
            const formData = new FormData();
            formData.append('file', file);

            // Send an HTTP POST request to the server to upload the file
            fetch(`/uploadfile?path=${encodeURIComponent(currentPath)}`, {
                method: 'POST',
                body: formData,
            })
            .then(response => {
                console.log('File uploaded:', file.name);
                // Reload the file list display to reflect the uploaded file
                loadFileList();
            })
            .catch(error => {
                console.error('Error uploading file:', error);
            });
        }
    });

    // Click the file input element to open the file selection dialog
    fileInput.click();
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

// Open a dialog box showing information about the current directory
function openInfoBox(totalBytes, usedBytes, fileSize) {
    const message = `Not enough flash memory\n\ntotal: ${totalBytes} bytes\nfree: ${(totalBytes-usedBytes)} bytes\nFile: ${fileSize} bytes`;
    alert(message);
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

    // If the file is an image, download it and display it in an <img> element.
    if (/\.(gif|jpe?g|tiff?|png|webp|bmp)$/i.test(fileName)) {
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
}

// Clear the currently selected file path
function clearSelectedFilePath() {
  selectedFilePath = "";
  document.getElementById("text-selected-file").value = selectedFilePath;
  document.getElementById("image-preview").style.display = "none";
  document.getElementsByClassName("text4")[0].style.display = "none";
  document.getElementById('fileSize').textContent = "";
}

// Create a canvas element and resize the image to 110x110 pixels.
const canvasPreview = document.createElement('canvas');
canvasPreview.width = 110;
canvasPreview.height = 110;
const ctx = canvasPreview.getContext('2d');

// Get the pixel buffer of the resized image and store it in the global variable.
const imageDataPreview = ctx.getImageData(0, 0, canvasPreview.width, canvasPreview.height);
previewPixels = new Uint8ClampedArray(imageDataPreview.data);

// Event listener for when the DOM is fully loaded
document.addEventListener('DOMContentLoaded', () => {
  console.log('Document loaded.');
  loadFileList();
});

// Event listener for upload file button
document.getElementById('upload-file').addEventListener('click', () => {
  uploadFile();
});

// Event listener for download file button
document.getElementById('download-file').addEventListener('click', () => {
  console.log("selectedFilePath = " + selectedFilePath);
  if (selectedFilePath !== "") {
    if (selectedFilePath.endsWith("/")) {
      alert("Can only download files, not folders.");
    } else {
      downloadFile(selectedFilePath);
    }
  } else {
    alert("Click and hold to select a file.");
  }
  clearSelectedFilePath();
});

// Event listener for delete button
document.getElementById("delete").onclick = () => {
  if (selectedFilePath !== "") {
    deleteFile(selectedFilePath);
  } else {
    alert("Click and hold to select a file or folder.");
  }
  clearSelectedFilePath();
};
  
// Event listener for rename button
document.getElementById("rename").onclick = () => {
  if (selectedFilePath !== "") {
    renameFile(selectedFilePath);
  } else {
    alert("Click and hold to select a file or folder.");
  }
  clearSelectedFilePath();
};
  
// Event listener for copy button
document.getElementById("copy").onclick = () => {
  if (selectedFilePath !== "") {
    copyFile(selectedFilePath, currentPath);
  } else {
    alert("Click and hold to select a file or folder.");
  }
  clearSelectedFilePath();
};
  
// Event listener for move button
document.getElementById("move").onclick = () => {
  if (selectedFilePath !== "") {
    moveFile(selectedFilePath, currentPath);
  } else {
    alert("Click and hold to select a file or folder.");
  }
  clearSelectedFilePath();
};
  
// Event listener for make directory button
document.getElementById("make-dir").onclick = () => {
  mkDir();
};
  
// Event listener for file manager done button
document.getElementById("fm-done").onclick = () => {
  window.location.href = "/configDone";
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
