
//****************************************************************************** cpu io
module cpu_io(
	
	input  clk,
	
	input  [7:0]cpu_dato,
	input  [20:0]cpu_addr,
	input  cpu_oe_n, 
	input  cpu_we_n,
	input  cpu_hsm,
	input cpu_irq_n,
	input cpu_rst_n,
	
	output CpuBus cpu
);

	
	
	assign cpu.data[7:0]		= cpu.we_sync ? data_sync : cpu_dato[7:0];
	assign cpu.addr[20:0]	= cpu.we_sync ? addr_sync : cpu_addr[20:0];
	assign cpu.oe				= !cpu_oe_n;
	assign cpu.we				= !cpu_we_n & we_stb;
	assign cpu.we_sync		= cpu_we_st[2:0] == 'b110;//potential bus problem
	assign cpu.oe_sync		= cpu_oe_st[2:0] == 'b011;//potential bus problem
	assign cpu.hsm				= cpu_hsm;
	assign cpu.irq				= !cpu_irq_n;
	assign cpu.rst				= !cpu_rst_n;

	
	
	reg [7:0]cpu_we_st;
	reg [7:0]cpu_oe_st;
	
	always @(posedge clk)
	begin
		cpu_oe_st[7:0]	<= {cpu_oe_st[6:0], cpu.oe};
		cpu_we_st[7:0]	<= {cpu_we_st[6:0], cpu.we};
	end
	
	reg [7:0]data_sync;
	reg [20:0]addr_sync;
	
	always @(negedge cpu.we)
	begin
		data_sync		<= cpu_dato[7:0];
		addr_sync		<= cpu_addr[20:0];
	end
	
	
	
	wire we_stb					= cpu.hsm ? 1 : we_stb_st[5];
	
	reg [7:0]we_stb_st;
	
	always @(posedge clk)
	begin
		we_stb_st	<= {we_stb_st[6:0], !cpu_we_n};
	end
	
endmodule

//****************************************************************************** timer
module timer(

	input  clk,
	input  a0,
	input  timer_latch,
	
	output [7:0]timer_dato
);

	assign timer_dato = !a0 ? timer_val[7:0] : timer_val[15:8];

	wire tick_ms = clk_ctr == 16'd49999;
	
	reg [15:0]clk_ctr;
	reg [15:0]timer_ms;
	reg [15:0]timer_val;
	
	always @(posedge clk)
	begin
		
		clk_ctr 			<= tick_ms ? 0 : clk_ctr + 1;
		
		
		if(tick_ms)
		begin
			timer_ms		<= timer_ms + 1;
		end
		
		
		if(timer_latch)
		begin
			timer_val	<= timer_ms;
		end
		
	end

endmodule

//****************************************************************************** sys_status
module sys_status(

	input  clk,
	input  CpuBus cpu,
	input  stat_ce,
	input  mcu_busy,
	
	output [7:0]dato
);
	
	reg strobe;
	reg mcu_busy_flag, fpg_busy_flag;//mcu flag resets when cmd complete, fpg flag resets after fpga reconfig
	reg mcu_busy_st;
	
	
	always @(posedge clk)
	begin
		
		mcu_busy_st		<= mcu_busy;
		
		if(stat_ce & cpu.oe_sync)
		begin
			dato[7:0]	<= {4'hA, strobe, fpg_busy_flag, mcu_busy_flag, 1'b1};// !mai.cfg.ct_gmode};
			strobe 		<= !strobe;
		end
	
		if(stat_ce & cpu.we_sync)
		begin
			mcu_busy_flag	<= cpu.data[1];//it resets when mcu finished cmd execution
			fpg_busy_flag 	<= cpu.data[2];//it resets only after fpga reconfig
		end
			else
		if(!mcu_busy_st)
		begin
			mcu_busy_flag 	<= 0;
		end
		
	end
	
endmodule
//****************************************************************************** clk div

module clk_dvp(

	input clk,
	input rst,
	input [31:0]ck_base,
	input [31:0]ck_targ,
	
	output reg ck_out
);

	
	parameter CLK_INC = 64'h20000;
	
	wire [31:0]ratio 	= ck_base * CLK_INC / ck_targ;
	
	reg [31:0]clk_ctr;
		
	always @(negedge clk)
	if(rst)
	begin
		clk_ctr	<= 0;
		ck_out	<= 0;
	end
		else
	begin
		
		if(clk_ctr >= (ratio-CLK_INC))
		begin
			clk_ctr	<= clk_ctr - (ratio-CLK_INC);
			ck_out 	<= 1;
		end
			else
		begin
			clk_ctr 	<= clk_ctr + CLK_INC;
			ck_out 	<= 0;
		end
		
	end
	

endmodule
//****************************************************************************** dma
module dma_io(

	input PiBus pi,
	input PiMap pm,
	input  [15:0]ram0_do,
	input  [15:0]ram1_do,
	
	output DmaBus dma
);
	
	assign dma.pi_data 	= 
	pm.ce_ram0 ? ram0_do : 
	pm.ce_ram1 ? ram1_do : 
	8'hff;
	
	assign dma.req_ram0 	= pm.ce_ram0;
	assign dma.req_ram1 	= pm.ce_ram1;
	assign dma.mem_req	= dma.req_ram0 | dma.req_ram1;
	
	assign dma.mem.dati	= pi.dato;
	assign dma.mem.addr	= pi.addr;
	assign dma.mem.ce		= pi.act & (pi.oe | pi.we);
	assign dma.mem.oe		= pi.act & pi.oe;
	assign dma.mem.we		= pi.act & pi.we;
	
endmodule

//****************************************************************************** mem io
module mem_io(
	
	input  clk,
	input  CpuBus cpu,
	input  ce_data,
	input  ce_addr,
	
	input  [7:0]mem0_dato,
	input  [7:0]mem1_dato,
	output MemCtrl mem,
	output mem_ce0,
	output mem_ce1,
	output [7:0]dato
);
	
	assign mem.dati	= cpu.data;
	assign mem.addr	= addr;
	assign mem.ce		= ce_data & (mem_ce0 | mem_ce1);
	assign mem.oe		= cpu.oe;
	assign mem.we		= cpu.we & addr[31];
	
	assign mem_ce0		= ce_data & addr[30:23] == 'h00;
	assign mem_ce1		= ce_data & addr[30:23] == 'h01;
	assign dato 		= addr[23] == 0 ? mem0_dato : mem1_dato;
	
	
	
	
	reg [31:0]addr;
	
	always @(posedge clk)
	if(cpu.we_sync & ce_addr)
	begin
		addr[31:0]	<= {cpu.data[7:0], addr[31:8]};
	end
		else
	if(mem_rw_end)
	begin
		addr	<= addr + 1;
	end
	
	
	
	wire mem_rw			= ce_data & (cpu.oe | cpu.we);
	wire mem_rw_end	= rw_st[2:0] == 'b110;
	
	reg [2:0]rw_st;
	
	always @(posedge clk)
	begin
		rw_st[2:0]	<= {rw_st[1:0], mem_rw};
	end
	
endmodule
//****************************************************************************** led ctrl

module led_ctrl(
	
	input  clk,
	input  led_i,
	output led_o
);
	
	assign led_o	= ctr_b != 0;
	
	reg [7:0]ctr_a;
	reg [7:0]ctr_b;
	
	always @(posedge clk)
	begin
	
		ctr_a	<= ctr_a + 1;
		
		if(led_i)
		begin
			ctr_b <= 1;
		end
			else
		if(ctr_b != 0 & ctr_a == 0)
		begin
			ctr_b <= ctr_b + 1;
		end
		
	end
	
endmodule
//****************************************************************************** reset control
module rst_ctrl(

	input clk,
	input btn,
	input rst_delay,
	input sst_on,
	
	
	output rst_cpu,
	output rst_cfg
);
	
	assign rst_cpu	= rst_delay & sst_on ? rst_long : btn_st;
	assign rst_cfg	= rst_delay ? rst_long : btn_st;
	
	
	reg btn_st;
	reg rst_long;
	reg [26:0]rst_ctr;
	
	always @(posedge clk)
	begin
		
		btn_st	<= btn;
		
		rst_long	<= rst_ctr[26];
		
		if(!btn_st)
		begin
			rst_ctr	<= 0;
		end
			else
		if(!rst_long)
		begin
			rst_ctr	<= rst_ctr + 1;
		end
		
	end

endmodule
