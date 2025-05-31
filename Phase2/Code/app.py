#Hala Almutairi 2211112873
#Zaharaa Alrashidi 2191118389
#KAYO Robotics Team
sys.path.insert(0,'/home/hala/path/to/venv/lib/python3.11/site-packages')
import serial
from flask import Flask, render_template, jsonify
from threading import Thread, Lock
import time
import random
import numpy as np
from collections import deque
import torch
import torch.nn as nn
import torch.optim as optim
import torch.nn.functional as F
import os

# Initialize Flask app
app = Flask(__name__)

# Sensor data
sensor_data = {
    'IR_R': 0,
    'IR_M': 0,
    'IR_L': 0,
    'training_sessions': 0,
    'current_reward': 0
}
sensor_lock = Lock()

# Training control
training_thread = None
training_active = False
training_lock = Lock()
paused = False 
stop_session = False

# Q-Learning constants
MAX_MEMORY = 100_000
BATCH_SIZE = 1000
LR = 0.001

# Action definitions
ACTIONS = ['forward', 'left', 'right', 'scan_left', 'scan_right','strongLeft','strongRight']
ACTION_MAP = {
    'forward': 'F',
    'left': 'L',
    'right': 'R',
    'strongLeft':'SL',
    'strongRight':'SR',
    'scan_left': 'SCAN_LEFT',
    'scan_right': 'SCAN_RIGHT',
    'stop': 'S',
    'back': 'B',
    'M': 'M',
    'X': 'X',
}

# Neural Network
class Linear_QNet(nn.Module):
    def __init__(self, input_size, hidden_size, output_size):
        super().__init__()
        self.linear1 = nn.Linear(input_size, hidden_size)
        self.linear2 = nn.Linear(hidden_size, output_size)
   
    def forward(self, x):
        x = F.relu(self.linear1(x))
        x = self.linear2(x)
        return x
   
    def save(self, file_name='model.pth'):
        model_folder_path = '/home/hala/WebServer'
        file_name = os.path.join(model_folder_path, file_name)
        torch.save(self.state_dict(), file_name)

class QTrainer:
    def __init__(self, model, lr, gamma):
        self.lr = lr
        self.gamma = 0.9
        self.model = model
        self.optimer = optim.Adam(model.parameters(), lr=self.lr)    
        self.criterion = nn.MSELoss()
            
    def train_step(self, state, action, reward, next_state, done):
        state = torch.tensor(state, dtype=torch.float)
        next_state = torch.tensor(next_state, dtype=torch.float)
        action = torch.tensor(action, dtype=torch.long)
        reward = torch.tensor(reward, dtype=torch.float)

        if len(state.shape) == 1:
            state = torch.unsqueeze(state, 0)
            next_state = torch.unsqueeze(next_state, 0)
            action = torch.unsqueeze(action, 0)
            reward = torch.unsqueeze(reward, 0)
            done = (done, )
            
        pred = self.model(state)
        target = pred.clone()
        for idx in range(len(done)):
            Q_new = reward[idx]
            if not done[idx]:
                Q_new = reward[idx] + self.gamma * torch.max(self.model(next_state[idx]))
            target[idx][torch.argmax(action[idx]).item()] = Q_new
       
        self.optimer.zero_grad()
        loss = self.criterion(target, pred)
        loss.backward()
        self.optimer.step()

class Agent:
    def __init__(self):
        self.n_games = 0
        self.epsilon = 0
        self.gamma = 0.9
        self.memory = deque(maxlen=MAX_MEMORY)
        self.state_memory = deque(maxlen=3)
        self.model = Linear_QNet(3, 256, len(ACTIONS))
        self.trainer = QTrainer(self.model, lr=LR, gamma=self.gamma)
        self._safe_load_model()

    def _safe_load_model(self, file_name='model.pth'):
        model_folder_path = '/home/hala/WebServer'
        file_name = os.path.join(model_folder_path, file_name)
        if not os.path.exists(file_name):
            print("No saved model found. Starting fresh.")
            self.n_games = 0
            self.epsilon = 0
            return
        try:
            checkpoint = torch.load(file_name)
            self.model.load_state_dict(checkpoint['model_state_dict'])
            self.trainer.optimer.load_state_dict(checkpoint['optimizer_state_dict'])
            self.n_games = checkpoint.get('n_games', 0)
            self.epsilon = checkpoint.get('epsilon', 0)
            print(f"Loaded model with {self.n_games} games")
        except Exception as e:
            print(f"Error loading model: {e}")

    def save_model(self, file_name='model.pth'):
        model_folder_path = '/home/hala/WebServer'
        os.makedirs(model_folder_path, exist_ok=True)
        file_name = os.path.join(model_folder_path, file_name)
        torch.save({
            'model_state_dict': self.model.state_dict(),
            'optimizer_state_dict': self.trainer.optimer.state_dict(),
            'n_games': self.n_games,
            'epsilon': self.epsilon,
        }, file_name)
        print(f"Saved model with {self.n_games} games")

    def get_state(self):
        with sensor_lock:
            raw_state = np.array([sensor_data['IR_L'], sensor_data['IR_M'], sensor_data['IR_R']])
            self.state_memory.append(raw_state)
            if len(self.state_memory) >= 3:
                filtered_state = np.round(np.mean(self.state_memory, axis=0)).astype(int)
            else:
                filtered_state = raw_state
            return filtered_state

    def get_action(self, state):
        self.epsilon = max(20, 80 - self.n_games)
        if random.randint(0, 100) < self.epsilon:
            move = random.randint(0, len(ACTIONS) - 1)
        else:
            state_tensor = torch.tensor(state, dtype=torch.float)
            prediction = self.model(state_tensor)
            move = torch.argmax(prediction).item()
        final_move = [0] * len(ACTIONS)
        final_move[move] = 1
        return final_move

    def remember(self, state, action, reward, next_state, done):
        self.memory.append((state, action, reward, next_state, done))

    def train_short_memory(self, state, action, reward, next_state, done):
        self.trainer.train_step(state, action, reward, next_state, done)

    def train_long_memory(self):
        if len(self.memory) > BATCH_SIZE:
            mini_sample = random.sample(self.memory, BATCH_SIZE)
        else:
            mini_sample = self.memory
        states, actions, rewards, next_states, dones = zip(*mini_sample)
        self.trainer.train_step(states, actions, rewards, next_states, dones)

# Initialize agent
agent = Agent()

# Arduino setup
try:
    arduino = serial.Serial('/dev/ttyACM0', 9600, timeout=0.1)
    arduino.flush()
    print("Arduino connected successfully")
except serial.SerialException as e:
    print(f"Failed to connect to Arduino: {e}")
    arduino = None

def update_sensor_data(line):
    try:
        if not line or ':' not in line:
            return False
        key, value = line.split(':', 1)
        value = int(value.strip())
        with sensor_lock:
            if key in sensor_data:
                sensor_data[key] = value
        return True
    except ValueError:
        return False

def read_serial_data():
    while True:
        try:
            if arduino and arduino.in_waiting > 0:
                try:
                    line = arduino.readline().decode('utf-8').strip()
                except UnicodeDecodeError:
                    line = arduino.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    update_sensor_data(line)
            time.sleep(0.001)
        except Exception as e:
            print(f"Serial read error: {e}")
            time.sleep(1)

def calculate_reward(old_state, action, new_state):
    reward = 0
    L, M, R = new_state

    # 1) All white – lost the path
    if (L, M, R) == (0, 0, 0):
        if action == 'forward':
            reward += 50
        else:
            reward -= 25

    # 2) Drifted left – only left sensor sees black
    elif (L, M, R) == (1, 0, 0):
        if action == 'right':
            reward += 10
        elif action == 'left':
            reward -= 10

    # 3) Drifted right – only right sensor sees black
    elif (L, M, R) == (0, 0, 1):
        if action == 'left':
            reward += 10
        elif action == 'right':
            reward -= 10

    # 4) Centered on the line – middle sensor sees black
    elif (L, M, R) == (0, 1, 0):
        if action in ('scan_left', 'scan_right'):
            reward += 8
        elif action == 'forward':
            reward -= 20

    # 5) Too far left – left + middle sensors see black
    elif (L, M, R) == (1, 1, 0):
        if action == 'strongRight':
            reward += 15
        elif action == 'strongLeft':
            reward -= 20

    # 6) Too far right – middle + right sensors see black
    elif (L, M, R) == (0, 1, 1):
        if action == 'strongLeft':
            reward += 15
        elif action == 'strongRight':
            reward -= 20

    # 7) Both edges black (intersection) – go straight through
    elif (L, M, R) == (1, 0, 1):
        if action == 'forward':
            reward += 12
        else:
            reward -= 20

    # 8) All black – completely off-track
    elif (L, M, R) == (1, 1, 1):
        if action in ('scan_left', 'scan_right'):
            reward += 10
        else:
            reward -= 10

    return reward



def train():
    global agent, training_active, paused, stop_session
    print("Training thread started")
    total_reward = 0
    iteration = 0
    
    try:
        while training_active:
            iteration += 1
            print(f"Training iteration {iteration}, Sessions: {agent.n_games}")
            
            if stop_session:
                print("Session stopped and discarded.")
                stop_session = False
                total_reward = 0
                continue
                
            state_old = agent.get_state()
            
            if paused:
                # Get current action before processing pause
                final_move = agent.get_action(state_old)
                action_idx = np.argmax(final_move)
                action = ACTIONS[action_idx]
                
                print(f"Pausing session. Current n_games: {agent.n_games}")
                reward = -40
                next_state = agent.get_state()
                
                agent.train_short_memory(state_old, final_move, reward, next_state, True)
                agent.remember(state_old, final_move, reward, next_state, True)
                
                total_reward += reward
                agent.n_games += 1
                agent.train_long_memory()
                
                with sensor_lock:
                    sensor_data.update({
                        'training_sessions': agent.n_games,
                        'current_reward': total_reward   
                    })
                
                total_reward = 0
                training_active = False
                print("Training deactivated (paused)")
                paused = False
                agent.save_model()
                continue

            # Normal training
            final_move = agent.get_action(state_old)
            action_idx = np.argmax(final_move)
            action = ACTIONS[action_idx]
            
            if arduino:
                try:
                    arduino.write((ACTION_MAP[action] + '\n').encode())
                except Exception as e:
                    print(f"Error sending command: {e}")
            
            time.sleep(0.1)
            state_new = agent.get_state()
            reward = calculate_reward(state_old, action, state_new)
            total_reward += reward
            
            agent.train_short_memory(state_old, final_move, reward, state_new, False)
            agent.remember(state_old, final_move, reward, state_new, False)
            
            with sensor_lock:
                sensor_data.update({
                    'training_sessions': agent.n_games,
                    'current_reward': total_reward,   
                })
            
            if not training_active:
                print("Training deactivated")
                break
                
    except Exception as e:
        print(f"Exception in training loop: {e}")
        import traceback
        traceback.print_exc()



# Flask routes
@app.route('/')
def index():
    return render_template('index.html')

@app.route('/get_sensors')
def get_sensors():
    with sensor_lock:
        return jsonify(sensor_data)

@app.route('/send_command/<cmd>')
def send_command(cmd):
    if arduino and cmd in ACTION_MAP:
        arduino.write((ACTION_MAP[cmd] + '\n').encode())
        return jsonify({'status': 'success'})
    return jsonify({'status': 'error'})

@app.route('/pause_training')
def pause_training():
    global paused
    print("Pause request received")
    paused = True
    if arduino:
        try:
            arduino.write((ACTION_MAP['stop'] + '\n').encode())
        except Exception as e:
            print(f"Error sending stop command: {e}")
    return jsonify({
        'status': 'success',
        'message': 'Training paused',
        'n_games': agent.n_games
    })

@app.route('/stop_training')
def stop_session_route():
    global stop_session, training_active
    stop_session = True
    training_active = False
    if arduino:
        try:
            arduino.write((ACTION_MAP['stop'] + '\n').encode())
        except Exception as e:
            print(f"Error sending stop command: {e}")
    return jsonify({
        'status': 'success',
        'message': 'Session stopped',
        'n_games': agent.n_games
    })

@app.route('/start_training')
def start_training():
    global training_thread, training_active, paused
    paused = False
    training_active = True
    
    if training_thread and training_thread.is_alive():
        return jsonify({'status': 'error', 'message': 'Training already running'})
    
    training_thread = Thread(target=train)
    training_thread.daemon = True
    training_thread.start()
    
    return jsonify({
        'status': 'success',
        'message': 'Training started',
        'n_games': agent.n_games
    })

if __name__ == '__main__':
    if arduino:
        serial_thread = Thread(target=read_serial_data)
        serial_thread.daemon = True
        serial_thread.start()
    # Run Website 
    app.run(debug=True, use_reloader=False, port=5000,
host='0.0.0.0',threaded=True)