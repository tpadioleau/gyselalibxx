#include <memory>

#include <ddc/ChunkSpan>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "geometry.hpp"
#include "iadvectionvx.hpp"
#include "iadvectionx.hpp"
#include "species_info.hpp"
#include "splitvlasovsolver.hpp"

class MockAdvectionX : public IAdvectionX
{
public:
    MOCK_METHOD(DSpanSpXVx, CallOp, (DSpanSpXVx fdistribu, double dt), (const));
    DSpanSpXVx operator()(DSpanSpXVx fdistribu, double dt) const override
    {
        return this->CallOp(fdistribu, dt);
    }
};

class MockAdvectionVx : public IAdvectionVx
{
public:
    MOCK_METHOD(DSpanSpXVx, CallOp, (DSpanSpXVx fdistribu, DViewX efield, double dt), (const));
    DSpanSpXVx operator()(DSpanSpXVx fdistribu, DViewX efield, double dt) const override
    {
        return this->CallOp(fdistribu, efield, dt);
    }
};

using namespace ::testing;

TEST(SplitVlasovSolver, ordering)
{
    IDimX mesh_x(CoordX(0.), CoordX(2.));
    IDimVx mesh_vx(CoordVx(0.), CoordVx(2.));
    IDimSp mesh_sp;
    IDomainSpXVx const dom(mesh_sp, mesh_x, mesh_vx, IndexSpXVx(0, 0, 0), IVectSpXVx(0, 0, 0));
    DFieldSpXVx const fdistribu(dom);
    DSpanSpXVx const fdistribu_s(fdistribu);
    DFieldX const efield(select<IDimX>(dom));
    double const dt = 0.;

    MockAdvectionX const advec_x;
    MockAdvectionVx const advec_vx;
    SplitVlasovSolver const solver(advec_x, advec_vx);

    {
        InSequence s;

        EXPECT_CALL(advec_x, CallOp).WillOnce(Return(fdistribu_s));
        EXPECT_CALL(advec_vx, CallOp).WillOnce(Return(fdistribu_s));
        EXPECT_CALL(advec_x, CallOp).WillOnce(Return(fdistribu_s));
    }

    solver(fdistribu_s, efield, dt);
}
