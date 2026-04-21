# Requirements Document

## Introduction
このドキュメントは、pfm-julia-coreの要件を定義します。既存のC++コード（pfm1）の物理的な挙動を忠実に再現しつつ、モダンなJuliaエコシステムに適合した高性能な計算エンジンを構築することを目的とします。

## Boundary Context
- **In scope**: 
    - ParallelStencil.jlを用いた2次元三相分離フェーズフィールド法の計算実装。
    - C++版（pfm1）の初期条件、物理定数、数値補正処理の忠実な再現。
    - 周期境界条件の自動処理。
    - 標準的なJuliaパッケージ構成（Project.toml, Test suites）。
    - 計算結果（濃度場データ）の外部ファイル保存機能。
- **Out of scope**: 
    - GLMakie等を用いた可視化UIの実装（pfm-julia-vizにて対応）。
    - 3次元計算の実装。
    - 分散メモリ環境（MPI）での並列化。
- **Adjacent expectations**: 
    - pfm-julia-vizからは、計算エンジンが保持する濃度場のArrayまたはObservableへのアクセスが期待されます。

## Requirements

### Requirement 1: シミュレーションエンジンの構築
**Objective:** As a 研究者, I want 計算コアがJuliaパッケージとして独立していること, so that 他の解析ツールや可視化ツールから容易に呼び出せる

#### Acceptance Criteria
1. The pfm-julia-core shall be structured as a standard Julia package with a Project.toml file and an src directory.
2. The pfm-julia-core shall expose a top-level function or structure to initialize and step the simulation.
3. When the package is loaded, the simulation engine shall initialize without external dependencies on graphics libraries.

### Requirement 2: 物理モデルと数値計算の再現
**Objective:** As a シミュレーション実行者, I want C++版（pfm1）と同じ物理的挙動を得ること, so that 過去の研究結果との整合性を維持できる

#### Acceptance Criteria
1. When initializing the grid, the pfm-julia-core shall generate a grid of user-specified dimensions (nx, ny) with average concentrations (c2a, c3a) plus a random noise of ±0.01.
2. The pfm-julia-core shall default to a 100x100 grid if no dimensions are specified, maintaining compatibility with pfm1.
3. The pfm-julia-core shall apply periodic boundary conditions across all edges of the 2D grid of any specified size.
4. The pfm-julia-core shall implement the mass conservation correction by adjusting concentrations based on the grid-wide average deviation at each step.
5. During concentration updates, the pfm-julia-core shall clamp values between 1e-6 and (1 - 1e-6) to ensure numerical stability.
6. The pfm-julia-core shall use the normalized interaction parameters (Ω=25000, T=900K, R=8.3145) and mobility coefficients as defined in pfm1.

### Requirement 3: パフォーマンスと並列化
**Objective:** As a 大規模計算を行うユーザー, I want 並列処理が効率的に行われること, so that C++版と同等以上の速度で計算できる

#### Acceptance Criteria
1. The pfm-julia-core shall use ParallelStencil.jl for the main diffusion equation solver.
2. While running on a multi-core CPU, the pfm-julia-core shall utilize Julia's multi-threading to parallelize grid updates.
3. The simulation kernel shall be written in a portable way such that it can target GPU acceleration in the future with minimal changes.

### Requirement 4: データ入出力と保存
**Objective:** As a データ分析者, I want 計算結果が整合性のある形式で保存されること, so that 既存のプロットツールや将来の可視化ツールで利用できる

#### Acceptance Criteria
1. When the simulation reaches a specified saving interval (e.g., every 2000 steps), the system shall write the current concentration fields (c2, c3) to a unique data file.
2. The system shall use a sequential numbering scheme based on the current time step for filenames (e.g., cfield_2000.dat, cfield_4000.dat) to store snapshots of the simulation.
3. Each output data file shall maintain compatibility with the pfm1 snapshot format (Paired concentration values for the entire grid).
4. If a save operation fails due to disk space or permissions, the system shall provide a clear error message without crashing the simulation.
