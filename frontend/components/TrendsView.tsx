"use client";

import { useState, useEffect } from "react";
import { useSimulation } from "../app/providers";
import { SimulationState } from "../lib/types";
import {
  formatLevel,
  formatFlowRate,
  formatTime,
} from "../lib/utils";

/**
 * TrendsView component displays historical simulation state updates
 * and provides a placeholder for future charting and analytics features.
 *
 * Maintains a rolling history of the last 10 state snapshots and
 * displays them in reverse chronological order (newest first).
 *
 * Displays:
 * - Last 10 state updates in a table
 * - Time, Level, Setpoint, Inlet Flow, Outlet Flow
 * - Placeholder message for future enhancements
 */
export function TrendsView() {
  const { state } = useSimulation();
  const [history, setHistory] = useState<SimulationState[]>([]);

  // Subscribe to state updates and maintain rolling history
  useEffect(() => {
    if (state !== null) {
      setHistory((prevHistory) => {
        // Add new state to beginning of array (reverse chronological)
        const updated = [state, ...prevHistory];
        // Keep only last 10 items
        return updated.slice(0, 10);
      });
    }
  }, [state]);

  return (
    <div className="w-full h-full flex flex-col">
      {/* Header section */}
      <div className="mb-6">
        <h2 className="text-2xl font-bold text-white mb-2">Trends View</h2>
        <p className="text-sm text-gray-400">
          Historical process trends and analytics
        </p>
      </div>

      {/* Placeholder message */}
      <div className="bg-blue-900 border border-blue-700 rounded-lg p-4 mb-6">
        <p className="text-sm text-blue-200">
          <span className="font-semibold">Placeholder:</span> Trend charts will be implemented in Phase 4 continued. Currently showing recent state updates to verify WebSocket connectivity.
        </p>
      </div>

      {/* Data history display */}
      {history.length === 0 ? (
        <div className="flex-1 flex items-center justify-center">
          <p className="text-gray-400 text-center">
            Waiting for state updates...
          </p>
        </div>
      ) : (
        <div className="bg-gray-800 rounded-lg p-6 overflow-auto flex-1">
          {/* Table header */}
          <table className="w-full text-sm">
            <thead>
              <tr className="border-b border-gray-700">
                <th className="text-left text-xs font-semibold text-gray-400 uppercase tracking-wide pb-3 px-3">
                  Time
                </th>
                <th className="text-right text-xs font-semibold text-gray-400 uppercase tracking-wide pb-3 px-3">
                  Level (m)
                </th>
                <th className="text-right text-xs font-semibold text-gray-400 uppercase tracking-wide pb-3 px-3">
                  Setpoint (m)
                </th>
                <th className="text-right text-xs font-semibold text-gray-400 uppercase tracking-wide pb-3 px-3">
                  Inlet (m³/s)
                </th>
                <th className="text-right text-xs font-semibold text-gray-400 uppercase tracking-wide pb-3 px-3">
                  Outlet (m³/s)
                </th>
              </tr>
            </thead>
            <tbody className="divide-y divide-gray-700">
              {history.map((item, index) => (
                <tr
                  key={index}
                  className="hover:bg-gray-700 transition-colors"
                >
                  <td className="py-3 px-3 font-mono text-white">
                    {formatTime(item.time)}
                  </td>
                  <td className="py-3 px-3 font-mono text-right text-white">
                    {formatLevel(item.tank_level)}
                  </td>
                  <td className="py-3 px-3 font-mono text-right text-white">
                    {formatLevel(item.setpoint)}
                  </td>
                  <td className="py-3 px-3 font-mono text-right text-white">
                    {formatFlowRate(item.inlet_flow)}
                  </td>
                  <td className="py-3 px-3 font-mono text-right text-white">
                    {formatFlowRate(item.outlet_flow)}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </div>
  );
}
