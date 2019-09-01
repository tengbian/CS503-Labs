cd ../compile
#make clean
#make rebuild
if make; then
	echo "Succeed"
	cs-console
else
	echo "Failed"
fi
#cd "$(../compile "$0")"
#cs-console


