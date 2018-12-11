VL_LOOP
  WRITE_INT_VRD(MMU.load_uint16(RS1 + insn.rvv_load_imm()), UINT16);
}
