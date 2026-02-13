"use client";

import React from "react";

/**
 * InletFlowControl component provides an interface for operators to control
 * inlet flow rate in two modes:
 * 1. Constant mode: fixed flow rate
 * 2. Brownian mode: random walk between min/max bounds
 */

interface InletFlowControlProps {
  currentFlow: number;
  onFlowChange: (value: number) => void;
  onModeChange: (
    mode: "constant" | "brownian",
    config?: { min: number; max: number; variance: number },
  ) => void;
}

export default function InletFlowControl({
  currentFlow,
  onFlowChange,
  onModeChange,
}: InletFlowControlProps) {
  // Local state for pending changes
  const [localMode, setLocalMode] = React.useState<"constant" | "brownian">(
    "constant",
  );
  const [localFlowRate, setLocalFlowRate] = React.useState(currentFlow);
  const [localMin, setLocalMin] = React.useState(0.8);
  const [localMax, setLocalMax] = React.useState(1.2);
  const [localVariance, setLocalVariance] = React.useState(0.05);
  const [errorMessage, setErrorMessage] = React.useState<string>("");
  const [hasLocalChanges, setHasLocalChanges] = React.useState(false);

  // Only update local state when props change AND user hasn't made local edits
  React.useEffect(() => {
    if (!hasLocalChanges) {
      setLocalFlowRate(currentFlow);
    }
  }, [currentFlow, hasLocalChanges]);

  // Validate inputs
  const validateInputs = (): boolean => {
    if (localMode === "constant") {
      if (localFlowRate < 0 || localFlowRate > 2.0) {
        setErrorMessage("Flow rate must be between 0.0 and 2.0 m³/s");
        return false;
      }
    } else {
      if (localMin < 0 || localMin > 2.0) {
        setErrorMessage("Min flow must be between 0.0 and 2.0 m³/s");
        return false;
      }
      if (localMax < 0 || localMax > 2.0) {
        setErrorMessage("Max flow must be between 0.0 and 2.0 m³/s");
        return false;
      }
      if (localMax <= localMin) {
        setErrorMessage("Max flow must be greater than min flow");
        return false;
      }
      if (localVariance < 0 || localVariance > 1.0) {
        setErrorMessage("Variance must be between 0.0 and 1.0");
        return false;
      }
    }
    setErrorMessage("");
    return true;
  };

  // Handle Apply button click
  const handleApply = () => {
    if (validateInputs()) {
      if (localMode === "constant") {
        onModeChange("constant");
        onFlowChange(localFlowRate);
      } else {
        onModeChange("brownian", {
          min: localMin,
          max: localMax,
          variance: localVariance,
        });
      }
      setHasLocalChanges(false);
    }
  };

  // Handle mode change
  const handleModeChange = (mode: "constant" | "brownian") => {
    setLocalMode(mode);
    setHasLocalChanges(true);
    setErrorMessage("");
    // Reset Brownian parameters to defaults when mode changes
    setLocalMin(0.8);
    setLocalMax(1.2);
    setLocalVariance(0.05);
  };

  // Handle flow rate input change
  const handleFlowRateChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = parseFloat(e.target.value);
    if (!isNaN(value)) {
      setLocalFlowRate(value);
      setHasLocalChanges(true);
      setErrorMessage("");
    }
  };

  // Handle min input change
  const handleMinChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = parseFloat(e.target.value);
    if (!isNaN(value)) {
      setLocalMin(value);
      setHasLocalChanges(true);
      setErrorMessage("");
    }
  };

  // Handle max input change
  const handleMaxChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = parseFloat(e.target.value);
    if (!isNaN(value)) {
      setLocalMax(value);
      setHasLocalChanges(true);
      setErrorMessage("");
    }
  };

  // Handle variance input change
  const handleVarianceChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = parseFloat(e.target.value);
    if (!isNaN(value)) {
      setLocalVariance(value);
      setHasLocalChanges(true);
      setErrorMessage("");
    }
  };

  return (
    <div className="bg-gray-800 rounded-lg p-4 w-full">
      <h3 className="text-lg font-semibold text-white mb-4">
        Inlet Flow Control
      </h3>

      {/* Mode selector */}
      <div className="mb-4">
        <div className="text-sm text-gray-400 mb-2">Mode</div>
        <div className="flex gap-6">
          <label className="flex items-center gap-2 cursor-pointer">
            <input
              type="radio"
              name="flow-mode"
              value="constant"
              checked={localMode === "constant"}
              onChange={() => handleModeChange("constant")}
              className="w-4 h-4 text-blue-500 bg-gray-700 border-gray-600 focus:ring-2 focus:ring-blue-500"
            />
            <span className="text-sm text-gray-300">Constant</span>
          </label>
          <label className="flex items-center gap-2 cursor-pointer">
            <input
              type="radio"
              name="flow-mode"
              value="brownian"
              checked={localMode === "brownian"}
              onChange={() => handleModeChange("brownian")}
              className="w-4 h-4 text-blue-500 bg-gray-700 border-gray-600 focus:ring-2 focus:ring-blue-500"
            />
            <span className="text-sm text-gray-300">Brownian</span>
          </label>
        </div>
      </div>

      {/* Conditional rendering based on mode */}
      {localMode === "constant" ? (
        // Constant mode inputs
        <div className="mb-4">
          <label className="block text-xs font-semibold text-gray-400 uppercase mb-1">
            Flow Rate [m³/s]
          </label>
          <input
            type="number"
            min="0"
            max="2.0"
            step="0.1"
            value={localFlowRate}
            onChange={handleFlowRateChange}
            className="w-full bg-gray-700 text-white border border-gray-600 rounded px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
          />
        </div>
      ) : (
        // Brownian mode inputs
        <div className="mb-4">
          <div className="mb-3">
            <label className="block text-xs font-semibold text-gray-400 uppercase mb-1">
              Min Flow [m³/s]
            </label>
            <input
              type="number"
              min="0"
              max="2.0"
              step="0.1"
              value={localMin}
              onChange={handleMinChange}
              className="w-full bg-gray-700 text-white border border-gray-600 rounded px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
            />
          </div>

          <div className="mb-3">
            <label className="block text-xs font-semibold text-gray-400 uppercase mb-1">
              Max Flow [m³/s]
            </label>
            <input
              type="number"
              min="0"
              max="2.0"
              step="0.1"
              value={localMax}
              onChange={handleMaxChange}
              className="w-full bg-gray-700 text-white border border-gray-600 rounded px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
            />
          </div>

          <div>
            <label className="block text-xs font-semibold text-gray-400 uppercase mb-1">
              Variance
            </label>
            <input
              type="number"
              min="0"
              max="1.0"
              step="0.01"
              value={localVariance}
              onChange={handleVarianceChange}
              className="w-full bg-gray-700 text-white border border-gray-600 rounded px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
            />
          </div>
        </div>
      )}

      {/* Error message */}
      {errorMessage && (
        <div className="mb-3 text-sm text-red-400 font-semibold">
          {errorMessage}
        </div>
      )}

      {/* Apply button */}
      <button
        onClick={handleApply}
        className="w-full bg-green-600 hover:bg-green-700 text-white font-bold py-2 px-4 rounded transition-colors"
      >
        Apply
      </button>
    </div>
  );
}
