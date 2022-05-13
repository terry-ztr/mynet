
module main (
	////////////////////////////////////
	// FPGA Pins
	////////////////////////////////////

	// Clock pins
	CLOCK_50,
	


	// Pushbuttons
	KEY,

	// LEDs
	LEDR,

	// Slider Switches
	SW,

	////////////////////////////////////
	// HPS Pins
	////////////////////////////////////
	
	// DDR3 SDRAM
	HPS_DDR3_ADDR,
	HPS_DDR3_BA,
	HPS_DDR3_CAS_N,
	HPS_DDR3_CKE,
	HPS_DDR3_CK_N,
	HPS_DDR3_CK_P,
	HPS_DDR3_CS_N,
	HPS_DDR3_DM,
	HPS_DDR3_DQ,
	HPS_DDR3_DQS_N,
	HPS_DDR3_DQS_P,
	HPS_DDR3_ODT,
	HPS_DDR3_RAS_N,
	HPS_DDR3_RESET_N,
	HPS_DDR3_RZQ,
	HPS_DDR3_WE_N
);

//=======================================================
//  PARAMETER declarations
//=======================================================


//=======================================================
//  PORT declarations
//=======================================================

////////////////////////////////////
// FPGA Pins
////////////////////////////////////

// Clock pins
input wire				CLOCK_50;

// Pushbuttons
input			[ 3: 0]	KEY;
//wire						key_reset;
//assign KEY = {3'b0, key_reset};

// LEDs
output		[ 9: 0]	LEDR;
assign LEDR[9] = ~KEY[0];

// Slider Switches
input			[ 9: 0]	SW;
//wire			[ 1: 0]	switches2_aux;
//assign SW = {switches2_aux, 8'b0};

// On-Chip RAM 0 s2 (read)
wire			[16: 0]	ocm0_addr;
wire						ocm0_chip;
wire						ocm0_clk_enab;
wire						ocm0_write;
wire			[ 7: 0]	ocm0_readdata;
wire			[ 7: 0]	ocm0_writedata;

// On-Chip RAM 1 s1 (write)
wire			[16: 0]	ocm1_addr;
wire						ocm1_chip;
wire						ocm1_clk_enab;
wire						ocm1_write;
wire			[ 7: 0]	ocm1_readdata;
wire			[ 7: 0]	ocm1_writedata;

// readOCM wires
wire						start_ocm_wire;
wire						finish_wire;



// Convolution wires
wire 						ocm_enable_wire;
wire 			[ 5: 0]	       ocm_data_wire;
wire						ocm_dv_wire;

wire						out_dv_wire;



readOCM(
	.clk					(CLOCK_50),
	.reset				(~KEY[0]),
	.start				(start_ocm_wire),
	.finish				(finish_wire),
	
	.ocm0_readdata		(ocm0_readdata),
	.ocm0_addr			(ocm0_addr),
	.ocm0_chip			(ocm0_chip),
	.ocm0_clk_enab		(ocm0_clk_enab),
	
	.data_out			(ocm_data_wire),
	.enable				(ocm_enable_wire),
	.in_dv				(ocm_dv_wire)
);

// Convolution HADDOC2
tenary_adder(
	.clk							(CLOCK_50),
	.rst_n						(KEY[0]),
	//.enable						(ocm_enable_wire),
	.data_in						(ocm_data_wire),
	//.in_dv						(ocm_dv_wire),
	
	.data_out					(out_data_wire),
      
	

);



wire     [5:0]	out_data_wire; // writeAuxDataOCM
//wire 			[7:0]	out_data_wire; // writeAuxDataOCM2
wire [15:0]			count_write_wire;

writeAuxDataOCM(
	.clk						(CLOCK_50),
	.reset					(~KEY[0]),
	
	.in_dv					(ocm_dv_wire),
	.out_dv					(out_dv_wire),
	
	//.out_data_aux			(ocm_data_wire),		// direct connection
	//.out_data_aux			(mux_data_out_wire), // tensorExtrator
	//.out_data_aux			(out_aux_data_wire), // dotProduct
	.out_data_aux			(out_data_wire),			// tanhLayer

	
	// On-Chip RAM 1 s1 (write)
	.ocm1_writedata		(ocm1_writedata),
	.ocm1_addr				(ocm1_addr),
	.ocm1_chip				(ocm1_chip),
	.ocm1_clk_enab			(ocm1_clk_enab),
	.ocm1_write				(ocm1_write),
	
	.count					(count_write_wire)
);


////////////////////////////////////
// HPS Pins
////////////////////////////////////

// DDR3 SDRAM
output		[14: 0]	HPS_DDR3_ADDR;
output		[ 2: 0]  HPS_DDR3_BA;
output					HPS_DDR3_CAS_N;
output					HPS_DDR3_CKE;
output					HPS_DDR3_CK_N;
output					HPS_DDR3_CK_P;
output					HPS_DDR3_CS_N;
output		[ 3: 0]	HPS_DDR3_DM;
inout			[31: 0]	HPS_DDR3_DQ;
inout			[ 3: 0]	HPS_DDR3_DQS_N;
inout			[ 3: 0]	HPS_DDR3_DQS_P;
output					HPS_DDR3_ODT;
output					HPS_DDR3_RAS_N;
output					HPS_DDR3_RESET_N;
input						HPS_DDR3_RZQ;
output					HPS_DDR3_WE_N;


cnn_hps_system The_System (
	////////////////////////////////////
	// FPGA Side
	////////////////////////////////////

	// Global signals
	.clk_clk 									(CLOCK_50),
	//.reset_reset_n								(1'b1),
	.reset_reset_n								(KEY[0]),
	
	// PIO to start the image reading from OCM
	.image_sent_ocm_export					(start_ocm_wire),
	
	// On-Chip RAM 0 s2 (read)
  .onchip_memory2_0_s2_address		(ocm0_addr),		//" Input 16 bits address"
  .onchip_memory2_0_s2_chipselect	(ocm0_chip),		//" Input 1 bit chipselect"
  .onchip_memory2_0_s2_clken			(ocm0_clk_enab),	//" Input 1 bit clken"
  .onchip_memory2_0_s2_write			(ocm0_write),		//" Input 1 bit write"
  .onchip_memory2_0_s2_readdata		(ocm0_readdata),	//" Output 8 bits readdata"
  .onchip_memory2_0_s2_writedata		(ocm0_writedata),	//" Input 8 bits writedata"
  
  	// On-Chip RAM 1 s1 (write)
  .onchip_memory2_1_s1_address		(ocm1_addr),		//" Input 16 bits address"
  .onchip_memory2_1_s1_chipselect	(ocm1_chip),		//" Input 1 bit chipselect"
  .onchip_memory2_1_s1_clken			(ocm1_clk_enab),	//" Input 1 bit clken"
  .onchip_memory2_1_s1_write			(ocm1_write),		//" Input 1 bit write"
  .onchip_memory2_1_s1_readdata		(ocm1_readdata),	//" Output 8 bits readdata"
  .onchip_memory2_1_s1_writedata		(ocm1_writedata),	//" Input 8 bits writedata"

	
	////////////////////////////////////
	// HPS Side
	////////////////////////////////////
	// DDR3 SDRAM
	.memory_mem_a			(HPS_DDR3_ADDR),
	.memory_mem_ba			(HPS_DDR3_BA),
	.memory_mem_ck			(HPS_DDR3_CK_P),
	.memory_mem_ck_n		(HPS_DDR3_CK_N),
	.memory_mem_cke		(HPS_DDR3_CKE),
	.memory_mem_cs_n		(HPS_DDR3_CS_N),
	.memory_mem_ras_n		(HPS_DDR3_RAS_N),
	.memory_mem_cas_n		(HPS_DDR3_CAS_N),
	.memory_mem_we_n		(HPS_DDR3_WE_N),
	.memory_mem_reset_n	(HPS_DDR3_RESET_N),
	.memory_mem_dq			(HPS_DDR3_DQ),
	.memory_mem_dqs		(HPS_DDR3_DQS_P),
	.memory_mem_dqs_n		(HPS_DDR3_DQS_N),
	.memory_mem_odt		(HPS_DDR3_ODT),
	.memory_mem_dm			(HPS_DDR3_DM),
	.memory_oct_rzqin		(HPS_DDR3_RZQ)
);


endmodule
