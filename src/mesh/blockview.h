#pragma once

#include <cassert>
#include <memory>
#include <type_traits>
#include <vector>

#include "mdomain.h"
#include "view.h"

template <class, class>
class Block;

template <class, class, bool = true>
class BlockView;

template <bool O_CONTIGUOUS, class OElementType, class... OTags>
static BlockView<MDomain<OTags...>, OElementType, O_CONTIGUOUS> make_view(
        RegularMesh<OTags...> const& mesh,
        ViewND<sizeof...(OTags), OElementType, O_CONTIGUOUS> const& raw_view);

template <class... Tags, class ElementType, bool CONTIGUOUS>
class BlockView<MDomain<Tags...>, ElementType, CONTIGUOUS>
{
public:
    /// ND memory view
    using RawView = ViewND<sizeof...(Tags), ElementType, CONTIGUOUS>;

    using MDomain_ = MDomain<Tags...>;

    using Mesh = typename MDomain_::Mesh;

    using MCoord_ = typename MDomain_::MCoord_;

    using extents_type = typename RawView::extents_type;

    using layout_type = typename RawView::layout_type;

    using accessor_type = typename RawView::accessor_type;

    using mapping_type = typename RawView::mapping_type;

    using element_type = typename RawView::element_type;

    using value_type = typename RawView::value_type;

    using index_type = typename RawView::index_type;

    using difference_type = typename RawView::difference_type;

    using pointer = typename RawView::pointer;

    using reference = typename RawView::reference;

    template <class, class, bool>
    friend class BlockView;

private:
    template <class QTag, class... CTags>
    static auto get_slicer_for(const MCoord<CTags...>& c)
    {
        if constexpr (has_tag_v<QTag, MCoord<CTags...>>) {
            return c.template get<QTag>();
        } else {
            return std::experimental::all;
        }
    }

    template <class... SliceSpecs>
    struct Slicer
    {
        template <class... OSliceSpecs>
        static inline constexpr auto slice(const BlockView& block, OSliceSpecs&&... slices)
        {
            auto view = subspan(block.raw_view(), std::forward<OSliceSpecs>(slices)...);
            auto mesh = submesh(block.mesh(), std::forward<OSliceSpecs>(slices)...);
            return make_view<::is_contiguous<decltype(view)>, ElementType>(mesh, view);
        }
    };

    template <class... STags>
    struct Slicer<MCoord<STags...>>
    {
        static inline constexpr auto slice(const BlockView& block, const MCoord<STags...>& slices)
        {
            return Slicer<MCoord<STags...>, MDomain_>::slice(block, std::move(slices));
        }
    };

    template <class... STags>
    struct Slicer<MCoord<STags...>, MDomain<>>
    {
        template <class... SliceSpecs>
        static inline constexpr auto slice(
                const BlockView& block,
                const MCoord<STags...>&,
                SliceSpecs&&... oslices)
        {
            return Slicer<SliceSpecs...>::slice(block, oslices...);
        }
    };

    template <class... STags, class OTag0, class... OTags>
    struct Slicer<MCoord<STags...>, MDomain<OTag0, OTags...>>
    {
        template <class... SliceSpecs>
        static inline constexpr auto slice(
                const BlockView& block,
                const MCoord<STags...>& slices,
                SliceSpecs&&... oslices)
        {
            return Slicer<MCoord<STags...>, MDomain<OTags...>>::
                    slice(block, slices, oslices..., get_slicer_for<OTag0>(slices));
        }
    };

    /// The raw view of the data
    RawView m_raw;

    /// The mesh on which this block is defined
    Mesh m_mesh;

public:
    /** Constructs a new BlockView by copy, yields a new view to the same data
     * @param other the BlockView to copy
     */
    inline constexpr BlockView(const BlockView& other) noexcept = default;

    /** Constructs a new BlockView by move
     * @param other the BlockView to move
     */
    inline constexpr BlockView(BlockView&& other) noexcept = default;

    /** Constructs a new BlockView from scratch
     * @param mesh the mesh that sustains the view
     * @param raw_view the raw view to the data
     */
    inline constexpr BlockView(const Mesh& mesh, RawView raw_view) : m_raw(raw_view), m_mesh(mesh)
    {
    }

    /** Copy-assigns a new value to this BlockView, yields a new view to the same data
     * @param other the BlockView to copy
     * @return *this
     */
    inline constexpr BlockView& operator=(const BlockView& other) noexcept = default;

    /** Move-assigns a new value to this BlockView
     * @param other the BlockView to move
     * @return *this
     */
    inline constexpr BlockView& operator=(BlockView&& other) noexcept = default;

    /** Slice out some dimensions
     * @param slices the coordinates to 
     */
    template <class SliceSpec>
    inline constexpr auto operator[](SliceSpec&& slice) const
    {
        return Slicer<std::remove_cv_t<std::remove_reference_t<SliceSpec>>>::
                slice(*this, std::forward<SliceSpec>(slice));
    }

    template <class... IndexType>
    inline constexpr reference operator()(IndexType&&... indices) const noexcept
    {
        return m_raw(std::forward<IndexType>(indices)...);
    }

    inline constexpr reference operator()(const MCoord_& indices) const noexcept
    {
        return m_raw(indices.array());
    }

    inline accessor_type accessor() const
    {
        return m_raw.accessor();
    }

    static inline constexpr int rank() noexcept
    {
        return extents_type::rank();
    }

    static inline constexpr int rank_dynamic() noexcept
    {
        return extents_type::rank_dynamic();
    }

    static inline constexpr index_type static_extent(size_t r) noexcept
    {
        return extents_type::static_extent(r);
    }

    inline constexpr extents_type extents() const noexcept
    {
        return m_raw.extents();
    }

    inline constexpr index_type extent(size_t dim) const noexcept
    {
        return m_raw.extent(dim);
    }

    inline constexpr index_type size() const noexcept
    {
        return m_raw.size();
    }

    inline constexpr index_type unique_size() const noexcept
    {
        return m_raw.unique_size();
    }

    static inline constexpr bool is_always_unique() noexcept
    {
        return mapping_type::is_always_unique();
    }

    static inline constexpr bool is_always_contiguous() noexcept
    {
        return mapping_type::is_always_contiguous();
    }

    static inline constexpr bool is_always_strided() noexcept
    {
        return mapping_type::is_always_strided();
    }

    inline constexpr mapping_type mapping() const noexcept
    {
        return m_raw.mapping();
    }

    inline constexpr bool is_unique() const noexcept
    {
        return m_raw.is_unique();
    }

    inline constexpr bool is_contiguous() const noexcept
    {
        return m_raw.is_contiguous();
    }

    inline constexpr bool is_strided() const noexcept
    {
        return m_raw.is_strided();
    }

    inline constexpr index_type stride(size_t r) const
    {
        return m_raw.stride();
    }

    /** Swaps this field with another
     * @param other the Block to swap with this one
     */
    inline constexpr void swap(BlockView& other)
    {
        BlockView tmp = std::move(other);
        other = std::move(*this);
        *this = std::move(tmp);
    }

    /** Provide access to the mesh on which this block is defined
     * @return the mesh on which this block is defined
     */
    inline constexpr Mesh mesh() const noexcept
    {
        return m_mesh;
    }

    /** Provide access to the domain on which this block is defined
     * @return the domain on which this block is defined
     */
    inline constexpr MDomain_ domain() const noexcept
    {
        return MDomain_(mesh(), mcoord_end<Tags...>(raw_view().extents()));
    }

    /** Provide access to the domain on which this block is defined
     * @return the domain on which this block is defined
     */
    template <class... OTags>
    inline constexpr MDomain<OTags...> domain() const noexcept
    {
        return MDomain<OTags...>(domain());
    }

    /** Provide a modifiable view of the data
     * @return a modifiable view of the data
     */
    inline constexpr RawView raw_view()
    {
        return m_raw;
    }

    /** Provide a constant view of the data
     * @return a constant view of the data
     */
    inline constexpr const RawView raw_view() const
    {
        return m_raw;
    }

    /** Slice out some dimensions
     * @param slices the coordinates to 
     */
    template <class... SliceSpecs>
    inline constexpr auto slice(SliceSpecs&&... slices) const
    {
        return Slicer<std::remove_cv_t<std::remove_reference_t<SliceSpecs>>...>::
                slice(*this, std::forward<SliceSpecs>(slices)...);
    }

    /** Duplicate the data of this view
     * @return a copy of the data of this view
     */
    inline constexpr Block<MDomain<Tags...>, ElementType> duplicate() const
    {
        Block<MDomain<Tags...>, ElementType> result(this->domain());
        deepcopy(result, *this);
        return result;
    }
};


/** Constructs a new BlockView from scratch
 * @param mesh the mesh that sustains the view
 * @param raw_view the raw view to the data
 */
template <bool O_CONTIGUOUS, class OElementType, class... OTags>
static BlockView<MDomain<OTags...>, OElementType, O_CONTIGUOUS> make_view(
        RegularMesh<OTags...> const& mesh,
        ViewND<sizeof...(OTags), OElementType, O_CONTIGUOUS> const& raw_view)
{
    return BlockView<MDomain<OTags...>, OElementType, O_CONTIGUOUS>(mesh, raw_view);
}

template <class... QueryTags, class... Tags, class ElementType, bool CONTIGUOUS>
RegularMDomain<QueryTags...> get_domain(
        const BlockView<MDomain<Tags...>, ElementType, CONTIGUOUS>& v)
{
    return v.template domain<Tags...>();
}

namespace detail {
template <class... Tags, class ElementType, bool CONTIGUOUS, class Functor, class... Indices>
inline void for_each_impl(
        const BlockView<MDomain<Tags...>, ElementType, CONTIGUOUS>& to,
        Functor&& f,
        Indices&&... idxs) noexcept
{
    if constexpr (
            sizeof...(Indices) == BlockView<MDomain<Tags...>, ElementType, CONTIGUOUS>::rank()) {
        f(std::forward<Indices>(idxs)...);
    } else {
        for (ptrdiff_t ii = 0; ii < to.extent(sizeof...(Indices)); ++ii) {
            for_each_impl(to, std::forward<Functor>(f), std::forward<Indices...>(idxs)..., ii);
        }
    }
}
} // namespace detail

template <class... Tags, class ElementType, bool CONTIGUOUS, class Functor>
inline void for_each(
        const BlockView<MDomain<Tags...>, ElementType, CONTIGUOUS>& to,
        Functor&& f) noexcept
{
    detail::for_each_impl(to, std::forward<Functor>(f));
}

using DBlockViewX = BlockView<MDomain<Dim::X>, double>;

using DBlockViewVx = BlockView<MDomain<Dim::Vx>, double>;

using DBlockViewXVx = BlockView<MDomain<Dim::X, Dim::Vx>, double>;

template <class... Tags, class ElementType, bool CONTIGUOUS, bool OCONTIGUOUS>
inline BlockView<MDomain<Tags...>, ElementType, CONTIGUOUS> deepcopy(
        BlockView<MDomain<Tags...>, ElementType, CONTIGUOUS> to,
        BlockView<MDomain<Tags...>, ElementType, OCONTIGUOUS> const& from) noexcept
{
    assert(to.extents() == from.extents());
    for_each(to, [&to, &from](auto... idxs) { to(idxs...) = from(idxs...); });
    return to;
}


template <class... Tags, class ElementType>
class Block<MDomain<Tags...>, ElementType> : public BlockView<MDomain<Tags...>, ElementType>
{
public:
    /// ND view on this block
    using BlockView_ = BlockView<MDomain<Tags...>, ElementType>;

    /// ND memory view
    using RawView = ViewND<sizeof...(Tags), ElementType>;

    using MDomain_ = MDomain<Tags...>;

    using Mesh = typename MDomain_::Mesh;

    using MCoord_ = typename MDomain_::MCoord_;

    using extents_type = typename BlockView_::extents_type;

    using layout_type = typename BlockView_::layout_type;

    using accessor_type = typename BlockView_::accessor_type;

    using mapping_type = typename BlockView_::mapping_type;

    using element_type = typename BlockView_::element_type;

    using value_type = typename BlockView_::value_type;

    using index_type = typename BlockView_::index_type;

    using difference_type = typename BlockView_::difference_type;

    using pointer = typename BlockView_::pointer;

    using reference = typename BlockView_::reference;

public:
    /** Construct a Block on a domain with uninitialized values
     */
    template <class... OTags>
    explicit inline constexpr Block(const MDomain<OTags...>& domain)
        : BlockView_(
                domain,
                RawView(new value_type[domain.size()],
                        ExtentsND<sizeof...(Tags)>(domain.template extent<Tags>()...)))
    {
    }

    /** Constructs a new Block by copy
     * 
     * This is deleted, one should use deepcopy
     * @param other the Block to copy
     */
    inline constexpr Block(const Block& other) = delete;

    /** Constructs a new Block by move
     * @param other the Block to move
     */
    inline constexpr Block(Block&& other) = default;

    inline ~Block()
    {
        delete[] this->raw_view().data();
    }

    /** Copy-assigns a new value to this field
     * @param other the Block to copy
     * @return *this
     */
    inline constexpr Block& operator=(const Block& other) = default;

    /** Move-assigns a new value to this field
     * @param other the Block to move
     * @return *this
     */
    inline constexpr Block& operator=(Block&& other) = default;

    /** Copy-assigns a new value to this field
     * @param other the Block to copy
     * @return *this
     */
    template <class... OTags, class OElementType>
    inline Block& operator=(Block<MDomain<OTags...>, OElementType>&& other)
    {
        copy(*this, other);
        return *this;
    }

    /** Swaps this field with another
     * @param other the Block to swap with this one
     */
    inline constexpr void swap(Block& other)
    {
        Block tmp = std::move(other);
        other = std::move(*this);
        *this = std::move(tmp);
    }
};

template <class ElementType>
using BlockX = Block<MDomain<Dim::X>, ElementType>;

using DBlockX = BlockX<double>;

template <class ElementType>
using BlockVx = Block<MDomain<Dim::Vx>, ElementType>;

using DBlockVx = BlockVx<double>;

template <class ElementType>
using BlockXVx = Block<MDomain<Dim::X, Dim::Vx>, ElementType>;

using DBlockXVx = BlockXVx<double>;
