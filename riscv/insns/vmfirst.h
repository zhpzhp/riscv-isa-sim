reg_t firstIndex = -1;
VL_LOOP_T(TRS1)
  if(VS1.x & 1) { firstIndex = eidx; break; }
}
WRITE_RD(firstIndex);
