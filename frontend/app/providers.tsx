"use client";

import React, { createContext, useContext, ReactNode } from "react";
import { useWebSocket } from "../hooks/useWebSocket";
import { SimulationState } from "../lib/types";
import { ConnectionStatus } from "../lib/websocket";

/**
 * Type for the simulation context value - matches useWebSocket return type
 */
type SimulationContextType = {
  state: SimulationState | null;
  connectionStatus: ConnectionStatus;
  error: string | null;
  setSetpoint: (value: number) => void;
  setPIDGains: (Kc: number, tau_I: number, tau_D: number) => void;
  setInletFlow: (value: number) => void;
  setInletMode: (
    mode: string,
    min: number,
    max: number,
    variance: number,
  ) => void;
  reconnect: () => void;
};

/**
 * Default context value - provides sensible defaults for IDE intellisense
 */
const defaultContextValue: SimulationContextType = {
  state: null,
  connectionStatus: "disconnected",
  error: null,
  setSetpoint: () => {},
  setPIDGains: () => {},
  setInletFlow: () => {},
  setInletMode: () => {},
  reconnect: () => {},
};

/**
 * Create the simulation context
 */
const SimulationContext = createContext<SimulationContextType>(
  defaultContextValue,
);

/**
 * SimulationProvider component that wraps the app with simulation state
 * Uses the useWebSocket hook internally to manage the single WebSocket connection
 */
export function SimulationProvider({ children }: { children: ReactNode }) {
  const websocketState = useWebSocket();

  return (
    <SimulationContext.Provider value={websocketState}>
      {children}
    </SimulationContext.Provider>
  );
}

/**
 * Hook to access simulation context
 * Throws error if used outside SimulationProvider
 */
export function useSimulation(): SimulationContextType {
  const context = useContext(SimulationContext);

  if (context === defaultContextValue) {
    throw new Error("useSimulation must be used within SimulationProvider");
  }

  return context;
}
