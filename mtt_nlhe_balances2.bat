@echo off
pushd \users\lloyd\pokerstars
\users\lloyd\tools\last 1 delta6_summary.out
type mtt_nlhe_balance.out
\users\lloyd\tools\last 1 2020_balance.out
\users\lloyd\tools\last 1 delta6_summary.blue_distance2
type balance.out
popd
