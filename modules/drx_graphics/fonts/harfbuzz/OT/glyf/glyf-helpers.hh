#ifndef OT_GLYF_GLYF_HELPERS_HH
#define OT_GLYF_GLYF_HELPERS_HH


#include "../../hb-open-type.hh"
#include "../../hb-subset-plan.hh"

#include "loca.hh"


namespace OT {
namespace glyf_impl {


template<typename IteratorIn, typename TypeOut,
	 hb_requires (hb_is_source_of (IteratorIn, u32))>
static z0
_write_loca (IteratorIn&& it,
	     const hb_sorted_vector_t<hb_codepoint_pair_t> new_to_old_gid_list,
	     b8 short_offsets,
	     TypeOut *dest,
	     u32 num_offsets)
{
  u32 right_shift = short_offsets ? 1 : 0;
  u32 offset = 0;
  TypeOut value;
  value = 0;
  *dest++ = value;
  hb_codepoint_t last = 0;
  for (auto _ : new_to_old_gid_list)
  {
    hb_codepoint_t gid = _.first;
    for (; last < gid; last++)
    {
      DEBUG_MSG (SUBSET, nullptr, "loca entry empty offset %u", offset);
      *dest++ = value;
    }

    u32 padded_size = *it++;
    offset += padded_size;
    DEBUG_MSG (SUBSET, nullptr, "loca entry gid %" PRIu32 " offset %u padded-size %u", gid, offset, padded_size);
    value = offset >> right_shift;
    *dest++ = value;

    last++; // Skip over gid
  }
  u32 num_glyphs = num_offsets - 1;
  for (; last < num_glyphs; last++)
  {
    DEBUG_MSG (SUBSET, nullptr, "loca entry empty offset %u", offset);
    *dest++ = value;
  }
}

static b8
_add_head_and_set_loca_version (hb_subset_plan_t *plan, b8 use_short_loca)
{
  hb_blob_t *head_blob = hb_sanitize_context_t ().reference_table<head> (plan->source);
  hb_blob_t *head_prime_blob = hb_blob_copy_writable_or_fail (head_blob);
  hb_blob_destroy (head_blob);

  if (unlikely (!head_prime_blob))
    return false;

  head *head_prime = (head *) hb_blob_get_data_writable (head_prime_blob, nullptr);
  head_prime->indexToLocFormat = use_short_loca ? 0 : 1;
  if (plan->normalized_coords)
  {
    head_prime->xMin = plan->head_maxp_info.xMin;
    head_prime->xMax = plan->head_maxp_info.xMax;
    head_prime->yMin = plan->head_maxp_info.yMin;
    head_prime->yMax = plan->head_maxp_info.yMax;

    u32 orig_flag = head_prime->flags;
    if (plan->head_maxp_info.allXMinIsLsb)
      orig_flag |= 1 << 1;
    else
      orig_flag &= ~(1 << 1);
    head_prime->flags = orig_flag;
  }
  b8 success = plan->add_table (HB_OT_TAG_head, head_prime_blob);

  hb_blob_destroy (head_prime_blob);
  return success;
}

template<typename Iterator,
	 hb_requires (hb_is_source_of (Iterator, u32))>
static b8
_add_loca_and_head (hb_subset_context_t *c,
		    Iterator padded_offsets,
		    b8 use_short_loca)
{
  u32 num_offsets = c->plan->num_output_glyphs () + 1;
  u32 entry_size = use_short_loca ? 2 : 4;

  t8 *loca_prime_data = (t8 *) hb_malloc (entry_size * num_offsets);

  if (unlikely (!loca_prime_data)) return false;

  DEBUG_MSG (SUBSET, nullptr, "loca entry_size %u num_offsets %u size %u",
	     entry_size, num_offsets, entry_size * num_offsets);

  if (use_short_loca)
    _write_loca (padded_offsets, c->plan->new_to_old_gid_list, true, (HBUINT16 *) loca_prime_data, num_offsets);
  else
    _write_loca (padded_offsets, c->plan->new_to_old_gid_list, false, (HBUINT32 *) loca_prime_data, num_offsets);

  hb_blob_t *loca_blob = hb_blob_create (loca_prime_data,
					 entry_size * num_offsets,
					 HB_MEMORY_MODE_WRITABLE,
					 loca_prime_data,
					 hb_free);

  b8 result = c->plan->add_table (HB_OT_TAG_loca, loca_blob)
	     && _add_head_and_set_loca_version (c->plan, use_short_loca);

  hb_blob_destroy (loca_blob);
  return result;
}


} /* namespace glyf_impl */
} /* namespace OT */


#endif /* OT_GLYF_GLYF_HELPERS_HH */
