import React, { useState } from "react";
import "./App.css";

function App() {
  const [device, setDevice] = useState(null);
  const [characteristic, setCharacteristic] = useState(null);

  const connectBluetooth = async () => {
    try {
      const btDevice = await navigator.bluetooth.requestDevice({
        acceptAllDevices: true,
        optionalServices: ["0000ffe0-0000-1000-8000-00805f9b34fb"], // service UUID
      });
      const server = await btDevice.gatt.connect();
      const service = await server.getPrimaryService(
        "0000ffe0-0000-1000-8000-00805f9b34fb" // service UUID
      );
      const btCharacteristic = await service.getCharacteristic(
        "0000ffe1-0000-1000-8000-00805f9b34fb" // characteristic UUID
      );
      setDevice(btDevice);
      setCharacteristic(btCharacteristic);
      console.log("Bluetooth connected");
      console.log(device);
    } catch (error) {
      console.error("Bluetooth connection failed:", error);
    }
  };

  const sendCommand = async (command) => {
    if (!characteristic) {
      alert("Please connect to Bluetooth first!");
      return;
    }
    const encoder = new TextEncoder();
    await characteristic.writeValue(encoder.encode(command));
    console.log(`Command sent: ${command}`);
  };

  return (
    <div>
      <h1>Car Controller</h1>
      <button onClick={connectBluetooth}>Connect Bluetooth</button>
      <br />
      <button onClick={() => sendCommand("f")}>Forward</button>
      <button onClick={() => sendCommand("b")}>Backward</button>
      <button onClick={() => sendCommand("l")}>Left</button>
      <button onClick={() => sendCommand("r")}>Right</button>
      <button onClick={() => sendCommand("s")}>Stop</button>
      <button onClick={() => sendCommand("a")}>Toggle Light</button>
    </div>
  );
}

export default App;
