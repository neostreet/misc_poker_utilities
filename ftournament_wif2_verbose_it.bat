@echo off
ftournament_wif -verbose \aidan\pokerstars\50000a %1 > %1.verbose.ftournament_wif2
call mydl ftournament_wif2
