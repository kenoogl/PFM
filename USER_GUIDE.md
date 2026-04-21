# PhaseFieldSim ユーザーガイド

本プロジェクトは、C++で実装された三相分離フェーズフィールド・シミュレーションをJuliaへ移植し、GLMakieによるリアルタイム可視化を可能にしたものです。

## 1. セットアップ

シミュレーションを実行する前に、必要な依存関係をインストールする必要があります。

```bash
# プロジェクトディレクトリで実行
julia --project=PhaseFieldSim -e 'using Pkg; Pkg.instantiate(); Pkg.add(["GLMakie", "Makie"])'
```

## 2. シミュレーションの実行

Julia REPLをプロジェクト環境で起動します。

```bash
julia --project=PhaseFieldSim
```

REPL内で以下のコマンドを入力して可視化ウィンドウを立ち上げます。

```julia
using PhaseFieldSim, GLMakie

# シミュレーションの初期化（格子サイズを指定可能、デフォルトは 100x100）
sim = Simulation(128, 128)

# 可視化ダッシュボードの表示
visualize(sim)
```

## 3. ダッシュボードの操作方法

可視化ウィンドウには以下の機能が備わっています。

### 実行制御 (Controls)
- **Start / Stop**: シミュレーションの計算を継続実行 / 一時停止します。
- **Step**: 一時停止中に1ステップだけ計算を進めます。
- **Reset**: 濃度場を初期状態（ランダムノイズ）に戻し、時間をリセットします。

### パラメータ調整 (Parameters)
スライダーを動かすことで、計算を止めることなく物理パラメータをリアルタイムに変更できます。
- **Ω (Interaction energy)**: 相分離の駆動力を調整します。
- **cmob22, cmob33...**: 各成分の移動度（拡散の速さ）を個別に調整します。

### 動画保存 (Recording)
- **File**: 保存するファイル名（例: `result.mp4`）を入力します（デフォルトは `output/simulation.mp4`）。
- **Record**: ボタンを押すと、その時点から **200ステップ分** のシミュレーションを録画し、ファイルとして保存します。
  * ※ FFMPEGがインストールされている必要があります。

## 4. 高度な利用方法

可視化を行わずに計算のみを実行し、結果を `.dat` ファイルとして保存することも可能です。

```julia
using PhaseFieldSim

sim = Simulation(200, 200)
for step in 1:10000
    step!(sim)
    if step % 2000 == 0
        save_snapshot(sim, step) # output/cfield_2000.dat 等に保存
    end
end
```

## 5. プロジェクト構造

- `src/Kernels.jl`: ParallelStencilを用いた高性能な計算カーネル。
- `src/Simulation.jl`: シミュレーションの状態管理と物理定数。
- `src/IO.jl`: pfm1互換フォーマットでのデータ出力。
- `ext/PhaseFieldSimMakieExt.jl`: GLMakieを用いた可視化機能の実装（Package Extension）。
