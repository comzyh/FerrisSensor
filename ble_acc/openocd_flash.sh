#!/usr/bin/expect

# This script is for flash NRF51 using openocd. Please install `expect` first.
# sudo apt install expect

set hostName [lindex $argv 0]
set port [lindex $argv 1]
set hexfile [lindex $argv 2]

spawn telnet $hostName $port

send "reset\n"
send "program ${hexfile}\n"
send "reset\n"
send "exit\n"

interact