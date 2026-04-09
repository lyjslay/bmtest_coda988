#!/usr/bin/perl
use Data::Dumper;
use strict;
use warnings;

# Load Excel Register Name & Default Value (Initialize)
my $InstanceName;
my $FileName;
my $RegName;
my %addr;
my %msb;
my %lsb;
my %bit;
my %default;
my $TableName;
my $RegarrayName;
my $isp_ini_mode = 0;
my $isp_ini_show_table_info;
my $isp_ini_show_regarray_info;

my $fptf;
my $fptw;
my $fptr;
my $fptr_list;
my $fptr_ini;

my $change_value_cnt = 0;
open($fptf,"<register.fm") || die "Can't open register.fm\n";

if(@ARGV==0){
  $isp_ini_mode = 1;
}


if ($isp_ini_mode == 1) {
  printf "no argument register.list not found, isp.ini parsing ... \n";
  open($fptr_ini,"<isp.ini") || die "Can't open isp.ini\n";
} else {
  printf "========== excel List file : $ARGV[0] ===============\n";
}

my $fm_err_flag = 0;
while(<$fptf>){
#  if ($_ =~ "^([a-z_A-Z\.0-9]*)[\s\+]0x([a-f0-9]*)[\s\+]([0-9]*)[\s\+]([0-9]*)[\s\+]([0-9]*)[\s\+]0x([0-9a-f]*)") { # (name addr msb lsb bit default)
  if ($_ =~ /^([a-z_A-Z\.0-9]*)\s+0x([a-f0-9]*)\s+([0-9]*)\s+([0-9]*)\s+([0-9]*)\s+0x([0-9a-f]*)/) { # (name addr msb lsb bit default)
  ##printf "~~~~~~~~~~ INFO:$1/$2/$3/$4/$5/$6===============\n";
    $addr{$1} = $2;
    $msb{$1} = $3;
    $lsb{$1} = $4;
    $bit{$1} = $5;
    $default{$1} = hex($6);
  }
  elsif ($_ =~ /\/\//){
    #printf "comment ... $_\n";
  }
  else {
    printf "not match expression ... $_";
    $fm_err_flag = 1;
  }
}

if ($fm_err_flag != 0) {
    printf "fm has error inside, please check log & register.fm, THANKS.\n";
    return ;
}

if (-e "register.cfg") {
  `mv register.cfg register.cfg.bak`;
  open($fptw,">register.cfg") || die "Can't open register.cfg\n";
} else {
  open($fptw,">register.cfg") || die "Can't open register.cfg\n";
}

if ($isp_ini_mode == 1) {

$isp_ini_show_table_info = 0;
$isp_ini_show_regarray_info = 0;
my @inst_a;
my $reg_name;
my $section;
my @table_values_a;
my @regarray_values_a;

  while (<$fptr_ini>) {
    #-----------------------------------------------------
    #  Parse sections
    #-----------------------------------------------------

    printf ("[INFO] process $_\n");

    if ($_ =~ "\\[(.*)\\]"){
      $section = remove_spaces($1);
      @inst_a = split("&",$section);
      $InstanceName = $1;
    }

    next if($section eq "isp_info");
    next if($section eq "sw");

    if ($_ =~ "(.*) = <<<END_OF_TEXT") {
      $isp_ini_show_table_info = 1;
      @table_values_a = ();
      $reg_name = $1;
    } elsif ($_ =~ "^END_OF_TEXT") {
      $isp_ini_show_table_info = 0;
      &write_table(\@table_values_a,\@inst_a,$reg_name);
    } else {
      if ($isp_ini_show_table_info == 0) {
        if ($_ =~ "^([0-9a-zA-Z_]*) *= h([0-9a-fA-F]*).*") {
          my $reg_name = $1;
          my $reg_value = $2;
          print_cfg_per_inst($reg_name,$reg_value,\@inst_a,"hex");
        } elsif ($_ =~ "^([0-9a-zA-Z_]*) *= (-[0-9]*).*") {
          my $reg_name = $1;
          my $reg_value = $2;
          print_cfg_per_inst($reg_name,$reg_value,\@inst_a,"negadeci");
        } elsif ($_ =~ "^([0-9a-zA-Z_]*) *= ([0-9]*).*") {
          my $reg_name = $1;
          my $reg_value = $2;
          print_cfg_per_inst($reg_name,$reg_value,\@inst_a,"deci");
        }
      } elsif ($isp_ini_show_table_info == 1) {
        if ($_ =~ "[0-9]*") {
          my @value_a = split(',', $_);
          pop @value_a; #there is an additional comma at each end of line
          push (@table_values_a,@value_a);
        }
      }
    } #else
  } #while (<fptr_ini>)
} else {

$isp_ini_show_table_info = 0;
$isp_ini_show_regarray_info = 0;
my @inst_a;
my $reg_name;
my $section;
my @table_values_a;
my @regarray_values_a;

  open($fptr_list,"<$ARGV[0]") || die "Can't open $ARGV[0]\n";

  while(<$fptr_list>){
    my @inst_a;
    $_ =~ s/\n//; # remove '\n'
    $FileName = $_;
    if (-e $_) {
      if ($FileName =~ "^([a-zA-Z0-9_&]*)\.ini") {
        @inst_a = split("&",$1);
      }
    }

    # Load Register Initial File
    open($fptr,"<$FileName ") || die "Can't open $FileName\n";

    printf "Parsing $FileName ..... Done\n";
    foreach my $InstanceName (@inst_a) {
      printf "Instance Name $InstanceName\n";
    }

    while (<$fptr>) {
      $_ = remove_pond_comment($_);
      #-----------------------------
      #  Parse section if there is
      #    ex: [isp_blc0&isp_blc1]
      #    ex: [isp_blc0]
      #-----------------------------
      if ($_ =~ "\\[([a-zA-Z0-9_&]*)\\].*") {
        my $section = remove_spaces($1);
        @inst_a = split("&",$section);
      } elsif ($_ =~ "(.*) = <<<END_OF_REGARRAY") {
      #-----------------------------
      #  Parse content with regbank type table
      #-----------------------------
        $isp_ini_show_table_info = 1;
        @table_values_a = ();
        $reg_name = $1;
      } elsif ($_ =~ "^END_OF_REGARRAY") {
        $isp_ini_show_table_info = 0;
        &write_regarray(\@table_values_a,\@inst_a,$reg_name);
      } elsif ($isp_ini_show_table_info == 1) {
        if ($_ =~ "[0-9]*") {
          my @value_a = split(',', $_);
          pop @value_a; #there is an additional comma at each end of line
          push (@table_values_a,@value_a);
        }
      #-----------------------------
      #  Parse content with hex format
      #    ex: reg_blc_address = h89ab
      #-----------------------------
           } elsif ($_ =~ "^([0-9a-zA-Z_]*) *= h([0-9a-fA-F]*).*") {
        my $reg_name = $1;
        my $reg_value = $2;
        print_cfg_per_inst($reg_name,$reg_value,\@inst_a,"hex");
      #-----------------------------
      #  Parse content with negative decimal format
      #    ex: reg_blc_address = -1234
      #-----------------------------
      } elsif ($_ =~ "^([0-9a-zA-Z_]*) *= (-[0-9]*).*") {
        my $reg_name = $1;
        my $reg_value = $2;
        print_cfg_per_inst($reg_name,$reg_value,\@inst_a,"negadeci");
      #-----------------------------
      #  Parse content with decimal format
      #    ex: reg_blc_address = 1234
      #-----------------------------
      } elsif ($_ =~ "^([0-9a-zA-Z_]*) *= ([0-9]*).*") {
        my $reg_name = $1;
        my $reg_value = $2;
        print_cfg_per_inst($reg_name,$reg_value,\@inst_a,"deci");
      }
    } # while
    close($fptr);
  } # if ($isp_hw_ini_mode == 1)
  close($fptr_list);
}

printf $fptw "xxxxxxxx_xxxxxxxx_xxxxxxxx // End_of_file\n";

close($fptw);
close($fptf);

sub  trim { my $s = shift; $s =~ s/^\s+|\s+$//g; return $s };

#&write_table(\@table_values_a,\@inst_a,$reg_name);
sub write_table {
  my @table_values_a = @{$_[0]};
  my @inst_a = @{$_[1]};
  my $reg_name = $_[2];
  my $fptw_table;

  foreach my $InstanceName (@inst_a) {
    $TableName = "$InstanceName\_\_$reg_name";
    $TableName = &trim($TableName);
    printf ("Generates Table %s.lut\n",$TableName);
    open($fptw_table,">$TableName.lut") || die "Can't open $TableName.lut\n";

    for my $i (0 .. $#table_values_a) {
      printf $fptw_table "%08x\n",$table_values_a[$i];
    }

    close($fptw_table);
    printf ("Close Table %s.lut\n",$TableName);
  }
}

#&write_regarray(\@regarray_values_a,\@inst_a,$reg_name);
sub write_regarray {
  my @regarray_values_a = @{$_[0]};
  my @inst_a = @{$_[1]};
  my $reg_name = $_[2];
  ##printf ("Regarray writing for %s.lut\n",$reg_name);

  foreach my $InstanceName (@inst_a) {
    ##$RegarrayName = "$InstanceName\_\_$reg_name";
    ##printf ("Generates Regarray %s.lut\n",$RegarrayName);

    for my $i (0 .. $#regarray_values_a) {
      ##printf ("[INFO\@write_regarray] %d\n",$regarray_values_a[$i]);
      ##printf ("[INFO\@write_regarray] %s, %d, %s\n",$reg_name, $i, $InstanceName);
      print_cfg_regarray($reg_name, $regarray_values_a[$i], $i, $InstanceName, "deci");
    }

    ##printf ("Close Regarray %s.lut\n",$RegarrayName);
  }
}

sub wr_config {
  my $fp = $_[0];
  my $addr_r = $_[1];
  my $data_r = $_[2];
  my $msb_r = $_[3];
  my $lsb_r = $_[4];
  my $bit_r = $_[5];
  my $name_r = $_[6];
  my $mask = 0;
  my $mask_1s;
  my $i_r;
  my $data_o = $_[2];

 if ($addr_r eq "") {
  #if ($addr_r == 0x00000000) {
    printf "// ======================================== //\n";
    printf "// ==== Parsing Warning : %s %s \n",$name_r, $addr_r;
    printf "// ======================================== //\n";
#    `rm register.cfg`;
#    exit;
  } elsif ($addr_r =~ /^00000000/) { # TODO: why bypassing 0x00000000 (?)
    for ($i_r=31 ; $i_r>=0 ; $i_r--) {
      if (($i_r >= $lsb_r) && ($i_r <= $msb_r)) {
        $mask++;
      }
      if ($i_r < $lsb_r) {
        $data_r=$data_r<<1;
      }
      if ($i_r != 0){
        $mask=$mask<<1;
      }
    }
    $mask_1s = 4294967295 - $mask;
    printf $fp "%08s_%08x_%08x // %s=h%08x\n",$addr_r,$mask_1s,($data_r & $mask),$name_r,$data_o;
  } else {
    for ($i_r=31 ; $i_r>=0 ; $i_r--) {
      if (($i_r >= $lsb_r) && ($i_r <= $msb_r)) {
        $mask++;
      }
      if ($i_r < $lsb_r) {
        $data_r=$data_r<<1;
      }
      if ($i_r != 0){
        $mask=$mask<<1;
      }
    }
    $mask_1s = 4294967295 - $mask;
    printf $fp "%08s_%08x_%08x // %s=h%08x\n",$addr_r,$mask_1s,($data_r & $mask),$name_r,$data_o;
#  printf $fp "  bench_top_0.apb_r(.addr(32'h%08s),.rdata(rdata));\n",$addr_r;
#  printf $fp "  wdata = (rdata&(32'h%08x)) | 32'h%08x;\n",$mask_1s,($data_r & $mask);
#  printf $fp "  bench_top_0.apb_w(.addr(32'h%08s),.be(4'b1111),.wdata(wdata));\n",$addr_r;
  }
}
sub remove_pond_comment{
  my $in = $_[0];
  $in =~ s/^#.*//g; # remove commnet
  return $in;
}

sub remove_spaces{
  my $in = $_[0];
  $in =~ s/\s//g; #remove spaces
  return $in;
}

sub wr_config_start {
  my $fp = $_[0];
  printf $fp "task task_reg_ini;\n";
  printf $fp "reg [31:0] wdata;\n";
  printf $fp "reg [31:0] rdata;\n";
  printf $fp "begin\n";
}

sub wr_config_end {
  my $fp = $_[0];
  printf $fp "end\n";
  printf $fp "endtask";
}

sub print_cfg_per_inst{
  my $reg_name = $_[0];
  my $reg_value = $_[1];
  my @inst_a = @{$_[2]};
  my $hex_or_dec = $_[3];

  foreach my $InstanceName (@inst_a) {
    $RegName = "$InstanceName.$reg_name";

    if(!check_reg($RegName)){
      return;
    }

    printf "Value Change (%3d) : $RegName from h%x ",$change_value_cnt,$default{$RegName};
    if($hex_or_dec eq "hex"){
      $default{$RegName} = hex($reg_value);
    }elsif($hex_or_dec eq "negadeci"){
      $default{$RegName} = $reg_value;
    }elsif($hex_or_dec eq "deci"){
      $default{$RegName} = $reg_value;
    }else{
      printf "[FATAL]forth argument of function of print_cfg_per_inst should be string 'deci' or 'hex' but $hex_or_dec \n";
      exit(0);
    }
    printf "to h%x\n",$default{$RegName};
    &wr_config($fptw,$addr{$RegName},$default{$RegName},$msb{$RegName},$lsb{$RegName},$bit{$RegName},$RegName);
    $change_value_cnt++;
  }
}

sub print_cfg_regarray{
  my $reg_name = $_[0];
  my $reg_value = $_[1];
  my $reg_index = $_[2];
  my @inst_a = ($_[3]);
  my $hex_or_dec = $_[4];

  foreach my $InstanceName (@inst_a) {
    $RegName = "$InstanceName.$reg_name$reg_index";

    if(!check_reg($RegName)){
      printf("$RegName is not found.\n");
      return;
    }

    printf "Value Change (%3d) : $RegName from h%x ",$change_value_cnt,$default{$RegName};
    if($hex_or_dec eq "hex"){
      $default{$RegName} = hex($reg_value);
    }elsif($hex_or_dec eq "deci"){
      $default{$RegName} = $reg_value;
    }else{
      printf "[FATAL]forth argument of function of print_cfg_per_inst should be string 'deci' or 'hex' but $hex_or_dec \n";
      exit(0);
    }
    printf "to h%x\n",$default{$RegName};
    &wr_config($fptw,$addr{$RegName},$default{$RegName},$msb{$RegName},$lsb{$RegName},$bit{$RegName},$RegName);
    $change_value_cnt++;
  }
}

sub check_reg{
  if (!exists $default{$RegName}) {
    printf "// ======================================== //\n";
    printf "// ==== Parsing Warning : %s is not found in register.fm \n",$RegName;
    printf "// ======================================== //\n";
    return 0;
  }
  if (!exists $addr{$RegName}) {
    printf "// ======================================== //\n";
    printf "// ==== Parsing Warning : %s is not found in addr hash table \n",$RegName;
    printf "// ======================================== //\n";
    return 0;
  }
  return 1;
}

#vim: ts=2: expandtab: