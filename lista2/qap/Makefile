build:
	@echo "Building the project"
	@mpic++ -o out.out src/*.cpp
	@echo "Build complete"

run:build
	@echo "Running the project"
	@mpiexec -n 5 ./out.out $(n) $(file)
	@rm out.out

run_esc16i:build
	@echo "Running the project"
	@mpiexec -n 5 ./out.out 16 ./data/esc16i.dat
	@rm out.out
