#include <iosfwd>
#include <memory>
#include <utility>

#include <gtest/gtest.h>

#include "mcoord.h"
#include "mdomain.h"
#include "rcoord.h"
#include "taggedvector.h"
#include "uniform_mesh.h"

class DimX;

using MeshX = UniformMesh<DimX>;
using MCoordX = MCoord<MeshX>;

using RCoordX = RCoord<DimX>;

class MDomainXTest : public ::testing::Test
{
protected:
    std::size_t npoints = 101;
    MeshX mesh_x = MeshX(0., 1., npoints);
    MDomain<MeshX> const dom = MDomain(mesh_x, npoints - 1);
};

TEST_F(MDomainXTest, Constructor)
{
    constexpr RCoordX origin(1);
    constexpr RCoordX unit_vec(3);
    constexpr MCoordX lbound(0);
    constexpr MCoordX ubound(10);
    constexpr RCoordX rmin = origin;
    constexpr RCoordX rmax = rmin + unit_vec * (double)(ubound - lbound);
    constexpr static MeshX mesh(origin, unit_vec);
    constexpr MDomain dom_a(mesh, ubound);
    constexpr MDomain dom_b(mesh, lbound, ubound);
    constexpr MDomain dom_d(dom_a);
    EXPECT_EQ(dom_a, dom_b);
    EXPECT_EQ(dom_a, dom_d);
    EXPECT_EQ(dom_a.rmin(), rmin);
    EXPECT_EQ(dom_a.rmax(), rmax);
    EXPECT_EQ(dom_a.front(), lbound);
    EXPECT_EQ(dom_a.back(), ubound);
}

TEST_F(MDomainXTest, ubound)
{
    EXPECT_EQ(dom.back().get<MeshX>(), 100ul);
    EXPECT_EQ(dom.back(), 100ul);
}

TEST_F(MDomainXTest, rmax)
{
    EXPECT_EQ(dom.mesh().to_real(get<MeshX>(dom.back())).get<DimX>(), 1.);
    EXPECT_EQ(dom.rmax().get<DimX>(), 1.);
    EXPECT_EQ(dom.rmax(), 1.);
}

TEST_F(MDomainXTest, RangeFor)
{
    std::size_t ii = 0;
    for (auto&& x : dom) {
        ASSERT_LE(0, x);
        EXPECT_EQ(x, ii);
        ASSERT_LT(x, npoints);
        ++ii;
    }
}
