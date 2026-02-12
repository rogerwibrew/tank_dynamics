/**
 * WebSocket connection status type
 */
export type ConnectionStatus =
  | "connecting"
  | "connected"
  | "disconnected"
  | "error";

/**
 * Callback function type for message events
 */
export type MessageCallback = (data: unknown) => void;

/**
 * Callback function type for connection events
 */
export type ConnectionCallback = () => void;

/**
 * Callback function type for error events
 */
export type ErrorCallback = (error: Event) => void;

/**
 * WebSocket client for communicating with the Tank Dynamics backend
 */
export class WebSocketClient {
  private url: string;
  private websocket: WebSocket | null = null;
  private connectionStatus: ConnectionStatus = "disconnected";
  private callbacks: {
    connect: ConnectionCallback[];
    message: MessageCallback[];
    disconnect: ConnectionCallback[];
    error: ErrorCallback[];
  } = {
    connect: [],
    message: [],
    disconnect: [],
    error: [],
  };
  private reconnectTimer: NodeJS.Timeout | null = null;
  private reconnectAttempts: number = 0;
  private maxReconnectAttempts: number = 5;
  private baseReconnectDelay: number = 1000; // 1 second
  private manualDisconnect: boolean = false;

  /**
   * Create a WebSocket client
   * @param url - WebSocket endpoint URL
   */
  constructor(url: string) {
    this.url = url;
  }

  /**
   * Get current connection status
   */
  getStatus(): ConnectionStatus {
    return this.connectionStatus;
  }

  /**
   * Connect to the WebSocket server
   */
  connect(): void {
    if (this.websocket !== null) {
      console.warn("WebSocket already connected or connecting");
      return;
    }

    try {
      this.manualDisconnect = false;
      this.connectionStatus = "connecting";
      this.websocket = new WebSocket(this.url);

      this.websocket.addEventListener("open", () => {
        this.connectionStatus = "connected";
        this.reconnectAttempts = 0; // Reset retry counter on successful connection
        this.callbacks.connect.forEach((callback) => callback());
      });

      this.websocket.addEventListener("message", (event: MessageEvent) => {
        try {
          const data = JSON.parse(event.data);
          this.callbacks.message.forEach((callback) => callback(data));
        } catch (error) {
          console.error("Failed to parse WebSocket message:", error);
        }
      });

      this.websocket.addEventListener("close", () => {
        this.connectionStatus = "disconnected";
        this.websocket = null;
        this.callbacks.disconnect.forEach((callback) => callback());

        // Attempt reconnection if not manually disconnected
        if (!this.manualDisconnect) {
          this.attemptReconnect();
        }
      });

      this.websocket.addEventListener("error", (event: Event) => {
        this.connectionStatus = "error";
        console.error("WebSocket error:", event);
        this.callbacks.error.forEach((callback) => callback(event));

        // Attempt reconnection on error
        if (!this.manualDisconnect) {
          this.attemptReconnect();
        }
      });
    } catch (error) {
      this.connectionStatus = "error";
      console.error("Failed to create WebSocket:", error);
    }
  }

  /**
   * Disconnect from the WebSocket server
   */
  disconnect(): void {
    this.manualDisconnect = true;

    // Clear any pending reconnection timer
    if (this.reconnectTimer !== null) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }

    if (
      this.websocket !== null &&
      this.websocket.readyState === WebSocket.OPEN
    ) {
      this.websocket.close(1000, "Normal closure");
    }
    this.websocket = null;
    this.connectionStatus = "disconnected";
  }

  /**
   * Attempt to reconnect with exponential backoff
   * @private
   */
  private attemptReconnect(): void {
    // Clear any existing reconnect timer
    if (this.reconnectTimer !== null) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }

    // Check if max attempts reached
    if (this.reconnectAttempts >= this.maxReconnectAttempts) {
      console.error(
        `Max reconnection attempts (${this.maxReconnectAttempts}) reached. Giving up.`,
      );
      return;
    }

    // Calculate delay with exponential backoff: delay = base * 2^attempts
    const delay = this.baseReconnectDelay * Math.pow(2, this.reconnectAttempts);
    this.reconnectAttempts++;

    console.log(
      `Attempting reconnection ${this.reconnectAttempts}/${this.maxReconnectAttempts} in ${delay}ms...`,
    );

    this.reconnectTimer = setTimeout(() => {
      this.reconnectTimer = null;
      this.connect();
    }, delay);
  }

  /**
   * Register a callback for WebSocket events
   * @param event - Event type: "connect", "message", "disconnect", or "error"
   * @param callback - Function to call when event occurs
   * @returns Function to unregister the callback
   */
  on(event: "connect", callback: ConnectionCallback): () => void;
  on(event: "message", callback: MessageCallback): () => void;
  on(event: "disconnect", callback: ConnectionCallback): () => void;
  on(event: "error", callback: ErrorCallback): () => void;
  on(
    event: "connect" | "message" | "disconnect" | "error",
    callback: ConnectionCallback | MessageCallback | ErrorCallback,
  ): () => void {
    const callbackArray = this.callbacks[event];
    callbackArray.push(callback as never);

    // Return unsubscribe function
    return () => {
      const index = callbackArray.indexOf(callback as never);
      if (index !== -1) {
        callbackArray.splice(index, 1);
      }
    };
  }

  /**
   * Send a message to the WebSocket server
   * Message format must match the backend protocol (see WebSocketMessage type in types.ts)
   * @param message - Message object to send (will be JSON-serialized)
   * @throws Error if not connected
   */
  send(message: Record<string, unknown>): void {
    if (this.connectionStatus !== "connected" || this.websocket === null) {
      throw new Error("Cannot send message: WebSocket not connected");
    }

    this.websocket.send(JSON.stringify(message));
  }
}
