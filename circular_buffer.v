`timescale 1ns/1ps

module circular_buffer(clk, rst_n, data_in, weight_in, buffer_weight_fire, buffer_data_fire, buffer_done, x11, x12, x13, x21, x22, x23, x31, x32, x33, w11, w12, w13, w21, w22, w23, w31,w32, w33
                      );

parameter [4:0] TI = 8;
parameter [8:0] INPUT_SIZE = 16;
parameter [10:0] INPUT_CHANNEL = 3;
parameter [3:0] ADDR_BITS = 4;

localparam ITERATION_TIMES = INPUT_CHANNEL/TI;
localparam INPUT_DEPTH = 4*ITERATION_TIMES*INPUT_SIZE;
localparam INPUT_COLS = TI/4;

input clk;
input rst_n;
input [23:0] data_in;
input [8:0] weight_in;
input buffer_data_fire;
input buffer_weight_fire;
input buffer_done;

output [5:0] x11;
output [5:0] x12;
output [5:0] x13;
output [5:0] x21;
output [5:0] x22;
output [5:0] x23;
output [5:0] x31;
output [5:0] x32;
output [5:0] x33;
output w11;
output w12;
output w13;
output w21;
output w22;
output w23;
output w31;
output w32;
output w33;

reg [8:0] weight_buffer [TI-1:0];
reg [23:0] data_buffer [INPUT_DEPTH-1:0][INPUT_COLS-1:0];
reg [ADDR_BITS-1:0] data_out_pointer;
reg [ADDR_BITS+7:0] data_in_pointer;
reg [4:0] weight_in_pointer; 
reg [4:0] out_counter;
reg [2:0] in_counter;
reg [5:0] x11;
reg [5:0] x12;
reg [5:0] x13;
reg [5:0] x21;
reg [5:0] x22;
reg [5:0] x23;
reg [5:0] x31;
reg [5:0] x32;
reg [5:0] x33;
reg [TI*6-1:0] x11_reg;
reg [TI*6-1:0] x12_reg;
reg [TI*6-1:0] x13_reg;
reg [TI*6-1:0] x21_reg;
reg [TI*6-1:0] x22_reg;
reg [TI*6-1:0] x23_reg;
reg [TI*6-1:0] x31_reg;
reg [TI*6-1:0] x32_reg;
reg [TI*6-1:0] x33_reg;
reg [8:0] kernel_reg;


//data_in buffer
always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     out_counter <= 5'd0;
   else if(out_counter == TI)
     out_counter <= 5'd1;
   else if(buffer_done)
     out_counter <= out_counter + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     in_counter <= 3'd0;
   else if(in_counter == INPUT_COLS-1)
     in_counter <= 3'd0;
   else if(buffer_data_fire)
     in_counter <= in_counter + 1'b1;
end


always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     data_out_pointer <= 0;
   else if (out_counter == TI)
     data_out_pointer <= data_out_pointer + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     data_in_pointer <= 0;
   else if(buffer_data_fire & (in_counter == INPUT_COLS-1))
     data_in_pointer <= data_in_pointer + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   else if (buffer_data_fire)
      data_buffer[data_in_pointer][in_counter] <= data_in;
end

integer i;

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
    begin
     x11_reg <= 0;
     x12_reg <= 0;
     x13_reg <= 0;
     x21_reg <= 0;
     x22_reg <= 0;
     x23_reg <= 0;
     x31_reg <= 0;
     x32_reg <= 0;
     x33_reg <= 0;
    end
   else if(buffer_done)
     x11_reg[23:0] <= data_buffer[data_out_pointer][0];
     x11_reg[47:24] <= data_buffer[data_out_pointer][1];
     x12_reg[23:0] <= data_buffer[data_out_pointer+1][0];
     x12_reg[47:24] <= data_buffer[data_out_pointer+1][1];
     x13_reg[23:0] <= data_buffer[data_out_pointer+2][0];
     x13_reg[47:24] <= data_buffer[data_out_pointer+2][1];
     x21_reg[23:0] <= data_buffer[data_out_pointer+INPUT_SIZE][0];
     x21_reg[47:24] <= data_buffer[data_out_pointer+INPUT_SIZE][1];
     x22_reg[23:0] <= data_buffer[data_out_pointer+INPUT_SIZE+1][0];
     x22_reg[47:24] <= data_buffer[data_out_pointer+INPUT_SIZE+1][1];
     x23_reg[23:0] <= data_buffer[data_out_pointer+INPUT_SIZE+2][0];
     x23_reg[47:24] <= data_buffer[data_out_pointer+INPUT_SIZE+2][1];
     x31_reg[23:0] <= data_buffer[data_out_pointer+INPUT_SIZE+INPUT_SIZE][0];
     x31_reg[47:24] <= data_buffer[data_out_pointer+INPUT_SIZE+INPUT_SIZE][1];
     x32_reg[23:0] <= data_buffer[data_out_pointer+INPUT_SIZE+INPUT_SIZE+1][0];
     x32_reg[47:24] <= data_buffer[data_out_pointer+INPUT_SIZE+INPUT_SIZE+1][1];
     x33_reg[23:0] <= data_buffer[data_out_pointer+INPUT_SIZE+INPUT_SIZE+2][0];
     x33_reg[47:24] <= data_buffer[data_out_pointer+INPUT_SIZE+INPUT_SIZE+2][1];
end


always@(buffer_done or data_out_pointer or out_counter)
begin
   if(buffer_done)
     case(out_counter)  
        5'd1 :
              begin  
                x11 = x11_reg[5:0]; 
                x12 = x12_reg[5:0]; 
                x13 = x13_reg[5:0]; 
                x21 = x21_reg[5:0]; 
                x22 = x22_reg[5:0]; 
                x23 = x23_reg[5:0]; 
                x31 = x31_reg[5:0]; 
                x32 = x32_reg[5:0]; 
                x33 = x33_reg[5:0]; 
              end
        5'd2 : 
              begin
                x11 = x11_reg[11:6];
                x12 = x12_reg[11:6];
                x13 = x13_reg[11:6];
                x21 = x21_reg[11:6];
                x22 = x22_reg[11:6];
                x23 = x23_reg[11:6];
                x31 = x31_reg[11:6];
                x32 = x32_reg[11:6];
                x33 = x33_reg[11:6];
              end
        5'd3 : 
              begin
                x11 = x11_reg[17:12]; 
                x12 = x12_reg[17:12]; 
                x13 = x13_reg[17:12]; 
                x21 = x21_reg[17:12]; 
                x22 = x22_reg[17:12]; 
                x23 = x23_reg[17:12]; 
                x31 = x31_reg[17:12]; 
                x32 = x32_reg[17:12]; 
                x33 = x33_reg[17:12]; 
              end
        5'd4 : 
              begin
                x11 = x11_reg[23:18]; 
                x12 = x12_reg[23:18]; 
                x13 = x13_reg[23:18]; 
                x21 = x21_reg[23:18]; 
                x22 = x22_reg[23:18]; 
                x23 = x23_reg[23:18]; 
                x31 = x31_reg[23:18]; 
                x32 = x32_reg[23:18]; 
                x33 = x33_reg[23:18]; 
              end
        5'd5 : 
              begin
                x11 = x11_reg[29:24]; 
                x12 = x12_reg[29:24]; 
                x13 = x13_reg[29:24]; 
                x21 = x21_reg[29:24]; 
                x22 = x22_reg[29:24]; 
                x23 = x23_reg[29:24]; 
                x31 = x31_reg[29:24]; 
                x32 = x32_reg[29:24]; 
                x33 = x33_reg[29:24]; 
              end
        5'd6 : 
              begin
                x11 = x11_reg[35:30]; 
                x12 = x12_reg[35:30]; 
                x13 = x13_reg[35:30]; 
                x21 = x21_reg[35:30]; 
                x22 = x22_reg[35:30]; 
                x23 = x23_reg[35:30]; 
                x31 = x31_reg[35:30]; 
                x32 = x32_reg[35:30]; 
                x33 = x33_reg[35:30]; 
              end
        5'd7 : 
              begin
                x11 = x11_reg[41:36]; 
                x12 = x12_reg[41:36]; 
                x13 = x13_reg[41:36]; 
                x21 = x21_reg[41:36]; 
                x22 = x22_reg[41:36]; 
                x23 = x23_reg[41:36]; 
                x31 = x31_reg[41:36]; 
                x32 = x32_reg[41:36]; 
                x33 = x33_reg[41:36]; 
              end
        5'd8 : 
              begin
                x11 = x11_reg[47:42]; 
                x11 = x12_reg[47:42]; 
                x11 = x13_reg[47:42]; 
                x21 = x21_reg[47:42]; 
                x22 = x22_reg[47:42]; 
                x23 = x23_reg[47:42]; 
                x31 = x31_reg[47:42]; 
                x32 = x32_reg[47:42]; 
                x33 = x33_reg[47:42]; 
              end
        default 
              begin
                x11 = 6'd0;
                x12 = 6'd0;
                x13 = 6'd0;
                x21 = 6'd0;
                x22 = 6'd0;
                x23 = 6'd0;
                x31 = 6'd0;
                x32 = 6'd0;
                x33 = 6'd0;
              end
     endcase
   else
     begin
        x11 = 6'd0;
        x12 = 6'd0;
        x13 = 6'd0;
        x21 = 6'd0;
        x22 = 6'd0;
        x23 = 6'd0;
        x31 = 6'd0;
        x32 = 6'd0;
        x33 = 6'd0;
     end
end

//weight_in buffer

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     weight_in_pointer <= 0;
   else if(buffer_weight_fire & (weight_in_pointer != TI))
     weight_in_pointer <= weight_in_pointer + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
   begin
      weight_buffer[0] <= 9'd0;
      weight_buffer[1] <= 9'd0;
      weight_buffer[2] <= 9'd0;
   end
   else if (buffer_weight_fire & (weight_in_pointer != TI))
      weight_buffer[weight_in_pointer] <= weight_in;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     kernel_reg <= 9'd0;
   else if(out_counter!=TI)
     kernel_reg <= weight_buffer[out_counter];
   else if(out_counter==TI)
     kernel_reg <= weight_buffer[0];
end

assign w11 = buffer_done ? kernel_reg[0] : 1'b0;
assign w12 = buffer_done ? kernel_reg[1] : 1'b0;
assign w13 = buffer_done ? kernel_reg[2] : 1'b0;
assign w21 = buffer_done ? kernel_reg[3] : 1'b0;
assign w22 = buffer_done ? kernel_reg[4] : 1'b0;
assign w23 = buffer_done ? kernel_reg[5] : 1'b0;
assign w31 = buffer_done ? kernel_reg[6] : 1'b0;
assign w32 = buffer_done ? kernel_reg[7] : 1'b0;
assign w33 = buffer_done ? kernel_reg[8] : 1'b0;

endmodule










