#pragma once

#include <iomanip>

#include <ddc/ddc.hpp>

#include <sll/gauss_legendre_integration.hpp>
#include <sll/math_tools.hpp>
#include <sll/null_boundary_value.hpp>
#include <sll/polar_spline.hpp>
#include <sll/polar_spline_evaluator.hpp>

#include <Eigen/Sparse>

//#include "chargedensitycalculator.hpp"
//#include "ipoissonsolver.hpp"

template <class PolarBSplines>
class PolarSplineFEMPoissonSolver
{
public:
    using BSplinesR = typename PolarBSplines::BSplinesR_tag;
    using BSplinesP = typename PolarBSplines::BSplinesP_tag;

    using DimR = typename BSplinesR::tag_type;
    using DimP = typename BSplinesP::tag_type;

public:
    using SplineRP = ddc::ChunkSpan<double, ddc::DiscreteDomain<BSplinesR, BSplinesP>>;
    using SplinePolar = PolarSpline<PolarBSplines>;

    struct RBasisSubset
    {
    };
    struct PBasisSubset
    {
    };
    struct RCellDim
    {
    };
    struct PCellDim
    {
    };

    template <class T>
    struct InternalTagGenerator
    {
        using tag = T;
    };

private:
    using QDimR = InternalTagGenerator<DimR>;
    using QDimP = InternalTagGenerator<DimP>;
    using QDimRMesh = ddc::NonUniformPointSampling<QDimR>;
    using QDimPMesh = ddc::NonUniformPointSampling<QDimP>;
    using QuadratureDomainR = ddc::DiscreteDomain<QDimRMesh>;
    using QuadratureDomainP = ddc::DiscreteDomain<QDimPMesh>;
    using QuadratureDomainRP = ddc::DiscreteDomain<QDimRMesh, QDimPMesh>;
    using QuadratureMeshR = ddc::DiscreteElement<QDimRMesh>;
    using QuadratureMeshP = ddc::DiscreteElement<QDimPMesh>;
    using QuadratureMeshRP = ddc::DiscreteElement<QDimRMesh, QDimPMesh>;
    using QuadratureLengthR = ddc::DiscreteVector<QDimRMesh>;
    using QuadratureLengthP = ddc::DiscreteVector<QDimPMesh>;

    struct EvalDeriv1DType
    {
        double value;
        double derivative;
    };
    struct EvalDeriv2DType
    {
        double value;
        double radial_derivative;
        double poloidal_derivative;
    };

    using BSpline2DDomain = ddc::DiscreteDomain<BSplinesR, BSplinesP>;
    using IDimBSpline2D = ddc::DiscreteElement<BSplinesR, BSplinesP>;
    using IDimPolarBspl = ddc::DiscreteElement<PolarBSplines>;

    using CellIndex = ddc::DiscreteElement<RCellDim, PCellDim>;

    using MatrixElement = Eigen::Triplet<double>;

private:
    static constexpr int n_gauss_legendre_r = BSplinesR::degree() + 1;
    static constexpr int n_gauss_legendre_p = BSplinesP::degree() + 1;
    static constexpr int n_overlap_cells = PolarBSplines::continuity + 1;

    static constexpr ddc::DiscreteVector<RBasisSubset> n_non_zero_bases_r
            = ddc::DiscreteVector<RBasisSubset>(BSplinesR::degree() + 1);
    static constexpr ddc::DiscreteVector<PBasisSubset> n_non_zero_bases_p
            = ddc::DiscreteVector<PBasisSubset>(BSplinesP::degree() + 1);

    static constexpr ddc::DiscreteDomain<RBasisSubset> non_zero_bases_r = ddc::DiscreteDomain<
            RBasisSubset>(ddc::DiscreteElement<RBasisSubset> {0}, n_non_zero_bases_r);
    static constexpr ddc::DiscreteDomain<PBasisSubset> non_zero_bases_p = ddc::DiscreteDomain<
            PBasisSubset>(ddc::DiscreteElement<PBasisSubset> {0}, n_non_zero_bases_p);

    const int nbasis_r;
    const int nbasis_p;

    // Domains
    ddc::DiscreteDomain<PolarBSplines> fem_non_singular_domain;
    ddc::DiscreteDomain<BSplinesR> radial_bsplines;
    ddc::DiscreteDomain<BSplinesP> polar_bsplines;

    QuadratureDomainR quadrature_domain_r;
    QuadratureDomainP quadrature_domain_p;
    QuadratureDomainRP quadrature_domain_singular;

    // Gauss-Legendre points and weights
    ddc::Chunk<ddc::Coordinate<QDimR>, QuadratureDomainR> points_r;
    ddc::Chunk<ddc::Coordinate<QDimP>, QuadratureDomainP> points_p;
    ddc::Chunk<double, QuadratureDomainR> weights_r;
    ddc::Chunk<double, QuadratureDomainP> weights_p;

    // Basis Spline values and derivatives at Gauss-Legendre points
    ddc::Chunk<EvalDeriv2DType, ddc::DiscreteDomain<PolarBSplines, QDimRMesh, QDimPMesh>>
            singular_basis_vals_and_derivs;
    ddc::Chunk<EvalDeriv1DType, ddc::DiscreteDomain<RBasisSubset, QDimRMesh>>
            r_basis_vals_and_derivs;
    ddc::Chunk<EvalDeriv1DType, ddc::DiscreteDomain<PBasisSubset, QDimPMesh>>
            p_basis_vals_and_derivs;

    ddc::Chunk<double, QuadratureDomainRP> int_volume;

    PolarSplineEvaluator<PolarBSplines> m_polar_spline_evaluator;

    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> m_matrix;

public:
    template <class Mapping>
    PolarSplineFEMPoissonSolver(
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_alpha,
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_beta,
            Mapping const& mapping)
        : nbasis_r(ddc::discrete_space<BSplinesR>().nbasis() - n_overlap_cells - 1)
        , nbasis_p(ddc::discrete_space<BSplinesP>().nbasis())
        , fem_non_singular_domain(
                  ddc::discrete_space<PolarBSplines>().non_singular_domain().remove_last(
                          ddc::DiscreteVector<PolarBSplines> {nbasis_p}))
        , radial_bsplines(ddc::discrete_space<BSplinesR>().full_domain().remove_first(
                  ddc::DiscreteVector<BSplinesR> {n_overlap_cells}))
        , polar_bsplines(ddc::discrete_space<BSplinesP>().full_domain().take_first(
                  ddc::DiscreteVector<BSplinesP> {nbasis_p}))
        , quadrature_domain_r(
                  ddc::DiscreteElement<QDimRMesh>(0),
                  ddc::DiscreteVector<QDimRMesh>(
                          n_gauss_legendre_r * ddc::discrete_space<BSplinesR>().ncells()))
        , quadrature_domain_p(
                  ddc::DiscreteElement<QDimPMesh>(0),
                  ddc::DiscreteVector<QDimPMesh>(
                          n_gauss_legendre_p * ddc::discrete_space<BSplinesP>().ncells()))
        , quadrature_domain_singular(
                  quadrature_domain_r.take_first(
                          ddc::DiscreteVector<QDimRMesh> {n_overlap_cells * n_gauss_legendre_r}),
                  quadrature_domain_p)
        , points_r(quadrature_domain_r)
        , points_p(quadrature_domain_p)
        , weights_r(quadrature_domain_r)
        , weights_p(quadrature_domain_p)
        , singular_basis_vals_and_derivs(ddc::DiscreteDomain<PolarBSplines, QDimRMesh, QDimPMesh>(
                  PolarBSplines::singular_domain(),
                  ddc::select<QDimRMesh>(quadrature_domain_singular),
                  ddc::select<QDimPMesh>(quadrature_domain_singular)))
        , r_basis_vals_and_derivs(ddc::DiscreteDomain<
                                  RBasisSubset,
                                  QDimRMesh>(non_zero_bases_r, quadrature_domain_r))
        , p_basis_vals_and_derivs(ddc::DiscreteDomain<
                                  PBasisSubset,
                                  QDimPMesh>(non_zero_bases_p, quadrature_domain_p))
        , int_volume(QuadratureDomainRP(quadrature_domain_r, quadrature_domain_p))
        , m_polar_spline_evaluator(g_polar_null_boundary_2d<PolarBSplines>)
    {
        const std::size_t ncells_r = ddc::discrete_space<BSplinesR>().ncells();
        const std::size_t ncells_p = ddc::discrete_space<BSplinesP>().ncells();

        // Get break points
        ddc::DiscreteDomain<RCellDim> r_edges_dom(
                ddc::DiscreteElement<RCellDim>(0),
                ddc::DiscreteVector<RCellDim>(ncells_r + 1));
        ddc::DiscreteDomain<PCellDim> p_edges_dom(
                ddc::DiscreteElement<PCellDim>(0),
                ddc::DiscreteVector<PCellDim>(ncells_p + 1));
        ddc::Chunk<ddc::Coordinate<QDimR>, ddc::DiscreteDomain<RCellDim>> breaks_r(r_edges_dom);
        ddc::Chunk<ddc::Coordinate<QDimP>, ddc::DiscreteDomain<PCellDim>> breaks_p(p_edges_dom);

        ddc::for_each(r_edges_dom, [&](ddc::DiscreteElement<RCellDim> i) {
            breaks_r(i) = ddc::Coordinate<QDimR>(
                    ddc::get<DimR>(ddc::discrete_space<BSplinesR>().get_knot(i.uid())));
        });
        ddc::for_each(p_edges_dom, [&](ddc::DiscreteElement<PCellDim> i) {
            breaks_p(i) = ddc::Coordinate<QDimP>(
                    ddc::get<DimP>(ddc::discrete_space<BSplinesP>().get_knot(i.uid())));
        });

        // Define quadrature points and weights
        GaussLegendre<QDimR> gl_coeffs_r(n_gauss_legendre_r);
        GaussLegendre<QDimP> gl_coeffs_p(n_gauss_legendre_p);
        gl_coeffs_r.compute_points_and_weights_on_mesh(
                points_r.span_view(),
                weights_r.span_view(),
                breaks_r.span_cview());
        gl_coeffs_p.compute_points_and_weights_on_mesh(
                points_p.span_view(),
                weights_p.span_view(),
                breaks_p.span_cview());

        std::vector<double> vect_points_r(points_r.size());
        for (auto i : quadrature_domain_r) {
            vect_points_r[i.uid()] = points_r(i);
        }
        std::vector<double> vect_points_p(points_p.size());
        for (auto i : quadrature_domain_p) {
            vect_points_p[i.uid()] = points_p(i);
        }

        // Create quadrature domain
        ddc::init_discrete_space<QDimRMesh>(vect_points_r);
        ddc::init_discrete_space<QDimPMesh>(vect_points_p);

        // Find value and derivative of 1D bsplines in radial direction
        ddc::for_each(quadrature_domain_r, [&](QuadratureMeshR const ir) {
            std::array<double, 2 * n_non_zero_bases_r> data;
            DSpan2D vals(data.data(), n_non_zero_bases_r, 2);
            ddc::discrete_space<BSplinesR>().eval_basis_and_n_derivs(vals, get_coordinate(ir), 1);
            for (auto ib : non_zero_bases_r) {
                r_basis_vals_and_derivs(ib, ir).value = vals(ib.uid(), 0);
                r_basis_vals_and_derivs(ib, ir).derivative = vals(ib.uid(), 1);
            }
        });

        // Find value and derivative of 1D bsplines in poloidal direction
        ddc::for_each(quadrature_domain_p, [&](QuadratureMeshP const ip) {
            std::array<double, 2 * n_non_zero_bases_p> data;
            DSpan2D vals(data.data(), n_non_zero_bases_p, 2);
            ddc::discrete_space<BSplinesP>().eval_basis_and_n_derivs(vals, get_coordinate(ip), 1);
            for (auto ib : non_zero_bases_p) {
                p_basis_vals_and_derivs(ib, ip).value = vals(ib.uid(), 0);
                p_basis_vals_and_derivs(ib, ip).derivative = vals(ib.uid(), 1);
            }
        });

        auto singular_domain = PolarBSplines::singular_domain();

        // Find value and derivative of 2D bsplines covering the singular point
        ddc::for_each(quadrature_domain_singular, [&](QuadratureMeshRP const irp) {
            std::array<double, PolarBSplines::n_singular_basis()> singular_data;
            std::array<double, n_non_zero_bases_r * n_non_zero_bases_p> data;
            DSpan1D singular_vals(singular_data.data(), PolarBSplines::n_singular_basis());
            DSpan2D vals(data.data(), n_non_zero_bases_r, n_non_zero_bases_p);

            QuadratureMeshR ir = ddc::select<QDimRMesh>(irp);
            QuadratureMeshP ip = ddc::select<QDimPMesh>(irp);

            const ddc::Coordinate<DimR, DimP> coord(get_coordinate(irp));

            // Calculate the value
            ddc::discrete_space<PolarBSplines>().eval_basis(singular_vals, vals, coord);
            for (auto ib : singular_domain) {
                singular_basis_vals_and_derivs(ib, ir, ip).value = singular_vals[ib.uid()];
            }

            // Calculate the radial derivative
            ddc::discrete_space<PolarBSplines>().eval_deriv_r(singular_vals, vals, coord);
            for (auto ib : singular_domain) {
                singular_basis_vals_and_derivs(ib, ir, ip).radial_derivative
                        = singular_vals[ib.uid()];
            }

            // Calculate the poloidal derivative
            ddc::discrete_space<PolarBSplines>().eval_deriv_p(singular_vals, vals, coord);
            for (auto ib : singular_domain) {
                singular_basis_vals_and_derivs(ib, ir, ip).poloidal_derivative
                        = singular_vals[ib.uid()];
            }
        });

        // Find the integral volume associated with each point used in the quadrature scheme
        QuadratureDomainRP all_quad_points(quadrature_domain_r, quadrature_domain_p);
        ddc::for_each(all_quad_points, [&](QuadratureMeshRP const irp) {
            QuadratureMeshR const ir = ddc::select<QDimRMesh>(irp);
            QuadratureMeshP const ip = ddc::select<QDimPMesh>(irp);
            ddc::Coordinate<DimR, DimP> coord(get_coordinate(ir), get_coordinate(ip));
            int_volume(ir, ip) = abs(mapping.jacobian(coord)) * weights_r(ir) * weights_p(ip);
        });

        SplineEvaluator2D<BSplinesR, BSplinesP> spline_evaluator(
                g_null_boundary_2d<BSplinesR, BSplinesP>,
                g_null_boundary_2d<BSplinesR, BSplinesP>,
                g_null_boundary_2d<BSplinesR, BSplinesP>,
                g_null_boundary_2d<BSplinesR, BSplinesP>);

        constexpr int n_elements_singular
                = PolarBSplines::n_singular_basis() * PolarBSplines::n_singular_basis();
        const int n_elements_overlap
                = 2 * (PolarBSplines::n_singular_basis() * BSplinesR::degree() * nbasis_p);
        const int n_stencil_p = nbasis_p * min(int(1 + 2 * BSplinesP::degree()), nbasis_p);
        const int n_stencil_r = nbasis_r * (1 + 2 * BSplinesR::degree())
                                - (1 + BSplinesR::degree()) * BSplinesR::degree();
        const int n_elements_stencil = n_stencil_r * n_stencil_p;

        // Matrix size is equal to the number Polar bspline
        const int n_matrix_size = ddc::discrete_space<PolarBSplines>().nbasis() - nbasis_p;
        Eigen::SparseMatrix<double> matrix(n_matrix_size, n_matrix_size);
        const int n_matrix_elements = n_elements_singular + n_elements_overlap + n_elements_stencil;
        std::vector<MatrixElement> matrix_elements(n_matrix_elements);
        int matrix_idx(0);
        // Calculate the matrix elements corresponding to the bsplines which cover the singular point
        ddc::for_each(singular_domain, [&](IDimPolarBspl const idx_test) {
            ddc::for_each(singular_domain, [&](IDimPolarBspl const idx_trial) {
                // Calculate the weak integral
                matrix_elements[matrix_idx++] = MatrixElement(
                        idx_test.uid(),
                        idx_trial.uid(),
                        ddc::transform_reduce(
                                quadrature_domain_singular,
                                0.0,
                                ddc::reducer::sum<double>(),
                                [&](QuadratureMeshRP const quad_idx) {
                                    QuadratureMeshR const ir = ddc::select<QDimRMesh>(quad_idx);
                                    QuadratureMeshP const ip = ddc::select<QDimPMesh>(quad_idx);
                                    std::optional<std::array<double, 3>> null(std::nullopt);
                                    return weak_integral_element(
                                            ir,
                                            ip,
                                            singular_basis_vals_and_derivs(idx_test, ir, ip),
                                            singular_basis_vals_and_derivs(idx_trial, ir, ip),
                                            coeff_alpha,
                                            coeff_beta,
                                            spline_evaluator,
                                            mapping);
                                }));
            });
        });
        assert(matrix_idx == n_elements_singular);

        // Create domains associated with the 2D splines
        ddc::DiscreteDomain<BSplinesR> central_radial_bspline_domain(
                radial_bsplines.take_first(ddc::DiscreteVector<BSplinesR> {BSplinesR::degree()}));

        BSpline2DDomain
                non_singular_domain_near_centre(central_radial_bspline_domain, polar_bsplines);

        const ddc::DiscreteDomain<RCellDim> r_cells_near_centre(
                ddc::DiscreteElement<RCellDim> {0},
                ddc::DiscreteVector<RCellDim> {n_overlap_cells});

        // Calculate the matrix elements where bspline products overlap the bsplines which cover the singular point
        ddc::for_each(singular_domain, [&](IDimPolarBspl const idx_test) {
            ddc::for_each(non_singular_domain_near_centre, [&](IDimBSpline2D const idx_trial) {
                const IDimPolarBspl polar_idx_trial(PolarBSplines::get_polar_index(idx_trial));
                const ddc::DiscreteElement<BSplinesR> r_idx_trial(
                        ddc::select<BSplinesR>(idx_trial));
                const ddc::DiscreteElement<BSplinesP> p_idx_trial(
                        ddc::select<BSplinesP>(idx_trial));

                // Find the domain covering the cells where both the test and trial functions are non-zero
                const ddc::DiscreteElement<RCellDim> first_overlap_element_r(
                        r_idx_trial.uid() < BSplinesR::degree()
                                ? 0
                                : r_idx_trial.uid() - BSplinesR::degree());
                const ddc::DiscreteElement<PCellDim> first_overlap_element_p(
                        pmod(p_idx_trial.uid() - BSplinesP::degree()));

                const ddc::DiscreteVector<RCellDim> n_overlap_r(
                        n_overlap_cells - first_overlap_element_r.uid());
                const ddc::DiscreteVector<PCellDim> n_overlap_p(BSplinesP::degree() + 1);

                const ddc::DiscreteDomain<RCellDim> r_cells(first_overlap_element_r, n_overlap_r);
                const ddc::DiscreteDomain<PCellDim> p_cells(first_overlap_element_p, n_overlap_p);
                const ddc::DiscreteDomain<RCellDim, PCellDim> non_zero_cells(r_cells, p_cells);

                if (n_overlap_r > 0) {
                    double element = 0.0;

                    ddc::for_each(non_zero_cells, [&](CellIndex const cell_idx) {
                        const int cell_idx_r(ddc::select<RCellDim>(cell_idx).uid());
                        const int cell_idx_p(pmod(ddc::select<PCellDim>(cell_idx).uid()));

                        const QuadratureDomainRP cell_quad_points(
                                get_quadrature_points_in_cell(cell_idx_r, cell_idx_p));
                        // Find the column where the non-zero data is stored
                        ddc::DiscreteElement<RBasisSubset> ib_trial_r(
                                r_idx_trial.uid() - cell_idx_r);
                        ddc::DiscreteElement<PBasisSubset> ib_trial_p(
                                pmod(p_idx_trial.uid() - cell_idx_p));
                        // Calculate the weak integral
                        element += ddc::transform_reduce(
                                cell_quad_points,
                                0.0,
                                ddc::reducer::sum<double>(),
                                [&](QuadratureMeshRP const quad_idx) {
                                    QuadratureMeshR const ir = ddc::select<QDimRMesh>(quad_idx);
                                    QuadratureMeshP const ip = ddc::select<QDimPMesh>(quad_idx);
                                    return weak_integral_element<Mapping>(
                                            ir,
                                            ip,
                                            singular_basis_vals_and_derivs(idx_test, ir, ip),
                                            r_basis_vals_and_derivs(ib_trial_r, ir),
                                            p_basis_vals_and_derivs(ib_trial_p, ip),
                                            coeff_alpha,
                                            coeff_beta,
                                            spline_evaluator,
                                            mapping);
                                });
                    });
                    matrix_elements[matrix_idx++]
                            = MatrixElement(idx_test.uid(), polar_idx_trial.uid(), element);
                    matrix_elements[matrix_idx++]
                            = MatrixElement(polar_idx_trial.uid(), idx_test.uid(), element);
                }
            });
        });
        assert(matrix_idx == n_elements_singular + n_elements_overlap);

        // Calculate the matrix elements following a stencil
        ddc::for_each(fem_non_singular_domain, [&](IDimPolarBspl const polar_idx_test) {
            const IDimBSpline2D idx_test(PolarBSplines::get_2d_index(polar_idx_test));
            const std::size_t r_idx_test(ddc::select<BSplinesR>(idx_test).uid());
            const std::size_t p_idx_test(ddc::select<BSplinesP>(idx_test).uid());

            // Calculate the index of the elements that are already filled
            ddc::DiscreteDomain<BSplinesP> remaining_p(
                    ddc::DiscreteElement<BSplinesP> {p_idx_test},
                    ddc::DiscreteVector<BSplinesP> {BSplinesP::degree() + 1});
            ddc::for_each(remaining_p, [&](auto const p_idx_trial) {
                IDimBSpline2D idx_trial(ddc::DiscreteElement<BSplinesR>(r_idx_test), p_idx_trial);
                IDimPolarBspl polar_idx_trial(PolarBSplines::get_polar_index(
                        IDimBSpline2D(r_idx_test, pmod(p_idx_trial.uid()))));
                double element = get_matrix_stencil_element(
                        idx_test,
                        idx_trial,
                        coeff_alpha,
                        coeff_beta,
                        spline_evaluator,
                        mapping);

                if (polar_idx_test.uid() == polar_idx_trial.uid()) {
                    matrix_elements[matrix_idx++]
                            = MatrixElement(polar_idx_test.uid(), polar_idx_trial.uid(), element);
                } else {
                    matrix_elements[matrix_idx++]
                            = MatrixElement(polar_idx_test.uid(), polar_idx_trial.uid(), element);
                    matrix_elements[matrix_idx++]
                            = MatrixElement(polar_idx_trial.uid(), polar_idx_test.uid(), element);
                }
            });
            ddc::DiscreteDomain<BSplinesR> remaining_r(
                    ddc::select<BSplinesR>(idx_test) + 1,
                    ddc::DiscreteVector<BSplinesR> {
                            min(BSplinesR::degree(),
                                ddc::discrete_space<BSplinesR>().nbasis() - 2 - r_idx_test)});
            ddc::DiscreteDomain<BSplinesP> relevant_p(
                    ddc::DiscreteElement<BSplinesP> {
                            p_idx_test + ddc::discrete_space<BSplinesP>().nbasis()
                            - BSplinesP::degree()},
                    ddc::DiscreteVector<BSplinesP> {2 * BSplinesP::degree() + 1});

            BSpline2DDomain trial_domain(remaining_r, relevant_p);

            ddc::for_each(trial_domain, [&](IDimBSpline2D const idx_trial) {
                const int r_idx_trial(ddc::select<BSplinesR>(idx_trial).uid());
                const int p_idx_trial(ddc::select<BSplinesP>(idx_trial).uid());
                IDimPolarBspl polar_idx_trial(PolarBSplines::get_polar_index(
                        IDimBSpline2D(r_idx_trial, pmod(p_idx_trial))));
                double element = get_matrix_stencil_element(
                        idx_test,
                        idx_trial,
                        coeff_alpha,
                        coeff_beta,
                        spline_evaluator,
                        mapping);
                if (polar_idx_test.uid() == polar_idx_trial.uid()) {
                    matrix_elements[matrix_idx++]
                            = MatrixElement(polar_idx_test.uid(), polar_idx_trial.uid(), element);
                } else {
                    matrix_elements[matrix_idx++]
                            = MatrixElement(polar_idx_test.uid(), polar_idx_trial.uid(), element);
                    matrix_elements[matrix_idx++]
                            = MatrixElement(polar_idx_trial.uid(), polar_idx_test.uid(), element);
                }
            });
        });
        matrix.setFromTriplets(matrix_elements.begin(), matrix_elements.end());
        assert(matrix_idx == n_elements_singular + n_elements_overlap + n_elements_stencil);
        m_matrix.compute(matrix);
    }

    template <class RHSFunction, class Domain>
    void operator()(
            RHSFunction const& rhs,
            ddc::ChunkSpan<ddc::Coordinate<DimR, DimP> const, Domain> const coords_eval,
            ddc::ChunkSpan<double, Domain> result) const
    {
        Eigen::VectorXd b(
                ddc::discrete_space<PolarBSplines>().nbasis()
                - ddc::discrete_space<BSplinesP>().nbasis());

        // Fill b
        ddc::for_each(PolarBSplines::singular_domain(), [&](IDimPolarBspl const idx) {
            b(idx.uid()) = ddc::transform_reduce(
                    quadrature_domain_singular,
                    0.0,
                    ddc::reducer::sum<double>(),
                    [&](QuadratureMeshRP const quad_idx) {
                        QuadratureMeshR const ir = ddc::select<QDimRMesh>(quad_idx);
                        QuadratureMeshP const ip = ddc::select<QDimPMesh>(quad_idx);
                        ddc::Coordinate<DimR, DimP> coord(get_coordinate(ir), get_coordinate(ip));
                        return rhs(coord) * singular_basis_vals_and_derivs(idx, ir, ip).value
                               * int_volume(ir, ip);
                    });
        });
        const std::size_t ncells_r = ddc::discrete_space<BSplinesR>().ncells();
        ddc::for_each(fem_non_singular_domain, [&](IDimPolarBspl const idx) {
            const IDimBSpline2D idx_2d(PolarBSplines::get_2d_index(idx));
            const std::size_t r_idx(ddc::select<BSplinesR>(idx_2d).uid());
            const std::size_t p_idx(ddc::select<BSplinesP>(idx_2d).uid());

            // Find the cells on which the bspline is non-zero
            int first_cell_r(r_idx - BSplinesR::degree());
            int first_cell_p(p_idx - BSplinesP::degree());
            std::size_t last_cell_r(r_idx + 1);
            if (first_cell_r < 0)
                first_cell_r = 0;
            if (last_cell_r > ncells_r)
                last_cell_r = ncells_r;
            ddc::DiscreteVector<RCellDim> const r_length(last_cell_r - first_cell_r);
            ddc::DiscreteVector<PCellDim> const p_length(BSplinesP::degree() + 1);


            ddc::DiscreteElement<RCellDim> const start_r(first_cell_r);
            ddc::DiscreteElement<PCellDim> const start_p(pmod(first_cell_p));
            const ddc::DiscreteDomain<RCellDim> r_cells(start_r, r_length);
            const ddc::DiscreteDomain<PCellDim> p_cells(start_p, p_length);
            const ddc::DiscreteDomain<RCellDim, PCellDim> non_zero_cells(r_cells, p_cells);
            assert(r_length * p_length > 0);
            double element = 0.0;
            ddc::for_each(non_zero_cells, [&](CellIndex const cell_idx) {
                const int cell_idx_r(ddc::select<RCellDim>(cell_idx).uid());
                const int cell_idx_p(pmod(ddc::select<PCellDim>(cell_idx).uid()));

                const QuadratureDomainRP cell_quad_points(
                        get_quadrature_points_in_cell(cell_idx_r, cell_idx_p));

                // Find the column where the non-zero data is stored
                ddc::DiscreteElement<RBasisSubset> ib_r(r_idx - cell_idx_r);
                ddc::DiscreteElement<PBasisSubset> ib_p(pmod(p_idx - cell_idx_p));

                // Calculate the weak integral
                element += ddc::transform_reduce(
                        cell_quad_points,
                        0.0,
                        ddc::reducer::sum<double>(),
                        [&](QuadratureMeshRP const quad_idx) {
                            QuadratureMeshR const ir = ddc::select<QDimRMesh>(quad_idx);
                            QuadratureMeshP const ip = ddc::select<QDimPMesh>(quad_idx);
                            ddc::Coordinate<DimR, DimP>
                                    coord(get_coordinate(ir), get_coordinate(ip));
                            double rb = r_basis_vals_and_derivs(ib_r, ir).value;
                            double pb = p_basis_vals_and_derivs(ib_p, ip).value;
                            return rhs(coord) * rb * pb * int_volume(ir, ip);
                        });
            });
            b(idx.uid()) = element;
        });

        // Solve the matrix equation
        Eigen::VectorXd x = m_matrix.solve(b);

        ddc::DiscreteDomain<BSplinesR, BSplinesP> non_singular_2d_domain(
                radial_bsplines.remove_last(ddc::DiscreteVector<BSplinesR> {1}),
                polar_bsplines);
        ddc::DiscreteDomain<BSplinesR, BSplinesP> dirichlet_boundary_domain(
                radial_bsplines.take_last(ddc::DiscreteVector<BSplinesR> {1}),
                polar_bsplines);
        ddc::DiscreteDomain<BSplinesP> polar_domain(ddc::discrete_space<BSplinesP>().full_domain());
        SplinePolar
                spline(PolarBSplines::singular_domain(),
                       ddc::DiscreteDomain<BSplinesR, BSplinesP>(radial_bsplines, polar_domain));

        // Fill the spline
        ddc::for_each(PolarBSplines::singular_domain(), [&](IDimPolarBspl const idx) {
            spline.singular_spline_coef(idx) = x(idx.uid());
        });
        ddc::for_each(fem_non_singular_domain, [&](IDimPolarBspl const idx) {
            const IDimBSpline2D idx_2d(PolarBSplines::get_2d_index(idx));
            spline.spline_coef(idx_2d) = x(idx.uid());
        });
        ddc::for_each(dirichlet_boundary_domain, [&](IDimBSpline2D const idx) {
            spline.spline_coef(idx) = 0.0;
        });

        // Copy the periodic elements
        ddc::DiscreteDomain<BSplinesR, BSplinesP> copy_domain(
                radial_bsplines,
                polar_domain.remove_first(
                        ddc::DiscreteVector<BSplinesP>(ddc::discrete_space<BSplinesP>().nbasis())));
        ddc::for_each(copy_domain, [&](IDimBSpline2D const idx_2d) {
            spline.spline_coef(ddc::select<BSplinesR>(idx_2d), ddc::select<BSplinesP>(idx_2d))
                    = spline.spline_coef(
                            ddc::select<BSplinesR>(idx_2d),
                            ddc::select<BSplinesP>(idx_2d)
                                    - ddc::discrete_space<BSplinesP>().nbasis());
        });

        m_polar_spline_evaluator(result, coords_eval, spline);
    }

private:
    static QuadratureDomainRP get_quadrature_points_in_cell(int cell_idx_r, int cell_idx_p)
    {
        const QuadratureMeshR first_quad_point_r(cell_idx_r * n_gauss_legendre_r);
        const QuadratureMeshP first_quad_point_p(cell_idx_p * n_gauss_legendre_p);
        constexpr QuadratureLengthR n_GL_r(n_gauss_legendre_r);
        constexpr QuadratureLengthP n_GL_p(n_gauss_legendre_p);
        const QuadratureDomainR quad_points_r(first_quad_point_r, n_GL_r);
        const QuadratureDomainP quad_points_p(first_quad_point_p, n_GL_p);
        return QuadratureDomainRP(quad_points_r, quad_points_p);
    }

    template <class Mapping>
    double weak_integral_element(
            QuadratureMeshR ir,
            QuadratureMeshP ip,
            EvalDeriv2DType const& test_bspline_val_and_deriv,
            EvalDeriv2DType const& trial_bspline_val_and_deriv,
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_alpha,
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_beta,
            SplineEvaluator2D<BSplinesR, BSplinesP> const& evaluator,
            Mapping const& mapping)
    {
        return templated_weak_integral_element(
                ir,
                ip,
                test_bspline_val_and_deriv,
                trial_bspline_val_and_deriv,
                test_bspline_val_and_deriv,
                trial_bspline_val_and_deriv,
                coeff_alpha,
                coeff_beta,
                evaluator,
                mapping);
    }

    template <class Mapping>
    double weak_integral_element(
            QuadratureMeshR ir,
            QuadratureMeshP ip,
            EvalDeriv2DType const& test_bspline_val_and_deriv,
            EvalDeriv1DType const& trial_bspline_val_and_deriv_r,
            EvalDeriv1DType const& trial_bspline_val_and_deriv_p,
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_alpha,
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_beta,
            SplineEvaluator2D<BSplinesR, BSplinesP> const& evaluator,
            Mapping const& mapping)
    {
        return templated_weak_integral_element(
                ir,
                ip,
                test_bspline_val_and_deriv,
                trial_bspline_val_and_deriv_r,
                test_bspline_val_and_deriv,
                trial_bspline_val_and_deriv_p,
                coeff_alpha,
                coeff_beta,
                evaluator,
                mapping);
    }

    template <class Mapping>
    double weak_integral_element(
            QuadratureMeshR ir,
            QuadratureMeshP ip,
            EvalDeriv1DType const& test_bspline_val_and_deriv_r,
            EvalDeriv2DType const& trial_bspline_val_and_deriv,
            EvalDeriv1DType const& test_bspline_val_and_deriv_p,
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_alpha,
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_beta,
            SplineEvaluator2D<BSplinesR, BSplinesP> const& evaluator,
            Mapping const& mapping)
    {
        return templated_weak_integral_element(
                ir,
                ip,
                test_bspline_val_and_deriv_r,
                trial_bspline_val_and_deriv,
                test_bspline_val_and_deriv_p,
                trial_bspline_val_and_deriv,
                coeff_alpha,
                coeff_beta,
                evaluator,
                mapping);
    }

    template <class Mapping>
    double weak_integral_element(
            QuadratureMeshR ir,
            QuadratureMeshP ip,
            EvalDeriv1DType const& test_bspline_val_and_deriv_r,
            EvalDeriv1DType const& trial_bspline_val_and_deriv_r,
            EvalDeriv1DType const& test_bspline_val_and_deriv_p,
            EvalDeriv1DType const& trial_bspline_val_and_deriv_p,
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_alpha,
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_beta,
            SplineEvaluator2D<BSplinesR, BSplinesP> const& evaluator,
            Mapping const& mapping)
    {
        return templated_weak_integral_element(
                ir,
                ip,
                test_bspline_val_and_deriv_r,
                trial_bspline_val_and_deriv_r,
                test_bspline_val_and_deriv_p,
                trial_bspline_val_and_deriv_p,
                coeff_alpha,
                coeff_beta,
                evaluator,
                mapping);
    }

    inline void get_value_and_gradient(
            double& value,
            std::array<double, 2>& gradient,
            EvalDeriv1DType const& r_basis,
            EvalDeriv1DType const& p_basis) const
    {
        value = r_basis.value * p_basis.value;
        gradient = {r_basis.derivative * p_basis.value, r_basis.value * p_basis.derivative};
    }

    inline void get_value_and_gradient(
            double& value,
            std::array<double, 2>& gradient,
            EvalDeriv2DType const& basis,
            EvalDeriv2DType const&) const // Last argument is duplicate
    {
        value = basis.value;
        gradient = {basis.radial_derivative, basis.poloidal_derivative};
    }

    template <class Mapping, class TestValDerivType, class TrialValDerivType>
    double templated_weak_integral_element(
            QuadratureMeshR ir,
            QuadratureMeshP ip,
            TestValDerivType const& test_bspline_val_and_deriv,
            TrialValDerivType const& trial_bspline_val_and_deriv,
            TestValDerivType const& test_bspline_val_and_deriv_p,
            TrialValDerivType const& trial_bspline_val_and_deriv_p,
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_alpha,
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_beta,
            SplineEvaluator2D<BSplinesR, BSplinesP> const& spline_evaluator,
            Mapping const& mapping)
    {
        static_assert(
                std::is_same_v<
                        TestValDerivType,
                        EvalDeriv1DType> || std::is_same_v<TestValDerivType, EvalDeriv2DType>);
        static_assert(
                std::is_same_v<
                        TrialValDerivType,
                        EvalDeriv1DType> || std::is_same_v<TrialValDerivType, EvalDeriv2DType>);

        // Calculate coefficients at quadrature point
        ddc::Coordinate<DimR, DimP> coord(get_coordinate(ir), get_coordinate(ip));
        const double alpha = spline_evaluator(coord, coeff_alpha);
        const double beta = spline_evaluator(coord, coeff_beta);

        // Define the value and gradient of the test and trial basis functions
        double basis_val_test_space;
        double basis_val_trial_space;
        std::array<double, 2> basis_gradient_test_space;
        std::array<double, 2> basis_gradient_trial_space;
        get_value_and_gradient(
                basis_val_test_space,
                basis_gradient_test_space,
                test_bspline_val_and_deriv,
                test_bspline_val_and_deriv_p);
        get_value_and_gradient(
                basis_val_trial_space,
                basis_gradient_trial_space,
                trial_bspline_val_and_deriv,
                trial_bspline_val_and_deriv_p);

        // Assemble the weak integral element
        return int_volume(ir, ip)
               * (alpha
                          * dot_product(
                                  basis_gradient_test_space,
                                  mapping.to_covariant(basis_gradient_trial_space, coord))
                  + beta * basis_val_test_space * basis_val_trial_space);
    }

    template <class Mapping>
    double get_matrix_stencil_element(
            IDimBSpline2D idx_test,
            IDimBSpline2D idx_trial,
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_alpha,
            ddc::ChunkSpan<double const, ddc::DiscreteDomain<BSplinesR, BSplinesP>> coeff_beta,
            SplineEvaluator2D<BSplinesR, BSplinesP> const& evaluator,
            Mapping const& mapping)
    {
        // 0 <= r_idx_test < 8
        // 0 <= r_idx_trial < 8
        // r_idx_test < r_idx_trial
        const int r_idx_test(ddc::select<BSplinesR>(idx_test).uid());
        const int r_idx_trial(ddc::select<BSplinesR>(idx_trial).uid());
        // 0 <= p_idx_test < 8
        // 0 <= p_idx_trial < 8
        int p_idx_test(pmod(ddc::select<BSplinesP>(idx_test).uid()));
        int p_idx_trial(pmod(ddc::select<BSplinesP>(idx_trial).uid()));

        const std::size_t ncells_r = ddc::discrete_space<BSplinesR>().ncells();
        const std::size_t nbasis_p = ddc::discrete_space<BSplinesP>().nbasis();

        // 0<= r_offset <= degree_r
        // -degree_p <= p_offset <= degree_p
        const int r_offset = r_idx_trial - r_idx_test;
        int p_offset = pmod(p_idx_trial - p_idx_test);
        if (p_offset >= int(nbasis_p - BSplinesP::degree())) {
            p_offset -= nbasis_p;
        }
        assert(r_offset >= 0);
        assert(r_offset <= int(BSplinesR::degree()));
        assert(p_offset >= -int(BSplinesP::degree()));
        assert(p_offset <= int(BSplinesP::degree()));

        // Find the domain covering the cells where both the test and trial functions are non-zero
        int n_overlap_stencil_r(BSplinesR::degree() + 1 - r_offset);
        int first_overlap_r(r_idx_trial - BSplinesR::degree());

        int first_overlap_p;
        int n_overlap_stencil_p;
        if (p_offset > 0) {
            n_overlap_stencil_p = BSplinesP::degree() + 1 - p_offset;
            first_overlap_p = pmod(p_idx_trial - BSplinesP::degree());
        } else {
            n_overlap_stencil_p = BSplinesP::degree() + 1 + p_offset;
            first_overlap_p = pmod(p_idx_test - BSplinesP::degree());
        }

        if (first_overlap_r < 0) {
            const int n_compact = first_overlap_r;
            first_overlap_r = 0;
            n_overlap_stencil_r += n_compact;
        }

        const int n_to_edge_r(ncells_r - first_overlap_r);

        const ddc::DiscreteVector<RCellDim> n_overlap_r(min(n_overlap_stencil_r, n_to_edge_r));
        const ddc::DiscreteVector<PCellDim> n_overlap_p(n_overlap_stencil_p);

        const ddc::DiscreteElement<RCellDim> first_overlap_element_r(first_overlap_r);
        const ddc::DiscreteElement<PCellDim> first_overlap_element_p(first_overlap_p);

        const ddc::DiscreteDomain<RCellDim> r_cells(first_overlap_element_r, n_overlap_r);
        const ddc::DiscreteDomain<PCellDim> p_cells(first_overlap_element_p, n_overlap_p);
        const ddc::DiscreteDomain<RCellDim, PCellDim> non_zero_cells(r_cells, p_cells);

        assert(n_overlap_r * n_overlap_p > 0);
        return ddc::transform_reduce(
                non_zero_cells,
                0.0,
                ddc::reducer::sum<double>(),
                [&](CellIndex const cell_idx) {
                    const int cell_idx_r(ddc::select<RCellDim>(cell_idx).uid());
                    const int cell_idx_p(pmod(ddc::select<PCellDim>(cell_idx).uid()));

                    const QuadratureDomainRP cell_quad_points(
                            get_quadrature_points_in_cell(cell_idx_r, cell_idx_p));

                    int ib_test_p_idx = p_idx_test - cell_idx_p;
                    int ib_trial_p_idx = p_idx_trial - cell_idx_p;

                    // Find the column where the non-zero data is stored
                    ddc::DiscreteElement<RBasisSubset> ib_test_r(r_idx_test - cell_idx_r);
                    ddc::DiscreteElement<PBasisSubset> ib_test_p(pmod(ib_test_p_idx));
                    ddc::DiscreteElement<RBasisSubset> ib_trial_r(r_idx_trial - cell_idx_r);
                    ddc::DiscreteElement<PBasisSubset> ib_trial_p(pmod(ib_trial_p_idx));

                    assert(ib_test_r.uid() < BSplinesR::degree() + 1);
                    assert(ib_test_p.uid() < BSplinesP::degree() + 1);
                    assert(ib_trial_r.uid() < BSplinesR::degree() + 1);
                    assert(ib_trial_p.uid() < BSplinesP::degree() + 1);

                    // Calculate the weak integral
                    return ddc::transform_reduce(
                            cell_quad_points,
                            0.0,
                            ddc::reducer::sum<double>(),
                            [&](QuadratureMeshRP const quad_idx) {
                                QuadratureMeshR const r_idx = ddc::select<QDimRMesh>(quad_idx);
                                QuadratureMeshP const p_idx = ddc::select<QDimPMesh>(quad_idx);
                                return weak_integral_element(
                                        r_idx,
                                        p_idx,
                                        r_basis_vals_and_derivs(ib_test_r, r_idx),
                                        r_basis_vals_and_derivs(ib_trial_r, r_idx),
                                        p_basis_vals_and_derivs(ib_test_p, p_idx),
                                        p_basis_vals_and_derivs(ib_trial_p, p_idx),
                                        coeff_alpha,
                                        coeff_beta,
                                        evaluator,
                                        mapping);
                            });
                });
    }

    template <class... QueryDDims>
    static ddc::Coordinate<typename QueryDDims::continuous_dimension_type::tag...> get_coordinate(
            ddc::DiscreteElement<QueryDDims...> mesh_idx)
    {
        ddc::Coordinate<typename QueryDDims::continuous_dimension_type...> coord(
                ddc::coordinate(ddc::select<QueryDDims>(mesh_idx))...);
        return ddc::Coordinate<typename QueryDDims::continuous_dimension_type::tag...>(
                ddc::get<typename QueryDDims::continuous_dimension_type>(coord)...);
    }

    static int pmod(int idx_p)
    {
        int ncells_p = ddc::discrete_space<BSplinesP>().ncells();
        while (idx_p < 0)
            idx_p += ncells_p;
        while (idx_p >= ncells_p)
            idx_p -= ncells_p;
        return idx_p;
    }
};
