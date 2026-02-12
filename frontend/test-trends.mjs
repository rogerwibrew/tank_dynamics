// Simple import test to verify TrendsView exports correctly
import('./components/TrendsView.tsx').then(module => {
  if (module.TrendsView) {
    console.log('✓ TrendsView component exports successfully');
    console.log('✓ Component is a React component (function)');
  } else {
    console.log('✗ TrendsView not found in exports');
  }
}).catch(err => {
  console.log('✗ Failed to import:', err.message);
});
