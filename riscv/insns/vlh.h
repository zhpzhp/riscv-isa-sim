VL_LOOP
  WRITE_INT_VRD(MMU.load_int16(RS1 + insn.rvv_load_imm()), INT16);
}
