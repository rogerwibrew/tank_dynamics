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
import { formatTime, formatFlowRate } from "../lib/utils";
import { ChartTooltip } from "./ChartTooltip";

/**
 * FlowsChart component displays inlet and outlet flows over time.
 * Uses a Recharts LineChart with two series:
 * - Cyan solid line for inlet flow (manipulated variable)
 * - Orange solid line for outlet flow (process response)
 */
interface FlowsChartProps {
  data: SimulationState[];
}

const LEGEND_STYLE = { fontSize: 14, cursor: "pointer" };

export default memo(function FlowsChart({ data }: FlowsChartProps) {
  const [hiddenLines, setHiddenLines] = useState<Record<string, boolean>>({
    inlet_flow: false,
    outlet_flow: false,
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
            label={{
              value: "Flow Rate (m³/s)",
              angle: -90,
              position: "insideLeft",
            }}
            stroke="#9ca3af"
            style={{ fontSize: 12 }}
          />

          <Tooltip
            content={
              <ChartTooltip
                formatter={(value) => `${formatFlowRate(value)} m³/s`}
              />
            }
          />

          <Legend wrapperStyle={LEGEND_STYLE} onClick={handleLegendClick} />

          <Line
            type="linear"
            dataKey="inlet_flow"
            stroke="#06b6d4"
            strokeWidth={2}
            dot={false}
            name="Inlet Flow"
            hide={hiddenLines.inlet_flow}
          />

          <Line
            type="linear"
            dataKey="outlet_flow"
            stroke="#f97316"
            strokeWidth={2}
            dot={false}
            name="Outlet Flow"
            hide={hiddenLines.outlet_flow}
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
});
