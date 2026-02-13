/**
 * Utility helper functions for the Tank Dynamics Simulator frontend.
 */

/**
 * Conditionally combine class names for Tailwind CSS styling.
 * Filters out falsy values and concatenates remaining class names.
 *
 * @param inputs - Variable number of class name arguments (strings, arrays, objects, or undefined)
 * @returns Single string of concatenated class names
 */
export function cn(
  ...inputs: (
    | string
    | string[]
    | Record<string, boolean>
    | undefined
    | null
    | false
  )[]
): string {
  const classes: string[] = [];

  for (const input of inputs) {
    if (!input) continue;

    if (typeof input === "string") {
      classes.push(input);
    } else if (Array.isArray(input)) {
      classes.push(...input.filter(Boolean));
    } else if (typeof input === "object") {
      for (const [key, value] of Object.entries(input)) {
        if (value) classes.push(key);
      }
    }
  }

  return classes.join(" ");
}

/**
 * Format tank level values for display.
 *
 * @param value - Tank level in meters
 * @returns Formatted string with 2 decimal places, or "N/A" if null/undefined
 */
export function formatLevel(value: number | null | undefined): string {
  if (value === null || value === undefined) return "N/A";
  return value.toFixed(2);
}

/**
 * Format flow rate values for display.
 *
 * @param value - Flow rate in m³/s
 * @returns Formatted string with 3 decimal places, or "N/A" if null/undefined
 */
export function formatFlowRate(value: number | null | undefined): string {
  if (value === null || value === undefined) return "N/A";
  return value.toFixed(3);
}

/**
 * Format valve position as percentage.
 *
 * @param value - Valve opening between 0 and 1
 * @returns Formatted string as percentage with 1 decimal place, or "N/A" if null/undefined
 */
export function formatValvePosition(value: number | null | undefined): string {
  if (value === null || value === undefined) return "N/A";
  return `${(value * 100).toFixed(1)}%`;
}

/**
 * Format simulation time in seconds to human-readable format.
 *
 * @param seconds - Number of seconds
 * @returns Formatted time string (MM:SS or HH:MM:SS), or "N/A" if null/undefined
 */
export function formatTime(seconds: number | null | undefined): string {
  if (seconds === null || seconds === undefined) return "N/A";

  const hours = Math.floor(seconds / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  const secs = Math.floor(seconds % 60);

  if (hours > 0) {
    return `${hours.toString().padStart(2, "0")}:${minutes.toString().padStart(2, "0")}:${secs.toString().padStart(2, "0")}`;
  } else {
    return `${minutes.toString().padStart(2, "0")}:${secs.toString().padStart(2, "0")}`;
  }
}

/**
 * Downsample an array to at most maxPoints entries using evenly-spaced selection.
 * Always preserves the first and last elements so chart endpoints are accurate.
 * Returns the original array if it's already within the limit.
 *
 * @param data - Source array to downsample
 * @param maxPoints - Maximum number of points in the result (default 500)
 * @returns Downsampled array, or the original if already small enough
 */
export function downsample<T>(data: T[], maxPoints: number = 500): T[] {
  if (data.length <= maxPoints) return data;

  const result: T[] = [data[0]];
  const step = (data.length - 1) / (maxPoints - 1);

  for (let i = 1; i < maxPoints - 1; i++) {
    result.push(data[Math.round(i * step)]);
  }

  result.push(data[data.length - 1]);
  return result;
}

/**
 * Constrain a value within a valid range.
 * Used for input validation before sending commands to backend.
 *
 * @param value - Value to constrain
 * @param min - Minimum allowed value
 * @param max - Maximum allowed value
 * @returns Clamped value within [min, max] range
 */
export function clampValue(value: number, min: number, max: number): number {
  return Math.min(Math.max(value, min), max);
}
