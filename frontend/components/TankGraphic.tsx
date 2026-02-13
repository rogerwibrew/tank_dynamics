"use client";

/**
 * TankGraphic component renders an SVG representation of a tank with animated liquid level.
 * Displays tank outline, liquid fill, scale markings, and setpoint indicator for SCADA interface.
 */

interface TankGraphicProps {
  level: number;
  setpoint: number;
  maxHeight: number;
  inletFlow: number;
  outletFlow: number;
}

export default function TankGraphic({
  level,
  setpoint,
  maxHeight,
  inletFlow,
  outletFlow,
}: TankGraphicProps) {
  // Calculate percentage of tank filled
  const levelPercentage = (level / maxHeight) * 100;
  const setpointPercentage = (setpoint / maxHeight) * 100;

  // SVG dimensions (viewBox coordinates)
  const viewBoxWidth = 400;
  const viewBoxHeight = 600;

  // Tank dimensions
  const tankLeft = 100;
  const tankTop = 80;
  const tankWidth = 200;
  const tankHeight = 400;
  const tankRight = tankLeft + tankWidth;
  const tankBottom = tankTop + tankHeight;

  // Calculate liquid fill height
  const liquidHeight = (levelPercentage / 100) * tankHeight;
  const liquidTop = tankBottom - liquidHeight;

  // Scale marks: 0, 1, 2, 3, 4, 5 meters
  const scaleMarks = [0, 1, 2, 3, 4, 5];
  const markSpacing = tankHeight / maxHeight;

  // Flow indicator colors
  const inletColor = inletFlow > 0 ? "#3b82f6" : "#6b7280";
  const outletColor = outletFlow > 0 ? "#3b82f6" : "#6b7280";
  const inletMarker =
    inletFlow > 0 ? "url(#arrowhead-blue)" : "url(#arrowhead-gray)";
  const outletMarker =
    outletFlow > 0 ? "url(#arrowhead-blue)" : "url(#arrowhead-gray)";

  // Inlet flow indicator positions
  const INLET_ARROW_X = 57.5;
  const INLET_ARROW_Y_START = 35;
  const INLET_ARROW_Y_END = 50;
  const INLET_LABEL_Y = 25;

  // Outlet flow indicator positions
  const OUTLET_ARROW_X = 182.5;
  const OUTLET_ARROW_Y_START = 480;
  const OUTLET_ARROW_Y_END = 530;
  const OUTLET_LABEL_Y = 565;

  // Inlet pipe dimensions and positions
  const INLET_PIPE_X = 30;
  const INLET_PIPE_Y = 50;
  const INLET_PIPE_WIDTH = 50;
  const INLET_PIPE_HEIGHT = 15;
  const INLET_CONNECTOR_WIDTH = 10;

  // Outlet pipe and valve dimensions
  const OUTLET_PIPE_X_OFFSET = 75;
  const OUTLET_PIPE_WIDTH = 15;
  const OUTLET_PIPE_HEIGHT = 40;
  const OUTLET_VALVE_X_OFFSET = 82.5;
  const OUTLET_VALVE_Y_OFFSET = 60;
  const OUTLET_VALVE_RADIUS = 12;
  const OUTLET_VALVE_CROSS_LEFT_OFFSET = 75;
  const OUTLET_VALVE_CROSS_RIGHT_OFFSET = 90;

  return (
    <div className="flex items-center justify-center w-full h-full bg-gray-900 rounded-lg p-4">
      <svg
        viewBox={`0 0 ${viewBoxWidth} ${viewBoxHeight}`}
        className="w-full max-w-md aspect-auto"
        xmlns="http://www.w3.org/2000/svg"
      >
        {/* Arrow marker definitions */}
        <defs>
          <marker
            id="arrowhead-blue"
            markerWidth="10"
            markerHeight="10"
            refX="5"
            refY="5"
            orient="auto"
          >
            <polygon points="0 0, 10 5, 0 10" fill="#3b82f6" />
          </marker>
          <marker
            id="arrowhead-gray"
            markerWidth="10"
            markerHeight="10"
            refX="5"
            refY="5"
            orient="auto"
          >
            <polygon points="0 0, 10 5, 0 10" fill="#6b7280" />
          </marker>
        </defs>

        {/* Scale marks on left side */}
        {scaleMarks.map((mark) => {
          const y = tankBottom - mark * markSpacing;
          return (
            <g key={`mark-${mark}`}>
              {/* Horizontal scale line */}
              <line
                x1={tankLeft - 15}
                y1={y}
                x2={tankLeft - 5}
                y2={y}
                stroke="#9ca3af"
                strokeWidth="2"
              />
              {/* Scale label */}
              <text
                x={tankLeft - 25}
                y={y + 5}
                fontSize="14"
                fill="#d1d5db"
                textAnchor="end"
                fontFamily="monospace"
              >
                {mark}m
              </text>
            </g>
          );
        })}

        {/* Tank outline */}
        <rect
          x={tankLeft}
          y={tankTop}
          width={tankWidth}
          height={tankHeight}
          stroke="#4b5563"
          fill="none"
          strokeWidth="3"
        />

        {/* Liquid fill */}
        <rect
          x={tankLeft}
          y={liquidTop}
          width={tankWidth}
          height={liquidHeight}
          fill="#3b82f6"
          className="transition-all duration-500"
        />

        {/* Setpoint line (horizontal dashed line) */}
        <line
          x1={tankLeft}
          y1={tankBottom - setpointPercentage * (tankHeight / 100)}
          x2={tankRight}
          y2={tankBottom - setpointPercentage * (tankHeight / 100)}
          stroke="#ef4444"
          strokeWidth="2"
          strokeDasharray="5,5"
        />

        {/* Inlet flow indicator arrow */}
        <line
          x1={INLET_ARROW_X}
          y1={INLET_ARROW_Y_START}
          x2={INLET_ARROW_X}
          y2={INLET_ARROW_Y_END}
          stroke={inletColor}
          strokeWidth="2"
          markerEnd={inletMarker}
          className={inletFlow > 0 ? "animate-pulse" : ""}
        />

        {/* Inlet flow label */}
        <text
          x={INLET_ARROW_X}
          y={INLET_LABEL_Y}
          fontSize="14"
          fill="#d1d5db"
          textAnchor="middle"
          fontFamily="monospace"
        >
          {inletFlow.toFixed(2)} m³/s
        </text>

        {/* Outlet flow indicator arrow */}
        <line
          x1={OUTLET_ARROW_X}
          y1={OUTLET_ARROW_Y_START}
          x2={OUTLET_ARROW_X}
          y2={OUTLET_ARROW_Y_END}
          stroke={outletColor}
          strokeWidth="2"
          markerEnd={outletMarker}
          className={outletFlow > 0 ? "animate-pulse" : ""}
        />

        {/* Outlet flow label */}
        <text
          x={OUTLET_ARROW_X}
          y={OUTLET_LABEL_Y}
          fontSize="14"
          fill="#d1d5db"
          textAnchor="middle"
          fontFamily="monospace"
        >
          {outletFlow.toFixed(2)} m³/s
        </text>

        {/* Inlet pipe at top */}
        <g>
          {/* Horizontal inlet pipe */}
          <rect
            x={INLET_PIPE_X}
            y={INLET_PIPE_Y}
            width={INLET_PIPE_WIDTH}
            height={INLET_PIPE_HEIGHT}
            fill="#6b7280"
          />
          {/* Connection to tank */}
          <rect
            x={tankLeft - 5}
            y={INLET_PIPE_Y}
            width={INLET_CONNECTOR_WIDTH}
            height={INLET_PIPE_HEIGHT}
            fill="#6b7280"
          />
        </g>

        {/* Outlet pipe at bottom with valve symbol */}
        <g>
          {/* Vertical outlet pipe */}
          <rect
            x={tankLeft + OUTLET_PIPE_X_OFFSET}
            y={tankBottom}
            width={OUTLET_PIPE_WIDTH}
            height={OUTLET_PIPE_HEIGHT}
            fill="#6b7280"
          />
          {/* Valve circle (simplified valve symbol) */}
          <circle
            cx={tankLeft + OUTLET_VALVE_X_OFFSET}
            cy={tankBottom + OUTLET_VALVE_Y_OFFSET}
            r={OUTLET_VALVE_RADIUS}
            fill="none"
            stroke="#6b7280"
            strokeWidth="2"
          />
          {/* Valve cross inside circle */}
          <line
            x1={tankLeft + OUTLET_VALVE_CROSS_LEFT_OFFSET}
            y1={tankBottom + OUTLET_VALVE_Y_OFFSET}
            x2={tankLeft + OUTLET_VALVE_CROSS_RIGHT_OFFSET}
            y2={tankBottom + OUTLET_VALVE_Y_OFFSET}
            stroke="#6b7280"
            strokeWidth="2"
          />
        </g>
      </svg>
    </div>
  );
}
