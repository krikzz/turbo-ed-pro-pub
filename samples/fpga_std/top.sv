
module top(

	//master clock 50Mhz
	input  clk,

	//cpu bus
	inout  [7:0]cpu_data,
	input  [20:0]cpu_addr,
	input  cpu_oe,
	input  cpu_we,
	input  cpu_hsm,
	inout  cpu_rst,
	output cpu_irq,
	output cpu_cart,
	
	//bus transceiver
	output dat_dir, dat_oe,
	
	//mcu spi
	output spi_miso,
	input  spi_mosi, spi_sck,
	input  spi_ss,
	
	//mcu control
	output mcu_fifo_rxf, mcu_mode, mcu_brm_n, mcu_rst,
	input  mcu_busy,
	input  region,

	//psram 0
	inout  [15:0]ram0_data,
	output [21:0]ram0_addr,
	output ram0_oe, ram0_we, ram0_ub, ram0_lb, ram0_ce,
	
	//psram 1
	inout  [15:0]ram1_data,
	output [21:0]ram1_addr,
	output ram1_oe, ram1_we, ram1_ub, ram1_lb, ram1_ce,
	
	//gpio
	inout  [3:0]gpio,
	
	//dac i2s bus
	output dac_mclk, dac_lrck, dac_sclk, dac_sdin,
	
	//var
	output led_n,
	input  btn_n
);


//************************************************************************************* mcu
	assign mcu_mode 			= !mcu_master;//mcu master mode (unused, should be 1)
	assign mcu_rst 			= 1;
	assign mcu_brm_n			= !mcu_brm_bc;
	wire mcu_master;
	wire mcu_brm_bc;
//************************************************************************************* cpu
	//assign data[15:8]			= !bus_oe ? 8'hzz : 8'hff;
	assign cpu_data[7:0]		= !bus_oe ? 8'hzz : region ? cpu_dati_tg : cpu_dati_pc;
	assign cpu_rst				= !cpu_rst_n ? 0 : 1'bz;
	assign cpu_cart 			= !cpu_cart_n ? 0 : 1'bz;
	assign cpu_irq 			= !cpu_irq_n ? 0 : 1;//1'bz;
	
	wire [7:0]cpu_dati_pc	= cpu_dati;
	wire [7:0]cpu_dati_tg	= {cpu_dati[0],cpu_dati[1],cpu_dati[2],cpu_dati[3],cpu_dati[4],cpu_dati[5],cpu_dati[6],cpu_dati[7]};
	
	wire [7:0]cpu_dato_pc	= cpu_data[7:0];
	wire [7:0]cpu_dato_tg	= {cpu_data[0],cpu_data[1],cpu_data[2],cpu_data[3],cpu_data[4],cpu_data[5],cpu_data[6],cpu_data[7]};
	wire [7:0]cpu_dati;
	wire [7:0]cpu_dato 		= region ? cpu_dato_tg : cpu_dato_pc;
	
	wire cpu_oe_n				= cpu_oe;
	wire cpu_we_n				= cpu_we;
	wire cpu_rst_n;
	wire cpu_cart_n;
//************************************************************************************* memory	
	assign ram0_addr[21:0] 	= mem0_addr[22:1];
	assign ram0_data[15:0] 	= mem0_oe ? 16'hzzzz : {mem0_dati[7:0], mem0_dati[7:0]};
	assign ram0_ce 			= !mem0_ce;
	assign ram0_oe 			= !mem0_oe;
	assign ram0_we 			= !mem0_we;
	assign ram0_ub 			= !mem0_ce ? 1 : mem0_addr[0] == 0 ? 0 : 1;
	assign ram0_lb 			= !mem0_ce ? 1 : mem0_addr[0] == 1 ? 0 : 1;
	
	
	assign ram1_addr[21:0] 	= mem1_addr[22:1];
	assign ram1_data[15:0] 	= mem1_oe ? 16'hzzzz : {mem1_dati[7:0], mem1_dati[7:0]};
	assign ram1_ce 			= !mem1_ce;
	assign ram1_oe 			= !mem1_oe;
	assign ram1_we 			= !mem1_we;
	assign ram1_ub 			= !mem1_ce ? 1 : mem1_addr[0] == 0 ? 0 : 1;
	assign ram1_lb 			= !mem1_ce ? 1 : mem1_addr[0] == 1 ? 0 : 1;
	
	
	wire [7:0]mem0_dati;
	wire [7:0]mem0_dato		= mem0_addr[0] == 0 ? ram0_data[15:8] : ram0_data[7:0];
	wire [23:0]mem0_addr;
	wire mem0_ce;
	wire mem0_oe;
	wire mem0_we;
	
	wire [7:0]mem1_dati;
	wire [7:0]mem1_dato		= mem1_addr[0] == 0 ? ram1_data[15:8] : ram1_data[7:0];
	wire [23:0]mem1_addr;
	wire mem1_ce;
	wire mem1_oe;
	wire mem1_we;
//************************************************************************************* bus drivers	
	assign dat_dir 			= bus_oe;//data bus direction
	assign dat_oe 				= !cpu_rst ? 1 : 0;//bus output always enabled
	wire bus_oe;
//************************************************************************************* leds
	assign led_n 				= led_r | led_g ? 0 : 1'bz;
	wire led_g;
	wire led_r;
//************************************************************************************* turbo-ed core
	turbo_ed turbo_inst(
	
		.clk(clk),
		
		.cpu_dati(cpu_dati),
		.cpu_dato(cpu_dato),
		.cpu_addr(cpu_addr),
		.cpu_oe_n(cpu_oe_n), 
		.cpu_we_n(cpu_we_n),
		.cpu_hsm(cpu_hsm),
		.cpu_irq_n(cpu_irq_n),
		.cpu_rst_n(cpu_rst_n),
		.cpu_cart_n(cpu_cart_n),
		
		.ram0_dati(mem0_dati),
		.ram0_dato(mem0_dato),
		.ram0_addr(mem0_addr),
		.ram0_ce(mem0_ce),
		.ram0_oe(mem0_oe),
		.ram0_we(mem0_we),
		
		.ram1_dati(mem1_dati),
		.ram1_dato(mem1_dato),
		.ram1_addr(mem1_addr),
		.ram1_ce(mem1_ce),
		.ram1_oe(mem1_oe),
		.ram1_we(mem1_we),
		
		.spi_miso(spi_miso),
		.spi_mosi(spi_mosi), 
		.spi_sck(spi_sck),
		.spi_ss(spi_ss),
		
		.mcu_busy(mcu_busy),
		.mcu_fifo_rxf(mcu_fifo_rxf),
		.mcu_master(mcu_master),
		.mcu_brm_bc(mcu_brm_bc),
		
		.dac_mclk(dac_mclk),
		.dac_lrck(dac_lrck),
		.dac_sclk(dac_sclk),
		.dac_sdin(dac_sdin),
		
		.bus_oe(bus_oe),
		.led_g(led_g),
		.led_r(led_r),
		.btn(!btn_n)
	);
//*************************************************************************************
	assign dbg_oe 		= !cpu_oe_n;
	assign dbg_we 		= !cpu_we_n;
	assign dbg_data	= cpu_data[7:0];
	assign dbg_addr 	= cpu_addr;
	
	assign dbg_ck	= (oe_sync | we_sync);// & dbg_cd;
	
	assign dbg_cd	= {cpu_addr[20:4], 4'd0} == 'h1FF800;
	
	wire oe_sync 	= oe_st[4:2] == 'b011;
	wire we_sync 	= we_st[2:0] == 'b011;
	
	reg [7:0]oe_st;
	reg [7:0]we_st;
	
	always @(posedge clk)
	begin
		
		oe_st[7:0] <= {oe_st[6:0], dbg_oe};
		we_st[7:0] <= {we_st[6:0], dbg_we};
		
	end
endmodule
