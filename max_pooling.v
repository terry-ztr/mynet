`timescale 1ns/1ps

module max_pooling(clk, rst_n, fire, data_in, row, data_out,done);

parameter [8:0] INPUT_SIZE = 16;
parameter [3:0] ADDR_BITS = 4;
localparam [7:0] LINE_SIZE = INPUT_SIZE/2;


input clk;
input rst_n;
input [5:0] data_in;
input row;
input fire;
output reg [5:0] data_out;
output reg done;

reg [5:0] line_buffer [LINE_SIZE-1:0];
reg [5:0] input_buffer;
reg pingpong;
reg [ADDR_BITS-1:0] pointer;

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     pingpong <= 1'b0;
   else if(fire)
     pingpong <= !pingpong;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     input_buffer <= 6'b0;
   else if((!pingpong) & fire)
     input_buffer <= data_in;
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     pointer <= 0;
   else if((pointer==LINE_SIZE-1) & pingpong & fire)
     pointer <= 0;
   else if(pingpong & fire)  
     pointer <= pointer + 1'b1;
end

always@(posedge clk or negedge rst_n)
begin
   if(pingpong & (!row) & fire)
       line_buffer[pointer] <= (input_buffer >= data_in) ? input_buffer : data_in;
end

always@(posedge clk or negedge rst_n)
begin 
   if(!rst_n)
     data_out <= 6'd0;
   else if(row & pingpong & fire)
     data_out <= (line_buffer[pointer] >= ((input_buffer >= data_in) ? input_buffer : data_in)) ? line_buffer[pointer] : ((input_buffer >= data_in) ? input_buffer : data_in);
end

always@(posedge clk or negedge rst_n)
begin
   if(!rst_n)
     done <= 1'b0;
   else if(row & (pointer==0) & pingpong)
     done <= 1'b1;
   else if(row & (pointer==LINE_SIZE-1) & pingpong)
     done <= 1'b0;
end 

endmodule




