# Brief: pfm-julia-core

## Problem
既存のC++コード（pfm1）は、拡張性やインタラクティブな実験が困難であり、モダンなJuliaの数値計算エコシステムの恩恵を受けられていない。

## Current State
C++で実装された100x100格子のフェーズフィールド・シミュレーションが存在する。標準Cライブラリのみを使用し、BMP出力による簡易的な可視化を行っている。

## Desired Outcome
ParallelStencil.jlを用いた高性能なJulia版シミュレーションエンジンが構築されていること。C++と同等以上の速度を出し、単体テストで正当性が保証されている。

## Approach
`ParallelStencil.jl` を用いて、周期境界条件と差分計算を抽象化・高速化する。標準的なJuliaパッケージ構造（`Project.toml`）を採用し、`Polyester.jl` 等によるマルチスレッド並列化も活用する。

## Scope
- **In**:
    - 物理定数および初期条件の設定。
    - フェーズフィールド法の時間発展計算（ParallelStencilカーネル）。
    - 周期境界条件の処理。
    - パッケージの基本構造とテスト。
- **Out**:
    - 可視化（GUI/Plot）コード。

## Boundary Candidates
- 計算カーネル（Stencil定義）。
- シミュレーションの実行制御ループ（Step-by-step実行）。

## Out of Boundary
- Makieなどの重いグラフィックス依存関係の導入。

## Upstream / Downstream
- **Upstream**: なし。
- **Downstream**: `pfm-julia-viz`

## Existing Spec Touchpoints
- なし。

## Constraints
- Julia 1.10以上に対応。
- `ParallelStencil` の抽象化レイヤーを維持する。
