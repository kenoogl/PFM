# Implementation Plan: pfm-julia-viz

## Foundation: Project Setup

- [ ] 1. Foundation: Project Scaffolding
- [x] 1.1 Configure Package Extension metadata
  - Add `GLMakie` and `Makie` to `[weakdeps]` in `PhaseFieldSim/Project.toml`.
  - Define `PhaseFieldSimMakieExt` in `[extensions]` section.
  - `Project.toml` correctly reflects the weak dependency structure for extensions.
  - _Requirements: 5.1_
  - _Boundary: Project Metadata_
- [x] 1.2 Define visualization entry point
  - Implement a `visualize(sim::Simulation)` stub in `src/PhaseFieldSim.jl` that throws an informative error if GLMakie is not loaded.
  - Export the `visualize` function from the main module.
  - Calling `visualize` without GLMakie returns a clear instruction to the user.
  - _Requirements: 5.2_
  - _Boundary: Core API_

## Core: Visualization Logic

- [ ] 2. Core: Visualization Logic
- [x] 2.1 (P) Heatmap Dashboard Implementation
  - Create `ext/PhaseFieldSimMakieExt.jl` and implement the main visualization window layout.
  - Setup `Observable` objects for concentration fields (c2, c3) and display them using heatmaps.
  - Implement display of current time and step number in the UI.
  - A functional dashboard window opens with heatmap displays when `using GLMakie` is active.
  - _Requirements: 1.1, 1.2, 1.3_
  - _Boundary: Visualization Extension_
- [x] 2.2 (P) Execution Control Interface
  - Add "Start/Stop", "Step", and "Reset" buttons to the dashboard.
  - Wire button clicks to an `is_running` Observable or internal state.
  - Buttons are visible and toggle the intended logical states in the UI.
  - _Requirements: 2.1, 2.2, 2.3_
  - _Boundary: Visualization Extension_
- [x] 2.3 (P) Dynamic Parameter Interface
  - Implement a `SliderGrid` or individual sliders for Ω (interaction parameter) and mobility coefficients.
  - Ensure slider values are linked to the simulation parameters.
  - Sliders are functional and reflect/update values in the simulation context.
  - _Requirements: 3.1, 3.2_
  - _Boundary: Visualization Extension_

## Core: Integration

- [x] 3. Core: Integration
- [x] 3.1 Non-blocking Simulation Loop
  - Implement the main execution task that calls `step!(sim)` while `is_running[]` is true.
  - Ensure the loop uses `yield()` or `sleep()` to maintain UI responsiveness.
  - シミュレーション progresses in the background while the UI remains interactive.
  - _Requirements: 2.1_
  - _Boundary: Visualization Extension_
  - _Depends: 2.1, 2.2_
- [x] 3.2 Parameter Synchronization
  - Ensure that changes in the UI sliders are immediately propagated to the underlying `Simulation` struct.
  - Verify that the simulation behavior changes in real-time as sliders are adjusted.
  - _Requirements: 3.2_
  - _Boundary: Visualization Extension_
  - _Depends: 2.3_

## Features: Export and Testing

- [ ] 4. Features: Export and Testing
- [x] 4.1 Animation Export Functionality
  - Implement the "Record" mode to capture and save simulation frames using Makie's `record` function.
  - Allow the user to specify output destination for the .mp4 file.
  - A valid video file is generated in the output directory after a recording session.
  - _Requirements: 4.1, 4.2_
  - _Boundary: Visualization Extension_
- [x] 4.2 Verification and Smoke Tests
  - Create `test/test_viz.jl` to verify that the extension is correctly detected by Julia when GLMakie is loaded.
  - Perform a manual smoke test to confirm UI responsiveness and basic visual correctness.
  - Extension loading is verified via `Base.get_extension`, and UI elements are visually confirmed.
  - _Requirements: 5.1_
  - _Boundary: Testing_
