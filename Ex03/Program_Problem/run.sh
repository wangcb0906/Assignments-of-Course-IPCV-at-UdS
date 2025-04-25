ins=("smoke" "fire")
heights=(0 1)
for input in ${ins[@]}
do
    for height in ${heights[@]}
    do
        echo $input.pgm > tes.txt
        echo result/$input.h=$height.log.spectrum.pgm >> tes.txt
        echo result/$input.h=$height.backtrans.pgm >> tes.txt
        echo $height >> tes.txt
        ./DFT < tes.txt >> 1.log.txt
    done
done
rm tes.txt