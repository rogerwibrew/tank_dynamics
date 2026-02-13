"use client";

import React from "react";

/**
 * PIDTuningControl component provides an interface for operators to tune
 * PID controller parameters (Kc, tau_I, tau_D) with validation.
 *
 * Operators enter positive gain values. A "Reverse Acting" checkbox controls
 * the sign of Kc sent to the backend (negated when reverse acting).
 */

interface PIDTuningControlProps {
  currentGains: { Kc: number; tau_I: number; tau_D: number };
  reverseActing: boolean;
  onGainsChange: (newGains: {
    Kc: number;
    tau_I: number;
    tau_D: number;
  }) => void;
  onReverseActingChange: (reverseActing: boolean) => void;
}

export default function PIDTuningControl({
  currentGains,
  reverseActing,
  onGainsChange,
  onReverseActingChange,
}: PIDTuningControlProps) {
  // Local state for pending changes
  const [localKc, setLocalKc] = React.useState(currentGains.Kc);
  const [localTauI, setLocalTauI] = React.useState(currentGains.tau_I);
  const [localTauD, setLocalTauD] = React.useState(currentGains.tau_D);
  const [localReverseActing, setLocalReverseActing] =
    React.useState(reverseActing);
  const [errorMessage, setErrorMessage] = React.useState<string>("");
  const [hasLocalChanges, setHasLocalChanges] = React.useState(false);

  // Only update local state when props change AND user hasn't made local edits
  React.useEffect(() => {
    if (!hasLocalChanges) {
      setLocalKc(currentGains.Kc);
      setLocalTauI(currentGains.tau_I);
      setLocalTauD(currentGains.tau_D);
      setLocalReverseActing(reverseActing);
    }
  }, [currentGains, reverseActing, hasLocalChanges]);

  // Validate inputs
  const validateInputs = (): boolean => {
    if (localKc < 0) {
      setErrorMessage("Proportional Gain (Kc) must be >= 0");
      return false;
    }
    if (localTauI < 0) {
      setErrorMessage("Integral Time (tau_I) must be >= 0");
      return false;
    }
    if (localTauD < 0) {
      setErrorMessage("Derivative Time (tau_D) must be >= 0");
      return false;
    }
    setErrorMessage("");
    return true;
  };

  // Handle Apply button click
  const handleApply = () => {
    if (validateInputs()) {
      const kcToSend = localReverseActing ? -localKc : localKc;
      onGainsChange({
        Kc: kcToSend,
        tau_I: localTauI,
        tau_D: localTauD,
      });
      onReverseActingChange(localReverseActing);
      setHasLocalChanges(false);
    }
  };

  // Handle input changes
  const handleKcChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = parseFloat(e.target.value);
    if (!isNaN(value)) {
      setLocalKc(value);
      setHasLocalChanges(true);
      setErrorMessage("");
    }
  };

  const handleTauIChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = parseFloat(e.target.value);
    if (!isNaN(value)) {
      setLocalTauI(value);
      setHasLocalChanges(true);
      setErrorMessage("");
    }
  };

  const handleTauDChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = parseFloat(e.target.value);
    if (!isNaN(value)) {
      setLocalTauD(value);
      setHasLocalChanges(true);
      setErrorMessage("");
    }
  };

  const handleReverseActingChange = (
    e: React.ChangeEvent<HTMLInputElement>,
  ) => {
    setLocalReverseActing(e.target.checked);
    setHasLocalChanges(true);
    setErrorMessage("");
  };

  return (
    <div className="bg-gray-800 rounded-lg p-4 w-full">
      <h3 className="text-lg font-semibold text-white mb-4">PID Tuning</h3>

      {/* Reverse Acting checkbox */}
      <div className="mb-4">
        <label className="flex items-center gap-2 cursor-pointer">
          <input
            type="checkbox"
            checked={localReverseActing}
            onChange={handleReverseActingChange}
            className="w-4 h-4 rounded border-gray-600 bg-gray-700 text-blue-500 focus:ring-blue-500 focus:ring-2"
          />
          <span className="text-sm text-gray-300">Reverse Acting</span>
        </label>
      </div>

      {/* Kc input */}
      <div className="mb-3">
        <label className="block text-xs font-semibold text-gray-400 uppercase mb-1">
          Proportional Gain (Kc)
        </label>
        <input
          type="number"
          min="0"
          step="0.1"
          value={localKc}
          onChange={handleKcChange}
          className="w-full bg-gray-700 text-white border border-gray-600 rounded px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
        />
      </div>

      {/* tau_I input */}
      <div className="mb-3">
        <label className="block text-xs font-semibold text-gray-400 uppercase mb-1">
          Integral Time (tau_I) [s]
        </label>
        <input
          type="number"
          min="0"
          step="1.0"
          value={localTauI}
          onChange={handleTauIChange}
          className="w-full bg-gray-700 text-white border border-gray-600 rounded px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
        />
      </div>

      {/* tau_D input */}
      <div className="mb-3">
        <label className="block text-xs font-semibold text-gray-400 uppercase mb-1">
          Derivative Time (tau_D) [s]
        </label>
        <input
          type="number"
          min="0"
          step="0.1"
          value={localTauD}
          onChange={handleTauDChange}
          className="w-full bg-gray-700 text-white border border-gray-600 rounded px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
        />
      </div>

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
