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
inline velt_t velt(uint8_t x) { return { .x=x }; }
inline velt_t velt(uint16_t x) { return { .x=x }; }
inline velt_t velt(uint32_t x) { return { .x=x }; }
inline velt_t velt(uint64_t x) { return { .x=x }; }
inline velt_t velt(int64_t x) { return { .x=(uint64_t)x }; } // immediate
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
#define VL_LOOP_T(t) for(size_t eidx = 0; eidx < VL_T(t); eidx++)
#define VL_LOOP VL_LOOP_T(TRD)


#define require_vec require_accelerator
#define dirty_vec_state dirty_ext_state
#ifndef RISCV_ENABLE_COMMITLOG
#define WRITE_VREG_ELEM(reg, elem, value) ({ STATE.VR[elem].write(reg, value); })
#else
#define WRITE_VREG_ELEM(reg, elem, value) ({ \
    if(vIsFP(VTY(reg))) \
      printf("Writing V%d[%d]=%llx:%llx\n",reg,elem,value.f.v[1],value.f.v[0]); \
    else \
      printf("Writing V%d[%d]=%llx\n",reg,elem,value.x); \
    STATE.VR[elem].write(reg, value);})
#endif
#define WRITE_VREG(reg, value) ({ \
    vIsScalar(VTY(reg)) ? WRITE_VREG_ELEM(reg, 0, value) : WRITE_VREG_ELEM(reg, eidx, value); })
#define WRITE_VRD(v1,v2,v3,value) WRITE_VREG(insn.rd(), DYN_TRUNCATE(TRD, TIN(v1,v2,v3), value))
#define READ_VREG_ELEM(reg, elem) ( STATE.VR[elem][reg] )
#define READ_VREG(reg) ( vIsScalar(VTY(reg)) ? READ_VREG_ELEM(reg, 0) : READ_VREG_ELEM(reg, eidx) )

#define VTYPE(S, W, R) ( (S << 11) | (R << 6) | W)
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


// Type permotion and demotion
#define INTER_TYPE(v1, t1, v2, t2, v3, t3) ({ \
    vtype_t oT = t1; \
    if(v1 && vIsFP(t1) && (!v2 || vIsFP(t2)) && (!v3 || vIsFP(t3))) \
      oT = VTYPE(0, W128, FP);  \
    oT; })

#define DYN_EXTEND(outT, inT, val) ({ \
    velt_t outV = velt(val); \
    if(VEW(inT) < VEW(outT)) \
      outV = vIsInt(inT) ? velt(sext_xlen(outV.x)) : (vIsUInt(inT) ? velt(zext_xlen(outV.x)) : \
          (vIsFP(inT) ? (vIs64(inT) ? velt(f64_to_f128(f64(outV.f))) : (vIs32(inT) ? velt(f32_to_f128(f32(outV.f))) : (vIs16(inT) ? velt(f16_to_f128(f16(outV.f))) : \
            throw trap_illegal_instruction(0)))) : \
            throw trap_illegal_instruction(0))); \
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

// Operations
#define DYN_OP2(iop, fop, ta, a, tb, b) ({ velt_t outV; \
  switch(VEREP(ta)) { \
  case INT: case UINT: \
    switch(VEREP(tb)) { \
    case INT: case UINT: outV = velt(a.x iop b.x); break;\
    default: throw trap_illegal_instruction(0); } break; \
  case FP: \
    switch(VEREP(tb)) { \
    case FP: outV = velt(f128_##fop(a.f, b.f)); break;\
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
    switch(VEREP(TRD)) { \
    case FP: \
      outV = vIs64(TRD) ? velt(f64(MMU.load_uint64(a+eidx*8))) : vIs32(TRD) ? velt(f32(MMU.load_uint32(a+eidx*4))) : vIs16(TRD) ? velt(f16(MMU.load_uint16(a+eidx*2))) : velt((float128_t){MMU.load_uint64(a+eidx*16), MMU.load_uint64(a+eidx*16+8)});\
      break; \
    case UINT: \
      outV = vIs64(TRD) ? velt(MMU.load_uint64(a+eidx*8)) : vIs32(TRD) ? velt(MMU.load_uint32(a+eidx*4)) : vIs16(TRD) ? velt(MMU.load_uint16(a+eidx*2)) : vIs8(TRD) ? velt(MMU.load_uint8(a+eidx)) : velt((float128_t){MMU.load_uint64(a+eidx*16), MMU.load_uint64(a+eidx*16+8)});\
      break; \
    case INT: \
      outV = vIs64(TRD) ? velt(MMU.load_uint64(a+eidx*8)) : vIs32(TRD) ? velt(MMU.load_uint32(a+eidx*4)) : vIs16(TRD) ? velt(MMU.load_uint16(a+eidx*2)) : vIs8(TRD) ? velt(MMU.load_uint8(a+eidx)) : velt((float128_t){MMU.load_uint64(a+eidx*16), MMU.load_uint64(a+eidx*16+8)});\
      break; \
    default: throw trap_illegal_instruction(0); } \
    outV; })

#define DYN_STORE(a, tb, b) ({ \
    switch(VEREP(tb)) { \
    case UINT: case INT: \
      if(vIs64(tb)) \
        (MMU.store_uint64(a+eidx*8, b.x)); \
      else if (vIs32(tb)) \
        (MMU.store_uint32(a+eidx*4, b.x)); \
      else if (vIs16(tb)) \
        (MMU.store_uint16(a+eidx*2, b.x)); \
      else if(vIs8(tb)) \
        (MMU.store_uint8(a+eidx, b.x)); \
      else \
        throw trap_illegal_instruction(0);\
      break; \
    case FP: \
      if(vIs64(tb) ) \
        (MMU.store_uint64(a+eidx*8, b.f.v[0])); \
      else if(vIs32(tb) ) \
        (MMU.store_uint32(a+eidx*4, b.f.v[0])); \
      else if(vIs16(tb) ) \
        (MMU.store_uint16(a+eidx*2, b.f.v[0])); \
      else if(vIs128(tb) ) { \
        MMU.store_uint64(a+eidx*16, b.f.v[0]); \
        MMU.store_uint64(a+eidx*16+8, b.f.v[1]); \
      } else \
        throw trap_illegal_instruction(0); \
      break; \
    default: \
      throw trap_illegal_instruction(0); \
      } \
    })

// Redirect for INT and UINT
#define mulAdd(a, b, c) ( a * b + c )
#define DYN_ADD(ta, a, tb, b) DYN_OP2(+, add, ta, a ## _12, tb, b ## _12)
#define DYN_ADDI(ta, a, b) DYN_OP2(+, add, ta, a ## _12, ta, b)
#define DYN_SUB(ta, a, tb, b) DYN_OP2(-, sub, ta, a, tb, b)
#define DYN_SL(ta, a, tb, b) DYN_OP2(<<, err, ta, a ## _12, tb, b ## _12)
#define DYN_SLI(ta, a, b) DYN_OP2(<<, err, ta, a ## _12, ta, b)
#define DYN_MADD(ta, a, tb, b, tc, c) DYN_OP3(mulAdd, ta, a ## _123, tb, b ## _123, tc, c ## _123)
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
