"use client";

import React, { Component, ReactNode } from "react";

interface Props {
  children: ReactNode;
  chartName: string;
}

interface State {
  hasError: boolean;
  error: Error | null;
}

/**
 * Error boundary component for chart components.
 * Catches rendering errors in Recharts and displays a user-friendly message
 * instead of crashing the entire application.
 *
 * Usage:
 * <ChartErrorBoundary chartName="Level Chart">
 *   <LevelChart data={data} />
 * </ChartErrorBoundary>
 */
export class ChartErrorBoundary extends Component<Props, State> {
  constructor(props: Props) {
    super(props);
    this.state = { hasError: false, error: null };
  }

  static getDerivedStateFromError(error: Error): State {
    // Update state so the next render will show the fallback UI
    return { hasError: true, error };
  }

  componentDidCatch(error: Error, errorInfo: React.ErrorInfo) {
    // Log error details for debugging
    console.error(`Chart error in ${this.props.chartName}:`, error, errorInfo);
  }

  render() {
    if (this.state.hasError) {
      return (
        <div className="bg-gray-800 border border-red-500 rounded-lg p-4">
          <div className="flex items-center gap-2 mb-2">
            <svg
              className="w-5 h-5 text-red-400"
              fill="none"
              stroke="currentColor"
              viewBox="0 0 24 24"
            >
              <path
                strokeLinecap="round"
                strokeLinejoin="round"
                strokeWidth={2}
                d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z"
              />
            </svg>
            <h3 className="text-red-400 font-semibold">
              {this.props.chartName} Failed to Render
            </h3>
          </div>
          <p className="text-gray-400 text-sm mb-2">
            {this.state.error?.message || "Unknown error occurred"}
          </p>
          <button
            onClick={() => this.setState({ hasError: false, error: null })}
            className="px-3 py-1 bg-gray-700 hover:bg-gray-600 text-white text-sm rounded transition-colors"
          >
            Try Again
          </button>
        </div>
      );
    }

    return this.props.children;
  }
}
