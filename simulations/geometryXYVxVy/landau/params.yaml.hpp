// SPDX-License-Identifier: MIT

#pragma once

constexpr char const* const params_yaml = R"PDI_CFG(Mesh:
  x_min: 0.0
  x_max: 12.56637061435917
  x_size: 64
  y_min: 0.0
  y_max: 12.56637061435917
  y_size : 64
  vx_min: -6.0
  vx_max: +6.0
  vx_size: 127
  vy_min: -6.0
  vy_max: +6.0
  vy_size: 127

SpeciesInfo:
- charge: -1
  mass: 0.0005
  density_eq: 1.
  temperature_eq: 1.
  mean_velocity_eq: 0.
  perturb_amplitude: 0.05
  perturb_mode: 1

Algorithm:
  deltat: 0.0625
  nbiter: 480

Output:
  time_diag: 0.25
)PDI_CFG";
