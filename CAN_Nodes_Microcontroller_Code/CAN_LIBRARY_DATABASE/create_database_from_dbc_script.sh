echo "Creating CAN_DB .h, .c and csv files from CAN_DB.dbc"

cd dbc_to_c
make -n clean
echo "Generating!"
./dbcc -v -o ../src/ ../CAN_DB.dbc
./dbcc -C -v -o ../ ../CAN_DB.dbc
cd ../
echo "Done!"

