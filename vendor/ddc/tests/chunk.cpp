// SPDX-License-Identifier: MIT
#include <ddc/ddc.hpp>

#include <gtest/gtest.h>

namespace {

using DElem0D = ddc::DiscreteElement<>;
using DVect0D = ddc::DiscreteVector<>;
using DDom0D = ddc::DiscreteDomain<>;

template <class Datatype>
using Chunk0D = ddc::Chunk<Datatype, DDom0D>;
template <class Datatype>
using ChunkSpan0D = ddc::ChunkSpan<Datatype, DDom0D>;


struct DDimX;
using DElemX = ddc::DiscreteElement<DDimX>;
using DVectX = ddc::DiscreteVector<DDimX>;
using DDomX = ddc::DiscreteDomain<DDimX>;

template <class Datatype>
using ChunkX = ddc::Chunk<Datatype, DDomX>;


struct DDimY;
using DElemY = ddc::DiscreteElement<DDimY>;
using DVectY = ddc::DiscreteVector<DDimY>;
using DDomY = ddc::DiscreteDomain<DDimY>;

template <class Datatype>
using ChunkY = ddc::Chunk<Datatype, DDomY>;


struct DDimZ;
using DElemZ = ddc::DiscreteElement<DDimZ>;
using DVectZ = ddc::DiscreteVector<DDimZ>;
using DDomZ = ddc::DiscreteDomain<DDimZ>;


using DElemXY = ddc::DiscreteElement<DDimX, DDimY>;
using DVectXY = ddc::DiscreteVector<DDimX, DDimY>;
using DDomXY = ddc::DiscreteDomain<DDimX, DDimY>;

template <class Datatype>
using ChunkXY = ddc::Chunk<Datatype, DDomXY>;


using DElemYX = ddc::DiscreteElement<DDimY, DDimX>;
using DVectYX = ddc::DiscreteVector<DDimY, DDimX>;
using DDomYX = ddc::DiscreteDomain<DDimY, DDimX>;

template <class Datatype>
using ChunkYX = ddc::Chunk<Datatype, DDomYX>;


static DElem0D constexpr lbound_0d {};
static DVect0D constexpr nelems_0d {};
static DDom0D constexpr dom_0d(lbound_0d, nelems_0d);

static DElemX constexpr lbound_x(50);
static DVectX constexpr nelems_x(3);
static DDomX constexpr dom_x(lbound_x, nelems_x);

static DElemY constexpr lbound_y(4);
static DVectY constexpr nelems_y(12);

static DElemXY constexpr lbound_x_y {lbound_x, lbound_y};
static DVectXY constexpr nelems_x_y(nelems_x, nelems_y);
static DDomXY constexpr dom_x_y(lbound_x_y, nelems_x_y);

} // namespace

// Member types of Chunk 1D \{

TEST(Chunk0DTest, LayoutType)
{
    EXPECT_TRUE((std::is_same_v<Chunk0D<double>::layout_type, std::experimental::layout_right>));
}

TEST(Chunk1DTest, LayoutType)
{
    ChunkX<double> chunk(dom_x);

    EXPECT_TRUE((std::is_same_v<
                 std::decay_t<decltype(chunk)>::layout_type,
                 std::experimental::layout_right>));
}

// TODO: many missing types

// \}
// Functions implemented in Chunk 1D (and free functions specific to it) \{

TEST(Chunk0DTest, ChunkSpanConversionConstructor)
{
    double constexpr factor = 1.391;
    Chunk0D<double> chunk(dom_0d);
    chunk() = factor;

    Chunk0D<double> chunk2(chunk.span_view());
    EXPECT_EQ(chunk2.domain(), dom_0d);
    EXPECT_DOUBLE_EQ(factor, chunk2());
}

TEST(Chunk1DTest, ChunkSpanConversionConstructor)
{
    double constexpr factor = 1.391;
    ChunkX<double> chunk(dom_x);
    for (auto&& ix : chunk.domain()) {
        chunk(ix) = factor * ix.uid();
    }

    ChunkX<double> chunk2(chunk.span_view());
    EXPECT_EQ(chunk2.domain(), dom_x);
    for (auto&& ix : chunk2.domain()) {
        EXPECT_DOUBLE_EQ(factor * ix.uid(), chunk2(ix));
    }
}

TEST(Chunk0DTest, MoveConstructor)
{
    double constexpr factor = 1.391;
    Chunk0D<double> chunk(dom_0d);
    chunk() = factor;

    Chunk0D<double> chunk2(std::move(chunk));
    EXPECT_EQ(chunk2.domain(), dom_0d);
    EXPECT_DOUBLE_EQ(factor, chunk2());
}

TEST(Chunk1DTest, MoveConstructor)
{
    double constexpr factor = 1.391;
    ChunkX<double> chunk(dom_x);
    for (auto&& ix : chunk.domain()) {
        chunk(ix) = factor * ix.uid();
    }

    ChunkX<double> chunk2(std::move(chunk));
    EXPECT_EQ(chunk2.domain(), dom_x);
    for (auto&& ix : chunk2.domain()) {
        // we expect exact equality, not EXPECT_DOUBLE_EQ: this is the same ref twice
        EXPECT_EQ(factor * ix.uid(), chunk2(ix));
    }
}

TEST(Chunk0DTest, MoveAssignment)
{
    double constexpr factor = 1.976;
    Chunk0D<double> chunk(dom_0d);
    chunk() = factor;

    Chunk0D<double> chunk2(DDom0D(lbound_0d, DVect0D()));
    chunk2 = std::move(chunk);
    EXPECT_EQ(chunk2.domain(), dom_0d);
    EXPECT_DOUBLE_EQ(factor, chunk2());
}

TEST(Chunk1DTest, MoveAssignment)
{
    double constexpr factor = 1.976;
    ChunkX<double> chunk(dom_x);
    for (auto&& ix : chunk.domain()) {
        chunk(ix) = factor * ix.uid();
    }

    ChunkX<double> chunk2(DDomX(lbound_x, DVectX(0)));
    chunk2 = std::move(chunk);
    EXPECT_EQ(chunk2.domain(), dom_x);
    for (auto&& ix : chunk2.domain()) {
        // we expect exact equality, not EXPECT_DOUBLE_EQ: this is the same ref twice
        EXPECT_EQ(factor * ix.uid(), chunk2(ix));
    }
}

TEST(Chunk0DTest, Swap)
{
    double constexpr factor = 1.976;
    Chunk0D<double> chunk(dom_0d);
    chunk() = factor;

    DDom0D empty_domain(lbound_0d, DVect0D());
    Chunk0D<double> chunk2(empty_domain);

    std::swap(chunk2, chunk);
    EXPECT_EQ(chunk.domain(), empty_domain);
    EXPECT_EQ(chunk2.domain(), dom_0d);
    EXPECT_DOUBLE_EQ(factor, chunk2());
}

TEST(Chunk1DTest, Swap)
{
    double constexpr factor = 1.976;
    ChunkX<double> chunk(dom_x);
    for (auto&& ix : chunk.domain()) {
        chunk(ix) = factor * ix.uid();
    }

    DDomX empty_domain(lbound_x, DVectX(0));
    ChunkX<double> chunk2(empty_domain);

    std::swap(chunk2, chunk);
    EXPECT_EQ(chunk.domain(), empty_domain);
    EXPECT_EQ(chunk2.domain(), dom_x);
    for (auto&& ix : chunk2.domain()) {
        // we expect exact equality, not EXPECT_DOUBLE_EQ: this is the same ref twice
        EXPECT_EQ(factor * ix.uid(), chunk2(ix));
    }
}

// no dim subset access in 1D

TEST(Chunk1DTest, AccessConst)
{
    double constexpr factor = 1.012;
    ChunkX<double> chunk(dom_x);
    ChunkX<double> const& chunk_cref = chunk;
    for (auto&& ix : chunk.domain()) {
        chunk(ix) = factor * ix.uid();
        // we expect exact equality, not EXPECT_DOUBLE_EQ: this is the same ref twice
        EXPECT_EQ(chunk_cref(ix), factor * ix.uid());
    }
}

TEST(Chunk1DTest, Access)
{
    double constexpr factor = 1.012;
    ChunkX<double> chunk(dom_x);
    for (auto&& ix : chunk.domain()) {
        chunk(ix) = factor * ix.uid();
        // we expect exact equality, not EXPECT_DOUBLE_EQ: this is the same ref twice
        EXPECT_EQ(chunk(ix), factor * ix.uid());
    }
}

TEST(Chunk1DTest, SpanCview)
{
    double constexpr factor = 1.567;
    ChunkX<double> chunk(dom_x);
    for (auto&& ix : chunk.domain()) {
        chunk(ix) = factor * ix.uid();
        // we expect exact equality, not EXPECT_DOUBLE_EQ: this is the same ref twice
        EXPECT_EQ(chunk.span_cview()(ix), factor * ix.uid());
    }
}

TEST(Chunk1DTest, ViewConst)
{
    double constexpr factor = 1.802;
    ChunkX<double> chunk(dom_x);
    ChunkX<double> const& chunk_cref = chunk;
    for (auto&& ix : chunk.domain()) {
        chunk(ix) = factor * ix.uid();
        // we expect exact equality, not EXPECT_DOUBLE_EQ: this is the same ref twice
        EXPECT_EQ(chunk_cref.span_view()(ix), factor * ix.uid());
    }
}

TEST(Chunk1DTest, View)
{
    double constexpr factor = 1.259;
    ChunkX<double> chunk(dom_x);
    for (auto&& ix : chunk.domain()) {
        chunk.span_view()(ix) = factor * ix.uid();
        // we expect exact equality, not EXPECT_DOUBLE_EQ: this is the same ref twice
        EXPECT_EQ(chunk(ix), factor * ix.uid());
    }
}

// \}
// Functions inherited from ChunkCommon (and free functions implemented for it) \{

// constructors are hidden

// assignment operators are hidden

// no slicing (operator[]) in 1D

// access (operator()) operators are hidden

// TODO: accessor

TEST(Chunk1DTest, Rank)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_EQ(ChunkX<double>::rank(), 1);
}

TEST(Chunk1DTest, RankDynamic)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_EQ(ChunkX<double>::rank_dynamic(), 1);
}

TEST(Chunk1DTest, Extents)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_EQ(chunk.extents(), nelems_x);
}

TEST(Chunk1DTest, Extent)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_EQ(chunk.extent<DDimX>(), nelems_x.value());
}

TEST(Chunk1DTest, Size)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_EQ(chunk.size(), nelems_x.value());
}

TEST(Chunk1DTest, IsAlwaysUnique)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_TRUE(chunk.is_always_unique());
}

TEST(Chunk1DTest, IsAlwaysContiguous)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_TRUE(chunk.is_always_exhaustive());
}

TEST(Chunk1DTest, is_always_strided)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_TRUE(chunk.is_always_strided());
}

// TODO: mapping

TEST(Chunk1DTest, IsUnique)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_TRUE(chunk.is_unique());
}

TEST(Chunk1DTest, IsContiguous)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_TRUE(chunk.is_exhaustive());
}

TEST(Chunk1DTest, IsStrided)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_TRUE(chunk.is_strided());
}

// TODO: stride

// swap is hidden

TEST(Chunk1DTest, Domain)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_EQ(dom_x, chunk.domain());
}

TEST(Chunk1DTest, DomainX)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_EQ(dom_x, chunk.domain<DDimX>());
}

// TODO: data_handle()

// TODO: internal_mdspan

// TODO: allocation_mdspan

TEST(Chunk1DTest, GetDomainX)
{
    ChunkX<double> chunk(dom_x);
    EXPECT_EQ(dom_x, ddc::get_domain<DDimX>(chunk));
}

TEST(Chunk1DTest, Deepcopy)
{
    ChunkX<double> chunk(dom_x);
    for (auto&& ix : chunk.domain()) {
        chunk(ix) = 1.001 * ix.uid();
    }
    ChunkX<double> chunk2(chunk.domain());
    ddc::deepcopy(chunk2, chunk);
    for (auto&& ix : chunk.domain()) {
        // we expect exact equality, not EXPECT_DOUBLE_EQ: these are copy
        EXPECT_EQ(chunk2(ix), chunk(ix));
    }
}

// \}
// Functions implemented in Chunk 2D (and free functions specific to it) \{

// TODO: lots to do still!

TEST(Chunk2DTest, Access)
{
    ChunkXY<double> chunk(dom_x_y);
    for (auto&& ix : chunk.domain<DDimX>()) {
        for (auto&& iy : chunk.domain<DDimY>()) {
            chunk(ix, iy) = 1.357 * ix.uid() + 1.159 * iy.uid();
            // we expect exact equality, not EXPECT_DOUBLE_EQ: this is the same ref twice
            EXPECT_EQ(chunk(ix, iy), chunk(ix, iy));
        }
    }
}

TEST(Chunk2DTest, AccessReordered)
{
    ChunkXY<double> chunk(dom_x_y);
    for (auto&& ix : chunk.domain<DDimX>()) {
        for (auto&& iy : chunk.domain<DDimY>()) {
            chunk(ix, iy) = 1.455 * ix.uid() + 1.522 * iy.uid();
            // we expect exact equality, not EXPECT_DOUBLE_EQ: this is the same ref twice
            EXPECT_EQ(chunk(iy, ix), chunk(ix, iy));
        }
    }
}

TEST(Chunk2DTest, Cview)
{
    ChunkXY<double> chunk(dom_x_y);
    for (auto&& ix : chunk.domain<DDimX>()) {
        for (auto&& iy : chunk.domain<DDimY>()) {
            chunk(ix, iy) = 1. * ix.uid() + .001 * iy.uid();
        }
    }
    auto cview = chunk.span_cview();
    for (auto&& ix : chunk.domain<DDimX>()) {
        for (auto&& iy : chunk.domain<DDimY>()) {
            // we expect complete equality, not EXPECT_DOUBLE_EQ: these are copy
            EXPECT_EQ(cview(ix, iy), chunk(ix, iy));
        }
    }
}

TEST(Chunk2DTest, SliceCoordX)
{
    DElemX constexpr slice_x_val = DElemX(lbound_x + 1);

    ChunkXY<double> chunk(dom_x_y);
    ChunkXY<double> const& chunk_cref = chunk;
    for (auto&& ix : chunk.domain<DDimX>()) {
        for (auto&& iy : chunk.domain<DDimY>()) {
            chunk(ix, iy) = 1. * ix.uid() + .001 * iy.uid();
        }
    }

    auto&& chunk_y = chunk_cref[slice_x_val];
    EXPECT_TRUE((std::is_same_v<
                 std::decay_t<decltype(chunk_y)>::layout_type,
                 std::experimental::layout_right>));
    EXPECT_EQ(chunk_y.extent<DDimY>(), chunk.extent<DDimY>());
    for (auto&& ix : chunk_cref.domain<DDimY>()) {
        // we expect complete equality, not EXPECT_DOUBLE_EQ: these are copy
        EXPECT_EQ(chunk_y(ix), chunk_cref(slice_x_val, ix));
    }
}

TEST(Chunk2DTest, SliceCoordY)
{
    DElemY constexpr slice_y_val = DElemY(lbound_y + 1);

    ChunkXY<double> chunk(dom_x_y);
    ChunkXY<double> const& chunk_cref = chunk;
    for (auto&& ix : chunk.domain<DDimX>()) {
        for (auto&& iy : chunk.domain<DDimY>()) {
            chunk(ix, iy) = 1. * ix.uid() + .001 * iy.uid();
        }
    }

    auto&& chunk_x = chunk_cref[slice_y_val];
    EXPECT_TRUE((std::is_same_v<
                 std::decay_t<decltype(chunk_x)>::layout_type,
                 std::experimental::layout_stride>));
    EXPECT_EQ(chunk_x.extent<DDimX>(), chunk.extent<DDimX>());
    for (auto&& ix : chunk_cref.domain<DDimX>()) {
        // we expect complete equality, not EXPECT_DOUBLE_EQ: these are copy
        EXPECT_EQ(chunk_x(ix), chunk_cref(ix, slice_y_val));
    }
}

TEST(Chunk2DTest, SliceDomainX)
{
    DDomX constexpr subdomain_x = DDomX(DElemX(lbound_x + 1), DVectX(nelems_x - 2));

    ChunkXY<double> chunk(dom_x_y);
    ChunkXY<double> const& chunk_cref = chunk;
    for (auto&& ix : chunk.domain<DDimX>()) {
        for (auto&& iy : chunk.domain<DDimY>()) {
            chunk(ix, iy) = 1. * ix.uid() + .001 * iy.uid();
        }
    }

    auto&& subchunk_x = chunk_cref[subdomain_x];
    EXPECT_TRUE((std::is_same_v<
                 std::decay_t<decltype(subchunk_x)>::layout_type,
                 std::experimental::layout_right>));

    EXPECT_EQ(subchunk_x.extent<DDimX>(), subdomain_x.size());
    EXPECT_EQ(subchunk_x.extent<DDimY>(), chunk.domain<DDimY>().size());
    for (auto&& ix : subchunk_x.domain<DDimX>()) {
        for (auto&& iy : subchunk_x.domain<DDimY>()) {
            // we expect complete equality, not EXPECT_DOUBLE_EQ: these are copy
            EXPECT_EQ(subchunk_x(ix, iy), chunk_cref(ix, iy));
        }
    }
}

TEST(Chunk2DTest, SliceDomainXTooearly)
{
    [[maybe_unused]] DDomX constexpr subdomain_x = DDomX(DElemX(lbound_x - 1), nelems_x);

    ChunkXY<double> chunk(dom_x_y);
#ifndef NDEBUG // The assertion is only checked if NDEBUG isn't defined
    // the error message is checked with clang & gcc only
    EXPECT_DEATH(
            chunk[subdomain_x],
            R"rgx([Aa]ssert.*uid<ODDims>\(m_element_begin\).*uid<ODDims>\(odomain\.m_element_begin\))rgx");
#endif
}

TEST(Chunk2DTest, SliceDomainXToolate)
{
    [[maybe_unused]] DDomX constexpr subdomain_x = DDomX(lbound_x, DVectX(nelems_x + 1));

    ChunkXY<double> chunk(dom_x_y);
#ifndef NDEBUG // The assertion is only checked if NDEBUG isn't defined
    // the error message is checked with clang & gcc only
    EXPECT_DEATH(
            chunk[subdomain_x],
            R"rgx([Aa]ssert.*uid<ODDims>\(m_element_end\).*uid<ODDims>\(odomain\.m_element_end\).*)rgx");
#endif
}

TEST(Chunk2DTest, SliceDomainY)
{
    DDomY constexpr subdomain_y = DDomY(DElemY(lbound_y + 1), DVectY(nelems_y - 2));

    ChunkXY<double> chunk(dom_x_y);
    ChunkXY<double> const& chunk_cref = chunk;
    for (auto&& ix : chunk.domain<DDimX>()) {
        for (auto&& iy : chunk.domain<DDimY>()) {
            chunk(ix, iy) = 1. * ix.uid() + .001 * iy.uid();
        }
    }
    auto&& subchunk_y = chunk_cref[subdomain_y];
    EXPECT_TRUE((std::is_same_v<
                 std::decay_t<decltype(subchunk_y)>::layout_type,
                 std::experimental::layout_stride>));

    EXPECT_EQ(subchunk_y.extent<DDimX>(), chunk.domain<DDimX>().size());
    EXPECT_EQ(subchunk_y.extent<DDimY>(), subdomain_y.size());
    for (auto&& ix : subchunk_y.domain<DDimX>()) {
        for (auto&& iy : subchunk_y.domain<DDimY>()) {
            // we expect complete equality, not EXPECT_DOUBLE_EQ: these are copy
            EXPECT_EQ(subchunk_y(ix, iy), chunk_cref(ix, iy));
        }
    }
}

TEST(Chunk2DTest, Deepcopy)
{
    ChunkXY<double> chunk(dom_x_y);
    for (auto&& ix : chunk.domain<DDimX>()) {
        for (auto&& iy : chunk.domain<DDimY>()) {
            chunk(ix, iy) = 1.739 * ix.uid() + 1.412 * iy.uid();
        }
    }
    ChunkXY<double> chunk2(chunk.domain());
    ddc::deepcopy(chunk2, chunk);
    for (auto&& ix : chunk.domain<DDimX>()) {
        for (auto&& iy : chunk.domain<DDimY>()) {
            // we expect complete equality, not EXPECT_DOUBLE_EQ: these are copy
            EXPECT_EQ(chunk2(ix, iy), chunk(ix, iy));
        }
    }
}

TEST(Chunk2DTest, DeepcopyReordered)
{
    ChunkXY<double> chunk(dom_x_y);
    for (auto&& ix : chunk.domain<DDimX>()) {
        for (auto&& iy : chunk.domain<DDimY>()) {
            chunk(ix, iy) = 1.739 * ix.uid() + 1.412 * iy.uid();
        }
    }
    ChunkYX<double> chunk2(ddc::select<DDimY, DDimX>(chunk.domain()));
    ddc::ChunkSpan<double, DDomXY, std::experimental::layout_left>
            chunk2_view(chunk2.data_handle(), chunk.domain());
    ddc::deepcopy(chunk2_view, chunk);
    for (auto&& ix : chunk.domain<DDimX>()) {
        for (auto&& iy : chunk.domain<DDimY>()) {
            // we expect complete equality, not EXPECT_DOUBLE_EQ: these are copy
            EXPECT_EQ(chunk2(ix, iy), chunk(ix, iy));
            EXPECT_EQ(chunk2(ix, iy), chunk(iy, ix));
        }
    }
}
