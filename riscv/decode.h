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

  int64_t rvv_imm() { return xs(20, 5); }
  int64_t rvv_uload_imm() { return (x(20, 5) + (xs(30, 2) << 5)); }
  int64_t rvv_load_imm() { return xs(30, 2); }
  int64_t rvv_ustore_imm() { return (x(20, 5) + (xs(10, 2) << 5)); }
  int64_t rvv_store_imm() { return xs(10, 2); }
  uint64_t rvv_conf_imm() { return (x(20, 7) << 3) + (x(12,3)); }
  uint64_t rvv_mask() { return x(25, 2); }
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
//ELEN is fixed at 64bit
#define ELEN 64
#define ELEN_GROUPS 8
#define VLEN (ELEN_GROUPS*ELEN)
typedef union velt {
  uint8_t b[8];
  uint16_t h[4];
  uint32_t w[2];
  uint64_t d;
} velt_t;

template <size_t N, bool zero_reg>
class vec_regfile_t
{
public:
  void write_d(size_t i, uint64_t value)
  {
    if (!zero_reg || i != 0)
      data[i].d = value;
  }
  void write_w(size_t i, size_t j, uint32_t value)
  {
    if (!zero_reg || i != 0)
      data[i].w[j] = value;
  }
  void write_h(size_t i, size_t j, uint16_t value)
  {
    if (!zero_reg || i != 0)
      data[i].h[j] = value;
  }
  void write_b(size_t i, size_t j, uint8_t value)
  {
    if (!zero_reg || i != 0)
      data[i].b[j] = value;
  }
  const velt_t& operator [] (size_t i) const
  {
    return data[i];
  }
private:
  velt_t data[N];
};

#define require_vec require_accelerator
#define dirty_vec_state dirty_ext_state
#define VL (insn.rvv_mask() == 0x2 ? 1 : STATE.vl)
#define VL_LOOP_FULL for(size_t eidx = 0; eidx < VL; eidx++) {
#define VL_LOOP VL_LOOP_FULL \
  if(!VM) continue;
#define VM ((insn.rvv_mask() == 0x1) ? READ_VREG(0) & 0x1 : \
           (insn.rvv_mask() == 0x0) ? !(READ_VREG(0) & 0x1) : true)

#define EGRP(elem) ( elem / (ELEN/SEW) )
#define RGRP(reg, elem, width) ( reg + (elem % width))
#define SGRP(elem, width) ( elem % ((ELEN/SEW)/width) )
#define READ_VREG_ELEM(reg, elem, type, width) ( \
    vIs8(type, width) ? \
      STATE.VR[EGRP(elem)][RGRP(reg, elem, width)].b[SGRP(elem, width)] : \
    vIs16(type, width) ? \
      STATE.VR[EGRP(elem)][RGRP(reg, elem, width)].h[SGRP(elem, width)] : \
    vIs32(type, width) ? \
      STATE.VR[EGRP(elem)][RGRP(reg, elem, width)].w[SGRP(elem, width)] : \
      STATE.VR[EGRP(elem)][RGRP(reg, elem, width)].d )
#define READ_VREG(reg) ( READ_VREG_ELEM(reg, eidx, VSEW, 1) )
#define READ_VREG_W(reg) ( READ_VREG_ELEM(reg, eidx, VSEW, 2) )
#ifndef RISCV_ENABLE_COMMITLOG
#define WRITE_VREG_ELEM(reg, elem, value, type, width) ({ \
    if(vIs8(type, width)) { \
      STATE.VR[EGRP(elem)].write_b(RGRP(reg, elem, width), SGRP(elem, width), value.b[0]); \
    } else if(vIs16(type, width)) { \
      STATE.VR[EGRP(elem)].write_h(RGRP(reg, elem, width), SGRP(elem, width), value.h[0]); \
    } else if(vIs32(type, width)) { \
      STATE.VR[EGRP(elem)].write_w(RGRP(reg, elem, width), SGRP(elem, width), value.w[0]); \
    } else if(vIs64(type, width)) { \
      STATE.VR[EGRP(elem)].write_d(RGRP(reg, elem, width), value.d); \
    }})
#else
#define WRITE_VREG_ELEM(reg, elem, value, type, width) ({ \
    if(vIs8(type, width)) { \
      fprintf(stderr, "Writing V%llu[%lu]=%hhx\n",reg,elem,value.b[0]); \
      STATE.VR[EGRP(elem)].write_b(RGRP(reg, elem, width), SGRP(elem, width), value.b[0]); \
    } else if(vIs16(type, width)) { \
      fprintf(stderr, "Writing V%llu[%lu]=%hx\n",reg,elem,value.h[0]); \
      STATE.VR[EGRP(elem)].write_h(RGRP(reg, elem, width), SGRP(elem, width), value.h[0]); \
    } else if(vIs32(type, width)) { \
      fprintf(stderr, "Writing V%llu[%lu]=%x\n",reg,elem,value.w[0]); \
      STATE.VR[EGRP(elem)].write_w(RGRP(reg, elem, width), SGRP(elem, width), value.w[0]); \
    } else if(vIs64(type, width)) { \
      fprintf(stderr, "Writing V%llu[%lu]=%llx\n",reg,elem,value.d); \
      STATE.VR[EGRP(elem)].write_d(RGRP(reg, elem, width), value.d); \
    }})
#endif
#define WRITE_VREG(reg, value, type, width) ({ WRITE_VREG_ELEM(reg, eidx, value, type, width); })

#define WRITE_VRD(value) WRITE_VREG(insn.rd(), value, VSEW, 1)
#define WRITE_VRD_W(value) WRITE_VREG(insn.rd(), value, VSEW, 2)
#define WRITE_VRD_TYPE(value, type) WRITE_VREG(insn.rd(), value, type, 1)
#define VS1 (READ_VREG(insn.rs1()))
#define VS1_W (READ_VREG_W(insn.rs1()))
#define VSS1 (READ_VREG_ELEM(insn.rs1(), 0, VSEW, 1))
#define VS2 (READ_VREG(insn.rs2()))
#define VS2_W (READ_VREG_W(insn.rs2()))
#define VSS2 (READ_VREG_ELEM(insn.rs2(), 0, VSEW, 1))
#define VS3 (READ_VREG(insn.rs3()))
#define VS3_W (READ_VREG_W(insn.rs3()))
#define VSS3 (READ_VREG_ELEM(insn.rs3(), 0, VSEW, 1))
#define VIMM (vIs8(VSEW, 1) ? velt(insn.rvv_imm()).b[0] : \
              vIs16(VSEW, 1) ? velt(insn.rvv_imm()).h[0] : \
              vIs32(VSEW, 1) ? velt(insn.rvv_imm()).w[0] : \
              velt(insn.rvv_imm()).d)

#define VTYPE (STATE.vtype)
#define VW(t) (t & 7)
#define VSEW (VW(VTYPE))
#define VLMUL ((VTYPE >> 6) & 0x3)
#define VR(t) ((t >> 8) & 0x1)
#define VREP (VR(VTYPE))
#define VTY(w, mul, rep) ((w & 7) | ((mul & 3) << 6) | ((rep & 1) << 8))
static const reg_t W8 = 0;
static const reg_t W16 = 1;
static const reg_t W32 = 2;
static const reg_t W64 = 3;
static const reg_t W128 = 4;
static const reg_t W256 = 5;
static const reg_t W512 = 6;
static const reg_t W1024 = 7;
static const reg_t INT = 0;
static const reg_t FP = 1;
#define INT8 VTY(W8, 0, INT)
#define vIsInt(t) ( VR(t) == INT )
#define vIsFP(t) ( VR(t) == FP )
#define vIs8(t, w) ( VW(t) == W8  && w == 1 )
#define vIs16(t, w) ( (VW(t) == W16  && w == 1) || (VW(t) == W8 && w == 2) )
#define vIs32(t, w) ( (VW(t) == W32  && w == 1) || (VW(t) == W16 && w == 2) || (VW(t) == W8 && w == 4) )
#define vIs64(t, w) ( (VW(t) == W64  && w == 1) || (VW(t) == W32 && w == 2) || (VW(t) == W16 && w == 4) || (VW(t) == W8 && w == 8) )
#define vIs128(t, w) ( (VW(t) == W128  && w == 1) || (VW(t) == W64 && w == 2) || (VW(t) == W32 && w == 4) || (VW(t) == W16 && w == 8) || (VW(t) == W8 && w == 16) )
#define vIs256(t, w) ( (VW(t) == W256  && w == 1) || (VW(t) == W128 && w == 2) || (VW(t) == W64 && w == 4) || (VW(t) == W32 && w == 8) || (VW(t) == W16 && w == 16) || (VW(t) == W8 && w == 32) )
#define vIs512(t, w) ( (VW(t) == W512  && w == 1) || (VW(t) == W256 && w == 2) || (VW(t) == W128 && w == 4) || (VW(t) == W64 && w == 8) || (VW(t) == W32 && w == 16) || (VW(t) == W16 && w == 32) || (VW(t) == W8 && w == 64) )
#define vIs1024(t, w) ( (VW(t) == W1024  && w == 1) || (VW(t) == W512 && w == 2) || (VW(t) == W256 && w == 4) || (VW(t) == W128 && w == 8) || (VW(t) == W64 && w == 16) || (VW(t) == W32 && w == 32) || (VW(t) == W16 && w == 64) || (VW(t) == W8 && w == 128) )
#define SEW ( vIs8(VSEW, 1) ? 8 : vIs16(VSEW, 1) ? 16 : vIs32(VSEW, 1) ? 32 : vIs64(VSEW, 1) ? 64 : vIs128(VSEW, 1) ? 128 : vIs256(VSEW, 1) ? 256 : vIs512(VSEW, 1) ? 512 : 1024 )

#define DYN_OP2(op, a, b) ({ velt_t outV; \
  switch(VREP) { \
  case INT: \
    outV = velt(op(a, b)); break;\
  case FP: \
    switch(VSEW) { \
    case W16: outV = velt(f16_##op(f16(a), f16(b))); break;\
    case W32: outV = velt(f32_##op(f32(a), f32(b))); break;\
    case W64: outV = velt(f64_##op(f64(a), f64(b))); break;\
    default: throw trap_illegal_instruction(0); } break; \
  default: throw trap_illegal_instruction(0); } \
  outV; })
#define DYN_OP3(op, a, b, c) ({ velt_t outV; \
  switch(VREP) { \
  case INT: \
    outV = velt(op(a, b, c)); break;\
  case FP: \
    switch(VSEW) { \
    case W16: outV = velt(f16_##op(f16(a), f16(b), f16(c))); break;\
    case W32: outV = velt(f32_##op(f32(a), f32(b), f32(c))); break;\
    case W64: outV = velt(f64_##op(f64(a), f64(b), f64(c))); break;\
    default: throw trap_illegal_instruction(0); } break; \
  default: throw trap_illegal_instruction(0); } \
  outV; })

#define DYN_LOAD(a, sign) ({ velt_t outV; \
    outV = DYN_LOAD_TYPE(a, VTYPE, sign); \
    outV; \
    })
#define DYN_LOAD_TYPE(a, type, sign) ({ velt_t outV; \
    if(vIs64(type, 1)) \
      outV = DYN_LOAD_ST_TYPE(a, 8, type, sign); \
    else if (vIs32(type, 1)) \
      outV = DYN_LOAD_ST_TYPE(a, 4, type, sign); \
    else if (vIs16(type, 1)) \
      outV = DYN_LOAD_ST_TYPE(a, 2, type, sign); \
    else if(vIs8(type, 1)) \
      outV = DYN_LOAD_ST_TYPE(a, 1, type, sign); \
    else \
      throw trap_illegal_instruction(0);\
    outV; \
    })
#define DYN_LOAD_ST(a, st, sign) ({ velt_t outV; \
    outV = DYN_LOAD_ST_TYPE(a, st, VTYPE, sign); \
    outV; \
    })
#define DYN_LOAD_ST_TYPE(a, st, type, sign) ({ velt_t outV; \
    fprintf(stderr,"Loading from:%llx\n",a+eidx*st); \
    outV = DYN_LOAD_ADDR_TYPE(a+eidx*st, type, sign); \
    outV; \
    })
#define DYN_LOAD_ADDR_TYPE(a, type, sign) ( \
  vIsFP(type) ? (vIs64(type, 1) ? velt(f64(MMU.load_uint64(a))) : vIs32(type, 1) ? velt(f32(MMU.load_uint32(a))) : vIs16(type, 1) ? velt(f16(MMU.load_uint16(a))) : throw trap_illegal_instruction(0)) : \
    (vIsInt(type) && sign == 1 ? (vIs64(type, 1) ? velt(MMU.load_int64(a)) : vIs32(type, 1) ? velt(MMU.load_int32(a)) : vIs16(type, 1) ? velt(MMU.load_int16(a)) : vIs8(type, 1) ? velt(MMU.load_int8(a)) : throw trap_illegal_instruction(0)) : \
    (vIsInt(type) && sign == 0 ? (vIs64(type, 1) ? velt(MMU.load_uint64(a)) : vIs32(type, 1) ? velt(MMU.load_uint32(a)) : vIs16(type, 1) ? velt(MMU.load_uint16(a)) : vIs8(type, 1) ? velt(MMU.load_uint8(a)) : throw trap_illegal_instruction(0)) : \
     throw trap_illegal_instruction(0))) )
#define DYN_STORE(a, b) ({ \
    DYN_STORE_TYPE(a, VTYPE, b); \
    })
#define DYN_STORE_TYPE(a, type, b) ({ \
    if(vIs64(type, 1)) \
      DYN_STORE_ST_TYPE(a, 8, type, b); \
    else if (vIs32(type, 1)) \
      DYN_STORE_ST_TYPE(a, 4, type, b); \
    else if (vIs16(type, 1)) \
      DYN_STORE_ST_TYPE(a, 2, type, b); \
    else if(vIs8(type, 1)) \
      DYN_STORE_ST_TYPE(a, 1, type, b); \
    else \
      throw trap_illegal_instruction(0);\
    })
#define DYN_STORE_ST(a, st, b) ({ \
    DYN_STORE_ST_TYPE(a, st, VTYPE, b); \
    })
#define DYN_STORE_ST_TYPE(a, st, type, b) ({ \
    if(vIs64(type, 1)) \
      (MMU.store_uint64(a+eidx*st, b)); \
    else if (vIs32(type, 1)) \
      (MMU.store_uint32(a+eidx*st, b)); \
    else if (vIs16(type, 1)) \
      (MMU.store_uint16(a+eidx*st, b)); \
    else if(vIs8(type, 1)) { \
      (MMU.store_uint8(a+eidx*st, b)); \
    } else \
      throw trap_illegal_instruction(0);\
    })

#define DYN_ADD(a, b) DYN_OP2(vecadd, a, b)
#define DYN_AND(a, b) DYN_OP2(vecand, a, b)
#define DYN_OR(a, b) DYN_OP2(vecor, a, b)
#define DYN_XOR(a, b) DYN_OP2(vecxor, a, b)
#define DYN_DIV(a, b) DYN_OP2(vecdiv, a, b)
#define DYN_MUL(a, b) DYN_OP2(vecmul, a, b)
#define DYN_REM(a, b) DYN_OP2(vecrem, a, b)
#define DYN_SEQ(a, b) DYN_OP2(veceq, a, b)
#define DYN_SNE(a, b) DYN_OP2(vecne, a, b)
#define DYN_SLT(a, b) DYN_OP2(veclt, a, b)
#define DYN_SGE(a, b) DYN_OP2(vecge, a, b)
#define DYN_SUB(a, b) DYN_OP2(vecsub, a, b)
#define DYN_SL(a, b) DYN_OP2(vecsll, a, b)
#define DYN_SR(a, b) DYN_OP2(vecsrl, a, b)
#define DYN_MAX(a, b) DYN_OP2(vecmax, a, b)
#define DYN_MIN(a, b) DYN_OP2(vecmin, a, b)
#define DYN_MADD(a, b, c) DYN_OP3(mulAdd, a, b, c)
#define DYN_MSUB(a, b, c) DYN_OP3(mulSub, a, b, c)
#define DYN_NMADD(a, b, c) DYN_OP3(negMulAdd, a, b, c)
#define DYN_NMSUB(a, b, c) DYN_OP3(negMulSub, a, b, c)

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
// Redirect for INT
#define mulAdd(a, b, c) ( a * b + c )
#define mulSub(a, b, c) ( a * b + (-c) )
#define negMulAdd(a, b, c) ( (-a) * b + (-c) )
#define negMulSub(a, b, c) ( (-a) * b + c )
// Redirect for FP
#define f16_vecadd(a, b) (f16_add(a, b))
#define f32_vecadd(a, b) (f32_add(a, b))
#define f64_vecadd(a, b) (f64_add(a, b))

inline velt_t velt(float16_t f) { return { .h[0]=f.v }; }
inline velt_t velt(float32_t f) { return { .w[0]=f.v }; }
inline velt_t velt(float64_t f) { return { .d=f.v }; }
inline velt_t velt(uint8_t x)   { return { .b[0]=x }; }
inline velt_t velt(uint16_t x)  { return { .h[0]=x }; }
inline velt_t velt(uint32_t x)  { return { .w[0]=x }; }
inline velt_t velt(uint64_t x)  { return { .d=x }; }
inline velt_t velt(int8_t x)   { return { .b[0]=(uint8_t)x }; }
inline velt_t velt(int16_t x)  { return { .h[0]=(uint16_t)x }; }
inline velt_t velt(int32_t x)  { return { .w[0]=(uint32_t)x }; }
inline velt_t velt(int64_t x)  { return { .d=(uint64_t)x }; }
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
