"use client";

import { useEffect, useState, useRef, useCallback } from "react";
import { WebSocketClient, ConnectionStatus } from "../lib/websocket";
import { SimulationState } from "../lib/types";
import { clampValue } from "../lib/utils";

/**
 * React hook that wraps the WebSocket client and integrates with React lifecycle.
 * Manages connection, state updates, and provides command methods with input validation.
 */
export function useWebSocket(): {
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
} {
  const [state, setState] = useState<SimulationState | null>(null);
  const [connectionStatus, setConnectionStatus] =
    useState<ConnectionStatus>("disconnected");
  const [error, setError] = useState<string | null>(null);
  const clientRef = useRef<WebSocketClient | null>(null);

  // Effect: Initialize connection on mount, cleanup on unmount
  useEffect(() => {
    const url = process.env.NEXT_PUBLIC_WS_URL || "ws://localhost:8000/ws";
    const client = new WebSocketClient(url);
    clientRef.current = client;

    const unsubConnect = client.on("connect", () => {
      setConnectionStatus("connected");
      setError(null);
    });

    const unsubDisconnect = client.on("disconnect", () => {
      setConnectionStatus("disconnected");
    });

    const unsubError = client.on("error", () => {
      setConnectionStatus("error");
      setError("WebSocket connection error");
    });

    const unsubMessage = client.on("message", (data: unknown) => {
      const msg = data as { type?: string; data?: unknown };
      if (msg.type === "state" && msg.data) {
        setState(msg.data as SimulationState);
      } else if (msg.type === "error") {
        const errMsg = msg as { message?: string };
        setError(errMsg.message || "Server error");
      }
    });

    client.connect();

    return () => {
      unsubConnect();
      unsubDisconnect();
      unsubError();
      unsubMessage();
      client.disconnect();
    };
  }, []);

  const setSetpoint = useCallback((value: number) => {
    // Validate input
    if (!Number.isFinite(value)) {
      setError("Invalid setpoint: must be a finite number");
      console.error("Invalid setpoint value:", value);
      return;
    }

    // Clamp to valid range (tank height 0-10m)
    const clamped = clampValue(value, 0, 10);
    if (clamped !== value) {
      console.warn(`Setpoint clamped from ${value} to ${clamped}`);
    }

    try {
      clientRef.current?.send({ type: "setpoint", value: clamped });
      setError(null);
    } catch (e) {
      const errorMsg = e instanceof Error ? e.message : "Unknown error";
      console.error("Failed to send setpoint command:", e);
      setError(`Failed to send setpoint: ${errorMsg}`);
    }
  }, []);

  const setPIDGains = useCallback(
    (Kc: number, tau_I: number, tau_D: number) => {
      // Validate inputs
      if (
        !Number.isFinite(Kc) ||
        !Number.isFinite(tau_I) ||
        !Number.isFinite(tau_D)
      ) {
        setError("Invalid PID gains: all values must be finite numbers");
        console.error("Invalid PID gains:", { Kc, tau_I, tau_D });
        return;
      }

      // Validate ranges (reasonable PID values)
      if (Kc < 0) {
        setError("Invalid Kc: must be non-negative");
        console.error("Invalid Kc value:", Kc);
        return;
      }
      if (tau_I < 0) {
        setError("Invalid tau_I: must be non-negative");
        console.error("Invalid tau_I value:", tau_I);
        return;
      }
      if (tau_D < 0) {
        setError("Invalid tau_D: must be non-negative");
        console.error("Invalid tau_D value:", tau_D);
        return;
      }

      try {
        clientRef.current?.send({ type: "pid", Kc, tau_I, tau_D });
        setError(null);
      } catch (e) {
        const errorMsg = e instanceof Error ? e.message : "Unknown error";
        console.error("Failed to send PID gains command:", e);
        setError(`Failed to send PID gains: ${errorMsg}`);
      }
    },
    [],
  );

  const setInletFlow = useCallback((value: number) => {
    // Validate input
    if (!Number.isFinite(value)) {
      setError("Invalid inlet flow: must be a finite number");
      console.error("Invalid inlet flow value:", value);
      return;
    }

    // Clamp to reasonable range (0-5 m³/s)
    const clamped = clampValue(value, 0, 5);
    if (clamped !== value) {
      console.warn(`Inlet flow clamped from ${value} to ${clamped}`);
    }

    try {
      clientRef.current?.send({ type: "inlet_flow", value: clamped });
      setError(null);
    } catch (e) {
      const errorMsg = e instanceof Error ? e.message : "Unknown error";
      console.error("Failed to send inlet flow command:", e);
      setError(`Failed to send inlet flow: ${errorMsg}`);
    }
  }, []);

  const setInletMode = useCallback(
    (mode: string, min: number, max: number, variance: number) => {
      // Validate mode
      if (mode !== "constant" && mode !== "brownian") {
        setError("Invalid inlet mode: must be 'constant' or 'brownian'");
        console.error("Invalid inlet mode:", mode);
        return;
      }

      // Validate numeric inputs
      if (
        !Number.isFinite(min) ||
        !Number.isFinite(max) ||
        !Number.isFinite(variance)
      ) {
        setError(
          "Invalid inlet mode parameters: all values must be finite numbers",
        );
        console.error("Invalid inlet mode parameters:", { min, max, variance });
        return;
      }

      // Validate ranges
      if (min < 0 || max < 0 || variance < 0) {
        setError("Invalid inlet mode parameters: values must be non-negative");
        console.error("Invalid inlet mode parameters:", { min, max, variance });
        return;
      }
      if (min >= max) {
        setError("Invalid inlet mode parameters: min must be less than max");
        console.error("Invalid inlet mode parameters: min >= max", {
          min,
          max,
        });
        return;
      }

      try {
        clientRef.current?.send({
          type: "inlet_mode",
          mode,
          min,
          max,
          variance,
        });
        setError(null);
      } catch (e) {
        const errorMsg = e instanceof Error ? e.message : "Unknown error";
        console.error("Failed to send inlet mode command:", e);
        setError(`Failed to send inlet mode: ${errorMsg}`);
      }
    },
    [],
  );

  const reconnect = useCallback(() => {
    setError(null);
    clientRef.current?.connect();
  }, []);

  return {
    state,
    connectionStatus,
    error,
    setSetpoint,
    setPIDGains,
    setInletFlow,
    setInletMode,
    reconnect,
  };
}
