input="kodim23.ppm"
ss=(1 2 4 8)
echo "log starts" > 1.log.txt
for s in ${ss[@]}
do
    echo $input > tes.txt
    echo $s >> tes.txt
    output="./result/output_q_equals_${s}"
    output=${output}".ppm"
    # if [ "$a" -eq 2 ]; then
    #     output=${output}"_and_noise"
    # fi
    echo $output >> tes.txt
    ./YCbCr < tes.txt >> 1.log.txt
done
rm tes.txt