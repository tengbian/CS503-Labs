cd ../rdserver
./rdserver 33560
make clean
make

cd ../compile
if make; then
	echo "Succeed"
	cs-console
else
	echo "Failed"
fi


