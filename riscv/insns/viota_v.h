reg_t iota_sum = 0;
VL_LOOP
  WRITE_VRD(velt(iota_sum));
  iota_sum++;
}
