VL_LOOP
  if(RS2+eidx < VL)
    WRITE_VREG_ELEM(insn.rd(), ((size_t)RS2)+eidx, velt(VS1), VTYPE, 1);
}
