make compile
#echo "Checking normal"
#diff <(make run | sort) <(make runseq | sort)
#echo "Checking long"
#diff <(make runlong | sort) <(make runlongseq | sort)
#echo "Checking large"
#diff <(make runlarge | sort) <(make runlargeseq | sort)
#echo "Checking largelong"
#diff <(make runlargelong | sort) <(make runlargelongseq | sort)
#echo "Checking diff numbers of procs"
#echo "5 procs"
#diff <(make run5 | sort) <(make runseq | sort)


echo "Checking normal"
diff <(make run) <(make runseq)
echo "Checking long"
diff <(make runlong) <(make runlongseq)
echo "Checking large"
diff <(make runlarge) <(make runlargeseq)
echo "Checking largelong"
diff <(make runlargelong) <(make runlargelongseq)
echo "Checking diff numbers of procs"
echo "5 procs"
diff <(make run5) <(make runseq)

