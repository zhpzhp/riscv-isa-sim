VL_LOOP
  WRITE_VRD(velt((RS2+eidx < VL) ? READ_VREG_ELEM(insn.rs1(), RS2+eidx, VTYPE, 1) : 0));
}
