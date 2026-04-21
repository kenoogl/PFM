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
    is_recording = Observable(false)
    
    # ボタンの配置用グリッド
    controls = fig[2, :] = GridLayout(tellwidth = false)
    
    btn_run = Button(controls[1, 1], label = lift(x -> x ? "Stop" : "Start", is_running))
    btn_step = Button(controls[1, 2], label = "Step")
    btn_reset = Button(controls[1, 3], label = "Reset")

    Label(controls[1, 4], "File:")
    txt_filename = Textbox(controls[1, 5], placeholder = "simulation.mp4", width = 150)
    btn_record = Button(controls[1, 6], label = lift(x -> x ? "Recording..." : "Record", is_recording))
    
    # Start/Stop ボタンのロジック
    on(btn_run.clicks) do _
        if !is_recording[]
            is_running[] = !is_running[]
        end
    end
    
    # Step ボタンのロジック
    on(btn_step.clicks) do _
        if !is_running[] && !is_recording[]
            PhaseFieldSim.step!(sim)
            obs_c2[] = Array(sim.c2)
            obs_c3[] = Array(sim.c3)
            obs_step[] += 1
            obs_time[] += sim.dt
        end
    end
    
    # Reset ボタンのロジック
    on(btn_reset.clicks) do _
        if !is_recording[]
            is_running[] = false
            PhaseFieldSim.initialize_noise!(sim)
            obs_c2[] = Array(sim.c2)
            obs_c3[] = Array(sim.c3)
            obs_step[] = 0
            obs_time[] = 0.0
        end
    end

    # Record ボタンのロジック (Requirement 4.1, 4.2)
    on(btn_record.clicks) do _
        if is_recording[]
            return
        end

        # ファイル名の決定
        fname = txt_filename.stored_string[]
        if isempty(fname)
            fname = "simulation.mp4"
        end

        # output ディレクトリをデフォルトとする
        if !contains(fname, "/") && !contains(fname, "\\")
            fname = joinpath("output", fname)
        end

        # ディレクトリの作成
        mkpath(dirname(fname))

        is_recording[] = true
        was_running = is_running[]
        is_running[] = false

        # 録画の実行 (200ステップ)
        # record はブロッキング関数なので、UIスレッドで実行される
        try
            @info "Recording to $fname ..."
            record(fig, fname, 1:200) do i
                PhaseFieldSim.step!(sim)
                obs_c2[] = Array(sim.c2)
                obs_c3[] = Array(sim.c3)
                obs_step[] += 1
                obs_time[] += sim.dt
            end
            @info "Recording finished."
        catch e
            @error "Recording failed: $e"
        finally
            is_recording[] = false
            is_running[] = was_running
        end
    end
    
    # --- パラメータ調整インターフェース (Requirement 3.1, 3.2) ---
    
    params_layout = fig[3, :] = GridLayout(tellwidth = false)
    
    # スライダーグリッドの作成
    sg = SliderGrid(
        params_layout[1, 1],
        (label = "Ω", range = 10000:100:50000, format = "{:.0f}", startvalue = 25000),
        (label = "cmob22", range = -1.0:0.01:1.0, format = "{:.2f}", startvalue = sim.cmob22),
        (label = "cmob33", range = -1.0:0.01:1.0, format = "{:.2f}", startvalue = sim.cmob33),
        (label = "cmob23", range = -1.0:0.01:1.0, format = "{:.2f}", startvalue = sim.cmob23),
        (label = "cmob32", range = -1.0:0.01:1.0, format = "{:.2f}", startvalue = sim.cmob32),
        width = 600,
        tellheight = true
    )
    
    # スライダーの値をシミュレーションのパラメータに反映
    on(sg.sliders[1].value) do val
        # Ω [J/mol] を無次元の相互作用パラメータに変換
        val_norm = val / (PhaseFieldSim.R_GAS * PhaseFieldSim.TEMP)
        sim.om12 = val_norm
        sim.om23 = val_norm
        sim.om13 = val_norm
    end
    
    on(sg.sliders[2].value) do val; sim.cmob22 = val; end
    on(sg.sliders[3].value) do val; sim.cmob33 = val; end
    on(sg.sliders[4].value) do val; sim.cmob23 = val; end
    on(sg.sliders[5].value) do val; sim.cmob32 = val; end
    
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
