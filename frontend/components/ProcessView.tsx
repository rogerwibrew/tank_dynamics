"use client";

import { useSimulation } from "../app/providers";
import { ConnectionStatus } from "./ConnectionStatus";
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
  const { state } = useSimulation();

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
        /* Data display section */
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
          <div className="grid grid-cols-2 gap-4">
            <div>
              <label className="text-xs font-semibold text-gray-400 uppercase tracking-wide">
                Valve Position
              </label>
              <p className="text-lg font-mono text-white mt-1">
                {formatValvePosition(state.valve_position)}
              </p>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
