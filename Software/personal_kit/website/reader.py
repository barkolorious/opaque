from flask import Flask, jsonify
from flask_cors import CORS
import threading
import serial
import time

app = Flask(__name__)
CORS(app)

ser = serial.Serial('COM4', 115200, timeout=1)  # Replace with your port
latest_data = {'data': '', 'timestamp': None}

def read_serial():
    global latest_data
    buffer = ''
    while True:
        line = ser.readline().decode(errors='ignore').strip()
        if line:
            print(f"Serial: {line}")
            latest_data['data'] = line
            latest_data['timestamp'] = time.time()

threading.Thread(target=read_serial, daemon=True).start()

@app.route('/latest')
def latest():
    return jsonify(latest_data)

app.run(host='0.0.0.0', port=5000)
