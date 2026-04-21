module PhaseFieldSimMakieExt

using PhaseFieldSim
using GLMakie
using Makie
using Printf

"""
    PhaseFieldSim.visualize(sim::Simulation)

Simulationオブジェクトを受け取り、GLMakieを用いた可視化ダッシュボードを表示します。
濃度場 c2, c3 のヒートマップ、およびステップ数とシミュレーション時間を表示します。
実行制御用のボタン（Start/Stop, Step, Reset）を備えています。
"""
function PhaseFieldSim.visualize(sim::Simulation)
    # Figureの作成
    fig = Figure(size = (1000, 700))
    
    # レイアウト設定: 濃度場 c2 と c3 の Axis を作成
    ax2 = Axis(fig[1, 1], title = "Concentration c2", aspect = DataAspect())
    ax3 = Axis(fig[1, 2], title = "Concentration c3", aspect = DataAspect())
    
    # データの Observable 化
    obs_c2 = Observable(Array(sim.c2))
    obs_c3 = Observable(Array(sim.c3))
    
    # ヒートマップの描画
    hm2 = heatmap!(ax2, obs_c2, colorrange = (0, 1), colormap = :viridis)
    hm3 = heatmap!(ax3, obs_c3, colorrange = (0, 1), colormap = :plasma)
    
    # カラーバーの追加
    Colorbar(fig[1, 3], hm2, label = "c2")
    Colorbar(fig[1, 4], hm3, label = "c3")

    # ステップ数と時間の表示用 Observable
    obs_step = Observable(0)
    obs_time = Observable(0.0)
    
    # 表示用テキストの生成
    label_text = lift(obs_step, obs_time) do s, t
        @sprintf("Step: %d, Time: %.3f", s, t)
    end
    
    # 上部に情報のラベルを配置
    Label(fig[0, :], label_text, fontsize = 20, font = :bold, tellwidth = false)
    
    # --- 実行制御インターフェース ---
    
    # 実行状態の管理
    is_running = Observable(false)
    
    # ボタンの配置用グリッド
    controls = fig[2, :] = GridLayout(tellwidth = false)
    
    btn_run = Button(controls[1, 1], label = lift(x -> x ? "Stop" : "Start", is_running))
    btn_step = Button(controls[1, 2], label = "Step")
    btn_reset = Button(controls[1, 3], label = "Reset")
    
    # Start/Stop ボタンのロジック
    on(btn_run.clicks) do _
        is_running[] = !is_running[]
    end
    
    # Step ボタンのロジック
    on(btn_step.clicks) do _
        if !is_running[]
            PhaseFieldSim.step!(sim)
            obs_c2[] = Array(sim.c2)
            obs_c3[] = Array(sim.c3)
            obs_step[] += 1
            obs_time[] += sim.dt
        end
    end
    
    # Reset ボタンのロジック
    on(btn_reset.clicks) do _
        is_running[] = false
        PhaseFieldSim.initialize_noise!(sim)
        obs_c2[] = Array(sim.c2)
        obs_c3[] = Array(sim.c3)
        obs_step[] = 0
        obs_time[] = 0.0
    end
    
    # シミュレーションループ
    # @async を使用して GUI スレッドをブロックせずに実行
    @async while isopen(fig.scene)
        if is_running[]
            PhaseFieldSim.step!(sim)
            obs_c2[] = Array(sim.c2)
            obs_c3[] = Array(sim.c3)
            obs_step[] += 1
            obs_time[] += sim.dt
            # 描画の更新タイミングを確保
            yield()
        else
            # 停止中は負荷を下げる
            sleep(0.1)
        end
    end
    
    # レイアウトの比率調整
    colsize!(fig.layout, 1, Relative(0.45))
    colsize!(fig.layout, 2, Relative(0.45))
    
    return fig
end

end # module
