"use client";

import { useState, useEffect, useRef, useMemo } from "react";
import { useHistory } from "../hooks/useHistory";
import { useSimulation } from "../app/providers";
import { SimulationState } from "../lib/types";
import { downsample } from "../lib/utils";
import LevelChart from "./LevelChart";
import FlowsChart from "./FlowsChart";
import ValveChart from "./ValveChart";
import { ChartErrorBoundary } from "./ChartErrorBoundary";

const MAX_CHART_POINTS = 500;

/**
 * TrendsView component displays historical simulation state updates
 * as interactive charts with configurable time ranges (1 min to 2 hours).
 *
 * Fetches historical data from the backend via useHistory hook
 * and displays three charts:
 * - Tank level vs setpoint over time
 * - Inlet and outlet flow rates over time
 * - Controller output (valve position) over time
 *
 * Features a time range selector to control how much historical data
 * to display (1 min, 5 min, 30 min, 1 hr, or 2 hr).
 *
 * Handles loading, error, and empty states appropriately.
 */
export function TrendsView() {
  const [duration, setDuration] = useState(3600); // Default: 1 hour
  const { history, loading, error } = useHistory(duration);
  const { state } = useSimulation();
  const [chartData, setChartData] = useState<SimulationState[]>([]);
  const latestTimeRef = useRef<number>(-Infinity);

  const TIME_RANGES = [
    { label: "1 min", value: 60 },
    { label: "5 min", value: 300 },
    { label: "30 min", value: 1800 },
    { label: "1 hr", value: 3600 },
    { label: "2 hr", value: 7200 },
  ];

  // Initialize chartData with historical data when it loads
  useEffect(() => {
    if (!loading && history.length > 0) {
      setChartData(history);
      latestTimeRef.current = history[history.length - 1].time;
    }
  }, [history, loading]);

  // Append real-time WebSocket updates to chartData
  useEffect(() => {
    if (state && state.time > latestTimeRef.current) {
      latestTimeRef.current = state.time;
      setChartData((prev) => {
        if (prev.length === 0) return prev;
        const updated = [...prev, state];
        // Limit to last 7200 entries (2 hours at 1Hz)
        if (updated.length > 7200) {
          return updated.slice(-7200);
        }
        return updated;
      });
    }
  }, [state]);

  // Stable downsampling using FIXED decimation factor.
  // Key: Use a constant decimation regardless of current data length.
  // This prevents previously rendered points from shifting positions.
  const displayData = useMemo(() => {
    if (chartData.length <= MAX_CHART_POINTS) {
      return chartData;
    }

    // Fixed decimation factor: designed for max buffer size (7200 points)
    // 7200 / 500 = ~14, so we take every 14th point
    // This factor remains constant, ensuring stable rendering
    const DECIMATION = 14;

    const result: SimulationState[] = [];

    // Take every DECIMATION-th point starting from index 0
    for (let i = 0; i < chartData.length; i += DECIMATION) {
      result.push(chartData[i]);
    }

    // Always include the very last point for current state accuracy
    const lastPoint = chartData[chartData.length - 1];
    if (result[result.length - 1] !== lastPoint) {
      result.push(lastPoint);
    }

    return result;
  }, [chartData]);

  return (
    <div className="w-full h-full flex flex-col">
      {/* Header section */}
      <div className="mb-6">
        <h2 className="text-2xl font-bold text-white mb-2">Trends View</h2>
        <p className="text-sm text-gray-400">
          Historical process trends and analytics
        </p>
      </div>

      {/* Time Range Selector */}
      <div className="mb-6">
        <label className="block text-sm font-medium text-gray-300 mb-2">
          Time Range
        </label>
        <div className="flex gap-2">
          {TIME_RANGES.map((range) => (
            <button
              key={range.value}
              onClick={() => setDuration(range.value)}
              className={`px-4 py-2 rounded font-medium transition-colors cursor-pointer ${
                duration === range.value
                  ? "bg-blue-600 text-white"
                  : "bg-gray-700 text-gray-300 hover:bg-gray-600"
              }`}
            >
              {range.label}
            </button>
          ))}
        </div>
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
      {!loading && !error && displayData.length === 0 && (
        <div className="flex-1 flex items-center justify-center">
          <p className="text-gray-400 text-center">
            No historical data available
          </p>
        </div>
      )}

      {/* Charts */}
      {!loading && !error && displayData.length > 0 && (
        <div className="flex-1 overflow-auto space-y-4">
          {/* Level Chart */}
          <div className="bg-gray-800 rounded-lg p-4">
            <h3 className="text-lg font-semibold text-white mb-3">
              Tank Level vs Setpoint
            </h3>
            <ChartErrorBoundary chartName="Level Chart">
              <LevelChart data={displayData} />
            </ChartErrorBoundary>
          </div>

          {/* Flows Chart */}
          <div className="bg-gray-800 rounded-lg p-4">
            <h3 className="text-lg font-semibold text-white mb-3">
              Inlet and Outlet Flows
            </h3>
            <ChartErrorBoundary chartName="Flows Chart">
              <FlowsChart data={displayData} />
            </ChartErrorBoundary>
          </div>

          {/* Valve Chart */}
          <div className="bg-gray-800 rounded-lg p-4">
            <h3 className="text-lg font-semibold text-white mb-3">
              Controller Output (Valve Position)
            </h3>
            <ChartErrorBoundary chartName="Valve Chart">
              <ValveChart data={displayData} />
            </ChartErrorBoundary>
          </div>
        </div>
      )}
    </div>
  );
}
