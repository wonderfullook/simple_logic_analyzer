module fifo245 #(

) (
    output wire debug,
    input wire [7 : 0] capture,
    output wire [7 : 0] data,
    input wire rxf_n,
    input wire txe_n,
    output wire rd_n,
    output reg wr_n,
    input wire clk,
    output wire oe_n,
    output wire siwu_n
);

assign oe_n = 'b1;
assign rd_n = 'b1;
assign siwu_n = 'b1;

localparam COUNTER_SIZE = 10;

reg [COUNTER_SIZE - 1:0] counter = 'b0;

reg [7:0] reg_data = 'b0;

initial begin
    wr_n = 1'b0;
end

assign debug = counter[COUNTER_SIZE - 1];

assign data = reg_data;

always @(posedge clk) begin
    if ((wr_n == 'b0) && (txe_n == 'b0))
        wr_n = 'b1;

    if (counter == 0) begin
        wr_n = 'b0;
        reg_data <= capture;
    end
    counter <= counter + 1'b1;
end

endmodule