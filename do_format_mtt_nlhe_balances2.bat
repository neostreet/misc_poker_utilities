@echo off
call mtt_nlhe_balances2 > mtt_nlhe_balances2.out
format_balances2 "MTT NLHE balance" "2020 balance" mtt_nlhe_balances2.out > format_mtt_nlhe_balances2.out
fixtext format_mtt_nlhe_balances2.out
call mydl out
