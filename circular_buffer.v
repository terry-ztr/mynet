`timescale 1ns/1ps

module circular_buffer(clk, rst_n, data_in, weight_in, buffer_weight_fire, buffer_data_fire, buffer_done, x11, x12, x13, x21, x22, x23, x31, x32, x33, w11, w12, w13, w21, w22, w23, w31,w32, w33
                      );

parameter [4:0] TI = 3;
parameter [8:0] INPUT_SIZE = 16;
parameter [10:0] INPUT_CHANNEL = 3;
parameter [3:0] ADDR_BITS = 4;

localparam ITERATION_TIMES = INPUT_CHANNEL/TI;
localparam INPUT_DEPTH = 4*ITERATION_TIMES*INPUT_SIZE;

input clk;
input rst_n;
input [6*TI-1:0] data_in;
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
reg [6*TI-1:0] data_buffer [INPUT_DEPTH-1:0];
reg [ADDR_BITS-1:0] data_out_pointer;
reg [ADDR_BITS+7:0] data_in_pointer;
reg [4:0] weight_in_pointer; 
reg [4:0] counter;
reg [5:0] x11;
reg [5:0] x12;
reg [5:0] x13;
reg [5:0] x21;
reg [5:0] x22;
reg [5:0] x23;
reg [5:0] x31;
reg [5:0] x32;
reg [5:0] x33;
reg [17:0] x11_reg;
reg [17:0] x12_reg;
reg [17:0] x13_reg;
reg [17:0] x21_reg;
reg [17:0] x22_reg;
reg [17:0] x23_reg;
reg [17:0] x31_reg;
reg [17:0] x32_reg;
reg [17:0] x33_reg;
reg [8:0] kernel_reg;


//data_in buffer
always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     counter <= 5'd0;
   else if(counter == TI)
     counter <= 5'd1;
   else if(buffer_done)
     counter <= counter + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     data_out_pointer <= 0;
   else if (counter == TI)
     data_out_pointer <= data_out_pointer + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     data_in_pointer <= 0;
   else if(buffer_data_fire)
     data_in_pointer <= data_in_pointer + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
   begin
      data_buffer[0] <= 18'd0;
      data_buffer[1] <= 18'd0;
      data_buffer[2] <= 18'd0;
      data_buffer[3] <= 18'd0;
      data_buffer[4] <= 18'd0;
      data_buffer[5] <= 18'd0;
      data_buffer[6] <= 18'd0;
      data_buffer[7] <= 18'd0;
      data_buffer[8] <= 18'd0;
      data_buffer[9] <= 18'd0;
      data_buffer[10] <= 18'd0;
      data_buffer[11] <= 18'd0;
      data_buffer[12] <= 18'd0;
      data_buffer[13] <= 18'd0;
      data_buffer[14] <= 18'd0;
      data_buffer[15] <= 18'd0;
   end
   else if (buffer_data_fire)
      data_buffer[data_in_pointer] <= data_in;
end

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
    begin
     x11_reg <= data_buffer[data_out_pointer];
     x12_reg <= data_buffer[data_out_pointer+1];
     x13_reg <= data_buffer[data_out_pointer+2];
     x21_reg <= data_buffer[data_out_pointer+INPUT_SIZE];
     x22_reg <= data_buffer[data_out_pointer+INPUT_SIZE+1];
     x23_reg <= data_buffer[data_out_pointer+INPUT_SIZE+2];
     x31_reg <= data_buffer[data_out_pointer+INPUT_SIZE+INPUT_SIZE];
     x32_reg <= data_buffer[data_out_pointer+INPUT_SIZE+INPUT_SIZE+1];
     x33_reg <= data_buffer[data_out_pointer+INPUT_SIZE+INPUT_SIZE+2];
    end
end


always@(buffer_done or data_out_pointer or counter)
begin
   if(buffer_done)
     case(counter)  
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
   else if(counter!=TI)
     kernel_reg <= weight_buffer[counter];
   else if(counter==TI)
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










