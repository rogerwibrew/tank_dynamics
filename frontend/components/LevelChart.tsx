"use client";

import React, { useState, useCallback, memo } from "react";
import {
  ResponsiveContainer,
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  Legend,
} from "recharts";
import { SimulationState } from "../lib/types";
import { formatTime, formatLevel } from "../lib/utils";

/**
 * LevelChart component displays tank level and setpoint over time.
 * Uses a Recharts LineChart with two series:
 * - Blue solid line for actual tank level
 * - Red dashed line for setpoint (target level)
 */
interface LevelChartProps {
  data: SimulationState[];
}

const LEGEND_STYLE = { fontSize: 14, cursor: "pointer" };

function CustomTooltip({ active, payload, label }: any) {
  if (!active || !payload || !payload.length) return null;

  return (
    <div className="bg-gray-900 border border-gray-600 rounded p-3">
      <p className="text-gray-400 text-sm mb-2">{formatTime(label)}</p>
      {payload.map((entry: any, index: number) => (
        <div key={index} className="flex items-center gap-2 text-sm">
          <div
            className="w-3 h-3 rounded-full"
            style={{ backgroundColor: entry.color }}
          />
          <span className="text-white">
            {entry.name}: {formatLevel(entry.value)} m
          </span>
        </div>
      ))}
    </div>
  );
}

export default memo(function LevelChart({ data }: LevelChartProps) {
  const [hiddenLines, setHiddenLines] = useState<Record<string, boolean>>({
    tank_level: false,
    setpoint: false,
  });

  const handleLegendClick = useCallback((e: any) => {
    setHiddenLines((prev) => ({
      ...prev,
      [e.dataKey]: !prev[e.dataKey],
    }));
  }, []);

  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg p-4">
      <h3 className="text-lg font-semibold text-white mb-4">
        Tank Level History
      </h3>

      <ResponsiveContainer width="100%" height={300}>
        <LineChart
          data={data}
          margin={{ top: 5, right: 30, left: 20, bottom: 5 }}
        >
          <CartesianGrid strokeDasharray="3 3" stroke="#374151" />

          <XAxis
            dataKey="time"
            tickFormatter={formatTime}
            stroke="#9ca3af"
            style={{ fontSize: 12 }}
          />

          <YAxis
            domain={[0, 5]}
            label={{ value: "Level (m)", angle: -90, position: "insideLeft" }}
            stroke="#9ca3af"
            style={{ fontSize: 12 }}
          />

          <Tooltip content={<CustomTooltip />} />

          <Legend wrapperStyle={LEGEND_STYLE} onClick={handleLegendClick} />

          <Line
            type="monotone"
            dataKey="tank_level"
            stroke="#3b82f6"
            strokeWidth={2}
            dot={false}
            name="Level"
            hide={hiddenLines.tank_level}
          />

          <Line
            type="monotone"
            dataKey="setpoint"
            stroke="#ef4444"
            strokeWidth={2}
            strokeDasharray="5 5"
            dot={false}
            name="Setpoint"
            hide={hiddenLines.setpoint}
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
});
