require_extension('V');
require_vec;
/*
fprintf(stderr, "Vtypes before config\n");
for(size_t i = 0; i < NVR; i++) {
  fprintf(stderr, "Vtype[%d]:%x\n", i, STATE.vtype[i]);
}
*/
p->set_csr(CSR_VCFG, vfastcfg(insn.rvv_imm(), STATE.vcfg));
WRITE_RD(STATE.vlmax);
fprintf(stderr, "Vtypes after config\n");
for(size_t i = 0; i < NVR; i++) {
  fprintf(stderr, "Vtype[%lu]:%lx\n", i, STATE.vtype[i]);
}

