#ifndef WEB_INTERFACE_HYBRID_V8_SMART_H
#define WEB_INTERFACE_HYBRID_V8_SMART_H

const char INDEX_HTML_HYBRID[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP32 Car - MEAM 5100 Final</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <style>
    /* === Global reset === */
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
      -webkit-tap-highlight-color: transparent;
    }

    /* === Background === */
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #a18cd1 0%, #fbc2eb 100%);
      color: #2d3748;
      min-height: 100vh;
      padding: 15px;
    }

    .container {
      max-width: 900px;
      margin: 0 auto;
      padding-bottom: 60px;
    }

    .header {
      text-align: center;
      margin-bottom: 25px;
    }
    .header h1 {
      font-size: 28px;
      color: #fff;
      text-shadow: 0 2px 4px rgba(0,0,0,0.2);
      margin-bottom: 10px;
      font-weight: 800;
    }
    .header .mode-badge {
      display: inline-block;
      background: #fffdf0;
      color: #b794f4;
      padding: 6px 18px;
      border-radius: 20px;
      font-size: 14px;
      font-weight: bold;
      box-shadow: 0 4px 10px rgba(0,0,0,0.1);
    }

    .panel, .speed-control, .tuning-section, .steering-control, .wf-param-section {
      background: rgba(252, 240, 255, 0.9);
      backdrop-filter: blur(12px);
      border-radius: 24px;
      padding: 20px;
      margin-bottom: 20px;
      box-shadow: 0 10px 25px -5px rgba(100, 50, 100, 0.1), 0 8px 10px -6px rgba(0, 0, 0, 0.05);
      border: 1px solid rgba(255, 255, 255, 0.5);
    }

    .panel-title, .section-header, .steering-title {
      font-size: 18px;
      margin-bottom: 15px;
      color: #553c9a;
      display: flex;
      align-items: center;
      gap: 10px;
      font-weight: 800;
    }

    .two-column {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 20px;
    }

    .connection-status {
      position: fixed;
      top: 15px;
      right: 15px;
      padding: 8px 16px;
      border-radius: 30px;
      font-size: 12px;
      font-weight: bold;
      box-shadow: 0 4px 15px rgba(0,0,0,0.1);
      z-index: 1000;
      color: #fff;
      border: 2px solid rgba(255,255,255,0.5);
    }
    .connection-status.connected { background: #68d391; }
    .connection-status.disconnected { background: #fc8181; }

    .team-select-container {
      display: flex;
      gap: 15px;
      justify-content: center;
      margin-bottom: 15px;
    }
    .team-btn {
      padding: 15px 35px;
      font-size: 18px;
      font-weight: 800;
      border: 3px solid transparent;
      border-radius: 16px;
      cursor: pointer;
      transition: all 0.3s;
      color: #fff;
      box-shadow: 0 4px 0 rgba(0,0,0,0.1);
    }
    .team-btn:active {
      transform: translateY(4px);
      box-shadow: none;
    }
    .team-btn.blue {
      background: linear-gradient(135deg, #a1c4fd 0%, #c2e9fb 100%);
      color: #2c5282;
    }
    .team-btn.red {
      background: linear-gradient(135deg, #fccb90 0%, #d57eeb 100%);
      color: #822727;
    }
    .team-btn.selected {
      border-color: #ffd700;
      box-shadow: 0 0 20px rgba(255, 215, 0, 0.5);
      transform: scale(1.05);
    }
    .team-indicator {
      text-align: center;
      padding: 10px;
      border-radius: 12px;
      font-weight: bold;
      font-size: 16px;
      background: rgba(255,255,255,0.5);
      color: #553c9a;
    }
    .team-indicator.blue-team {
      background: rgba(161, 196, 253, 0.3);
      color: #2c5282;
    }
    .team-indicator.red-team {
      background: rgba(252, 203, 144, 0.3);
      color: #822727;
    }

    .vive-coord-display {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 20px;
      margin-bottom: 20px;
    }
    .vive-coord-box {
      background: rgba(255, 255, 255, 0.7);
      border-radius: 16px;
      padding: 20px;
      text-align: center;
      border: 2px solid #e9d8fd;
      box-shadow: 0 4px 15px rgba(100, 50, 100, 0.1);
    }
    .vive-coord-label {
      font-size: 14px;
      color: #805ad5;
      font-weight: 700;
      margin-bottom: 8px;
      text-transform: uppercase;
    }
    .vive-coord-value {
      font-size: 36px;
      font-weight: 800;
      color: #553c9a;
      font-family: 'Courier New', monospace;
      line-height: 1.2;
    }
    .vive-coord-inch {
      font-size: 14px;
      color: #718096;
      margin-top: 5px;
    }
    
    .vive-signal-bar {
      display: flex;
      justify-content: space-between;
      align-items: center;
      background: rgba(255,255,255,0.6);
      border-radius: 12px;
      padding: 12px 20px;
      margin-bottom: 15px;
      flex-wrap: wrap;
      gap: 15px;
      border: 1px solid #e9d8fd;
    }
    .vive-signal-item {
      display: flex;
      align-items: center;
      gap: 8px;
    }
    .vive-signal-label {
      font-size: 13px;
      color: #718096;
      font-weight: 600;
    }
    .vive-signal-indicator {
      font-size: 20px;
      transition: color 0.3s;
    }
    .vive-signal-indicator.valid { color: #38a169; text-shadow: 0 0 10px rgba(56, 161, 105, 0.5); }
    .vive-signal-indicator.invalid { color: #e53e3e; }
    .vive-signal-text {
      font-size: 13px;
      font-weight: 700;
    }
    .vive-signal-text.valid { color: #38a169; }
    .vive-signal-text.invalid { color: #e53e3e; }
    
    .vive-heading-value {
      font-size: 16px;
      font-weight: 800;
      color: #805ad5;
      font-family: monospace;
    }

    /* UART调试状态 */
    .dual-vive-status {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
      margin-top: 10px;
    }
    .vive-sensor-box {
      background: rgba(255,255,255,0.6);
      border-radius: 10px;
      padding: 10px;
      text-align: center;
      border: 1px solid #e9d8fd;
    }
    .vive-sensor-label {
      font-size: 12px;
      color: #805ad5;
      font-weight: 700;
    }
    .vive-sensor-value {
      font-size: 14px;
      color: #553c9a;
      font-family: monospace;
    }
    .vive-reliable { color: #38a169; }
    .vive-unreliable { color: #e53e3e; }

      font-weight: 800;
      color: #553c9a;
      padding: 4px 12px;
      background: rgba(233, 216, 253, 0.5);
      border-radius: 8px;
    }

    .nav-btn-grid {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 12px;
      margin-bottom: 20px;
    }
    .nav-btn {
      padding: 15px;
      border: none;
      border-radius: 16px;
      font-size: 14px;
      font-weight: 700;
      cursor: pointer;
      color: #fff;
      transition: all 0.2s;
      text-shadow: 0 1px 1px rgba(0,0,0,0.1);
      box-shadow: 0 4px 0 rgba(0,0,0,0.05);
    }
    .nav-btn:active { transform: translateY(4px); box-shadow: none; }
    .nav-btn.capture-low { background: linear-gradient(135deg, #84fab0 0%, #8fd3f4 100%); }
    .nav-btn.capture-high { background: linear-gradient(135deg, #fccb90 0%, #d57eeb 100%); }
    .nav-btn.attack-nexus { background: linear-gradient(135deg, #a1c4fd 0%, #c2e9fb 100%); }
    .nav-btn.wall-follow { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); }
    .nav-btn.stop-nav {
      grid-column: 1 / -1;
      background: linear-gradient(135deg, #ff9a9e 0%, #fecfef 100%);
      font-size: 16px;
      box-shadow: 0 4px 0 rgba(255, 154, 158, 0.3);
    }
    .nav-btn.goto-coord { background: linear-gradient(135deg, #e9d8fd 0%, #d6bcfa 100%); color: #553c9a; }
    
    .nav-btn.smart-nav {
      background: linear-gradient(135deg, #6ee7b7 0%, #34d399 100%);
      color: #064e3b;
      border: 2px solid #10b981;
    }
    .smart-badge {
      display: inline-block;
      background: rgba(255,255,255,0.3);
      padding: 2px 8px;
      border-radius: 10px;
      font-size: 10px;
      margin-left: 5px;
    }

    .coord-input-section {
      background: rgba(255,255,255,0.5);
      border-radius: 16px;
      padding: 15px;
      margin-bottom: 15px;
      border: 1px solid #e9d8fd;
    }
    .coord-input-row {
      display: flex;
      align-items: center;
      gap: 10px;
      margin: 8px 0;
    }
    .coord-input-row label {
      font-weight: 700;
      color: #553c9a;
      min-width: 30px;
    }
    .coord-btn-row {
      display: flex;
      gap: 10px;
      margin-top: 12px;
    }
    .coord-btn-row button { flex: 1; }

    .nav-status-section {
      background: rgba(255, 255, 255, 0.5);
      padding: 15px;
      border-radius: 12px;
      margin-top: 15px;
      border: 1px solid #e9d8fd;
    }
    .nav-status-row {
      display: flex;
      justify-content: space-between;
      margin: 6px 0;
      font-size: 13px;
    }
    .nav-status-label { color: #718096; }
    .nav-status-value {
      font-weight: bold;
      color: #805ad5;
      font-family: monospace;
    }
    
    .path-info {
      background: rgba(16, 185, 129, 0.1);
      border: 1px solid #10b981;
      border-radius: 10px;
      padding: 10px;
      margin-top: 10px;
    }
    .path-info-title {
      font-size: 12px;
      color: #065f46;
      font-weight: 700;
      margin-bottom: 5px;
    }
    .path-waypoint-list {
      font-size: 11px;
      color: #047857;
      font-family: monospace;
    }

    .joystick-container {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 15px;
    }
    #joystick {
      width: 220px;
      height: 220px;
      background: rgba(230, 230, 250, 0.5);
      border-radius: 50%;
      position: relative;
      touch-action: none;
      cursor: pointer;
      border: 2px solid rgba(255,255,255,0.6);
      box-shadow: inset 0 0 20px rgba(255, 255, 255, 0.2);
    }
    #knob {
      width: 60px;
      height: 60px;
      background: linear-gradient(135deg, #ff9a9e 0%, #fecfef 99%, #a6c1ee 100%);
      border-radius: 50%;
      position: absolute;
      top: 50%;
      left: 50%;
      margin-left: -30px;
      margin-top: -30px;
      box-shadow: 0 6px 15px rgba(255, 154, 158, 0.4);
      border: 3px solid #fff;
      pointer-events: none;
    }

    .slider-row {
      display: flex;
      align-items: center;
      gap: 12px;
      margin: 12px 0;
    }
    .slider-label {
      width: 80px;
      font-weight: 700;
      color: #666;
    }
    .slider-value {
      width: 45px;
      text-align: right;
      font-weight: 800;
      color: #805ad5;
      font-family: 'Courier New', monospace;
    }
    input[type="range"] {
      flex: 1;
      height: 10px;
      border-radius: 5px;
      background: #e9d8fd;
      appearance: none;
      outline: none;
    }
    input[type="range"]::-webkit-slider-thumb {
      appearance: none;
      width: 24px;
      height: 24px;
      border-radius: 50%;
      background: #fff;
      border: 3px solid #d6bcfa;
      box-shadow: 0 2px 5px rgba(0,0,0,0.1);
      transition: transform 0.1s;
    }

    .controls, .steering-buttons {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 12px;
    }
    button, .btn-turn-left, .btn-straight, .btn-turn-right {
      padding: 14px;
      border: none;
      border-radius: 16px;
      font-size: 15px;
      font-weight: 700;
      cursor: pointer;
      transition: all 0.2s;
      color: #fff;
      text-shadow: 0 1px 1px rgba(0,0,0,0.1);
      box-shadow: 0 4px 0 rgba(0,0,0,0.05);
    }
    button:active {
      transform: translateY(4px);
      box-shadow: none;
    }

    .btn-forward, .btn-straight, .btn-wf-start {
      background: linear-gradient(135deg, #84fab0 0%, #8fd3f4 100%);
    }
    .btn-neutral, .btn-turn-right {
      background: linear-gradient(135deg, #a1c4fd 0%, #c2e9fb 100%);
    }
    .btn-reverse, .btn-turn-left {
      background: linear-gradient(135deg, #fccb90 0%, #d57eeb 100%);
    }
    .btn-stop, .btn-wf-stop {
      background: linear-gradient(135deg, #ff9a9e 0%, #fecfef 100%);
      grid-column: 1 / -1;
      font-size: 18px;
      padding: 16px;
      box-shadow: 0 4px 0 rgba(255, 154, 158, 0.3);
    }

    .param-row, .wf-param-row {
      display: grid;
      grid-template-columns: auto 1fr 70px;
      gap: 12px;
      align-items: center;
      margin: 10px 0;
    }
    .param-label, .wf-param-label {
      font-weight: 700;
      color: #553c9a;
      font-size: 14px;
      min-width: 45px;
    }
    .wf-param-value {
      text-align: right;
      font-weight: 800;
      color: #d53f8c;
      font-size: 15px;
    }
    input[type="number"], input[type="text"] {
      width: 70px;
      padding: 8px;
      border-radius: 10px;
      border: 2px solid #e9d8fd;
      background: #fffdf5;
      font-weight: bold;
      color: #553c9a;
      text-align: center;
    }
    input[type="number"]:focus, input[type="text"]:focus {
      border-color: #d6bcfa;
      background: #fff;
      outline: none;
    }

    .preset-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 10px;
      margin-top: 15px;
    }
    .btn-preset {
      background: #f7fafc;
      color: #718096;
      box-shadow: none;
      border: 1px solid #cbd5e0;
    }
    .btn-preset:hover {
      background: #edf2f7;
    }

    .status-grid, .ff-grid {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 15px;
    }
    .status-card, .ff-card {
      background: rgba(240, 255, 244, 0.8);
      padding: 15px;
      border-radius: 16px;
      border: 1px solid #c6f6d5;
    }
    .status-card.right {
      background: rgba(235, 248, 255, 0.8);
      border-color: #bee3f8;
      border-left-color: #90cdf4;
    }
    .status-card { border-left: 5px solid #9ae6b4; }

    .status-title, .ff-title {
      font-size: 13px;
      color: #718096;
      margin-bottom: 8px;
      font-weight: 800;
      text-transform: uppercase;
    }
    .status-row {
      display: flex;
      justify-content: space-between;
      margin: 6px 0;
      font-size: 13px;
      color: #4a5568;
    }
    .status-value {
      font-weight: bold;
      color: #2d3748;
      font-family: 'Courier New', monospace;
    }
    .status-value.error { color: #e53e3e; }
    .status-value.good { color: #38a169; }

    .wf-status-badge {
      display: inline-block;
      padding: 5px 12px;
      border-radius: 20px;
      font-size: 12px;
      font-weight: bold;
      margin-left: 10px;
      border: 1px solid rgba(0,0,0,0.05);
    }
    .wf-status-badge.running {
      background: #d1fae5;
      color: #065f46;
      animation: pulse 2s infinite;
    }
    .wf-status-badge.stopped {
      background: #fff5f5;
      color: #c53030;
    }

    .wf-control-grid {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 12px;
      margin-bottom: 15px;
    }

    .wf-tof-display {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 10px;
      margin-top: 15px;
    }
    .wf-tof-item {
      background: rgba(255, 255, 255, 0.6);
      padding: 10px;
      border-radius: 12px;
      text-align: center;
      border: 1px solid #e2e8f0;
    }
    .wf-tof-label {
      font-size: 11px;
      color: #718096;
      margin-bottom: 5px;
    }
    .wf-tof-value {
      font-size: 16px;
      font-weight: 800;
      color: #553c9a;
      font-family: monospace;
    }
    .wf-tof-value.danger { color: #e53e3e; background: #fed7d7; border-radius: 6px; }
    .wf-tof-value.warning { color: #dd6b20; background: #feebc8; border-radius: 6px; }

    @keyframes pulse {
      0% { transform: scale(1); }
      50% { transform: scale(1.05); }
      100% { transform: scale(1); }
    }

    @media (max-width: 768px) {
      .two-column { grid-template-columns: 1fr; }
      .header h1 { font-size: 24px; }
      #joystick { width: 200px; height: 200px; }
      .team-btn { padding: 12px 25px; font-size: 16px; }
      .vive-coord-value { font-size: 28px; }
      .vive-signal-bar { flex-direction: column; align-items: flex-start; }
      .ff-grid { grid-template-columns: 1fr; }
      .preset-grid { grid-template-columns: 1fr; }
      .nav-btn-grid { grid-template-columns: 1fr; }
    }
  </style>
</head>
<body>
  <div class="connection-status connected" id="connStatus">● Connected</div>

  <div class="container">
    <div class="header">
      <h1>🏎️ ESP32 Racing Car</h1>
      <div class="mode-badge">🎮 MEAM 5100 Final Project</div>
    </div>

    <!-- ========== team choose========== -->
    <div class="panel">
      <div class="panel-title">🎯 Team Selection</div>
      <div class="team-select-container">
        <button class="team-btn blue" id="teamBlueBtn" onclick="selectTeam('blue')">🔵 BLUE TEAM</button>
        <button class="team-btn red" id="teamRedBtn" onclick="selectTeam('red')">🔴 RED TEAM</button>
      </div>
      <div class="team-indicator blue-team" id="teamIndicator">Current Team: BLUE</div>
    </div>

    <!-- ========== Dual Vive Position Display ========== -->
    <div class="panel">
      <div class="panel-title">📡 Dual Vive Position Tracking</div>
      
      <div class="vive-coord-display">
        <div class="vive-coord-box">
          <div class="vive-coord-label">X Position (Center)</div>
          <div class="vive-coord-value" id="viveXLarge">----</div>
          <div class="vive-coord-inch" id="viveXInch">-- inch</div>
        </div>
        <div class="vive-coord-box">
          <div class="vive-coord-label">Y Position (Center)</div>
          <div class="vive-coord-value" id="viveYLarge">----</div>
          <div class="vive-coord-inch" id="viveYInch">-- inch</div>
        </div>
      </div>
      
      <div class="vive-signal-bar">
        <div class="vive-signal-item">
          <span class="vive-signal-label">Signal:</span>
          <span class="vive-signal-indicator" id="viveSignalLarge">●</span>
          <span class="vive-signal-text" id="viveSignalText">NO SIGNAL</span>
        </div>
        <div class="vive-signal-item">
          <span class="vive-signal-label">Heading:</span>
          <span class="vive-heading-value" id="viveHeadingLarge">--°</span>
        </div>
      </div>
      
      <div class="dual-vive-status" style="background: rgba(255,200,200,0.3);">
        <div class="vive-sensor-box">
          <div class="vive-sensor-label">📡 UART Raw A</div>
          <div class="vive-sensor-value" id="uartRawA">X:-- Y:--</div>
          <div class="vive-sensor-value" id="uartValidA">--</div>
        </div>
        <div class="vive-sensor-box">
          <div class="vive-sensor-label">📡 UART Raw B</div>
          <div class="vive-sensor-value" id="uartRawB">X:-- Y:--</div>
          <div class="vive-sensor-value" id="uartValidB">--</div>
        </div>
      </div>
      <div style="text-align: center; font-size: 11px; color: #666; margin-top: 8px;">
        ⏱️ Last UART: <span id="uartAge">--</span>ms ago
      </div>
      
    </div>

    <div class="panel">
      <div class="panel-title">🧭 Autonomous Navigation</div>
      
      <div class="nav-btn-grid">
        <button class="nav-btn capture-low" onclick="taskCaptureLow()">🏰 Capture Low Tower</button>
        <button class="nav-btn capture-high" onclick="taskCaptureHigh()">⛰️ Capture High Tower</button>
        <button class="nav-btn attack-nexus" onclick="taskAttackNexus()">💥 Attack Nexus</button>
        <button class="nav-btn wall-follow" onclick="taskWallFollow()">🧱 Wall Follow Full</button>
        <button class="nav-btn stop-nav" onclick="stopNavigation()">🛑 STOP Navigation</button>
      </div>
      
      <div class="coord-input-section">
        <div style="font-size: 14px; color: #553c9a; margin-bottom: 12px; font-weight: bold;">📍 Manual Coordinate Navigation</div>
        
        <div style="margin-bottom: 10px; font-size: 12px; color: #666;">Single Coordinate:</div>
        <div class="coord-input-row">
          <label>X:</label>
          <input type="number" id="coord1X" placeholder="X" value="4000">
          <label>Y:</label>
          <input type="number" id="coord1Y" placeholder="Y" value="4000">
        </div>
        <div class="coord-btn-row">
          <button class="nav-btn goto-coord" onclick="gotoSingleCoord()">GO Direct →</button>
          <button class="nav-btn smart-nav" onclick="gotoSmartCoord()">🧠 Smart Nav<span class="smart-badge">AUTO</span></button>
        </div>
        
        <div style="margin: 15px 0 10px 0; font-size: 12px; color: #666;">Three Coordinates:</div>
        <div class="coord-input-row">
          <label>P1:</label>
          <input type="number" id="coord2X1" placeholder="X1" value="3500" style="width:70px;">
          <input type="number" id="coord2Y1" placeholder="Y1" value="3000" style="width:70px;">
        </div>
        <div class="coord-input-row">
          <label>P2:</label>
          <input type="number" id="coord2X2" placeholder="X2" value="4500" style="width:70px;">
          <input type="number" id="coord2Y2" placeholder="Y2" value="4500" style="width:70px;">
        </div>
        <div class="coord-input-row">
          <label>P3:</label>
          <input type="number" id="coord2X3" placeholder="X3" value="4000" style="width:70px;">
          <input type="number" id="coord2Y3" placeholder="Y3" value="6000" style="width:70px;">
        </div>
        <div class="coord-btn-row">
          <button class="nav-btn goto-coord" onclick="gotoThreeCoords()">🎯 Go to 3 Locations</button>
        </div>
        
        <div style="margin-top: 15px; font-size: 12px; color: #666; margin-bottom: 8px;">Quick Targets:</div>
        <div class="preset-grid">
          <button class="btn-preset" onclick="setQuickTarget(3300, 4150)">🏔️ High Tower</button>
          <button class="btn-preset" onclick="setQuickTarget(4452, 4124)">🏛️ Low Tower</button>
          <button class="btn-preset" onclick="setQuickTarget(4452, 1900)">💥 Nexus 1</button>
          <button class="btn-preset" onclick="setQuickTarget(4452, 6400)">💥 Nexus 2</button>
          <button class="btn-preset" onclick="setQuickTarget(4000, 4000)">🎯 Center</button>
          <button class="btn-preset" onclick="setQuickTarget(3270, 1720)">🚪 Ramp Entry</button>
        </div>
      </div>
      
      <div class="nav-status-section">
        <div style="font-size: 14px; color: #805ad5; margin-bottom: 10px; font-weight: bold;">📊 Navigation Status</div>
        <div class="nav-status-row">
          <span class="nav-status-label">State:</span>
          <span class="nav-status-value" id="navState">IDLE</span>
        </div>
        <div class="nav-status-row">
          <span class="nav-status-label">Task:</span>
          <span class="nav-status-value" id="navTask">NONE</span>
        </div>
        <div class="nav-status-row">
          <span class="nav-status-label">Target:</span>
          <span class="nav-status-value" id="navTarget">--</span>
        </div>
        <div class="nav-status-row">
          <span class="nav-status-label">Distance:</span>
          <span class="nav-status-value" id="navDistance">--</span>
        </div>
        <div class="nav-status-row">
          <span class="nav-status-label">Progress:</span>
          <span class="nav-status-value" id="navProgress">--</span>
        </div>
        
        <div class="path-info" id="pathInfo" style="display: none;">
          <div class="path-info-title">🛤️ Planned Path</div>
          <div class="path-waypoint-list" id="pathWaypoints">--</div>
        </div>
      </div>
    </div>

    <div class="two-column">
      <div class="panel">
        <div class="panel-title">🎮 Vehicle Control</div>
        <div class="joystick-container">
          <div id="joystick">
            <div id="knob"></div>
          </div>
          <div class="speed-control">
            <div class="slider-row">
              <span class="slider-label">Speed:</span>
              <input type="range" id="speedSlider" min="20" max="100" value="60" oninput="updateSpeed()">
              <span class="slider-value" id="speedValue">60%</span>
            </div>
            <div class="slider-row">
              <span class="slider-label">Turn:</span>
              <input type="range" id="turnSlider" min="-50" max="50" value="0" oninput="updateTurnManual()">
              <span class="slider-value" id="turnValue">0</span>
            </div>
          </div>
        </div>
        <div class="controls">
          <button class="btn-forward" onclick="setForward()">⬆ FWD</button>
          <button class="btn-neutral" onclick="setNeutral()">⏸ STOP</button>
          <button class="btn-reverse" onclick="setReverse()">⬇ REV</button>
          <button class="btn-stop" onclick="emergencyStop()">🛑 EMERGENCY STOP</button>
        </div>
        <div class="steering-control">
          <div class="steering-title">🎯 Steering Control</div>
          <div class="steering-buttons">
            <button class="btn-turn-left" onclick="setTurnLeft()">← Turn Left</button>
            <button class="btn-straight" onclick="setStraight()">↑ Straight</button>
            <button class="btn-turn-right" onclick="setTurnRight()">Turn Right →</button>
          </div>
        </div>
      </div>

      <div class="panel">
        <div class="panel-title">📊 Real-time Status</div>
        <div class="status-grid">
          <div class="status-card">
            <div class="status-title">🔵 Left Wheel</div>
            <div class="status-row">
              <span class="status-label">Current RPM:</span>
              <span class="status-value" id="leftRPM">0.0</span>
            </div>
            <div class="status-row">
              <span class="status-label">Target RPM:</span>
              <span class="status-value" id="leftTarget">0.0</span>
            </div>
            <div class="status-row">
              <span class="status-label">Error:</span>
              <span class="status-value error" id="leftError">0.0</span>
            </div>
            <div class="status-row">
              <span class="status-label">PWM:</span>
              <span class="status-value" id="leftPWM">0</span>
            </div>
            <div class="status-row">
              <span class="status-label">FF PWM:</span>
              <span class="status-value" id="leftFF">0</span>
            </div>
            <div class="status-row">
              <span class="status-label">PID Corr:</span>
              <span class="status-value" id="leftCorr">0</span>
            </div>
          </div>
          
          <div class="status-card right">
            <div class="status-title">🔵 Right Wheel</div>
            <div class="status-row">
              <span class="status-label">Current RPM:</span>
              <span class="status-value" id="rightRPM">0.0</span>
            </div>
            <div class="status-row">
              <span class="status-label">Target RPM:</span>
              <span class="status-value" id="rightTarget">0.0</span>
            </div>
            <div class="status-row">
              <span class="status-label">Error:</span>
              <span class="status-value error" id="rightError">0.0</span>
            </div>
            <div class="status-row">
              <span class="status-label">PWM:</span>
              <span class="status-value" id="rightPWM">0</span>
            </div>
            <div class="status-row">
              <span class="status-label">FF PWM:</span>
              <span class="status-value" id="rightFF">0</span>
            </div>
            <div class="status-row">
              <span class="status-label">PID Corr:</span>
              <span class="status-value" id="rightCorr">0</span>
            </div>
          </div>
        </div>
      </div>
    </div>

    <div class="panel">
      <div class="panel-title">
        🧱 Wall Follow Control (Left Side)
        <span class="wf-status-badge stopped" id="wfStatusBadge">STOPPED</span>
      </div>
      
      <div class="wf-control-grid">
        <button class="btn-wf-start" onclick="startWallFollow()">▶️ START Wall Follow</button>
        <button class="btn-wf-stop" onclick="stopWallFollow()">⏹️ STOP Wall Follow</button>
      </div>
      
      <div class="wf-param-section">
        <div style="font-size: 14px; color: #805ad5; margin-bottom: 12px; font-weight: bold;">⚙️ All Parameters (slider + input)</div>
        
        <div class="param-row">
          <span class="param-label" style="width: 90px;">Speed (%):</span>
          <input type="range" id="wfSpeedSlider" min="20" max="108" step="1" value="60" oninput="updateWfParams()">
          <input type="number" id="wfSpeedBox" min="20" max="108" step="1" value="60" onchange="updateWfParamsFromInput()" style="width:60px;">
        </div>
        <div style="font-size: 10px; color: #888; margin: 2px 0 10px 100px;">Max 108 = 108 RPM</div>
        
        <div class="param-row">
          <span class="param-label" style="width: 90px;">Side Dist (cm):</span>
          <input type="range" id="wfSideSlider" min="10" max="50" step="1" value="5" oninput="updateWfParams()">
          <input type="number" id="wfSideBox" min="10" max="50" step="1" value="5" onchange="updateWfParamsFromInput()" style="width:60px;">
        </div>
        <div style="font-size: 10px; color: #888; margin: 2px 0 10px 100px;">Target distance from left wall</div>
        
        <div class="param-row">
          <span class="param-label" style="width: 90px;">Front Thres (cm):</span>
          <input type="range" id="wfFrontSlider" min="15" max="60" step="1" value="20" oninput="updateWfParams()">
          <input type="number" id="wfFrontBox" min="15" max="60" step="1" value="20" onchange="updateWfParamsFromInput()" style="width:60px;">
        </div>
        <div style="font-size: 10px; color: #888; margin: 2px 0 10px 100px;">Front obstacle threshold</div>
        
        <div class="param-row">
          <span class="param-label" style="width: 90px;">Max Turn R:</span>
          <input type="range" id="wfMaxTurnRSlider" min="20" max="100" step="5" value="50" oninput="updateWfParams()">
          <input type="number" id="wfMaxTurnRBox" min="20" max="100" step="5" value="50" onchange="updateWfParamsFromInput()" style="width:60px;">
        </div>
        <div style="font-size: 10px; color: #888; margin: 2px 0 10px 100px;">右转强度 (避障/corner/太近)</div>
        
        <div class="param-row">
          <span class="param-label" style="width: 90px;">Max Turn L:</span>
          <input type="range" id="wfMaxTurnLSlider" min="20" max="100" step="5" value="50" oninput="updateWfParams()">
          <input type="number" id="wfMaxTurnLBox" min="20" max="100" step="5" value="50" onchange="updateWfParamsFromInput()" style="width:60px;">
        </div>
        <div style="font-size: 10px; color: #888; margin: 2px 0 10px 100px;">左转强度 (找墙/出胡同/太远)</div>
        
        <div class="param-row">
          <span class="param-label" style="width: 90px;">Dist Kp:</span>
          <input type="range" id="wfDistKpSlider" min="0" max="1.0" step="0.01" value="0.15" oninput="updateWfParams()">
          <input type="number" id="wfDistKpBox" min="0" max="1.0" step="0.01" value="0.15" onchange="updateWfParamsFromInput()" style="width:60px;">
        </div>
        <div style="font-size: 10px; color: #888; margin: 2px 0 10px 100px;">距离修正增益</div>
        
        <div class="param-row">
          <span class="param-label" style="width: 90px;">Angle Kp:</span>
          <input type="range" id="wfAngleKpSlider" min="0" max="5.0" step="0.1" value="2.5" oninput="updateWfParams()">
          <input type="number" id="wfAngleKpBox" min="0" max="5.0" step="0.1" value="2.5" onchange="updateWfParamsFromInput()" style="width:60px;">
        </div>
        <div style="font-size: 10px; color: #888; margin: 2px 0 10px 100px;">角度修正增益</div>
      </div>
      
      <div class="wf-param-section" style="margin-top: 15px;">
        <div style="font-size: 14px; color: #805ad5; margin-bottom: 10px; font-weight: bold;">📊 Debug Info</div>
        <div class="status-row">
          <span class="status-label">State:</span>
          <span class="status-value" id="wfStateDisplay">--</span>
        </div>
        <div class="status-row">
          <span class="status-label">Wall Angle:</span>
          <span class="status-value" id="wfAngleDisplay">--</span>
        </div>
        <div class="status-row">
          <span class="status-label">Dist Error:</span>
          <span class="status-value" id="wfDistErrDisplay">--</span>
        </div>
        <div class="status-row">
          <span class="status-label">Current Turn:</span>
          <span class="status-value" id="wfTurnDisplay">--</span>
        </div>
      </div>
      
      <div class="wf-tof-display">
        <div class="wf-tof-item">
          <div class="wf-tof-label">🎯 FRONT TOF</div>
          <div class="wf-tof-value" id="wfTofFront">--</div>
        </div>
        <div class="wf-tof-item">
          <div class="wf-tof-label">◀ LEFT1 (Front)</div>
          <div class="wf-tof-value" id="wfTofLeft1">--</div>
        </div>
        <div class="wf-tof-item">
          <div class="wf-tof-label">◀ LEFT2 (Rear)</div>
          <div class="wf-tof-value" id="wfTofLeft2">--</div>
        </div>
      </div>
    </div>

    <div class="panel">
      <div class="panel-title">⚙️ PID Parameters Tuning</div>
      <div class="two-column">
        <div class="tuning-section">
          <div class="section-header">
            <span>🔵 Left Wheel PID</span>
            <button onclick="syncPID('LtoR')" style="padding: 4px 10px; font-size: 11px; background: #667eea;">Copy →</button>
          </div>
          <div class="param-row">
            <span class="param-label">Kp:</span>
            <input type="range" id="kpL" min="0" max="10" step="0.1" value="2.5" oninput="updatePID()">
            <input type="number" id="kpLval" min="0" max="10" step="0.1" value="2.5" onchange="updatePIDFromInput()">
          </div>
          <div class="param-row">
            <span class="param-label">Ki:</span>
            <input type="range" id="kiL" min="0" max="5" step="0.1" value="0.6" oninput="updatePID()">
            <input type="number" id="kiLval" min="0" max="5" step="0.1" value="0.6" onchange="updatePIDFromInput()">
          </div>
          <div class="param-row">
            <span class="param-label">Kd:</span>
            <input type="range" id="kdL" min="0" max="2" step="0.01" value="0.1" oninput="updatePID()">
            <input type="number" id="kdLval" min="0" max="2" step="0.01" value="0.1" onchange="updatePIDFromInput()">
          </div>
        </div>
        
        <div class="tuning-section">
          <div class="section-header">
            <button onclick="syncPID('RtoL')" style="padding: 4px 10px; font-size: 11px; background: #667eea;">← Copy</button>
            <span>🔵 Right Wheel PID</span>
          </div>
          <div class="param-row">
            <span class="param-label">Kp:</span>
            <input type="range" id="kpR" min="0" max="10" step="0.1" value="2.5" oninput="updatePID()">
            <input type="number" id="kpRval" min="0" max="10" step="0.1" value="2.5" onchange="updatePIDFromInput()">
          </div>
          <div class="param-row">
            <span class="param-label">Ki:</span>
            <input type="range" id="kiR" min="0" max="5" step="0.1" value="0.6" oninput="updatePID()">
            <input type="number" id="kiRval" min="0" max="5" step="0.1" value="0.6" onchange="updatePIDFromInput()">
          </div>
          <div class="param-row">
            <span class="param-label">Kd:</span>
            <input type="range" id="kdR" min="0" max="2" step="0.01" value="0.1" oninput="updatePID()">
            <input type="number" id="kdRval" min="0" max="2" step="0.01" value="0.1" onchange="updatePIDFromInput()">
          </div>
        </div>
      </div>
      
      <div class="preset-grid">
        <button class="btn-preset" onclick="loadPreset('conservative')">🐢 Conservative<br>(Kp=1.5 Ki=0.3 Kd=0.05)</button>
        <button class="btn-preset" onclick="loadPreset('balanced')">⚖️ Balanced<br>(Kp=2.5 Ki=0.6 Kd=0.1)</button>
        <button class="btn-preset" onclick="loadPreset('aggressive')">🚀 Aggressive<br>(Kp=4.0 Ki=1.2 Kd=0.2)</button>
      </div>
    </div>

    <div class="panel">
      <div class="panel-title">📊 Feedforward Parameters</div>
      <div class="ff-grid">
        <div class="ff-card">
          <div class="ff-title">Velocity Gain (Kv)</div>
          <div class="param-row">
            <span class="param-label">Kv:</span>
            <input type="range" id="ffKv" min="5" max="15" step="0.1" value="8.9" oninput="updateFF()">
            <input type="number" id="ffKvVal" min="5" max="15" step="0.1" value="8.9" onchange="updateFFFromInput()">
          </div>
          <div style="font-size: 11px; color: #aaa; margin-top: 5px;">
            PWM增益：RPM → PWM的转换系数
          </div>
        </div>
        
        <div class="ff-card">
          <div class="ff-title">Static Friction</div>
          <div class="param-row">
            <span class="param-label">Sf:</span>
            <input type="range" id="ffStatic" min="0" max="300" step="10" value="60" oninput="updateFF()">
            <input type="number" id="ffStaticVal" min="0" max="300" step="10" value="60" onchange="updateFFFromInput()">
          </div>
          <div style="font-size: 11px; color: #aaa; margin-top: 5px;">
            Static friction compensation
          </div>
        </div>
        
        <div class="ff-card">
          <div class="ff-title">Wheel Calibration</div>
          <div class="param-row">
            <span class="param-label">L:</span>
            <input type="range" id="leftTune" min="0.5" max="1.5" step="0.01" value="1.0" oninput="updateTuning()">
            <input type="number" id="leftTuneVal" min="0.5" max="1.5" step="0.01" value="1.0" onchange="updateTuningFromInput()">
          </div>
          <div class="param-row">
            <span class="param-label">R:</span>
            <input type="range" id="rightTune" min="0.5" max="1.5" step="0.01" value="1.0" oninput="updateTuning()">
            <input type="number" id="rightTuneVal" min="0.5" max="1.5" step="0.01" value="1.0" onchange="updateTuningFromInput()">
          </div>
        </div>
      </div>
    </div>
  </div>

  <script>
    const joystick = document.getElementById('joystick');
    const knob = document.getElementById('knob');
    
    let isDragging = false;
    let currentSpeed = 0;
    let currentTurn = 0;
    let speedSetting = 60;
    let isConnected = true;
    let currentTeamColor = 'blue';
    
    const maxRadius = 82;
    
    const presets = {
      conservative: { kp: 1.5, ki: 0.3, kd: 0.05 },
      balanced: { kp: 2.5, ki: 0.6, kd: 0.1 },
      aggressive: { kp: 4.0, ki: 1.2, kd: 0.2 }
    };
    
    function selectTeam(team) {
      currentTeamColor = team;
      const blueBtn = document.getElementById('teamBlueBtn');
      const redBtn = document.getElementById('teamRedBtn');
      const indicator = document.getElementById('teamIndicator');
      
      if (team === 'blue') {
        blueBtn.classList.add('selected');
        redBtn.classList.remove('selected');
        indicator.textContent = 'Current Team: BLUE';
        indicator.className = 'team-indicator blue-team';
      } else {
        blueBtn.classList.remove('selected');
        redBtn.classList.add('selected');
        indicator.textContent = 'Current Team: RED';
        indicator.className = 'team-indicator red-team';
      }
      
      fetch('/setTeam?team=' + (team === 'blue' ? 0 : 1))
        .then(() => updateConnectionStatus(true))
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function taskCaptureLow() {
      fetch('/taskCaptureLow')
        .then(() => updateConnectionStatus(true))
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function taskCaptureHigh() {
      fetch('/taskCaptureHigh')
        .then(() => updateConnectionStatus(true))
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function taskAttackNexus() {
      fetch('/taskAttackNexus')
        .then(() => updateConnectionStatus(true))
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function taskWallFollow() {
      fetch('/taskWallFollow')
        .then(() => updateConnectionStatus(true))
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function stopNavigation() {
      fetch('/stopNavigation')
        .then(() => {
          updateConnectionStatus(true);
          document.getElementById('pathInfo').style.display = 'none';
        })
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function gotoSingleCoord() {
      const x = document.getElementById('coord1X').value;
      const y = document.getElementById('coord1Y').value;
      fetch('/gotoSingle?x=' + x + '&y=' + y)
        .then(() => updateConnectionStatus(true))
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function gotoSmartCoord() {
      const x = document.getElementById('coord1X').value;
      const y = document.getElementById('coord1Y').value;
      fetch('/gotoSmart?x=' + x + '&y=' + y)
        .then(() => updateConnectionStatus(true))
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function gotoThreeCoords() {
      const x1 = document.getElementById('coord2X1').value;
      const y1 = document.getElementById('coord2Y1').value;
      const x2 = document.getElementById('coord2X2').value;
      const y2 = document.getElementById('coord2Y2').value;
      const x3 = document.getElementById('coord2X3').value;
      const y3 = document.getElementById('coord2Y3').value;
      fetch('/gotoThree?x1=' + x1 + '&y1=' + y1 + '&x2=' + x2 + '&y2=' + y2 + '&x3=' + x3 + '&y3=' + y3)
        .then(() => updateConnectionStatus(true))
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function setQuickTarget(x, y) {
      document.getElementById('coord1X').value = x;
      document.getElementById('coord1Y').value = y;
    }
    
    joystick.addEventListener('mousedown', startDrag);
    joystick.addEventListener('touchstart', startDrag);
    document.addEventListener('mousemove', drag);
    document.addEventListener('touchmove', drag);
    document.addEventListener('mouseup', endDrag);
    document.addEventListener('touchend', endDrag);
    
    function startDrag(e) {
      isDragging = true;
      e.preventDefault();
    }
    
    function drag(e) {
      if (!isDragging) return;
      e.preventDefault();
      const rect = joystick.getBoundingClientRect();
      let clientX = e.touches ? e.touches[0].clientX : e.clientX;
      let clientY = e.touches ? e.touches[0].clientY : e.clientY;
      
      let x = clientX - rect.left - rect.width/2;
      let y = clientY - rect.top - rect.height/2;
      
      const distance = Math.sqrt(x*x + y*y);
      if (distance > maxRadius) {
        const angle = Math.atan2(y, x);
        x = maxRadius * Math.cos(angle);
        y = maxRadius * Math.sin(angle);
      }
      
      knob.style.transform = 'translate(' + x + 'px, ' + y + 'px)';
      
      const deadzone = maxRadius * 0.15;
      if (y < -deadzone) currentSpeed = speedSetting;
      else if (y > deadzone) currentSpeed = -speedSetting;
      else currentSpeed = 0;
      
      currentTurn = Math.round(x / maxRadius * 50);
      document.getElementById('turnSlider').value = currentTurn;
      document.getElementById('turnValue').textContent = currentTurn;
      
      sendCommand();
    }
    
    function endDrag() {
      isDragging = false;
    }
    
    function sendCommand() {
      const direction = currentSpeed >= 0 ? 'Forward' : 'Backward';
      const speed = Math.abs(currentSpeed);
      const url = '/setMotor?speed=' + speed + '&forwardBackward=' + direction + '&turnRate=' + currentTurn;
      fetch(url)
        .then(() => updateConnectionStatus(true))
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function setForward() {
      const knobY = -(speedSetting / 100) * maxRadius;
      knob.style.transform = 'translate(0px, ' + knobY + 'px)';
      currentSpeed = speedSetting;
      currentTurn = 0;
      document.getElementById('turnSlider').value = 0;
      document.getElementById('turnValue').textContent = '0';
      sendCommand();
    }
    
    function setNeutral() {
      knob.style.transform = 'translate(0px, 0px)';
      currentSpeed = 0;
      currentTurn = 0;
      document.getElementById('turnSlider').value = 0;
      document.getElementById('turnValue').textContent = '0';
      sendCommand();
    }
    
    function setReverse() {
      const knobY = (speedSetting / 100) * maxRadius;
      knob.style.transform = 'translate(0px, ' + knobY + 'px)';
      currentSpeed = -speedSetting;
      currentTurn = 0;
      document.getElementById('turnSlider').value = 0;
      document.getElementById('turnValue').textContent = '0';
      sendCommand();
    }
    
    function updateSpeed() {
      const slider = document.getElementById('speedSlider');
      speedSetting = parseInt(slider.value);
      document.getElementById('speedValue').textContent = speedSetting + '%';
    }
    
    function updateTurnManual() {
      currentTurn = parseInt(document.getElementById('turnSlider').value);
      document.getElementById('turnValue').textContent = currentTurn;
      sendCommand();
    }
    
    function emergencyStop() {
      fetch('/emergencyStop').then(() => {
        setNeutral();
        alert('🛑 Emergency Stop Activated!');
      }).catch(err => console.error(err));
    }
    
    function setTurnLeft() {
      currentTurn = -50;
      document.getElementById('turnSlider').value = currentTurn;
      document.getElementById('turnValue').textContent = currentTurn;
      sendCommand();
    }
    
    function setStraight() {
      currentTurn = 0;
      document.getElementById('turnSlider').value = currentTurn;
      document.getElementById('turnValue').textContent = currentTurn;
      sendCommand();
    }
    
    function setTurnRight() {
      currentTurn = 50;
      document.getElementById('turnSlider').value = currentTurn;
      document.getElementById('turnValue').textContent = currentTurn;
      sendCommand();
    }
    
    function updatePID() {
      document.getElementById('kpLval').value = document.getElementById('kpL').value;
      document.getElementById('kiLval').value = document.getElementById('kiL').value;
      document.getElementById('kdLval').value = document.getElementById('kdL').value;
      document.getElementById('kpRval').value = document.getElementById('kpR').value;
      document.getElementById('kiRval').value = document.getElementById('kiR').value;
      document.getElementById('kdRval').value = document.getElementById('kdR').value;
      sendPID();
    }
    
    function updatePIDFromInput() {
      document.getElementById('kpL').value = document.getElementById('kpLval').value;
      document.getElementById('kiL').value = document.getElementById('kiLval').value;
      document.getElementById('kdL').value = document.getElementById('kdLval').value;
      document.getElementById('kpR').value = document.getElementById('kpRval').value;
      document.getElementById('kiR').value = document.getElementById('kiRval').value;
      document.getElementById('kdR').value = document.getElementById('kdRval').value;
      sendPID();
    }
    
    function sendPID() {
      const kpL = document.getElementById('kpLval').value;
      const kiL = document.getElementById('kiLval').value;
      const kdL = document.getElementById('kdLval').value;
      const kpR = document.getElementById('kpRval').value;
      const kiR = document.getElementById('kiRval').value;
      const kdR = document.getElementById('kdRval').value;
      
      const url = '/setPID?kpL=' + kpL + '&kiL=' + kiL + '&kdL=' + kdL + '&kpR=' + kpR + '&kiR=' + kiR + '&kdR=' + kdR;
      fetch(url)
        .then(() => updateConnectionStatus(true))
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function loadPreset(name) {
      const p = presets[name];
      ['kpL', 'kpR'].forEach(id => {
        document.getElementById(id).value = p.kp;
        document.getElementById(id + 'val').value = p.kp;
      });
      ['kiL', 'kiR'].forEach(id => {
        document.getElementById(id).value = p.ki;
        document.getElementById(id + 'val').value = p.ki;
      });
      ['kdL', 'kdR'].forEach(id => {
        document.getElementById(id).value = p.kd;
        document.getElementById(id + 'val').value = p.kd;
      });
      sendPID();
    }
    
    function syncPID(direction) {
      if (direction === 'LtoR') {
        document.getElementById('kpR').value = document.getElementById('kpL').value;
        document.getElementById('kiR').value = document.getElementById('kiL').value;
        document.getElementById('kdR').value = document.getElementById('kdL').value;
        document.getElementById('kpRval').value = document.getElementById('kpLval').value;
        document.getElementById('kiRval').value = document.getElementById('kiLval').value;
        document.getElementById('kdRval').value = document.getElementById('kdLval').value;
      } else {
        document.getElementById('kpL').value = document.getElementById('kpR').value;
        document.getElementById('kiL').value = document.getElementById('kiR').value;
        document.getElementById('kdL').value = document.getElementById('kdR').value;
        document.getElementById('kpLval').value = document.getElementById('kpRval').value;
        document.getElementById('kiLval').value = document.getElementById('kiRval').value;
        document.getElementById('kdLval').value = document.getElementById('kdRval').value;
      }
      sendPID();
    }
    
    function updateFF() {
      document.getElementById('ffKvVal').value = document.getElementById('ffKv').value;
      document.getElementById('ffStaticVal').value = document.getElementById('ffStatic').value;
      sendFF();
    }
    
    function updateFFFromInput() {
      document.getElementById('ffKv').value = document.getElementById('ffKvVal').value;
      document.getElementById('ffStatic').value = document.getElementById('ffStaticVal').value;
      sendFF();
    }
    
    function sendFF() {
      const kv = document.getElementById('ffKvVal').value;
      const sf = document.getElementById('ffStaticVal').value;
      const url = '/setFF?kv=' + kv + '&sf=' + sf;
      fetch(url)
        .then(() => updateConnectionStatus(true))
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function updateTuning() {
      document.getElementById('leftTuneVal').value = document.getElementById('leftTune').value;
      document.getElementById('rightTuneVal').value = document.getElementById('rightTune').value;
      sendTuning();
    }
    
    function updateTuningFromInput() {
      document.getElementById('leftTune').value = document.getElementById('leftTuneVal').value;
      document.getElementById('rightTune').value = document.getElementById('rightTuneVal').value;
      sendTuning();
    }
    
    function sendTuning() {
      const left = document.getElementById('leftTuneVal').value;
      const right = document.getElementById('rightTuneVal').value;
      const url = '/setWheelTuning?left=' + left + '&right=' + right;
      fetch(url)
        .then(() => updateConnectionStatus(true))
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function updateWfParams() {
      document.getElementById('wfSpeedBox').value = document.getElementById('wfSpeedSlider').value;
      document.getElementById('wfSideBox').value = document.getElementById('wfSideSlider').value;
      document.getElementById('wfFrontBox').value = document.getElementById('wfFrontSlider').value;
      document.getElementById('wfMaxTurnRBox').value = document.getElementById('wfMaxTurnRSlider').value;
      document.getElementById('wfMaxTurnLBox').value = document.getElementById('wfMaxTurnLSlider').value;
      document.getElementById('wfDistKpBox').value = document.getElementById('wfDistKpSlider').value;
      document.getElementById('wfAngleKpBox').value = document.getElementById('wfAngleKpSlider').value;
      sendWfParams();
    }
    
    function updateWfParamsFromInput() {
      document.getElementById('wfSpeedSlider').value = document.getElementById('wfSpeedBox').value;
      document.getElementById('wfSideSlider').value = document.getElementById('wfSideBox').value;
      document.getElementById('wfFrontSlider').value = document.getElementById('wfFrontBox').value;
      document.getElementById('wfMaxTurnRSlider').value = document.getElementById('wfMaxTurnRBox').value;
      document.getElementById('wfMaxTurnLSlider').value = document.getElementById('wfMaxTurnLBox').value;
      document.getElementById('wfDistKpSlider').value = document.getElementById('wfDistKpBox').value;
      document.getElementById('wfAngleKpSlider').value = document.getElementById('wfAngleKpBox').value;
      sendWfParams();
    }
    
    function sendWfParams() {
      const speed = document.getElementById('wfSpeedBox').value;
      const side = document.getElementById('wfSideBox').value;
      const front = document.getElementById('wfFrontBox').value;
      const maxTurnR = document.getElementById('wfMaxTurnRBox').value;
      const maxTurnL = document.getElementById('wfMaxTurnLBox').value;
      const distKp = document.getElementById('wfDistKpBox').value;
      const angleKp = document.getElementById('wfAngleKpBox').value;
      
      const url = '/setTofParams?speed=' + speed + '&side=' + side + '&front=' + front + '&maxTurnR=' + maxTurnR + '&maxTurnL=' + maxTurnL + '&distKp=' + distKp + '&angleKp=' + angleKp;
      fetch(url)
        .then(() => updateConnectionStatus(true))
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function setTofDisplay(id, val, threshold) {
      const el = document.getElementById(id);
      if (val && val > 0) {
        el.textContent = val + ' mm';
        const threshMm = threshold * 10;
        if (val < threshMm * 0.5) {
          el.className = 'wf-tof-value danger';
        } else if (val < threshMm) {
          el.className = 'wf-tof-value warning';
        } else {
          el.className = 'wf-tof-value';
        }
      } else {
        el.textContent = '--';
        el.className = 'wf-tof-value';
      }
    }
    
    function startWallFollow() {
      sendWfParams();
      setTimeout(() => {
        fetch('/startWallFollow')
          .then(() => {
            updateConnectionStatus(true);
            const badge = document.getElementById('wfStatusBadge');
            badge.textContent = 'RUNNING';
            badge.className = 'wf-status-badge running';
          })
          .catch(err => { console.error(err); updateConnectionStatus(false); });
      }, 100);
    }
    
    function stopWallFollow() {
      fetch('/stopWallFollow')
        .then(() => {
          updateConnectionStatus(true);
          const badge = document.getElementById('wfStatusBadge');
          badge.textContent = 'STOPPED';
          badge.className = 'wf-status-badge stopped';
        })
        .catch(err => { console.error(err); updateConnectionStatus(false); });
    }
    
    function updateConnectionStatus(connected) {
      isConnected = connected;
      const status = document.getElementById('connStatus');
      if (connected) {
        status.className = 'connection-status connected';
        status.textContent = '● Connected';
      } else {
        status.className = 'connection-status disconnected';
        status.textContent = '● Disconnected';
      }
    }
    
    function updateStatus() {
      fetch('/getStatus')
        .then(r => r.json())
        .then(data => {
          updateConnectionStatus(true);
          
          document.getElementById('leftRPM').textContent = data.leftSpeed.toFixed(1);
          document.getElementById('leftTarget').textContent = data.leftTarget.toFixed(1);
          document.getElementById('leftError').textContent = data.leftError.toFixed(1);
          document.getElementById('leftPWM').textContent = data.leftPWM;
          document.getElementById('leftFF').textContent = data.leftFF || 'N/A';
          document.getElementById('leftCorr').textContent = data.leftCorr || 'N/A';
          
          document.getElementById('rightRPM').textContent = data.rightSpeed.toFixed(1);
          document.getElementById('rightTarget').textContent = data.rightTarget.toFixed(1);
          document.getElementById('rightError').textContent = data.rightError.toFixed(1);
          document.getElementById('rightPWM').textContent = data.rightPWM;
          document.getElementById('rightFF').textContent = data.rightFF || 'N/A';
          document.getElementById('rightCorr').textContent = data.rightCorr || 'N/A';
          
          ['leftError', 'rightError'].forEach(id => {
            const elem = document.getElementById(id);
            const val = Math.abs(parseFloat(elem.textContent));
            elem.className = val < 5 ? 'status-value good' : 'status-value error';
          });
          
          if (data.wfRunning !== undefined) {
            const badge = document.getElementById('wfStatusBadge');
            badge.textContent = data.wfRunning ? 'RUNNING' : 'STOPPED';
            badge.className = 'wf-status-badge ' + (data.wfRunning ? 'running' : 'stopped');
          }
          
          if (data.tofFront !== undefined) setTofDisplay('wfTofFront', data.tofFront, 30);
          if (data.tofLeft1 !== undefined) setTofDisplay('wfTofLeft1', data.tofLeft1, 20);
          if (data.tofLeft2 !== undefined) setTofDisplay('wfTofLeft2', data.tofLeft2, 20);
          
          if (data.wfState !== undefined) {
            const stateEl = document.getElementById('wfStateDisplay');
            stateEl.textContent = data.wfState;
            if (data.wfState === 'NORMAL') stateEl.style.color = '#38a169';
            else if (data.wfState === 'LOST_WALL') stateEl.style.color = '#dd6b20';
            else stateEl.style.color = '#e53e3e';
          }
          if (data.wallAngle !== undefined) {
            document.getElementById('wfAngleDisplay').textContent = data.wallAngle.toFixed(1) + '°';
          }
          if (data.distError !== undefined) {
            document.getElementById('wfDistErrDisplay').textContent = data.distError.toFixed(0) + ' mm';
          }
          if (data.turn !== undefined) {
            document.getElementById('wfTurnDisplay').textContent = data.turn;
          }
          
          if (data.viveX !== undefined) {
            document.getElementById('viveXLarge').textContent = data.viveX;
            document.getElementById('viveXInch').textContent = (data.viveX / 40).toFixed(1) + ' inch';
          }
          if (data.viveY !== undefined) {
            document.getElementById('viveYLarge').textContent = data.viveY;
            document.getElementById('viveYInch').textContent = (data.viveY / 40).toFixed(1) + ' inch';
          }
          if (data.viveValid !== undefined) {
            const signalLarge = document.getElementById('viveSignalLarge');
            const signalText = document.getElementById('viveSignalText');
            if (data.viveValid) {
              signalLarge.className = 'vive-signal-indicator valid';
              signalText.textContent = 'RECEIVING';
              signalText.className = 'vive-signal-text valid';
            } else {
              signalLarge.className = 'vive-signal-indicator invalid';
              signalText.textContent = 'NO SIGNAL';
              signalText.className = 'vive-signal-text invalid';
            }
          }
          if (data.viveHeading !== undefined) {
            document.getElementById('viveHeadingLarge').textContent = data.viveHeading.toFixed(1) + '°';
          }
          
          if (data.uartRawAX !== undefined && data.uartRawAY !== undefined) {
            document.getElementById('uartRawA').textContent = 'X:' + data.uartRawAX + ' Y:' + data.uartRawAY;
          }
          if (data.uartRawAValid !== undefined) {
            const el = document.getElementById('uartValidA');
            el.textContent = data.uartRawAValid ? '✓ Valid' : '✗ Invalid';
            el.className = 'vive-sensor-value ' + (data.uartRawAValid ? 'vive-reliable' : 'vive-unreliable');
          }
          if (data.uartRawBX !== undefined && data.uartRawBY !== undefined) {
            document.getElementById('uartRawB').textContent = 'X:' + data.uartRawBX + ' Y:' + data.uartRawBY;
          }
          if (data.uartRawBValid !== undefined) {
            const el = document.getElementById('uartValidB');
            el.textContent = data.uartRawBValid ? '✓ Valid' : '✗ Invalid';
            el.className = 'vive-sensor-value ' + (data.uartRawBValid ? 'vive-reliable' : 'vive-unreliable');
          }
          if (data.uartAge !== undefined) {
            document.getElementById('uartAge').textContent = data.uartAge;
          }
          
          if (data.navState !== undefined) {
            document.getElementById('navState').textContent = data.navState;
          }
          if (data.navTask !== undefined) {
            document.getElementById('navTask').textContent = data.navTask;
            if (data.navTask === 'SMART_NAV' && data.navTotal > 1) {
              document.getElementById('pathInfo').style.display = 'block';
            } else {
              document.getElementById('pathInfo').style.display = 'none';
            }
          }
          if (data.navTargetX !== undefined && data.navTargetY !== undefined) {
            document.getElementById('navTarget').textContent = '(' + data.navTargetX + ', ' + data.navTargetY + ')';
          }
          if (data.navDistance !== undefined) {
            document.getElementById('navDistance').textContent = data.navDistance.toFixed(0) + ' vive';
          }
          if (data.navProgress !== undefined && data.navTotal !== undefined) {
            document.getElementById('navProgress').textContent = data.navProgress + '/' + data.navTotal;
            document.getElementById('pathWaypoints').textContent = 'Waypoint ' + data.navProgress + ' of ' + data.navTotal;
          }
        })
        .catch(err => {
          console.error(err);
          updateConnectionStatus(false);
        });
    }
    
    selectTeam('blue');
    setInterval(updateStatus, 300);
    updateStatus();
  </script>
</body>
</html>
)=====";

#endif
