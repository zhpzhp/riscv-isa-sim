reg_t mPop = 0;
VL_LOOP
  if(VS1.x & 1) mPop++;
}
WRITE_RD(mPop);
