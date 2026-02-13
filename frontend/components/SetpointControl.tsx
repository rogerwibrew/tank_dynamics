"use client";

/**
 * SetpointControl component provides an interface for operators to adjust
 * the tank level setpoint with validation and visual feedback.
 */

interface SetpointControlProps {
  currentSetpoint: number;
  currentLevel: number;
  onSetpointChange: (newValue: number) => void;
}

export default function SetpointControl({
  currentSetpoint,
  currentLevel,
  onSetpointChange,
}: SetpointControlProps) {
  const MIN_SETPOINT = 0.0;
  const MAX_SETPOINT = 5.0;
  const STEP = 0.1;

  // Calculate error (setpoint - level)
  const error = currentSetpoint - currentLevel;

  // Determine error color based on magnitude (not direction)
  const getErrorColor = (): string => {
    const absoluteError = Math.abs(error);
    if (absoluteError < 0.2) {
      return "text-gray-400"; // Small error: gray
    } else if (absoluteError < 0.5) {
      return "text-yellow-400"; // Medium error: yellow
    } else {
      return "text-red-400"; // Large error: red
    }
  };

  // Validate and clamp value to valid range
  const clampValue = (value: number): number => {
    const clamped = Math.max(MIN_SETPOINT, Math.min(MAX_SETPOINT, value));
    return Math.round(clamped * 10) / 10; // Round to 1 decimal place
  };

  const handleInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = parseFloat(e.target.value);
    if (!isNaN(value)) {
      const validValue = clampValue(value);
      onSetpointChange(validValue);
    }
  };

  const handleIncrement = () => {
    const newValue = clampValue(currentSetpoint + STEP);
    onSetpointChange(newValue);
  };

  const handleDecrement = () => {
    const newValue = clampValue(currentSetpoint - STEP);
    onSetpointChange(newValue);
  };

  // Check if buttons should be disabled
  const isAtMax = currentSetpoint >= MAX_SETPOINT;
  const isAtMin = currentSetpoint <= MIN_SETPOINT;

  return (
    <div className="bg-gray-800 rounded-lg p-4 w-full">
      <h3 className="text-lg font-semibold text-white mb-4">
        Setpoint Control
      </h3>

      {/* Current setpoint display */}
      <div className="mb-4">
        <div className="text-sm text-gray-400 mb-1">Current Setpoint</div>
        <div className="text-3xl font-bold text-blue-400">
          {currentSetpoint.toFixed(1)} m
        </div>
      </div>

      {/* Number input with increment/decrement buttons */}
      <div className="flex items-center gap-2 mb-4">
        <button
          onClick={handleDecrement}
          disabled={isAtMin}
          className="bg-blue-600 hover:bg-blue-700 disabled:bg-gray-700 disabled:cursor-not-allowed text-white font-bold py-2 px-4 rounded transition-colors"
          aria-label="Decrease setpoint"
        >
          −0.1
        </button>

        <input
          type="number"
          min={MIN_SETPOINT}
          max={MAX_SETPOINT}
          step={STEP}
          value={currentSetpoint}
          onChange={handleInputChange}
          className="flex-1 bg-gray-700 text-white border border-gray-600 rounded px-3 py-2 text-center font-mono text-lg focus:outline-none focus:ring-2 focus:ring-blue-500"
          aria-label="Setpoint value"
        />

        <button
          onClick={handleIncrement}
          disabled={isAtMax}
          className="bg-blue-600 hover:bg-blue-700 disabled:bg-gray-700 disabled:cursor-not-allowed text-white font-bold py-2 px-4 rounded transition-colors"
          aria-label="Increase setpoint"
        >
          +0.1
        </button>
      </div>

      {/* Error display (setpoint - level) */}
      <div className="text-sm text-gray-400">
        <span className="font-semibold">Error (SP - PV):</span>{" "}
        <span className={getErrorColor()}>
          {error >= 0 ? "+" : ""}
          {error.toFixed(2)} m
        </span>
      </div>
    </div>
  );
}
