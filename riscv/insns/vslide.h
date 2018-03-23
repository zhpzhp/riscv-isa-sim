VL_LOOP
  WRITE_VRD(1,0,0, (RS2+eidx < VL) ? DYN_EXTEND(TIN_1, TRS1, READ_VREG_ELEM(insn.rs1(), RS2+eidx)) : velt(0));
}
