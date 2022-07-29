// See LICENSE for license details.

#include "insn_template.h"
#include "insn_macros.h"

reg_t rv32i_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  #define xlen 32
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  unsigned long int v_rs1=0,v_rs2=0,v_rd=0,v_imm=0,v_mem=0;
  unsigned int id_rs1=0,id_rs2=0,id_rd=0,id_csr=0;
  unsigned long int addr_mem=0;
  bool valid_rs1=0,valid_rs2=0,valid_rd=0,valid_imm=0,valid_mem=0,op_mem=0,branch=0,is_fence=0,is_env=0,is_csr=0,write_csr=0,is_branch=0;
  unsigned int bits_rs1=0,bits_rs2=0,bits_rd=0,bits_imm=0,bits_mem=0;
  unsigned int print_pc=(unsigned int)pc;
  static bool print_flag=0;
  bool enable_print=0;
  auto cal_bits=[](unsigned long int a)
  {
    if (a==0x80000000)
    {
      return 32;
    }
    if (a>0x80000000)
    {
      a=0xffffffff-a;
    }
    unsigned int bits=(unsigned int)(floor(log2(a)));
    if (bits>16)
      return 32;
    if (bits>8)
      return 16;
    if (bits>4)
      return 8;
    if (bits>2)
      return 4;
    if (bits>1)
      return 2;
    return 1;
  };

  uint64_t bits = insn.bits() & ((1ULL << (8 * insn_length(insn.bits()))) - 1);
  #include "insns/NAME.h"
  trace_opcode(p, OPCODE, insn);
  if (bits!=0x0000006f && enable_print)
  {
    //fprintf(stderr,"%08x,NAME,%d,%d,%d,%d,%03x,%d,%d,%d,%02d,%08lx,%02d,%d,%02d,%08lx,%02d,%d,%02d,%08lx,%02d,%d,%08lx,%02d,%d,%d,%08lx,%08lx,%02d\n",print_pc,is_branch,branch,is_csr,write_csr,id_csr,is_fence,is_env,valid_rs1,id_rs1,v_rs1,bits_rs1,valid_rs2,id_rs2,v_rs2,bits_rs2,valid_rd,id_rd,v_rd,bits_rd,valid_imm,v_imm,bits_imm,valid_mem,op_mem,addr_mem,v_mem,bits_mem);
    fprintf(stderr,"%08x,NAME,%d,%d\n",print_pc,is_branch,branch);
  
  }
  else
  {
    if (!print_flag && enable_print)
    {
      //fprintf(stderr,"%08x,NAME,%d,%d,%d,%d,%03x,%d,%d,%d,%02d,%08lx,%02d,%d,%02d,%08lx,%02d,%d,%02d,%08lx,%02d,%d,%08lx,%02d,%d,%d,%08lx,%08lx,%02d\n",print_pc,is_branch,branch,is_csr,write_csr,id_csr,is_fence,is_env,valid_rs1,id_rs1,v_rs1,bits_rs1,valid_rs2,id_rs2,v_rs2,bits_rs2,valid_rd,id_rd,v_rd,bits_rd,valid_imm,v_imm,bits_imm,valid_mem,op_mem,addr_mem,v_mem,bits_mem);
      fprintf(stderr,"%08x,NAME,%d,%d\n",print_pc,is_branch,branch);
      print_flag=1;
    }
  }
  #undef xlen
  return npc;
}

reg_t rv64i_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  #define xlen 64
  unsigned long int v_rs1=0,v_rs2=0,v_rd=0,v_imm=0,v_mem=0;
  unsigned int id_rs1=0,id_rs2=0,id_rd=0,id_csr=0;
  unsigned long int addr_mem=0;
  bool valid_rs1=0,valid_rs2=0,valid_rd=0,valid_imm=0,valid_mem=0,op_mem=0,branch=0,is_fence=0,is_env=0,is_csr=0,write_csr=0,is_branch=0;
  unsigned int bits_rs1=0,bits_rs2=0,bits_rd=0,bits_imm=0,bits_mem=0;
  bool enable_print=0;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  auto cal_bits=[](unsigned long int a) {return 1;};
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  #include "insns/NAME.h"
  trace_opcode(p, OPCODE, insn);
  #undef xlen
  return npc;
}

#undef CHECK_REG
#define CHECK_REG(reg) require((reg) < 16)

reg_t rv32e_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  #define xlen 32
  unsigned long int v_rs1=0,v_rs2=0,v_rd=0,v_imm=0,v_mem=0;
  unsigned int id_rs1=0,id_rs2=0,id_rd=0,id_csr=0;
  unsigned long int addr_mem=0;
  bool valid_rs1=0,valid_rs2=0,valid_rd=0,valid_imm=0,valid_mem=0,op_mem=0,branch=0,is_fence=0,is_env=0,is_csr=0,write_csr=0,is_branch=0;
  unsigned int bits_rs1=0,bits_rs2=0,bits_rd=0,bits_imm=0,bits_mem=0;
  bool enable_print=0;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  auto cal_bits=[](unsigned long int a) {return 1;};
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  #include "insns/NAME.h"
  trace_opcode(p, OPCODE, insn);
  #undef xlen
  return npc;
}

reg_t rv64e_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  #define xlen 64
  unsigned long int v_rs1=0,v_rs2=0,v_rd=0,v_imm=0,v_mem=0;
  unsigned int id_rs1=0,id_rs2=0,id_rd=0,id_csr=0;
  unsigned long int addr_mem=0;
  bool valid_rs1=0,valid_rs2=0,valid_rd=0,valid_imm=0,valid_mem=0,op_mem=0,branch=0,is_fence=0,is_env=0,is_csr=0,write_csr=0,is_branch=0;
  unsigned int bits_rs1=0,bits_rs2=0,bits_rd=0,bits_imm=0,bits_mem=0;
  bool enable_print=0;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  auto cal_bits=[](unsigned long int a) {return 1;};
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  #include "insns/NAME.h"
  trace_opcode(p, OPCODE, insn);
  #undef xlen
  return npc;
}
