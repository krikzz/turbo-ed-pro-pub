

module pi_map(

	input  PiBus pi,
	output PiMap pm
);

	wire pi_exec			= pi.oe | pi.we;
	wire [4:0]pi_dst 		= pi.addr[24:20];//1M blocks
	
	assign pm.ce_ram0		= pi_dst[4:3] == 0  & pi_exec;//8M 0x0000000 psram
	assign pm.ce_ram1		= pi_dst[4:3] == 1  & pi_exec;//8M 0x0800000 sram
	assign pm.ce_sys 		= pi_dst[4:0] == 24 & pi_exec;//1M 0x1800000 system  registers
	
//******** system registers
	wire [3:0]sys_dst		= pi.addr[19:16];//64K blocks
	
	assign pm.ce_cfg		= pm.ce_sys & sys_dst == 0 & {pi.addr[15:3], 3'd0} == 'h00f8;//sys cfg. last 8 of 256 bytes
	assign pm.ce_cc_ram	= pm.ce_sys & sys_dst == 0 & {pi.addr[15:8], 8'd0} == 'h0100;//256B ram codes
	assign pm.ce_cc_rom	= pm.ce_sys & sys_dst == 0 & {pi.addr[15:8], 8'd0} == 'h0200;//256B rom codes
	assign pm.ce_fifo 	= pm.ce_sys & sys_dst == 1;//64K 0x1810000 fifo. do not use next 64k
	assign pm.ce_exp		= pm.ce_sys & sys_dst == 3;//64K 0x1830000 exp cards (cd-rom)
	assign pm.ce_brm		= pm.ce_sys & sys_dst == 4;//64K 0x1840000 bram data + bram ack
	
endmodule

