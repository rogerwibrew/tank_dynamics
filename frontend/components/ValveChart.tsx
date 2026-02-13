"use client";

import React from "react";
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
import { formatTime } from "../lib/utils";

/**
 * ValveChart component displays valve position (controller output) over time.
 * Uses a Recharts LineChart with a single series:
 * - Purple solid line for valve position (0-1 range, controller output)
 */
interface ValveChartProps {
  data: SimulationState[];
}

export default function ValveChart({ data }: ValveChartProps) {
  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg p-4">
      <h3 className="text-lg font-semibold text-white mb-4">
        Valve Position History
      </h3>

      <ResponsiveContainer width="100%" height={250}>
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
            domain={[0, 1]}
            label={{ value: "Valve Position", angle: -90, position: "insideLeft" }}
            stroke="#9ca3af"
            style={{ fontSize: 12 }}
          />

          <Tooltip
            contentStyle={{
              backgroundColor: "#1f2937",
              border: "1px solid #374151",
            }}
            labelStyle={{ color: "#9ca3af" }}
            itemStyle={{ color: "#fff" }}
            labelFormatter={(label: any) => formatTime(label)}
          />

          <Legend wrapperStyle={{ fontSize: 14 }} />

          <Line
            type="monotone"
            dataKey="valve_position"
            stroke="#a855f7"
            strokeWidth={2}
            dot={false}
            name="Valve Position"
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}
