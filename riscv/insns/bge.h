enable_print=1;
is_branch=1;
valid_rs1=1;
valid_rs2=1;
id_rs1=(unsigned int)insn.rs1();
id_rs2=(unsigned int)insn.rs2();
v_rs1=(unsigned long int)RS1;
v_rs1<<=32;
v_rs1>>=32;
bits_rs1=cal_bits(v_rs1);
v_rs2=(unsigned long int)RS2;
v_rs2<<=32;
v_rs2>>=32;
bits_rs2=cal_bits(v_rs2);

if(sreg_t(RS1) >= sreg_t(RS2))
{
  branch=1;
  set_pc(BRANCH_TARGET);
}
