using PhaseFieldSim
using Test

@testset "PhaseFieldSim.jl" begin
    @test PhaseFieldSim.OMEGA == 25000.0
    @test PhaseFieldSim.TEMP == 900.0
    @test PhaseFieldSim.R_GAS == 8.3145
end
