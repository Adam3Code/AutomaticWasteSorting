from flask import Flask, request, jsonify
import os
import requests
from PIL import Image
import io
import json
from datetime import datetime

app = Flask(__name__)


UPLOAD_FOLDER = 'uploads'
CLASSIFICATION_RESULTS = {}
OLLAMA_API_URL = "http://localhost:11434/api/generate"  # Default Ollama API URL


os.makedirs(UPLOAD_FOLDER, exist_ok=True)

def classify_image(image_data):
    try:
        
        prompt = "Analyze this image and classify the waste as either 'food waste' or 'other waste'. Respond with only one of these two categories."
        #  request to Ollama
        response = requests.post(
            OLLAMA_API_URL,
            json={
                "model": "llama2-3.2-vision",
                "prompt": prompt,
                "images": [image_data]
            }
        )
        
        if response.status_code == 200:
            result = response.json()
            return result.get("response", "unknown")
        else:
            return "error"
    except Exception as e:
        print(f"Error in classification: {str(e)}")
        return "error"

@app.route('/upload', methods=['POST'])
def upload_image():
    """Endpoint for ESP32 to upload images"""
    if 'image' not in request.files:
        return jsonify({'error': 'No image provided'}), 400
    
    file = request.files['image']
    if file.filename == '':
        return jsonify({'error': 'No selected file'}), 400
    
    # Generate unique filename
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"{timestamp}_{file.filename}"
    filepath = os.path.join(UPLOAD_FOLDER, filename)
    
    # Save the image
    file.save(filepath)
    
    # Read the image for classification
    with open(filepath, 'rb') as img_file:
        image_data = img_file.read()
    
    # Classify the image
    classification = classify_image(image_data)
    
    # Store the result
    CLASSIFICATION_RESULTS[filename] = classification
    
    return jsonify({
        'message': 'Image uploaded and classified successfully',
        'filename': filename,
        'classification': classification
    })

@app.route('/classification/<filename>', methods=['GET'])
def get_classification(filename):
    """Endpoint for ESP32 to fetch classification results"""
    if filename in CLASSIFICATION_RESULTS:
        return jsonify({
            'filename': filename,
            'classification': CLASSIFICATION_RESULTS[filename]
        })
    else:
        return jsonify({'error': 'Classification not found'}), 404

@app.route('/status', methods=['GET'])
def status():
    """Health check endpoint"""
    return jsonify({'status': 'shawarma'})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True) 