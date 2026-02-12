/**
 * Type definitions for Tank Dynamics Simulator frontend
 * These interfaces match the Pydantic models from the FastAPI backend
 */

/**
 * PID controller gains configuration
 */
export interface PIDGains {
  Kc: number;
  tau_I: number;
  tau_D: number;
}

/**
 * Inlet mode configuration for Brownian mode
 */
export interface InletConfig {
  min: number;
  max: number;
  variance: number;
}

/**
 * Real-time simulation state snapshot sent via WebSocket every second
 * Fields match api/simulation.py SimulationManager.get_state() output
 */
export interface SimulationState {
  time: number;
  tank_level: number;
  setpoint: number;
  error: number;
  inlet_flow: number;
  outlet_flow: number;
  valve_position: number;
  controller_output: number;
}

/**
 * Simulation configuration returned by GET /api/config
 */
export interface ConfigResponse {
  tank_height: number;
  tank_area: number;
  valve_coefficient: number;
  initial_level: number;
  initial_setpoint: number;
  pid_gains: PIDGains;
  timestep: number;
  history_capacity: number;
  history_size: number;
}

/**
 * Single historical data point with same fields as SimulationState
 */
export type HistoryPoint = SimulationState;

/**
 * WebSocket message from client to server
 * Discriminated union for type-safe command handling
 */
export type WebSocketMessage =
  | {
      type: "setpoint";
      value: number;
    }
  | {
      type: "pid";
      Kc: number;
      tau_I: number;
      tau_D: number;
    }
  | {
      type: "inlet_flow";
      value: number;
    }
  | {
      type: "inlet_mode";
      mode: "constant" | "brownian";
      min?: number;
      max?: number;
      variance?: number;
    };
