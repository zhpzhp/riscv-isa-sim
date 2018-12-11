VL_LOOP
  WRITE_INT_VRD(MMU.load_int32(RS1 + insn.rvv_load_imm()), INT32);
}
