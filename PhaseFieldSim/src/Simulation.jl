# Simulation.jl

"""
    Simulation{T, A}

フェーズフィールドシミュレーションの状態とパラメータを管理する構造体です。
- `nx, ny`: 格子サイズ
- `c2, c3`: コンポーネント2および3の濃度場
- `c2_new, c3_new`: 次のステップの濃度場用バッファ
- `mu2, mu3`: 化学ポテンシャル場
- `om12, om23, om13`: 相互作用パラメータ
- `kappa2, kappa3`: 勾配エネルギ係数
- `cmob22, cmob33, cmob23, cmob32`: 動的移動度係数
- `dt`: 時間刻み
- `c2a, c3a`: 平均濃度（初期値）
"""
struct Simulation{T, A}
    nx::Int
    ny::Int
    c2::A
    c3::A
    c2_new::A
    c3_new::A
    mu2::A
    mu3::A
    
    # パラメータ
    om12::T
    om23::T
    om13::T
    kappa2::T
    kappa3::T
    cmob22::T
    cmob33::T
    cmob23::T
    cmob32::T
    dt::T
    
    # 初期平均濃度
    c2a::T
    c3a::T
end

"""
    Simulation(nx::Int=100, ny::Int=100; c2a=0.3, c3a=0.3)

Simulation構造体を初期化します。デフォルトは100x100の格子です。
濃度場は平均値 (c2a, c3a) に ±0.01 のランダムノイズを加えた状態で初期化されます。
"""
function Simulation(nx::Int=100, ny::Int=100; c2a=0.3, c3a=0.3)
    # ParallelStencilのデータ型を使用して配列を初期化
    # @zeros マクロは @init_parallel_stencil で指定された型（Float64）を使用します
    c2 = @zeros(nx, ny)
    c3 = @zeros(nx, ny)
    c2_new = @zeros(nx, ny)
    c3_new = @zeros(nx, ny)
    mu2 = @zeros(nx, ny)
    mu3 = @zeros(nx, ny)
    
    # 物理定数から相互作用パラメータを計算
    # PhaseFieldSimモジュールで定義された OMEGA, R_GAS, TEMP を使用
    om = OMEGA / (R_GAS * TEMP)
    
    # pfm1のデフォルト値
    om12 = om
    om23 = om
    om13 = om
    kappa2 = 1.0
    kappa3 = 1.0
    cmob22 = 1.0
    cmob33 = 1.0
    cmob23 = 0.0
    cmob32 = 0.0
    dt = 0.1
    
    T = Data.Number
    A = Data.Array
    
    sim = Simulation{T, A}(
        nx, ny, c2, c3, c2_new, c3_new, mu2, mu3,
        T(om12), T(om23), T(om13), T(kappa2), T(kappa3),
        T(cmob22), T(cmob33), T(cmob23), T(cmob32), T(dt),
        T(c2a), T(c3a)
    )
    
    # ノイズによる初期化とクランプ
    initialize_noise!(sim)
    
    return sim
end

"""
    initialize_noise!(sim::Simulation)

濃度場に ±0.01 のランダムノイズを加え、質量保存を維持するように補正した後、
濃度範囲を [1e-6, 1-1e-6] にクランプします。
"""
function initialize_noise!(sim::Simulation)
    # ホスト側でノイズを生成して補正
    h_c2 = Array(sim.c2)
    h_c3 = Array(sim.c3)
    
    nx, ny = sim.nx, sim.ny
    
    # ±0.01のノイズを加える
    for i in 1:nx, j in 1:ny
        h_c2[i, j] = sim.c2a + 0.01 * (2.0 * rand() - 1.0)
        h_c3[i, j] = sim.c3a + 0.01 * (2.0 * rand() - 1.0)
    end
    
    # 質量保存の補正 (合計を目標値に合わせる)
    avg_c2 = sum(h_c2) / (nx * ny)
    avg_c3 = sum(h_c3) / (nx * ny)
    
    h_c2 .-= (avg_c2 - sim.c2a)
    h_c3 .-= (avg_c3 - sim.c3a)
    
    # ParallelStencil配列へ書き戻し
    copyto!(sim.c2, h_c2)
    copyto!(sim.c3, h_c3)
    
    # カーネルによるクランプ処理
    @parallel clamp_concentrations_kernel!(sim.c2, sim.c3)
    
    return
end
