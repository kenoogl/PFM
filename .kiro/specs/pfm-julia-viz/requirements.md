# Requirements Document

## Introduction
このドキュメントは、pfm-julia-vizの要件を定義します。pfm-julia-coreによる計算エンジンと連携し、GLMakieを用いたインタラクティブな可視化およびデータ保存機能を提供することを目的とします。

## Boundary Context
- **In scope**: 
    - GLMakieを用いた濃度場（c2, c3）のリアルタイム・ヒートマップ表示。
    - シミュレーションの再生、一時停止、ステップ実行、リセット機能。
    - シミュレーション過程の動画（mp4等）としての保存機能。
    - Package Extension構成による、Makieロード時のみの機能有効化。
    - 物理定数（拡散係数、相互作用パラメータ等）の動的な調整インターフェース。
- **Out of scope**: 
    - 物理計算ロジック自体（pfm-julia-coreに委譲）。
    - Webブラウザ経由の遠隔監視機能。
    - 3次元データの可視化。
- **Adjacent expectations**: 
    - pfm-julia-coreの `Simulation` オブジェクトから濃度場データへアクセスできること。
    - シミュレーションループが可視化側のリフレッシュレートを考慮して制御可能であること。

## Requirements

### Requirement 1: リアルタイム・ヒートマップ表示
**Objective:** As a 研究者, I want シミュレーションの進行状況を視覚的に確認すること, so that 三相分離のパターン形成を直感的に理解できる

#### Acceptance Criteria
1. While the simulation is running, the pfm-julia-viz shall update the heatmap display with the latest concentration values (c2, c3).
2. The pfm-julia-viz shall display the current time step and physical time on the visualization window.
3. The pfm-julia-viz shall allow the user to toggle between different visualization modes (e.g., individual component views or a composite RGB view).

### Requirement 2: シミュレーションの実行制御
**Objective:** As a ユーザー, I want シミュレーションの開始、一時停止、リセットを操作すること, so that 特定の時点での状態を詳細に観察できる

#### Acceptance Criteria
1. When the user clicks the "Start/Stop" button, the pfm-julia-viz shall toggle the execution state of the simulation loop.
2. When the user clicks the "Reset" button, the pfm-julia-viz shall re-initialize the simulation to its starting state.
3. When the user clicks the "Step" button, the pfm-julia-viz shall advance the simulation by exactly one time step.

### Requirement 3: パラメータの動的調整
**Objective:** As a 実験者, I want シミュレーション実行中に物理パラメータを変更すること, so that パラメータ感度を対話的に調査できる

#### Acceptance Criteria
1. While the simulation is paused or running, the pfm-julia-viz shall provide sliders or input fields to modify parameters such as interaction energy (Ω) and mobility coefficients.
2. When a parameter is modified via the UI, the pfm-julia-viz shall immediately update the values in the simulation engine.

### Requirement 4: アニメーションの保存
**Objective:** As a 発表者, I want シミュレーション結果を動画としてエクスポートすること, so that 研究発表や資料作成に利用できる

#### Acceptance Criteria
1. When the user enables the "Record" mode, the pfm-julia-viz shall capture frames of the visualization and save them as a video file (e.g., .mp4).
2. The pfm-julia-viz shall allow the user to specify the output filename and directory for the recorded animation.

### Requirement 5: パッケージ拡張による統合
**Objective:** As a 開発者, I want 可視化機能をオプションとして利用すること, so that 計算のみを行う際に不要なライブラリをロードしなくて済む

#### Acceptance Criteria
1. The pfm-julia-viz shall be implemented using Julia's Package Extension mechanism, ensuring that GLMakie dependencies are only loaded when explicitly imported by the user.
2. The pfm-julia-viz shall provide a simple entry point (e.g., `visualize(sim)`) that becomes available when both PhaseFieldSim and GLMakie are loaded.
