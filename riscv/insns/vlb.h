VL_LOOP
  WRITE_INT_VRD(MMU.load_int8(RS1 + insn.rvv_load_imm()), INT8);
}
