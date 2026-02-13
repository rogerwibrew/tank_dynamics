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
 * FlowsChart component displays inlet and outlet flows over time.
 * Uses a Recharts LineChart with two series:
 * - Cyan solid line for inlet flow (manipulated variable)
 * - Orange solid line for outlet flow (process response)
 */
interface FlowsChartProps {
  data: SimulationState[];
}

export default function FlowsChart({ data }: FlowsChartProps) {
  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg p-4">
      <h3 className="text-lg font-semibold text-white mb-4">
        Flow Rates History
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
            domain={[0, 2]}
            label={{ value: "Flow Rate (m³/s)", angle: -90, position: "insideLeft" }}
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
            dataKey="inlet_flow"
            stroke="#06b6d4"
            strokeWidth={2}
            dot={false}
            name="Inlet Flow"
          />

          <Line
            type="monotone"
            dataKey="outlet_flow"
            stroke="#f97316"
            strokeWidth={2}
            dot={false}
            name="Outlet Flow"
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}
