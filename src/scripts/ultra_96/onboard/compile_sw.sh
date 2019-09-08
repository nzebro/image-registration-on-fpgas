g++ -D"ULTRA" -std=c++11 -pthread *.cpp -lcma -o iron-standalone `pkg-config --cflags --libs opencv`
echo "[INFO] Add define AVERAGE_REPS if want average execution time of hw and sw"
