@echo off
ftournament_wif -verbose \aidan\pokerstars\50000b %1 > %1.verbose.ftournament_wif1
call mydl ftournament_wif1
