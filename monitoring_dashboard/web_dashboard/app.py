import serial
import time
import json
from flask import Flask, render_template, Response
import serial_collector


app = Flask(__name__)


SERIAL_PORT = 'COM4' 
BAUD_RATE = 9600


# sensor data
latest_data = {
    "waterLevel": 0,
    "moistureLevel": 0,
    "earthquakeStatus": 0
}


@app.route('/')
def index():
    return render_template('index.html')


@app.route('/sensor-data')
def sensor_data():
    def generate():
        while True:
            # Try to read data from Arduino
            # read_arduino_data()
            data = serial_collector.collectData()
            if len(data) == 3:
                    latest_data["waterLevel"] = float(data[0])
                    latest_data["moistureLevel"] = float(data[1])
                    latest_data["earthquakeStatus"] = float(data[2])
            # Send the data to the client
            data_str = json.dumps(latest_data)
            
            yield f"data: {data_str}\n\n"
            
            # Wait before the next update
            time.sleep(2)
    
    return Response(generate(), mimetype='text/event-stream')


if __name__ == '__main__':

    app.run(debug=True, threaded=True)