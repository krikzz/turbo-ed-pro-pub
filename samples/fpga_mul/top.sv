
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


//************************************************************************************* initialization for unused stuff
	assign mcu_mode 			= 1;//mcu master mode (unused, should be 1)
	assign mcu_brm_n			= 1;
	assign mcu_fifo_rxf		= 1;
	
	assign dat_oe 				= 0;//bus output always enabled
	assign cpu_cart			= 0;//cartridge detect. Needs for Duo's built in system card management
	assign cpu_irq				= 1'bz;
	assign led_n 				= 1'bz;//blinking led
	
	//turn off second psram chip
	assign ram1_addr[21:0] 	= 0;
	assign ram1_data[15:0] 	= 0;
	assign ram1_ce 			= 1;
	assign ram1_oe 			= 1;
	assign ram1_we 			= 1;
	assign ram1_ub 			= 1;
	assign ram1_lb 			= 1;
//************************************************************************************* cpu data bus assigments
	assign cpu_data[7:0]		= !bus_oe ? 8'hzz : region ? cpu_dati_tg : cpu_dati_pc;
	assign dat_dir 			= bus_oe;//data bus direction

	wire bus_oe					= cpu_addr[20] == 0 & !cpu_oe;
	wire [7:0]cpu_dati_pc	= cpu_dati;
	wire [7:0]cpu_dati_tg	= {cpu_dati[0],cpu_dati[1],cpu_dati[2],cpu_dati[3],cpu_dati[4],cpu_dati[5],cpu_dati[6],cpu_dati[7]};
	
	wire [7:0]cpu_dato_pc	= cpu_data[7:0];
	wire [7:0]cpu_dato_tg	= {cpu_data[0],cpu_data[1],cpu_data[2],cpu_data[3],cpu_data[4],cpu_data[5],cpu_data[6],cpu_data[7]};
	
	wire [7:0]cpu_dati		= mul_ce ? mul_8 : cpu_addr[0] == 0 ? ram0_data[15:8] : ram0_data[7:0];
	wire [7:0]cpu_dato 		= region ? cpu_dato_tg : cpu_dato_pc;
//************************************************************************************* memory assigments
	assign ram0_addr[18:0] 	= cpu_addr[19:1];
	assign ram0_data[15:0] 	= !ram0_oe ? 16'hzzzz : {cpu_dati[7:0], cpu_dati[7:0]};
	assign ram0_ce 			= cpu_addr[20] == 0 & (!cpu_oe | !cpu_we) ? 0 : 1;
	assign ram0_oe 			= cpu_oe;
	assign ram0_we 			= cpu_we;
	assign ram0_ub 			= cpu_addr[0] == 0 ? 0 : 1;
	assign ram0_lb 			= cpu_addr[0] == 1 ? 0 : 1;
//************************************************************************************* reset control
	assign mcu_rst 			= btn_n;//return to menu request for mcu
	assign cpu_rst				= rst_ctr[23];//system cpu reset
	
	
	reg [23:0]rst_ctr;
	reg btn_n_st;
	
	always @(posedge clk)
	begin
		
		btn_n_st		<= btn_n;
		
		if(btn_n_st == 0)
		begin
			rst_ctr	<= 0;
		end
			else
		if(rst_ctr[23] == 0)
		begin
			rst_ctr	<= rst_ctr + 1;//initial system reset
		end
		
	end
//************************************************************************************* hv multiplier

	wire mul_ce 		= {cpu_addr[20:3], 3'b0} == 21'hFFFF8;//map multiplier registers to the last 8 bytes of rom space
	wire mul_we 		= mul_ce & !cpu_we;
	
	wire [31:0]mul_32	=	mul_arg_0 * mul_arg_1;
	
	wire [7:0]mul_8	=  
	cpu_addr[1:0] == 0 ? mul_32[7:0]   : 
	cpu_addr[1:0] == 1 ? mul_32[15:8]  : 
	cpu_addr[1:0] == 2 ? mul_32[23:16] : mul_32[31:24];
	
	reg [31:0]mul_arg_0;
	reg [31:0]mul_arg_1;
	
	always @(negedge mul_we)
	case(cpu_addr[2:0])
		0:mul_arg_0[7:0]		<= cpu_dato;
		1:mul_arg_0[15:8]		<= cpu_dato;
		2:mul_arg_0[23:16]	<= cpu_dato;
		3:mul_arg_0[31:24]	<= cpu_dato;
		4:mul_arg_1[7:0]		<= cpu_dato;
		5:mul_arg_1[15:8]		<= cpu_dato;
		6:mul_arg_1[23:16]	<= cpu_dato;
		7:mul_arg_1[31:24]	<= cpu_dato;
	endcase
	
	
endmodule
