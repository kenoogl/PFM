# Research & Design Decisions: pfm-julia-core

## Summary
- **Feature**: pfm-julia-core
- **Discovery Scope**: New Feature (Porting from C++)
- **Key Findings**:
  - `ParallelStencil.jl` は、格子計算（stencil updates）を抽象化し、CPU/GPUの両方で高性能なコードを生成できる。
  - C++版の質量保存補正ロジックは、全格子の平均値を計算して調整するものであり、これは `ParallelStencil` のリダクション操作として実装可能。
  - Julia 1.11以降のパッケージ構造では、`public` キーワードや `Package Extension` の活用が推奨される。

## Research Log

### ParallelStencil.jl の適用可能性
- **Context**: C++版の陽解法（有限差分）をJuliaで高速化しつつ、将来のGPU対応を容易にしたい。
- **Sources Consulted**: [ParallelStencil.jl Documentation](https://github.com/omlins/ParallelStencil.jl)
- **Findings**:
  - `@init_parallel_stencil` でバックエンド（CPU/CUDA/AMDGPU等）を切り替え可能。
  - `@parallel_indices` でインデックスベースのカーネルを記述できる。
  - `Data.Number`, `Data.Array` 型を使用してバックエンド非依存なデータ構造を作成できる。
- **Implications**: C++の多重ループをほぼそのままマクロ内に移植でき、かつ並列性能も確保できる。

### 質量保存補正の並列実装
- **Context**: `three_phase_decomp.cpp` では、毎ステップ全格子の濃度を合計して平均のズレを補正している。
- **Findings**:
  - `ParallelStencil` には合計（`sum`）などのリダクション機能がある。
  - 補正（`c2h[i][j] = c2h2[i][j] - dc2a`）も並列カーネルとして記述可能。
- **Implications**: 計算全体をCPU/GPUにオフロード可能。

## Architecture Pattern Evaluation

| Option | Description | Strengths | Risks / Limitations | Notes |
|--------|-------------|-----------|---------------------|-------|
| ParallelStencil | 物理シミュレーション特化のStencil抽象化 | CPU/GPUポータビリティ、数式に近い記述 | マクロの学習コスト、バックエンドの事前決定 | 本プロジェクトに最適 |
| KernelAbstractions | 低レベルな並列プログラミング抽象化 | 柔軟性が高い、多くのデバイスをサポート | Stencil計算を自前で抽象化する必要がある | 今回はParallelStencilの方が簡潔 |

## Design Decisions

### Decision: 構造体ベースのシミュレーション管理
- **Context**: 任意サイズ（nx, ny）や物理定数を柔軟に管理したい。
- **Selected Approach**: `Simulation` 構造体を定義し、データ配列とパラメータを保持する。
- **Rationale**: 複数のシミュレーションを並列に走らせたり、パラメータ変更を容易にするため。

### Decision: データ保存形式
- **Context**: Requirement 4.2（連番ファイル保存）。
- **Selected Approach**: `Printf.jl` を用いて `cfield_0002000.dat` のような形式で保存。
- **Rationale**: ファイル名ソートが容易になり、可視化ツールでの読み込みがスムーズになる。

## Risks & Mitigations
- 浮動小数点精度の差異 — Juliaではデフォルトで `Float64` を使用し、必要に応じて `Float32` に切り替え可能にする。
- ParallelStencilの初期化コスト — パッケージの初回ロード時にコンパイルが発生するが、Julia 1.10+ のプリコンパイル強化により軽減。
