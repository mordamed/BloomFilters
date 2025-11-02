# Remove build artifacts and plots
dist-clean: clean
	rm -f $(OUT) || del /Q $(OUT)
	@echo "Removed binaries"

run :
	g++ -std=c++17 experiments/TestBloom.cpp src/BloomFilter.cpp -Iinclude -O2 -o test_bloom
	.\test_bloom.exe 
	python scripts/plot_results.py results/results.csv