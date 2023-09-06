@echo off
call balances3 > balances3.out
format_balances3 2023 balances3.out > format_balances3.out
fixtext format_balances3.out
call mydl out
