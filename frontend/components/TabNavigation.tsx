interface TabNavigationProps {
  activeTab: string;
  onTabChange: (tab: string) => void;
}

export default function TabNavigation({ activeTab, onTabChange }: TabNavigationProps) {
  return (
    <div role="tablist" className="flex border-b border-gray-700">
      <button
        role="tab"
        aria-selected={activeTab === "process"}
        aria-controls="process-panel"
        onClick={() => onTabChange("process")}
        className={`px-6 py-3 font-medium transition-colors duration-200 ${
          activeTab === "process"
            ? "bg-blue-600 text-white border-b-2 border-blue-400"
            : "bg-gray-800 text-gray-300 hover:bg-gray-700 hover:text-white"
        }`}
      >
        Process
      </button>
      <button
        role="tab"
        aria-selected={activeTab === "trends"}
        aria-controls="trends-panel"
        onClick={() => onTabChange("trends")}
        className={`px-6 py-3 font-medium transition-colors duration-200 ${
          activeTab === "trends"
            ? "bg-blue-600 text-white border-b-2 border-blue-400"
            : "bg-gray-800 text-gray-300 hover:bg-gray-700 hover:text-white"
        }`}
      >
        Trends
      </button>
    </div>
  );
}
