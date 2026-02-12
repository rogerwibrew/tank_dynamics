"use client";

import React, {
  createContext,
  useContext,
  ReactNode,
  useRef,
  useEffect,
  useState,
} from "react";
import { useWebSocket } from "../hooks/useWebSocket";
import { SimulationState } from "../lib/types";
import { ConnectionStatus } from "../lib/websocket";

/**
 * Type for the simulation context value - extends useWebSocket return type with history
 */
type SimulationContextType = {
  state: SimulationState | null;
  connectionStatus: ConnectionStatus;
  error: string | null;
  history: SimulationState[];
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
  history: [],
  setSetpoint: () => {},
  setPIDGains: () => {},
  setInletFlow: () => {},
  setInletMode: () => {},
  reconnect: () => {},
};

/**
 * Create the simulation context
 */
const SimulationContext =
  createContext<SimulationContextType>(defaultContextValue);

/**
 * SimulationProvider component that wraps the app with simulation state
 * Uses the useWebSocket hook internally to manage the single WebSocket connection
 * Maintains a rolling history of the last 10 state updates for display in TrendsView
 */
export function SimulationProvider({ children }: { children: ReactNode }) {
  const websocketState = useWebSocket();
  const [history, setHistory] = useState<SimulationState[]>([]);
  const lastStateRef = useRef<SimulationState | null>(null);

  // Update history when state changes from WebSocket (external system)
  // This is a legitimate use of setState in useEffect: we're subscribing to updates
  // from an external system (WebSocket) and maintaining a derived cache (history buffer).
  // The ref check ensures we only update on genuinely new state objects.
  useEffect(() => {
    if (
      websocketState.state !== null &&
      websocketState.state !== lastStateRef.current
    ) {
      lastStateRef.current = websocketState.state;
      // eslint-disable-next-line react-hooks/set-state-in-effect
      setHistory((prev) => {
        const updated = [websocketState.state!, ...prev];
        return updated.slice(0, 10);
      });
    }
  }, [websocketState.state]);

  const contextValue = {
    ...websocketState,
    history,
  };

  return (
    <SimulationContext.Provider value={contextValue}>
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
