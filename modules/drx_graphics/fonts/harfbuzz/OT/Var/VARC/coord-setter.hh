#ifndef OT_VAR_VARC_COORD_SETTER_HH
#define OT_VAR_VARC_COORD_SETTER_HH


#include "../../../hb.hh"


namespace OT {
//namespace Var {


struct coord_setter_t
{
  coord_setter_t (hb_array_t<i32k> coords) :
    coords (coords) {}

  i32& operator [] (u32 idx)
  {
    if (unlikely (idx >= HB_VAR_COMPOSITE_MAX_AXES))
      return Crap(i32);
    if (coords.length < idx + 1)
      coords.resize (idx + 1);
    return coords[idx];
  }

  hb_array_t<i32> get_coords ()
  { return coords.as_array (); }

  hb_vector_t<i32> coords;
};


//} // namespace Var

} // namespace OT

#endif /* OT_VAR_VARC_COORD_SETTER_HH */
