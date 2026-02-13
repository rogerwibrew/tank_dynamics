"use client";

import { useState, useEffect } from "react";
import { useHistory } from "../hooks/useHistory";
import { useSimulation } from "../app/providers";
import { SimulationState } from "../lib/types";
import LevelChart from "./LevelChart";
import FlowsChart from "./FlowsChart";
import ValveChart from "./ValveChart";

/**
 * TrendsView component displays historical simulation state updates
 * as interactive charts spanning up to 1 hour of historical data.
 *
 * Fetches historical data from the backend via useHistory hook
 * and displays three charts:
 * - Tank level vs setpoint over time
 * - Inlet and outlet flow rates over time
 * - Controller output (valve position) over time
 *
 * Handles loading, error, and empty states appropriately.
 */
export function TrendsView() {
  const { history, loading, error } = useHistory(3600);
  const { state } = useSimulation();
  const [chartData, setChartData] = useState<SimulationState[]>([]);

  // Initialize chartData with historical data when it loads
  useEffect(() => {
    if (!loading && history.length > 0) {
      setChartData(history);
    }
  }, [history, loading]);

  // Append real-time WebSocket updates to chartData
  useEffect(() => {
    if (state && chartData.length > 0) {
      const latestTime = chartData[chartData.length - 1].time;

      // Only append if this is genuinely new data (newer timestamp)
      if (state.time > latestTime) {
        setChartData((prev) => {
          const updated = [...prev, state];
          // Limit to last 7200 entries (2 hours at 1Hz)
          return updated.slice(-7200);
        });
      }
    }
  }, [state, chartData.length]);

  return (
    <div className="w-full h-full flex flex-col">
      {/* Header section */}
      <div className="mb-6">
        <h2 className="text-2xl font-bold text-white mb-2">Trends View</h2>
        <p className="text-sm text-gray-400">
          Historical process trends and analytics
        </p>
      </div>

      {/* Loading state */}
      {loading && (
        <div className="flex-1 flex items-center justify-center">
          <p className="text-gray-400 text-center">
            Loading historical data...
          </p>
        </div>
      )}

      {/* Error state */}
      {error && !loading && (
        <div className="flex-1 flex items-center justify-center">
          <p className="text-red-400 text-center">{error}</p>
        </div>
      )}

      {/* Empty state */}
      {!loading && !error && chartData.length === 0 && (
        <div className="flex-1 flex items-center justify-center">
          <p className="text-gray-400 text-center">
            No historical data available
          </p>
        </div>
      )}

      {/* Charts */}
      {!loading && !error && chartData.length > 0 && (
        <div className="flex-1 overflow-auto space-y-4">
          {/* Level Chart */}
          <div className="bg-gray-800 rounded-lg p-4">
            <h3 className="text-lg font-semibold text-white mb-3">
              Tank Level vs Setpoint
            </h3>
            <LevelChart data={chartData} />
          </div>

          {/* Flows Chart */}
          <div className="bg-gray-800 rounded-lg p-4">
            <h3 className="text-lg font-semibold text-white mb-3">
              Inlet and Outlet Flows
            </h3>
            <FlowsChart data={chartData} />
          </div>

          {/* Valve Chart */}
          <div className="bg-gray-800 rounded-lg p-4">
            <h3 className="text-lg font-semibold text-white mb-3">
              Controller Output (Valve Position)
            </h3>
            <ValveChart data={chartData} />
          </div>
        </div>
      )}
    </div>
  );
}
