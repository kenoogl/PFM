module PhaseFieldSim

using ParallelStencil
using Printf
using StaticArrays

# Requirement 2.6: Normalized interaction parameters and physical constants
const OMEGA = 25000.0  # Ω interaction parameter [J/mol]
const TEMP  = 900.0    # Temperature [K]
const R_GAS = 8.3145   # Gas constant [J/(mol*K)]

export OMEGA, TEMP, R_GAS

"""
    Simulation

Placeholder for the simulation state. To be implemented in Task 2.
"""
struct Simulation
    # Placeholder
end

function greet()
    println("PhaseFieldSim initialized.")
end

end # module
