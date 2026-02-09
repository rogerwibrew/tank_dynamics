#chemical-engineering

# Deriving the ODEs Related to Tank Level Dynamics

## Aim

Derive the ODEs describing the operation of a tank with a single liquid feed and an outlet control valve with position governed by PID controlling level.

This will feed into a C++ function that gives the values of the derivatives given the current values of the state vector. The function will be used by an external ODE stepper function to derive the values of the state vector as time progresses.

## Description

Fixed tank with incompressible (constant density) fluid flowing into the tank. Tank outflow is controlled by a valve with a setpoint controlled by a PID controlling tank level.

## Equation Derivation

### Main ODE - Material Balance in Tank

mass In - mass Out = Gen + Acc

We do not generate any mass so Gen is 0

We assume a constant density $\rho$.  As such $m = \rho V$ and $dm = dV$

$$ q_{in} - q_{out} = \frac{dV}{dt}$$
As the cross sectional area is constant $V=Ah$.

$$
\boxed{\dot{h}(t)=\frac{1}{A}\left(q_{in}(t)-q_{out}(t)\right)}
$$

### Valve dynamics

$q_{in}$ is a function of external processes and we can assume it is out of our control. $q_{out}$ is within our ability to control as we can open and close the control valve.

We assume gravity draining of the tank.  Therefore the flow out of the tank will be a function of tank level.  

Outlet (gravity drain) supplementary equation:

$$
\boxed{q_{out}(t)=C_d\,A_v\!\big(x(t)\big)\,\sqrt{2g\,h(t)}}
$$

Common linear valve-area map:

$$
\boxed{A_v(x)=A_{v,\max}\,x(t)}, \qquad 0 \le x \le 1
$$

So equivalently:

$$
\boxed{q_{out}(t)=k_v\,x(t)\sqrt{h(t)}}, \qquad \boxed{k_v=C_d\,A_{v,\max}\sqrt{2g}}
$$
we decide on a constant that is sensible and the valve is around 50% open at normal conditions.

### PID Control Equations 

Error:
$$
\boxed{e(t)=h_{sp}(t)-h(t)}
$$

Integral state:
$$
\boxed{\dot{z}(t)=e(t)}
$$

PID law (valve opening with saturation):
$$
\boxed{x(t)=\mathrm{sat}\!\left(u_b+K_c\left(e(t)+\frac{1}{\tau_I}z(t)+\tau_D\,\dot{e}(t)\right),\,0,\,1\right)}
$$

Error derivative:
$$
\boxed{\dot{e}(t)=\dot{h}_{sp}(t)-\dot{h}(t)}
$$

Saturation function:
$$
\boxed{\mathrm{sat}(v,0,1)=\min\!\left(1,\max\!\left(0,v\right)\right)}
$$

### PID controller pseudocode (instant valve position, with saturation)

**Given:** `Kc, tau_I, tau_D, u_b, dt, x_min=0, x_max=1`  
**Persistent state:** `z` (integrator state)

```text
# Inputs each time step:
#   h      = measured level
#   h_sp   = setpoint
#   h_dot  = level time-derivative (from plant model), if using D term
# Output:
#   x      = valve opening (0..1)

e = h_sp - h

# Integrator state ODE discretized (e.g., forward Euler)
z = z + e * dt

# Error derivative (if setpoint is constant, e_dot = -h_dot)
e_dot = h_sp_dot - h_dot    # if h_sp_dot is known; else set h_sp_dot = 0

# Unsaturated PID output
x_unsat = u_b + Kc * ( e + (1/tau_I) * z + tau_D * e_dot )

# Saturate to physical valve limits
x = clamp(x_unsat, x_min, x_max)

# Helper
function clamp(v, lo, hi):
    if v < lo: return lo
    if v > hi: return hi
    return v
```

## Related

- [[Process Dynamics Simulation]]
- [[Two-Film Model]]
- [[Fick's Law in One Dimension for Mass Transfer]]

