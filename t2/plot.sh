#!/bin/bash
gnuplot -e "set term png" -e "set output 'plot_f$2.png'" -e "set zeroaxis" -e "set samples 1000" -e "plot $1"
gnuplot -e "set term svg" -e "set output 'plot_f$2.svg'" -e "set zeroaxis" -e "set samples 1000" -e "plot $1"
