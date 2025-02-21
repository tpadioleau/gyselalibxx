// SPDX-License-Identifier: MIT

#include <ddc/ddc.hpp>

#include "maxwellianequilibrium.hpp"

MaxwellianEquilibrium::MaxwellianEquilibrium(
        DFieldSp density_eq,
        DFieldSp temperature_eq,
        DFieldSp mean_velocity_eq)
    : m_density_eq(std::move(density_eq))
    , m_temperature_eq(std::move(temperature_eq))
    , m_mean_velocity_eq(std::move(mean_velocity_eq))
{
}

DSpanSpVx MaxwellianEquilibrium::operator()(DSpanSpVx const allfequilibrium) const
{
    IDomainVx const gridvx = allfequilibrium.domain<IDimVx>();
    IDomainSp const gridsp = allfequilibrium.domain<IDimSp>();

    // Initialization of the maxwellian
    DFieldVx maxwellian(gridvx);
    ddc::for_each(gridsp, [&](IndexSp const isp) {
        compute_maxwellian(
                maxwellian,
                m_density_eq(isp),
                m_temperature_eq(isp),
                m_mean_velocity_eq(isp));

        ddc::for_each(gridvx, [&](IndexVx const iv) { allfequilibrium(isp, iv) = maxwellian(iv); });
    });
    return allfequilibrium;
}

/*
 Computing the non-centered Maxwellian function as
   fM(v) = n/(sqrt(2*PI*T))*exp(-(v-u)**2/(2*T))
  with n the density and T the temperature and
  where u is the mean velocity
*/
void MaxwellianEquilibrium::compute_maxwellian(
        DSpanVx const fMaxwellian,
        double const density,
        double const temperature,
        double const mean_velocity)
{
    double const inv_sqrt_2piT = 1. / std::sqrt(2. * M_PI * temperature);
    IDomainVx const gridvx = fMaxwellian.domain();
    for (IndexVx const iv : gridvx) {
        CoordVx const v = ddc::coordinate(iv);
        fMaxwellian(iv)
                = density * inv_sqrt_2piT
                  * std::exp(-(v - mean_velocity) * (v - mean_velocity) / (2. * temperature));
    }
}
