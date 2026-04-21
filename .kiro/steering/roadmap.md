# Roadmap

## Overview
pfm1にあるC++製の三相分離フェーズフィールド・シミュレーションを、モダンなJulia環境に移植し、高速化と高度な可視化を実現します。

## Approach Decision
- **Chosen**: ParallelStencil.jl + Makie.jl (Package Extension構成)
- **Why**: 
    - `ParallelStencil.jl` を使用することで、C++と同等のパフォーマンスを維持しつつ、将来的なGPU対応を容易にする。
    - `Package Extension` を活用し、シミュレーションエンジンと可視化機能を分離することで、計算のみを行う際のロード時間を短縮する。
    - `Makie.jl` の `Observable` を利用し、インタラクティブな可視化を実現する。
- **Rejected alternatives**: 
    - `LoopVectorization.jl (@turbo)`: Julia 1.11以降での非推奨化に伴い採用を見送り。
    - 単一スクリプト実装: 再利用性とテスト容易性を重視し、パッケージ構造を採用。

## Scope
- **In**:
    - フェーズフィールド法（三相分離）のJulia実装。
    - ParallelStencil.jlによるCPU/GPUポータブルな計算カーネル。
    - GLMakieを用いたリアルタイム可視化および動画保存。
    - プロジェクトパッケージ化（Project.toml, src/, test/）。
- **Out**:
    - 分散並列計算（MPI）。
    - 3Dへの拡張（設計レベルでは考慮するが、実装は2Dに限定）。

## Boundary Strategy
- **Why this split**: 計算コア（Core）と可視化（Viz）を分離することで、計算エンジンの純粋なテストと、可視化UIの柔軟な変更を両立させる。
- **Shared seams to watch**: シミュレーションの各ステップ後のデータ受け渡し（Observableの更新タイミング）。

## Specs (dependency order)
- [ ] pfm-julia-core -- 高性能シミュレーションエンジンの構築。Dependencies: none
- [ ] pfm-julia-viz -- Makieによるインタラクティブ可視化機能。Dependencies: pfm-julia-core
