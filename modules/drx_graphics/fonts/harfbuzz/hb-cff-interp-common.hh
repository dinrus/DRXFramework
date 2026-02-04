/*
 * Copyright © 2018 Adobe Inc.
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
 * Adobe Author(s): Michiharu Ariza
 */
#ifndef HB_CFF_INTERP_COMMON_HH
#define HB_CFF_INTERP_COMMON_HH

extern HB_INTERNAL u8k *endchar_str;

namespace CFF {

using namespace OT;

typedef u32 op_code_t;


/* === Dict operators === */

/* One byte operators (0-31) */
#define OpCode_version		  0 /* CFF Top */
#define OpCode_Notice		  1 /* CFF Top */
#define OpCode_FullName		  2 /* CFF Top */
#define OpCode_FamilyName	  3 /* CFF Top */
#define OpCode_Weight		  4 /* CFF Top */
#define OpCode_FontBBox		  5 /* CFF Top */
#define OpCode_BlueValues	  6 /* CFF Private, CFF2 Private */
#define OpCode_OtherBlues	  7 /* CFF Private, CFF2 Private */
#define OpCode_FamilyBlues	  8 /* CFF Private, CFF2 Private */
#define OpCode_FamilyOtherBlues	  9 /* CFF Private, CFF2 Private */
#define OpCode_StdHW		 10 /* CFF Private, CFF2 Private */
#define OpCode_StdVW		 11 /* CFF Private, CFF2 Private */
#define OpCode_escape		 12 /* All. Shared with CS */
#define OpCode_UniqueID		 13 /* CFF Top */
#define OpCode_XUID		 14 /* CFF Top */
#define OpCode_charset		 15 /* CFF Top (0) */
#define OpCode_Encoding		 16 /* CFF Top (0) */
#define OpCode_CharStrings	 17 /* CFF Top, CFF2 Top */
#define OpCode_Private		 18 /* CFF Top, CFF2 FD */
#define OpCode_Subrs		 19 /* CFF Private, CFF2 Private */
#define OpCode_defaultWidthX	 20 /* CFF Private (0) */
#define OpCode_nominalWidthX	 21 /* CFF Private (0) */
#define OpCode_vsindexdict	 22 /* CFF2 Private/CS */
#define OpCode_blenddict	 23 /* CFF2 Private/CS */
#define OpCode_vstore		 24 /* CFF2 Top */
#define OpCode_reserved25	 25
#define OpCode_reserved26	 26
#define OpCode_reserved27	 27

/* Numbers */
#define OpCode_shortint		 28 /* 16-bit integer, All */
#define OpCode_longintdict	 29 /* 32-bit integer, All */
#define OpCode_BCD		 30 /* Real number, CFF2 Top/FD */
#define OpCode_reserved31	 31

/* 1-byte integers */
#define OpCode_OneByteIntFirst	 32 /* All. beginning of the range of first byte ints */
#define OpCode_OneByteIntLast	246 /* All. ending of the range of first byte i32 */

/* 2-byte integers */
#define OpCode_TwoBytePosInt0	247 /* All. first byte of two byte positive i32 (+108 to +1131) */
#define OpCode_TwoBytePosInt1	248
#define OpCode_TwoBytePosInt2	249
#define OpCode_TwoBytePosInt3	250

#define OpCode_TwoByteNegInt0	251 /* All. first byte of two byte negative i32 (-1131 to -108) */
#define OpCode_TwoByteNegInt1	252
#define OpCode_TwoByteNegInt2	253
#define OpCode_TwoByteNegInt3	254

/* Two byte escape operators 12, (0-41) */
#define OpCode_ESC_Base		256
#define Make_OpCode_ESC(byte2)	((op_code_t)(OpCode_ESC_Base + (byte2)))

inline op_code_t Unmake_OpCode_ESC (op_code_t op)  { return (op_code_t)(op - OpCode_ESC_Base); }
inline b8 Is_OpCode_ESC (op_code_t op) { return op >= OpCode_ESC_Base; }
inline u32 OpCode_Size (op_code_t op) { return Is_OpCode_ESC (op) ? 2: 1; }

#define OpCode_Copyright	Make_OpCode_ESC(0) /* CFF Top */
#define OpCode_isFixedPitch	Make_OpCode_ESC(1) /* CFF Top (false) */
#define OpCode_ItalicAngle	Make_OpCode_ESC(2) /* CFF Top (0) */
#define OpCode_UnderlinePosition Make_OpCode_ESC(3) /* CFF Top (-100) */
#define OpCode_UnderlineThickness Make_OpCode_ESC(4) /* CFF Top (50) */
#define OpCode_PaintType	Make_OpCode_ESC(5) /* CFF Top (0) */
#define OpCode_CharstringType	Make_OpCode_ESC(6) /* CFF Top (2) */
#define OpCode_FontMatrix	Make_OpCode_ESC(7) /* CFF Top, CFF2 Top (.001 0 0 .001 0 0)*/
#define OpCode_StrokeWidth	Make_OpCode_ESC(8) /* CFF Top (0) */
#define OpCode_BlueScale	Make_OpCode_ESC(9) /* CFF Private, CFF2 Private (0.039625) */
#define OpCode_BlueShift	Make_OpCode_ESC(10) /* CFF Private, CFF2 Private (7) */
#define OpCode_BlueFuzz		Make_OpCode_ESC(11) /* CFF Private, CFF2 Private (1) */
#define OpCode_StemSnapH	Make_OpCode_ESC(12) /* CFF Private, CFF2 Private */
#define OpCode_StemSnapV	Make_OpCode_ESC(13) /* CFF Private, CFF2 Private */
#define OpCode_ForceBold	Make_OpCode_ESC(14) /* CFF Private (false) */
#define OpCode_reservedESC15	Make_OpCode_ESC(15)
#define OpCode_reservedESC16	Make_OpCode_ESC(16)
#define OpCode_LanguageGroup	Make_OpCode_ESC(17) /* CFF Private, CFF2 Private (0) */
#define OpCode_ExpansionFactor	Make_OpCode_ESC(18) /* CFF Private, CFF2 Private (0.06) */
#define OpCode_initialRandomSeed Make_OpCode_ESC(19) /* CFF Private (0) */
#define OpCode_SyntheticBase	Make_OpCode_ESC(20) /* CFF Top */
#define OpCode_PostScript	Make_OpCode_ESC(21) /* CFF Top */
#define OpCode_BaseFontName	Make_OpCode_ESC(22) /* CFF Top */
#define OpCode_BaseFontBlend	Make_OpCode_ESC(23) /* CFF Top */
#define OpCode_reservedESC24	Make_OpCode_ESC(24)
#define OpCode_reservedESC25	Make_OpCode_ESC(25)
#define OpCode_reservedESC26	Make_OpCode_ESC(26)
#define OpCode_reservedESC27	Make_OpCode_ESC(27)
#define OpCode_reservedESC28	Make_OpCode_ESC(28)
#define OpCode_reservedESC29	Make_OpCode_ESC(29)
#define OpCode_ROS		Make_OpCode_ESC(30) /* CFF Top_CID */
#define OpCode_CIDFontVersion	Make_OpCode_ESC(31) /* CFF Top_CID (0) */
#define OpCode_CIDFontRevision	Make_OpCode_ESC(32) /* CFF Top_CID (0) */
#define OpCode_CIDFontType	Make_OpCode_ESC(33) /* CFF Top_CID (0) */
#define OpCode_CIDCount		Make_OpCode_ESC(34) /* CFF Top_CID (8720) */
#define OpCode_UIDBase		Make_OpCode_ESC(35) /* CFF Top_CID */
#define OpCode_FDArray		Make_OpCode_ESC(36) /* CFF Top_CID, CFF2 Top */
#define OpCode_FDSelect		Make_OpCode_ESC(37) /* CFF Top_CID, CFF2 Top */
#define OpCode_FontName		Make_OpCode_ESC(38) /* CFF Top_CID */


/* === CharString operators === */

#define OpCode_hstem		  1 /* CFF, CFF2 */
#define OpCode_Reserved2	  2
#define OpCode_vstem		  3 /* CFF, CFF2 */
#define OpCode_vmoveto		  4 /* CFF, CFF2 */
#define OpCode_rlineto		  5 /* CFF, CFF2 */
#define OpCode_hlineto		  6 /* CFF, CFF2 */
#define OpCode_vlineto		  7 /* CFF, CFF2 */
#define OpCode_rrcurveto	  8 /* CFF, CFF2 */
#define OpCode_Reserved9	  9
#define OpCode_callsubr		 10 /* CFF, CFF2 */
#define OpCode_return		 11 /* CFF */
//#define OpCode_escape		 12 /* CFF, CFF2 */
#define OpCode_Reserved13	 13
#define OpCode_endchar		 14 /* CFF */
#define OpCode_vsindexcs	 15 /* CFF2 */
#define OpCode_blendcs		 16 /* CFF2 */
#define OpCode_Reserved17	 17
#define OpCode_hstemhm		 18 /* CFF, CFF2 */
#define OpCode_hintmask		 19 /* CFF, CFF2 */
#define OpCode_cntrmask		 20 /* CFF, CFF2 */
#define OpCode_rmoveto		 21 /* CFF, CFF2 */
#define OpCode_hmoveto		 22 /* CFF, CFF2 */
#define OpCode_vstemhm		 23 /* CFF, CFF2 */
#define OpCode_rcurveline	 24 /* CFF, CFF2 */
#define OpCode_rlinecurve	 25 /* CFF, CFF2 */
#define OpCode_vvcurveto	 26 /* CFF, CFF2 */
#define OpCode_hhcurveto	 27 /* CFF, CFF2 */
//#define OpCode_shortint	 28 /* CFF, CFF2 */
#define OpCode_callgsubr	 29 /* CFF, CFF2 */
#define OpCode_vhcurveto	 30 /* CFF, CFF2 */
#define OpCode_hvcurveto	 31 /* CFF, CFF2 */

#define OpCode_fixedcs		255 /* 32-bit fixed */

/* Two byte escape operators 12, (0-41) */
#define OpCode_dotsection	Make_OpCode_ESC(0) /* CFF (obsoleted) */
#define OpCode_ReservedESC1	Make_OpCode_ESC(1)
#define OpCode_ReservedESC2	Make_OpCode_ESC(2)
#define OpCode_and		Make_OpCode_ESC(3) /* CFF */
#define OpCode_or		Make_OpCode_ESC(4) /* CFF */
#define OpCode_not		Make_OpCode_ESC(5) /* CFF */
#define OpCode_ReservedESC6	Make_OpCode_ESC(6)
#define OpCode_ReservedESC7	Make_OpCode_ESC(7)
#define OpCode_ReservedESC8	Make_OpCode_ESC(8)
#define OpCode_abs		Make_OpCode_ESC(9) /* CFF */
#define OpCode_add		Make_OpCode_ESC(10) /* CFF */
#define OpCode_sub		Make_OpCode_ESC(11) /* CFF */
#define OpCode_div		Make_OpCode_ESC(12) /* CFF */
#define OpCode_ReservedESC13	Make_OpCode_ESC(13)
#define OpCode_neg		Make_OpCode_ESC(14) /* CFF */
#define OpCode_eq		Make_OpCode_ESC(15) /* CFF */
#define OpCode_ReservedESC16	Make_OpCode_ESC(16)
#define OpCode_ReservedESC17	Make_OpCode_ESC(17)
#define OpCode_drop		Make_OpCode_ESC(18) /* CFF */
#define OpCode_ReservedESC19	Make_OpCode_ESC(19)
#define OpCode_put		Make_OpCode_ESC(20) /* CFF */
#define OpCode_get		Make_OpCode_ESC(21) /* CFF */
#define OpCode_ifelse		Make_OpCode_ESC(22) /* CFF */
#define OpCode_random		Make_OpCode_ESC(23) /* CFF */
#define OpCode_mul		Make_OpCode_ESC(24) /* CFF */
//#define OpCode_reservedESC25	Make_OpCode_ESC(25)
#define OpCode_sqrt		Make_OpCode_ESC(26) /* CFF */
#define OpCode_dup		Make_OpCode_ESC(27) /* CFF */
#define OpCode_exch		Make_OpCode_ESC(28) /* CFF */
#define OpCode_index		Make_OpCode_ESC(29) /* CFF */
#define OpCode_roll		Make_OpCode_ESC(30) /* CFF */
#define OpCode_reservedESC31	Make_OpCode_ESC(31)
#define OpCode_reservedESC32	Make_OpCode_ESC(32)
#define OpCode_reservedESC33	Make_OpCode_ESC(33)
#define OpCode_hflex		Make_OpCode_ESC(34) /* CFF, CFF2 */
#define OpCode_flex		Make_OpCode_ESC(35) /* CFF, CFF2 */
#define OpCode_hflex1		Make_OpCode_ESC(36) /* CFF, CFF2 */
#define OpCode_flex1		Make_OpCode_ESC(37) /* CFF, CFF2 */


#define OpCode_Invalid		0xFFFFu


struct number_t
{
  z0 set_int (i32 v)       { value = v; }
  i32 to_int () const        { return value; }

  z0 set_fixed (int32_t v) { value = v / 65536.0; }
  int32_t to_fixed () const  { return value * 65536.0; }

  z0 set_real (f64 v)   { value = v; }
  f64 to_real () const    { return value; }

  b8 in_int_range () const
  { return ((f64) (int16_t) to_int () == value); }

  b8 operator >  (const number_t &n) const { return value > n.to_real (); }
  b8 operator <  (const number_t &n) const { return n > *this; }
  b8 operator >= (const number_t &n) const { return !(*this < n); }
  b8 operator <= (const number_t &n) const { return !(*this > n); }

  const number_t &operator += (const number_t &n)
  {
    set_real (to_real () + n.to_real ());

    return *this;
  }

  protected:
  f64 value = 0.;
};

/* byte string */
struct UnsizedByteStr : UnsizedArrayOf <HBUINT8>
{
  hb_ubytes_t as_ubytes (u32 l) const
  { return hb_ubytes_t ((u8k *) this, l); }

  // encode 2-byte i32 (Dict/CharString) or 4-byte i32 (Dict)
  template <typename T, typename V>
  static b8 serialize_int (hb_serialize_context_t *c, op_code_t intOp, V value)
  {
    TRACE_SERIALIZE (this);

    HBUINT8 *p = c->allocate_size<HBUINT8> (1);
    if (unlikely (!p)) return_trace (false);
    *p = intOp;

    T *ip = c->allocate_size<T> (T::static_size);
    if (unlikely (!ip)) return_trace (false);
    return_trace (c->check_assign (*ip, value, HB_SERIALIZE_ERROR_INT_OVERFLOW));
  }

  template <typename V>
  static b8 serialize_int4 (hb_serialize_context_t *c, V value)
  { return serialize_int<HBINT32> (c, OpCode_longintdict, value); }

  template <typename V>
  static b8 serialize_int2 (hb_serialize_context_t *c, V value)
  { return serialize_int<HBINT16> (c, OpCode_shortint, value); }

  /* Defining null_size allows a Null object may be created. Should be safe because:
   * A descendent struct Dict uses a Null pointer to indicate a missing table,
   * checked before access.
   */
  DEFINE_SIZE_MIN(0);
};

/* A byte string associated with the current offset and an error condition */
struct byte_str_ref_t
{
  byte_str_ref_t ()
    : str () {}

  byte_str_ref_t (const hb_ubytes_t &str_, u32 offset_ = 0)
    : str (str_) { set_offset (offset_); }

  z0 reset (const hb_ubytes_t &str_, u32 offset_ = 0)
  {
    str = str_;
    set_offset (offset_);
  }

  u8k& operator [] (i32 i) {
    if (unlikely ((u32) (get_offset () + i) >= str.length))
    {
      set_error ();
      return Null (u8);
    }
    return str.arrayZ[get_offset () + i];
  }

  u8 head_unchecked () const { return str.arrayZ[get_offset ()]; }

  /* Conversion to hb_ubytes_t */
  operator hb_ubytes_t () const { return str.sub_array (get_offset ()); }

  hb_ubytes_t sub_array (u32 offset_, u32 len_) const
  { return str.sub_array (offset_, len_); }

  b8 avail (u32 count=1) const
  { return get_offset () + count <= str.length; }
  z0 inc (u32 count=1)
  {
    /* Automatically puts us in error if count is out-of-range. */
    set_offset (get_offset () + count);
  }

  /* We (ab)use ubytes backwards_length as a cursor (called offset),
   * as well as to store error condition. */

  u32 get_offset () const { return str.backwards_length; }
  z0 set_offset (u32 offset) { str.backwards_length = offset; }

  z0 set_error ()      { str.backwards_length = str.length + 1; }
  b8 in_error () const { return str.backwards_length > str.length; }

  u32 total_size () const { return str.length; }

  protected:
  hb_ubytes_t       str;
};

/* stack */
template <typename ELEM, i32 LIMIT>
struct cff_stack_t
{
  ELEM& operator [] (u32 i)
  {
    if (unlikely (i >= count))
    {
      set_error ();
      return Crap (ELEM);
    }
    return elements[i];
  }

  z0 push (const ELEM &v)
  {
    if (likely (count < LIMIT))
      elements[count++] = v;
    else
      set_error ();
  }
  ELEM &push ()
  {
    if (likely (count < LIMIT))
      return elements[count++];
    else
    {
      set_error ();
      return Crap (ELEM);
    }
  }

  ELEM& pop ()
  {
    if (likely (count > 0))
      return elements[--count];
    else
    {
      set_error ();
      return Crap (ELEM);
    }
  }
  z0 pop (u32 n)
  {
    if (likely (count >= n))
      count -= n;
    else
      set_error ();
  }

  const ELEM& peek ()
  {
    if (unlikely (count == 0))
    {
      set_error ();
      return Null (ELEM);
    }
    return elements[count - 1];
  }

  z0 unpop ()
  {
    if (likely (count < LIMIT))
      count++;
    else
      set_error ();
  }

  z0 clear () { count = 0; }

  b8 in_error () const { return (error); }
  z0 set_error ()      { error = true; }

  u32 get_count () const { return count; }
  b8 is_empty () const          { return !count; }

  hb_array_t<const ELEM> sub_array (u32 start, u32 length) const
  { return hb_array_t<const ELEM> (elements).sub_array (start, length); }

  private:
  b8 error = false;
  u32 count = 0;
  ELEM elements[LIMIT];
};

/* argument stack */
template <typename ARG=number_t>
struct arg_stack_t : cff_stack_t<ARG, 513>
{
  z0 push_int (i32 v)
  {
    ARG &n = S::push ();
    n.set_int (v);
  }

  z0 push_fixed (int32_t v)
  {
    ARG &n = S::push ();
    n.set_fixed (v);
  }

  z0 push_real (f64 v)
  {
    ARG &n = S::push ();
    n.set_real (v);
  }

  ARG& pop_num () { return this->pop (); }

  i32 pop_int ()  { return this->pop ().to_int (); }

  u32 pop_uint ()
  {
    i32 i = pop_int ();
    if (unlikely (i < 0))
    {
      i = 0;
      S::set_error ();
    }
    return (u32) i;
  }

  z0 push_longint_from_substr (byte_str_ref_t& str_ref)
  {
    push_int ((str_ref[0] << 24) | (str_ref[1] << 16) | (str_ref[2] << 8) | (str_ref[3]));
    str_ref.inc (4);
  }

  b8 push_fixed_from_substr (byte_str_ref_t& str_ref)
  {
    if (unlikely (!str_ref.avail (4)))
      return false;
    push_fixed ((int32_t)*(const HBUINT32*)&str_ref[0]);
    str_ref.inc (4);
    return true;
  }

  private:
  typedef cff_stack_t<ARG, 513> S;
};

/* an operator prefixed by its operands in a byte string */
struct op_str_t
{
  /* This used to have a hb_ubytes_t. Using a pointer and length
   * in a particular order, saves 8 bytes in this struct and more
   * in our parsed_cs_op_t subclass. */

  u8k *ptr = nullptr;

  op_code_t  op = OpCode_Invalid;

  uint8_t length = 0;
};

/* base of OP_SERIALIZER */
struct op_serializer_t
{
  protected:
  b8 copy_opstr (hb_serialize_context_t *c, const op_str_t& opstr) const
  {
    TRACE_SERIALIZE (this);

    u8 *d = c->allocate_size<u8> (opstr.length);
    if (unlikely (!d)) return_trace (false);
    /* Faster than hb_memcpy for small strings. */
    for (u32 i = 0; i < opstr.length; i++)
      d[i] = opstr.ptr[i];
    return_trace (true);
  }
};

template <typename VAL>
struct parsed_values_t
{
  z0 init ()
  {
    opStart = 0;
    values.init ();
  }
  z0 fini () { values.fini (); }

  z0 alloc (u32 n)
  {
    values.alloc (n, true);
  }

  z0 add_op (op_code_t op, const byte_str_ref_t& str_ref = byte_str_ref_t (), const VAL &v = VAL ())
  {
    VAL *val = values.push (v);
    val->op = op;
    auto arr = str_ref.sub_array (opStart, str_ref.get_offset () - opStart);
    val->ptr = arr.arrayZ;
    val->length = arr.length;
    opStart = str_ref.get_offset ();
  }

  b8 has_op (op_code_t op) const
  {
    for (const auto& v : values)
      if (v.op == op) return true;
    return false;
  }

  u32 get_count () const { return values.length; }
  const VAL &operator [] (u32 i) const { return values[i]; }

  u32       opStart;
  hb_vector_t<VAL>   values;
};

template <typename ARG=number_t>
struct interp_env_t
{
  interp_env_t () {}
  interp_env_t (const hb_ubytes_t &str_)
  {
    str_ref.reset (str_);
  }
  b8 in_error () const
  { return str_ref.in_error () || argStack.in_error (); }

  z0 set_error () { str_ref.set_error (); }

  op_code_t fetch_op ()
  {
    op_code_t  op = OpCode_Invalid;
    if (unlikely (!str_ref.avail ()))
      return OpCode_Invalid;
    op = (op_code_t) str_ref.head_unchecked ();
    str_ref.inc ();
    if (op == OpCode_escape) {
      if (unlikely (!str_ref.avail ()))
	return OpCode_Invalid;
      op = Make_OpCode_ESC (str_ref.head_unchecked ());
      str_ref.inc ();
    }
    return op;
  }

  const ARG& eval_arg (u32 i) { return argStack[i]; }

  ARG& pop_arg () { return argStack.pop (); }
  z0 pop_n_args (u32 n) { argStack.pop (n); }

  z0 clear_args () { pop_n_args (argStack.get_count ()); }

  byte_str_ref_t
		str_ref;
  arg_stack_t<ARG>
		argStack;
};

using num_interp_env_t =  interp_env_t<>;

template <typename ARG=number_t>
struct opset_t
{
  static z0 process_op (op_code_t op, interp_env_t<ARG>& env)
  {
    switch (op) {
      case OpCode_shortint:
	env.argStack.push_int ((int16_t)((env.str_ref[0] << 8) | env.str_ref[1]));
	env.str_ref.inc (2);
	break;

      case OpCode_TwoBytePosInt0: case OpCode_TwoBytePosInt1:
      case OpCode_TwoBytePosInt2: case OpCode_TwoBytePosInt3:
	env.argStack.push_int ((int16_t)((op - OpCode_TwoBytePosInt0) * 256 + env.str_ref[0] + 108));
	env.str_ref.inc ();
	break;

      case OpCode_TwoByteNegInt0: case OpCode_TwoByteNegInt1:
      case OpCode_TwoByteNegInt2: case OpCode_TwoByteNegInt3:
	env.argStack.push_int ((-(int16_t)(op - OpCode_TwoByteNegInt0) * 256 - env.str_ref[0] - 108));
	env.str_ref.inc ();
	break;

      default:
	/* 1-byte integer */
	if (likely ((OpCode_OneByteIntFirst <= op) && (op <= OpCode_OneByteIntLast)))
	{
	  env.argStack.push_int ((i32)op - 139);
	} else {
	  /* invalid unknown operator */
	  env.clear_args ();
	}
	break;
    }
  }
};

template <typename ENV>
struct interpreter_t
{
  interpreter_t (ENV& env_) : env (env_) {}
  ENV& env;
};

} /* namespace CFF */

#endif /* HB_CFF_INTERP_COMMON_HH */
