# Research & Design Decisions: pfm-julia-viz

## Summary
- **Feature**: pfm-julia-viz
- **Discovery Scope**: New Feature (Extension to pfm-julia-core)
- **Key Findings**:
  - `Observables.jl`（Makieの核）を用いることで、濃度場データの変更を効率的に描画に反映できる。大規模データの場合、配列を直接書き換えて `notify` を呼ぶのがベストプラクティス。
  - `Package Extensions` により、`using GLMakie` された時のみ可視化ロジックをロードする構成が可能。
  - `record` 関数により、シミュレーションループと同期した動画生成が容易に行える。

## Research Log

### GLMakie リアルタイム更新の最適化
- **Context**: 100x100（あるいはそれ以上）の格子を高速に更新したい。
- **Findings**: 
  - `heatmap!` に渡すデータ自体を `Observable` にする。
  - 毎ステップ `obs[] = new_data` とするとコピーが発生するため、`obs.val .= new_data` してから `notify(obs)` を呼ぶのが高速。
- **Implications**: `Simulation` 構造体のデータを直接参照するか、可視化用のバッファを用意して `notify` する。

### 実行制御（Start/Stop）の実装
- **Context**: UIボタンでシミュレーションの実行状態を切り替えたい。
- **Findings**:
  - `is_running = Observable(false)` を用意する。
  - シミュレーションループを `is_running` の監視、あるいは非同期タスク内で回す。
- **Implications**: `while is_running[] ... end` ループ内で `yield()` を挟むことでUIの応答性を確保する。

### Package Extensions の構造
- **Context**: `PhaseFieldSim` パッケージ本体に Makie 依存を入れたくない。
- **Findings**:
  - `Project.toml` の `[weakdeps]` に `GLMakie` を追加。
  - `[extensions]` に `PhaseFieldSimMakieExt = "GLMakie"` を定義。
  - `ext/PhaseFieldSimMakieExt.jl` に実装を記述。
- **Implications**: ユーザーが `using PhaseFieldSim, GLMakie` とした時だけ `visualize` 関数が有効になる。

## Design Decisions

### Decision: 可視化用エントリポイントの設計
- **Context**: ユーザーがどのように可視化を開始するか。
- **Selected Approach**: `PhaseFieldSim.visualize(sim::Simulation)` というスタブ関数を定義し、Extension側でオーバーライドする。
- **Rationale**: パッケージの公開APIとして一貫性を保つため。

### Decision: 描画データの形式
- **Context**: 三相（c1, c2, c3）をどう見せるか。
- **Selected Approach**: c2 と c3 のヒートマップを並べる、あるいは RGB 合成（R=c1, G=c2, B=c3）を選択可能にする。
- **Rationale**: 三相分離の挙動を確認するには、成分ごとの分布と全体のバランスの両方が重要。

## Risks & Mitigations
- **描画負荷**: 高解像度格子では描画がボトルネックになる可能性がある。 -> `sleep` 時間の調整や、数ステップに1回の描画更新を行えるようにする。
- **動画保存時の不具合**: FFMPEGがインストールされていない環境でのエラー。 -> エラーハンドリングを行い、適切なメッセージを表示する。
