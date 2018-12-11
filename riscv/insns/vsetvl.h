require_extension('V');
require_vec;
STATE.vl = RS1 < (reg_t)STATE.vlmax ? RS1 : (reg_t)STATE.vlmax;
WRITE_RD(STATE.vl);
