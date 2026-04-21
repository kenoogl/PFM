using Printf

"""
    save_snapshot(sim::Simulation, step::Int; dir="output")

シミュレーションの現在の状態（濃度場）をファイルに保存します。
ファイル名は `cfield_[step].dat` 形式です。
Requirement 4.2, 4.3 に基づき、pfm1互換のフォーマットで出力します。
"""
function save_snapshot(sim::Simulation, step::Int; dir="output")
    # 出力ディレクトリの作成 (Requirement 4.4)
    if !isdir(dir)
        mkpath(dir)
    end

    # ファイル名の生成
    # Requirement 4.2: cfield_2000.dat
    filename = joinpath(dir, @sprintf("cfield_%d.dat", step))

    # 非同期で保存を実行 (Instruction: Asynchronous saving)
    # 濃度場データのコピーをキャプチャして非同期処理に渡す
    c2_data = Array(sim.c2)
    c3_data = Array(sim.c3)
    time_val = step * sim.dt

    @async try
        open(filename, "w") do io
            # 1行目: 時間 (Requirement 4.3, pfm1 compatibility)
            @printf(io, "%f\n", time_val)
            
            # 2行目: 濃度場のペア (c2, c3)
            # pfm1: fprintf(stream, "%e  %e  ", c2h[i][j], c3h[i][j]);
            nx, ny = size(c2_data)
            for i in 1:nx
                for j in 1:ny
                    @printf(io, "%e  %e  ", c2_data[i, j], c3_data[i, j])
                end
            end
            println(io) # 末尾に改行
        end
    catch e
        @error "Failed to save snapshot to $filename" exception=(e, catch_backtrace())
    end
end
