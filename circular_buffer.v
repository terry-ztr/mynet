`timescale 1ns/1ps

module circular_buffer(clk, rst_n, data_in, fire, done, x11, x12, x13, x21, x22, x23, x31, x32, x33
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
input fire;
input done;

output [5:0] x11;
output [5:0] x12;
output [5:0] x13;
output [5:0] x21;
output [5:0] x22;
output [5:0] x23;
output [5:0] x31;
output [5:0] x32;
output [5:0] x33;


reg [6*TI-1:0] input_buffer [INPUT_DEPTH-1:0];
reg [ADDR_BITS-1:0] out_pointer;
reg [ADDR_BITS+7:0] in_pointer;
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

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     counter <= 5'd0;
   else if(counter == TI)
     counter <= 5'd1;
   else if(done)
     counter <= counter + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     out_pointer <= 0;
   else if (counter == TI)
     out_pointer <= out_pointer + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     in_pointer <= 0;
   else if(fire)
     in_pointer <= in_pointer + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
   begin
      input_buffer[0] <= 18'd0;
      input_buffer[1] <= 18'd0;
      input_buffer[2] <= 18'd0;
      input_buffer[3] <= 18'd0;
      input_buffer[4] <= 18'd0;
      input_buffer[5] <= 18'd0;
      input_buffer[6] <= 18'd0;
      input_buffer[7] <= 18'd0;
      input_buffer[8] <= 18'd0;
      input_buffer[9] <= 18'd0;
      input_buffer[10] <= 18'd0;
      input_buffer[11] <= 18'd0;
      input_buffer[12] <= 18'd0;
      input_buffer[13] <= 18'd0;
      input_buffer[14] <= 18'd0;
      input_buffer[15] <= 18'd0;
   end
   else if (fire)
      input_buffer[in_pointer] <= data_in;
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
   else if(done)
    begin
     x11_reg <= input_buffer[out_pointer];
     x12_reg <= input_buffer[out_pointer+1];
     x13_reg <= input_buffer[out_pointer+2];
     x21_reg <= input_buffer[out_pointer+INPUT_SIZE];
     x22_reg <= input_buffer[out_pointer+INPUT_SIZE+1];
     x23_reg <= input_buffer[out_pointer+INPUT_SIZE+2];
     x31_reg <= input_buffer[out_pointer+INPUT_SIZE+INPUT_SIZE];
     x32_reg <= input_buffer[out_pointer+INPUT_SIZE+INPUT_SIZE+1];
     x33_reg <= input_buffer[out_pointer+INPUT_SIZE+INPUT_SIZE+2];
    end
end



always@(done or out_pointer or counter)
begin
   if(done)
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


endmodule










