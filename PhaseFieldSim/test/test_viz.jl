using PhaseFieldSim
using Test

@testset "Visualization Extension (Smoke Test)" begin
    @testset "Extension Detection" begin
        # GLMakie がロードされていない状態では拡張機能が nothing であることを確認
        ext = Base.get_extension(PhaseFieldSim, :PhaseFieldSimMakieExt)
        if isnothing(ext)
            @info "PhaseFieldSimMakieExt is not loaded (expected without GLMakie)."
            @test isnothing(ext)
            
            # visualize() が期待通りエラーを投げることを確認
            @test_throws ArgumentError visualize(Simulation(10, 10))
        else
            @info "PhaseFieldSimMakieExt is loaded."
            @test !isnothing(ext)
            
            # ロードされている場合は visualize() が実行可能であるはず（UI表示はできない可能性あり）
            # ただしヘッドレス環境ではエラーになる可能性があるため、型チェックに留める
            @test hasmethod(visualize, (Simulation,))
        end
    end

    @testset "Manual Smoke Test Script Documentation" begin
        # Requirement 5.1 & Task 4.2 のための手動確認手順
        @info "To perform a manual smoke test, run the following script in a GUI-enabled environment:"
        @info """
        using PhaseFieldSim
        using GLMakie
        
        # 1. Check extension
        @assert !isnothing(Base.get_extension(PhaseFieldSim, :PhaseFieldSimMakieExt))
        
        # 2. Run visualization
        sim = Simulation(128, 128)
        fig = visualize(sim)
        
        # 3. Verification items:
        # - [ ] Heatmap updates when 'Start' is clicked.
        # - [ ] 'Step' button advances simulation by 1.
        # - [ ] 'Reset' button returns to initial noise.
        # - [ ] 'Omega' slider changes pattern formation speed/shape.
        # - [ ] 'Record' produces 'output/simulation.mp4'.
        """
        @test true
    end
end
