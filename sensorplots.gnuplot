#!/usr/bin/gnuplot

set terminal postscript eps size 2.5,1.5 enhanced color font 'Helvetica,10'

set output 'sensorfusion.eps'
set title 'Angular Probability'
set xlabel 'Obstacle Angle'
set ylabel 'Probability'
set key left

plot 'left.csv' with lines title "Left Sensor", 'leftmid.csv' with lines title "Left+Mid Sensors", 'mid.csv' with lines title "Mid Sensor", 'rightmid.csv' with lines title "Mid+Right Sensors", 'right.csv' with lines title "Right Sensor", 'inferred.csv' with linespoints  title "Inferred Distribution" lc -1
#pause -1

set terminal postscript eps size 2.5,1.5 enhanced color font 'Helvetica,10'

set output 'leftmotormovement.eps'
set title 'Left Motor Movement Probability'
set xlabel 'Obstacle Angle'
set ylabel 'Probability'
set key top
plot 'left_motor_forward.csv' with lines title "Motor Forward", 'left_motor_reverse.csv' with lines title "Motor Reverse", 'left_inferred_forward.csv' with linespoints title "Inferred Probability Forward", 'left_inferred_reverse.csv' with linespoints title "Inferred Probability reverse"

set output 'rightmotormovement.eps'
set title 'Right Motor Movement Probability'
set xlabel 'Obstacle Angle'
set ylabel 'Probability'
set key mid
plot 'right_motor_forward.csv' with lines title "Motor Forward", 'right_motor_reverse.csv' with lines title "Motor Reverse", 'right_inferred_forward.csv' with linespoints title "Inferred Probability Forward", 'right_inferred_reverse.csv' with linespoints title "Inferred Probability reverse"
