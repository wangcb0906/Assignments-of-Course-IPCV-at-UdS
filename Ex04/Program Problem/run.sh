input="boats.pgm"
tests=("whole" "8X8" "whole_high_frequency_removed" "8X8_high_frequency_removed" "8X8_equal_quatified" "8X8_jpeg_quantified")
for ((a=0;a<6;a++))
do
    echo $input > tes.txt
    echo ./results/${tests[${a}]}_spectrum.pgm >> tes.txt
    echo ./results/${tests[${a}]}_backtransfered.pgm >> tes.txt
    b=$[a+1]
    echo $b >> tes.txt
    ./dct < tes.txt >> 1.log.txt
done
rm tes.txt