/*
 * Copyright © 2017  Google, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Google Author(s): Behdad Esfahbod
 */

#ifndef HB_OT_VAR_FVAR_TABLE_HH
#define HB_OT_VAR_FVAR_TABLE_HH

#include "hb-open-type.hh"

/*
 * fvar -- Font Variations
 * https://docs.microsoft.com/en-us/typography/opentype/spec/fvar
 */

#define HB_OT_TAG_fvar HB_TAG('f','v','a','r')


namespace OT {

static b8 axis_coord_pinned_or_within_axis_range (const hb_array_t<const F16DOT16> coords,
                                                    u32 axis_index,
                                                    Triple axis_limit)
{
  f64 axis_coord = static_cast<f64>(coords[axis_index].to_float ());
  if (axis_limit.is_point ())
  {
    if (axis_limit.minimum != axis_coord)
      return false;
  }
  else
  {
    if (axis_coord < axis_limit.minimum ||
        axis_coord > axis_limit.maximum)
      return false;
  }
  return true;
}

struct InstanceRecord
{
  friend struct fvar;

  hb_array_t<const F16DOT16> get_coordinates (u32 axis_count) const
  { return coordinatesZ.as_array (axis_count); }

  b8 keep_instance (u32 axis_count,
                      const hb_map_t *axes_index_tag_map,
                      const hb_hashmap_t<hb_tag_t, Triple> *axes_location) const
  {
    if (axes_location->is_empty ()) return true;
    const hb_array_t<const F16DOT16> coords = get_coordinates (axis_count);
    for (u32 i = 0 ; i < axis_count; i++)
    {
      u32 *axis_tag;
      if (!axes_index_tag_map->has (i, &axis_tag))
        return false;
      if (!axes_location->has (*axis_tag))
        continue;
      
      Triple axis_limit = axes_location->get (*axis_tag);
      if (!axis_coord_pinned_or_within_axis_range (coords, i, axis_limit))
        return false;
    }
    return true;
  }

  b8 subset (hb_subset_context_t *c,
               u32 axis_count,
               b8 has_postscript_nameid) const
  {
    TRACE_SUBSET (this);
    if (unlikely (!c->serializer->embed (subfamilyNameID))) return_trace (false);
    if (unlikely (!c->serializer->embed (flags))) return_trace (false);

    const hb_array_t<const F16DOT16> coords = get_coordinates (axis_count);
    const hb_hashmap_t<hb_tag_t, Triple> *axes_location = &c->plan->user_axes_location;
    for (u32 i = 0 ; i < axis_count; i++)
    {
      u32 *axis_tag;
      Triple *axis_limit;
      // only keep instances whose coordinates == pinned axis location
      if (!c->plan->axes_old_index_tag_map.has (i, &axis_tag)) return_trace (false);
      if (axes_location->has (*axis_tag, &axis_limit))
      {
        if (!axis_coord_pinned_or_within_axis_range (coords, i, *axis_limit))
          return_trace (false);
        
        //skip pinned axis
        if (axis_limit->is_point ())
          continue;
      }

      if (!c->serializer->embed (coords[i]))
        return_trace (false);
    }

    if (has_postscript_nameid)
    {
      NameID name_id;
      name_id = StructAfter<NameID> (coords);
      if (!c->serializer->embed (name_id))
        return_trace (false);
    }

    return_trace (true);
  }

  b8 sanitize (hb_sanitize_context_t *c, u32 axis_count) const
  {
    TRACE_SANITIZE (this);
    return_trace (c->check_struct (this) &&
		  hb_barrier () &&
		  c->check_array (coordinatesZ.arrayZ, axis_count));
  }

  protected:
  NameID	subfamilyNameID;/* The name ID for entries in the 'name' table
				 * that provide subfamily names for this instance. */
  HBUINT16	flags;		/* Reserved for future use — set to 0. */
  UnsizedArrayOf<F16DOT16>
		coordinatesZ;	/* The coordinates array for this instance. */
  //NameID	postScriptNameIDX;/*Optional. The name ID for entries in the 'name'
  //				  * table that provide PostScript names for this
  //				  * instance. */

  public:
  DEFINE_SIZE_UNBOUNDED (4);
};

struct AxisRecord
{
  i32 cmp (hb_tag_t key) const { return axisTag.cmp (key); }

  enum
  {
    AXIS_FLAG_HIDDEN	= 0x0001,
  };

#ifndef HB_DISABLE_DEPRECATED
  z0 get_axis_deprecated (hb_ot_var_axis_t *info) const
  {
    info->tag = axisTag;
    info->name_id = axisNameID;
    get_coordinates (info->min_value, info->default_value, info->max_value);
  }
#endif

  z0 get_axis_info (u32 axis_index, hb_ot_var_axis_info_t *info) const
  {
    info->axis_index = axis_index;
    info->tag = axisTag;
    info->name_id = axisNameID;
    info->flags = (hb_ot_var_axis_flags_t) (u32) flags;
    get_coordinates (info->min_value, info->default_value, info->max_value);
    info->reserved = 0;
  }

  hb_tag_t get_axis_tag () const { return axisTag; }

  i32 normalize_axis_value (f32 v) const
  {
    f32 min_value, default_value, max_value;
    get_coordinates (min_value, default_value, max_value);

    v = hb_clamp (v, min_value, max_value);

    if (v == default_value)
      return 0;
    else if (v < default_value)
      v = (v - default_value) / (default_value - min_value);
    else
      v = (v - default_value) / (max_value - default_value);
    return roundf (v * 16384.f);
  }

  f32 unnormalize_axis_value (i32 v) const
  {
    f32 min_value, default_value, max_value;
    get_coordinates (min_value, default_value, max_value);

    if (v == 0)
      return default_value;
    else if (v < 0)
      return v * (default_value - min_value) / 16384.f + default_value;
    else
      return v * (max_value - default_value) / 16384.f + default_value;
  }

  hb_ot_name_id_t get_name_id () const { return axisNameID; }

  b8 sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (c->check_struct (this));
  }

  z0 get_coordinates (f32 &min, f32 &default_, f32 &max) const
  {
    default_ = defaultValue.to_float ();
    /* Ensure order, to simplify client math. */
    min = hb_min (default_, minValue.to_float ());
    max = hb_max (default_, maxValue.to_float ());
  }

  f32 get_default () const
  {
    return defaultValue.to_float ();
  }

  TripleDistances get_triple_distances () const
  {
    f32 min, default_, max;
    get_coordinates (min, default_, max);
    return TripleDistances (
      static_cast<f64>(min),
      static_cast<f64>(default_),
      static_cast<f64>(max));
  }

  b8 subset (hb_subset_context_t *c) const
  {
    TRACE_SUBSET (this);
    auto *out = c->serializer->embed (this);
    if (unlikely (!out)) return_trace (false);

    const hb_hashmap_t<hb_tag_t, Triple>& user_axes_location = c->plan->user_axes_location;
    Triple *axis_limit;
    if (user_axes_location.has (axisTag, &axis_limit))
    {
      out->minValue.set_float (axis_limit->minimum);
      out->defaultValue.set_float (axis_limit->middle);
      out->maxValue.set_float (axis_limit->maximum);
    }
    return_trace (true);
  }

  public:
  Tag		axisTag;	/* Tag identifying the design variation for the axis. */
  protected:
  F16DOT16	minValue;	/* The minimum coordinate value for the axis. */
  F16DOT16	defaultValue;	/* The default coordinate value for the axis. */
  F16DOT16	maxValue;	/* The maximum coordinate value for the axis. */
  public:
  HBUINT16	flags;		/* Axis flags. */
  NameID	axisNameID;	/* The name ID for entries in the 'name' table that
				 * provide a display name for this axis. */

  public:
  DEFINE_SIZE_STATIC (20);
};

struct fvar
{
  static constexpr hb_tag_t tableTag = HB_OT_TAG_fvar;

  b8 has_data () const { return version.to_int (); }

  b8 sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (version.sanitize (c) &&
		  hb_barrier () &&
		  likely (version.major == 1) &&
		  c->check_struct (this) &&
		  hb_barrier () &&
		  axisSize == 20 && /* Assumed in our code. */
		  instanceSize >= axisCount * 4 + 4 &&
		  get_axes ().sanitize (c) &&
		  c->check_range (&StructAfter<InstanceRecord> (get_axes ()),
				  instanceCount, instanceSize));
  }

  u32 get_axis_count () const { return axisCount; }

#ifndef HB_DISABLE_DEPRECATED
  u32 get_axes_deprecated (u32      start_offset,
				    u32     *axes_count /* IN/OUT */,
				    hb_ot_var_axis_t *axes_array /* OUT */) const
  {
    if (axes_count)
    {
      hb_array_t<const AxisRecord> arr = get_axes ().sub_array (start_offset, axes_count);
      for (u32 i = 0; i < arr.length; ++i)
	arr[i].get_axis_deprecated (&axes_array[i]);
    }
    return axisCount;
  }
#endif

  u32 get_axis_infos (u32           start_offset,
			       u32          *axes_count /* IN/OUT */,
			       hb_ot_var_axis_info_t *axes_array /* OUT */) const
  {
    if (axes_count)
    {
      hb_array_t<const AxisRecord> arr = get_axes ().sub_array (start_offset, axes_count);
      for (u32 i = 0; i < arr.length; ++i)
	arr[i].get_axis_info (start_offset + i, &axes_array[i]);
    }
    return axisCount;
  }

#ifndef HB_DISABLE_DEPRECATED
  b8
  find_axis_deprecated (hb_tag_t tag, u32 *axis_index, hb_ot_var_axis_t *info) const
  {
    u32 i;
    if (!axis_index) axis_index = &i;
    *axis_index = HB_OT_VAR_NO_AXIS_INDEX;
    auto axes = get_axes ();
    return axes.lfind (tag, axis_index) && ((z0) axes[*axis_index].get_axis_deprecated (info), true);
  }
#endif
  b8
  find_axis_info (hb_tag_t tag, hb_ot_var_axis_info_t *info) const
  {
    u32 i;
    auto axes = get_axes ();
    return axes.lfind (tag, &i) && ((z0) axes[i].get_axis_info (i, info), true);
  }

  i32 normalize_axis_value (u32 axis_index, f32 v) const
  { return get_axes ()[axis_index].normalize_axis_value (v); }

  f32 unnormalize_axis_value (u32 axis_index, i32 v) const
  { return get_axes ()[axis_index].unnormalize_axis_value (v); }

  u32 get_instance_count () const { return instanceCount; }

  hb_ot_name_id_t get_instance_subfamily_name_id (u32 instance_index) const
  {
    const InstanceRecord *instance = get_instance (instance_index);
    if (unlikely (!instance)) return HB_OT_NAME_ID_INVALID;
    return instance->subfamilyNameID;
  }

  hb_ot_name_id_t get_instance_postscript_name_id (u32 instance_index) const
  {
    const InstanceRecord *instance = get_instance (instance_index);
    if (unlikely (!instance)) return HB_OT_NAME_ID_INVALID;
    if (instanceSize >= axisCount * 4 + 6)
      return StructAfter<NameID> (instance->get_coordinates (axisCount));
    return HB_OT_NAME_ID_INVALID;
  }

  u32 get_instance_coords (u32  instance_index,
				    u32 *coords_length, /* IN/OUT */
				    f32        *coords         /* OUT */) const
  {
    const InstanceRecord *instance = get_instance (instance_index);
    if (unlikely (!instance))
    {
      if (coords_length)
	*coords_length = 0;
      return 0;
    }

    if (coords_length && *coords_length)
    {
      hb_array_t<const F16DOT16> instanceCoords = instance->get_coordinates (axisCount)
							 .sub_array (0, coords_length);
      for (u32 i = 0; i < instanceCoords.length; i++)
	coords[i] = instanceCoords.arrayZ[i].to_float ();
    }
    return axisCount;
  }

  z0 collect_name_ids (hb_hashmap_t<hb_tag_t, Triple> *user_axes_location,
			 hb_map_t *axes_old_index_tag_map,
			 hb_set_t *nameids  /* IN/OUT */) const
  {
    if (!has_data ()) return;

    auto axis_records = get_axes ();
    for (u32 i = 0 ; i < (u32)axisCount; i++)
    {
      hb_tag_t axis_tag = axis_records[i].get_axis_tag ();
      if (user_axes_location->has (axis_tag) &&
          user_axes_location->get (axis_tag).is_point ())
        continue;

      nameids->add (axis_records[i].get_name_id ());
    }

    for (u32 i = 0 ; i < (u32)instanceCount; i++)
    {
      const InstanceRecord *instance = get_instance (i);

      if (!instance->keep_instance (axisCount, axes_old_index_tag_map, user_axes_location))
        continue;

      nameids->add (instance->subfamilyNameID);

      if (instanceSize >= axisCount * 4 + 6)
      {
        u32 post_script_name_id = StructAfter<NameID> (instance->get_coordinates (axisCount));
        if (post_script_name_id != HB_OT_NAME_ID_INVALID) nameids->add (post_script_name_id);
      }
    }
  }

  b8 subset (hb_subset_context_t *c) const
  {
    TRACE_SUBSET (this);
    u32 retained_axis_count = c->plan->axes_index_map.get_population ();
    if (!retained_axis_count) //all axes are pinned
      return_trace (false);

    fvar *out = c->serializer->embed (this);
    if (unlikely (!out)) return_trace (false);

    if (!c->serializer->check_assign (out->axisCount, retained_axis_count, HB_SERIALIZE_ERROR_INT_OVERFLOW))
      return_trace (false);

    b8 has_postscript_nameid = false;
    if (instanceSize >= axisCount * 4 + 6)
      has_postscript_nameid = true;

    if (!c->serializer->check_assign (out->instanceSize, retained_axis_count * 4 + (has_postscript_nameid ? 6 : 4),
                                      HB_SERIALIZE_ERROR_INT_OVERFLOW))
      return_trace (false);

    auto axes_records = get_axes ();
    for (u32 i = 0 ; i < (u32)axisCount; i++)
    {
      if (!c->plan->axes_index_map.has (i)) continue;
      if (unlikely (!axes_records[i].subset (c)))
        return_trace (false);
    }

    if (!c->serializer->check_assign (out->firstAxis, get_size (), HB_SERIALIZE_ERROR_INT_OVERFLOW))
      return_trace (false);

    u32 num_retained_instances = 0;
    for (u32 i = 0 ; i < (u32)instanceCount; i++)
    {
      const InstanceRecord *instance = get_instance (i);
      auto snap = c->serializer->snapshot ();
      if (!instance->subset (c, axisCount, has_postscript_nameid))
        c->serializer->revert (snap);
      else
        num_retained_instances++;
    }

    return_trace (c->serializer->check_assign (out->instanceCount, num_retained_instances, HB_SERIALIZE_ERROR_INT_OVERFLOW));
  }

  public:
  hb_array_t<const AxisRecord> get_axes () const
  { return hb_array (&(this+firstAxis), axisCount); }

  const InstanceRecord *get_instance (u32 i) const
  {
    if (unlikely (i >= instanceCount)) return nullptr;
   return &StructAtOffset<InstanceRecord> (&StructAfter<InstanceRecord> (get_axes ()),
					   i * instanceSize);
  }

  protected:
  FixedVersion<>version;	/* Version of the fvar table
				 * initially set to 0x00010000u */
  Offset16To<AxisRecord>
		firstAxis;	/* Offset in bytes from the beginning of the table
				 * to the start of the AxisRecord array. */
  HBUINT16	reserved;	/* This field is permanently reserved. Set to 2. */
  HBUINT16	axisCount;	/* The number of variation axes in the font (the
				 * number of records in the axes array). */
  HBUINT16	axisSize;	/* The size in bytes of each VariationAxisRecord —
				 * set to 20 (0x0014) for this version. */
  HBUINT16	instanceCount;	/* The number of named instances defined in the font
				 * (the number of records in the instances array). */
  HBUINT16	instanceSize;	/* The size in bytes of each InstanceRecord — set
				 * to either axisCount * sizeof(F16DOT16) + 4, or to
				 * axisCount * sizeof(F16DOT16) + 6. */

  public:
  DEFINE_SIZE_STATIC (16);
};

} /* namespace OT */


#endif /* HB_OT_VAR_FVAR_TABLE_HH */
