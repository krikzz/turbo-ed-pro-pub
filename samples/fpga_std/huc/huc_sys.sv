

module huc_sys(//everdrive menu mapper

	input  HucIn  huc_i,
	output HucOut huc_o
);
	wire clk;
	CpuBus cpu;
	MemCtrl rom;
	MemCtrl ram;
	
	assign clk 			= huc_i.clk;
	assign cpu 			= huc_i.cpu;	
	
	assign huc_o.rom 	= rom;
	assign huc_o.ram 	= ram;
	
	assign huc_o.cart_ce		= rom.ce | ram.ce;
	assign huc_o.cart_dato	= rom.ce ? huc_i.rom_dato : huc_i.ram_dato;
//************************************************************************************* 
	
	assign rom.dati	= cpu.data;
	assign rom.addr	= {7'h7f, cpu.addr[16:0]};
	assign rom.ce		= cpu.addr[20] == 0;
	assign rom.oe		= cpu.oe;
	assign rom.we		= 0;//cpu.we & cpu.addr[16:13] == 'b1111;//wr accepted for last bank (malloc_ed backup)
	
endmodule
