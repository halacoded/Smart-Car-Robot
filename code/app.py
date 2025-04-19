
#Hala Almutairi 2211112873
#Zaharaa Alrashidi 2191118389
#KAYO Robotics Team
import serial #connecting to Arduino via serial
from flask import Flask, render_template, jsonify #creating web application ---> FLAsK
from threading import Thread, Lock #running serial reading in a separate thread --> threading
import time #delays pk

# Initialize flask
app = Flask(__name__)

#Initialize sensor
sensor_data = {
    'ultrasonic': {'middle': 0, 'right': 0, 'left': 0},
    'ir_right': {'right': 0, 'middle': 0, 'left': 0},
    'ir_left': {'right': 0, 'middle': 0, 'left': 0}
}
sensor_lock = Lock()

# ---------------------------------Check Arduino connection-------------------------------
try:
    arduino = serial.Serial('/dev/ttyACM0', 9600, timeout=0.1) #connecting to Arduino via serial
    arduino.flush()
except serial.SerialException as e: #if connection fail ---> Arduino = none
    print(f"Failed to connect to Arduino: {e}")
    arduino = None 
    
#----------------------------------------Functions---------------------------------------
#Take Arduino serial data and update the sensor_data dictionary"
def update_sensor_data(line):
    
    try:
        if not line or ':' not in line:
            return False
        
        key, value = line.split(':', 1) #split line into (key-value)
        value = int(value) #convert String to int
        with sensor_lock:  #update sensors value based on key
            if key == 'US_M':
                sensor_data['ultrasonic']['middle'] = value
            elif key == 'US_R':
                sensor_data['ultrasonic']['right'] = value
            elif key == 'US_L':
                sensor_data['ultrasonic']['left'] = value
            elif key == 'IR_R_R':
                sensor_data['ir_right']['right'] = value
            elif key == 'IR_R_M':
                sensor_data['ir_right']['middle'] = value
            elif key == 'IR_R_L':
                sensor_data['ir_right']['left'] = value
            elif key == 'IR_L_R':
                sensor_data['ir_left']['right'] = value
            elif key == 'IR_L_M':
                sensor_data['ir_left']['middle'] = value
            elif key == 'IR_L_L':
                sensor_data['ir_left']['left'] = value
            else:
                return False #if it isnt sensor data ---> false
        return True
    except ValueError: #if converting String to int failed ---> handel error
        return False

#continuously reads Arduino serial data 
def read_serial_data():
    while True:
        if arduino and arduino.in_waiting > 0:
            try:
                line = arduino.readline().decode('utf-8', errors='ignore').strip() #read from serial
                if line: #check if there is reading
                    update_sensor_data(line)  #yes -->  update sensors with new data
            except Exception as e: #no ---> handel error
                print(f"Serial read error: {e}")
        time.sleep(0.01)

#----------------------------------Flask Website Endpoints------------------------------------------
#endpoint to get current (now) sensor data
@app.route('/get_sensors')
def get_sensors():
    with sensor_lock:
        return jsonify(sensor_data)
    
#endpoint to send commands to Arduino
@app.route('/send_command/<cmd>')
def send_command(cmd):
    if arduino is None:
        return jsonify({'status': 'error', 'message': 'Arduino not connected'}), 500
    command_map = {
        'forward': 'f',
        'back': 'b',
        'left': 'l',
        'right': 'r',
        'stop': 's',
        'remote mode': 'R',
        'stop mode': 'x',
        'slow':   'L',
        'medium': 'M',
        'fast':   'H'
    }
    if cmd in command_map:
        arduino.write(command_map[cmd].encode())  #send command to Arduino
    return jsonify({'status': 'success'})

#Main app page endpoint --> Render HTML file
@app.route('/')
def index():
    return render_template('index.html')
#----------------------------------start serial reading thread and run Flask--------------------------
if __name__ == '__main__':
    if arduino is not None:
        serial_thread = Thread(target=read_serial_data)
        serial_thread.daemon = True
        serial_thread.start()
    
    # Run Website 
    app.run(debug=True, use_reloader=False, port=5000,
host='0.0.0.0',threaded=True)

