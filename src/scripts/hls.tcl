set proj_name      [lindex $argv 2]
set src_dir          [lindex $argv 3]
set tb_file [lindex $argv 4]
set proj_part      [lindex $argv 5]
set clk      [lindex $argv 6]
set toplevel    [lindex $argv 7]
set incldirs       [lindex $argv 8]
set steps [lindex $argv 9]

if {$steps == 0} {
    set opts "PRJ"
}
if {$steps == 1} {
    set opts "SIM"
}
if {$steps == 2} {
    set opts "SYNTH"
}
if {$steps == 3} {
    set opts "COSIM"
}
if {$steps == 4} {
    set opts "IP EXPORT"
}
if {$steps == 5} {
    set opts "SYNTH and IP"
}

puts ""
puts "***************************************************************"
puts ""
puts "HLS project: $proj_name"
puts "HLS sources files: $src_dir"
puts "Target platform: $proj_part"
puts "Clock period: $clk ns"
puts "Top level function: $toplevel"
puts "Include directories: $incldirs"
puts "HLS options: $steps, $opts"
puts ""
puts "***************************************************************"
puts ""
open_project $proj_name
set_top $toplevel
add_files $src_dir -cflags "-std=c++0x -I$incldirs"
puts "add_files $src_dir -cflags \"-std=c++0x -I$incldirs\""
add_files -tb $tb_file
open_solution "solution1"
set_part $proj_part
create_clock -period $clk -name default
if {$steps == 1} {
    csim_design -clean
}
if {$steps == 2} {
    csynth_design
}
if {$steps == 3} {
    cosim_design
}
if {$steps == 4} {
    export_design -rtl verilog -format ip_catalog
}
if {$steps == 5} {
    csynth_design
    export_design -rtl verilog -format ip_catalog
}
close_project
exit 0
