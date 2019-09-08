#TCL for project creation

if {$argc < 5} {
  puts "Expected: <prj_root> <proj name> <proj dir> <ip_dir> <freq>"
  exit
}


set prj_root  [lindex $argv 0]
set prj_root_build "$prj_root/build/"
set prj_name [lindex $argv 1]
set prj_dir [lindex $argv 2]
set ip_dir [lindex $argv 3]
set trgt_freq [lindex $argv 4]
# fixed for platform
set prj_part "xczu3eg-sbva484-1-i"
set xdc_dir "$prj_root/src/scripts/ultra_96/building"

puts "$ip_dir"
puts "$xdc_dir"
# set up project
create_project $prj_name $prj_dir -part $prj_part
update_ip_catalog

#Add Ultra96 XDC
add_files -fileset constrs_1 -norecurse "${xdc_dir}/ultra96.xdc"
update_compile_order -fileset sources_1

 
#add hls ip
set_property  ip_repo_paths $ip_dir [current_project]
update_ip_catalog

# create block design
create_bd_design "iron"



source "${xdc_dir}/ultra96.tcl"

#enable AXI HP ports
set_property -dict [list CONFIG.PSU__USE__M_AXI_GP0 {1} CONFIG.PSU__USE__S_AXI_GP2 {1}] [get_bd_cells zynq_ultra_ps_e_0]
set_property -dict [list CONFIG.PSU__SAXIGP2__DATA_WIDTH {128}] [get_bd_cells zynq_ultra_ps_e_0]

set clkdiv [expr { int(1500/$trgt_freq) }]
set_property -dict [list CONFIG.PSU__CRL_APB__PL2_REF_CTRL__DIVISOR0 $clkdiv] [get_bd_cells zynq_ultra_ps_e_0]
#instantiate iron
create_bd_cell -type ip -vlnv xilinx.com:hls:mutual_information_master:1.0 mutual_information_m_0

apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { Clk_master {/zynq_ultra_ps_e_0/pl_clk2 (99 MHz)} Clk_slave {/zynq_ultra_ps_e_0/pl_clk2 (99 MHz)} Clk_xbar {/zynq_ultra_ps_e_0/pl_clk2 (99 MHz)} Master {/mutual_information_m_0/m_axi_gmem} Slave {/zynq_ultra_ps_e_0/S_AXI_HP0_FPD} intc_ip {Auto} master_apm {0}}  [get_bd_intf_pins zynq_ultra_ps_e_0/S_AXI_HP0_FPD]
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { Clk_master {/zynq_ultra_ps_e_0/pl_clk2 (99 MHz)} Clk_slave {/zynq_ultra_ps_e_0/pl_clk2 (99 MHz)} Clk_xbar {/zynq_ultra_ps_e_0/pl_clk2 (99 MHz)} Master {/zynq_ultra_ps_e_0/M_AXI_HPM0_FPD} Slave {/mutual_information_m_0/s_axi_AXI_Lite_1} intc_ip {New AXI Interconnect} master_apm {0}}  [get_bd_intf_pins mutual_information_m_0/s_axi_AXI_Lite_1]
regenerate_bd_layout
validate_bd_design
save_bd_design

# create HDL wrapper
make_wrapper -files [get_files $prj_dir/$prj_name.srcs/sources_1/bd/iron/iron.bd] -top
add_files -norecurse $prj_dir/$prj_name.srcs/sources_1/bd/iron/hdl/iron_wrapper.v
update_compile_order -fileset sources_1
update_compile_order -fileset sim_1

set_property source_mgmt_mode None [current_project]
set_property top iron_wrapper [current_fileset]
set_property source_mgmt_mode All [current_project]
update_compile_order -fileset sources_1

set_property strategy Flow_PerfOptimized_high [get_runs synth_1]

set_property STEPS.SYNTH_DESIGN.ARGS.DIRECTIVE AlternateRoutability [get_runs synth_1]
set_property STEPS.SYNTH_DESIGN.ARGS.RETIMING true [get_runs synth_1]

set_property strategy Performance_ExtraTimingOpt [get_runs impl_1]
set_property STEPS.OPT_DESIGN.ARGS.DIRECTIVE Explore [get_runs impl_1]
set_property STEPS.POST_ROUTE_PHYS_OPT_DESIGN.ARGS.DIRECTIVE AggressiveExplore [get_runs impl_1]
set_property STEPS.PHYS_OPT_DESIGN.ARGS.DIRECTIVE AggressiveExplore [get_runs impl_1]
set_property STEPS.POST_ROUTE_PHYS_OPT_DESIGN.IS_ENABLED true [get_runs impl_1]
set_property STEPS.ROUTE_DESIGN.ARGS.DIRECTIVE AlternateCLBRouting [get_runs impl_1]
set_property STEPS.OPT_DESIGN.ARGS.DIRECTIVE ExploreWithRemap [get_runs impl_1]

write_bd_tcl $prj_dir/iron_wrapper.tcl