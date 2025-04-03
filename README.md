# Waste Classification Server

This server handles image uploads from an ESP32 camera, classifies the images using Ollama's Llama 3.2-vision model, and provides classification results back to the ESP32.

## Prerequisites
1. Install Docker and ensure it is open
2. Install Ollama (can be done on google)
## Setup
1. Run "ollama pull llama3.2-vision" (only run ones)
2. docker build -t automaticwastesorting .
3. docker run -p 5000:5000 automaticwastesorting


## API Endpoints
### Upload Image
- **URL**: `/upload`
- **Method**: POST
- **Content-Type**: multipart/form-data
- **Parameters**: 
  - `image`: The image file to upload
- **Response**: JSON with filename and classification result

### Get Classification
- **URL**: `/classification/<filename>`
- **Method**: GET
- **Response**: JSON with filename and classification result

### Status Check
- **URL**: `/status`
- **Method**: GET
- **Response**: JSON with server status

## ESP32 Integration

The ESP32 should:
1. Capture an image using the camera
2. Send a POST request to `/upload` with the image
3. Store the returned filename
4. Periodically check `/classification/<filename>` for the classification result

## Notes

- Images are stored in the `uploads` directory
- Classification results are stored in memory and will be lost if the server restarts
- Make sure Ollama is running on the default port (11434) 