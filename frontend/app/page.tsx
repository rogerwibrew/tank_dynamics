"use client";

import { useState } from "react";
import TabNavigation from "@/components/TabNavigation";
import { ProcessView } from "@/components/ProcessView";
import { TrendsView } from "@/components/TrendsView";
import { ConnectionStatus } from "@/components/ConnectionStatus";

export default function Home() {
  const [activeTab, setActiveTab] = useState("process");

  return (
    <main className="h-screen flex flex-col bg-gray-950 text-white">
      {/* Header Section */}
      <div className="bg-gray-900 border-b border-gray-700 px-8 py-6">
        <div className="flex items-start justify-between">
          <div>
            <h1 className="text-3xl font-bold text-white">
              Tank Dynamics Simulator
            </h1>
            <p className="text-gray-400 mt-1">Real-time SCADA Interface</p>
          </div>
          <ConnectionStatus />
        </div>
      </div>

      {/* Tab Navigation Section */}
      <TabNavigation activeTab={activeTab} onTabChange={setActiveTab} />

      {/* Main Content Area */}
      <div className="flex-1 overflow-auto px-8 py-6">
        {activeTab === "process" ? <ProcessView /> : <TrendsView />}
      </div>
    </main>
  );
}
