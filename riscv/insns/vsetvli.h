require_extension('V');
require_vec;
p->set_csr(CSR_VTYPE, insn.rvv_conf_imm());
STATE.vl = RS1 < (reg_t)STATE.vlmax ? RS1 : (reg_t)STATE.vlmax;
WRITE_RD(STATE.vl);

