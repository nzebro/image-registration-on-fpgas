if {$argc != 4} {
  puts "Expected: <project_to_synthesize> <prj_dir> <prj_name> <top>"
  exit
}

set prj_dir [lindex $argv 1]
set prj_name [lindex $argv 2]
set top [lindex $argv 3]


open_project [lindex $argv 0]
launch_runs synth_1 -jobs 2
wait_on_run synth_1
launch_runs impl_1 -to_step write_bitstream -jobs 2
wait_on_run impl_1
file copy -force $prj_dir/$prj_name.runs/impl_1/$top.sysdef $prj_dir/$top.hdf

