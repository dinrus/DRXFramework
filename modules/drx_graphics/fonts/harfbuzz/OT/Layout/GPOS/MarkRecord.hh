#ifndef OT_LAYOUT_GPOS_MARKRECORD_HH
#define OT_LAYOUT_GPOS_MARKRECORD_HH

namespace OT {
namespace Layout {
namespace GPOS_impl {

struct MarkRecord
{
  friend struct MarkArray;

  public:
  HBUINT16      klass;                  /* Class defined for this mark */
  Offset16To<Anchor>
                markAnchor;             /* Offset to Anchor table--from
                                         * beginning of MarkArray table */
  public:
  DEFINE_SIZE_STATIC (4);

  u32 get_class () const { return (u32) klass; }
  b8 sanitize (hb_sanitize_context_t *c, ukk base) const
  {
    TRACE_SANITIZE (this);
    return_trace (c->check_struct (this) && markAnchor.sanitize (c, base));
  }

  b8 subset (hb_subset_context_t    *c,
	       const z0             *src_base,
	       const hb_map_t         *klass_mapping) const
  {
    TRACE_SUBSET (this);
    auto *out = c->serializer->embed (this);
    if (unlikely (!out)) return_trace (false);

    out->klass = klass_mapping->get (klass);
    return_trace (out->markAnchor.serialize_subset (c, markAnchor, src_base));
  }

  z0 collect_variation_indices (hb_collect_variation_indices_context_t *c,
                                  ukk src_base) const
  {
    (src_base+markAnchor).collect_variation_indices (c);
  }
};


}
}
}

#endif /* OT_LAYOUT_GPOS_MARKRECORD_HH */
