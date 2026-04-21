# Implementation Plan: pfm-julia-core

## Foundation: Project Setup

- [x] 1. Foundation: Project Scaffolding
  - Create standard Julia package structure for `PhaseFieldSim`.
  - Configure `Project.toml` with dependencies: `ParallelStencil`, `Printf`, `StaticArrays`.
  - Establish `src/PhaseFieldSim.jl` as the main entry point without any graphics dependencies.
  - `julia --project -e 'using PhaseFieldSim'` succeeds without errors.
  - _Requirements: 1.1, 1.3, 2.6_

## Core: Computational Kernels

- [ ] 2. Core: Computational Kernels
- [ ] 2.1 (P) Diffusion Equation Kernels
  - Define `ParallelStencil` kernels for Cahn-Hilliard diffusion and chemical potential calculation.
  - Implement periodic boundary index mapping within the `@parallel_indices` macros.
  - Ensure kernels are designed for future portability between CPU and GPU backends.
  - Kernels for chemical potential and concentration updates are defined and compile within `src/Kernels.jl`.
  - _Requirements: 2.3, 3.1, 3.3_
  - _Boundary: Kernels_
- [ ] 2.2 (P) Numerical Utility Kernels
  - Define kernels for concentration clamping between `1e-6` and `1 - 1e-6`.
  - Implement reduction kernels for mass conservation summation across the grid.
  - Clamping and summation kernels are verified to work correctly on `ParallelStencil` arrays.
  - _Requirements: 2.4, 2.5_
  - _Boundary: Kernels_

## Core: Simulation Engine

- [ ] 3. Core: Simulation Engine
- [ ] 3.1 Simulation State Management
  - Implement the `Simulation` struct to hold grid data, buffers, and physical constants.
  - Implement initialization logic using random noise (±0.01) with support for arbitrary grid dimensions (nx, ny).
  - `Simulation(nx, ny)` creates a valid state with mass-conserved noise, defaulting to 100x100.
  - _Requirements: 1.2, 2.1, 2.2_
  - _Boundary: Simulation_
- [ ] 3.2 Simulation Control Flow
  - Implement the high-level `step!` function to coordinate the sequence of kernel calls.
  - Integrate the mass conservation correction logic into the simulation loop.
  - Configure the simulation to utilize Julia's multi-threading for grid updates.
  - `step!(sim)` updates the concentration fields while strictly maintaining total mass and stability.
  - _Requirements: 1.2, 2.4, 3.2_
  - _Boundary: Simulation_
  - _Depends: 2.1, 2.2_

## Core: Data IO

- [ ] 4. Data IO: Snapshot Storage
  - Implement asynchronous file saving logic using the `cfield_[STEP].dat` sequential naming scheme.
  - Ensure the output format maintains compatibility with pfm1's paired numerical data structure.
  - Implement robustness checks to handle file system errors gracefully.
  - Snapshot files are successfully written to the `output/` directory at the requested intervals.
  - _Requirements: 4.1, 4.2, 4.3, 4.4_
  - _Boundary: IO_

## Validation: Test Suite

- [ ] 5. Validation: Test Suite
  - Implement unit tests for noise initialization, grid scaling, and concentration clamping.
  - Implement integration tests to verify mass conservation stability over long simulation runs.
  - Implement format verification to ensure generated .dat files match pfm1's sample data.
  - `test/runtests.jl` passes all physics, stability, and compatibility checks.
  - _Requirements: 2.1, 2.4, 2.5, 4.3_
  - _Boundary: Testing_
