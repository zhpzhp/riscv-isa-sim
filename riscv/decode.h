// See LICENSE for license details.

#ifndef _RISCV_DECODE_H
#define _RISCV_DECODE_H

#if (-1 != ~0) || ((-1 >> 1) != -1)
# error spike requires a two''s-complement c++ implementation
#endif

#ifdef WORDS_BIGENDIAN
# error spike requires a little-endian host
#endif

#include <cstdint>
#include <string.h>
#include <strings.h>
#include "encoding.h"
#include "config.h"
#include "common.h"
#include "softfloat_types.h"
#include "specialize.h"
#include <cinttypes>

typedef int64_t sreg_t;
typedef uint64_t reg_t;

const int NXPR = 32;
const int NFPR = 32;
const int NVR = 32;
const int NCSR = 4096;

#define X_RA 1
#define X_SP 2

#define FP_RD_NE  0
#define FP_RD_0   1
#define FP_RD_DN  2
#define FP_RD_UP  3
#define FP_RD_NMM 4

#define FSR_RD_SHIFT 5
#define FSR_RD   (0x7 << FSR_RD_SHIFT)

#define FPEXC_NX 0x01
#define FPEXC_UF 0x02
#define FPEXC_OF 0x04
#define FPEXC_DZ 0x08
#define FPEXC_NV 0x10

#define FSR_AEXC_SHIFT 0
#define FSR_NVA  (FPEXC_NV << FSR_AEXC_SHIFT)
#define FSR_OFA  (FPEXC_OF << FSR_AEXC_SHIFT)
#define FSR_UFA  (FPEXC_UF << FSR_AEXC_SHIFT)
#define FSR_DZA  (FPEXC_DZ << FSR_AEXC_SHIFT)
#define FSR_NXA  (FPEXC_NX << FSR_AEXC_SHIFT)
#define FSR_AEXC (FSR_NVA | FSR_OFA | FSR_UFA | FSR_DZA | FSR_NXA)

#define insn_length(x) \
  (((x) & 0x03) < 0x03 ? 2 : \
   ((x) & 0x1f) < 0x1f ? 4 : \
   ((x) & 0x3f) < 0x3f ? 6 : \
   8)
#define MAX_INSN_LENGTH 8
#define PC_ALIGN 2

typedef uint64_t insn_bits_t;
class insn_t
{
public:
  insn_t() = default;
  insn_t(insn_bits_t bits) : b(bits) {}
  insn_bits_t bits() { return b; }
  int length() { return insn_length(b); }
  int64_t i_imm() { return int64_t(b) >> 20; }
  int64_t s_imm() { return x(7, 5) + (xs(25, 7) << 5); }
  int64_t sb_imm() { return (x(8, 4) << 1) + (x(25,6) << 5) + (x(7,1) << 11) + (imm_sign() << 12); }
  int64_t u_imm() { return int64_t(b) >> 12 << 12; }
  int64_t uj_imm() { return (x(21, 10) << 1) + (x(20, 1) << 11) + (x(12, 8) << 12) + (imm_sign() << 20); }
  uint64_t rd() { return x(7, 5); }
  uint64_t rs1() { return x(15, 5); }
  uint64_t rs2() { return x(20, 5); }
  uint64_t rs3() { return x(27, 5); }
  uint64_t rm() { return x(12, 3); }
  uint64_t csr() { return x(20, 12); }

  int64_t rvc_imm() { return x(2, 5) + (xs(12, 1) << 5); }
  int64_t rvc_zimm() { return x(2, 5) + (x(12, 1) << 5); }
  int64_t rvc_addi4spn_imm() { return (x(6, 1) << 2) + (x(5, 1) << 3) + (x(11, 2) << 4) + (x(7, 4) << 6); }
  int64_t rvc_addi16sp_imm() { return (x(6, 1) << 4) + (x(2, 1) << 5) + (x(5, 1) << 6) + (x(3, 2) << 7) + (xs(12, 1) << 9); }
  int64_t rvc_lwsp_imm() { return (x(4, 3) << 2) + (x(12, 1) << 5) + (x(2, 2) << 6); }
  int64_t rvc_ldsp_imm() { return (x(5, 2) << 3) + (x(12, 1) << 5) + (x(2, 3) << 6); }
  int64_t rvc_swsp_imm() { return (x(9, 4) << 2) + (x(7, 2) << 6); }
  int64_t rvc_sdsp_imm() { return (x(10, 3) << 3) + (x(7, 3) << 6); }
  int64_t rvc_lw_imm() { return (x(6, 1) << 2) + (x(10, 3) << 3) + (x(5, 1) << 6); }
  int64_t rvc_ld_imm() { return (x(10, 3) << 3) + (x(5, 2) << 6); }
  int64_t rvc_j_imm() { return (x(3, 3) << 1) + (x(11, 1) << 4) + (x(2, 1) << 5) + (x(7, 1) << 6) + (x(6, 1) << 7) + (x(9, 2) << 8) + (x(8, 1) << 10) + (xs(12, 1) << 11); }
  int64_t rvc_b_imm() { return (x(3, 2) << 1) + (x(10, 2) << 3) + (x(2, 1) << 5) + (x(5, 2) << 6) + (xs(12, 1) << 8); }
  int64_t rvc_simm3() { return x(10, 3); }
  uint64_t rvc_rd() { return rd(); }
  uint64_t rvc_rs1() { return rd(); }
  uint64_t rvc_rs2() { return x(2, 5); }
  uint64_t rvc_rs1s() { return 8 + x(7, 3); }
  uint64_t rvc_rs2s() { return 8 + x(2, 3); }

  int64_t rvv_imm() { return xs(20, 8); }
  int64_t rvv_load_imm() { return x(27, 5); }
  int64_t rvv_store_imm() { return x(7, 5); }
  uint64_t rvv_mask() { return x(12, 2); }
private:
  insn_bits_t b;
  uint64_t x(int lo, int len) { return (b >> lo) & ((insn_bits_t(1) << len)-1); }
  uint64_t xs(int lo, int len) { return int64_t(b) << (64-lo-len) >> (64-len); }
  uint64_t imm_sign() { return xs(63, 1); }
};

template <class T, size_t N, bool zero_reg>
class regfile_t
{
public:
  void write(size_t i, T value)
  {
    if (!zero_reg || i != 0)
      data[i] = value;
  }
  const T& operator [] (size_t i) const
  {
    return data[i];
  }
private:
  T data[N];
};

// helpful macros, etc
#define MMU (*p->get_mmu())
#define STATE (*p->get_state())
#define READ_REG(reg) STATE.XPR[reg]
#define READ_FREG(reg) STATE.FPR[reg]
#define RS1 READ_REG(insn.rs1())
#define RS2 READ_REG(insn.rs2())
#define WRITE_RD(value) WRITE_REG(insn.rd(), value)

#ifndef RISCV_ENABLE_COMMITLOG
# define WRITE_REG(reg, value) STATE.XPR.write(reg, value)
# define WRITE_FREG(reg, value) DO_WRITE_FREG(reg, freg(value))
#else
# define WRITE_REG(reg, value) ({ \
    reg_t wdata = (value); /* value may have side effects */ \
    STATE.log_reg_write = (commit_log_reg_t){(reg) << 1, {wdata, 0}}; \
    STATE.XPR.write(reg, wdata); \
  })
# define WRITE_FREG(reg, value) ({ \
    freg_t wdata = freg(value); /* value may have side effects */ \
    STATE.log_reg_write = (commit_log_reg_t){((reg) << 1) | 1, wdata}; \
    DO_WRITE_FREG(reg, wdata); \
  })
#endif

// RVC macros
#define WRITE_RVC_RS1S(value) WRITE_REG(insn.rvc_rs1s(), value)
#define WRITE_RVC_RS2S(value) WRITE_REG(insn.rvc_rs2s(), value)
#define WRITE_RVC_FRS2S(value) WRITE_FREG(insn.rvc_rs2s(), value)
#define RVC_RS1 READ_REG(insn.rvc_rs1())
#define RVC_RS2 READ_REG(insn.rvc_rs2())
#define RVC_RS1S READ_REG(insn.rvc_rs1s())
#define RVC_RS2S READ_REG(insn.rvc_rs2s())
#define RVC_FRS2 READ_FREG(insn.rvc_rs2())
#define RVC_FRS2S READ_FREG(insn.rvc_rs2s())
#define RVC_SP READ_REG(X_SP)

// FPU macros
#define FRS1 READ_FREG(insn.rs1())
#define FRS2 READ_FREG(insn.rs2())
#define FRS3 READ_FREG(insn.rs3())
#define dirty_fp_state (STATE.mstatus |= MSTATUS_FS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define dirty_ext_state (STATE.mstatus |= MSTATUS_XS | (xlen == 64 ? MSTATUS64_SD : MSTATUS32_SD))
#define DO_WRITE_FREG(reg, value) (STATE.FPR.write(reg, value), dirty_fp_state)
#define WRITE_FRD(value) WRITE_FREG(insn.rd(), value)
 
#define SHAMT (insn.i_imm() & 0x3F)
#define BRANCH_TARGET (pc + insn.sb_imm())
#define JUMP_TARGET (pc + insn.uj_imm())
#define RM ({ int rm = insn.rm(); \
              if(rm == 7) rm = STATE.frm; \
              if(rm > 4) throw trap_illegal_instruction(0); \
              rm; })

#define get_field(reg, mask) (((reg) & (decltype(reg))(mask)) / ((mask) & ~((mask) << 1)))
#define set_field(reg, mask, val) (((reg) & ~(decltype(reg))(mask)) | (((decltype(reg))(val) * ((mask) & ~((mask) << 1))) & (decltype(reg))(mask)))

#define require(x) if (unlikely(!(x))) throw trap_illegal_instruction(0)
#define require_privilege(p) require(STATE.prv >= (p))
#define require_rv64 require(xlen == 64)
#define require_rv32 require(xlen == 32)
#define require_extension(s) require(p->supports_extension(s))
#define require_fp require((STATE.mstatus & MSTATUS_FS) != 0)
#define require_accelerator require((STATE.mstatus & MSTATUS_XS) != 0)

#define set_fp_exceptions ({ if (softfloat_exceptionFlags) { \
                               dirty_fp_state; \
                               STATE.fflags |= softfloat_exceptionFlags; \
                             } \
                             softfloat_exceptionFlags = 0; })

#define sext32(x) ((sreg_t)(int32_t)(x))
#define zext32(x) ((reg_t)(uint32_t)(x))
#define sext_xlen(x) (((sreg_t)(x) << (64-xlen)) >> (64-xlen))
#define zext_xlen(x) (((reg_t)(x) << (64-xlen)) >> (64-xlen))

#define set_pc(x) \
  do { if (unlikely(((x) & 2)) && !p->supports_extension('C')) \
         throw trap_instruction_address_misaligned(x); \
       npc = sext_xlen(x); \
     } while(0)

#define set_pc_and_serialize(x) \
  do { reg_t __npc = (x); \
       set_pc(__npc); /* check alignment */ \
       npc = PC_SERIALIZE_AFTER; \
       STATE.pc = __npc; \
     } while(0)

/* Sentinel PC values to serialize simulator pipeline */
#define PC_SERIALIZE_BEFORE 3
#define PC_SERIALIZE_AFTER 5
#define invalid_pc(pc) ((pc) & 1)

/* Convenience wrappers to simplify softfloat code sequences */
#define isBoxedF16(r) (isBoxedF32(r) && ((uint16_t)((r.v[0] >> 16) + 1) == 0))
#define unboxF16(r) (isBoxedF16(r) ? (uint16_t)r.v[0] : defaultNaNF16UI)
#define isBoxedF32(r) (isBoxedF64(r) && ((uint32_t)((r.v[0] >> 32) + 1) == 0))
#define unboxF32(r) (isBoxedF32(r) ? (uint32_t)r.v[0] : defaultNaNF32UI)
#define isBoxedF64(r) ((r.v[1] + 1) == 0)
#define unboxF64(r) (isBoxedF64(r) ? r.v[0] : defaultNaNF64UI)
#define unbox(r, w) (vIs16(w) ? unboxF16(r) : vIs32(w) ? unboxF32(r) : vIs64(w) ? unboxF64(r) : throw trap_illegal_instruction(0))
typedef float128_t freg_t;
inline float16_t f16(uint16_t v) { return { v }; }
inline float32_t f32(uint32_t v) { return { v }; }
inline float64_t f64(uint64_t v) { return { v }; }
inline float16_t f16(freg_t r) { return f16(unboxF16(r)); }
inline float32_t f32(freg_t r) { return f32(unboxF32(r)); }
inline float64_t f64(freg_t r) { return f64(unboxF64(r)); }
inline float128_t f128(freg_t r) { return r; }
inline freg_t freg(float16_t f) { return { ((uint64_t)-1 << 16) | f.v, (uint64_t)-1 }; }
inline freg_t freg(float32_t f) { return { ((uint64_t)-1 << 32) | f.v, (uint64_t)-1 }; }
inline freg_t freg(float64_t f) { return { f.v, (uint64_t)-1 }; }
inline freg_t freg(float128_t f) { return f; }
#define F16_SIGN ((uint16_t)1 << 15)
#define F32_SIGN ((uint32_t)1 << 31)
#define F64_SIGN ((uint64_t)1 << 63)
#define fsgnj32(a, b, n, x) \
  f32((f32(a).v & ~F32_SIGN) | ((((x) ? f32(a).v : (n) ? F32_SIGN : 0) ^ f32(b).v) & F32_SIGN))
#define fsgnj64(a, b, n, x) \
  f64((f64(a).v & ~F64_SIGN) | ((((x) ? f64(a).v : (n) ? F64_SIGN : 0) ^ f64(b).v) & F64_SIGN))

#define isNaNF128(x) isNaNF128UI(x.v[1], x.v[0])
inline float128_t defaultNaNF128()
{
  float128_t nan;
  nan.v[1] = defaultNaNF128UI64;
  nan.v[0] = defaultNaNF128UI0;
  return nan;
}
inline freg_t fsgnj128(freg_t a, freg_t b, bool n, bool x)
{
  a.v[1] = (a.v[1] & ~F64_SIGN) | (((x ? a.v[1] : n ? F64_SIGN : 0) ^ b.v[1]) & F64_SIGN);
  return a;
}
inline freg_t f128_negate(freg_t a)
{
  a.v[1] ^= F64_SIGN;
  return a;
}

/* Vector extension wrappers */
typedef union velt {
  reg_t x;
  freg_t f;
} velt_t;
inline velt_t velt(float16_t f) { velt_t elt; elt.f = {((uint64_t)-1 << 16) | f.v, (uint64_t)-1 }; return elt; }
inline velt_t velt(float32_t f) { velt_t elt; elt.f = {((uint64_t)-1 << 32) | f.v, (uint64_t)-1 }; return elt; }
inline velt_t velt(float64_t f) { velt_t elt; elt.f = {f.v, (uint64_t)-1 }; return elt; }
inline velt_t velt(float128_t f) { velt_t elt; elt.f = f; return elt; }
inline velt_t velt(bool x) { return { .x=x }; }
inline velt_t velt(uint8_t x) { return { .x=x }; }
inline velt_t velt(uint16_t x) { return { .x=x }; }
inline velt_t velt(uint32_t x) { return { .x=x }; }
inline velt_t velt(uint64_t x) { return { .x=x }; }
inline velt_t velt(int8_t x) { return { .x=(uint8_t)x }; }
inline velt_t velt(int16_t x) { return { .x=(uint16_t)x }; }
inline velt_t velt(int32_t x) { return { .x=(uint32_t)x }; }
inline velt_t velt(int64_t x) { return { .x=(uint64_t)x }; }
inline velt_t velt(velt_t v) { return v; }
typedef uint64_t vtype_t;
static const vtype_t UINT = 0;
static const vtype_t INT = 1;
static const vtype_t FP = 3;
static const vtype_t W128 = 48;
static const vtype_t W64 = 32;
static const vtype_t W32 = 24;
static const vtype_t W16 = 16;
static const vtype_t W8 = 8;
static const vtype_t SCALAR = 0;
static const vtype_t VECTOR = 4;

#define VL_T(t) ( vIsScalar(t) ? 1 : STATE.vl )
#define VL VL_T(TRD)
#define VL_LOOP_FULL_T(t) for(size_t eidx = 0; eidx < VL_T(t); eidx++) {
#define VL_LOOP_T(t) VL_LOOP_FULL_T(t) \
  if(!VM) continue;
#define VL_LOOP VL_LOOP_T(TRD)
#define VL_LOOP_FULL VL_LOOP_FULL_T(TRD)
#define VM ((insn.rvv_mask() == 0x3) ? READ_VREG(1).x & 0x1 : \
           (insn.rvv_mask() == 0x2) ? !(READ_VREG(1).x & 0x1) : true)

inline reg_t vregcfg(reg_t vregmax, reg_t vregd, reg_t vregq) {
  return (vregq & 0x7) << 9 | (vregd & 0xf) << 5 | ((vregmax-1) & 0x1f);
}

inline reg_t vfastcfg(reg_t imm, reg_t old_cfg) {
  reg_t prefix = (imm & 0x60) >> 5;
  reg_t vregd = (imm & 0x07);
  reg_t vregq = (imm & 0x18) >> 3;
  if(prefix == 3)
    return (old_cfg & 0xf000ffff) | (vregcfg(32, vregd, vregq) << 16);
  if(prefix == 2)
    return (old_cfg & 0xf000ffff) | (vregcfg(16, vregd, vregq) << 16);
  prefix = (imm & 0x70) >> 4;
  vregq = (imm & 0x08) >> 3;
  if(prefix == 3)
    return (old_cfg & 0xf000ffff) | (vregcfg(10, vregd, vregq) << 16);
  if(prefix == 2)
    return (old_cfg & 0xf000ffff) | (vregcfg(8, vregd, vregq) << 16);
  prefix = (imm & 0x78) >> 3;
  vregq = 0;
  if(prefix == 3)
    return (old_cfg & 0xf000ffff) | (vregcfg(6, vregd, vregq) << 16);
  if(prefix == 2)
    return (old_cfg & 0xf000ffff) | (vregcfg(5, vregd, vregq) << 16);
  prefix = (imm & 0x7C) >> 2;
  vregd = (imm & 0x03);
  if(prefix == 3)
    return (old_cfg & 0xf000ffff) | (vregcfg(4, vregd, vregq) << 16);
  if(prefix == 2)
    return (old_cfg & 0xf000ffff) | (vregcfg(3, vregd, vregq) << 16);
  if(prefix == 1)
    return (old_cfg & 0xf000ffff) | (vregcfg(2, vregd, vregq) << 16);
  if(prefix == 0)
    return (old_cfg & 0xf000ffff) | (vregcfg(1, vregd, vregq) << 16);
  return 0; //TODO: Do we trap on bad configs or not?
}

#define require_vec require_accelerator
#define dirty_vec_state dirty_ext_state
#ifndef RISCV_ENABLE_COMMITLOG
#define WRITE_VREG_ELEM(reg, elem, value) ({ STATE.VR[elem].write(reg, value); })
#else
#define WRITE_VREG_ELEM(reg, elem, value) ({ \
    if(vIsFP(VTY(reg))) \
      fprintf(stderr, "Writing V%lu[%lu]=%lx:%lx\n",reg,elem,value.f.v[1],value.f.v[0]); \
    else \
      fprintf(stderr, "Writing V%lu[%lu]=%lx\n",reg,elem,value.x); \
    STATE.VR[elem].write(reg, value);})
#endif
#define WRITE_VREG(reg, value) ({ \
    vIsScalar(VTY(reg)) ? WRITE_VREG_ELEM(reg, (uint64_t)0, value) : WRITE_VREG_ELEM(reg, eidx, value); })
#define WRITE_VRD(v1,v2,v3,value) WRITE_VREG(insn.rd(), DYN_TRUNCATE(TRD, TIN(v1,v2,v3), value))
#define READ_VREG_ELEM(reg, elem) ( STATE.VR[elem][reg] )
#define READ_VREG(reg) ( vIsScalar(VTY(reg)) ? READ_VREG_ELEM(reg, 0) : READ_VREG_ELEM(reg, eidx) )

#define VTYPE(S, R, W) ( (S << 11) | (R << 6) | W)
#define UINT8 VTYPE(VECTOR, UINT, W8)
#define INT8 VTYPE(VECTOR, INT, W8)
#define UINT16 VTYPE(VECTOR, UINT, W16)
#define INT16 VTYPE(VECTOR, INT, W16)
#define UINT32 VTYPE(VECTOR, UINT, W32)
#define INT32 VTYPE(VECTOR, INT, W32)
#define UINT64 VTYPE(VECTOR, UINT, W64)
#define INT64 VTYPE(VECTOR, INT, W64)
#define FP16 VTYPE(VECTOR, FP, W16)
#define FP32 VTYPE(VECTOR, FP, W32)
#define FP64 VTYPE(VECTOR, FP, W64)
#define VTY(reg) (STATE.vtype[reg])
#define VEW(ty) (ty & 0x3f)
#define VEREP(ty) ((ty >> 6) & 0x1f)
#define VESHAPE(ty) ((ty >> 11) & 0x1f)
#define vIsInt(t) ( VEREP(t) == INT )
#define vIsUInt(t) ( VEREP(t) == UINT )
#define vIsFP(t) ( VEREP(t) == FP )
#define vIs128(t) ( VEW(t) == W128 )
#define vIs64(t) ( VEW(t) == W64 )
#define vIs32(t) ( VEW(t) == W32 )
#define vIs16(t) ( VEW(t) == W16 )
#define vIs8(t) ( VEW(t) == W8 )
#define vIsScalar(t) ( VESHAPE(t) == SCALAR )
#define vIsVector(t) ( VESHAPE(t) == VECTOR )

#define TRD VTY(insn.rd())
#define TRS1 VTY(insn.rs1())
#define TRS2 VTY(insn.rs2())
#define TRS3 VTY(insn.rs3())
#define TIN(v1,v2,v3) INTER_TYPE(v1, TRS1, v2, TRS2, v3, TRS3)
#define TIN_1 TIN(1, 0, 0)
#define TIN_12 TIN(1, 1, 0)
#define TIN_123 TIN(1, 1, 1)
#define TIN_3 TIN(0, 0, 1)
#define VS1 (READ_VREG(insn.rs1())) // TODO: why can't we remove these
#define VS2 (READ_VREG(insn.rs2()))
#define VS3 (READ_VREG(insn.rs3()))
#define VS1_12 DYN_EXTEND(TIN_12, TRS1, READ_VREG(insn.rs1()))
#define VS2_12 DYN_EXTEND(TIN_12, TRS2, READ_VREG(insn.rs2()))
#define VS3_3 DYN_EXTEND(TIN_3, TRS3, READ_VREG(insn.rs3()))
#define VS1_123 DYN_EXTEND(TIN_123, TRS1, READ_VREG(insn.rs1()))
#define VS2_123 DYN_EXTEND(TIN_123, TRS2, READ_VREG(insn.rs2()))
#define VS3_123 DYN_EXTEND(TIN_123, TRS3, READ_VREG(insn.rs3()))

// Assumes you have only types defined in base
#define MASK(w) (vIs8(w) ? 0xfful : vIs16(w) ? 0xfffful : vIs32(w) ? 0xfffffffful : 0xfffffffffffffffful)
//TODO: Last return should be a sign extension to 128 or illegals inst
// Assumes you never sext_to 8bits which is true in the current spec
#define sext_to(v, w) (velt(vIs16(w) ? (sreg_t)(int16_t)(v.x) : vIs32(w) ? (sreg_t)(int32_t)(v.x) : vIs64(w) ? (sreg_t)(int64_t)(v.x) : v.x))
#define trunc_to(v, w) (velt(v.x & MASK(w)))

// Assumes no 128 bit fregs
#define WRITE_FP_VRD(val, OP) (WRITE_VREG(insn.rd(), (VEW(TRD) > VEW(OP) ? velt(freg(val)) : VEW(TRD) == VEW(OP) ? velt(val) : throw trap_illegal_instruction(0) )))
#define VFS1(OP) (VEW(TRS1)>VEW(OP) ? unbox(VS1.f, OP) : VEW(TRS1) == VEW(OP) ? VS1.f.v[0] : throw trap_illegal_instruction(0))
#define VFS2(OP) (VEW(TRS2)>VEW(OP) ? unbox(VS2.f, OP) : VEW(TRS2) == VEW(OP) ? VS2.f.v[0] : throw trap_illegal_instruction(0))

#define UIOP (TRS1)
#define BIOP (std::max(VEW(TRS1), VEW(TRS2)))
#define TIOP (std::max(VEW(TRS1), std::max(VEW(TRS2), VEW(TRS3))))
#define WRITE_INT_VRD(val, OP) (WRITE_VREG(insn.rd(), (VEW(TRD) > VEW(OP) ? sext_to(velt(val), OP) : VEW(TRD) == VEW(OP) ? velt(val) : trunc_to(velt(val), OP))))
#define VIS1(OP) (VEW(TRS1)>VEW(OP) ? trunc_to(VS1, OP).x : VEW(TRS1) == VEW(OP) ? velt(VS1).x : sext_to(VS1, OP).x)
#define VIS2(OP) (VEW(TRS2)>VEW(OP) ? trunc_to(VS2, OP).x : VEW(TRS2) == VEW(OP) ? velt(VS2).x : sext_to(VS2, OP).x)
#define VIS3(OP) (VEW(TRS3)>VEW(OP) ? trunc_to(VS3, OP).x : VEW(TRS3) == VEW(OP) ? velt(VS3).x : sext_to(VS3, OP).x)
#define VIMM (velt(insn.rvv_imm()).x)

// Type permotion and demotion
#define INTER_TYPE(v1, t1, v2, t2, v3, t3) ({ \
    vtype_t oT = t1; \
    if(v1 && vIsFP(t1) && (!v2 || vIsFP(t2)) && (!v3 || vIsFP(t3))) \
      oT = VTYPE(0, FP, W128);  \
    oT; })

#define DYN_EXTEND(outT, inT, val) ({ \
    velt_t outV = velt(val); \
    if(VEW(inT) < VEW(outT)) \
      outV = vIsInt(inT) ? velt(sext_xlen(outV.x)) : (vIsUInt(inT) ? velt(zext_xlen(outV.x)) : \
          (vIsFP(inT) ? (vIs64(inT) ? velt(f64_to_f128(f64(outV.f))) : (vIs32(inT) ? velt(f32_to_f128(f32(outV.f))) : (vIs16(inT) ? velt(f16_to_f128(f16(outV.f))) : \
            throw trap_illegal_instruction(0)))) : \
            throw trap_illegal_instruction(0))); \
    outV; })

// Forced conversion
#define DYN_CONVERT(outT, val) ({ \
    velt_t outV = vIsFP(outT) ? (vIs64(outT) ? velt(f64(val)) : (vIs32(outT) ? velt(f32(val)) : (vIs16(outT) ? velt(f16(val)) : throw trap_illegal_instruction(0)))) : \
                 vIsInt(outT) ? velt(sext_xlen(val)) : (vIsUInt(outT) ? velt(zext_xlen(val)) : throw trap_illegal_instruction(0)); \
    outV; })

#define DYN_TRUNCATE(outT, inT, val) ({ \
    velt_t outV = velt(val); \
    if(VEW(inT) > VEW(outT)) \
      if(vIsFP(inT) && vIsFP(outT)) \
        outV = vIs64(outT) ? velt(f128_to_f64(outV.f)) : (vIs32(outT) ? velt(f128_to_f32(outV.f)) : (vIs16(outT) ? velt(f128_to_f16(outV.f)) : \
            throw trap_illegal_instruction(0))); \
      else \
        outV.x = vIs64(outT) ? 0xffffffffffffffff & outV.x : (vIs32(outT) ? 0xffffffff & outV.x : (vIs16(outT) ? 0xffff & outV.x : (vIs8(outT) ? 0xff & outV.x : \
            throw trap_illegal_instruction(0)))); \
    outV; })

// Catch invalid fp functions
#define f128_err(f1, f2) ({ throw trap_illegal_instruction(0); velt((uint64_t) 0); })
// Operations
#define DYN_OP2(op, ta, a, tb, b) ({ velt_t outV; \
  switch(VEREP(ta)) { \
  case INT: case UINT: \
    switch(VEREP(tb)) { \
    case INT: case UINT: outV = velt(op(a.x, b.x)); break;\
    default: throw trap_illegal_instruction(0); } break; \
  case FP: \
    switch(VEREP(tb)) { \
    case FP: outV = velt(f128_##op(a.f, b.f)); break;\
    default: throw trap_illegal_instruction(0); } break; \
  default: throw trap_illegal_instruction(0); } \
  outV; })

#define DYN_OP3(op, ta, a, tb, b, tc, c) ({ velt_t outV; \
  switch(VEREP(ta)) { \
  case INT: case UINT: \
    switch(VEREP(tb)) { \
    case INT: case UINT: \
      switch(VEREP(tc)) { \
      case INT: case UINT: outV = velt(op(a.x, b.x, c.x)); break;\
      default: throw trap_illegal_instruction(0); } break; \
    default: throw trap_illegal_instruction(0); } break; \
  case FP: \
    switch(VEREP(tb)) { \
    case FP: \
      switch(VEREP(tc)) { \
      case FP: outV = velt(f128_##op(a.f, b.f, c.f)); break;\
      default: throw trap_illegal_instruction(0); } break; \
    default: throw trap_illegal_instruction(0); } break; \
  default: throw trap_illegal_instruction(0); } \
  outV; })

#define DYN_LOAD(a) ({ velt_t outV; \
    if(vIs128(TRD)) \
      outV = DYN_LOAD_ST(a, 16); \
    else if(vIs64(TRD)) \
      outV = DYN_LOAD_ST(a, 8); \
    else if (vIs32(TRD)) \
      outV = DYN_LOAD_ST(a, 4); \
    else if (vIs16(TRD)) \
      outV = DYN_LOAD_ST(a, 2); \
    else if(vIs8(TRD)) \
      outV = DYN_LOAD_ST(a, 1); \
    else \
      throw trap_illegal_instruction(0);\
    outV; \
    })
#define DYN_LOAD_ST(a, st) ({ velt_t outV; \
    outV = DYN_LOAD_ST_TY(TRD, a+eidx*st); \
    outV; \
    })
#define DYN_LOAD_ST_TY(ta, a) ( \
  vIsFP(ta) ? (vIs64(ta) ? velt(f64(MMU.load_uint64(a))) : vIs32(ta) ? velt(f32(MMU.load_uint32(a))) : vIs16(ta) ? velt(f16(MMU.load_uint16(a))) : vIs128(ta) ? velt((float128_t){MMU.load_uint64(a), MMU.load_uint64(a+8)}) : throw trap_illegal_instruction(0)) : \
    (vIsUInt(ta) ? (vIs64(ta) ? velt(MMU.load_uint64(a)) : vIs32(ta) ? velt(MMU.load_uint32(a)) : vIs16(ta) ? velt(MMU.load_uint16(a)) : vIs8(ta) ? velt(MMU.load_uint8(a)) : throw trap_illegal_instruction(0)) : \
    (vIsInt(ta) ? (vIs64(ta) ? velt(MMU.load_int64(a)) : vIs32(ta) ? velt(MMU.load_int32(a)) : vIs16(ta) ? velt(MMU.load_int16(a)) : vIs8(ta) ? velt(MMU.load_int8(a)) : throw trap_illegal_instruction(0)) : \
     throw trap_illegal_instruction(0))) )
//Only useable for 64bit or smaller
#define DYN_VALUE(ta, a) ( \
    vIsInt(ta) || vIsUInt(ta) ? a.x : (vIsFP(ta) ? a.f.v[0] : throw trap_illegal_instruction(0)) \
    )
#define DYN_STORE(a, tb, b) ({ \
    if(vIs128(tb)) \
      DYN_STORE_ST(a, 16, tb, b); \
    else if(vIs64(tb)) \
      DYN_STORE_ST(a, 8, tb, b); \
    else if (vIs32(tb)) \
      DYN_STORE_ST(a, 4, tb, b); \
    else if (vIs16(tb)) \
      DYN_STORE_ST(a, 2, tb, b); \
    else if(vIs8(tb)) \
      DYN_STORE_ST(a, 1, tb, b); \
    else \
      throw trap_illegal_instruction(0);\
    })
#define DYN_STORE_ST(a, st, tb, b) ({ \
    if(vIs128(tb)) { \
      if(!vIsFP(tb)) throw trap_illegal_instruction(0); \
      DYN_STORE_ST_TY(a, st, float128, b.f); \
    } else if(vIs64(tb)) \
      DYN_STORE_ST_TY(a, st, uint64, DYN_VALUE(tb, b)); \
    else if (vIs32(tb)) \
      DYN_STORE_ST_TY(a, st, uint32, DYN_VALUE(tb, b)); \
    else if (vIs16(tb)) \
      DYN_STORE_ST_TY(a, st, uint16, DYN_VALUE(tb, b)); \
    else if(vIs8(tb)) { \
      if(!(vIsInt(tb) || vIsUInt(tb))) throw trap_illegal_instruction(0); \
      DYN_STORE_ST_TY(a, st, uint8, DYN_VALUE(tb, b)); \
    } else \
      throw trap_illegal_instruction(0);\
    })
#define DYN_STORE_ST_TY(a, st, func, b) ({ \
    (MMU.store_ ## func(a+eidx*st, b)); \
    })

//TODO: vinsert and vextract should use integer truncate rules?


// Infix helpers
#define vecadd(a, b) (a + b)
#define vecand(a, b) (a & b)
#define vecor(a, b) (a | b)
#define vecxor(a, b) (a ^ b)
#define vecsub(a, b) (a - b)
#define vecdiv(a, b) (a / b)
#define vecmul(a, b) (a * b)
#define vecrem(a, b) (a % b)
#define veceq(a, b) (a == b)
#define vecne(a, b) (a != b)
#define veclt(a, b) (a < b)
#define vecge(a, b) (a >= b)
#define vecsll(a, b) ((a) << (b))
#define vecsrl(a, b) ((a) >> (b))
#define vecmax(a, b) (a > b ? a : b)
#define vecmin(a, b) (a < b ? a : b)
// Redirect for INT and UINT
#define mulAdd(a, b, c) ( a * b + c )
#define mulSub(a, b, c) ( a * b + (-c) )
#define negMulAdd(a, b, c) ( (-a) * b + (-c) )
#define negMulSub(a, b, c) ( (-a) * b + c )
// f128 helpers
#define f128_mulSub(a, b, c) ( f128_mulAdd(a, b, f128_negate(c)) )
#define f128_negMulAdd(a, b, c) ( f128_mulAdd(f128_negate(a), b, f128_negate(c)) )
#define f128_negMulSub(a, b, c) ( f128_mulAdd(f128_negate(a), b, c) )
#define f128_vecadd(a, b) (f128_add(a, b))
#define f128_vecsub(a, b) (f128_sub(a, b))
#define f128_vecmul(a, b) (f128_mul(a, b))
#define f128_vecdiv(a, b) (f128_div(a, b))
#define f128_vecrem(a, b) (f128_rem(a, b))
#define f128_veceq(a, b) (f128_eq(a, b))
#define f128_vecne(a, b) (!f128_eq(a, b))
#define f128_veclt(a, b) (f128_lt(a, b))
#define f128_vecge(a, b) (f128_le(b, a))
#define f128_vecsll(a, b) (f128_err(a, b))
#define f128_vecsrl(a, b) (f128_err(a, b))
#define f128_vecand(a, b) (f128_err(a, b))
#define f128_vecor(a, b) (f128_err(a, b))
#define f128_vecxor(a, b) (f128_err(a, b))
#define f128_vecmax(a, b) ({ \
    bool greater = f128_lt_quiet(b, a) || (f128_eq(b, a) && (f128(b).v[1] & F64_SIGN));\
    (isNaNF128(a) && isNaNF128(b)) ? defaultNaNF128() : \
    (greater || isNaNF128(b) ? a : b); \
    })
#define f128_vecmin(a, b) ({ \
    bool less = f128_lt_quiet(a, b) || (f128_eq(a, b) && (f128(a).v[1] & F64_SIGN));\
    (isNaNF128(a) && isNaNF128(b)) ? defaultNaNF128() : \
    (less || isNaNF128(b) ? a : b); \
    })
#define DYN_ADD(ta, a, tb, b) DYN_OP2(vecadd, ta, a ## _12, tb, b ## _12)
#define DYN_ADDI(ta, a, b) DYN_OP2(vecadd, ta, a ## _12, ta, b)
#define DYN_AND(ta, a, tb, b) DYN_OP2(vecand, ta, a ## _12, tb, b ## _12)
#define DYN_ANDI(ta, a, b) DYN_OP2(vecand, ta, a ## _12, ta, b)
#define DYN_OR(ta, a, tb, b) DYN_OP2(vecor, ta, a ## _12, tb, b ## _12)
#define DYN_ORI(ta, a, b) DYN_OP2(vecor, ta, a ## _12, ta, b)
#define DYN_XOR(ta, a, tb, b) DYN_OP2(vecxor, ta, a ## _12, tb, b ## _12)
#define DYN_XORI(ta, a, b) DYN_OP2(vecxor, ta, a ## _12, ta, b)
#define DYN_DIV(ta, a, tb, b) DYN_OP2(vecdiv, ta, a ## _12, tb, b ## _12)
#define DYN_MUL(ta, a, tb, b) DYN_OP2(vecmul, ta, a ## _12, tb, b ## _12)
#define DYN_REM(ta, a, tb, b) DYN_OP2(vecrem, ta, a ## _12, tb, b ## _12)
#define DYN_SEQ(ta, a, tb, b) DYN_OP2(veceq, ta, a ## _12, tb, b ## _12)
#define DYN_SNE(ta, a, tb, b) DYN_OP2(vecne, ta, a ## _12, tb, b ## _12)
#define DYN_SLT(ta, a, tb, b) DYN_OP2(veclt, ta, a ## _12, tb, b ## _12)
#define DYN_SGE(ta, a, tb, b) DYN_OP2(vecge, ta, a ## _12, tb, b ## _12)
#define DYN_SUB(ta, a, tb, b) DYN_OP2(vecsub, ta, a ## _12, tb, b ## _12)
#define DYN_SL(ta, a, tb, b) DYN_OP2(vecsll, ta, a ## _12, tb, b ## _12)
#define DYN_SLI(ta, a, b) DYN_OP2(vecsll, ta, a ## _12, ta, b)
#define DYN_SR(ta, a, tb, b) DYN_OP2(vecsrl, ta, a ## _12, tb, b ## _12)
#define DYN_SRI(ta, a, b) DYN_OP2(vecsrl, ta, a ## _12, ta, b)
#define DYN_MAX(ta, a, tb, b) DYN_OP2(vecmax, ta, a ## _12, ta, b ## _12)
#define DYN_MIN(ta, a, tb, b) DYN_OP2(vecmin, ta, a ## _12, ta, b ## _12)
#define DYN_MADD(ta, a, tb, b, tc, c) DYN_OP3(mulAdd, ta, a ## _123, tb, b ## _123, tc, c ## _123)
#define DYN_MSUB(ta, a, tb, b, tc, c) DYN_OP3(mulSub, ta, a ## _123, tb, b ## _123, tc, c ## _123)
#define DYN_NMADD(ta, a, tb, b, tc, c) DYN_OP3(negMulAdd, ta, a ## _123, tb, b ## _123, tc, c ## _123)
#define DYN_NMSUB(ta, a, tb, b, tc, c) DYN_OP3(negMulSub, ta, a ## _123, tb, b ## _123, tc, c ## _123)
/* End Vector extension */

#define validate_csr(which, write) ({ \
  if (!STATE.serialized) return PC_SERIALIZE_BEFORE; \
  STATE.serialized = false; \
  unsigned csr_priv = get_field((which), 0x300); \
  unsigned csr_read_only = get_field((which), 0xC00) == 3; \
  if (((write) && csr_read_only) || STATE.prv < csr_priv) \
    throw trap_illegal_instruction(0); \
  (which); })

// Seems that 0x0 doesn't work.
#define DEBUG_START             0x100
#define DEBUG_END                 (0x1000 - 1)

#endif
