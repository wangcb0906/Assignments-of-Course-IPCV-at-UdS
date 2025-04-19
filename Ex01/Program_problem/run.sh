input="boat.pgm"
qs=(1 3 6)
for ((a=1;a<=2;a++))
do
    for q in ${qs[@]}
    do
        echo $input > tes.txt
        echo $q >> tes.txt
        echo $a >> tes.txt
        output="./results/quantisation_image_with_q_equals_$q"
        if [ "$a" -eq 2 ]; then
            output=${output}"_and_noise"
        fi
        output=$output".pgm"
        echo $output >> tes.txt
        ./quantisation < tes.txt >> 1.log.txt
    done
done
rm tes.txt