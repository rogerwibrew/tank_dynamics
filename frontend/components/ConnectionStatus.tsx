"use client";

import { useSimulation } from "../app/providers";

/**
 * ConnectionStatus component displays the current WebSocket connection status
 * with a visual indicator (colored dot) and status text.
 *
 * Status colors (SCADA dark theme):
 * - connected: green
 * - connecting: yellow
 * - disconnected: gray
 * - error: red
 */
export function ConnectionStatus() {
  const { connectionStatus } = useSimulation();

  // Map connection status to visual styling
  const statusConfig = {
    connected: {
      color: "bg-green-500",
      text: "Connected",
      textColor: "text-green-400",
    },
    connecting: {
      color: "bg-yellow-500",
      text: "Connecting",
      textColor: "text-yellow-400",
    },
    disconnected: {
      color: "bg-gray-500",
      text: "Disconnected",
      textColor: "text-gray-400",
    },
    error: {
      color: "bg-red-500",
      text: "Error",
      textColor: "text-red-400",
    },
  };

  const config = statusConfig[connectionStatus];

  return (
    <div className="flex items-center gap-2">
      {/* Status indicator dot */}
      <div className={`w-3 h-3 rounded-full ${config.color}`} />
      {/* Status text */}
      <span className={`text-sm font-medium ${config.textColor}`}>
        {config.text}
      </span>
    </div>
  );
}
