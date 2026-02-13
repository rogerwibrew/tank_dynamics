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
import { formatTime, formatValvePosition } from "../lib/utils";
import { ChartTooltip } from "./ChartTooltip";

/**
 * ValveChart component displays valve position (controller output) over time.
 * Uses a Recharts LineChart with a single series:
 * - Purple solid line for valve position (0-1 range, controller output)
 */
interface ValveChartProps {
  data: SimulationState[];
}

const LEGEND_STYLE = { fontSize: 14, cursor: "pointer" };

export default memo(function ValveChart({ data }: ValveChartProps) {
  const [hiddenLines, setHiddenLines] = useState<Record<string, boolean>>({
    valve_position: false,
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
            label={{
              value: "Valve Position",
              angle: -90,
              position: "insideLeft",
            }}
            stroke="#9ca3af"
            style={{ fontSize: 12 }}
          />

          <Tooltip
            content={
              <ChartTooltip formatter={(value) => formatValvePosition(value)} />
            }
          />

          <Legend wrapperStyle={LEGEND_STYLE} onClick={handleLegendClick} />

          <Line
            type="linear"
            dataKey="valve_position"
            stroke="#a855f7"
            strokeWidth={2}
            dot={false}
            name="Valve Position"
            hide={hiddenLines.valve_position}
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
});
