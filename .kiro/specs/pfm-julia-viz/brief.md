# Brief: pfm-julia-viz

## Problem
シミュレーションの進行状況をリアルタイムで確認したり、パラメータを動的に変更して結果を観察したりする手段がない。

## Current State
C++版では事後処理としてのBMP画像生成のみ。Julia版のCoreエンジンが構築予定（pfm-julia-core）。

## Desired Outcome
GLMakieを用いたインタラクティブなダッシュボードが実現されていること。計算と同期してヒートマップが更新され、シミュレーションの途中経過を動画として保存できる。

## Approach
`Package Extensions` (Julia 1.9+) を使用し、`Makie` がロードされた場合のみ可視化機能を有効にする。`Observable` を介して `pfm-julia-core` の計算ループとUIを連携させる。

## Scope
- **In**:
    - GLMakieによるヒートマップ表示。
    - Observableを用いたリアルタイム更新。
    - アニメーション記録（mp4保存）機能。
    - （オプション）パラメータ調整用スライダー。
- **Out**:
    - 物理計算ロジック（Core側に委譲）。

## Boundary Candidates
- UIレイアウト定義。
- データ変換（シミュレーション格子 -> Makie描画データ）。

## Out of Boundary
- 高度な分析ツール（フーリエ変換等）の実装。

## Upstream / Downstream
- **Upstream**: `pfm-julia-core`
- **Downstream**: なし。

## Existing Spec Touchpoints
- **Adjacent**: `pfm-julia-core` とデータ構造を共有。

## Constraints
- `Package Extension` 構成を維持し、Coreの軽量性を損なわないこと。
