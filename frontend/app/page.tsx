"use client";

export default function Home() {
  return (
    <main className="h-screen flex flex-col items-center justify-center bg-gray-950 text-white">
      <div className="text-center max-w-2xl px-8">
        <h1 className="text-4xl font-bold mb-4">Tank Dynamics Simulator</h1>
        <h2 className="text-xl text-gray-300 mb-8">
          Real-time SCADA Interface
        </h2>

        <div className="mb-8 text-gray-400">
          <p className="mb-4">
            A real-time tank level control simulator with WebSocket-based
            communication and SCADA-style interface for process monitoring and
            control.
          </p>
        </div>

        <div className="text-sm text-gray-500">
          <p>Status: Waiting for connection setup in next tasks</p>
        </div>
      </div>
    </main>
  );
}
