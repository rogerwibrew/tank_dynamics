"use client";

import React from "react";
import { useSimulation } from "../app/providers";
import { ConnectionStatus } from "./ConnectionStatus";
import TankGraphic from "./TankGraphic";
import SetpointControl from "./SetpointControl";
import PIDTuningControl from "./PIDTuningControl";
import InletFlowControl from "./InletFlowControl";
import {
  formatLevel,
  formatFlowRate,
  formatValvePosition,
  formatTime,
} from "../lib/utils";

/**
 * ProcessView component displays current simulation state and provides
 * a placeholder for tank visualization and real-time controls.
 *
 * Displays:
 * - Simulation time
 * - Tank level and setpoint
 * - Inlet and outlet flow rates
 * - Valve position
 * - Connection status
 * - Waiting message when disconnected
 */
export function ProcessView() {
  const { state, setSetpoint, setPIDGains, setInletFlow, setInletMode } =
    useSimulation();
  const [currentPIDGains, setCurrentPIDGains] = React.useState({
    Kc: 1.0,
    tau_I: 10.0,
    tau_D: 1.0,
  });
  const [reverseActing, setReverseActing] = React.useState(true);

  // Fetch PID gains from backend on mount
  React.useEffect(() => {
    const fetchPIDGains = async () => {
      try {
        const response = await fetch("/api/config");
        if (!response.ok) {
          console.error(
            "Failed to fetch config:",
            response.status,
            response.statusText,
          );
          return;
        }
        const data = await response.json();
        if (data.pid_gains) {
          const { Kc, tau_I, tau_D } = data.pid_gains;
          // Display positive value; determine reverse acting from sign
          setCurrentPIDGains({
            Kc: Math.abs(Kc),
            tau_I,
            tau_D,
          });
          setReverseActing(Kc < 0);
        }
      } catch (error) {
        console.error("Error fetching PID gains:", error);
        // Keep default hardcoded values if fetch fails
      }
    };

    fetchPIDGains();
  }, []);

  const handleSetpointChange = (newValue: number) => {
    setSetpoint(newValue);
  };

  const handlePIDChange = (newGains: {
    Kc: number;
    tau_I: number;
    tau_D: number;
  }) => {
    // newGains.Kc is already negated by PIDTuningControl when reverse acting
    setPIDGains(newGains.Kc, newGains.tau_I, newGains.tau_D);
    // Store the positive display value for the UI
    setCurrentPIDGains({
      Kc: Math.abs(newGains.Kc),
      tau_I: newGains.tau_I,
      tau_D: newGains.tau_D,
    });
  };

  const handleFlowChange = (value: number) => {
    setInletFlow(value);
  };

  const handleModeChange = (
    mode: "constant" | "brownian",
    config?: { min: number; max: number; variance: number },
  ) => {
    if (mode === "constant") {
      setInletMode("constant", 0, 0, 0);
    } else if (config) {
      setInletMode(mode, config.min, config.max, config.variance);
    }
  };

  return (
    <div className="w-full h-full flex flex-col">
      {/* Header section with title and connection status */}
      <div className="flex items-start justify-between mb-6">
        <div>
          <h2 className="text-2xl font-bold text-white mb-2">Process View</h2>
          <p className="text-sm text-gray-400">
            Tank visualization and real-time controls (coming next)
          </p>
        </div>
        <ConnectionStatus />
      </div>

      {/* Main content section */}
      {state === null ? (
        /* Waiting message when disconnected */
        <div className="flex-1 flex items-center justify-center">
          <p className="text-gray-400 text-center">
            Waiting for WebSocket connection...
          </p>
        </div>
      ) : (
        /* Two-column layout: tank graphic and data display */
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-8">
          {/* Left column: Tank Graphic */}
          <div className="flex justify-center">
            <div className="w-full max-w-md">
              <TankGraphic
                level={state.tank_level}
                setpoint={state.setpoint}
                maxHeight={5.0}
                inletFlow={state.inlet_flow}
                outletFlow={state.outlet_flow}
              />
            </div>
          </div>

          {/* Right column: Data display section */}
          <div className="bg-gray-800 rounded-lg p-6 space-y-4">
            {/* Simulation Time */}
            <div className="grid grid-cols-2 gap-4 pb-4 border-b border-gray-700">
              <div>
                <label className="text-xs font-semibold text-gray-400 uppercase tracking-wide">
                  Simulation Time
                </label>
                <p className="text-lg font-mono text-white mt-1">
                  {formatTime(state.time)}
                </p>
              </div>
            </div>

            {/* Tank Level and Setpoint */}
            <div className="grid grid-cols-2 gap-4 pb-4 border-b border-gray-700">
              <div>
                <label className="text-xs font-semibold text-gray-400 uppercase tracking-wide">
                  Tank Level
                </label>
                <p className="text-lg font-mono text-white mt-1">
                  {formatLevel(state.tank_level)} m
                </p>
              </div>
              <div>
                <label className="text-xs font-semibold text-gray-400 uppercase tracking-wide">
                  Setpoint
                </label>
                <p className="text-lg font-mono text-white mt-1">
                  {formatLevel(state.setpoint)} m
                </p>
              </div>
            </div>

            {/* Flow Rates */}
            <div className="grid grid-cols-2 gap-4 pb-4 border-b border-gray-700">
              <div>
                <label className="text-xs font-semibold text-gray-400 uppercase tracking-wide">
                  Inlet Flow
                </label>
                <p className="text-lg font-mono text-white mt-1">
                  {formatFlowRate(state.inlet_flow)} m³/s
                </p>
              </div>
              <div>
                <label className="text-xs font-semibold text-gray-400 uppercase tracking-wide">
                  Outlet Flow
                </label>
                <p className="text-lg font-mono text-white mt-1">
                  {formatFlowRate(state.outlet_flow)} m³/s
                </p>
              </div>
            </div>

            {/* Valve Position */}
            <div className="grid grid-cols-2 gap-4 pb-4 border-b border-gray-700">
              <div>
                <label className="text-xs font-semibold text-gray-400 uppercase tracking-wide">
                  Valve Position
                </label>
                <p className="text-lg font-mono text-white mt-1">
                  {formatValvePosition(state.valve_position)}
                </p>
              </div>
            </div>

            {/* Setpoint Control */}
            <div className="pt-4 border-t border-gray-700">
              <SetpointControl
                currentSetpoint={state.setpoint}
                currentLevel={state.tank_level}
                onSetpointChange={handleSetpointChange}
              />
            </div>

            {/* PID Tuning Control */}
            <div className="pt-4 border-t border-gray-700">
              <PIDTuningControl
                currentGains={currentPIDGains}
                reverseActing={reverseActing}
                onGainsChange={handlePIDChange}
                onReverseActingChange={setReverseActing}
              />
            </div>

            {/* Inlet Flow Control */}
            <div className="pt-4 border-t border-gray-700">
              <InletFlowControl
                currentFlow={state.inlet_flow}
                onFlowChange={handleFlowChange}
                onModeChange={handleModeChange}
              />
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
