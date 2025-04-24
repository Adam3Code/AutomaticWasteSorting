import ollama
import os

# Send the image content in the chat message
response = ollama.chat(
    model='llama3.2-vision',
    messages=[{
        'role': 'user',
        'content': 'Look at the image. If the object is food, reply with "Food". If it is not food, reply with "Non-Food". Only reply with one of those words.',
        'images': ['single-red-apple.jpg'] #prøv at converte til base64 senere
    }]
)

classification = response['message']['content'].strip() 
print(response['message']['content'].strip()) 

