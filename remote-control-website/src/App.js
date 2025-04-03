import React, { useState } from "react";
import "./App.css";

function App() {
  const [device, setDevice] = useState(null);
  const [characteristic, setCharacteristic] = useState(null);
  const [mode, setMode] = useState("Remote");

  const connectBluetooth = async () => {
    try {
      const btDevice = await navigator.bluetooth.requestDevice({
        acceptAllDevices: true,
        optionalServices: ["0000ffe0-0000-1000-8000-00805f9b34fb"],
      });
      const server = await btDevice.gatt.connect();
      const service = await server.getPrimaryService(
        "0000ffe0-0000-1000-8000-00805f9b34fb"
      );
      const btCharacteristic = await service.getCharacteristic(
        "0000ffe1-0000-1000-8000-00805f9b34fb"
      );
      setDevice(btDevice);
      setCharacteristic(btCharacteristic);
      console.log("Bluetooth connected", btDevice);
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
    console.log(device);

    if (command === "R") setMode("Remote");
    if (command === "x") setMode("STOP");
  };

  return (
    <div className="container">
      <h1>Car Controller</h1>
      <button onClick={connectBluetooth}>Connect Bluetooth</button>
      <h2>Mode: {mode}</h2>

      <div className="button-group">
        <button onClick={() => sendCommand("R")}>Remote Mode</button>
        <button onClick={() => sendCommand("x")}>STOP Mode</button>
      </div>

      <div className="controls">
        <button className="arrow up" onClick={() => sendCommand("f")}>
          &#8593;
        </button>
        <button className="arrow left" onClick={() => sendCommand("l")}>
          &#8592;
        </button>
        <button className="arrow right" onClick={() => sendCommand("r")}>
          &#8594;
        </button>
        <button className="arrow down" onClick={() => sendCommand("b")}>
          &#8595;
        </button>
        <button className="stop-btn" onClick={() => sendCommand("s")}>
          Stop
        </button>
      </div>
    </div>
  );
}

export default App;
