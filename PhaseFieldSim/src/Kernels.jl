using ParallelStencil

# Requirement 2.3: Periodic boundary conditions
# Requirement 3.1: ParallelStencil kernels

"""
    compute_mu_kernel!(mu2, mu3, c2, c3, om12, om23, om13, kappa2, kappa3)

ParallelStencil kernel to calculate chemical potentials mu2 and mu3 for a three-phase system.
Handles periodic boundary conditions internally.
"""
@parallel_indices (i, j) function compute_mu_kernel!(mu2, mu3, c2, c3, om12, om23, om13, kappa2, kappa3)
    nx, ny = size(c2)
    
    # Periodic boundary mapping
    ip = (i == nx) ? 1 : i + 1
    im = (i == 1)  ? nx : i - 1
    jp = (j == ny) ? 1 : j + 1
    jm = (j == 1)  ? ny : j - 1

    c2_val = c2[i, j]
    c3_val = c3[i, j]
    c1_val = 1.0 - c2_val - c3_val
    
    # Laplacian of concentrations (Finite difference)
    lap2 = c2[ip, j] + c2[im, j] + c2[i, jp] + c2[i, jm] - 4.0 * c2_val
    lap3 = c3[ip, j] + c3[im, j] + c3[i, jp] + c3[i, jm] - 4.0 * c3_val
    
    # Chemical potential (derivative of free energy + gradient term)
    # Using log(c) with values clamped to [1e-6, 1-1e-6] elsewhere to avoid singularity
    mu2_chem = om12 * (c1_val - c2_val) - om13 * c3_val + om23 * c3_val + log(c2_val) - log(c1_val)
    mu2_grad = -2.0 * kappa2 * lap2 - kappa3 * lap3
    mu2[i, j] = mu2_chem + mu2_grad
    
    mu3_chem = om13 * (c1_val - c3_val) - om12 * c2_val + om23 * c2_val + log(c3_val) - log(c1_val)
    mu3_grad = -2.0 * kappa3 * lap3 - kappa2 * lap2
    mu3[i, j] = mu3_chem + mu3_grad
    
    return
end

"""
    update_c_kernel!(c2_new, c3_new, c2, c3, mu2, mu3, cmob22, cmob33, cmob23, cmob32, dt)

ParallelStencil kernel to update concentration fields c2 and c3 based on chemical potentials.
Implements the Cahn-Hilliard equation with periodic boundary conditions.
"""
@parallel_indices (i, j) function update_c_kernel!(c2_new, c3_new, c2, c3, mu2, mu3, cmob22, cmob33, cmob23, cmob32, dt)
    nx, ny = size(c2)
    
    # Periodic boundary mapping
    ip = (i == nx) ? 1 : i + 1
    im = (i == 1)  ? nx : i - 1
    jp = (j == ny) ? 1 : j + 1
    jm = (j == 1)  ? ny : j - 1

    # Laplacian of chemical potentials
    lap_mu2 = mu2[ip, j] + mu2[im, j] + mu2[i, jp] + mu2[i, jm] - 4.0 * mu2[i, j]
    lap_mu3 = mu3[ip, j] + mu3[im, j] + mu3[i, jp] + mu3[i, jm] - 4.0 * mu3[i, j]
    
    # Cahn-Hilliard update
    c2_new[i, j] = c2[i, j] + (cmob22 * lap_mu2 + cmob23 * lap_mu3) * dt
    c3_new[i, j] = c3[i, j] + (cmob32 * lap_mu2 + cmob33 * lap_mu3) * dt
    
    return
end

"""
    apply_correction_kernel!(c, dc)

ParallelStencil kernel to apply mass conservation correction.
Subtracts the average deviation `dc` from each cell in concentration field `c`.
"""
@parallel_indices (i, j) function apply_correction_kernel!(c, dc)
    c[i, j] = c[i, j] - dc
    return
end

"""
    clamp_concentrations_kernel!(c2, c3)

ParallelStencil kernel to clamp concentration values between 1e-6 and 1-1e-6.
Also ensures c2 + c3 <= 1.0 by scaling if necessary, matching C++ pfm1 logic.
"""
@parallel_indices (i, j) function clamp_concentrations_kernel!(c2, c3)
    kEps = 1.0e-06
    
    # Read current values
    val2 = c2[i, j]
    val3 = c3[i, j]
    
    # Individual clamping
    if val2 >= 1.0; val2 = 1.0 - kEps; end
    if val2 <= 0.0; val2 = kEps; end
    if val3 >= 1.0; val3 = 1.0 - kEps; end
    if val3 <= 0.0; val3 = kEps; end
    
    # Combined clamping/scaling
    if (val2 + val3 >= 1.0)
        scale = (1.0 - 2.0 * kEps) / (val2 + val3)
        val2 = val2 * scale
        val3 = val3 * scale
    end
    
    # Write back
    c2[i, j] = val2
    c3[i, j] = val3
    
    return
end
