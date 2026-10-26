import React, { useState, useEffect, useRef, useMemo } from 'react';
import { Play, Pause, RefreshCw, Activity, Battery, Zap, Wifi, WifiOff, AlertTriangle } from 'lucide-react';

// --- Simple 1D Kalman Filter Implementation ---
class KalmanFilter {
  constructor(R = 1, Q = 1, A = 1, B = 0, C = 1) {
    this.R = R; // Noise covariance (Measurement Noise)
    this.Q = Q; // Process covariance (Environment Noise)
    this.A = A; // State vector
    this.B = B; // Control vector
    this.C = C; // Measurement vector
    this.cov = NaN;
    this.x = NaN; // Estimated value
  }

  filter(z, u = 0) {
    if (isNaN(this.x)) {
      this.x = (1 / this.C) * z;
      this.cov = (1 / this.C) * this.R * (1 / this.C);
    } else {
      // Prediction
      const predX = (this.A * this.x) + (this.B * u);
      const predCov = ((this.A * this.cov) * this.A) + this.Q;

      // Update
      const K = predCov * this.C * (1 / ((this.C * predCov * this.C) + this.R));
      this.x = predX + K * (z - (this.C * predX));
      this.cov = predCov - (K * this.C * predCov);
    }
    return this.x;
  }
}

// --- Helper for generating Gaussian noise ---
const gaussianRandom = (mean = 0, stdev = 1) => {
  const u = 1 - Math.random();
  const v = Math.random();
  const z = Math.sqrt(-2.0 * Math.log(u)) * Math.cos(2.0 * Math.PI * v);
  return z * stdev + mean;
};

// --- Mock Route Data (Waypoints) ---
const ROUTE_POINTS = [
  { x: 50, y: 200, label: 'Start' },
  { x: 150, y: 150, label: 'Waypoint 1' },
  { x: 300, y: 250, label: 'Hill Climb' },
  { x: 500, y: 100, label: 'City' },
  { x: 700, y: 200, label: 'End' },
];

const App = () => {
  // Simulation State
  const [isRunning, setIsRunning] = useState(false);
  const [realSoc, setRealSoc] = useState(100);
  const [estimatedSoc, setEstimatedSoc] = useState(100);
  const [noisySoc, setNoisySoc] = useState(100);
  const [carPos, setCarPos] = useState({ x: 50, y: 200 });
  const [step, setStep] = useState(0);
  
  // Robustness Settings
  const [commError, setCommError] = useState(false);
  const [highNoise, setHighNoise] = useState(false);

  // History for graphing
  const [history, setHistory] = useState([]);

  // Refs for logic that doesn't need to trigger re-renders directly
  const kfRef = useRef(new KalmanFilter(0.1, 0.1)); // Tuned for this simulation
  const requestRef = useRef();

  // Reset Simulation
  const handleReset = () => {
    setIsRunning(false);
    setRealSoc(100);
    setEstimatedSoc(100);
    setNoisySoc(100);
    setCarPos(ROUTE_POINTS[0]);
    setStep(0);
    setHistory([]);
    kfRef.current = new KalmanFilter(0.1, 0.1);
  };

  // Interpolate position between waypoints
  const getPositionAtStep = (s) => {
    const totalSteps = 400; 
    const segmentSize = totalSteps / (ROUTE_POINTS.length - 1);
    const segmentIndex = Math.floor(s / segmentSize);
    
    if (segmentIndex >= ROUTE_POINTS.length - 1) return ROUTE_POINTS[ROUTE_POINTS.length - 1];

    const p1 = ROUTE_POINTS[segmentIndex];
    const p2 = ROUTE_POINTS[segmentIndex + 1];
    const progress = (s % segmentSize) / segmentSize;

    return {
      x: p1.x + (p2.x - p1.x) * progress,
      y: p1.y + (p2.y - p1.y) * progress,
    };
  };

  // Main Simulation Loop
  const updateSimulation = () => {
    if (!isRunning) return;

    setStep((prevStep) => {
      const nextStep = prevStep + 1;
      
      if (nextStep > 400) {
        setIsRunning(false);
        return prevStep;
      }

      // 1. Move Car
      const newPos = getPositionAtStep(nextStep);
      setCarPos(newPos);

      // 2. Simulate Physics (Battery Drain)
      // Drains faster on "uphills" (y decreasing in screen coords means up usually, but lets say y increasing is drag)
      // Let's just say random load + base load
      const baseDrain = 0.05;
      const loadFactor = highNoise ? 0.2 : 0.02; // High noise adds physics jitter
      const currentDrain = baseDrain + Math.random() * loadFactor;
      
      setRealSoc((prevReal) => {
        const nextReal = Math.max(0, prevReal - currentDrain);
        
        // 3. Simulate Sensor Noise & Comm Failure
        let measurementNoise = gaussianRandom(0, 1.5); // Normal noise
        if (highNoise) measurementNoise = gaussianRandom(0, 5.0); // Extreme sensor noise

        // Simulating packet loss/comm error: 
        // If Comm Error is ON, we might get a "stuck" value (0 update) or a "bad packet" (0 value)
        // Here we simulate a noisy transmission channel
        let measuredValue = nextReal + measurementNoise;
        
        if (commError) {
             // 10% chance of dropped packet (value becomes 0 or holds previous)
             // or huge interference spike
             if (Math.random() < 0.2) measuredValue = 0; // Dropped packet
             if (Math.random() < 0.1) measuredValue = nextReal + 20; // Glitch
        }

        setNoisySoc(measuredValue);

        // 4. Apply Kalman Filter
        // Note: Real KFs usually handle "control inputs" (u). 
        // Here we simplify: The KF tries to find the true state amidst the noise.
        // We can make the KF robust to dropped packets by ignoring Zeros if we want,
        // or let the KF do its job smoothing it.
        const estimate = kfRef.current.filter(measuredValue);
        setEstimatedSoc(estimate);

        setHistory((prev) => {
          const newHistory = [...prev, { step: nextStep, real: nextReal, measured: measuredValue, estimated: estimate }];
          if (newHistory.length > 100) newHistory.shift(); // Keep last 100 pts
          return newHistory;
        });

        return nextReal;
      });

      return nextStep;
    });
  };

  // Animation Loop
  useEffect(() => {
    if (isRunning) {
      const interval = setInterval(updateSimulation, 50);
      return () => clearInterval(interval);
    }
  }, [isRunning, commError, highNoise]);


  // --- Visualization Helpers ---
  const GraphLine = ({ data, color, dataKey, offset = 0 }) => {
    if (data.length < 2) return null;
    const maxStep = 100; // Window size
    const height = 100;
    const width = 300;
    
    // Scale points to SVG
    const points = data.map((d, i) => {
        const x = (i / (maxStep - 1)) * width;
        // Map SOC 0-100 to y (100-0)
        let val = d[dataKey];
        // Clamp for visuals
        val = Math.max(0, Math.min(120, val));
        const y = height - (val / 120) * height; 
        return `${x},${y}`;
    }).join(' ');

    return <polyline fill="none" stroke={color} strokeWidth="2" points={points} />;
  };

  return (
    <div className="flex flex-col h-screen bg-slate-50 font-sans text-slate-800 overflow-hidden">
      
      {/* Header */}
      <div className="bg-white p-4 shadow-sm border-b border-slate-200 flex justify-between items-center z-10">
        <div>
          <h1 className="text-xl font-bold flex items-center gap-2">
            <Activity className="text-blue-600" />
            BMS State Estimation Prototype
          </h1>
          <p className="text-sm text-slate-500">EV Routing Context • Kalman Filter • Robust Comm</p>
        </div>
        
        <div className="flex gap-2">
           <button 
            onClick={() => setIsRunning(!isRunning)}
            className={`flex items-center gap-2 px-4 py-2 rounded-lg font-medium transition ${isRunning ? 'bg-amber-100 text-amber-700 hover:bg-amber-200' : 'bg-blue-600 text-white hover:bg-blue-700'}`}
          >
            {isRunning ? <><Pause size={18} /> Pause</> : <><Play size={18} /> Start Simulation</>}
          </button>
          <button 
            onClick={handleReset}
            className="flex items-center gap-2 px-4 py-2 bg-slate-100 text-slate-600 rounded-lg hover:bg-slate-200 font-medium transition"
          >
            <RefreshCw size={18} /> Reset
          </button>
        </div>
      </div>

      <div className="flex flex-1 overflow-hidden">
        
        {/* Left Panel: Visual Routing */}
        <div className="w-2/3 relative bg-slate-200 overflow-hidden">
            {/* Background Map Grid */}
            <div className="absolute inset-0 opacity-10" 
                 style={{ backgroundImage: 'radial-gradient(#64748b 1px, transparent 1px)', backgroundSize: '20px 20px' }}>
            </div>

            {/* Render Route */}
            <svg className="absolute inset-0 w-full h-full pointer-events-none">
                <polyline 
                    points={ROUTE_POINTS.map(p => `${p.x},${p.y}`).join(' ')} 
                    fill="none" 
                    stroke="#cbd5e1" 
                    strokeWidth="8" 
                    strokeLinecap="round" 
                    strokeLinejoin="round"
                />
                 <polyline 
                    points={ROUTE_POINTS.map(p => `${p.x},${p.y}`).join(' ')} 
                    fill="none" 
                    stroke="#3b82f6" 
                    strokeWidth="3" 
                    strokeDasharray="10,5"
                    strokeLinecap="round" 
                    strokeLinejoin="round"
                />
                {/* Waypoints */}
                {ROUTE_POINTS.map((pt, i) => (
                    <circle key={i} cx={pt.x} cy={pt.y} r="6" fill="#3b82f6" />
                ))}
            </svg>

            {/* The EV */}
            <div 
                className="absolute w-12 h-12 bg-white rounded-full shadow-xl border-4 border-blue-600 flex items-center justify-center transform -translate-x-1/2 -translate-y-1/2 transition-all duration-75"
                style={{ left: carPos.x, top: carPos.y }}
            >
                <div className="text-xs font-bold text-blue-800">EV</div>
            </div>

            {/* Floating Stats on Map */}
            <div className="absolute top-4 left-4 bg-white/90 backdrop-blur p-3 rounded-lg shadow-lg border border-slate-200">
                <div className="text-xs font-semibold text-slate-500 uppercase mb-1">Current State</div>
                <div className="flex items-center gap-4">
                    <div>
                        <div className="text-xs text-slate-400">Actual SOC</div>
                        <div className="text-lg font-bold text-slate-800">{realSoc.toFixed(1)}%</div>
                    </div>
                    <div>
                        <div className="text-xs text-blue-500 font-semibold">Estimated (KF)</div>
                        <div className="text-lg font-bold text-blue-600">{estimatedSoc.toFixed(1)}%</div>
                    </div>
                </div>
            </div>
            
            {/* Communication Status Indicator */}
            <div className={`absolute top-4 right-4 p-3 rounded-lg shadow-lg border backdrop-blur transition-all ${commError ? 'bg-red-50/90 border-red-200' : 'bg-emerald-50/90 border-emerald-200'}`}>
                <div className="flex items-center gap-2">
                    {commError ? <WifiOff size={20} className="text-red-500 animate-pulse" /> : <Wifi size={20} className="text-emerald-500" />}
                    <span className={`font-bold ${commError ? 'text-red-700' : 'text-emerald-700'}`}>
                        {commError ? 'Packet Loss / Interference' : 'Communication Stable'}
                    </span>
                </div>
            </div>
        </div>

        {/* Right Panel: Controls & Graphs */}
        <div className="w-1/3 bg-white border-l border-slate-200 flex flex-col">
            
            {/* Controls Section */}
            <div className="p-6 border-b border-slate-100">
                <h2 className="text-sm font-bold uppercase text-slate-400 mb-4">Simulation Parameters</h2>
                
                <div className="space-y-4">
                    <label className="flex items-center justify-between p-3 border rounded-lg hover:bg-slate-50 cursor-pointer transition">
                        <div className="flex items-center gap-3">
                            <div className={`p-2 rounded-md ${highNoise ? 'bg-amber-100 text-amber-600' : 'bg-slate-100 text-slate-500'}`}>
                                <Zap size={18} />
                            </div>
                            <div>
                                <div className="font-semibold text-slate-700">High Sensor Noise</div>
                                <div className="text-xs text-slate-500">Inject Gaussian noise to measurements</div>
                            </div>
                        </div>
                        <input type="checkbox" checked={highNoise} onChange={(e) => setHighNoise(e.target.checked)} className="h-5 w-5 accent-blue-600" />
                    </label>

                    <label className="flex items-center justify-between p-3 border rounded-lg hover:bg-slate-50 cursor-pointer transition">
                        <div className="flex items-center gap-3">
                            <div className={`p-2 rounded-md ${commError ? 'bg-red-100 text-red-600' : 'bg-slate-100 text-slate-500'}`}>
                                <AlertTriangle size={18} />
                            </div>
                            <div>
                                <div className="font-semibold text-slate-700">Network Instability</div>
                                <div className="text-xs text-slate-500">Simulate packet drops & latency</div>
                            </div>
                        </div>
                        <input type="checkbox" checked={commError} onChange={(e) => setCommError(e.target.checked)} className="h-5 w-5 accent-blue-600" />
                    </label>
                </div>
            </div>

            {/* Graphs Section */}
            <div className="flex-1 p-6 overflow-y-auto">
                <h2 className="text-sm font-bold uppercase text-slate-400 mb-2">Real-time Data Stream</h2>
                
                <div className="mb-6">
                    <div className="flex justify-between items-end mb-2">
                         <span className="text-xs font-semibold text-slate-600">SOC Estimation (Blue = Filtered)</span>
                         <div className="flex gap-2 text-xs">
                             <span className="flex items-center gap-1"><span className="w-2 h-2 rounded-full bg-slate-300"></span> Real</span>
                             <span className="flex items-center gap-1"><span className="w-2 h-2 rounded-full bg-red-400 opacity-50"></span> Raw</span>
                             <span className="flex items-center gap-1"><span className="w-2 h-2 rounded-full bg-blue-600"></span> Kalman</span>
                         </div>
                    </div>
                    <div className="h-32 bg-slate-50 border border-slate-100 rounded-lg relative overflow-hidden">
                        {/* Legends/Grid lines */}
                        <div className="absolute inset-0 flex flex-col justify-between p-2 pointer-events-none opacity-30">
                            <div className="border-t border-slate-300 w-full h-0"></div>
                            <div className="border-t border-slate-300 w-full h-0"></div>
                            <div className="border-t border-slate-300 w-full h-0"></div>
                        </div>

                        {/* Chart */}
                        <svg className="w-full h-full" viewBox="0 0 300 100" preserveAspectRatio="none">
                            {/* Raw / Noisy Data */}
                            <GraphLine data={history} dataKey="measured" color="#f87171" />
                             {/* Real Data */}
                            <GraphLine data={history} dataKey="real" color="#cbd5e1" />
                            {/* Kalman Filtered Data */}
                            <GraphLine data={history} dataKey="estimated" color="#2563eb" />
                        </svg>
                    </div>
                </div>

                {/* Console Log Area */}
                <div className="bg-slate-900 text-green-400 p-4 rounded-lg font-mono text-xs h-48 overflow-hidden flex flex-col">
                    <div className="mb-2 uppercase text-slate-500 font-bold tracking-wider">System Logs</div>
                    <div className="flex-1 overflow-hidden flex flex-col-reverse gap-1">
                        {history.slice(-8).reverse().map((h, i) => (
                            <div key={i} className="opacity-90">
                                <span className="text-slate-500">[{h.step}]</span> Raw: {h.measured.toFixed(1)} | Est: {h.estimated.toFixed(1)} | <span className={Math.abs(h.real - h.estimated) < 2 ? 'text-green-400' : 'text-yellow-400'}>Diff: {(h.estimated - h.real).toFixed(2)}</span>
                            </div>
                        ))}
                        {history.length === 0 && <div className="text-slate-600 italic">Waiting to start...</div>}
                    </div>
                </div>

            </div>
        </div>
      </div>
    </div>
  );
};

export default App;