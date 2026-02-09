# React Fundamentals for Code Review

This guide helps you evaluate React code quality when reviewing AI-generated output.

## Core Concepts

### Components

React apps are built from components - reusable pieces of UI.

**Function Component (Modern Standard):**
```jsx
function Button({ label, onClick }) {
  return <button onClick={onClick}>{label}</button>;
}
```

**What to check:**
- Components should do ONE thing
- Props are destructured in the function signature
- Component names are PascalCase
- Return JSX (HTML-like syntax)

### JSX Rules

JSX looks like HTML but has differences:

| HTML | JSX |
|------|-----|
| `class="..."` | `className="..."` |
| `for="..."` | `htmlFor="..."` |
| `onclick="..."` | `onClick={...}` |
| `style="color: red"` | `style={{ color: 'red' }}` |

**What to check:**
- `className` not `class`
- Event handlers are camelCase (`onClick`, `onChange`)
- Style is an object, not a string
- Self-closing tags must end with `/>` (e.g., `<img />`)

### State with useState

State is data that changes over time and triggers re-renders.

```jsx
function Counter() {
  const [count, setCount] = useState(0);

  return (
    <button onClick={() => setCount(count + 1)}>
      Count: {count}
    </button>
  );
}
```

**What to check:**
- `useState` returns `[value, setValue]` array
- Setter function name starts with `set`
- State updates trigger re-render
- Initial value is passed to `useState()`

### Effects with useEffect

Side effects (API calls, subscriptions, DOM manipulation) go in useEffect.

```jsx
function UserProfile({ userId }) {
  const [user, setUser] = useState(null);

  useEffect(() => {
    // Runs when userId changes
    fetch(`/api/users/${userId}`)
      .then(res => res.json())
      .then(data => setUser(data));
  }, [userId]); // Dependency array

  return user ? <div>{user.name}</div> : <div>Loading...</div>;
}
```

**What to check:**
- Dependencies in the array match what's used in the effect
- Empty array `[]` means "run once on mount"
- No array means "run on every render" (usually wrong)
- Cleanup function returned if needed (subscriptions, timers)

### Event Handling

```jsx
function Form() {
  const [value, setValue] = useState('');

  const handleSubmit = (e) => {
    e.preventDefault(); // Prevent page reload
    console.log(value);
  };

  return (
    <form onSubmit={handleSubmit}>
      <input
        value={value}
        onChange={(e) => setValue(e.target.value)}
      />
      <button type="submit">Submit</button>
    </form>
  );
}
```

**What to check:**
- `e.preventDefault()` on form submit
- Controlled inputs have both `value` and `onChange`
- Event handlers don't have `()` when passed as props: `onClick={handleClick}` not `onClick={handleClick()}`

## Common Patterns

### Conditional Rendering

```jsx
// Good: && operator for simple conditions
{isLoggedIn && <LogoutButton />}

// Good: Ternary for if/else
{isLoggedIn ? <LogoutButton /> : <LoginButton />}

// Good: Early return for loading/error states
if (loading) return <Spinner />;
if (error) return <Error message={error} />;
return <Content data={data} />;
```

### Lists and Keys

```jsx
function TodoList({ todos }) {
  return (
    <ul>
      {todos.map(todo => (
        <li key={todo.id}>{todo.text}</li>
      ))}
    </ul>
  );
}
```

**What to check:**
- Every item in a list needs a `key` prop
- Keys should be stable IDs, not array indexes (unless list is static)
- Keys must be unique among siblings

### Props Passing

```jsx
// Passing props down
<Button label="Click me" onClick={handleClick} disabled={isLoading} />

// Receiving props
function Button({ label, onClick, disabled = false }) {
  return <button onClick={onClick} disabled={disabled}>{label}</button>;
}
```

**What to check:**
- Props are read-only (never mutate them)
- Default values can be set with `= value` in destructuring
- Boolean props can be passed without value: `<Button disabled />` equals `disabled={true}`

## Red Flags in AI-Generated React

### 1. Direct DOM Manipulation

```jsx
// BAD - Don't do this in React
document.getElementById('myDiv').innerHTML = 'Hello';

// GOOD - Let React handle the DOM
setContent('Hello');
```

### 2. Mutating State Directly

```jsx
// BAD - Mutating state
const [items, setItems] = useState([]);
items.push(newItem); // WRONG

// GOOD - Create new array
setItems([...items, newItem]);
```

### 3. Missing Dependencies in useEffect

```jsx
// BAD - Missing dependency
useEffect(() => {
  fetchData(userId);
}, []); // userId should be in the array

// GOOD
useEffect(() => {
  fetchData(userId);
}, [userId]);
```

### 4. Infinite Loops

```jsx
// BAD - Causes infinite re-renders
useEffect(() => {
  setCount(count + 1);
}); // No dependency array = runs every render

// BAD - Object/array in dependency causes infinite loop
useEffect(() => {
  doSomething();
}, [{ someObject }]); // New object every render
```

### 5. Not Handling Async Properly

```jsx
// BAD - Setting state after unmount
useEffect(() => {
  fetch('/api/data').then(res => setData(res));
}, []);

// GOOD - Cancel or check if mounted
useEffect(() => {
  let cancelled = false;
  fetch('/api/data').then(res => {
    if (!cancelled) setData(res);
  });
  return () => { cancelled = true; };
}, []);
```

### 6. Incorrect Event Handler Syntax

```jsx
// BAD - Calls function immediately
<button onClick={handleClick()}>Click</button>

// GOOD - Passes function reference
<button onClick={handleClick}>Click</button>

// GOOD - Arrow function for passing arguments
<button onClick={() => handleClick(id)}>Click</button>
```

## Quick Checklist for Code Review

When reviewing React code, check:

- [ ] Components are function components (not class)
- [ ] State uses `useState` hook
- [ ] Effects use `useEffect` with correct dependencies
- [ ] Lists have unique `key` props
- [ ] No direct DOM manipulation
- [ ] State is never mutated directly
- [ ] Event handlers don't have `()` when passed as props
- [ ] `className` used instead of `class`
- [ ] Forms have `onSubmit` with `preventDefault`
- [ ] Loading and error states are handled

## Modern React Patterns (2024+)

### Custom Hooks

Extract reusable logic into custom hooks:

```jsx
function useApi(url) {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  useEffect(() => {
    fetch(url)
      .then(res => res.json())
      .then(setData)
      .catch(setError)
      .finally(() => setLoading(false));
  }, [url]);

  return { data, loading, error };
}

// Usage
function UserList() {
  const { data, loading, error } = useApi('/api/users');
  // ...
}
```

### Component Composition

Prefer composition over prop drilling:

```jsx
// Instead of passing many props down
<Layout user={user} theme={theme} notifications={notifications}>
  <Sidebar user={user} notifications={notifications} />
  <Content user={user} theme={theme} />
</Layout>

// Use composition
<Layout>
  <Sidebar>
    <UserInfo user={user} />
    <NotificationBadge count={notifications.length} />
  </Sidebar>
  <Content theme={theme}>
    <UserDashboard user={user} />
  </Content>
</Layout>
```

## Resources for Deeper Learning

- [React Documentation](https://react.dev/) - Official docs (excellent)
- [React Tutorial](https://react.dev/learn) - Interactive tutorial
- [Thinking in React](https://react.dev/learn/thinking-in-react) - Mental model

The official React documentation at react.dev is genuinely excellent and covers these concepts in depth with interactive examples.
