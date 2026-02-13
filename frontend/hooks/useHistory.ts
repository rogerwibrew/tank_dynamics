"use client";

import { useState, useEffect, useCallback } from "react";
import { SimulationState } from "../lib/types";

const MIN_DURATION = 1;
const MAX_DURATION = 7200;
const DEFAULT_DURATION = 3600;

/**
 * Clamps duration to valid range (1-7200 seconds).
 * Logs warning to console if clamping occurs.
 */
function clampDuration(duration: number): number {
  if (!Number.isFinite(duration)) {
    console.warn(
      `Duration is not a finite number (${duration}), clamping to ${DEFAULT_DURATION}`,
    );
    return DEFAULT_DURATION;
  }

  if (duration < MIN_DURATION) {
    console.warn(`Duration ${duration} is below minimum (${MIN_DURATION}), clamping to ${MIN_DURATION}`);
    return MIN_DURATION;
  }

  if (duration > MAX_DURATION) {
    console.warn(`Duration ${duration} exceeds maximum (${MAX_DURATION}), clamping to ${MAX_DURATION}`);
    return MAX_DURATION;
  }

  return duration;
}

/**
 * React hook that fetches and manages historical simulation data.
 *
 * Fetches data from /api/history endpoint with the specified duration.
 * Automatically refetches when duration changes.
 *
 * @param durationSeconds - Number of seconds of history to fetch (1-7200, default 3600)
 * @returns Object with history data, loading state, error state, and refetch function
 *
 * @example
 * const { history, loading, error, refetch } = useHistory(1800);
 * if (loading) return <div>Loading...</div>;
 * if (error) return <div>Error: {error}</div>;
 * return <Chart data={history} />;
 */
export function useHistory(durationSeconds: number = DEFAULT_DURATION): {
  history: SimulationState[];
  loading: boolean;
  error: string | null;
  refetch: () => Promise<void>;
} {
  const [history, setHistory] = useState<SimulationState[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const fetchHistory = useCallback(async () => {
    try {
      setLoading(true);
      setError(null);

      const validDuration = clampDuration(durationSeconds);
      const url = `/api/history?duration=${validDuration}`;

      const response = await fetch(url);

      if (!response.ok) {
        const statusText = response.statusText || "Unknown error";
        throw new Error(`Server error: ${response.status} ${statusText}`);
      }

      const data: unknown = await response.json();

      // Validate that response is an array
      if (!Array.isArray(data)) {
        throw new Error("Invalid response format: expected array of SimulationState objects");
      }

      setHistory(data as SimulationState[]);
    } catch (e) {
      const errorMsg = e instanceof Error ? e.message : "Unknown error";
      console.error("Failed to fetch history:", e);
      setError(`Failed to fetch history: ${errorMsg}`);
    } finally {
      setLoading(false);
    }
  }, [durationSeconds]);

  useEffect(() => {
    fetchHistory();
  }, [durationSeconds, fetchHistory]);

  return { history, loading, error, refetch: fetchHistory };
}
