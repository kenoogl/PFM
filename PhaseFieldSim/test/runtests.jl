using PhaseFieldSim
using Test
using Statistics
using Printf
using ParallelStencil

@testset "PhaseFieldSim.jl" begin
    @testset "Physical Constants" begin
        @test PhaseFieldSim.OMEGA == 25000.0
        @test PhaseFieldSim.TEMP == 900.0
        @test PhaseFieldSim.R_GAS == 8.3145
    end

    @testset "Simulation Initialization (Req 2.1, 2.2)" begin
        # Default (100x100)
        sim1 = Simulation()
        @test sim1.nx == 100
        @test sim1.ny == 100
        @test size(sim1.c2) == (100, 100)

        # Arbitrary grid size (Req 2.1: nx, ny)
        nx, ny = 64, 128
        c2a, c3a = 0.3, 0.3
        sim2 = Simulation(nx, ny; c2a=c2a, c3a=c3a)
        @test sim2.nx == nx
        @test sim2.ny == ny
        @test size(sim2.c2) == (64, 128)

        # Noise initialization (Req 2.1: ±0.01)
        h_c2 = Array(sim2.c2)
        h_c3 = Array(sim2.c3)
        # We allow a bit more margin for clamping and correction effects
        @test all(h_c2 .>= c2a - 0.015)
        @test all(h_c2 .<= c2a + 0.015)
        @test all(h_c3 .>= c3a - 0.015)
        @test all(h_c3 .<= c3a + 0.015)

        # Initial mass conservation (Req 2.4)
        @test mean(h_c2) ≈ c2a atol=1e-10
        @test mean(h_c3) ≈ c3a atol=1e-10

        # Concentration clamping (Req 2.5)
        kEps = 1e-6
        @test all(h_c2 .>= kEps)
        @test all(h_c2 .<= 1.0 - kEps)
        @test all(h_c3 .>= kEps)
        @test all(h_c3 .<= 1.0 - kEps)
        @test all(h_c2 .+ h_c3 .<= 1.0)
    end

    @testset "Stability and Mass Conservation (Req 2.4, 2.5)" begin
        nx, ny = 64, 64
        c2a, c3a = 0.3, 0.3
        sim = Simulation(nx, ny; c2a=c2a, c3a=c3a)
        
        # Long simulation run (1000 steps)
        steps = 1000
        for i in 1:steps
            step!(sim)
            # Occasional check for NaNs to fail early
            if i % 100 == 0
                @test !any(isnan.(Array(sim.c2)))
                @test !any(isnan.(Array(sim.c3)))
            end
        end
        
        h_c2 = Array(sim.c2)
        h_c3 = Array(sim.c3)
        
        # Stability: No NaNs
        @test !any(isnan.(h_c2))
        @test !any(isnan.(h_c3))
        
        # Mass conservation: Mean concentration should remain constant
        @test mean(h_c2) ≈ c2a atol=1e-10
        @test mean(h_c3) ≈ c3a atol=1e-10
        
        # Concentration clamping: Should be maintained over time
        kEps = 1e-6
        @test all(h_c2 .>= kEps)
        @test all(h_c3 .>= kEps)
        @test all(h_c2 .+ h_c3 .<= 1.0)
        
        # Dynamics: Fields should have evolved from initial random noise
        @test !all(h_c2 .== Array(Simulation(nx, ny; c2a=c2a, c3a=c3a).c2)) # Different from new random start
    end

    @testset "Format Verification and IO (Req 4.3)" begin
        nx, ny = 10, 10
        sim = Simulation(nx, ny)
        output_dir = "test_output_suite"
        step_num = 2000
        
        if isdir(output_dir)
            rm(output_dir, recursive=true)
        end
        
        save_snapshot(sim, step_num, dir=output_dir)
        sleep(0.5) # Wait for async IO
        
        expected_file = joinpath(output_dir, @sprintf("cfield_%d.dat", step_num))
        @test isfile(expected_file)
        
        # Verify format matches pfm1 pattern
        lines = readlines(expected_file)
        @test length(lines) >= 2
        
        # 1st line: Time as Float
        @test !isnothing(tryparse(Float64, lines[1]))
        @test parse(Float64, lines[1]) ≈ step_num * sim.dt
        
        # 2nd line and onwards: Data pairs
        # In pfm1, there might be multiple lines of data.
        # Let's check the total number of elements.
        all_data = Float64[]
        for i in 2:length(lines)
            append!(all_data, [parse(Float64, x) for x in split(lines[i]) if !isempty(x)])
        end
        
        # Total elements should be 2 * nx * ny (c2 and c3 for each point)
        @test length(all_data) == 2 * nx * ny
        
        # Values should be within [0, 1]
        @test all(0.0 .<= all_data .<= 1.0)
        
        # Format check: pfm1 usually uses 2 or more space between values or specific scientific notation
        # Just check if it's readable as Float64 and has correct count
        
        # Cleanup
        rm(output_dir, recursive=true)
    end

    @testset "Dynamic Parameters (Task 2.3)" begin
        sim = Simulation(10, 10)
        
        # Test om12 update
        new_om = 123.45
        sim.om12 = new_om
        @test sim.om12 == new_om
        
        # Test mobility update
        new_mob = 0.88
        sim.cmob22 = new_mob
        @test sim.cmob22 == new_mob
        
        # Ensure step! works with new parameters
        @test (step!(sim); true)
    end

    @testset "Visualization API (Req 5.2)" begin
        sim = Simulation(10, 10)
        # Calling visualize without GLMakie should throw ArgumentError
        @test_throws ArgumentError visualize(sim)
        
        # Verify the error message contains GLMakie
        try
            visualize(sim)
        catch e
            @test e isa ArgumentError
            @test occursin("GLMakie", e.msg)
        end
    end

    include("test_viz.jl")
end
