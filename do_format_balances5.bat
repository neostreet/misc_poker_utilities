@echo off
call balances5 > balances5.out
format_balances5 "2023" balances5.out > format_balances5.out
fixtext format_balances5.out
call mydl out
