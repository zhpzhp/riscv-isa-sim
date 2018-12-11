VL_LOOP
  WRITE_INT_VRD(MMU.load_int64(RS1 + insn.rvv_load_imm()), INT64);
}
