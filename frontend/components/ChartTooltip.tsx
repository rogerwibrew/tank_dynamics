"use client";

import { formatTime } from "../lib/utils";

/**
 * TypeScript interface for Recharts tooltip props.
 * Based on observed Recharts behavior since official types aren't exported.
 */
interface RechartsTooltipPayload {
  name: string;
  value: number;
  color: string;
  dataKey: string;
}

interface ChartTooltipProps {
  active?: boolean;
  payload?: RechartsTooltipPayload[];
  label?: number;
  formatter: (value: number) => string;
}

/**
 * Shared custom tooltip component for all charts.
 * Displays timestamp and formatted values with color indicators.
 *
 * @param active - Whether tooltip is active (hovering over chart)
 * @param payload - Array of data points at this x-axis position
 * @param label - X-axis value (simulation time in seconds)
 * @param formatter - Function to format the value (e.g., formatLevel, formatFlowRate)
 */
export function ChartTooltip({
  active,
  payload,
  label,
  formatter,
}: ChartTooltipProps) {
  if (!active || !payload || !payload.length) return null;

  return (
    <div className="bg-gray-900 border border-gray-600 rounded p-3">
      <p className="text-gray-400 text-sm mb-2">{formatTime(label)}</p>
      {payload.map((entry, index) => (
        <div key={index} className="flex items-center gap-2 text-sm">
          <div
            className="w-3 h-3 rounded-full"
            style={{ backgroundColor: entry.color }}
          />
          <span className="text-white">
            {entry.name}: {formatter(entry.value)}
          </span>
        </div>
      ))}
    </div>
  );
}
